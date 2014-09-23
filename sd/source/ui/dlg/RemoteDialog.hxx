/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOG_HXX
#define INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOG_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <svtools/treelistbox.hxx>

#include "RemoteDialogClientBox.hxx"

namespace sd
{

class RemoteDialog : public ModalDialog
{
private:
    PushButton*     m_pButtonConnect;
    CloseButton*    m_pButtonClose;
    ClientBox*      m_pClientBox;

    DECL_DLLPRIVATE_LINK( HandleConnectButton, void * );
    DECL_LINK( CloseHdl, void * );
public:
    RemoteDialog( vcl::Window* pWindow );
};

}

#endif // INCLUDED_SD_SOURCE_UI_DLG_REMOTEDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
