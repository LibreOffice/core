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

#include <officecfg/Office/Common.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

enum SvtRemoteDlgMode
{
    REMOTEDLG_MODE_OPEN = 0,
    REMOTEDLG_MODE_SAVE = 1
};

typedef std::shared_ptr<Place> ServicePtr;

class SVT_DLLPUBLIC RemoteFilesDialog : public ModalDialog
{
public:
    RemoteFilesDialog(vcl::Window* pParent, WinBits nBits);

private:
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_context;

    SvtRemoteDlgMode m_eMode;

    VclPtr<PushButton> m_pOpen_btn;
    VclPtr<PushButton> m_pSave_btn;
    VclPtr<CancelButton> m_pCancel_btn;
    VclPtr<MenuButton> m_pAddService_btn;
    VclPtr<ListBox> m_pServices_lb;

    std::vector<ServicePtr> m_aServices;

    void fillServicesListbox();

    DECL_LINK ( AddServiceHdl, void * );
    DECL_LINK_TYPED ( EditServiceMenuHdl, MenuButton *, void );
};

#endif // INCLUDED_SVTOOLS_REMOTEFILESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
