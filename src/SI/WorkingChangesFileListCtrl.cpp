// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2015 - TortoiseGit
// Copyright (C) 2003-2008, 2013 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "resource.h"
#include "..\\TortoiseShell\resource.h"
#include "WorkingChangesFileListCtrl.h"
#include "MessageBox.h"
#include "MyMemDC.h"
#include "UnicodeUtils.h"
#include "AppUtils.h"
#include "PathUtils.h"
#include "TempFile.h"
#include "StringUtils.h"
#include "DirFileEnum.h"
#include "LoglistUtils.h"
#include "Git.h"
#include "GitDiff.h"
#include "GitProgressDlg.h"
#include "SysImageList.h"
#include "TGitPath.h"
#include "registry.h"
#include "InputDlg.h"
#include "ShellUpdater.h"
#include "GitAdminDir.h"
#include "DropFiles.h"
#include "ProgressCommands/AddProgressCommand.h"
#include "IconMenu.h"
#include "FormatMessageWrapper.h"
#include "BrowseFolder.h"

// registry version number of column-settings of GitLogListBase
#define GITSLC_COL_VERSION 5

#ifndef assert
#define assert(x) ATLASSERT(x)
#endif

const UINT CWorkingChangesFileListCtrl::GITSLNM_ITEMCOUNTCHANGED = ::RegisterWindowMessage(_T("GITSLNM_ITEMCOUNTCHANGED"));
const UINT CWorkingChangesFileListCtrl::GITSLNM_NEEDSREFRESH     = ::RegisterWindowMessage(_T("GITSLNM_NEEDSREFRESH"));
const UINT CWorkingChangesFileListCtrl::GITSLNM_ADDFILE          = ::RegisterWindowMessage(_T("GITSLNM_ADDFILE"));
const UINT CWorkingChangesFileListCtrl::GITSLNM_CHECKCHANGED     = ::RegisterWindowMessage(_T("GITSLNM_CHECKCHANGED"));
const UINT CWorkingChangesFileListCtrl::GITSLNM_ITEMCHANGED      = ::RegisterWindowMessage(_T("GITSLNM_ITEMCHANGED"));


BEGIN_MESSAGE_MAP(CWorkingChangesFileListCtrl, CListCtrl)
	ON_NOTIFY(HDN_ITEMCLICKA,            0,  CWorkingChangesFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ITEMCLICKW,            0,  CWorkingChangesFileListCtrl::OnHdnItemclick)
	ON_NOTIFY(HDN_ENDTRACK,              0,  CWorkingChangesFileListCtrl::OnColumnResized)
	ON_NOTIFY(HDN_ENDDRAG,               0,  CWorkingChangesFileListCtrl::OnColumnMoved)
	ON_NOTIFY(HDN_BEGINTRACKA,           0, &CWorkingChangesFileListCtrl::OnHdnBegintrack)
	ON_NOTIFY(HDN_BEGINTRACKW,           0, &CWorkingChangesFileListCtrl::OnHdnBegintrack)
	ON_NOTIFY(HDN_ITEMCHANGINGA,         0, &CWorkingChangesFileListCtrl::OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW,         0, &CWorkingChangesFileListCtrl::OnHdnItemchanging)
	ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED,    CWorkingChangesFileListCtrl::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK,             CWorkingChangesFileListCtrl::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP,        CWorkingChangesFileListCtrl::OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,         CWorkingChangesFileListCtrl::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_RETURN,             CWorkingChangesFileListCtrl::OnNMReturn)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG,         CWorkingChangesFileListCtrl::OnBeginDrag)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING,     &CWorkingChangesFileListCtrl::OnLvnItemchanging)
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// Constructor
CWorkingChangesFileListCtrl::CWorkingChangesFileListCtrl() : CListCtrl()
	, m_pbCanceled(NULL)
	, m_pStatLabel(NULL)
	, m_pSelectButton(NULL)
	, m_pConfirmButton(NULL)
	, m_bBusy(false)
	, m_bEmpty(false)
	, m_bShowIgnores(false)
	, m_pDropTarget(NULL)
	, m_bIgnoreRemoveOnly(false)
	, m_bCheckChildrenWithParent(false)
	, m_bUnversionedLast(true)
	, m_bHasChangeLists(false)
	, m_bHasLocks(false)
	, m_bBlock(false)
	, m_bBlockUI(false)
	, m_bHasCheckboxes(false)
	, m_bCheckIfGroupsExist(true)
	, m_bFileDropsEnabled(false)
	, m_bOwnDrag(false)
	, m_dwDefaultColumns(0)
	, m_ColumnManager(this)
	, m_bAscending(false)
	, m_nSortedColumn(-1)
	, m_bHasExternalsFromDifferentRepos(false)
	, m_sNoPropValueText(MAKEINTRESOURCE(IDS_STATUSLIST_NOPROPVALUE))
	, m_amend(false)
	, m_bDoNotAutoselectSubmodules(false)
	, m_bHasWC(true)
	, m_hwndLogicalParent(NULL)
	, m_bHasUnversionedItems(FALSE)
	, m_nTargetCount(0)
	, m_bHasExternals(false)
	, m_bHasIgnoreGroup(false)
	, m_nUnversioned(0)
	, m_nNormal(0)
	, m_nModified(0)
	, m_nAdded(0)
	, m_nDeleted(0)
	, m_nConflicted(0)
	, m_nTotal(0)
	, m_nSelected(0)
	, m_nRenamed(0)
	, m_nShownUnversioned(0)
	, m_nShownModified(0)
	, m_nShownAdded(0)
	, m_nShownDeleted(0)
	, m_nShownConflicted(0)
	, m_nShownFiles(0)
	, m_nShownSubmodules(0)
	, m_dwShow(0)
	, m_bShowFolders(false)
	, m_bUpdate(false)
	, m_dwContextMenus(0)
	, m_nIconFolder(0)
	, m_nRestoreOvl(0)
	, m_FileLoaded(0)
{
	m_critSec.Init();
	m_bIsRevertTheirMy = false;
	m_nLineAdded = m_nLineDeleted = 0;
}

// Destructor
CWorkingChangesFileListCtrl::~CWorkingChangesFileListCtrl()
{
}


void CWorkingChangesFileListCtrl::Init(
	DWORD dwColumns, 
	const CString& sColumnInfoContainer, 
	unsigned __int64 dwContextMenus /* = GitSLC_POPALL */, 
	bool bHasCheckboxes /* = true */, 
	bool bHasWC /* = true */, 
	DWORD allowedColumns /* = 0xffffffff */)
{
	Locker lock(m_critSec);

	m_dwDefaultColumns = dwColumns | 1;
	m_dwContextMenus = dwContextMenus;
	m_bHasCheckboxes = bHasCheckboxes;
	m_bHasWC = bHasWC;

	// set the extended style of the listcontrol
	// the style LVS_EX_FULLROWSELECT interferes with the background watermark image but it's more important to be able to select in the whole row.
	CRegDWORD regFullRowSelect(_T("Software\\TortoiseGit\\FullRowSelect"), TRUE);
	DWORD exStyle = LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_SUBITEMIMAGES;
	if (DWORD(regFullRowSelect))
		exStyle |= LVS_EX_FULLROWSELECT;
	exStyle |= (bHasCheckboxes ? LVS_EX_CHECKBOXES : 0);
	SetRedraw(false);
	SetExtendedStyle(exStyle);

	SetWindowTheme(m_hWnd, L"Explorer", NULL);

	m_nIconFolder = SYS_IMAGE_LIST().GetDirIconIndex();
	m_nRestoreOvl = SYS_IMAGE_LIST().AddIcon((HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_RESTOREOVL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
	SYS_IMAGE_LIST().SetOverlayImage(m_nRestoreOvl, OVL_RESTORE);
	SetImageList(&SYS_IMAGE_LIST(), LVSIL_SMALL);

	// keep CSorter::operator() in sync!!
	static UINT standardColumnNames[SIWC_NUMCOLUMNS]= { 
        IDS_STATUSLIST_COLFILE, 
		IDS_STATUSLIST_COLFILENAME, 
		IDS_STATUSLIST_COLEXT, 
		IDS_STATUSLIST_COLSTATUS, 
		IDS_STATUSLIST_COLADD, 
		IDS_STATUSLIST_COLDEL, 
		IDS_STATUSLIST_COLLASTMODIFIED, 
		IDS_STATUSLIST_COLSIZE
	};

	m_ColumnManager.SetNames(standardColumnNames,SIWC_NUMCOLUMNS);
	m_ColumnManager.ReadSettings(m_dwDefaultColumns, 0xffffffff & ~(allowedColumns | m_dwDefaultColumns), sColumnInfoContainer, SIWC_NUMCOLUMNS);

	SetRedraw(true);
}


bool CWorkingChangesFileListCtrl::SetBackgroundImage(UINT nID)
{
	return CAppUtils::SetListCtrlBackgroundImage(GetSafeHwnd(), nID);
}


BOOL CWorkingChangesFileListCtrl::GetStatus ( const CTGitPathList* pathList
									, bool bUpdate /* = FALSE */
									, bool bShowIgnores /* = false */
									, bool bShowUnRev /* = false */
									, bool bShowLocalChangesIgnored /* = false */)
{
	Locker lock(m_critSec);
	int mask= CWorkingChangesFileListCtrl::FILELIST_MODIFY;
	if(bShowIgnores)
		mask|= CWorkingChangesFileListCtrl::FILELIST_IGNORE;
	if(bShowUnRev)
		mask|= CWorkingChangesFileListCtrl::FILELIST_UNVER;
	if (bShowLocalChangesIgnored)
		mask |= CWorkingChangesFileListCtrl::FILELIST_LOCALCHANGESIGNORED;
	this->UpdateFileList(mask,bUpdate,(CTGitPathList*)pathList);

	if (pathList && m_mapDirectFiles.empty())
	{
		// remember files which are selected by users so that those can be preselected
		for (int i = 0; i < pathList->GetCount(); ++i)
			if (!(*pathList)[i].IsDirectory())
				m_mapDirectFiles[(*pathList)[i].GetGitPathString()] = true;
	}

	BuildStatistics();
	return TRUE;
}


// Get the show-flags bitmap value which corresponds to a particular Git status
DWORD CWorkingChangesFileListCtrl::GetShowFlagsFromGitStatus(git_wc_status_kind status)
{
	switch (status)
	{
	case git_wc_status_none:
	case git_wc_status_unversioned:
		return GITSLC_SHOWUNVERSIONED;
	case git_wc_status_ignored:
		if (!m_bShowIgnores)
			return GITSLC_SHOWDIRECTS;
		return GITSLC_SHOWDIRECTS|GITSLC_SHOWIGNORED;
	case git_wc_status_incomplete:
		return GITSLC_SHOWINCOMPLETE;
	case git_wc_status_normal:
		return GITSLC_SHOWNORMAL;
	case git_wc_status_external:
		return GITSLC_SHOWEXTERNAL;
	case git_wc_status_added:
		return GITSLC_SHOWADDED;
	case git_wc_status_missing:
		return GITSLC_SHOWMISSING;
	case git_wc_status_deleted:
		return GITSLC_SHOWREMOVED;
	case git_wc_status_replaced:
		return GITSLC_SHOWREPLACED;
	case git_wc_status_modified:
		return GITSLC_SHOWMODIFIED;
	case git_wc_status_merged:
		return GITSLC_SHOWMERGED;
	case git_wc_status_conflicted:
		return GITSLC_SHOWCONFLICTED;
	case git_wc_status_obstructed:
		return GITSLC_SHOWOBSTRUCTED;
	default:
		// we should NEVER get here!
		ASSERT(FALSE);
		break;
	}
	return 0;
}


void CWorkingChangesFileListCtrl::Show(unsigned int dwShow, unsigned int dwCheck /*=0*/, bool /*bShowFolders*/ /* = true */,BOOL UpdateStatusList,bool UseStoredCheckStatus)
{
	CWinApp * pApp = AfxGetApp();
	if (pApp)
		pApp->DoWaitCursor(1);

	Locker lock(m_critSec);
	WORD langID = (WORD)CRegStdDWORD(_T("Software\\TortoiseGit\\LanguageID"), GetUserDefaultLangID());

	//SetItemCount(listIndex);
	SetRedraw(FALSE);
	DeleteAllItems();
	m_nSelected = 0;

	m_nShownUnversioned = 0;
	m_nShownModified = 0;
	m_nShownAdded = 0;
	m_nShownDeleted = 0;
	m_nShownConflicted = 0;
	m_nShownFiles = 0;
	m_nShownSubmodules = 0;

	if(UpdateStatusList)
	{
		m_arStatusArray.clear();
		for (int i = 0; i < m_StatusFileList.GetCount(); ++i)
		{
			m_arStatusArray.push_back((CTGitPath*)&m_StatusFileList[i]);
		}

		for (int i = 0; i < m_UnRevFileList.GetCount(); ++i)
		{
			m_arStatusArray.push_back((CTGitPath*)&m_UnRevFileList[i]);
		}

		for (int i = 0; i < m_IgnoreFileList.GetCount(); ++i)
		{
			m_arStatusArray.push_back((CTGitPath*)&m_IgnoreFileList[i]);
		}
	}
	PrepareGroups();
	if (m_nSortedColumn >= 0)
	{
		CSorter predicate (&m_ColumnManager, m_nSortedColumn, m_bAscending);
		std::sort(m_arStatusArray.begin(), m_arStatusArray.end(), predicate);
	}

	int index =0;
	for (size_t i = 0; i < m_arStatusArray.size(); ++i)
	{
		//set default checkbox status
		CTGitPath* entry = ((CTGitPath*)m_arStatusArray[i]);
		CString path = entry->GetGitPathString();
		if (!m_mapFilenameToChecked.empty() && m_mapFilenameToChecked.find(path) != m_mapFilenameToChecked.end())
		{
			entry->m_Checked=m_mapFilenameToChecked[path];
		}
		else if (!UseStoredCheckStatus)
		{
			bool autoSelectSubmodules = !(entry->IsDirectory() && m_bDoNotAutoselectSubmodules);
			if ((entry->m_Action & dwCheck || dwShow & GITSLC_SHOWDIRECTFILES && m_mapDirectFiles.find(path) != m_mapDirectFiles.end()) && autoSelectSubmodules)
				entry->m_Checked=true;
			else
				entry->m_Checked=false;
			m_mapFilenameToChecked[path] = entry->m_Checked;
		}

		if(entry->m_Action & dwShow)
		{
			AddEntry(entry,langID,index);
			index++;
		}
	}

	int maxcol = ((CHeaderCtrl*)(GetDlgItem(0)))->GetItemCount()-1;
	for (int col = 0; col <= maxcol; col++)
		SetColumnWidth (col, m_ColumnManager.GetWidth (col, true));

	SetRedraw(TRUE);
	GetStatisticsString();

	CHeaderCtrl * pHeader = GetHeaderCtrl();
	HDITEM HeaderItem = {0};
	HeaderItem.mask = HDI_FORMAT;
	for (int i=0; i<pHeader->GetItemCount(); ++i)
	{
		pHeader->GetItem(i, &HeaderItem);
		HeaderItem.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		pHeader->SetItem(i, &HeaderItem);
	}
	if (m_nSortedColumn >= 0)
	{
		pHeader->GetItem(m_nSortedColumn, &HeaderItem);
		HeaderItem.fmt |= (m_bAscending ? HDF_SORTUP : HDF_SORTDOWN);
		pHeader->SetItem(m_nSortedColumn, &HeaderItem);
	}

	if (pApp)
		pApp->DoWaitCursor(-1);

	Invalidate();

	m_dwShow = dwShow;

	this->BuildStatistics();
}


void CWorkingChangesFileListCtrl::Show(unsigned int /*dwShow*/, const CTGitPathList& checkedList, bool /*bShowFolders*/ /* = true */)
{
	DeleteAllItems();
	for (int i = 0; i < checkedList.GetCount(); ++i)
		this->AddEntry((CTGitPath *)&checkedList[i],0,i);
	return ;
}


int CWorkingChangesFileListCtrl::GetColumnIndex(int mask)
{
	for (int i = 0; i < 32; ++i)
		if(mask&0x1)
			return i;
		else
			mask=mask>>1;
	return -1;
}


void CWorkingChangesFileListCtrl::AddEntry(CTGitPath * GitPath, WORD /*langID*/, int listIndex)
{
	static CString from(MAKEINTRESOURCE(IDS_STATUSLIST_FROM));
	static HINSTANCE hResourceHandle(AfxGetResourceHandle());
	static bool abbreviateRenamings(((DWORD)CRegDWORD(_T("Software\\TortoiseGit\\AbbreviateRenamings"), FALSE)) == TRUE);
	static bool relativeTimes = (CRegDWORD(_T("Software\\TortoiseGit\\RelativeTimes"), FALSE) != FALSE);

	CString path = GitPath->GetGitPathString();

	m_bBlock = TRUE;
	int index = listIndex;
	int nCol = 1;
	CString entryname = GitPath->GetGitPathString();
	int icon_idx = 0;
	if (GitPath->IsDirectory())
	{
		icon_idx = m_nIconFolder;
		m_nShownSubmodules++;
	}
	else
	{
		icon_idx = SYS_IMAGE_LIST().GetPathIconIndex(*GitPath);
		m_nShownFiles++;
	}
	switch (GitPath->m_Action)
	{
	case CTGitPath::LOGACTIONS_ADDED:
	case CTGitPath::LOGACTIONS_COPY:
		m_nShownAdded++;
		break;
	case CTGitPath::LOGACTIONS_DELETED:
		m_nShownDeleted++;
		break;
	case CTGitPath::LOGACTIONS_REPLACED:
	case CTGitPath::LOGACTIONS_MODIFIED:
	case CTGitPath::LOGACTIONS_MERGED:
		m_nShownModified++;
		break;
	case CTGitPath::LOGACTIONS_UNMERGED:
		m_nShownConflicted++;
		break;
	case CTGitPath::LOGACTIONS_UNVER:
		m_nShownUnversioned++;
		break;
	default:
		m_nShownUnversioned++;
		break;
	}
	if(GitPath->m_Action & (CTGitPath::LOGACTIONS_REPLACED|CTGitPath::LOGACTIONS_COPY) && !GitPath->GetGitOldPathString().IsEmpty())
	{
		if (!abbreviateRenamings)
		{
			// relative path
			CString rename;
			rename.Format(from, GitPath->GetGitOldPathString());
			entryname += _T(" ") + rename;
		}
		else
		{
			CTGitPathList tgpl;
			tgpl.AddPath(*GitPath);
			CTGitPath old(GitPath->GetGitOldPathString());
			tgpl.AddPath(old);
			CString commonRoot = tgpl.GetCommonRoot().GetGitPathString();
			if (!commonRoot.IsEmpty())
				commonRoot += _T("/");
			if (old.GetFileOrDirectoryName() == GitPath->GetFileOrDirectoryName() && old.GetContainingDirectory().GetGitPathString() != "" && GitPath->GetContainingDirectory().GetGitPathString())
			{
				entryname = commonRoot + _T("{") + GitPath->GetGitOldPathString().Mid(commonRoot.GetLength(), old.GetGitPathString().GetLength() - commonRoot.GetLength() - old.GetFileOrDirectoryName().GetLength() - 1) + _T(" => ") + GitPath->GetGitPathString().Mid(commonRoot.GetLength(), GitPath->GetGitPathString().GetLength() - commonRoot.GetLength() - old.GetFileOrDirectoryName().GetLength() - 1) +  _T("}/") + old.GetFileOrDirectoryName();
			}
			else if (!commonRoot.IsEmpty())
			{
				entryname = commonRoot + _T("{") + GitPath->GetGitOldPathString().Mid(commonRoot.GetLength()) + _T(" => ") + GitPath->GetGitPathString().Mid(commonRoot.GetLength()) + _T("}");
			}
			else
			{
				entryname = GitPath->GetGitOldPathString().Mid(commonRoot.GetLength()) + _T(" => ") + GitPath->GetGitPathString().Mid(commonRoot.GetLength());
			}
		}
	}

	InsertItem(index, entryname, icon_idx);
	if (m_restorepaths.find(GitPath->GetWinPathString()) != m_restorepaths.end())
		SetItemState(index, INDEXTOOVERLAYMASK(OVL_RESTORE), TVIS_OVERLAYMASK);

	this->SetItemData(index, (DWORD_PTR)GitPath);
	// SVNSLC_COLFILENAME
	SetItemText(index, nCol++, GitPath->GetFileOrDirectoryName());
	// SVNSLC_COLEXT
	SetItemText(index, nCol++, GitPath->GetFileExtension());
	// SVNSLC_COLSTATUS
	SetItemText(index, nCol++, GitPath->GetActionName());

	SetItemText(index, GetColumnIndex(GITSLC_COLADD),GitPath->m_StatAdd);
	SetItemText(index, GetColumnIndex(GITSLC_COLDEL),GitPath->m_StatDel);

	{
		CString modificationDate;
		__int64 filetime = GitPath->GetLastWriteTime();
		if (filetime && (GitPath->m_Action != CTGitPath::LOGACTIONS_DELETED))
		{
			FILETIME* f = (FILETIME*)(__int64*)&filetime;
			modificationDate = CLoglistUtils::FormatDateAndTime(CTime(g_Git.filetime_to_time_t(f)), DATE_SHORTDATE, true, relativeTimes);
		}
		SetItemText(index, GetColumnIndex(GITSLC_COLMODIFICATIONDATE), modificationDate);
	}
	// SVNSLC_COLSIZE
	if (GitPath->IsDirectory())
		SetItemText(index, GetColumnIndex(GITSLC_COLSIZE), _T(""));
	else
	{
		TCHAR buf[100] = { 0 };
		StrFormatByteSize64(GitPath->GetFileSize(), buf, 100);
		SetItemText(index, GetColumnIndex(GITSLC_COLSIZE), buf);
	}

	SetCheck(index, GitPath->m_Checked);
	if (GitPath->m_Checked)
		m_nSelected++;

	if ((GitPath->m_Action & CTGitPath::LOGACTIONS_SKIPWORKTREE) || (GitPath->m_Action & CTGitPath::LOGACTIONS_ASSUMEVALID))
		SetItemGroup(index, 3);
	else if (GitPath->m_Action & CTGitPath::LOGACTIONS_IGNORE)
		SetItemGroup(index, 2);
	else if( GitPath->m_Action & CTGitPath::LOGACTIONS_UNVER)
		SetItemGroup(index,1);
	else
	{
		SetItemGroup(index, GitPath->m_ParentNo&(PARENT_MASK|MERGE_MASK));
	}

	m_bBlock = FALSE;


}


bool CWorkingChangesFileListCtrl::SetItemGroup(int item, int groupindex)
{
	if (groupindex < 0)
		return false;
	LVITEM i = {0};
	i.mask = LVIF_GROUPID;
	i.iItem = item;
	i.iSubItem = 0;
	i.iGroupId = groupindex;

	return !!SetItem(&i);
}


void CWorkingChangesFileListCtrl::OnHdnItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
	if (m_bBlock || m_arStatusArray.empty())
		return;
	m_bBlock = TRUE;
	if (m_nSortedColumn == phdr->iItem)
		m_bAscending = !m_bAscending;
	else
		m_bAscending = TRUE;
	m_nSortedColumn = phdr->iItem;
	Show(m_dwShow, 0, m_bShowFolders,false,true);

	m_bBlock = FALSE;
}


void CWorkingChangesFileListCtrl::OnLvnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

#define ISCHECKED(x) ((x) ? ((((x)&LVIS_STATEIMAGEMASK)>>12)-1) : FALSE)
	if ((m_bBlock)&&(m_bBlockUI))
	{
		// if we're blocked, prevent changing of the check state
		if ((!ISCHECKED(pNMLV->uOldState) && ISCHECKED(pNMLV->uNewState))||
			(ISCHECKED(pNMLV->uOldState) && !ISCHECKED(pNMLV->uNewState)))
			*pResult = TRUE;
	}
}


