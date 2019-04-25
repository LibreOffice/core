/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>

#include "RemoteDialogClientBox.hxx"

namespace sd
{

class RemoteDialog : public ModalDialog
{
private:
    VclPtr<PushButton>     m_pButtonConnect;
    VclPtr<CloseButton>    m_pButtonClose;
    VclPtr<ClientBox>      m_pClientBox;

    DECL_LINK( HandleConnectButton, Button*, void );
    DECL_LINK( CloseHdl, SystemWindow&, void );
    DECL_LINK( CloseClickHdl, Button*, void );
public:
    explicit RemoteDialog( vcl::Window* pWindow );
    virtual ~RemoteDialog() override;
    virtual void dispose() override;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
