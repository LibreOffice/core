/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/toolkit/menubtn.hxx>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

class ManagedMenuButton : public MenuButton
{
public:
    ManagedMenuButton(vcl::Window* pParent, WinBits nStyle);
    ~ManagedMenuButton() override;

    void Activate() override;
    void dispose() override;

private:
    css::uno::Reference<css::awt::XPopupMenu> m_xPopupMenu;
    css::uno::Reference<css::frame::XPopupMenuController> m_xPopupController;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