BOOL CWorkingChangesFileListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
	CWnd* pParent = GetLogicalParent();
	if (NULL != pParent && NULL != pParent->GetSafeHwnd())
	{
		pParent->SendMessage(GITSLNM_ITEMCHANGED, pNMLV->iItem);
	}

	if ((pNMLV->uNewState==0)||(pNMLV->uNewState & LVIS_SELECTED)||(pNMLV->uNewState & LVIS_FOCUSED))
		return FALSE;

	if (m_bBlock)
		return FALSE;

	bool bSelected = !!(ListView_GetItemState(m_hWnd, pNMLV->iItem, LVIS_SELECTED) & LVIS_SELECTED);
	int nListItems = GetItemCount();

	m_bBlock = TRUE;
	// was the item checked?

	//CTGitPath *gitpath=(CTGitPath*)GetItemData(pNMLV->iItem);
	//gitpath->m_Checked=GetCheck(pNMLV->iItem);

	if (GetCheck(pNMLV->iItem))
	{
		CheckEntry(pNMLV->iItem, nListItems);
		if (bSelected)
		{
			POSITION pos = GetFirstSelectedItemPosition();
			int index;
			while ((index = GetNextSelectedItem(pos)) >= 0)
			{
				if (index != pNMLV->iItem)
					CheckEntry(index, nListItems);
			}
		}
	}
	else
	{
		UncheckEntry(pNMLV->iItem, nListItems);
		if (bSelected)
		{
			POSITION pos = GetFirstSelectedItemPosition();
			int index;
			while ((index = GetNextSelectedItem(pos)) >= 0)
			{
				if (index != pNMLV->iItem)
					UncheckEntry(index, nListItems);
			}
		}
	}

	GetStatisticsString();
	m_bBlock = FALSE;
	NotifyCheck();

	return FALSE;
}


void CWorkingChangesFileListCtrl::OnColumnResized(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_ColumnManager.OnColumnResized(pNMHDR,pResult);

	*pResult = FALSE;
}


void CWorkingChangesFileListCtrl::OnColumnMoved(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_ColumnManager.OnColumnMoved(pNMHDR, pResult);

	Invalidate(FALSE);
}


void CWorkingChangesFileListCtrl::CheckEntry(int index, int /*nListItems*/)
{
	Locker lock(m_critSec);
	//FileEntry * entry = GetListEntry(index);
	CTGitPath *path=(CTGitPath*)GetItemData(index);
	ASSERT(path != NULL);
	if (path == NULL)
		return;
	m_mapFilenameToChecked[path->GetGitPathString()] = true;
	SetCheck(index, TRUE);

	if ( !path->m_Checked )
	{
		path->m_Checked = TRUE;
		m_nSelected++;
	}
}

void CWorkingChangesFileListCtrl::UncheckEntry(int index, int /*nListItems*/)
{
	Locker lock(m_critSec);
	CTGitPath *path=(CTGitPath*)GetItemData(index);
	ASSERT(path != NULL);
	if (path == NULL)
		return;
	SetCheck(index, FALSE);
	m_mapFilenameToChecked[path->GetGitPathString()] = false;

	if ( path->m_Checked )
	{
		path->m_Checked  = FALSE;
		m_nSelected--;
	}
}


bool CWorkingChangesFileListCtrl::BuildStatistics()
{

	bool bRefetchStatus = false;

	// now gather some statistics
	m_nUnversioned = 0;
	m_nNormal = 0;
	m_nModified = 0;
	m_nAdded = 0;
	m_nDeleted = 0;
	m_nConflicted = 0;
	m_nTotal = 0;
	m_nSelected = 0;
	m_nLineAdded = 0;
	m_nLineDeleted = 0;
	m_nRenamed = 0;

	for (size_t i = 0; i < m_arStatusArray.size(); ++i)
	{
		int status=((CTGitPath*)m_arStatusArray[i])->m_Action;

		m_nLineAdded += _tstol(((CTGitPath*)m_arStatusArray[i])->m_StatAdd);
		m_nLineDeleted += _tstol(((CTGitPath*)m_arStatusArray[i])->m_StatDel);

		if(status&(CTGitPath::LOGACTIONS_ADDED|CTGitPath::LOGACTIONS_COPY))
			m_nAdded++;

		if(status&CTGitPath::LOGACTIONS_DELETED)
			m_nDeleted++;

		if(status&(CTGitPath::LOGACTIONS_REPLACED|CTGitPath::LOGACTIONS_MODIFIED))
			m_nModified++;

		if(status&CTGitPath::LOGACTIONS_UNMERGED)
			m_nConflicted++;

		if(status&(CTGitPath::LOGACTIONS_IGNORE|CTGitPath::LOGACTIONS_UNVER))
			m_nUnversioned++;

		if(status&(CTGitPath::LOGACTIONS_REPLACED))
			m_nRenamed++;

		if(((CTGitPath*)m_arStatusArray[i])->m_Checked)
			m_nSelected++;
	}
	return !bRefetchStatus;
}


int CWorkingChangesFileListCtrl::GetGroupFromPoint(POINT * ppt)
{
	// the point must be relative to the upper left corner of the control

	if (ppt == NULL)
		return -1;
	if (!IsGroupViewEnabled())
		return -1;

	POINT pt = *ppt;
	pt.x = 10;
	UINT flags = 0;
	int nItem = -1;
	RECT rc;
	GetWindowRect(&rc);
	while (((flags & LVHT_BELOW) == 0)&&(pt.y < rc.bottom))
	{
		nItem = HitTest(pt, &flags);
		if ((flags & LVHT_ONITEM)||(flags & LVHT_EX_GROUP_HEADER))
		{
			// the first item below the point

			// check if the point is too much right (i.e. if the point
			// is farther to the right than the width of the item)
			RECT r;
			GetItemRect(nItem, &r, LVIR_LABEL);
			if (ppt->x > r.right)
				return -1;

			LVITEM lv = {0};
			lv.mask = LVIF_GROUPID;
			lv.iItem = nItem;
			GetItem(&lv);
			int groupID = lv.iGroupId;
			// now we search upwards and check if the item above this one
			// belongs to another group. If it belongs to the same group,
			// we're not over a group header
			while (pt.y >= 0)
			{
				pt.y -= 2;
				nItem = HitTest(pt, &flags);
				if ((flags & LVHT_ONITEM)&&(nItem >= 0))
				{
					// the first item below the point
					LVITEM lv = {0};
					lv.mask = LVIF_GROUPID;
					lv.iItem = nItem;
					GetItem(&lv);
					if (lv.iGroupId != groupID)
						return groupID;
					else
						return -1;
				}
			}
			if (pt.y < 0)
				return groupID;
			return -1;
		}
		pt.y += 2;
	};
	return -1;
}


void CWorkingChangesFileListCtrl::OnContextMenuGroup(CWnd * /*pWnd*/, CPoint point)
{
	POINT clientpoint = point;
	ScreenToClient(&clientpoint);
	if ((IsGroupViewEnabled())&&(GetGroupFromPoint(&clientpoint) >= 0))
	{
		CMenu popup;
		if (popup.CreatePopupMenu())
		{
			CString temp;
			temp.LoadString(IDS_STATUSLIST_CHECKGROUP);
			popup.AppendMenu(MF_STRING | MF_ENABLED, IDGITLC_CHECKGROUP, temp);
			temp.LoadString(IDS_STATUSLIST_UNCHECKGROUP);
			popup.AppendMenu(MF_STRING | MF_ENABLED, IDGITLC_UNCHECKGROUP, temp);
			int cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this, 0);
			bool bCheck = false;
			switch (cmd)
			{
			case IDGITLC_CHECKGROUP:
				bCheck = true;
				// fall through here
			case IDGITLC_UNCHECKGROUP:
				{
					int group = GetGroupFromPoint(&clientpoint);
					// go through all items and check/uncheck those assigned to the group
					// but block the OnLvnItemChanged handler
					m_bBlock = true;
					LVITEM lv;
					for (int i=0; i<GetItemCount(); ++i)
					{
						SecureZeroMemory(&lv, sizeof(LVITEM));
						lv.mask = LVIF_GROUPID;
						lv.iItem = i;
						GetItem(&lv);

						if (lv.iGroupId == group)
						{
							CTGitPath * entry = (CTGitPath*)GetItemData(i);
							if (entry)
							{
								bool bOldCheck = entry->m_Checked;
								SetEntryCheck(entry, i, bCheck);
								if (bCheck != bOldCheck)
								{
									if (bCheck)
										m_nSelected++;
									else
										m_nSelected--;
								}
							}
						}

					}
					GetStatisticsString();
					NotifyCheck();
					m_bBlock = false;
				}
				break;
			}
		}
	}
}


