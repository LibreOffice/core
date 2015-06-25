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

#include <svtools/svtdllapi.h>
#include <svtools/place.hxx>
#include <svtools/PlaceEditDialog.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/breadcrumb.hxx>
#include <svtools/fileview.hxx>
#include <svtools/treelistentry.hxx>

#include <vcl/button.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <ucbhelper/commandenvironment.hxx>

#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::comphelper;
using namespace ::svt;

#define WB_MULTISELECTION 0x20000000L

#define FILTER_ALL "*.*"

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
class Breadcrumb;
class FolderTree;

class SVT_DLLPUBLIC RemoteFilesDialog : public ModalDialog
{
public:
    RemoteFilesDialog( vcl::Window* pParent, WinBits nBits );
    virtual ~RemoteFilesDialog();

    virtual void dispose() SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;

    void AddFilter( OUString sName, OUString sType );

    OUString GetPath() const;

private:
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_context;

    SvtRemoteDlgMode m_eMode;
    SvtRemoteDlgType m_eType;
    bool m_bMultiselection;
    bool m_bIsUpdated;
    Image m_aFolderImage;

    OUString m_sPath;

    VclPtr< PushButton > m_pOpen_btn;
    VclPtr< PushButton > m_pSave_btn;
    VclPtr< CancelButton > m_pCancel_btn;
    VclPtr< MenuButton > m_pAddService_btn;
    VclPtr< ListBox > m_pServices_lb;
    VclPtr< Breadcrumb > m_pPath;
    VclPtr< Splitter > m_pSplitter;
    VclPtr< FolderTree > m_pTreeView;
    VclPtr< SvtFileView > m_pFileView;
    VclPtr< FileViewContainer > m_pContainer;
    VclPtr< ListBox > m_pFilter_lb;
    VclPtr< Edit > m_pName_ed;

    std::vector< ServicePtr > m_aServices;
    std::vector< OUString > m_aFilters;

    void FillServicesListbox();

    /* If failure returns < 0 */
    int GetSelectedServicePos();

    OUString GetCurrentFilter();

    FileViewResult OpenURL( OUString sURL );

    DECL_LINK ( AddServiceHdl, void * );
    DECL_LINK ( SelectServiceHdl, void * );
    DECL_LINK_TYPED ( EditServiceMenuHdl, MenuButton *, void );

    DECL_LINK( DoubleClickHdl, void * );
    DECL_LINK( SelectHdl, void * );

    DECL_LINK( SplitHdl, void * );

    DECL_LINK( SelectFilterHdl, void * );

    DECL_LINK( TreeSelectHdl, FolderTree * );

    DECL_LINK( SelectBreadcrumbHdl, Breadcrumb * );

    DECL_LINK( OkHdl, void * );
};

#endif // INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
