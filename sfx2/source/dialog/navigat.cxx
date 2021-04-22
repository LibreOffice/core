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

#include <sal/config.h>

#include <sfx2/bindings.hxx>
#include <sfx2/navigat.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/layout.hxx>
#include <helpids.h>
#include <tools/debug.hxx>

SfxNavigatorWrapper::SfxNavigatorWrapper(vcl::Window* pParentWnd, sal_uInt16 nId)
    : SfxChildWindow(pParentWnd , nId)
{
}

void SfxNavigatorWrapper::Initialize()
{
    SetHideNotDelete(true);
}

SfxNavigator::SfxNavigator(SfxBindings* pBind ,
                           SfxChildWindow* pChildWin ,
                           vcl::Window* pParent,
                           SfxChildWinInfo* pInfo)
                        : SfxDockingWindow(pBind ,
                                           pChildWin ,
                                           pParent ,
                                           "Navigator", "sfx/ui/navigator.ui")
{
    SetText(SfxResId(STR_SID_NAVIGATOR));
    SetHelpId(HID_NAVIGATOR_WINDOW);
    SetOutputSizePixel(Size(270, 240));
    Initialize(pInfo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