void CWorkingChangesFileListCtrl::OnContextMenuList(CWnd * pWnd, CPoint point)
{

	//WORD langID = (WORD)CRegStdDWORD(_T("Software\\TortoiseGit\\LanguageID"), GetUserDefaultLangID());

	//bool bShift = !!(GetAsyncKeyState(VK_SHIFT) & 0x8000);
	CTGitPath * filepath;

	int selIndex = GetSelectionMark();
	if ((point.x == -1) && (point.y == -1))
	{
		CRect rect;
		GetItemRect(selIndex, &rect, LVIR_LABEL);
		ClientToScreen(&rect);
		point = rect.CenterPoint();
	}
	if ((GetSelectedCount() == 0) && (m_bHasCheckboxes))
	{
		// nothing selected could mean the context menu is requested for
		// a group header
		OnContextMenuGroup(pWnd, point);
	}
	else if (selIndex >= 0)
	{
		//FileEntry * entry = GetListEntry(selIndex);

		filepath = (CTGitPath * )GetItemData(selIndex);

		ASSERT(filepath != NULL);
		if (filepath == NULL)
			return;

		//const CTGitPath& filepath = entry->path;
		int wcStatus = filepath->m_Action;
		// entry is selected, now show the popup menu
		Locker lock(m_critSec);
		CIconMenu popup;
		CMenu changelistSubMenu;
		CMenu ignoreSubMenu;
		if (popup.CreatePopupMenu())
		{
			//Add Menu for version controlled file

			if (GetSelectedCount() > 0 && wcStatus & CTGitPath::LOGACTIONS_UNMERGED)
			{
				if (GetSelectedCount() == 1 && (m_dwContextMenus & GITSLC_POPCONFLICT)/*&&(entry->textstatus == git_wc_status_conflicted)*/)
				{
					popup.AppendMenuIcon(IDGITLC_EDITCONFLICT, IDS_MENUCONFLICT, IDI_CONFLICT);
				}
				if (m_dwContextMenus & GITSLC_POPRESOLVE)
				{
					popup.AppendMenuIcon(IDGITLC_RESOLVECONFLICT, IDS_STATUSLIST_CONTEXT_RESOLVED, IDI_RESOLVE);
				}
				if ((m_dwContextMenus & GITSLC_POPRESOLVE)/*&&(entry->textstatus == git_wc_status_conflicted)*/)
				{
					popup.AppendMenuIcon(IDGITLC_RESOLVETHEIRS, IDS_SVNPROGRESS_MENUUSETHEIRS, IDI_RESOLVE);
					popup.AppendMenuIcon(IDGITLC_RESOLVEMINE, IDS_SVNPROGRESS_MENUUSEMINE, IDI_RESOLVE);
				}
				if ((m_dwContextMenus & GITSLC_POPCONFLICT)||(m_dwContextMenus & GITSLC_POPRESOLVE))
					popup.AppendMenu(MF_SEPARATOR);
			}

			if (GetSelectedCount() > 0)
			{
				if (wcStatus & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE))
				{
					if (m_dwContextMenus & GITSLC_POPADD)
					{
						//if ( entry->IsFolder() )
						//{
						//	popup.AppendMenuIcon(IDSVNLC_ADD_RECURSIVE, IDS_STATUSLIST_CONTEXT_ADD_RECURSIVE, IDI_ADD);
						//}
						//else
						{
							popup.AppendMenuIcon(IDGITLC_ADD, IDS_STATUSLIST_CONTEXT_ADD, IDI_ADD);
						}
					}
					if (m_dwContextMenus & GITSLC_POPCOMMIT)
					{
						popup.AppendMenuIcon(IDGITLC_COMMIT, IDS_STATUSLIST_CONTEXT_COMMIT, IDI_COMMIT);
					}
				}
			}

			if (!(wcStatus & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE)) && GetSelectedCount() > 0)
			{
				bool bEntryAdded = false;
				if (m_dwContextMenus & GITSLC_POPCOMPAREWITHBASE)
				{
					if(filepath->m_ParentNo & MERGE_MASK)
						popup.AppendMenuIcon(IDGITLC_COMPARE, IDS_TREE_DIFF, IDI_DIFF);
					else
						popup.AppendMenuIcon(IDGITLC_COMPARE, IDS_LOG_COMPAREWITHBASE, IDI_DIFF);

					popup.SetDefaultItem(IDGITLC_COMPARE, FALSE);
					bEntryAdded = true;
				}

				if (!g_Git.IsInitRepos() && (m_dwContextMenus & GITSLC_POPGNUDIFF))
				{
					popup.AppendMenuIcon(IDGITLC_GNUDIFF1, IDS_LOG_POPUP_GNUDIFF, IDI_DIFF);
					bEntryAdded = true;
				}

				if ((m_dwContextMenus & this->GetContextMenuBit(IDGITLC_COMPAREWC)) && m_bHasWC)
				{
					if ((!m_CurrentVersion.IsEmpty()) && m_CurrentVersion != GIT_REV_ZERO)
					{
						popup.AppendMenuIcon(IDGITLC_COMPAREWC, IDS_LOG_POPUP_COMPARE, IDI_DIFF);
						bEntryAdded = true;
					}
				}

				if (bEntryAdded)
					popup.AppendMenu(MF_SEPARATOR);
			}

			if (!m_Rev1.IsEmpty() && !m_Rev2.IsEmpty())
			{
				if(GetSelectedCount() == 1)
				{
					if (m_dwContextMenus & this->GetContextMenuBit(IDGITLC_COMPARETWOREVISIONS))
					{
						popup.AppendMenuIcon(IDGITLC_COMPARETWOREVISIONS, IDS_LOG_POPUP_COMPARETWO, IDI_DIFF);
						popup.SetDefaultItem(IDGITLC_COMPARETWOREVISIONS, FALSE);
					}
					if (m_dwContextMenus & this->GetContextMenuBit(IDGITLC_GNUDIFF2REVISIONS))
					{
						popup.AppendMenuIcon(IDGITLC_GNUDIFF2REVISIONS, IDS_LOG_POPUP_GNUDIFF, IDI_DIFF);
					}
				}
			}

			//Select Multi item
			if (GetSelectedCount() > 0)
			{
				if ((GetSelectedCount() == 2) && (m_dwContextMenus & GITSLC_POPCOMPARETWOFILES) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO))
				{
					POSITION pos = GetFirstSelectedItemPosition();
					int index = GetNextSelectedItem(pos);
					if (index >= 0)
					{
						CTGitPath * entry2 = NULL;
						bool bothItemsAreExistingFiles = true;
						entry2 = (CTGitPath * )GetItemData(index);
						if (entry2)
							bothItemsAreExistingFiles = !entry2->IsDirectory() && entry2->Exists();
						index = GetNextSelectedItem(pos);
						if (index >= 0)
						{
							entry2 = (CTGitPath * )GetItemData(index);
							if (entry2)
								bothItemsAreExistingFiles = bothItemsAreExistingFiles && !entry2->IsDirectory() && entry2->Exists();
							if (bothItemsAreExistingFiles)
								popup.AppendMenuIcon(IDGITLC_COMPARETWOFILES, IDS_STATUSLIST_CONTEXT_COMPARETWOFILES, IDI_DIFF);
						}
					}
				}
			}

			if ( (GetSelectedCount() >0 ) && (!(wcStatus & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE))) && m_bHasWC)
			{
				if ((m_dwContextMenus & GITSLC_POPCOMMIT) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO) && !(wcStatus & (CTGitPath::LOGACTIONS_SKIPWORKTREE | CTGitPath::LOGACTIONS_ASSUMEVALID)))
				{
					popup.AppendMenuIcon(IDGITLC_COMMIT, IDS_STATUSLIST_CONTEXT_COMMIT, IDI_COMMIT);
				}

				if ((m_dwContextMenus & GITSLC_POPREVERT) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO))
				{
					popup.AppendMenuIcon(IDGITLC_REVERT, IDS_MENUREVERT, IDI_REVERT);
				}

				if ((m_dwContextMenus & GITSLC_POPSKIPWORKTREE) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO) && !(wcStatus & (CTGitPath::LOGACTIONS_ADDED | CTGitPath::LOGACTIONS_UNMERGED | CTGitPath::LOGACTIONS_SKIPWORKTREE)) && !filepath->IsDirectory())
				{
					popup.AppendMenuIcon(IDGITLC_SKIPWORKTREE, IDS_STATUSLIST_SKIPWORKTREE);
				}

				if ((m_dwContextMenus & GITSLC_POPASSUMEVALID) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO) && !(wcStatus & (CTGitPath::LOGACTIONS_ADDED | CTGitPath::LOGACTIONS_DELETED | CTGitPath::LOGACTIONS_UNMERGED | CTGitPath::LOGACTIONS_ASSUMEVALID)) && !filepath->IsDirectory())
				{
					popup.AppendMenuIcon(IDGITLC_ASSUMEVALID, IDS_MENUASSUMEVALID);
				}

				if ((m_dwContextMenus & GITLC_POPUNSETIGNORELOCALCHANGES) && (this->m_CurrentVersion.IsEmpty() || this->m_CurrentVersion == GIT_REV_ZERO) && (wcStatus & (CTGitPath::LOGACTIONS_SKIPWORKTREE | CTGitPath::LOGACTIONS_ASSUMEVALID)) && !filepath->IsDirectory())
				{
					popup.AppendMenuIcon(IDGITLC_UNSETIGNORELOCALCHANGES, IDS_STATUSLIST_UNSETIGNORELOCALCHANGES);
				}

				if (m_dwContextMenus & GITSLC_POPRESTORE && !filepath->IsDirectory())
				{
					if (m_restorepaths.find(filepath->GetWinPathString()) == m_restorepaths.end())
						popup.AppendMenuIcon(IDGITLC_CREATERESTORE, IDS_MENUCREATERESTORE, IDI_RESTORE);
					else
						popup.AppendMenuIcon(IDGITLC_RESTOREPATH, IDS_MENURESTORE, IDI_RESTORE);
				}

				if ((m_dwContextMenus & GetContextMenuBit(IDGITLC_REVERTTOREV)) && ( !this->m_CurrentVersion.IsEmpty() )
					&& this->m_CurrentVersion != GIT_REV_ZERO && !(wcStatus & CTGitPath::LOGACTIONS_DELETED))
				{
					popup.AppendMenuIcon(IDGITLC_REVERTTOREV, IDS_LOG_POPUP_REVERTTOREV, IDI_REVERT);
				}

				if ((m_dwContextMenus & GetContextMenuBit(IDGITLC_REVERTTOPARENT)) && ( !this->m_CurrentVersion.IsEmpty() )
					&& this->m_CurrentVersion != GIT_REV_ZERO && !(wcStatus & CTGitPath::LOGACTIONS_ADDED))
				{
					popup.AppendMenuIcon(IDGITLC_REVERTTOPARENT, IDS_LOG_POPUP_REVERTTOPARENT, IDI_REVERT);
				}
			}

			if ((GetSelectedCount() == 1)&&(!(wcStatus & CTGitPath::LOGACTIONS_UNVER))
				&&(!(wcStatus & CTGitPath::LOGACTIONS_IGNORE)))
			{
				if (m_dwContextMenus & GITSLC_POPSHOWLOG)
				{
					popup.AppendMenuIcon(IDGITLC_LOG, IDS_REPOBROWSE_SHOWLOG, IDI_LOG);
				}
				if (m_dwContextMenus & GITSLC_POPSHOWLOGSUBMODULE && filepath->IsDirectory())
				{
					popup.AppendMenuIcon(IDGITLC_LOGSUBMODULE, IDS_LOG_SUBMODULE, IDI_LOG);
				}
				if (m_dwContextMenus & GITSLC_POPSHOWLOGOLDNAME && (wcStatus & (CTGitPath::LOGACTIONS_REPLACED|CTGitPath::LOGACTIONS_COPY) && !filepath->GetGitOldPathString().IsEmpty()))
				{
					popup.AppendMenuIcon(IDGITLC_LOGOLDNAME, IDS_STATUSLIST_SHOWLOGOLDNAME, IDI_LOG);
				}
				if (m_dwContextMenus & GITSLC_POPBLAME && ! filepath->IsDirectory() && !(wcStatus & CTGitPath::LOGACTIONS_DELETED) && m_bHasWC)
				{
					popup.AppendMenuIcon(IDGITLC_BLAME, IDS_MENUBLAME, IDI_BLAME);
				}
			}

			if (GetSelectedCount() > 0)
			{
				if ((m_dwContextMenus & GetContextMenuBit(IDGITLC_EXPORT)) && !(wcStatus & CTGitPath::LOGACTIONS_DELETED))
					popup.AppendMenuIcon(IDGITLC_EXPORT, IDS_LOG_POPUP_EXPORT, IDI_EXPORT);
			}

			if ( (GetSelectedCount() == 1) )
			{
				if (m_dwContextMenus & this->GetContextMenuBit(IDGITLC_SAVEAS) && ! filepath->IsDirectory() && !(wcStatus & CTGitPath::LOGACTIONS_DELETED))
				{
					popup.AppendMenuIcon(IDGITLC_SAVEAS, IDS_LOG_POPUP_SAVE, IDI_SAVEAS);
				}

				if (m_dwContextMenus & GITSLC_POPOPEN && ! filepath->IsDirectory() && !(wcStatus & CTGitPath::LOGACTIONS_DELETED))
				{
					popup.AppendMenuIcon(IDGITLC_VIEWREV, IDS_LOG_POPUP_VIEWREV);
					popup.AppendMenuIcon(IDGITLC_OPEN, IDS_REPOBROWSE_OPEN, IDI_OPEN);
					popup.AppendMenuIcon(IDGITLC_OPENWITH, IDS_LOG_POPUP_OPENWITH, IDI_OPEN);
					if (wcStatus & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE)) {
						popup.SetDefaultItem(IDGITLC_OPEN, FALSE);
					}
				}

				if (m_dwContextMenus & GITSLC_POPEXPLORE && !(wcStatus & CTGitPath::LOGACTIONS_DELETED) && m_bHasWC)
				{
					popup.AppendMenuIcon(IDGITLC_EXPLORE, IDS_STATUSLIST_CONTEXT_EXPLORE, IDI_EXPLORER);
				}

			}

			if (GetSelectedCount() > 0)
			{
				if ((wcStatus & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE))/*||(wcStatus == git_wc_status_deleted)*/)
				{
					if (m_dwContextMenus & GITSLC_POPDELETE)
					{
						popup.AppendMenuIcon(IDGITLC_DELETE, IDS_MENUREMOVE, IDI_DELETE);
					}
				}
				if ( (wcStatus & CTGitPath::LOGACTIONS_UNVER || wcStatus & CTGitPath::LOGACTIONS_DELETED) )
				{
					if (m_dwContextMenus & GITSLC_POPIGNORE)
					{

						CTGitPathList ignorelist;
						FillListOfSelectedItemPaths(ignorelist);
						//check if all selected entries have the same extension
						bool bSameExt = true;
						CString sExt;
						for (int i=0; i<ignorelist.GetCount(); ++i)
						{
							if (sExt.IsEmpty() && (i==0))
								sExt = ignorelist[i].GetFileExtension();
							else if (sExt.CompareNoCase(ignorelist[i].GetFileExtension())!=0)
								bSameExt = false;
						}
						if (bSameExt)
						{
							if (ignoreSubMenu.CreateMenu())
							{
								CString ignorepath;
								if (ignorelist.GetCount()==1)
									ignorepath = ignorelist[0].GetFileOrDirectoryName();
								else
									ignorepath.Format(IDS_MENUIGNOREMULTIPLE, ignorelist.GetCount());
								ignoreSubMenu.AppendMenu(MF_STRING | MF_ENABLED, IDGITLC_IGNORE, ignorepath);
								ignorepath = _T("*")+sExt;
								ignoreSubMenu.AppendMenu(MF_STRING | MF_ENABLED, IDGITLC_IGNOREMASK, ignorepath);
								if (ignorelist.GetCount() == 1 && !ignorelist[0].GetContainingDirectory().GetGitPathString().IsEmpty())
									ignoreSubMenu.AppendMenu(MF_STRING | MF_ENABLED, IDGITLC_IGNOREFOLDER, ignorelist[0].GetContainingDirectory().GetGitPathString());
								CString temp;
								temp.LoadString(IDS_MENUIGNORE);
								popup.InsertMenu((UINT)-1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)ignoreSubMenu.m_hMenu, temp);
							}
						}
						else
						{
							CString temp;
							if (ignorelist.GetCount()==1)
							{
								temp.LoadString(IDS_MENUIGNORE);
							}
							else
							{
								temp.Format(IDS_MENUIGNOREMULTIPLE, ignorelist.GetCount());
							}
							popup.AppendMenuIcon(IDGITLC_IGNORE, temp, IDI_IGNORE);
							temp.Format(IDS_MENUIGNOREMULTIPLEMASK, ignorelist.GetCount());
							popup.AppendMenuIcon(IDGITLC_IGNOREMASK, temp, IDI_IGNORE);
						}
					}
				}
			}



			if (GetSelectedCount() > 0)
			{
				popup.AppendMenu(MF_SEPARATOR);
				popup.AppendMenuIcon(IDGITLC_COPY, IDS_STATUSLIST_CONTEXT_COPY, IDI_COPYCLIP);
				popup.AppendMenuIcon(IDGITLC_COPYEXT, IDS_STATUSLIST_CONTEXT_COPYEXT, IDI_COPYCLIP);
			}

			int cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this, 0);

			m_bBlock = TRUE;
			AfxGetApp()->DoWaitCursor(1);
			//int iItemCountBeforeMenuCmd = GetItemCount();
			//bool bForce = false;
			switch (cmd)
			{
			case IDGITLC_VIEWREV:
				OpenFile(filepath, ALTERNATIVEEDITOR);
				break;

			case IDGITLC_OPEN:
				OpenFile(filepath,OPEN);
				break;

			case IDGITLC_OPENWITH:
				OpenFile(filepath,OPEN_WITH);
				break;

			case IDGITLC_EXPLORE:
				CAppUtils::ExploreTo(GetSafeHwnd(), g_Git.CombinePath(filepath));
				break;

			case IDGITLC_CREATERESTORE:
				{
					POSITION pos = GetFirstSelectedItemPosition();
					while (pos)
					{
						int index = GetNextSelectedItem(pos);
						CTGitPath * entry2 = (CTGitPath * )GetItemData(index);
						ASSERT(entry2 != NULL);
						if (entry2 == NULL || entry2->IsDirectory())
							continue;
						if (m_restorepaths.find(entry2->GetWinPathString()) != m_restorepaths.end())
							continue;
						CTGitPath tempFile = CTempFiles::Instance().GetTempFilePath(false);
						// delete the temp file: the temp file has the FILE_ATTRIBUTE_TEMPORARY flag set
						// and copying the real file over it would leave that temp flag.
						DeleteFile(tempFile.GetWinPath());
						if (CopyFile(g_Git.CombinePath(entry2), tempFile.GetWinPath(), FALSE))
						{
							m_restorepaths[entry2->GetWinPathString()] = tempFile.GetWinPathString();
							SetItemState(index, INDEXTOOVERLAYMASK(OVL_RESTORE), LVIS_OVERLAYMASK);
						}
					}
					Invalidate();
				}
				break;

			case IDGITLC_RESTOREPATH:
				{
					if (CMessageBox::Show(m_hWnd, IDS_STATUSLIST_RESTOREPATH, IDS_APPNAME, 2, IDI_QUESTION, IDS_RESTOREBUTTON, IDS_ABORTBUTTON) == 2)
						break;
					POSITION pos = GetFirstSelectedItemPosition();
					while (pos)
					{
						int index = GetNextSelectedItem(pos);
						CTGitPath * entry2 = (CTGitPath * )GetItemData(index);
						ASSERT(entry2 != NULL);
						if (entry2 == NULL)
							continue;
						if (m_restorepaths.find(entry2->GetWinPathString()) == m_restorepaths.end())
							continue;
						if (CopyFile(m_restorepaths[entry2->GetWinPathString()], g_Git.CombinePath(entry2), FALSE))
						{
							m_restorepaths.erase(entry2->GetWinPathString());
							SetItemState(index, 0, LVIS_OVERLAYMASK);
						}
					}
					Invalidate();
				}
				break;

			// Compare current version and work copy.
			case IDGITLC_COMPAREWC:
				{
					if (!CheckMultipleDiffs())
						break;
					POSITION pos = GetFirstSelectedItemPosition();
					while ( pos )
					{
						int index = GetNextSelectedItem(pos);
						StartDiffWC(index);
					}
				}
				break;

			// Compare with base version. when current version is zero, compare workcopy and HEAD.
			case IDGITLC_COMPARE:
				{
					if (!CheckMultipleDiffs())
						break;
					POSITION pos = GetFirstSelectedItemPosition();
					while ( pos )
					{
						int index = GetNextSelectedItem(pos);
						StartDiff(index);
					}
				}
				break;

			case IDGITLC_COMPARETWOREVISIONS:
				{
					POSITION pos = GetFirstSelectedItemPosition();
					while ( pos )
					{
						int index = GetNextSelectedItem(pos);
						StartDiffTwo(index);
					}
				}
				break;

			case IDGITLC_COMPARETWOFILES:
				{
					POSITION pos = GetFirstSelectedItemPosition();
					CTGitPath * firstfilepath = NULL, * secondfilepath = NULL;
					if (pos)
					{
						firstfilepath = (CTGitPath * )GetItemData(GetNextSelectedItem(pos));
						ASSERT(firstfilepath != NULL);
						if (firstfilepath == NULL)
							break;

						secondfilepath = (CTGitPath * )GetItemData(GetNextSelectedItem(pos));
						ASSERT(secondfilepath != NULL);
						if (secondfilepath == NULL)
							break;

						CString sCmd;
						sCmd.Format(_T("/command:diff /path:\"%s\" /path2:\"%s\" /hwnd:%p"), firstfilepath->GetWinPath(), secondfilepath->GetWinPath(), (void*)m_hWnd);
						CAppUtils::RunTortoiseGitProc(sCmd);
					}
				}
				break;

			case IDGITLC_GNUDIFF1:
				{
					POSITION pos = GetFirstSelectedItemPosition();
					while (pos)
					{
						CTGitPath * selectedFilepath = (CTGitPath * )GetItemData(GetNextSelectedItem(pos));
						if (m_CurrentVersion.IsEmpty() || m_CurrentVersion == GIT_REV_ZERO)
						{
							CString fromwhere;
							if (m_amend)
								fromwhere = _T("~1");
							CAppUtils::StartShowUnifiedDiff(m_hWnd, *selectedFilepath, GitRev::GetHead() + fromwhere, *selectedFilepath, GitRev::GetWorkingCopy());
						}
						else
						{
							if ((selectedFilepath->m_ParentNo & (PARENT_MASK | MERGE_MASK)) == 0)
								CAppUtils::StartShowUnifiedDiff(m_hWnd, *selectedFilepath, m_CurrentVersion + _T("~1"), *selectedFilepath, m_CurrentVersion);
							else
							{
								CString str;
								if (!(selectedFilepath->m_ParentNo & MERGE_MASK))
									str.Format(_T("%s^%d"), m_CurrentVersion, (selectedFilepath->m_ParentNo & PARENT_MASK) + 1);

								CAppUtils::StartShowUnifiedDiff(m_hWnd, *selectedFilepath, str, *selectedFilepath, m_CurrentVersion, false, false, false, false, !!(selectedFilepath->m_ParentNo & MERGE_MASK));
							}
						}
					}
				}
				break;

			case IDGITLC_GNUDIFF2REVISIONS:
				{
					CAppUtils::StartShowUnifiedDiff(m_hWnd, *filepath, m_Rev2, *filepath, m_Rev1);
				}
				break;

			case IDGITLC_ADD:
				{
					CTGitPathList paths;
					FillListOfSelectedItemPaths(paths, true);

					CGitProgressDlg progDlg;
					AddProgressCommand addCommand;
					progDlg.SetCommand(&addCommand);
					addCommand.SetPathList(paths);
					progDlg.SetItemCount(paths.GetCount());
					progDlg.DoModal();

					// reset unchecked status
					POSITION pos = GetFirstSelectedItemPosition();
					int index;
					while ((index = GetNextSelectedItem(pos)) >= 0)
					{
						m_mapFilenameToChecked.erase(((CTGitPath*)GetItemData(index))->GetGitPathString());
					}

					if (NULL != GetLogicalParent() && NULL != GetLogicalParent()->GetSafeHwnd())
						GetLogicalParent()->SendMessage(GITSLNM_NEEDSREFRESH);

					SetRedraw(TRUE);
				}
				break;

			case IDGITLC_DELETE:
				DeleteSelectedFiles();
				break;

			case IDGITLC_BLAME:
				{
					CAppUtils::LaunchTortoiseBlame(g_Git.CombinePath(filepath), m_CurrentVersion);
				}
				break;

			case IDGITLC_LOG:
			case IDGITLC_LOGSUBMODULE:
				{
					CString sCmd;
					sCmd.Format(_T("/command:log /path:\"%s\""), g_Git.CombinePath(filepath));
					if (cmd == IDGITLC_LOG && filepath->IsDirectory())
						sCmd += _T(" /submodule");
					if (!m_sDisplayedBranch.IsEmpty())
						sCmd += _T(" /range:\"") + m_sDisplayedBranch + _T("\"");
					CAppUtils::RunTortoiseGitProc(sCmd, false, !(cmd == IDGITLC_LOGSUBMODULE));
				}
				break;

			case IDGITLC_LOGOLDNAME:
				{
					CTGitPath oldName(filepath->GetGitOldPathString());
					CString sCmd;
					sCmd.Format(_T("/command:log /path:\"%s\""), g_Git.CombinePath(oldName));
					if (!m_sDisplayedBranch.IsEmpty())
						sCmd += _T(" /range:\"") + m_sDisplayedBranch + _T("\"");
					CAppUtils::RunTortoiseGitProc(sCmd);
				}
				break;

			case IDGITLC_EDITCONFLICT:
				{
					if (CAppUtils::ConflictEdit(*filepath, false, m_bIsRevertTheirMy, GetLogicalParent() ? GetLogicalParent()->GetSafeHwnd() : nullptr))
					{
						CString conflictedFile = g_Git.CombinePath(filepath);
						if (!PathFileExists(conflictedFile) && NULL != GetLogicalParent() && NULL != GetLogicalParent()->GetSafeHwnd())
							GetLogicalParent()->SendMessage(GITSLNM_NEEDSREFRESH);
					}
				}
				break;

			case IDGITLC_RESOLVETHEIRS: //follow up
			case IDGITLC_RESOLVEMINE:   //follow up
			case IDGITLC_RESOLVECONFLICT:
				{
					if (CMessageBox::Show(m_hWnd, IDS_PROC_RESOLVE, IDS_APPNAME, MB_ICONQUESTION | MB_YESNO)==IDYES)
					{
						POSITION pos = GetFirstSelectedItemPosition();
						while (pos != 0)
						{
							int index;
							index = GetNextSelectedItem(pos);
							CTGitPath * fentry =(CTGitPath*) this->GetItemData(index);
							if(fentry == NULL)
								continue;

							CAppUtils::resolve_with resolveWith = CAppUtils::RESOLVE_WITH_CURRENT;
							if (((!this->m_bIsRevertTheirMy) && cmd == IDGITLC_RESOLVETHEIRS) || ((this->m_bIsRevertTheirMy) && cmd == IDGITLC_RESOLVEMINE))
								resolveWith = CAppUtils::RESOLVE_WITH_THEIRS;
							else if (((!this->m_bIsRevertTheirMy) && cmd == IDGITLC_RESOLVEMINE) || ((this->m_bIsRevertTheirMy) && cmd == IDGITLC_RESOLVETHEIRS))
								resolveWith = CAppUtils::RESOLVE_WITH_MINE;
							if (CAppUtils::ResolveConflict(*fentry, resolveWith) == 0 && fentry->m_Action & CTGitPath::LOGACTIONS_UNMERGED && CRegDWORD(_T("Software\\TortoiseGit\\RefreshFileListAfterResolvingConflict"), TRUE) == TRUE)
							{
								CWnd* pParent = GetLogicalParent();
								if (pParent && pParent->GetSafeHwnd())
									pParent->SendMessage(GITSLNM_NEEDSREFRESH);
								SetRedraw(TRUE);
								break;
							}
						}
						Show(m_dwShow, 0, m_bShowFolders,0,true);
					}
				}
				break;

			case IDGITLC_IGNORE:
				{
					CTGitPathList ignorelist;
					//std::vector<CString> toremove;
					FillListOfSelectedItemPaths(ignorelist, true);

					if(!CAppUtils::IgnoreFile(ignorelist,false))
						break;

					SetRedraw(FALSE);
					CWnd* pParent = GetLogicalParent();
					if (NULL != pParent && NULL != pParent->GetSafeHwnd())
					{
						pParent->SendMessage(GITSLNM_NEEDSREFRESH);
					}
					SetRedraw(TRUE);
				}
				break;

			case IDGITLC_IGNOREMASK:
				{
					CString common;
					CString ext=filepath->GetFileExtension();
					CTGitPathList ignorelist;
					FillListOfSelectedItemPaths(ignorelist, true);

					if (!CAppUtils::IgnoreFile(ignorelist,true))
						break;

					SetRedraw(FALSE);
					CWnd* pParent = GetLogicalParent();
					if (NULL != pParent && NULL != pParent->GetSafeHwnd())
					{
						pParent->SendMessage(GITSLNM_NEEDSREFRESH);
					}

					SetRedraw(TRUE);
				}
				break;

			case IDGITLC_IGNOREFOLDER:
				{
					CTGitPathList ignorelist;
					ignorelist.AddPath(filepath->GetContainingDirectory());

					if (!CAppUtils::IgnoreFile(ignorelist, false))
						break;

					SetRedraw(FALSE);
					CWnd *pParent = GetLogicalParent();
					if (NULL != pParent && NULL != pParent->GetSafeHwnd())
						pParent->SendMessage(GITSLNM_NEEDSREFRESH);

					SetRedraw(TRUE);
				}
				break;
			case IDGITLC_COMMIT:
				{
					CTGitPathList targetList;
					FillListOfSelectedItemPaths(targetList);
					CTGitPath tempFile = CTempFiles::Instance().GetTempFilePath(false);
					VERIFY(targetList.WriteToFile(tempFile.GetWinPathString()));
					CString commandline = _T("/command:commit /pathfile:\"");
					commandline += tempFile.GetWinPathString();
					commandline += _T("\"");
					commandline += _T(" /deletepathfile");
					CAppUtils::RunTortoiseGitProc(commandline);
				}
				break;
			case IDGITLC_REVERT:
				{
					// If at least one item is not in the status "added"
					// we ask for a confirmation
					BOOL bConfirm = FALSE;
					POSITION pos = GetFirstSelectedItemPosition();
					int index;
					while ((index = GetNextSelectedItem(pos)) >= 0)
					{
						//FileEntry * fentry = GetListEntry(index);
						CTGitPath *fentry=(CTGitPath*)GetItemData(index);
						if(fentry && fentry->m_Action &CTGitPath::LOGACTIONS_MODIFIED && !fentry->IsDirectory())
						{
							bConfirm = TRUE;
							break;
						}
					}

					CString str;
					str.Format(IDS_PROC_WARNREVERT,GetSelectedCount());

					if (!bConfirm || CMessageBox::Show(this->m_hWnd, str, _T("TortoiseGit"), MB_YESNO | MB_ICONQUESTION)==IDYES)
					{
						CTGitPathList targetList;
						FillListOfSelectedItemPaths(targetList);

						// make sure that the list is reverse sorted, so that
						// children are removed before any parents
						targetList.SortByPathname(true);

						// put all reverted files in the trashbin, except the ones with 'added'
						// status because they are not restored by the revert.
						CTGitPathList delList;
						POSITION pos = GetFirstSelectedItemPosition();
						int index;
						while ((index = GetNextSelectedItem(pos)) >= 0)
						{
							CTGitPath *entry=(CTGitPath *)GetItemData(index);
							if (entry&&(!(entry->m_Action& CTGitPath::LOGACTIONS_ADDED))
									&& (!(entry->m_Action& CTGitPath::LOGACTIONS_REPLACED)) && !entry->IsDirectory())
							{
								CTGitPath fullpath;
								fullpath.SetFromWin(g_Git.CombinePath(entry));
								delList.AddPath(fullpath);
							}
						}
						if (DWORD(CRegDWORD(_T("Software\\TortoiseGit\\RevertWithRecycleBin"), TRUE)))
							delList.DeleteAllFiles(true);

						CString revertToCommit = _T("HEAD");
						if (m_amend)
							revertToCommit = _T("HEAD~1");
						CString err;
						if (g_Git.Revert(revertToCommit, targetList, err))
						{
							CMessageBox::Show(this->m_hWnd, _T("Revert failed:\n") + err, _T("TortoiseGit"), MB_ICONERROR);
						}
						else
						{
							bool updateStatusList = false;
							for (int i = 0 ; i < targetList.GetCount(); ++i)
							{
								int nListboxEntries = GetItemCount();
								for (int nItem=0; nItem<nListboxEntries; ++nItem)
								{
									CTGitPath *path=(CTGitPath*)GetItemData(nItem);
									if (path->GetGitPathString()==targetList[i].GetGitPathString() && !path->IsDirectory())
									{
										if(path->m_Action & CTGitPath::LOGACTIONS_ADDED)
										{
											path->m_Action = CTGitPath::LOGACTIONS_UNVER;
											SetEntryCheck(path,nItem,false);
											updateStatusList = true;
										}
										else
										{
											if (GetCheck(nItem))
												m_nSelected--;
											RemoveListEntry(nItem);
										}
										break;
									}
									else if (path->GetGitPathString()==targetList[i].GetGitPathString() && path->IsDirectory() && path->IsWCRoot())
									{
										CString sCmd;
										sCmd.Format(_T("/command:revert /path:\"%s\""), path->GetGitPathString());
										CCommonAppUtils::RunTortoiseGitProc(sCmd);
									}
								}
							}
							SetRedraw(TRUE);
							SaveColumnWidths();
							if (updateStatusList && nullptr != GetLogicalParent() && nullptr != GetLogicalParent()->GetSafeHwnd())
								GetLogicalParent()->SendMessage(GITSLNM_NEEDSREFRESH);
						}
					}
				}
				break;

			case IDGITLC_ASSUMEVALID:
				SetGitIndexFlagsForSelectedFiles(IDS_PROC_MARK_ASSUMEVALID, BST_CHECKED, BST_INDETERMINATE);
				break;
			case IDGITLC_SKIPWORKTREE:
				SetGitIndexFlagsForSelectedFiles(IDS_PROC_MARK_SKIPWORKTREE, BST_INDETERMINATE, BST_CHECKED);
				break;
			case IDGITLC_UNSETIGNORELOCALCHANGES:
				SetGitIndexFlagsForSelectedFiles(IDS_PROC_UNSET_IGNORELOCALCHANGES, BST_UNCHECKED, BST_UNCHECKED);
				break;
			case IDGITLC_COPY:
				CopySelectedEntriesToClipboard(0);
				break;
			case IDGITLC_COPYEXT:
				CopySelectedEntriesToClipboard((DWORD)-1);
				break;
			case IDGITLC_EXPORT:
				FilesExport();
				break;
			case IDGITLC_SAVEAS:
				FileSaveAs(filepath);
				break;

			case IDGITLC_REVERTTOREV:
				RevertSelectedItemToVersion();
				break;
			case IDGITLC_REVERTTOPARENT:
				RevertSelectedItemToVersion(true);
				break;
			} // switch (cmd)
			m_bBlock = FALSE;
			AfxGetApp()->DoWaitCursor(-1);
			GetStatisticsString();
			//int iItemCountAfterMenuCmd = GetItemCount();
			//if (iItemCountAfterMenuCmd != iItemCountBeforeMenuCmd)
			//{
			//	CWnd* pParent = GetParent();
			//	if (NULL != pParent && NULL != pParent->GetSafeHwnd())
			//	{
			//		pParent->SendMessage(SVNSLNM_ITEMCOUNTCHANGED);
			//	}
			//}
		} // if (popup.CreatePopupMenu())
	} // if (selIndex >= 0)

}


