// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2014 - TortoiseGit
// Copyright (C) 2003-2008, 2014 - TortoiseSVN

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
#pragma once
#include "stdafx.h"
#include "IntegrityActions.h"
//#include "TGitPath.h"
//#include "GitStatus.h"
//#include "GitRev.h"
//#include "Colors.h"
//#include "LoglistCommonResource.h"
//#include "HintListCtrl.h"

#define GIT_WC_ENTRY_WORKING_SIZE_UNKNOWN (-1)

typedef enum SIWC_Columns_e : DWORD {
	SIWC_FULLPATH  = 1 << 0,
	SIWC_FILENAME  = 1 << 1,
	SIWC_EXTENSION = 1 << 2,
	SIWC_STATUS    = 1 << 3,
} SIWC_Columns;

#define SIWC_NUMCOLUMNS 4

#define GITSLC_SHOWUNVERSIONED	CTGitPath::LOGACTIONS_UNVER
#define GITSLC_SHOWNORMAL		0x00000000
#define GITSLC_SHOWMODIFIED		(CTGitPath::LOGACTIONS_MODIFIED)
#define GITSLC_SHOWADDED		(CTGitPath::LOGACTIONS_ADDED|CTGitPath::LOGACTIONS_COPY)
#define GITSLC_SHOWREMOVED		CTGitPath::LOGACTIONS_DELETED
#define GITSLC_SHOWCONFLICTED	CTGitPath::LOGACTIONS_UNMERGED
#define GITSLC_SHOWMISSING		0x00000000
#define GITSLC_SHOWREPLACED		CTGitPath::LOGACTIONS_REPLACED
#define GITSLC_SHOWMERGED		CTGitPath::LOGACTIONS_MERGED
#define GITSLC_SHOWIGNORED		CTGitPath::LOGACTIONS_IGNORE
#define GITSLC_SHOWOBSTRUCTED	0x00000000
#define GITSLC_SHOWEXTERNAL		0x00000000
#define GITSLC_SHOWINCOMPLETE	0x00000000
#define GITSLC_SHOWINEXTERNALS	0x00000000
#define GITSLC_SHOWREMOVEDANDPRESENT 0x00000000
#define GITSLC_SHOWLOCKS		0x00000000
#define GITSLC_SHOWDIRECTFILES	0x04000000
#define GITSLC_SHOWDIRECTFOLDER 0x00000000
#define GITSLC_SHOWEXTERNALFROMDIFFERENTREPO 0x00000000
#define GITSLC_SHOWSWITCHED		0x00000000
#define GITSLC_SHOWINCHANGELIST 0x00000000
#define GITSLC_SHOWASSUMEVALID  CTGitPath::LOGACTIONS_ASSUMEVALID
#define GITSLC_SHOWSKIPWORKTREE CTGitPath::LOGACTIONS_SKIPWORKTREE

#define GITSLC_SHOWDIRECTS		(GITSLC_SHOWDIRECTFILES | GITSLC_SHOWDIRECTFOLDER)

#define GITSLC_SHOWFILES		0x01000000
#define GITSLC_SHOWSUBMODULES	0x02000000
#define GITSLC_SHOWEVERYTHING	0xffffffff

#define GITSLC_SHOWVERSIONED (CTGitPath::LOGACTIONS_FORWORD|GITSLC_SHOWNORMAL|GITSLC_SHOWMODIFIED|\
GITSLC_SHOWADDED|GITSLC_SHOWREMOVED|GITSLC_SHOWCONFLICTED|GITSLC_SHOWMISSING|\
GITSLC_SHOWREPLACED|GITSLC_SHOWMERGED|GITSLC_SHOWIGNORED|GITSLC_SHOWOBSTRUCTED|\
GITSLC_SHOWEXTERNAL|GITSLC_SHOWINCOMPLETE|GITSLC_SHOWINEXTERNALS|\
GITSLC_SHOWEXTERNALFROMDIFFERENTREPO)

#define GITSLC_SHOWVERSIONEDBUTNORMAL (GITSLC_SHOWMODIFIED|GITSLC_SHOWADDED|\
GITSLC_SHOWREMOVED|GITSLC_SHOWCONFLICTED|GITSLC_SHOWMISSING|\
GITSLC_SHOWREPLACED|GITSLC_SHOWMERGED|GITSLC_SHOWIGNORED|GITSLC_SHOWOBSTRUCTED|\
GITSLC_SHOWEXTERNAL|GITSLC_SHOWINCOMPLETE|GITSLC_SHOWINEXTERNALS|\
GITSLC_SHOWEXTERNALFROMDIFFERENTREPO)

