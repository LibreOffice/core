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

#include <vcl/window.hxx>
#include <vcl/waitobj.hxx>
#include <window.h>
#include <vcl/cursor.hxx>

WaitObject::WaitObject(vcl::Window* pWindow)
    : mpWindow(pWindow)
{
    if (mpWindow)
        mpWindow->EnterWait();
}

WaitObject::~WaitObject()
{
    if (mpWindow)
        mpWindow->LeaveWait();
}

namespace vcl
{
Size Window::GetOptimalSize() const { return Size(); }

void Window::ImplAdjustNWFSizes()
{
    for (Window* pWin = GetWindow(GetWindowType::FirstChild); pWin;
         pWin = pWin->GetWindow(GetWindowType::Next))
        pWin->ImplAdjustNWFSizes();
}

void Window::ImplClearFontData(bool bNewFontLists)
{
    OutputDevice::ImplClearFontData(bNewFontLists);
    for (Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext)
        pChild->ImplClearFontData(bNewFontLists);
}

void Window::ImplRefreshFontData(bool bNewFontLists)
{
    OutputDevice::ImplRefreshFontData(bNewFontLists);
    for (Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext)
        pChild->ImplRefreshFontData(bNewFontLists);
}

void Window::ImplInitMapModeObjects()
{
    OutputDevice::ImplInitMapModeObjects();
    if (mpWindowImpl->mpCursor)
        mpWindowImpl->mpCursor->ImplNew();
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