void CWorkingChangesFileListCtrl::SetGitIndexFlagsForSelectedFiles(UINT message, BOOL assumevalid, BOOL skipworktree)
{
	if (CMessageBox::Show(GetSafeHwnd(), message, IDS_APPNAME, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) != IDYES)
		return;

	CAutoRepository repository(g_Git.GetGitRepository());
	if (!repository)
	{
		CMessageBox::Show(m_hWnd, g_Git.GetLibGit2LastErr(), _T("TortoiseGit"), MB_ICONERROR);
		return;
	}

	CAutoIndex gitindex;
	if (git_repository_index(gitindex.GetPointer(), repository))
	{
		CMessageBox::Show(m_hWnd, g_Git.GetLibGit2LastErr(), _T("TortoiseGit"), MB_ICONERROR);
		return;
	}

	POSITION pos = GetFirstSelectedItemPosition();
	int index = -1;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		CTGitPath * path = (CTGitPath *)GetItemData(index);
		ASSERT(path);
		if (path == nullptr)
			continue;

		size_t idx;
		if (!git_index_find(&idx, gitindex, CUnicodeUtils::GetMulti(path->GetGitPathString(), CP_UTF8)))
		{
			git_index_entry *e = const_cast<git_index_entry *>(git_index_get_byindex(gitindex, idx)); // HACK
			if (assumevalid == BST_UNCHECKED)
				e->flags &= ~GIT_IDXENTRY_VALID;
			else if (assumevalid == BST_CHECKED)
				e->flags |= GIT_IDXENTRY_VALID;
			if (skipworktree == BST_UNCHECKED)
				e->flags_extended &= ~GIT_IDXENTRY_SKIP_WORKTREE;
			else if (skipworktree == BST_CHECKED)
				e->flags_extended |= GIT_IDXENTRY_SKIP_WORKTREE;
			git_index_add(gitindex, e);
		}
		else
			CMessageBox::Show(m_hWnd, g_Git.GetLibGit2LastErr(), _T("TortoiseGit"), MB_ICONERROR);
	}

	if (git_index_write(gitindex))
	{
		CMessageBox::Show(m_hWnd, g_Git.GetLibGit2LastErr(), _T("TortoiseGit"), MB_ICONERROR);
		return;
	}

	if (nullptr != GetLogicalParent() && nullptr != GetLogicalParent()->GetSafeHwnd())
		GetLogicalParent()->SendMessage(GITSLNM_NEEDSREFRESH);

	SetRedraw(TRUE);
}


void CWorkingChangesFileListCtrl::OnContextMenuHeader(CWnd * pWnd, CPoint point)
{
	Locker lock(m_critSec);
	m_ColumnManager.OnContextMenuHeader(pWnd,point,!!IsGroupViewEnabled());
}


void CWorkingChangesFileListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{

	if (pWnd == this)
	{
		OnContextMenuList(pWnd, point);
	} // if (pWnd == this)
	else if (pWnd == GetHeaderCtrl())
	{
		OnContextMenuHeader(pWnd, point);
	}
}


void CWorkingChangesFileListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{

	Locker lock(m_critSec);
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
	if (m_bBlock)
		return;

	if (pNMLV->iItem < 0)
		return;

	CTGitPath *file=(CTGitPath*)GetItemData(pNMLV->iItem);

	if (file->m_Action & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE)) {
		OpenFile(file, OPEN);
		return;
	}
	if( file->m_Action&CTGitPath::LOGACTIONS_UNMERGED )
	{
		if (CAppUtils::ConflictEdit(*file, false, m_bIsRevertTheirMy, GetLogicalParent() ? GetLogicalParent()->GetSafeHwnd() : nullptr))
		{
			CString conflictedFile = g_Git.CombinePath(file);
			if (!PathFileExists(conflictedFile) && NULL != GetLogicalParent() && NULL != GetLogicalParent()->GetSafeHwnd())
				GetLogicalParent()->SendMessage(GITSLNM_NEEDSREFRESH);
		}
	}
	else
	{
		if (!m_Rev1.IsEmpty() && !m_Rev2.IsEmpty())
			StartDiffTwo(pNMLV->iItem);
		else
			StartDiff(pNMLV->iItem);
	}

}


