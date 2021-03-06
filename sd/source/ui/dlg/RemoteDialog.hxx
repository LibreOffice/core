/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/weld.hxx>

#include "RemoteDialogClientBox.hxx"

namespace sd
{
class RemoteDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Button> m_xButtonConnect;
    std::unique_ptr<ClientBox> m_xClientBox;

    DECL_LINK(HandleConnectButton, weld::Button&, void);

public:
    explicit RemoteDialog(weld::Window* pWindow);
    virtual short run() override;
    virtual ~RemoteDialog() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
