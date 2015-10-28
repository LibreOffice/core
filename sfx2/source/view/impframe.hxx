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

#ifndef INCLUDED_SFX2_SOURCE_VIEW_IMPFRAME_HXX
#define INCLUDED_SFX2_SOURCE_VIEW_IMPFRAME_HXX

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

class SfxViewFrame;
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <cppuhelper/weak.hxx>

#include <sfx2/viewsh.hxx>
#include <sfx2/sfxuno.hxx>

class SfxFrame_Impl : public SfxBroadcaster
{
public:
    css::uno::Reference< css::frame::XFrame > xFrame;
    sal_uInt32                nType;
    SfxViewFrame*             pCurrentViewFrame;
    SfxFrameDescriptor*       pDescr;
    sal_uInt16                nLocks;
    bool                      bClosing : 1;
    bool                      bPrepClosing : 1;
    bool                      bInCancelTransfers : 1;
    bool                      bOwnsBindings : 1;
    bool                      bReleasingComponent : 1;
    bool                      bInPlace : 1;
    SfxFrame*                 pFrame;
    SfxWorkWindow*            pWorkWin;
    SvBorder                  aBorder;
    // formerly SfxTopFrame
    VclPtr<vcl::Window>       pExternalContainerWindow;
    bool                      bHidden;
    bool                      bLockResize;
    bool                      bMenuBarOn;

    explicit SfxFrame_Impl( SfxFrame* pAntiImplP )
        :nType( 0L )
        ,pCurrentViewFrame( NULL )
        ,pDescr( NULL )
        ,nLocks( 0 )
        ,bClosing(false)
        ,bPrepClosing(false)
        ,bInCancelTransfers( false )
        ,bOwnsBindings( false )
        ,bReleasingComponent( false )
        ,bInPlace( false )
        ,pFrame( pAntiImplP )
        ,pWorkWin( 0 )
        ,pExternalContainerWindow( NULL )
        ,bHidden( false )
        ,bLockResize( false )
        ,bMenuBarOn( true )
    {
    }

    virtual ~SfxFrame_Impl() { }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