void CWorkingChangesFileListCtrl::StartDiffTwo(int fileindex)
{
	if(fileindex<0)
		return;

	auto ptr = (CTGitPath*)GetItemData(fileindex);
	if (ptr == nullptr)
		return;
	CTGitPath file1 = *ptr;

	if (file1.m_Action & CTGitPath::LOGACTIONS_ADDED)
		CGitDiff::DiffNull(&file1, m_Rev1, true);
	else if (file1.m_Action & CTGitPath::LOGACTIONS_DELETED)
		CGitDiff::DiffNull(&file1, m_Rev2, false);
	else
		CGitDiff::Diff(&file1, &file1, m_Rev1, m_Rev2);

}


void CWorkingChangesFileListCtrl::StartDiffWC(int fileindex)
{
	if(fileindex<0)
		return;

	CString Ver;
	if(this->m_CurrentVersion.IsEmpty() || m_CurrentVersion== GIT_REV_ZERO)
		return;

	auto ptr = (CTGitPath*)GetItemData(fileindex);
	if (ptr == nullptr)
		return;
	CTGitPath file1 = *ptr;
	file1.m_Action = 0; // reset action, so that diff is not started as added/deleted file; see issue #1757

	CGitDiff::Diff(&file1,&file1, GIT_REV_ZERO, m_CurrentVersion);

}


void CWorkingChangesFileListCtrl::StartDiff(int fileindex)
{
	if(fileindex<0)
		return;

	auto ptr = (CTGitPath*)GetItemData(fileindex);
	if (ptr == nullptr)
		return;
	CTGitPath file1 = *ptr;
	CTGitPath file2;
	if(file1.m_Action & (CTGitPath::LOGACTIONS_REPLACED|CTGitPath::LOGACTIONS_COPY))
	{
		file2.SetFromGit(file1.GetGitOldPathString());
	}
	else
	{
		file2=file1;
	}

	if(this->m_CurrentVersion.IsEmpty() || m_CurrentVersion== GIT_REV_ZERO)
	{
		CString fromwhere;
		if(m_amend && (file1.m_Action & CTGitPath::LOGACTIONS_ADDED) == 0)
			fromwhere = _T("~1");
		if( g_Git.IsInitRepos())
			CGitDiff::DiffNull((CTGitPath*)GetItemData(fileindex),
					GIT_REV_ZERO);
		else if( file1.m_Action&CTGitPath::LOGACTIONS_ADDED )
			CGitDiff::DiffNull((CTGitPath*)GetItemData(fileindex),
					m_CurrentVersion+fromwhere,true);
		else if( file1.m_Action&CTGitPath::LOGACTIONS_DELETED )
			CGitDiff::DiffNull((CTGitPath*)GetItemData(fileindex),
					GitRev::GetHead()+fromwhere,false);
		else
			CGitDiff::Diff(&file1,&file2,
					CString(GIT_REV_ZERO),
					GitRev::GetHead()+fromwhere);
	}
	else
	{
		CGitHash hash;
		CString fromwhere = m_CurrentVersion+_T("~1");
		if(m_amend)
			fromwhere = m_CurrentVersion+_T("~2");
		bool revfail = !!g_Git.GetHash(hash, fromwhere);
		if (revfail || (file1.m_Action & file1.LOGACTIONS_ADDED))
		{
			CGitDiff::DiffNull(&file1,m_CurrentVersion,true);

		}
		else if (file1.m_Action & file1.LOGACTIONS_DELETED)
		{
			if (file1.m_ParentNo > 0)
				fromwhere.Format(_T("%s^%d"), m_CurrentVersion, file1.m_ParentNo + 1);

			CGitDiff::DiffNull(&file1,fromwhere,false);
		}
		else
		{
			if( file1.m_ParentNo & MERGE_MASK)
			{

				CTGitPath base, theirs, mine, merge;

				CString temppath;
				GetTempPath(temppath);
				temppath.TrimRight(_T("\\"));

				mine.SetFromGit(temppath + _T("\\") + file1.GetFileOrDirectoryName() + _T(".LOCAL") + file1.GetFileExtension());
				theirs.SetFromGit(temppath + _T("\\") + file1.GetFileOrDirectoryName() + _T(".REMOTE") + file1.GetFileExtension());
				base.SetFromGit(temppath + _T("\\") + file1.GetFileOrDirectoryName() + _T(".BASE") + file1.GetFileExtension());

				CFile tempfile;
				//create a empty file, incase stage is not three
				tempfile.Open(mine.GetWinPathString(),CFile::modeCreate|CFile::modeReadWrite);
				tempfile.Close();
				tempfile.Open(theirs.GetWinPathString(),CFile::modeCreate|CFile::modeReadWrite);
				tempfile.Close();
				tempfile.Open(base.GetWinPathString(),CFile::modeCreate|CFile::modeReadWrite);
				tempfile.Close();

				merge.SetFromGit(temppath + _T("\\") + file1.GetFileOrDirectoryName() + _T(".Merged") + file1.GetFileExtension());

				int parent1=-1, parent2 =-1;
				for (size_t i = 0; i < m_arStatusArray.size(); ++i)
				{
					if(m_arStatusArray[i]->GetGitPathString() == file1.GetGitPathString())
					{
						if(m_arStatusArray[i]->m_ParentNo & MERGE_MASK)
						{
						}
						else
						{
							if(parent1<0)
							{
								parent1 = m_arStatusArray[i]->m_ParentNo & PARENT_MASK;
							}
							else if (parent2 <0)
							{
								parent2 = m_arStatusArray[i]->m_ParentNo & PARENT_MASK;
							}
						}
					}
				}

				if(g_Git.GetOneFile(m_CurrentVersion, file1, (CString&)merge.GetWinPathString()))
				{
					CMessageBox::Show(NULL, IDS_STATUSLIST_FAILEDGETMERGEFILE, IDS_APPNAME, MB_OK | MB_ICONERROR);
				}

				if(parent1>=0)
				{
					CString str;
					str.Format(_T("%s^%d"),this->m_CurrentVersion, parent1+1);

					if(g_Git.GetOneFile(str, file1, (CString&)mine.GetWinPathString()))
					{
						CMessageBox::Show(NULL, IDS_STATUSLIST_FAILEDGETMERGEFILE, IDS_APPNAME, MB_OK | MB_ICONERROR);
					}
				}

				if(parent2>=0)
				{
					CString str;
					str.Format(_T("%s^%d"),this->m_CurrentVersion, parent2+1);

					if(g_Git.GetOneFile(str, file1, (CString&)theirs.GetWinPathString()))
					{
						CMessageBox::Show(NULL, IDS_STATUSLIST_FAILEDGETMERGEFILE, IDS_APPNAME, MB_OK | MB_ICONERROR);
					}
				}

				if(parent1>=0 && parent2>=0)
				{
					CString cmd, output;
					cmd.Format(_T("git.exe merge-base %s^%d %s^%d"), this->m_CurrentVersion, parent1+1,
						this->m_CurrentVersion,parent2+1);

					if (g_Git.Run(cmd, &output, NULL, CP_UTF8))
					{
					}
					else
					{
						if(g_Git.GetOneFile(output.Left(40), file1, (CString&)base.GetWinPathString()))
						{
							CMessageBox::Show(NULL, IDS_STATUSLIST_FAILEDGETBASEFILE, IDS_APPNAME, MB_OK | MB_ICONERROR);
						}
					}
				}
				CAppUtils::StartExtMerge(base, theirs, mine, merge,_T("BASE"),_T("REMOTE"),_T("LOCAL"));

			}
			else
			{
				CString str;
				if( (file1.m_ParentNo&PARENT_MASK) == 0)
				{
					str = _T("~1");
				}
				else
				{
					str.Format(_T("^%d"), (file1.m_ParentNo&PARENT_MASK)+1);
				}
				CGitDiff::Diff(&file1,&file2,
					m_CurrentVersion,
					m_CurrentVersion+str);
			}
		}
	}
}


CString CWorkingChangesFileListCtrl::GetStatisticsString(bool simple)
{
	CString sNormal = CString(MAKEINTRESOURCE(IDS_STATUSNORMAL));
	CString sAdded = CString(MAKEINTRESOURCE(IDS_STATUSADDED));
	CString sDeleted = CString(MAKEINTRESOURCE(IDS_STATUSDELETED));
	CString sModified = CString(MAKEINTRESOURCE(IDS_STATUSMODIFIED));
	CString sConflicted = CString(MAKEINTRESOURCE(IDS_STATUSCONFLICTED));
	CString sUnversioned = CString(MAKEINTRESOURCE(IDS_STATUSUNVERSIONED));
	CString sRenamed = CString(MAKEINTRESOURCE(IDS_STATUSREPLACED));
	CString sToolTip;
	if(simple)
	{
		sToolTip.Format(IDS_STATUSLIST_STATUSLINE1,
			this->m_nLineAdded,this->m_nLineDeleted,
				(LPCTSTR)sModified, m_nModified,
				(LPCTSTR)sAdded, m_nAdded,
				(LPCTSTR)sDeleted, m_nDeleted,
				(LPCTSTR)sRenamed, m_nRenamed
			);
	}
	else
	{
		sToolTip.Format(IDS_STATUSLIST_STATUSLINE2,
			this->m_nLineAdded,this->m_nLineDeleted,
				(LPCTSTR)sNormal, m_nNormal,
				(LPCTSTR)sUnversioned, m_nUnversioned,
				(LPCTSTR)sModified, m_nModified,
				(LPCTSTR)sAdded, m_nAdded,
				(LPCTSTR)sDeleted, m_nDeleted,
				(LPCTSTR)sConflicted, m_nConflicted
			);
	}
	CString sStats;
	sStats.Format(IDS_COMMITDLG_STATISTICSFORMAT, m_nSelected, GetItemCount());
	if (m_pStatLabel)
	{
		m_pStatLabel->SetWindowText(sStats);
	}

	if (m_pSelectButton)
	{
		if (m_nSelected == 0)
			m_pSelectButton->SetCheck(BST_UNCHECKED);
		else if (m_nSelected != GetItemCount())
			m_pSelectButton->SetCheck(BST_INDETERMINATE);
		else
			m_pSelectButton->SetCheck(BST_CHECKED);
	}

	if (m_pConfirmButton)
	{
		m_pConfirmButton->EnableWindow(m_nSelected>0);
	}

	return sToolTip;


}


CString CWorkingChangesFileListCtrl::GetCommonDirectory(bool bStrict)
{
	if (!bStrict)
	{
		// not strict means that the selected folder has priority
		if (!m_StatusFileList.GetCommonDirectory().IsEmpty())
			return m_StatusFileList.GetCommonDirectory().GetWinPath();
	}

	CTGitPath commonBaseDirectory;
	int nListItems = GetItemCount();
	for (int i=0; i<nListItems; ++i)
	{
		CTGitPath baseDirectory,*p= (CTGitPath*)this->GetItemData(i);
		ASSERT(p);
		if(p==NULL)
			continue;
		baseDirectory = p->GetDirectory();

		if(commonBaseDirectory.IsEmpty())
		{
			commonBaseDirectory = baseDirectory;
		}
		else
		{
			if (commonBaseDirectory.GetWinPathString().GetLength() > baseDirectory.GetWinPathString().GetLength())
			{
				if (baseDirectory.IsAncestorOf(commonBaseDirectory))
					commonBaseDirectory = baseDirectory;
			}
		}
	}
	return g_Git.CombinePath(commonBaseDirectory);
}


void CWorkingChangesFileListCtrl::SelectAll(bool bSelect, bool /*bIncludeNoCommits*/)
{
	CWaitCursor waitCursor;
	// block here so the LVN_ITEMCHANGED messages
	// get ignored
	m_bBlock = TRUE;
	SetRedraw(FALSE);

	int nListItems = GetItemCount();
	if (bSelect)
		m_nSelected = nListItems;
	else
		m_nSelected = 0;

	for (int i=0; i<nListItems; ++i)
	{
		//FileEntry * entry = GetListEntry(i);
		//ASSERT(entry != NULL);
		CTGitPath *path = (CTGitPath *) GetItemData(i);
		if (path == NULL)
			continue;
		//if ((bIncludeNoCommits)||(entry->GetChangeList().Compare(SVNSLC_IGNORECHANGELIST)))
		SetEntryCheck(path,i,bSelect);
	}

	// unblock before redrawing
	m_bBlock = FALSE;
	SetRedraw(TRUE);
	GetStatisticsString();
	NotifyCheck();
}


void CWorkingChangesFileListCtrl::Check(DWORD dwCheck, bool check)
{
	CWaitCursor waitCursor;
	// block here so the LVN_ITEMCHANGED messages
	// get ignored
	m_bBlock = TRUE;
	SetRedraw(FALSE);

	int nListItems = GetItemCount();

	for (int i = 0; i < nListItems; ++i)
	{
		CTGitPath *entry = (CTGitPath *) GetItemData(i);
		if (entry == NULL)
			continue;

		DWORD showFlags = entry->m_Action;
		if (entry->IsDirectory())
			showFlags |= GITSLC_SHOWSUBMODULES;
		else
			showFlags |= GITSLC_SHOWFILES;

		if (check && (showFlags & dwCheck) && !GetCheck(i) && !(entry->IsDirectory() && m_bDoNotAutoselectSubmodules && !(dwCheck & GITSLC_SHOWSUBMODULES)))
		{
			SetEntryCheck(entry, i, true);
			m_nSelected++;
		}
		else if (!check && (showFlags & dwCheck) && GetCheck(i))
		{
			SetEntryCheck(entry, i, false);
			m_nSelected--;
		}
	}
	// unblock before redrawing
	m_bBlock = FALSE;
	SetRedraw(TRUE);
	GetStatisticsString();
	NotifyCheck();
}


void CWorkingChangesFileListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	*pResult = 0;
	if (m_bBlock)
		return;

	CTGitPath *entry=(CTGitPath *)GetItemData(pGetInfoTip->iItem);

	if (entry)
		if (pGetInfoTip->cchTextMax > entry->GetGitPathString().GetLength() + g_Git.m_CurrentDir.GetLength())
		{
			CString str = g_Git.CombinePath(entry->GetWinPathString());
			_tcsncpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, str.GetBuffer(), str.GetLength());
		}
}


void CWorkingChangesFileListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

	// Take the default processing unless we set this to something else below.
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.

	switch (pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		{
			// This is the prepaint stage for an item. Here's where we set the
			// item's text color. Our return value will tell Windows to draw the
			// item itself, but it will use the new color we set here.

			// Tell Windows to paint the control itself.
			*pResult = CDRF_DODEFAULT;
			if (m_bBlock)
				return;

			COLORREF crText = GetSysColor(COLOR_WINDOWTEXT);

			if (m_arStatusArray.size() > (DWORD_PTR)pLVCD->nmcd.dwItemSpec)
			{

				//FileEntry * entry = GetListEntry((int)pLVCD->nmcd.dwItemSpec);
				CTGitPath *entry=(CTGitPath *)GetItemData((int)pLVCD->nmcd.dwItemSpec);
				if (entry == NULL)
					return;

				// coloring
				// ========
				// black  : unversioned, normal
				// purple : added
				// blue   : modified
				// brown  : missing, deleted, replaced
				// green  : merged (or potential merges)
				// red    : conflicts or sure conflicts
				if(entry->m_Action & CTGitPath::LOGACTIONS_GRAY)
				{
					crText = RGB(128,128,128);

				}
				else if(entry->m_Action & CTGitPath::LOGACTIONS_UNMERGED)
				{
					crText = m_Colors.GetColor(CColors::Conflict);

				}
				else if(entry->m_Action & (CTGitPath::LOGACTIONS_MODIFIED))
				{
					crText = m_Colors.GetColor(CColors::Modified);

				}
				else if(entry->m_Action & (CTGitPath::LOGACTIONS_ADDED|CTGitPath::LOGACTIONS_COPY))
				{
					crText = m_Colors.GetColor(CColors::Added);
				}
				else if(entry->m_Action & CTGitPath::LOGACTIONS_DELETED)
				{
					crText = m_Colors.GetColor(CColors::Deleted);
				}
				else if(entry->m_Action & CTGitPath::LOGACTIONS_REPLACED)
				{
					crText = m_Colors.GetColor(CColors::Renamed);
				}
				else if(entry->m_Action & CTGitPath::LOGACTIONS_MERGED)
				{
					crText = m_Colors.GetColor(CColors::Merged);
				}
				else
				{
					crText = GetSysColor(COLOR_WINDOWTEXT);
				}
				// Store the color back in the NMLVCUSTOMDRAW struct.
				pLVCD->clrText = crText;
			}
		}
		break;
	}
}


BOOL CWorkingChangesFileListCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (pWnd != this)
		return CListCtrl::OnSetCursor(pWnd, nHitTest, message);
	if (!m_bBlock)
	{
		HCURSOR hCur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		SetCursor(hCur);
		return CListCtrl::OnSetCursor(pWnd, nHitTest, message);
	}
	HCURSOR hCur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	SetCursor(hCur);
	return TRUE;
}


void CWorkingChangesFileListCtrl::RemoveListEntry(int index)
{

	Locker lock(m_critSec);
	DeleteItem(index);

	m_arStatusArray.erase(m_arStatusArray.begin()+index);
}


///< Set a checkbox on an entry in the listbox
// NEVER, EVER call SetCheck directly, because you'll end-up with the checkboxes and the 'checked' flag getting out of sync
void CWorkingChangesFileListCtrl::SetEntryCheck(CTGitPath* pEntry, int listboxIndex, bool bCheck)
{
	pEntry->m_Checked = bCheck;
	m_mapFilenameToChecked[pEntry->GetGitPathString()] = bCheck;
	SetCheck(listboxIndex, bCheck);
}


void CWorkingChangesFileListCtrl::WriteCheckedNamesToPathList(CTGitPathList& pathList)
{

	pathList.Clear();
	int nListItems = GetItemCount();
	for (int i = 0; i< nListItems; ++i)
	{
		CTGitPath * entry = (CTGitPath*)GetItemData(i);
		ASSERT(entry != NULL);
		if (entry->m_Checked)
		{
			pathList.AddPath(*entry);
		}
	}
	pathList.SortByPathname();

}


