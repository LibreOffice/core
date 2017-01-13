/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/popupmenuwindow.hxx>

#include <limits>

struct PopupMenuFloatingWindow::ImplData
{
    sal_uInt16      mnMenuStackLevel;  // Store the stack level of a popup menu.  0 = top-level menu.

    ImplData();
};

PopupMenuFloatingWindow::ImplData::ImplData() :
    mnMenuStackLevel( ::std::numeric_limits<sal_uInt16>::max() )
{
}

PopupMenuFloatingWindow::PopupMenuFloatingWindow( vcl::Window* pParent ) :
    FloatingWindow(pParent, WB_SYSTEMFLOATWIN | WB_SYSTEMWINDOW | WB_NOBORDER ),
    mpImplData(new ImplData)
{
}

PopupMenuFloatingWindow::~PopupMenuFloatingWindow()
{
    disposeOnce();
}

void PopupMenuFloatingWindow::dispose()
{
    delete mpImplData;
    FloatingWindow::dispose();
}

sal_uInt16 PopupMenuFloatingWindow::GetMenuStackLevel() const
{
    return mpImplData->mnMenuStackLevel;
}

void PopupMenuFloatingWindow::SetMenuStackLevel( sal_uInt16 nLevel )
{
    mpImplData->mnMenuStackLevel = nLevel;
}

bool PopupMenuFloatingWindow::IsPopupMenu() const
{
    return mpImplData->mnMenuStackLevel != ::std::numeric_limits<sal_uInt16>::max();
}

bool PopupMenuFloatingWindow::isPopupMenu(const vcl::Window *pWindow)
{
    const PopupMenuFloatingWindow* pChild = dynamic_cast<const PopupMenuFloatingWindow*>(pWindow);
    return pChild && pChild->IsPopupMenu();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
