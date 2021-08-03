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

#pragma once

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

#include <tools/svborder.hxx>
#include <vcl/window.hxx>

class SfxFrame_Impl : public SfxBroadcaster
{
public:
    css::uno::Reference< css::frame::XFrame > xFrame;
    bool                      mbHasTitle;
    SfxViewFrame*             pCurrentViewFrame;
    SfxFrameDescriptor*       pDescr;
    bool                      bClosing : 1;
    bool                      bPrepClosing : 1;
    bool                      bInCancelTransfers : 1;
    bool                      bOwnsBindings : 1;
    bool                      bReleasingComponent : 1;
    bool                      bInPlace : 1;
    SfxWorkWindow*            pWorkWin;
    SvBorder                  aBorder;
    // formerly SfxTopFrame
    VclPtr<vcl::Window>       pExternalContainerWindow;
    bool                      bHidden;
    bool                      bLockResize;
    bool                      bMenuBarOn;

    explicit SfxFrame_Impl()
        :mbHasTitle( false )
        ,pCurrentViewFrame( nullptr )
        ,pDescr( nullptr )
        ,bClosing(false)
        ,bPrepClosing(false)
        ,bInCancelTransfers( false )
        ,bOwnsBindings( false )
        ,bReleasingComponent( false )
        ,bInPlace( false )
        ,pWorkWin( nullptr )
        ,pExternalContainerWindow( nullptr )
        ,bHidden( false )
        ,bLockResize( false )
        ,bMenuBarOn( true )
    {
    }
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