/// Build a path list of all the selected items in the list (NOTE - SELECTED, not CHECKED)
void CWorkingChangesFileListCtrl::FillListOfSelectedItemPaths(CTGitPathList& pathList, bool /*bNoIgnored*/)
{
	pathList.Clear();

	POSITION pos = GetFirstSelectedItemPosition();
	int index;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		CTGitPath * entry = (CTGitPath*)GetItemData(index);
		//if ((bNoIgnored)&&(entry->status == git_wc_status_ignored))
		//	continue;
		pathList.AddPath(*entry);
	}
}


UINT CWorkingChangesFileListCtrl::OnGetDlgCode()
{
	// we want to process the return key and not have that one
	// routed to the default pushbutton
	return CListCtrl::OnGetDlgCode() | DLGC_WANTALLKEYS;
}


void CWorkingChangesFileListCtrl::OnNMReturn(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;
	if (m_bBlock)
		return;
	if (!CheckMultipleDiffs())
		return;
	POSITION pos = GetFirstSelectedItemPosition();
	while ( pos )
	{
		int index = GetNextSelectedItem(pos);
		if (index < 0)
			return;
		CTGitPath *file=(CTGitPath*)GetItemData(index);
		if (file == nullptr)
			return;
		if (file->m_Action & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE))
		{
			OpenFile(file, OPEN);
		}
		else
		{
			if (!m_Rev1.IsEmpty() && !m_Rev2.IsEmpty())
				StartDiffTwo(index);
			else
				StartDiff(index);
		}
	}
}


void CWorkingChangesFileListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Since we catch all keystrokes (to have the enter key processed here instead
	// of routed to the default pushbutton) we have to make sure that other
	// keys like Tab and Esc still do what they're supposed to do
	// Tab = change focus to next/previous control
	// Esc = quit the dialog
	switch (nChar)
	{
	case (VK_TAB):
		{
			::PostMessage(GetLogicalParent()->GetSafeHwnd(), WM_NEXTDLGCTL, GetKeyState(VK_SHIFT)&0x8000, 0);
			return;
		}
		break;
	case (VK_ESCAPE):
		{
			::SendMessage(GetLogicalParent()->GetSafeHwnd(), WM_CLOSE, 0, 0);
		}
		break;
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CWorkingChangesFileListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();
	EnableToolTips(TRUE);
	SetWindowTheme(GetSafeHwnd(), L"Explorer", NULL);
}


void CWorkingChangesFileListCtrl::OnPaint()
{
	LRESULT defres = Default();
	if ((m_bBusy)||(m_bEmpty))
	{
		CString str;
		if (m_bBusy)
		{
			if (m_sBusy.IsEmpty())
				str.LoadString(IDS_STATUSLIST_BUSYMSG);
			else
				str = m_sBusy;
		}
		else
		{
			if (m_sEmpty.IsEmpty())
				str.LoadString(IDS_STATUSLIST_EMPTYMSG);
			else
				str = m_sEmpty;
		}
		COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrTextBk;
		if (IsWindowEnabled())
			clrTextBk = ::GetSysColor(COLOR_WINDOW);
		else
			clrTextBk = ::GetSysColor(COLOR_3DFACE);

		CRect rc;
		GetClientRect(&rc);
		CHeaderCtrl* pHC;
		pHC = GetHeaderCtrl();
		if (pHC != NULL)
		{
			CRect rcH;
			pHC->GetItemRect(0, &rcH);
			rc.top += rcH.bottom;
		}
		CDC* pDC = GetDC();
		{
			CMyMemDC memDC(pDC, &rc);

			memDC.SetTextColor(clrText);
			memDC.SetBkColor(clrTextBk);
			memDC.FillSolidRect(rc, clrTextBk);
			rc.top += 10;
			CGdiObject * oldfont = memDC.SelectStockObject(DEFAULT_GUI_FONT);
			memDC.DrawText(str, rc, DT_CENTER | DT_VCENTER |
				DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
			memDC.SelectObject(oldfont);
		}
		ReleaseDC(pDC);
	}
	if (defres)
	{
		// the Default() call did not process the WM_PAINT message!
		// Validate the update region ourselves to avoid
		// an endless loop repainting
		CRect rc;
		GetUpdateRect(&rc, FALSE);
		if (!rc.IsRectEmpty())
			ValidateRect(rc);
	}
}


// prevent users from extending our hidden (size 0) columns
void CWorkingChangesFileListCtrl::OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_ColumnManager.OnHdnBegintrack(pNMHDR, pResult);
}


// prevent any function from extending our hidden (size 0) columns
void CWorkingChangesFileListCtrl::OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(!m_ColumnManager.OnHdnItemchanging(pNMHDR, pResult))
		Default();
}


void CWorkingChangesFileListCtrl::OnDestroy()
{
	SaveColumnWidths(true);
	CListCtrl::OnDestroy();
}


void CWorkingChangesFileListCtrl::OnBeginDrag(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	Locker lock(m_critSec);
	CDropFiles dropFiles; // class for creating DROPFILES struct

	int index;
	POSITION pos = GetFirstSelectedItemPosition();
	if (!pos)
		return;

	bool bTempDirCreated = false;
	CString tempDir;
	while ( (index = GetNextSelectedItem(pos)) >= 0 )
	{
		CTGitPath *path = (CTGitPath *)GetItemData(index); // m_arStatusArray[index] does not work with SyncDlg
		if (path->IsDirectory())
			continue;

		CString version;
		if (!this->m_CurrentVersion.IsEmpty() && this->m_CurrentVersion != GIT_REV_ZERO)
		{
			if (path->m_Action & CTGitPath::LOGACTIONS_DELETED)
				version.Format(_T("%s^%d"), m_CurrentVersion, (path->m_ParentNo + 1) & PARENT_MASK);
			else
				version = m_CurrentVersion;
		}
		else
		{
			if (path->m_Action & CTGitPath::LOGACTIONS_DELETED)
				version = _T("HEAD");
		}

		CString tempFile;
		if (version.IsEmpty())
		{
			TCHAR abspath[MAX_PATH] = {0};
			PathCombine(abspath, g_Git.m_CurrentDir, path->GetWinPath());
			tempFile = abspath;
		}
		else
		{
			if (!bTempDirCreated)
			{
				tempDir = GetTempFile();
				::DeleteFile(tempDir);
				::CreateDirectory(tempDir, NULL);
				bTempDirCreated = true;
			}
			tempFile = tempDir + _T("\\") + path->GetWinPathString();
			CString tempSubDir = tempDir + _T("\\") + path->GetContainingDirectory().GetWinPathString();
			CPathUtils::MakeSureDirectoryPathExists(tempSubDir);
			if (g_Git.GetOneFile(version, *path, tempFile))
			{
				CString out;
				out.Format(IDS_STATUSLIST_CHECKOUTFILEFAILED, path->GetGitPathString(), version, tempFile);
				CMessageBox::Show(nullptr, g_Git.GetGitLastErr(out, CGit::GIT_CMD_GETONEFILE), _T("TortoiseGit"), MB_OK);
				return;
			}
		}
		dropFiles.AddFile(tempFile);
	}

	if (!dropFiles.IsEmpty())
	{
		m_bOwnDrag = true;
		dropFiles.CreateStructure();
		m_bOwnDrag = false;
	}
	*pResult = 0;
}


void CWorkingChangesFileListCtrl::SaveColumnWidths(bool bSaveToRegistry /* = false */)
{
	int maxcol = ((CHeaderCtrl*)(GetDlgItem(0)))->GetItemCount()-1;
	for (int col = 0; col <= maxcol; col++)
		if (m_ColumnManager.IsVisible (col))
			m_ColumnManager.ColumnResized (col);

	if (bSaveToRegistry)
		m_ColumnManager.WriteSettings();
}


bool CWorkingChangesFileListCtrl::EnableFileDrop()
{
	m_bFileDropsEnabled = true;
	return true;
}


bool CWorkingChangesFileListCtrl::HasPath(const CTGitPath& path)
{
	for (size_t i=0; i < m_arStatusArray.size(); ++i)
	{
		if (m_arStatusArray[i]->IsEquivalentTo(path))
			return true;
	}

	return false;
}


BOOL CWorkingChangesFileListCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case 'A':
			{
				if (GetAsyncKeyState(VK_CONTROL)&0x8000)
				{
					// select all entries
					for (int i=0; i<GetItemCount(); ++i)
					{
						SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
					}
					return TRUE;
				}
			}
			break;
		case 'C':
		case VK_INSERT:
			{
				if (GetAsyncKeyState(VK_CONTROL)&0x8000)
				{
					// copy all selected paths to the clipboard
					if (GetAsyncKeyState(VK_SHIFT)&0x8000)
						CopySelectedEntriesToClipboard(GITSLC_COLSTATUS);
					else
						CopySelectedEntriesToClipboard(0);
					return TRUE;
				}
			}
			break;
		case VK_DELETE:
			{
				if ((GetSelectedCount() > 0) && (m_dwContextMenus & GITSLC_POPDELETE))
				{
					m_bBlock = TRUE;
					CTGitPath * filepath = (CTGitPath *)GetItemData(GetSelectionMark());
					if (filepath != nullptr && (filepath->m_Action & (CTGitPath::LOGACTIONS_UNVER | CTGitPath::LOGACTIONS_IGNORE)))
						DeleteSelectedFiles();
					m_bBlock = FALSE;
				}
			}
			break;
		}
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}


bool CWorkingChangesFileListCtrl::CopySelectedEntriesToClipboard(DWORD dwCols)
{

	static HINSTANCE hResourceHandle(AfxGetResourceHandle());

	CString sClipboard;
	CString temp;
	//TCHAR buf[100];
	if (GetSelectedCount() == 0)
		return false;

	DWORD selection = 0;
	for (int i = 0, count = m_ColumnManager.GetColumnCount(); i < count; ++i)
		if (   ((dwCols == -1) && m_ColumnManager.IsVisible (i))
			|| ((dwCols != 1) && (i < 32) && ((dwCols & (1 << i)) != 0)))
		{
			sClipboard += _T("\t") + m_ColumnManager.GetName(i);

			if (i < 32)
				selection += 1 << i;
		}

	if(dwCols)
		sClipboard += _T("\r\n");

	POSITION pos = GetFirstSelectedItemPosition();
	int index;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		CTGitPath * entry = (CTGitPath*)GetItemData(index);
		if(entry == NULL)
			continue;

		sClipboard += entry->GetWinPathString();
		if (selection & GITSLC_COLFILENAME)
		{
			sClipboard += _T("\t")+entry->GetFileOrDirectoryName();
		}
		if (selection & GITSLC_COLEXT)
		{
			sClipboard += _T("\t")+entry->GetFileExtension();
		}

		if (selection & GITSLC_COLSTATUS)
		{
			sClipboard += _T("\t")+entry->GetActionName();
		}

		if (selection & GITSLC_COLADD)
		{
			sClipboard += _T("\t")+entry->m_StatAdd;
		}
		if (selection & GITSLC_COLDEL)
		{
			sClipboard += _T("\t")+entry->m_StatDel;
		}

		sClipboard += _T("\r\n");
	}

	return CStringUtils::WriteAsciiStringToClipboard(sClipboard);
}


size_t CWorkingChangesFileListCtrl::GetNumberOfChangelistsInSelection()
{
	return 0;
}


bool CWorkingChangesFileListCtrl::PrepareGroups(bool bForce /* = false */)
{
	bool bEnableGroupView = false;

	// Enable group view if we have any change packages or working file changes
	if (!m_changePackages.empty() || !m_workingFileChanges.empty() || bForce ) {
		bEnableGroupView = true;
	}

	RemoveAllGroups();
	EnableGroupView( bEnableGroupView );

	if( bEnableGroupView )
	{

		LVGROUP grp = {0};
		grp.cbSize = sizeof(LVGROUP);
		grp.mask = LVGF_ALIGN | LVGF_GROUPID | LVGF_HEADER;
		
		int groupIndex = 0;

		// Add groups for each change package
		for( std::shared_ptr<IntegrityActions::ChangePackage> *cp : m_changePackages ) {
			std::wstring cpid = (*cp)->getId();
			grp.pszHeader = cpid.c_str();
			grp.cchHeader = cpid.size();
			grp.iGroupId = groupIndex;
			grp.uAlign = LVGA_HEADER_LEFT;
			InsertGroup( groupIndex++, &grp );
		}

		// Add group for working file changes
		TCHAR groupname[1024] = { 0 };
		_tcsncpy_s(groupname, 1024, (LPCTSTR)MAKEINTRESOURCE(IDS_WFC_GROUP_NOCP), 1023);
		grp.pszHeader = groupname;
		grp.cchHeader = 1024;
		grp.iGroupId = groupIndex;
		grp.uAlign = LVGA_HEADER_LEFT;
		InsertGroup( groupIndex++, &grp );
	}

	return bEnableGroupView;
}


void CWorkingChangesFileListCtrl::NotifyCheck()
{
	CWnd* pParent = GetLogicalParent();
	if (NULL != pParent && NULL != pParent->GetSafeHwnd())
	{
		pParent->SendMessage(GITSLNM_CHECKCHANGED, m_nSelected);
	}
}