#define GITSLC_SHOWVERSIONEDBUTNORMALANDEXTERNALSFROMDIFFERENTREPOS (GITSLC_SHOWMODIFIED|\
GITSLC_SHOWADDED|GITSLC_SHOWREMOVED|GITSLC_SHOWCONFLICTED|GITSLC_SHOWMISSING|\
GITSLC_SHOWREPLACED|GITSLC_SHOWMERGED|GITSLC_SHOWIGNORED|GITSLC_SHOWOBSTRUCTED|\
GITSLC_SHOWINCOMPLETE|GITSLC_SHOWEXTERNAL|GITSLC_SHOWINEXTERNALS)

#define GITSLC_SHOWVERSIONEDBUTNORMALANDEXTERNALS (GITSLC_SHOWMODIFIED|\
	GITSLC_SHOWADDED|GITSLC_SHOWREMOVED|GITSLC_SHOWCONFLICTED|GITSLC_SHOWMISSING|\
	GITSLC_SHOWREPLACED|GITSLC_SHOWMERGED|GITSLC_SHOWIGNORED|GITSLC_SHOWOBSTRUCTED|\
	GITSLC_SHOWINCOMPLETE)

#define GITSLC_SHOWALL (GITSLC_SHOWVERSIONED|GITSLC_SHOWUNVERSIONED)

#define GITSLC_POPALL					0xFFFFFFFFFFFFFFFF
#define GITSLC_POPCOMPAREWITHBASE		CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_COMPARE)
#define GITSLC_POPCOMPARE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_COMPAREWC)
#define GITSLC_POPGNUDIFF				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_GNUDIFF1)
#define GITSLC_POPREVERT				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_REVERT)
#define GITSLC_POPSHOWLOG				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_LOG)
#define GITSLC_POPSHOWLOGSUBMODULE		CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_LOGSUBMODULE)
#define GITSLC_POPSHOWLOGOLDNAME		CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_LOGOLDNAME)
#define GITSLC_POPOPEN					CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_OPEN)
#define GITSLC_POPDELETE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_DELETE)
#define GITSLC_POPADD					CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_ADD)
#define GITSLC_POPIGNORE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_IGNORE)
#define GITSLC_POPCONFLICT				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_EDITCONFLICT)
#define GITSLC_POPRESOLVE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_RESOLVECONFLICT)
#define GITSLC_POPEXPLORE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_EXPLORE)
#define GITSLC_POPCOMMIT				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_COMMIT)
#define GITSLC_POPCHANGELISTS			CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_CHECKGROUP)
#define GITSLC_POPBLAME					CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_BLAME)
#define GITSLC_POPSAVEAS				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_SAVEAS)
#define GITSLC_POPCOMPARETWOFILES		CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_COMPARETWOFILES)
#define GITSLC_POPRESTORE				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_POPRESTORE)
#define GITSLC_POPASSUMEVALID			CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_ASSUMEVALID)
#define GITSLC_POPSKIPWORKTREE			CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_SKIPWORKTREE)
#define GITSLC_POPEXPORT				CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_EXPORT)
#define GITLC_POPUNSETIGNORELOCALCHANGES CWorkingChangesFileListCtrl::GetContextMenuBit(CWorkingChangesFileListCtrl::IDGITLC_UNSETIGNORELOCALCHANGES)

#define GITSLC_IGNORECHANGELIST			_T("ignore-on-commit")

// This gives up to 64 standard properties and menu entries
#define GITSLC_MAXCOLUMNCOUNT           0xff

#define OVL_RESTORE			1

