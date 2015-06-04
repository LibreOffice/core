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

#include <vcl/button.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/vclptr.hxx>

#include <svtools/fileview.hxx>
#include <svtools/treelistentry.hxx>

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

#define WB_MULTISELECTION 0x20000000L

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

typedef std::shared_ptr<Place> ServicePtr;
typedef ::com::sun::star::uno::Sequence<OUString>  OUStringList;

class FileViewContainer;

class SVT_DLLPUBLIC RemoteFilesDialog : public ModalDialog
{
public:
    RemoteFilesDialog(vcl::Window* pParent, WinBits nBits);
    ~RemoteFilesDialog();

    virtual void dispose() SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;

private:
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_context;

    SvtRemoteDlgMode m_eMode;
    SvtRemoteDlgType m_eType;
    bool m_bMultiselection;
    bool m_bIsUpdated;

    VclPtr<PushButton> m_pOpen_btn;
    VclPtr<PushButton> m_pSave_btn;
    VclPtr<CancelButton> m_pCancel_btn;
    VclPtr<MenuButton> m_pAddService_btn;
    VclPtr<ListBox> m_pServices_lb;
    VclPtr<Edit> m_pPath_ed;
    VclPtr<SvtFileView> m_pFileView;
    VclPtr<FileViewContainer> m_pContainer;
    VclPtr<ListBox> m_pFilter_lb;
    VclPtr<Edit> m_pName_ed;

    std::vector<ServicePtr> m_aServices;

    void FillServicesListbox();

    /* If failure returns < 0 */
    int GetSelectedServicePos();

    OUString getCurrentFilter();

    void OpenURL( OUString sURL );

    DECL_LINK ( AddServiceHdl, void * );
    DECL_LINK ( SelectServiceHdl, void * );
    DECL_LINK_TYPED ( EditServiceMenuHdl, MenuButton *, void );

    DECL_LINK( DoubleClickHdl, void * );
    DECL_LINK( SelectHdl, void * );
};

#endif // INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