int CWorkingChangesFileListCtrl::UpdateFileList(git_revnum_t hash,CTGitPathList *list)
{
	CString cmdList;
	BYTE_VECTOR out;
	this->m_bBusy=TRUE;
	m_CurrentVersion=hash;

	int count = 0;
	if(list == NULL)
		count = 1;
	else
		count = list->GetCount();

	CString head = _T("HEAD");
	if(m_amend)
		head = _T("HEAD~1");

	if(hash == GIT_REV_ZERO)
	{
		for (int i = 0; i < count; ++i)
		{
			BYTE_VECTOR cmdout;
			cmdout.clear();
			CString cmd;
			if(!g_Git.IsInitRepos())
			{
				if (CGit::ms_bCygwinGit)
				{
					// Prevent showing all files as modified when using cygwin's git
					if (list == NULL)
						cmd = (_T("git.exe status --"));
					else
						cmd.Format(_T("git.exe status -- \"%s\""), (*list)[i].GetGitPathString());
					cmdList += cmd + _T("\n");
					g_Git.Run(cmd, &cmdout);
					cmdout.clear();
				}

				// also list staged files which will be in the commit
				cmd=(_T("git.exe diff-index --cached --raw ") + head + _T(" --numstat -C -M -z --"));
				cmdList += cmd + _T("\n");
				g_Git.Run(cmd, &cmdout);

				if(list == NULL)
					cmd=(_T("git.exe diff-index --raw ") + head + _T("  --numstat -C -M -z --"));
				else
					cmd.Format(_T("git.exe diff-index --raw ") + head + _T("  --numstat -C -M -z -- \"%s\""),(*list)[i].GetGitPathString());
				cmdList += cmd + _T("\n");

				BYTE_VECTOR cmdErr;
				if(g_Git.Run(cmd, &cmdout, &cmdErr))
				{
					int last = cmdErr.RevertFind(0,-1);
					CString str;
					g_Git.StringAppend(&str, &cmdErr[last + 1], CP_UTF8, (int)cmdErr.size() - last -1);
					CMessageBox::Show(NULL, str, _T("TortoiseGit"), MB_OK|MB_ICONERROR);
				}

				out.append(cmdout, 0);
			}
			else // Init Repository
			{
				//We will list all added file for init repository because commit will comit these
				//if(list == NULL)
				cmd=_T("git.exe ls-files -s -t -z");
				cmdList += cmd + _T("\n");
				//else
				//	cmd.Format(_T("git.exe ls-files -s -t -z -- \"%s\""),(*list)[i].GetGitPathString());

				g_Git.Run(cmd, &cmdout);
				//out+=cmdout;
				out.append(cmdout,0);

				break;
			}
		}

		if(g_Git.IsInitRepos())
		{
			if (m_StatusFileList.ParserFromLsFile(out))
			{
				CString tempFile1 = GetTempFile();
				CFile file1(tempFile1, CFile::modeWrite | CFile::typeBinary);
				file1.Write(out.data(), (UINT)out.size());
				file1.Close();
				CString tempFile2 = GetTempFile();
				CFile file2(tempFile2, CFile::modeWrite);
				file2.Write(cmdList, sizeof(TCHAR) * cmdList.GetLength());
				file2.Close();
				CMessageBox::Show(NULL, _T("Parse ls-files failed!\nPlease inspect ") + tempFile1 + _T("\nand ") + tempFile2, _T("TortoiseGit"), MB_OK);
			}
			for (int i = 0; i < m_StatusFileList.GetCount(); ++i)
				((CTGitPath&)(m_StatusFileList[i])).m_Action=CTGitPath::LOGACTIONS_ADDED;
		}
		else
			this->m_StatusFileList.ParserFromLog(out);

		//handle delete conflict case, when remote : modified, local : deleted.
		for (int i = 0; i < count; ++i)
		{
			BYTE_VECTOR cmdout;
			CString cmd;

			if(list == NULL)
				cmd=_T("git.exe ls-files -u -t -z");
			else
				cmd.Format(_T("git.exe ls-files -u -t -z -- \"%s\""),(*list)[i].GetGitPathString());

			g_Git.Run(cmd, &cmdout);

			CTGitPathList conflictlist;
			conflictlist.ParserFromLog(cmdout);
			for (int i = 0; i < conflictlist.GetCount(); ++i)
			{
				CTGitPath *p=m_StatusFileList.LookForGitPath(conflictlist[i].GetGitPathString());
				if(p)
					p->m_Action|=CTGitPath::LOGACTIONS_UNMERGED;
				else
					m_StatusFileList.AddPath(conflictlist[i]);
			}
		}

		// handle source files of file renames/moves (issue #860)
		// if a file gets renamed and the new file "git add"ed, diff-index doesn't list the source file anymore
		for (int i = 0; i < count; ++i)
		{
			BYTE_VECTOR cmdout;
			CString cmd;

			if(list == NULL)
				cmd = _T("git.exe ls-files -d -z");
			else
				cmd.Format(_T("git.exe ls-files -d -z -- \"%s\""),(*list)[i].GetGitPathString());

			g_Git.Run(cmd, &cmdout);

			CTGitPathList deletelist;
			deletelist.ParserFromLog(cmdout, true);
			BOOL bDeleteChecked = FALSE;
			int deleteFromIndex = 0;
			for (int i = 0; i < deletelist.GetCount(); ++i)
			{
				CTGitPath *p = m_StatusFileList.LookForGitPath(deletelist[i].GetGitPathString());
				if(!p)
					m_StatusFileList.AddPath(deletelist[i]);
				else if ((p->m_Action == CTGitPath::LOGACTIONS_ADDED || p->m_Action == CTGitPath::LOGACTIONS_REPLACED) && !p->Exists())
				{
					if (!bDeleteChecked)
					{
						CString message;
						message.Format(IDS_ASK_REMOVE_FROM_INDEX, p->GetWinPathString());
						deleteFromIndex = CMessageBox::ShowCheck(m_hWnd, message, _T("TortoiseGit"), 1, IDI_EXCLAMATION, CString(MAKEINTRESOURCE(IDS_REMOVE_FROM_INDEX)), CString(MAKEINTRESOURCE(IDS_IGNOREBUTTON)), NULL, NULL, CString(MAKEINTRESOURCE(IDS_DO_SAME_FOR_REST)), &bDeleteChecked);
					}
					if (deleteFromIndex == 1)
					{
						g_Git.Run(_T("git.exe rm -f --cache -- \"") + p->GetWinPathString() + _T("\""), &cmdout);
						m_StatusFileList.RemoveItem(*p);
					}
				}
			}
		}
	}
	else
	{
		int count = 0;
		if(list == NULL)
			count = 1;
		else
			count = list->GetCount();

		for (int i = 0; i < count; ++i)
		{
			BYTE_VECTOR cmdout;
			CString cmd;
			if(list == NULL)
				cmd.Format(_T("git.exe diff-tree --raw --numstat -C -M -z %s --"), hash);
			else
				cmd.Format(_T("git.exe diff-tree --raw  --numstat -C -M %s -z -- \"%s\""),hash,(*list)[i].GetGitPathString());

			g_Git.Run(cmd, &cmdout, NULL);

			out.append(cmdout);
		}
		this->m_StatusFileList.ParserFromLog(out);

	}

	for (int i = 0; i < m_StatusFileList.GetCount(); ++i)
	{
		CTGitPath * gitpatch=(CTGitPath*)&m_StatusFileList[i];
		gitpatch->m_Checked = TRUE;
		m_arStatusArray.push_back((CTGitPath*)&m_StatusFileList[i]);
	}

	this->m_bBusy=FALSE;
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateWithGitPathList(CTGitPathList &list)
{
	m_arStatusArray.clear();
	for (int i = 0; i < list.GetCount(); ++i)
	{
		CTGitPath * gitpath=(CTGitPath*)&list[i];

		if(gitpath ->m_Action & CTGitPath::LOGACTIONS_HIDE)
			continue;

		gitpath->m_Checked = TRUE;
		m_arStatusArray.push_back((CTGitPath*)&list[i]);
	}
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateUnRevFileList(CTGitPathList &list)
{
	m_UnRevFileList = list;
	for (int i = 0; i < m_UnRevFileList.GetCount(); ++i)
	{
		CTGitPath * gitpatch=(CTGitPath*)&m_UnRevFileList[i];
		gitpatch->m_Checked = FALSE;
		m_arStatusArray.push_back((CTGitPath*)&m_UnRevFileList[i]);
	}
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateUnRevFileList(CTGitPathList *List)
{
	CString err;
	if (m_UnRevFileList.FillUnRev(CTGitPath::LOGACTIONS_UNVER, List, &err))
	{
		CMessageBox::Show(NULL, _T("Failed to get UnRev file list\n") + err, _T("TortoiseGit"), MB_OK);
		return -1;
	}

	for (int i = 0; i < m_UnRevFileList.GetCount(); ++i)
	{
		CTGitPath * gitpatch=(CTGitPath*)&m_UnRevFileList[i];
		gitpatch->m_Checked = FALSE;
		m_arStatusArray.push_back((CTGitPath*)&m_UnRevFileList[i]);
	}
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateIgnoreFileList(CTGitPathList *List)
{
	CString err;
	if (m_IgnoreFileList.FillUnRev(CTGitPath::LOGACTIONS_IGNORE, List, &err))
	{
		CMessageBox::Show(NULL, _T("Failed to get Ignore file list\n") + err, _T("TortoiseGit"), MB_OK);
		return -1;
	}

	for (int i = 0; i < m_IgnoreFileList.GetCount(); ++i)
	{
		CTGitPath * gitpatch=(CTGitPath*)&m_IgnoreFileList[i];
		gitpatch->m_Checked = FALSE;
		m_arStatusArray.push_back((CTGitPath*)&m_IgnoreFileList[i]);
	}
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateLocalChangesIgnoredFileList(CTGitPathList *list)
{
	m_LocalChangesIgnoredFileList.FillBasedOnIndexFlags(GIT_IDXENTRY_VALID | GIT_IDXENTRY_SKIP_WORKTREE, list);
	for (int i = 0; i < m_LocalChangesIgnoredFileList.GetCount(); ++i)
	{
		CTGitPath * gitpatch = (CTGitPath*)&m_LocalChangesIgnoredFileList[i];
		gitpatch->m_Checked = FALSE;
		m_arStatusArray.push_back((CTGitPath*)&m_LocalChangesIgnoredFileList[i]);
	}
	return 0;
}


int CWorkingChangesFileListCtrl::UpdateFileList(int mask,bool once,CTGitPathList *List)
{
	if(mask&CWorkingChangesFileListCtrl::FILELIST_MODIFY)
	{
		if(once || (!(m_FileLoaded&CWorkingChangesFileListCtrl::FILELIST_MODIFY)))
		{
			UpdateFileList(git_revnum_t(GIT_REV_ZERO),List);
			m_FileLoaded|=CWorkingChangesFileListCtrl::FILELIST_MODIFY;
		}
	}
	if (mask & CWorkingChangesFileListCtrl::FILELIST_UNVER || mask & CWorkingChangesFileListCtrl::FILELIST_IGNORE)
	{
		if(once || (!(m_FileLoaded&CWorkingChangesFileListCtrl::FILELIST_UNVER)))
		{
			UpdateUnRevFileList(List);
			m_FileLoaded|=CWorkingChangesFileListCtrl::FILELIST_UNVER;
		}
		if(mask&CWorkingChangesFileListCtrl::FILELIST_IGNORE && (once || (!(m_FileLoaded&CWorkingChangesFileListCtrl::FILELIST_IGNORE))))
		{
			UpdateIgnoreFileList(List);
			m_FileLoaded |= CWorkingChangesFileListCtrl::FILELIST_IGNORE;
		}
	}
	if (mask & CWorkingChangesFileListCtrl::FILELIST_LOCALCHANGESIGNORED && (once || (!(m_FileLoaded & CWorkingChangesFileListCtrl::FILELIST_LOCALCHANGESIGNORED))))
	{
		UpdateLocalChangesIgnoredFileList(List);
		m_FileLoaded |= CWorkingChangesFileListCtrl::FILELIST_LOCALCHANGESIGNORED;
	}
	return 0;
}


void CWorkingChangesFileListCtrl::Clear()
{
	m_FileLoaded=0;
	this->DeleteAllItems();
	this->m_arListArray.clear();
	this->m_arStatusArray.clear();
	this->m_changelists.clear();
}


bool CWorkingChangesFileListCtrl::CheckMultipleDiffs()
{
	UINT selCount = GetSelectedCount();
	if (selCount > max(3, (DWORD)CRegDWORD(_T("Software\\TortoiseGit\\NumDiffWarning"), 10)))
	{
		CString message;
		message.Format(CString(MAKEINTRESOURCE(IDS_STATUSLIST_WARN_MAXDIFF)), selCount);
		return ::MessageBox(GetSafeHwnd(), message, _T("TortoiseGit"), MB_YESNO | MB_ICONQUESTION) == IDYES;
	}
	return true;
}


void CWorkingChangesFileListCtrl::FilesExport()
{
	CString exportDir;
	// export all changed files to a folder
	CBrowseFolder browseFolder;
	browseFolder.m_style = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	if (browseFolder.Show(GetSafeHwnd(), exportDir) != CBrowseFolder::OK)
		return;

	POSITION pos = GetFirstSelectedItemPosition();
	int index;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		CTGitPath *fd = (CTGitPath*)GetItemData(index);
		// we cannot export directories or folders
		if (fd->m_Action == CTGitPath::LOGACTIONS_DELETED || fd->IsDirectory())
			continue;

		CAppUtils::CreateMultipleDirectory(exportDir + _T("\\") + fd->GetContainingDirectory().GetWinPathString());
		CString filename = exportDir + _T("\\") + fd->GetWinPathString();
		if (m_CurrentVersion == GIT_REV_ZERO)
		{
			if (!CopyFile(g_Git.CombinePath(fd), filename, false))
			{
				MessageBox(CFormatMessageWrapper(), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				return;
			}
		}
		else
		{
			if (g_Git.GetOneFile(m_CurrentVersion, *fd, filename))
			{
				CString out;
				out.Format(IDS_STATUSLIST_CHECKOUTFILEFAILED, fd->GetGitPathString(), m_CurrentVersion, filename);
				if (CMessageBox::Show(nullptr, g_Git.GetGitLastErr(out, CGit::GIT_CMD_GETONEFILE), _T("TortoiseGit"), 2, IDI_WARNING, CString(MAKEINTRESOURCE(IDS_IGNOREBUTTON)), CString(MAKEINTRESOURCE(IDS_ABORTBUTTON))) == 2)
					return;
			}
		}
	}
}


void CWorkingChangesFileListCtrl::FileSaveAs(CTGitPath *path)
{
	CString filename;
	filename.Format(_T("%s-%s%s"), path->GetBaseFilename(), this->m_CurrentVersion.Left(g_Git.GetShortHASHLength()), path->GetFileExtension());
	CFileDialog dlg(FALSE,NULL,
					filename,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					NULL);
	CString currentpath;
	currentpath = g_Git.CombinePath(path->GetContainingDirectory());

	dlg.m_ofn.lpstrInitialDir=currentpath.GetBuffer();

	CString cmd,out;
	INT_PTR ret = dlg.DoModal();
	SetCurrentDirectory(g_Git.m_CurrentDir);
	if (ret == IDOK)
	{
		filename = dlg.GetPathName();
		if(m_CurrentVersion == GIT_REV_ZERO)
		{
			if(!CopyFile(g_Git.CombinePath(path), filename, false))
			{
				MessageBox(CFormatMessageWrapper(), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				return;
			}

		}
		else
		{
			if(g_Git.GetOneFile(m_CurrentVersion,*path,filename))
			{
				out.Format(IDS_STATUSLIST_CHECKOUTFILEFAILED, path->GetGitPathString(), m_CurrentVersion, filename);
				CMessageBox::Show(nullptr, g_Git.GetGitLastErr(out, CGit::GIT_CMD_GETONEFILE), _T("TortoiseGit"), MB_OK);
				return;
			}
		}
	}

}


int CWorkingChangesFileListCtrl::RevertSelectedItemToVersion(bool parent)
{
	if(this->m_CurrentVersion.IsEmpty())
		return 0;
	if(this->m_CurrentVersion == GIT_REV_ZERO)
		return 0;

	POSITION pos = GetFirstSelectedItemPosition();
	int index;
	CString cmd,out;
	std::map<CString, int> versionMap;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		CTGitPath *fentry=(CTGitPath*)GetItemData(index);
		CString version;
		if (parent)
		{
			int parentNo = fentry->m_ParentNo & PARENT_MASK;
			CString ref;
			ref.Format(_T("%s^%d"), m_CurrentVersion, parentNo + 1);
			CGitHash hash;
			if (g_Git.GetHash(hash, ref))
			{
				MessageBox(g_Git.GetGitLastErr(_T("Could not get hash of ref \"") + ref + _T("\".")), _T("TortoiseGit"), MB_ICONERROR);
				continue;
			}

			version = hash.ToString();
		}
		else
			version = m_CurrentVersion;

		CString filename = fentry->GetGitPathString();
		if (!fentry->GetGitOldPathString().IsEmpty())
			filename = fentry->GetGitOldPathString();
		cmd.Format(_T("git.exe checkout %s -- \"%s\""), version, filename);
		out.Empty();
		if (g_Git.Run(cmd, &out, CP_UTF8))
		{
			if (MessageBox(out, _T("TortoiseGit"), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
				continue;
		}
		else
			versionMap[version]++;
	}

	out.Empty();
	for (auto it = versionMap.begin(); it != versionMap.end(); ++it)
	{
		CString versionEntry;
		versionEntry.Format(IDS_STATUSLIST_FILESREVERTED, it->second, it->first);
		out += versionEntry + _T("\r\n");
	}
	if (!out.IsEmpty())
		CMessageBox::Show(nullptr, out, _T("TortoiseGit"), MB_OK);
	return 0;
}


void CWorkingChangesFileListCtrl::OpenFile(CTGitPath*filepath,int mode)
{
	CString file;
	if(this->m_CurrentVersion.IsEmpty() || m_CurrentVersion == GIT_REV_ZERO)
	{
		file = g_Git.CombinePath(filepath);
	}
	else
	{
		CString temppath;
		GetTempPath(temppath);
		TCHAR szTempName[MAX_PATH] = {0};
		GetTempFileName(temppath, filepath->GetBaseFilename(), 0, szTempName);
		CString temp(szTempName);
		DeleteFile(szTempName);
		CreateDirectory(szTempName, NULL);
		file.Format(_T("%s\\%s_%s%s"),
					temp,
					filepath->GetBaseFilename(),
					m_CurrentVersion.Left(g_Git.GetShortHASHLength()),
					filepath->GetFileExtension());
		CString cmd,out;
		if(g_Git.GetOneFile(m_CurrentVersion, *filepath, file))
		{
			out.Format(IDS_STATUSLIST_CHECKOUTFILEFAILED, filepath->GetGitPathString(), m_CurrentVersion, file);
			CMessageBox::Show(nullptr, g_Git.GetGitLastErr(out, CGit::GIT_CMD_GETONEFILE), _T("TortoiseGit"), MB_OK);
			return;
		}
		SetFileAttributes(file, FILE_ATTRIBUTE_READONLY);
	}
	if(mode == ALTERNATIVEEDITOR)
	{
		CAppUtils::LaunchAlternativeEditor(file);
		return;
	}

	if (mode == OPEN)
		CAppUtils::ShellOpen(file, GetSafeHwnd());
	else
		CAppUtils::ShowOpenWithDialog(file, GetSafeHwnd());
}


void CWorkingChangesFileListCtrl::DeleteSelectedFiles()
{
	//Collect paths
	std::vector<int> selectIndex;

	POSITION pos = GetFirstSelectedItemPosition();
	int index;
	while ((index = GetNextSelectedItem(pos)) >= 0)
	{
		selectIndex.push_back(index);
	}

	//Create file-list ('\0' separated) for SHFileOperation
	CString filelist;
	for (size_t i = 0; i < selectIndex.size(); ++i)
	{
		index = selectIndex[i];

		CTGitPath * path=(CTGitPath*)GetItemData(index);
		ASSERT(path);
		if (path == nullptr)
			continue;

		filelist += path->GetWinPathString();
		filelist += _T("|");
	}
	filelist += _T("|");
	int len = filelist.GetLength();
	std::unique_ptr<TCHAR[]> buf(new TCHAR[len + 2]);
	_tcscpy_s(buf.get(), len + 2, filelist);
	CStringUtils::PipesToNulls(buf.get(), len + 2);
	SHFILEOPSTRUCT fileop;
	fileop.hwnd = this->m_hWnd;
	fileop.wFunc = FO_DELETE;
	fileop.pFrom = buf.get();
	fileop.pTo = NULL;
	fileop.fFlags = FOF_NO_CONNECTED_ELEMENTS | ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 0 : FOF_ALLOWUNDO);
	fileop.lpszProgressTitle = _T("deleting file");
	int result = SHFileOperation(&fileop);

	if ((result == 0) && (!fileop.fAnyOperationsAborted))
	{
		SetRedraw(FALSE);
		POSITION pos = NULL;
		while ((pos = GetFirstSelectedItemPosition()) != 0)
		{
			int index = GetNextSelectedItem(pos);
			if (GetCheck(index))
				m_nSelected--;
			m_nTotal--;

			RemoveListEntry(index);
		}
		SetRedraw(TRUE);
	}
}


// registry access
void ColumnManager::ReadSettings(DWORD defaultColumns, DWORD hideColumns, const CString& containerName, int maxsize, int * widthlist)
{
	// Defaults
	DWORD selectedStandardColumns = defaultColumns & ~hideColumns;
	m_dwDefaultColumns = selectedStandardColumns;

	m_columns.resize(maxsize); 

	for (int i = 0, p = 1; i < maxsize; ++i, p *= 2)
	{
		m_columns[i].index = static_cast<int>(i);

		if (widthlist == NULL)
			m_columns[i].width = 0;
		else
			m_columns[i].width = widthlist[i];

		m_columns[i].visible = true;
		m_columns[i].relevant = !(hideColumns & p);
	}

	// where the settings are stored within the registry
	m_registryPrefix = _T("Software\\TortoiseSI\\WFCColumns\\") + containerName;

	// we accept settings of current version only
	bool valid = (DWORD)CRegDWORD(m_registryPrefix + _T("Version"), 0xff) == GITSLC_COL_VERSION;

	if (valid)
	{
		// read (possibly different) column selection
		selectedStandardColumns = CRegDWORD(m_registryPrefix, selectedStandardColumns) & ~hideColumns;

		// read column widths
		CString colWidths = CRegString(m_registryPrefix + _T("_Width"));

		ParseWidths(colWidths);
	}

	// process old-style visibility setting
	SetStandardColumnVisibility(selectedStandardColumns);

	// clear all previously set header columns
	int c = ((CHeaderCtrl*)(m_control->GetDlgItem(0)))->GetItemCount() - 1;
	while (c >= 0)
		m_control->DeleteColumn(c--);

	// create columns
	for (int i = 0, count = GetColumnCount(); i < count; ++i)
		m_control->InsertColumn(i, GetName(i), LVCFMT_LEFT, IsVisible(i) && IsRelevant(i) ? -1 : GetVisibleWidth(i, false));

	// restore column ordering
	if (valid)
		ParseColumnOrder(CRegString(m_registryPrefix + _T("_Order")));
	else
		ParseColumnOrder(CString());

	ApplyColumnOrder();

	// auto-size the columns so we can see them while fetching status
	// (seems the same values will not take affect in InsertColumn)

	for (int i = 0, count = GetColumnCount(); i < count; ++i)
		if (IsVisible(i))
			m_control->SetColumnWidth(i, GetVisibleWidth(i, true));
}


void ColumnManager::WriteSettings() const
{
	CRegDWORD regVersion(m_registryPrefix + _T("Version"), 0, TRUE);
	regVersion = GITSLC_COL_VERSION;

	// write (possibly different) column selection

	CRegDWORD regStandardColumns(m_registryPrefix, 0, TRUE);
	regStandardColumns = GetSelectedStandardColumns();

	// write column widths

	CRegString regWidths(m_registryPrefix + _T("_Width"), CString(), TRUE);
	regWidths = GetWidthString();

	// write column ordering

	CRegString regColumnOrder(m_registryPrefix + _T("_Order"), CString(), TRUE);
	regColumnOrder = GetColumnOrderString();
}


// read column definitions

int ColumnManager::GetColumnCount() const
{
	return static_cast<int>(m_columns.size());
}

bool ColumnManager::IsVisible(int column) const
{
	size_t index = static_cast<size_t>(column);
	assert(m_columns.size() > index);

	return m_columns[index].visible;
}

int ColumnManager::GetInvisibleCount() const
{
	int invisibleCount = 0;
	for (std::vector<ColumnInfo>::const_iterator it = m_columns.begin(); it != m_columns.end(); ++it)
	{
		if (!it->visible)
			invisibleCount++;
	}
	return invisibleCount;
}

bool ColumnManager::IsRelevant(int column) const
{
	size_t index = static_cast<size_t>(column);
	assert(m_columns.size() > index);

	return m_columns[index].relevant;
}

int ColumnManager::SetNames(UINT* buffer, int size)
{
	itemName.clear();
	for (int i = 0; i < size; ++i)
		itemName.push_back(*buffer++);
	return 0;
}

CString ColumnManager::GetName(int column) const
{
	// standard columns
	size_t index = static_cast<size_t>(column);
	if (index < itemName.size())
	{
		CString result;
		result.LoadString(itemName[index]);
		return result;
	}

	// user-prop columns

	//	if (index < m_columns.size())
	//		return userProps[m_columns[index].index - SVNSLC_USERPROPCOLOFFSET].name;

	// default: empty

	return CString();
}

int ColumnManager::GetWidth(int column, bool useDefaults) const
{
	size_t index = static_cast<size_t>(column);
	assert(m_columns.size() > index);

	int width = m_columns[index].width;
	if ((width == 0) && useDefaults)
		width = LVSCW_AUTOSIZE;

	return width;
}

int ColumnManager::GetVisibleWidth(int column, bool useDefaults) const
{
	return IsVisible(column)
		? GetWidth(column, useDefaults)
		: 0;
}

// switch columns on and off

void ColumnManager::SetVisible
(int column
, bool visible)
{
	size_t index = static_cast<size_t>(column);
	assert(index < m_columns.size());

	if (m_columns[index].visible != visible)
	{
		m_columns[index].visible = visible;
		m_columns[index].relevant |= visible;
		if (!visible)
			m_columns[index].width = 0;

		m_control->SetColumnWidth(column, GetVisibleWidth(column, true));
		ApplyColumnOrder();

		m_control->Invalidate(FALSE);
	}
}

// tracking column modifications

void ColumnManager::ColumnMoved(int column, int position)
{
	// in front of what column has it been inserted?

	int index = m_columns[column].index;

	std::vector<int> gridColumnOrder = GetGridColumnOrder();

	size_t visiblePosition = static_cast<size_t>(position);
	size_t columnCount = gridColumnOrder.size();

	int next = -1;
	if (visiblePosition < columnCount - 1)
	{
		// the new position (visiblePosition) is the column id w/o the moved column
		gridColumnOrder.erase(std::find(gridColumnOrder.begin(), gridColumnOrder.end(), index));
		next = gridColumnOrder[visiblePosition];
	}

	// move logical column index just in front of that "next" column

	columnOrder.erase(std::find(columnOrder.begin(), columnOrder.end(), index));
	columnOrder.insert(std::find(columnOrder.begin(), columnOrder.end(), next), index);

	// make sure, invisible columns are still put in front of all others

	ApplyColumnOrder();
}

void ColumnManager::ColumnResized(int column)
{
	size_t index = static_cast<size_t>(column);
	assert(index < m_columns.size());
	assert(m_columns[index].visible);

	int width = m_control->GetColumnWidth(column);
	m_columns[index].width = width;

	m_control->Invalidate(FALSE);
}

void ColumnManager::RemoveUnusedProps()
{
	// determine what column indexes / IDs to keep.
	// map them onto new IDs (we may delete some IDs in between)

	std::map<int, int> validIndices;

	for (size_t i = 0, count = m_columns.size(); i < count; ++i)
	{
		int index = m_columns[i].index;

		if (itemProps.find(GetName((int)i)) != itemProps.end()
			|| m_columns[i].visible)
		{
			validIndices[index] = index;
		}
	}

	// remove everything else:

	// remove from columns and control.
	// also update index values in columns

	for (size_t i = m_columns.size(); i > 0; --i)
	{
		std::map<int, int>::const_iterator iter
			= validIndices.find(m_columns[i - 1].index);

		if (iter == validIndices.end())
		{
			m_control->DeleteColumn(static_cast<int>(i - 1));
			m_columns.erase(m_columns.begin() + i - 1);
		}
		else
		{
			m_columns[i - 1].index = iter->second;
		}
	}

	// remove from and update column order

	for (size_t i = columnOrder.size(); i > 0; --i)
	{
		std::map<int, int>::const_iterator iter
			= validIndices.find(columnOrder[i - 1]);

		if (iter == validIndices.end())
			columnOrder.erase(columnOrder.begin() + i - 1);
		else
			columnOrder[i - 1] = iter->second;
	}
}

// bring everything back to its "natural" order

void ColumnManager::ResetColumns(DWORD defaultColumns)
{
	// update internal data

	std::sort(columnOrder.begin(), columnOrder.end());

	for (size_t i = 0, count = m_columns.size(); i < count; ++i)
	{
		m_columns[i].width = 0;
		m_columns[i].visible = (i < 32) && (((defaultColumns >> i) & 1) != 0);
	}

	// update UI

	for (int i = 0, count = GetColumnCount(); i < count; ++i)
		m_control->SetColumnWidth(i, GetVisibleWidth(i, true));

	ApplyColumnOrder();

	m_control->Invalidate(FALSE);
}

// initialization utilities

void ColumnManager::ParseWidths(const CString& widths)
{
	for (int i = 0, count = widths.GetLength() / 8; i < count; ++i)
	{
		long width = _tcstol(widths.Mid(i * 8, 8), NULL, 16);
		if (i < (int)itemName.size())
		{
			// a standard column

			m_columns[i].width = width;
		}
		else
		{
			// there is no such column

			assert(width == 0);
		}
	}
}

void ColumnManager::SetStandardColumnVisibility
(DWORD visibility)
{
	for (size_t i = 0; i < itemName.size(); ++i)
	{
		m_columns[i].visible = (visibility & 1) > 0;
		visibility /= 2;
	}
}

void ColumnManager::ParseColumnOrder
(const CString& widths)
{
	std::set<int> alreadyPlaced;
	columnOrder.clear();

	// place columns according to valid entries in orderString

	for (int i = 0, count = widths.GetLength() / 2; i < count; ++i)
	{
		int index = _tcstol(widths.Mid(i * 2, 2), NULL, 16);
		if ((index < (int)itemName.size()))
		{
			alreadyPlaced.insert(index);
			columnOrder.push_back(index);
		}
	}

	// place the remaining colums behind it

	for (int i = 0; i < (int)itemName.size(); ++i)
		if (alreadyPlaced.find(i) == alreadyPlaced.end())
			columnOrder.push_back(i);
}

// map internal column order onto visible column order
// (all invisibles in front)

std::vector<int> ColumnManager::GetGridColumnOrder() const
{
	// extract order of used columns from order of all columns

	std::vector<int> result;
	result.reserve(GITSLC_MAXCOLUMNCOUNT + 1);

	size_t colCount = m_columns.size();
	bool visible = false;

	do
	{
		// put invisible cols in front

		for (size_t i = 0, count = columnOrder.size(); i < count; ++i)
		{
			int index = columnOrder[i];
			for (size_t k = 0; k < colCount; ++k)
			{
				const ColumnInfo& column = m_columns[k];
				if ((column.index == index) && (column.visible == visible))
					result.push_back(static_cast<int>(k));
			}
		}

		visible = !visible;
	} while (visible);

	return result;
}

void ColumnManager::ApplyColumnOrder()
{
	// extract order of used columns from order of all columns

	int order[GITSLC_MAXCOLUMNCOUNT + 1];
	SecureZeroMemory(order, sizeof(order));

	std::vector<int> gridColumnOrder = GetGridColumnOrder();
	std::copy(gridColumnOrder.begin(), gridColumnOrder.end(), stdext::checked_array_iterator<int*>(&order[0], sizeof(order)));

	// we must have placed all columns or something is really fishy ..

	assert(gridColumnOrder.size() == m_columns.size());
	assert(GetColumnCount() == ((CHeaderCtrl*)(m_control->GetDlgItem(0)))->GetItemCount());

	// o.k., apply our column ordering

	m_control->SetColumnOrderArray(GetColumnCount(), order);
}

// utilities used when writing data to the registry

DWORD ColumnManager::GetSelectedStandardColumns() const
{
	DWORD result = 0;
	for (size_t i = itemName.size(); i > 0; --i)
		result = result * 2 + (m_columns[i - 1].visible ? 1 : 0);

	return result;
}

CString ColumnManager::GetWidthString() const
{
	CString result;

	// regular columns

	TCHAR buf[10] = { 0 };
	for (size_t i = 0; i < itemName.size(); ++i)
	{
		_stprintf_s(buf, 10, _T("%08X"), m_columns[i].width);
		result += buf;
	}

	return result;
}

CString ColumnManager::GetColumnOrderString() const
{
	CString result;

	TCHAR buf[3] = { 0 };
	for (size_t i = 0, count = columnOrder.size(); i < count; ++i)
	{
		_stprintf_s(buf, 3, _T("%02X"), columnOrder[i]);
		result += buf;
	}

	return result;
}


void ColumnManager::OnColumnResized(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER header = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if ((header != NULL)
		&& (header->iItem >= 0)
		&& (header->iItem < GetColumnCount()))
	{
		ColumnResized(header->iItem);
	}
	*pResult = 0;
}

void ColumnManager::OnColumnMoved(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER header = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = TRUE;
	if ((header != NULL)
		&& (header->iItem >= 0)
		&& (header->iItem < GetColumnCount())
		// only allow the reordering if the column was not moved left of the first
		// visible item - otherwise the 'invisible' columns are not at the far left
		// anymore and we get all kinds of redrawing problems.
		&& (header->pitem)
		&& (header->pitem->iOrder >= GetInvisibleCount()))
	{
		ColumnMoved(header->iItem, header->pitem->iOrder);
	}
}

void ColumnManager::OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
	if ((phdr->iItem < 0) || (phdr->iItem >= (int)itemName.size()))
		return;

	if (IsVisible(phdr->iItem))
	{
		return;
	}
	*pResult = 1;
}

int ColumnManager::OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
	if ((phdr->iItem < 0) || (phdr->iItem >= (int)itemName.size()))
	{
		return 0;
	}

	// visible columns may be modified

	if (IsVisible(phdr->iItem))
	{
		return 0;
	}

	// columns already marked as "invisible" internally may be (re-)sized to 0

	if ((phdr->pitem != NULL)
		&& (phdr->pitem->mask == HDI_WIDTH)
		&& (phdr->pitem->cxy == 0))
	{
		return 0;
	}

	if ((phdr->pitem != NULL)
		&& (phdr->pitem->mask != HDI_WIDTH))
	{
		return 0;
	}

	*pResult = 1;
	return 1;
}

void ColumnManager::OnContextMenuHeader(CWnd * pWnd, CPoint point, bool isGroundEnable = false)
{
	CHeaderCtrl * pHeaderCtrl = (CHeaderCtrl *)pWnd;
	if ((point.x == -1) && (point.y == -1))
	{
		CRect rect;
		pHeaderCtrl->GetItemRect(0, &rect);
		pHeaderCtrl->ClientToScreen(&rect);
		point = rect.CenterPoint();
	}

	CMenu popup;
	if (popup.CreatePopupMenu())
	{
		int columnCount = GetColumnCount();

		CString temp;
		UINT uCheckedFlags = MF_STRING | MF_ENABLED | MF_CHECKED;
		UINT uUnCheckedFlags = MF_STRING | MF_ENABLED;

		// build control menu
		temp.LoadString(IDS_STATUSLIST_RESETCOLUMNORDER);
		popup.AppendMenu(uUnCheckedFlags, columnCount + 2, temp);
		popup.AppendMenu(MF_SEPARATOR);

		// standard columns
		AddMenuItem(&popup);

		// user-prop columns:
		// find relevant ones and sort 'em

		std::map<CString, int> sortedProps;
		for (int i = (int)itemName.size(); i < columnCount; ++i)
			if (IsRelevant(i))
				sortedProps[GetName(i)] = i;

		if (!sortedProps.empty())
		{
			// add 'em to the menu

			popup.AppendMenu(MF_SEPARATOR);P

			typedef std::map<CString, int>::const_iterator CIT;
			for (CIT iter = sortedProps.begin(), end = sortedProps.end()
				; iter != end
				; ++iter)
			{
				popup.AppendMenu(IsVisible(iter->second)
					? uCheckedFlags
					: uUnCheckedFlags
					, iter->second
					, iter->first);
			}
		}

		// show menu & let user pick an entry

		int cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, pWnd, 0);
		if ((cmd >= 1) && (cmd < columnCount))
		{
			SetVisible(cmd, !IsVisible(cmd));
		}
		else if (cmd == columnCount)
		{
			pWnd->GetParent()->SendMessage(LVM_ENABLEGROUPVIEW, !isGroundEnable, NULL);
			//EnableGroupView(!isGroundEnable);
		}
		else if (cmd == columnCount + 1)
		{
			RemoveUnusedProps();
		}
		else if (cmd == columnCount + 2)
		{
			temp.LoadString(IDS_CONFIRMRESETCOLUMNORDER);
			if (MessageBox(pWnd->m_hWnd, temp, _T("TortoiseGit"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				ResetColumns(m_dwDefaultColumns);
		}
	}
}

void ColumnManager::AddMenuItem(CMenu *pop)
{
	UINT uCheckedFlags = MF_STRING | MF_ENABLED | MF_CHECKED;
	UINT uUnCheckedFlags = MF_STRING | MF_ENABLED;

	for (int i = 1; i < (int)itemName.size(); ++i)
	{
		if (IsRelevant(i))
			pop->AppendMenu(IsVisible(i)
			? uCheckedFlags
			: uUnCheckedFlags
			, i
			, GetName(i));
	}
}


// sorter utility class, only used by GitStatusList!
CSorter::CSorter( ColumnManager* columnManager, int sortedColumn, bool ascending )
	: columnManager(columnManager),
	  sortedColumn(sortedColumn),
	  ascending(ascending)
{
}

bool CSorter::operator() (const std::wstring entry1, const std::wstring entry2) const
{
#define SGN(x) ((x)==0?0:((x)>0?1:-1))

	int result = 0;
	switch (sortedColumn)
	{
	case 7: // File size
	{
		if (result == 0)
		{
			__int64 fileSize1 = entry1->IsDirectory() ? 0 : entry1->GetFileSize();
			__int64 fileSize2 = entry2->IsDirectory() ? 0 : entry2->GetFileSize();

			result = int(fileSize1 - fileSize2);
		}
		break;
	}
	case 6: //Last Modification Date
	{
		if (result == 0)
		{
			__int64 writetime1 = entry1->GetLastWriteTime();
			__int64 writetime2 = entry2->GetLastWriteTime();

			FILETIME* filetime1 = (FILETIME*)(__int64*)&writetime1;
			FILETIME* filetime2 = (FILETIME*)(__int64*)&writetime2;

			result = CompareFileTime(filetime1, filetime2);
		}
		break;
	}
	case 5: //Del Number
	{
		if (result == 0)
		{
			//				result = entry1->lock_comment.CompareNoCase(entry2->lock_comment);
			result = A2L(entry1->m_StatDel) - A2L(entry2->m_StatDel);
		}
		break;
	}
	case 4: //Add Number
	{
		if (result == 0)
		{
			//result = entry1->lock_owner.CompareNoCase(entry2->lock_owner);
			result = A2L(entry1->m_StatAdd) - A2L(entry2->m_StatAdd);
		}
		break;
	}

	case 3: // Status
	{
		if (result == 0)
		{
			result = entry1->GetActionName(entry1->m_Action).CompareNoCase(entry2->GetActionName(entry2->m_Action));
		}
		break;
	}
	case 2: //Ext file
	{
		if (result == 0)
		{
			result = entry1->GetFileExtension().CompareNoCase(entry2->GetFileExtension());
		}
		break;
	}
	case 1: // File name
	{
		if (result == 0)
		{
			result = entry1->GetFileOrDirectoryName().CompareNoCase(entry2->GetFileOrDirectoryName());
		}
		break;
	}
	case 0: // Full path column
	{
		if (result == 0)
		{
			result = CTGitPath::Compare(entry1->GetGitPathString(), entry2->GetGitPathString());
		}
		break;
	}
	} // switch (m_nSortedColumn)
	// sort by path name as second priority
	if (sortedColumn > 0 && result == 0)
		result = CTGitPath::Compare(entry1->GetGitPathString(), entry2->GetGitPathString());
	if (!ascending)
		result = -result;

	return result < 0;
}