typedef int (__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
typedef CComCritSecLock<CComCriticalSection> Locker;

class CWorkingChangesFileListCtrlDropTarget;

/**
* \ingroup TortoiseSIProc
* Helper class for CWorkingChangesFileListCtrl that represents
* the columns visible and their order as well as
* persisting that data in the registry.
*
* It assigns logical index values to the (potential) columns:
* 0 .. GetColumnCount()-1 contain the standard attributes
*
* The column vector contains the columns that are actually
* available in the control.
*
*/
class ColumnManager
{
public:

	// Constructor
	ColumnManager (CListCtrl* control) 
	{
		m_control = control;
		m_dwDefaultColumns = 0;
	};

	// Destructor
	~ColumnManager() {};

	// Registry access
	void ReadSettings (DWORD defaultColumns, DWORD hideColumns, const CString& containerName, int ReadSettings, int *withlist=NULL);
	void WriteSettings() const;

	// Total number of columns
	int GetColumnCount() const;  

	// Read column definitions
	bool IsVisible (int column) const;
	int GetInvisibleCount() const;
	bool IsRelevant (int column) const;
	CString GetName (int column) const;
	int SetNames(UINT * buff, int size);
	int GetWidth (int column, bool useDefaults = false) const;
	int GetVisibleWidth (int column, bool useDefaults) const;

	// Switch columns on and off
	void SetVisible (int column, bool visible);

	// Tracking column modifications
	void ColumnMoved (int column, int position);
	void ColumnResized (int column);

	// Don't clutter the context menu with irrelevant prop info
	void RemoveUnusedProps();

	// Bring everything back to its "natural" order
	void ResetColumns (DWORD defaultColumns);

	void OnColumnResized(NMHDR *pNMHDR, LRESULT *pResult);
	void OnColumnMoved(NMHDR *pNMHDR, LRESULT *pResult);
	void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
	int OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	void OnContextMenuHeader(CWnd * pWnd, CPoint point, bool isGroundEnable = false);
	void AddMenuItem(CMenu *pop);

protected:

	// Initialization utilities
	void ParseWidths (const CString& widths);
	void SetStandardColumnVisibility (DWORD visibility);
	void ParseColumnOrder (const CString& widths);

	// Map internal column order onto visible column order
	// (all invisibles in front)
	std::vector<int> GetGridColumnOrder() const;
	void ApplyColumnOrder();

	// Utilities used when writing data to the registry
	DWORD GetSelectedStandardColumns() const;
	CString GetWidthString() const;
	CString GetColumnOrderString() const;

	// Our parent control and its data
	CListCtrl* m_control;

	// Where to store in the registry
	CString m_registryPrefix;

	// All columns in their "natural" order
	typedef struct ColumnInfo_s
	{
		int index;     // Is a user prop when < GitSLC_USERPROPCOLOFFSET
		int width;
		bool visible;
		bool relevant; // Set to @a visible, if no *shown* item has that property
	} ColumnInfo;

	std::vector<ColumnInfo> m_columns;

	// User-defined properties
	std::set<CString> itemProps;

	// Global column ordering including unused user props
	std::vector<int> columnOrder;

	std::vector<int> itemName;

	DWORD m_dwDefaultColumns;
};

/**
* \ingroup TortoiseSIProc
* Simple utility class that defines the sort column order.
*/
class CSorter
{
public:

	CSorter ( ColumnManager* columnManager, int sortedColumn, bool ascending);

	bool operator() ( const std::wstring entry1, const std::wstring entry2) const;

	static int A2L(const CString &str)
	{
		if(str==_T("-"))
			return -1;
		else
			return _ttol(str);
	}

private:

	ColumnManager* columnManager;
	int sortedColumn;
	bool ascending;
};

/**
 * \ingroup TortoiseSIProc
 * A List control, based on the MFC CListCtrl which shows a list of
 * files with their status. The files will be grouped by change package
 * and non-members.  The control also provides a context menu to do some SI 
 * commands on the selected files.
 *
 * This is the TortoiseSI equivalent to the working file changes view.
 */
class CWorkingChangesFileListCtrl :	public CListCtrl
{
public:
	enum
	{
		IDGITLC_REVERT = 1,
		/** Compare with base version. when current version is zero (i.e. working tree changes), compare working tree and HEAD */
		IDGITLC_COMPARE,
		IDGITLC_OPEN,
		IDGITLC_DELETE,
		IDGITLC_IGNORE,
		/** Compare with base version and generate unified diff. when current version is zero (i.e. working tree changes), compare working tree and HEAD  */
		IDGITLC_GNUDIFF1		 ,
		IDGITLC_LOG              ,
		IDGITLC_LOGOLDNAME,
		IDGITLC_LOGSUBMODULE,
		IDGITLC_EDITCONFLICT     ,
		IDGITLC_IGNOREMASK	    ,
		IDGITLC_IGNOREFOLDER    ,
		IDGITLC_ADD			    ,
		IDGITLC_RESOLVECONFLICT ,
		IDGITLC_OPENWITH		,
		IDGITLC_EXPLORE			,
		IDGITLC_RESOLVETHEIRS	,
		IDGITLC_RESOLVEMINE		,
		IDGITLC_REMOVE			,
		IDGITLC_COMMIT			,
		IDGITLC_COPY			,
		IDGITLC_COPYEXT			,
		IDGITLC_REMOVEFROMCS	,
		IDGITLC_CREATECS		,
		IDGITLC_CREATEIGNORECS	,
		IDGITLC_CHECKGROUP		,
		IDGITLC_UNCHECKGROUP	,
		/** Compare current version and working tree */
		IDGITLC_COMPAREWC		,
		IDGITLC_BLAME			,
		IDGITLC_SAVEAS			,
		IDGITLC_REVERTTOREV		,
		IDGITLC_REVERTTOPARENT	,
		IDGITLC_VIEWREV			,
		IDGITLC_FINDENTRY       ,
		/** used in sync dlg, compare in/out file changes; in combination with m_Rev1 and m_Rev2 */
		IDGITLC_COMPARETWOREVISIONS,
		/** used in sync dlg, compare in/out file changes; in combination with m_Rev1 and m_Rev2 */
		IDGITLC_GNUDIFF2REVISIONS,
		/** Compare two selected files */
		IDGITLC_COMPARETWOFILES	,
		IDGITLC_POPRESTORE		,
		IDGITLC_CREATERESTORE	,
		IDGITLC_RESTOREPATH		,
		IDGITLC_ASSUMEVALID		,
		IDGITLC_SKIPWORKTREE	,
		IDGITLC_EXPORT			,
		IDGITLC_UNSETIGNORELOCALCHANGES,
        /** the IDSVNLC_MOVETOCS *must* be the last index, because it contains a dynamic submenu where */
        /** the submenu items get command ID's sequent to this number */
		IDGITLC_MOVETOCS		,
	};

	int GetColumnIndex(int colmask);

	static inline unsigned __int64 GetContextMenuBit(int i) { 
		return ((unsigned __int64 )0x1)<<i ;
	}

	/**
	 * Sent to the parent window (using ::SendMessage) after a context menu
	 * command has finished if the item count has changed.
	 */
	static const UINT GITSLNM_ITEMCOUNTCHANGED;

	/**
	 * Sent to the parent window (using ::SendMessage) when the control needs
	 * to be refreshed. Since this is done usually in the parent window using
	 * a thread, this message is used to tell the parent to do exactly that.
	 */
	static const UINT GITSLNM_NEEDSREFRESH;

	/**
	 * Sent to the parent window (using ::SendMessage) when the user drops
	 * files on the control. The LPARAM is a pointer to a TCHAR string
	 * containing the dropped path.
	 */
	static const UINT GITSLNM_ADDFILE;

	/**
	 * Sent to the parent window (using ::SendMessage) when the user checks/unchecks
	 * one or more items in the control. The WPARAM contains the number of
	 * checked items in the control.
	 */
	static const UINT GITSLNM_CHECKCHANGED;

	static const UINT GITSLNM_ITEMCHANGED;

	CWorkingChangesFileListCtrl(void);
	~CWorkingChangesFileListCtrl(void);

	/**
	 * Initializes the control, sets up the columns.
	 * \param dwColumns mask of columns to show. Use the SIWC_Columns_t enum.
	 * \param sColumnInfoContainer Name of a registry key
	 *                             where the position and visibility of each column
	 *                             is saved and used from. If the registry key
	 *                             doesn't exist, the default order is used
	 *                             and dwColumns tells which columns are visible.
	 * \param dwContextMenus mask of context menus to be active, not all make sense for every use of this control.
	 *                       Use the GitSLC_POPxxx defines.
	 * \param bHasCheckboxes TRUE if the control should show check boxes on the left of each file entry.
	 */
	void Init(DWORD dwColumns, const CString& sColumnInfoContainer, unsigned __int64 dwContextMenus = ((GITSLC_POPALL ^ GITSLC_POPCOMMIT) ^ GITSLC_POPRESTORE), bool bHasCheckboxes = true, DWORD allowedColumns = 0xffffffff);

	/**
	 * Sets a background image for the list control.
	 * The image is shown in the right bottom corner.
	 * \param nID the resource ID of the bitmap to use as the background
	 */
	bool SetBackgroundImage(UINT nID);

	/**
	 * Makes the 'ignore' context menu only ignore the files and not add the
	 * folder which gets the Git:ignore property changed to the list.
	 * This is needed e.g. for the Add-dialog, where the modified folder
	 * showing up would break the resulting "add" command.
	 */
	void SetIgnoreRemoveOnly(bool bRemoveOnly = true) {m_bIgnoreRemoveOnly = bRemoveOnly;}

	/**
	 * The unversioned items are by default shown after all other files in the list.
	 * If that behavior should be changed, set this value to false.
	 */
	void PutUnversionedLast(bool bLast) {m_bUnversionedLast = bLast;}

	/**
	 * Fetches the git status of all files and stores the information
	 * about them in an internal array.
	 * \param sFilePath path to a file which contains a list of files and/or folders for which to
	 *                  fetch the status, separated by newlines.
	 * \param bUpdate TRUE if the remote status is requested too.
	 * \return TRUE on success.
	 */
	BOOL GetStatus ( const CTGitPathList* pathList=NULL
				   , bool bUpdate = false
				   , bool bShowIgnores = false
				   , bool bShowUnRev = false
				   , bool bShowLocalChangesIgnored = false);

	/**
	 * Populates the list control with the previously (with GetStatus) gathered status information.
	 * \param dwShow mask of file types to show. Use the GitSLC_SHOWxxx defines.
	 * \param dwCheck mask of file types to check. Use GitLC_SHOWxxx defines. Default (0) means 'use the entry's stored check status'
	 */
	void Show(unsigned int dwShow, unsigned int dwCheck = 0, bool bShowFolders = true,BOOL updateStatusList=FALSE, bool UseStoredCheckStatus=false);
	void Show(unsigned int dwShow, const std::wstring & checkedList, bool bShowFolders = true);

	/**
	 * Copies the selected entries in the control to the clipboard. The entries
	 * are separated by newlines.
	 * \param dwCols the columns to copy. Each column is separated by a tab.
	 */
	bool CopySelectedEntriesToClipboard(DWORD dwCols);

	/**
	 * If during the call to GetStatus() some Git:externals are found from different
	 * repositories than the first one checked, then this method returns TRUE.
	 */
	BOOL HasExternalsFromDifferentRepos() const {return m_bHasExternalsFromDifferentRepos;}

	/**
	 * If during the call to GetStatus() some Git:externals are found then this method returns TRUE.
	 */
	BOOL HasExternals() const {return m_bHasExternals;}

	/**
	 * If unversioned files are found (but not necessarily shown) TRUE is returned.
	 */
	BOOL HasUnversionedItems() {return m_bHasUnversionedItems;}

	/**
	 * If there are any locks in the working copy, TRUE is returned
	 */
	BOOL HasLocks() const {return m_bHasLocks;}

	/**
	 * If there are any change lists defined in the working copy, TRUE is returned
	 */
	BOOL HasChangeLists() const {return m_bHasChangeLists;}

	/**
	 * Returns the file entry data for the list control index.
	 */
	//CWorkingChangesFileListCtrl::FileEntry * GetListEntry(UINT_PTR index);

	/**
	 * Returns the file entry data for the specified path.
	 * \note The entry might not be shown in the list control.
	 */
	//CWorkingChangesFileListCtrl::FileEntry * GetListEntry(const CTGitPath& path);

	/**
	 * Returns the index of the list control entry with the specified path,
	 * or -1 if the path is not in the list control.
	 */
	int GetIndex(const CTGitPath& path);

	/**
	 * Returns the file entry data for the specified path in the list control.
	 */
	//CWorkingChangesFileListCtrl::FileEntry * GetVisibleListEntry(const CTGitPath& path);

	/**
	 * Returns a String containing some statistics like number of modified, normal, deleted,...
	 * files.
	 */
	CString GetStatisticsString(bool simple=false);

	/**
	 * Set a static control which will be updated automatically with
	 * the number of selected and total files shown in the list control.
	 */
	void SetStatLabel(CWnd * pStatLabel){m_pStatLabel = pStatLabel;};

	/**
	 * Set a tri-state checkbox which is updated automatically if the
	 * user checks/unchecks file entries in the list control to indicate
	 * if all files are checked, none are checked or some are checked.
	 */
	void SetSelectButton(CButton * pButton) {m_pSelectButton = pButton;}

	/**
	 * Set a button which is de-/activated automatically. The button is
	 * only set active if at least one item is selected.
	 */
	void SetConfirmButton(CButton * pButton) {m_pConfirmButton = pButton;}

	/**
	 * Select/unselect all entries in the list control.
	 * \param bSelect TRUE to check, FALSE to uncheck.
	 */
	void SelectAll(bool bSelect, bool bIncludeNoCommits = false);

	/**
	 * Checks or unchecks all specified items
	 * \param dwCheck GITLC_SHOWxxx defines
	 * \param check if true matching items will be selected, false unchecks matching items
	 */
	void Check(DWORD dwCheck, bool check = true);

	/** Set a checkbox on an entry in the listbox
	 * Keeps the listctrl checked state and the FileEntry's checked flag in sync
	 */
	void SetEntryCheck(CTGitPath* pEntry, int listboxIndex, bool bCheck);

	/** Write a list of the checked items' paths into a path list
	 */
	void WriteCheckedNamesToPathList(CTGitPathList& pathList);

	/** fills in \a lMin and \a lMax with the lowest/highest revision of all
	 * files/folders in the working copy.
	 * \param bShownOnly if true, the min/max revisions are calculated only for shown items
	 * \param bCheckedOnly if true, the min/max revisions are calculated only for items
	 *                   which are checked.
	 * \remark Since an item can only be checked if it is visible/shown in the list control
	 *         bShownOnly is automatically set to true if bCheckedOnly is true
	 */
	void GetMinMaxRevisions(git_revnum_t& rMin, git_revnum_t& rMax, bool bShownOnly, bool bCheckedOnly);

	/**
	 * Returns the parent directory of all entries in the control.
	 * if \a bStrict is set to false, then the paths passed to the control
	 * to fetch the status (in GetStatus()) are used if possible.
	 */
	CString GetCommonDirectory(bool bStrict);

	/**
	 * Returns the parent url of all entries in the control.
	 * if \a bStrict is set to false, then the paths passed to the control
	 * to fetch the status (in GetStatus()) are used if possible.
	 */
	CTGitPath GetCommonURL(bool bStrict);

	/**
	 * Sets a pointer to a boolean variable which is checked periodically
	 * during the status fetching. As soon as the variable changes to true,
	 * the operations stops.
	 */
	void SetCancelBool(bool * pbCanceled) {m_pbCanceled = pbCanceled;}

	/**
	 * Sets the string shown in the control while the status is fetched.
	 * If not set, it defaults to "please wait..."
	 */
	void SetBusyString(const CString& str) {m_sBusy = str;}
	void SetBusyString(UINT id) {m_sBusy.LoadString(id);}

	/**
	 * Sets the string shown in the control if no items are shown. This
	 * can happen for example if there's nothing modified and the unversioned
	 * files aren't shown either, so there's nothing to commit.
	 * If not set, it defaults to "file list is empty".
	 */
	void SetEmptyString(const CString& str) {m_sEmpty = str;}
	void SetEmptyString(UINT id) {m_sEmpty.LoadString(id);}

	/**
	 * Returns the number of selected items
	 */
	LONG GetSelected(){return m_nSelected;};

	/**
	 * Enables dropping of files on the control.
	 */
	bool EnableFileDrop();

	/**
	 * Checks if the path already exists in the list.
	 */
	bool HasPath(const CTGitPath& path);
	
	/**
	 * Checks if the path is shown/visible in the list control.
	 */
	bool IsPathShown(const CTGitPath& path);
	
	/**
	 * Forces the children to be checked when the parent folder is checked,
	 * and the parent folder to be unchecked if one of its children is unchecked.
	 */
	void CheckChildrenWithParent(bool bCheck) {m_bCheckChildrenWithParent = bCheck;}

	/**
	 * Allows checking the items if change lists are present. If set to false,
	 * items are not checked if at least one changelist is available.
	 */
	void CheckIfChangelistsArePresent(bool bCheck) {m_bCheckIfGroupsExist = bCheck;}
	
	/**
	 * Returns the currently used show flags passed to the Show() method.
	 */
	
	DWORD GetShowFlags() {return m_dwShow;}

public:
	CString GetLastErrorMessage() {return m_sLastError;}

	void Block(BOOL block, BOOL blockUI) {m_bBlock = block; m_bBlockUI = blockUI;}

	LONG GetUnversionedCount() { return m_nShownUnversioned; }
	LONG GetModifiedCount() { return m_nShownModified; }
	LONG GetAddedCount() { return m_nShownAdded; }
	LONG GetDeletedCount() { return m_nShownDeleted; }
	LONG GetConflictedCount() { return m_nShownConflicted; }
	LONG GetFileCount() { return m_nShownFiles; }
	LONG GetSubmoduleCount() { return m_nShownSubmodules; }

	LONG						m_nTargetCount;		//< number of targets in the file passed to GetStatus()

	CString						m_sURL;				//< the URL of the target or "(multiple targets)"

	GitRev						m_HeadRev;			//< the HEAD revision of the repository if bUpdate was TRUE

	bool						m_amend;			//< if true show the changes to the revision before the last commit

	CString						m_sUUID;			//< the UUID of the associated repository

	CString						m_sDisplayedBranch; //< When on LogDialog, what is the current displayed branch

	bool						m_bIsRevertTheirMy;	//< at rebase case, Their and My version is revert.

	CWnd						*m_hwndLogicalParent;

	DECLARE_MESSAGE_MAP()

public:
	void SetBusy(bool b) {m_bBusy = b; Invalidate();}
	void SetHasCheckboxes(bool bHasCheckboxes)
	{
		m_bHasCheckboxes = bHasCheckboxes;
		DWORD exStyle = GetExtendedStyle();
		if (bHasCheckboxes)
			exStyle |= LVS_EX_CHECKBOXES;
		else
			exStyle &= ~LVS_EX_CHECKBOXES;
		SetExtendedStyle(exStyle);
	}

private:
	void SaveColumnWidths(bool bSaveToRegistry = false);
	//void AddEntry(FileEntry * entry, WORD langID, int listIndex);	//< add an entry to the control
	void RemoveListEntry(int index);	//< removes an entry from the listcontrol and both arrays
	bool BuildStatistics();	//< build the statistics and correct the case of files/folders
	void StartDiff(int fileindex);	//< start the external diff program
	void StartDiffWC(int fileindex);	//< start the external diff program
	void StartDiffTwo(int fileindex);

	void SetGitIndexFlagsForSelectedFiles(UINT message, BOOL assumevalid, BOOL skipworktree);
	
	bool SetBackgroundImage(HWND hWnd, UINT nID, int width, int height);

	enum
	{
		ALTERNATIVEEDITOR,
		OPEN,
		OPEN_WITH,
	};
	void OpenFile(CTGitPath *path,int mode);

	// Sort predicate function - Compare the paths of two entries without regard to case
	//static bool EntryPathCompareNoCase(const FileEntry* pEntry1, const FileEntry* pEntry2);

	// Predicate used to build a list of only the versioned entries of the FileEntry array
	//static bool IsEntryVersioned(const FileEntry* pEntry1);

	// Look up the relevant show flags for a particular Git status value
	DWORD GetShowFlagsFromGitStatus(git_wc_status_kind status);

	// Adjust the checkbox-state on all descendants of a specific item
	//void SetCheckOnAllDescendentsOf(const FileEntry* parentEntry, bool bCheck);

	// Build a path list of all the selected items in the list (NOTE - SELECTED, not CHECKED)
	void FillListOfSelectedItemPaths(CTGitPathList& pathList, bool bNoIgnored = false);

	// Enables/Disables group view and adds all groups to the list control.
	// If bForce is true, then group view is enabled and the 'null' group is added.
	bool PrepareGroups(bool bForce = false);
	// Returns the group number to which the group header belongs
	// If the point is not over a group header, -1 is returned
	int GetGroupFromPoint(POINT * ppt);
	// Returns the number of change lists the selection has
	size_t GetNumberOfChangelistsInSelection();

	// Puts the item to the corresponding group
	bool SetItemGroup(int item, int groupindex);

	void CheckEntry(int index, int nListItems);
	void UncheckEntry(int index, int nListItems);

	// sends an GitSLNM_CHECKCHANGED notification to the parent
	void NotifyCheck();
	CWnd * GetLogicalParent() { return m_hwndLogicalParent != NULL ? m_hwndLogicalParent : this->GetParent(); }

	void OnContextMenuList(CWnd * pWnd, CPoint point);
	void OnContextMenuGroup(CWnd * pWnd, CPoint point);
	void OnContextMenuHeader(CWnd * pWnd, CPoint point);
	bool CheckMultipleDiffs();

	void DeleteSelectedFiles();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnToolTipText(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHeadingItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListViewItemChanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnListViewItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnColumnResized(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnColumnMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	void CreateChangeList(const CString& name);

	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnNMReturn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnHdnBegintrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();


	void FilesExport();
	void FileSaveAs(CTGitPath *path);
	int RevertSelectedItemToVersion(bool parent = false);

private:

	std::vector<std::shared_ptr<IntegrityActions::ChangePackage> *>   m_changePackages;
	std::vector<std::shared_ptr<IntegrityActions::WorkingFileChange>> m_workingFileChanges;

	bool *						m_pbCanceled;
	bool					    m_bAscending;		//< sort direction
	int					        m_nSortedColumn;	//< which column to sort
	bool						m_bHasCheckboxes;
	bool						m_bUnversionedLast;
	bool						m_bHasExternalsFromDifferentRepos;
	bool						m_bHasExternals;
	BOOL						m_bHasUnversionedItems;
	bool						m_bHasLocks;
	bool						m_bHasChangeLists;
	//typedef std::vector<FileEntry*> FileEntryVector;
	//FileEntryVector				m_arStatusArray;
	std::vector<CTGitPath*>		m_arStatusArray;
	std::vector<size_t>			m_arListArray;
	std::map<CString, int>	    m_changelists;
	bool						m_bHasIgnoreGroup;
	//CTGitPathList				m_ConflictFileList;
	CTGitPathList				m_StatusFileList;
	CTGitPathList				m_UnRevFileList;
	CTGitPathList				m_IgnoreFileList;
	CTGitPathList				m_LocalChangesIgnoredFileList; // assume valid & skip worktree
	//CTGitPathList				m_StatusUrlList;
	CString						m_sLastError;

	LONG						m_nUnversioned;
	LONG						m_nNormal;
	LONG						m_nModified;
	LONG						m_nAdded;
	LONG						m_nDeleted;
	LONG						m_nConflicted;
	LONG						m_nTotal;
	LONG						m_nSelected;
	LONG						m_nLineAdded;
	LONG						m_nLineDeleted;
	LONG						m_nRenamed;

	LONG						m_nShownUnversioned;
	LONG						m_nShownModified;
	LONG						m_nShownAdded;
	LONG						m_nShownDeleted;
	LONG						m_nShownConflicted;
	LONG						m_nShownFiles;
	LONG						m_nShownSubmodules;

	DWORD						m_dwDefaultColumns;
	DWORD						m_dwShow;
	bool						m_bShowFolders;
	bool						m_bShowIgnores;
	bool						m_bUpdate;
	unsigned __int64			m_dwContextMenus;
	BOOL						m_bBlock;
	BOOL						m_bBlockUI;
	bool						m_bBusy;
	bool						m_bEmpty;
	bool						m_bIgnoreRemoveOnly;
	bool						m_bCheckIfGroupsExist;
	bool						m_bFileDropsEnabled;
	bool						m_bOwnDrag;

	int							m_nIconFolder;
	int							m_nRestoreOvl;

	CWnd *						m_pStatLabel;
	CButton *					m_pSelectButton;
	CButton *					m_pConfirmButton;
	CColors						m_Colors;

	CString						m_sEmpty;
	CString						m_sBusy;
	CString						m_sNoPropValueText;

	bool						m_bCheckChildrenWithParent;
	CWorkingChangesFileListCtrlDropTarget * m_pDropTarget;

	ColumnManager				m_ColumnManager;

	std::map<CString,bool>		m_mapFilenameToChecked; //< Remember de-/selected items
	std::map<CString,bool>		m_mapDirectFiles;
	CComCriticalSection			m_criticalSection;

	friend class CWorkingChangesFileListCtrlDropTarget;
public:
	enum
	{
		FILELIST_MODIFY= 0x1,
		FILELIST_UNVER = 0x2,
		FILELIST_IGNORE =0x4,
		FILELIST_LOCALCHANGESIGNORED = 0x8, // assume valid & skip worktree files
	};
public:
	int UpdateFileList(git_revnum_t hash,CTGitPathList *List=NULL);
	int UpdateFileList(int mask, bool once=true,CTGitPathList *List=NULL);
	int UpdateUnRevFileList(CTGitPathList &list);
	int UpdateUnRevFileList(CTGitPathList *List=NULL);
	int UpdateIgnoreFileList(CTGitPathList *List=NULL);
	int UpdateLocalChangesIgnoredFileList(CTGitPathList* list = nullptr);

	int UpdateWithGitPathList(CTGitPathList &list);

	void AddEntry(CTGitPath* path, WORD langID, int ListIndex);
	void Clear();
	int m_FileLoaded;
	git_revnum_t m_CurrentVersion;
	bool m_bDoNotAutoselectSubmodules;
	std::map<CString, CString>	m_restorepaths;
};
