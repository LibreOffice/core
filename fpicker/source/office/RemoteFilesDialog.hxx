/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "autocmpledit.hxx"
#include <svtools/place.hxx>

#include <unotools/viewoptions.hxx>

#include <vcl/svapp.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/task/XPasswordContainer2.hpp>

#include <vector>

#include "asyncfilepicker.hxx"
#include "fpdialogbase.hxx"
#include "breadcrumb.hxx"
#include "fileview.hxx"
#include "foldertree.hxx"

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

class RemoteFilesDialog : public SvtFileDialog_Base
{
public:
    RemoteFilesDialog( weld::Window* pParent, PickerFlags nBits );
    virtual ~RemoteFilesDialog() override;

    virtual short run() override;

    // SvtFileDialog_Base

    virtual SvtFileView* GetView() override;

    virtual void SetHasFilename( bool ) override;
    virtual void SetDenyList( const css::uno::Sequence< OUString >& rDenyList ) override;
    virtual const css::uno::Sequence< OUString >& GetDenyList() const override;
    virtual void SetStandardDir( const OUString& rStdDir ) override;
    virtual const OUString& GetStandardDir() const override;
    virtual void SetPath( const OUString& rNewURL ) override;
    virtual const OUString& GetPath() override;
    virtual std::vector<OUString> GetPathList() const override;
    virtual bool ContentIsFolder( const OUString& rURL ) override;
    bool ContentIsDocument( const OUString& rURL );

    virtual OUString getCurrentFileText() const override;
    virtual void setCurrentFileText( const OUString& rText, bool bSelectAll = false ) override;

    virtual void AddFilter( const OUString& rFilter, const OUString& rType ) override;
    virtual void AddFilterGroup( const OUString& _rFilter,
                                const css::uno::Sequence< css::beans::StringPair >& rFilters ) override;
    virtual OUString GetCurFilter() const override;
    virtual void SetCurFilter( const OUString& rFilter ) override;
    virtual void FilterSelect() override;

    virtual void SetFileCallback( ::svt::IFilePickerListener *pNotifier ) override;
    virtual void onAsyncOperationStarted() override;
    virtual void onAsyncOperationFinished() override;
    virtual void UpdateControls( const OUString& rURL ) override;

    virtual void EnableAutocompletion( bool = true) override;

    virtual sal_Int32 getAvailableWidth() override;
    virtual sal_Int32 getAvailableHeight() override;

    virtual void setImage( const css::uno::Any& rImage ) override;

    virtual bool getShowState() override;

    virtual weld::Widget* getControl( sal_Int16 nControlId, bool bLabelControl = false ) const override;
    virtual void enableControl( sal_Int16 nControlId, bool bEnable ) override;
    virtual OUString getCurFilter( ) const override;

private:
    Reference< XComponentContext > m_xContext;
    Reference< XPasswordContainer2 > m_xMasterPasswd;

    SvtRemoteDlgMode m_eMode;
    SvtRemoteDlgType m_eType;
    bool m_bIsUpdated;
    bool m_bIsConnected;
    bool m_bServiceChanged;

    OUString m_sIniKey;

    bool m_bIsInExecute;

    OUString m_sPath;
    OUString m_sStdDir;
    OUString m_sRootLabel;
    OUString m_sLastServiceUrl;
    int m_nCurrentFilter;

    ::rtl::Reference< ::svt::AsyncPickerAction > m_pCurrentAsyncAction;

    css::uno::Sequence< OUString > m_aDenyList;

    std::unique_ptr<weld::Button> m_xOk_btn;
    std::unique_ptr<weld::Button> m_xCancel_btn;
    std::unique_ptr<weld::MenuButton> m_xManageServices;
    std::unique_ptr<weld::ComboBox> m_xServices_lb;
    std::unique_ptr<weld::Container> m_xPathContainer;
    std::unique_ptr<Breadcrumb> m_xPath;
    std::unique_ptr<weld::Button> m_xNewFolder;
    std::unique_ptr<weld::ToggleButton> m_xListView_btn;
    std::unique_ptr<weld::ToggleButton> m_xIconView_btn;
    std::unique_ptr<FolderTree> m_xTreeView;
    std::unique_ptr<SvtFileView> m_xFileView;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::ComboBox> m_xFilter_lb;
    std::unique_ptr<AutocompleteEdit> m_xName_ed;

    std::vector< ServicePtr > m_aServices;
    std::vector< std::pair< OUString, OUString > > m_aFilters;

    void InitSize();

    void FillServicesListbox();

    /* If failure returns < 0 */
    int GetSelectedServicePos();

    void OpenURL( OUString const & sURL );

    void AddFileExtension();

    void EnableExtraMenuItems(bool bEnable);
    void EnableControls();
    void DisableControls();

    void SavePassword(const OUString& rURL, const OUString& rUser,
                      const OUString& rPassword, bool bPersistent);

    void AddService();

    DECL_LINK ( SelectServiceHdl, weld::ComboBox&, void );
    DECL_LINK ( EditServiceMenuHdl, const OString&, void );

    DECL_LINK( DoubleClickHdl, SvtFileView*, bool );
    DECL_LINK( SelectHdl, SvtFileView*, void );

    DECL_LINK( FileNameGetFocusHdl, weld::Widget&, void );
    DECL_LINK( FileNameModifyHdl, weld::Entry&, void );

    DECL_LINK( SelectFilterHdl, weld::ComboBox&, void );

    DECL_LINK( TreeSelectHdl, weld::TreeView&, void );

    DECL_LINK( SelectBreadcrumbHdl, Breadcrumb*, bool );

    DECL_LINK( NewFolderHdl, weld::Button&, void );
    DECL_LINK( IconViewHdl, weld::Button&, void );
    DECL_LINK( ListViewHdl, weld::Button&, void );

    DECL_LINK( OkHdl, weld::Button&, void );
    DECL_LINK( CancelHdl, weld::Button&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
