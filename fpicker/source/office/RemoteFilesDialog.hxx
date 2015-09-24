/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX
#define INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX

#include <comphelper/docpasswordrequest.hxx>

#include <svtools/autocmpledit.hxx>
#include <svtools/foldertree.hxx>
#include <svtools/place.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <svtools/breadcrumb.hxx>
#include <svtools/fileview.hxx>

#include <tools/errinf.hxx>
#include <tools/resid.hxx>

#include <unotools/viewoptions.hxx>

#include <vcl/button.hxx>
#include <vcl/fpicker.hrc>
#include <vcl/menubtn.hxx>
#include <vcl/dialog.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>

#include <vector>

#include "fpdialogbase.hxx"
#include "fpsofficeResMgr.hxx"
#include "OfficeFilePicker.hrc"
#include "QueryFolderName.hxx"
#include "iodlg.hrc"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;

enum SvtRemoteDlgMode
{
    REMOTEDLG_MODE_OPEN = 0,
    REMOTEDLG_MODE_SAVE = 1
};

enum SvtRemoteDlgType
{
    REMOTEDLG_TYPE_FILEDLG = 0,
    REMOTEDLG_TYPE_PATHDLG = 1
};

typedef std::shared_ptr< Place > ServicePtr;
typedef ::com::sun::star::uno::Sequence< OUString >  OUStringList;

class FileViewContainer;

class RemoteFilesDialog : public SvtFileDialog_Base
{
public:
    RemoteFilesDialog( vcl::Window* pParent, WinBits nBits );
    virtual ~RemoteFilesDialog();

    virtual void dispose() SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
    virtual short Execute() SAL_OVERRIDE;
    virtual void Show();

    // SvtFileDialog_Base

    virtual SvtFileView* GetView() SAL_OVERRIDE;

    virtual void SetHasFilename( bool ) SAL_OVERRIDE;
    virtual void SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList ) SAL_OVERRIDE;
    virtual const ::com::sun::star::uno::Sequence< OUString >& GetBlackList() const SAL_OVERRIDE;
    virtual void SetStandardDir( const OUString& rStdDir ) SAL_OVERRIDE;
    virtual const OUString& GetStandardDir() const SAL_OVERRIDE;
    virtual void SetPath( const OUString& rNewURL ) SAL_OVERRIDE;
    virtual const OUString& GetPath() SAL_OVERRIDE;
    virtual std::vector<OUString> GetPathList() const SAL_OVERRIDE;
    virtual bool ContentIsFolder( const OUString& rURL ) SAL_OVERRIDE;
    virtual bool ContentIsDocument( const OUString& rURL );

    virtual OUString getCurrentFileText() const SAL_OVERRIDE;
    virtual void setCurrentFileText( const OUString& rText, bool bSelectAll = false ) SAL_OVERRIDE;

    virtual void AddFilter( const OUString& rFilter, const OUString& rType ) SAL_OVERRIDE;
    virtual void AddFilterGroup( const OUString& _rFilter,
                                const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& rFilters ) SAL_OVERRIDE;
    virtual OUString GetCurFilter() const SAL_OVERRIDE;
    virtual void SetCurFilter( const OUString& rFilter ) SAL_OVERRIDE;
    virtual void FilterSelect() SAL_OVERRIDE;

    virtual void SetFileCallback( ::svt::IFilePickerListener *pNotifier ) SAL_OVERRIDE;
    virtual void onAsyncOperationStarted() SAL_OVERRIDE;
    virtual void onAsyncOperationFinished() SAL_OVERRIDE;
    virtual void UpdateControls( const OUString& rURL ) SAL_OVERRIDE;

    virtual void EnableAutocompletion( bool ) SAL_OVERRIDE;

    virtual sal_Int32 getTargetColorDepth() SAL_OVERRIDE;
    virtual sal_Int32 getAvailableWidth() SAL_OVERRIDE;
    virtual sal_Int32 getAvailableHeight() SAL_OVERRIDE;

    virtual void setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& rImage ) SAL_OVERRIDE;

    virtual bool getShowState() SAL_OVERRIDE;

    virtual Control* getControl( sal_Int16 nControlId, bool bLabelControl = false ) const SAL_OVERRIDE;
    virtual void enableControl( sal_Int16 nControlId, bool bEnable ) SAL_OVERRIDE;
    virtual OUString getCurFilter( ) const SAL_OVERRIDE;

private:
    Reference< XComponentContext > m_xContext;
    Reference< XPasswordContainer2 > m_xMasterPasswd;

    SvtRemoteDlgMode m_eMode;
    SvtRemoteDlgType m_eType;
    bool m_bMultiselection;
    bool m_bIsUpdated;
    bool m_bIsConnected;
    bool m_bServiceChanged;

    OUString m_sIniKey;
    int m_nWidth;
    int m_nHeight;

    OUString m_sPath;
    OUString m_sStdDir;
    OUString m_sRootLabel;
    OUString m_sLastServiceUrl;
    unsigned int m_nCurrentFilter;

    ::rtl::Reference< ::svt::AsyncPickerAction > m_pCurrentAsyncAction;

    ::com::sun::star::uno::Sequence< OUString > m_aBlackList;
    ::svt::IFilePickerListener* m_pFileNotifier;

    VclPtr< PushButton > m_pOk_btn;
    VclPtr< CancelButton > m_pCancel_btn;
    VclPtr< MenuButton > m_pAddService_btn;
    VclPtr< ListBox > m_pServices_lb;
    VclPtr< Breadcrumb > m_pPath;
    VclPtr<PushButton> m_pNewFolder;
    VclPtr< Splitter > m_pSplitter;
    VclPtr< FolderTree > m_pTreeView;
    VclPtr< SvtFileView > m_pFileView;
    VclPtr< FileViewContainer > m_pContainer;
    VclPtr< ListBox > m_pFilter_lb;
    VclPtr< AutocompleteEdit > m_pName_ed;
    PopupMenu* m_pAddMenu;

    ImageList m_aImages;

    std::vector< ServicePtr > m_aServices;
    std::vector< std::pair< OUString, OUString > > m_aFilters;

    void InitSize();

    void FillServicesListbox();

    /* If failure returns < 0 */
    int GetSelectedServicePos();

    FileViewResult OpenURL( OUString const & sURL );

    void AddFileExtension();

    void EnableControls();
    void DisableControls();

    void SavePassword( const OUString& rURL, const OUString& rUser
                    , const OUString& rPassword, bool bPersistent );

    DECL_LINK_TYPED ( AddServiceHdl, Button*, void );
    DECL_LINK ( SelectServiceHdl, void * );
    DECL_LINK_TYPED ( EditServiceMenuHdl, MenuButton *, void );

    DECL_LINK_TYPED( DoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( SelectHdl, SvTreeListBox*, void );

    DECL_LINK_TYPED( FileNameGetFocusHdl, Control&, void );
    DECL_LINK( FileNameModifyHdl, void * );

    DECL_LINK_TYPED( SplitHdl, Splitter*, void );

    DECL_LINK( SelectFilterHdl, void * );

    DECL_LINK_TYPED( TreeSelectHdl, SvTreeListBox*, void );

    DECL_LINK_TYPED( SelectBreadcrumbHdl, Breadcrumb *, void );

    DECL_LINK_TYPED( NewFolderHdl, Button*, void );

    DECL_LINK_TYPED( OkHdl, Button*, void );
    DECL_LINK_TYPED( CancelHdl, Button*, void );
};

#endif // INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
