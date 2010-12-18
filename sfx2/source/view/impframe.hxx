/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SFX_IMPFRAME_HXX
#define _SFX_IMPFRAME_HXX

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>                  // SvBorder

class SfxViewFrame;
class SfxObjectShell;
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <cppuhelper/weak.hxx>

#include <sfx2/viewsh.hxx>
#include <sfx2/sfxuno.hxx>

#ifndef FRAME_SEARCH_PARENT
#define FRAME_SEARCH_PARENT     0x00000001
#define FRAME_SEARCH_SELF       0x00000002
#define FRAME_SEARCH_CHILDREN   0x00000004
#define FRAME_SEARCH_CREATE     0x00000008
#endif

class SfxFrame_Impl : public SfxBroadcaster, public SvCompatWeakBase
{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;
    sal_uInt32                nType;
    SfxViewFrame*             pCurrentViewFrame;
    SfxFrameDescriptor*       pDescr;
    sal_uInt16                nLocks;
    sal_Bool                  bClosing : 1;
    sal_Bool                  bPrepClosing : 1;
    sal_Bool                  bInCancelTransfers : 1;
    sal_Bool                  bOwnsBindings : 1;
    sal_Bool                  bReleasingComponent : 1;
    sal_Bool                  bInPlace : 1;
    SfxFrame*                 pFrame;
    SfxWorkWindow*            pWorkWin;
    SvBorder                  aBorder;
    // formerly SfxTopFrame
    Window*                   pExternalContainerWindow;
    bool                      bHidden;
    bool                      bLockResize;
    bool                      bMenuBarOn;

    SfxFrame_Impl( SfxFrame* pAntiImplP )
        :SvCompatWeakBase( pAntiImplP )
        ,nType( 0L )
        ,pCurrentViewFrame( NULL )
        ,pDescr( NULL )
        ,nLocks( 0 )
        ,bClosing(sal_False)
        ,bPrepClosing(sal_False)
        ,bInCancelTransfers( sal_False )
        ,bOwnsBindings( sal_False )
        ,bReleasingComponent( sal_False )
        ,bInPlace( sal_False )
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
