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

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>


#include <vcl/keycodes.hxx>
#include <vcl/event.hxx>
#include <sal/log.hxx>

#include <unx/salunx.h>
#include <unx/salinst.h>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salobj.h>

#include <salwtype.hxx>

// SalInstance member to create and destroy a SalObject

SalObject* X11SalInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow )
{
    return X11SalObject::CreateObject( pParent, pWindowData, bShow );
}

X11SalObject* X11SalObject::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow )
{
    int error_base, event_base;
    X11SalObject*       pObject  = new X11SalObject();
    SystemEnvData*    pObjData = const_cast<SystemEnvData*>(pObject->GetSystemData());

    if ( ! XShapeQueryExtension( static_cast<Display*>(pObjData->pDisplay),
                                  &event_base, &error_base ) )
    {
        delete pObject;
        return nullptr;
    }

    pObject->mpParent = pParent;

    SalDisplay* pSalDisp        = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    const SystemEnvData* pEnv   = pParent->GetSystemData();
    Display* pDisp              = pSalDisp->GetDisplay();
    ::Window aObjectParent      = static_cast<::Window>(pEnv->aWindow);
    pObject->maParentWin = aObjectParent;

    // find out on which screen that window is
    XWindowAttributes aParentAttr;
    XGetWindowAttributes( pDisp, aObjectParent, &aParentAttr );
    SalX11Screen nXScreen( XScreenNumberOfScreen( aParentAttr.screen ) );
    Visual* pVisual = (pWindowData && pWindowData->pVisual) ?
                      static_cast<Visual*>(pWindowData->pVisual) :
                      pSalDisp->GetVisual( nXScreen ).GetVisual();
    // get visual info
    VisualID aVisID = XVisualIDFromVisual( pVisual );
    XVisualInfo aTemplate;
    aTemplate.visualid = aVisID;
    int nVisuals = 0;
    XVisualInfo* pInfo = XGetVisualInfo( pDisp, VisualIDMask, &aTemplate, &nVisuals );
    // only one VisualInfo structure can match the visual id
    SAL_WARN_IF( nVisuals != 1, "vcl", "match count for visual id is not 1" );
    unsigned int nDepth     = pInfo->depth;
    XFree( pInfo );
    XSetWindowAttributes aAttribs;
    aAttribs.event_mask =   StructureNotifyMask
                          | ButtonPressMask
                          | ButtonReleaseMask
                          | PointerMotionMask
                          | EnterWindowMask
                          | LeaveWindowMask
                          | FocusChangeMask
                          | ExposureMask
                          ;

    pObject->maPrimary =
        XCreateSimpleWindow( pDisp,
                             aObjectParent,
                             0, 0,
                             1, 1, 0,
                             pSalDisp->GetColormap( nXScreen ).GetBlackPixel(),
                             pSalDisp->GetColormap( nXScreen ).GetWhitePixel()
                             );
    if( aVisID == pSalDisp->GetVisual( nXScreen ).GetVisualId() )
    {
        pObject->maSecondary =
            XCreateSimpleWindow( pDisp,
                                 pObject->maPrimary,
                                 0, 0,
                                 1, 1, 0,
                                 pSalDisp->GetColormap( nXScreen ).GetBlackPixel(),
                                 pSalDisp->GetColormap( nXScreen ).GetWhitePixel()
                                 );
    }
    else
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "visual id of vcl %x, of visual %x\n",
                 static_cast<unsigned int> (pSalDisp->GetVisual( nXScreen ).GetVisualId()),
                 static_cast<unsigned int> (aVisID) );
        #endif
        GetGenericUnixSalData()->ErrorTrapPush();

        // create colormap for visual - there might not be one
        pObject->maColormap = aAttribs.colormap = XCreateColormap(
            pDisp,
            pSalDisp->GetRootWindow( nXScreen ),
            pVisual,
            AllocNone );

        pObject->maSecondary =
            XCreateWindow( pDisp,
                           pSalDisp->GetRootWindow( nXScreen ),
                           0, 0,
                           1, 1, 0,
                           nDepth, InputOutput,
                           pVisual,
                           CWEventMask|CWColormap, &aAttribs );
        XSync( pDisp, False );
        if( GetGenericUnixSalData()->ErrorTrapPop( false ) )
        {
            pObject->maSecondary = None;
            delete pObject;
            return nullptr;
        }
        XReparentWindow( pDisp, pObject->maSecondary, pObject->maPrimary, 0, 0 );
    }

    GetGenericUnixSalData()->ErrorTrapPush();
    if( bShow ) {
        XMapWindow( pDisp, pObject->maSecondary );
        XMapWindow( pDisp, pObject->maPrimary );
    }

    pObjData->pDisplay      = pDisp;
    pObjData->aWindow       = pObject->maSecondary;
    pObjData->pWidget       = nullptr;
    pObjData->pVisual       = pVisual;

    XSync(pDisp, False);
    if( GetGenericUnixSalData()->ErrorTrapPop( false ) )
    {
        delete pObject;
        return nullptr;
    }

    return pObject;
}

void X11SalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}

// SalClipRegion is a member of SalObject
// definition of SalClipRegion my be found in vcl/inc/unx/salobj.h

SalClipRegion::SalClipRegion()
{
    ClipRectangleList = nullptr;
    numClipRectangles = 0;
    maxClipRectangles = 0;
}

SalClipRegion::~SalClipRegion()
{
}

void
SalClipRegion::BeginSetClipRegion( sal_uInt32 nRects )
{
    ClipRectangleList.reset( new XRectangle[nRects] );
    numClipRectangles = 0;
    maxClipRectangles = nRects;
}

void
SalClipRegion::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if ( nWidth && nHeight && (numClipRectangles < maxClipRectangles) )
    {
        XRectangle& aRect = ClipRectangleList[numClipRectangles];

        aRect.x     = static_cast<short>(nX);
        aRect.y     = static_cast<short>(nY);
        aRect.width = static_cast<unsigned short>(nWidth);
        aRect.height= static_cast<unsigned short>(nHeight);

        numClipRectangles++;
    }
}

// SalObject Implementation
X11SalObject::X11SalObject()
    : mpParent(nullptr)
    , maParentWin(0)
    , maPrimary(0)
    , maSecondary(0)
    , maColormap(0)
    , mbVisible(false)
{
    maSystemChildData.nSize     = sizeof( SystemEnvData );
    maSystemChildData.pDisplay  = vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetDisplay();
    maSystemChildData.aWindow       = None;
    maSystemChildData.pSalFrame = nullptr;
    maSystemChildData.pWidget       = nullptr;
    maSystemChildData.pVisual       = nullptr;
    maSystemChildData.aShellWindow  = 0;

    std::list< SalObject* >& rObjects = vcl_sal::getSalDisplay(GetGenericUnixSalData())->getSalObjects();
    rObjects.push_back( this );
}

X11SalObject::~X11SalObject()
{
    std::list< SalObject* >& rObjects = vcl_sal::getSalDisplay(GetGenericUnixSalData())->getSalObjects();
    rObjects.remove( this );

    GetGenericUnixSalData()->ErrorTrapPush();
    ::Window aObjectParent = maParentWin;
    XSetWindowBackgroundPixmap(static_cast<Display*>(maSystemChildData.pDisplay), aObjectParent, None);
    if ( maSecondary )
        XDestroyWindow( static_cast<Display*>(maSystemChildData.pDisplay), maSecondary );
    if ( maPrimary )
        XDestroyWindow( static_cast<Display*>(maSystemChildData.pDisplay), maPrimary );
    if ( maColormap )
        XFreeColormap(static_cast<Display*>(maSystemChildData.pDisplay), maColormap);
    XSync( static_cast<Display*>(maSystemChildData.pDisplay), False );
    GetGenericUnixSalData()->ErrorTrapPop();
}

void
X11SalObject::ResetClipRegion()
{
    maClipRegion.ResetClipRegion();

    const int   dest_kind   = ShapeBounding;
    const int   op          = ShapeSet;
    const int   ordering    = YSorted;

    XWindowAttributes win_attrib;
    XRectangle        win_size;

    ::Window aShapeWindow = maPrimary;

    XGetWindowAttributes ( static_cast<Display*>(maSystemChildData.pDisplay),
                           aShapeWindow,
                           &win_attrib );

    win_size.x      = 0;
    win_size.y      = 0;
    win_size.width  = win_attrib.width;
    win_size.height = win_attrib.height;

    XShapeCombineRectangles ( static_cast<Display*>(maSystemChildData.pDisplay),
                              aShapeWindow,
                              dest_kind,
                              0, 0,             // x_off, y_off
                              &win_size,        // list of rectangles
                              1,                // number of rectangles
                              op, ordering );
}

void
X11SalObject::BeginSetClipRegion( sal_uInt32 nRectCount )
{
    maClipRegion.BeginSetClipRegion ( nRectCount );
}

void
X11SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    maClipRegion.UnionClipRegion ( nX, nY, nWidth, nHeight );
}

void
X11SalObject::EndSetClipRegion()
{
    XRectangle *pRectangles = maClipRegion.EndSetClipRegion ();
    const int   nRectangles = maClipRegion.GetRectangleCount();

    ::Window aShapeWindow = maPrimary;

    XShapeCombineRectangles ( static_cast<Display*>(maSystemChildData.pDisplay),
                              aShapeWindow,
                              ShapeBounding,
                              0, 0, // x_off, y_off
                              pRectangles,
                              nRectangles,
                              ShapeSet, YSorted );
}

void
X11SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    if ( maPrimary && maSecondary && nWidth && nHeight )
    {
        XMoveResizeWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                           maPrimary,
                            nX, nY, nWidth, nHeight );
        XMoveResizeWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                           maSecondary,
                            0, 0, nWidth, nHeight );
    }
}

void
X11SalObject::Show( bool bVisible )
{
    if  ( ! maSystemChildData.aWindow )
        return;

    if ( bVisible ) {
        XMapWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                    maSecondary );
        XMapWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                    maPrimary );
    } else {
        XUnmapWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                      maPrimary );
        XUnmapWindow( static_cast<Display*>(maSystemChildData.pDisplay),
                      maSecondary );
    }
    mbVisible = bVisible;
}

void X11SalObject::GrabFocus()
{
    if( mbVisible )
         XSetInputFocus( static_cast<Display*>(maSystemChildData.pDisplay),
                         maSystemChildData.aWindow,
                         RevertToNone,
                         CurrentTime );
}

const SystemEnvData* X11SalObject::GetSystemData() const
{
    return &maSystemChildData;
}

static sal_uInt16 sal_GetCode( int state )
{
    sal_uInt16 nCode = 0;

    if( state & Button1Mask )
        nCode |= MOUSE_LEFT;
    if( state & Button2Mask )
        nCode |= MOUSE_MIDDLE;
    if( state & Button3Mask )
        nCode |= MOUSE_RIGHT;

    if( state & ShiftMask )
        nCode |= KEY_SHIFT;
    if( state & ControlMask )
        nCode |= KEY_MOD1;
    if( state & Mod1Mask )
        nCode |= KEY_MOD2;
    if( state & Mod3Mask )
        nCode |= KEY_MOD3;

    return nCode;
}

bool X11SalObject::Dispatch( XEvent* pEvent )
{
    std::list< SalObject* >& rObjects = vcl_sal::getSalDisplay(GetGenericUnixSalData())->getSalObjects();

    for (auto const& elem : rObjects)
    {
        X11SalObject* pObject = static_cast<X11SalObject*>(elem);
        if( pEvent->xany.window == pObject->maPrimary ||
            pEvent->xany.window == pObject->maSecondary )
        {
            if( pObject->IsMouseTransparent() && (
                    pEvent->type == ButtonPress     ||
                    pEvent->type == ButtonRelease   ||
                    pEvent->type == EnterNotify     ||
                    pEvent->type == LeaveNotify     ||
                    pEvent->type == MotionNotify
                    )
               )
            {
                SalMouseEvent aEvt;
                int dest_x, dest_y;
                ::Window aChild = None;
                XTranslateCoordinates( pEvent->xbutton.display,
                                       pEvent->xbutton.root,
                                       pObject->maParentWin,
                                       pEvent->xbutton.x_root,
                                       pEvent->xbutton.y_root,
                                       &dest_x, &dest_y,
                                       &aChild );
                aEvt.mnX        = dest_x;
                aEvt.mnY        = dest_y;
                aEvt.mnTime     = pEvent->xbutton.time;
                aEvt.mnCode     = sal_GetCode( pEvent->xbutton.state );
                aEvt.mnButton   = 0;
                SalEvent nEvent = SalEvent::NONE;
                if( pEvent->type == ButtonPress ||
                    pEvent->type == ButtonRelease )
                {
                    switch( pEvent->xbutton.button )
                    {
                        case Button1: aEvt.mnButton = MOUSE_LEFT;break;
                        case Button2: aEvt.mnButton = MOUSE_MIDDLE;break;
                        case Button3: aEvt.mnButton = MOUSE_RIGHT;break;
                    }
                    nEvent = (pEvent->type == ButtonPress) ?
                             SalEvent::MouseButtonDown :
                             SalEvent::MouseButtonUp;
                }
                else if( pEvent->type == EnterNotify )
                    nEvent = SalEvent::MouseLeave;
                else
                    nEvent = SalEvent::MouseMove;
                pObject->mpParent->CallCallback( nEvent, &aEvt );
            }
            else
            {
                switch( pEvent->type )
                {
                    case UnmapNotify:
                    pObject->mbVisible = false;
                    return true;
                    case MapNotify:
                    pObject->mbVisible = true;
                    return true;
                    case ButtonPress:
                    pObject->CallCallback( SalObjEvent::ToTop );
                    return true;
                    case FocusIn:
                    pObject->CallCallback( SalObjEvent::GetFocus );
                    return true;
                    case FocusOut:
                    pObject->CallCallback( SalObjEvent::LoseFocus );
                    return true;
                    default: break;
                }
            }
            return false;
        }
    }
    return false;
}

void X11SalObject::SetLeaveEnterBackgrounds(const css::uno::Sequence<css::uno::Any>& rLeaveArgs, const css::uno::Sequence<css::uno::Any>& rEnterArgs)
{
    SalDisplay* pSalDisp        = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    Display* pDisp              = pSalDisp->GetDisplay();
    ::Window aObjectParent      = maParentWin;

    bool bFreePixmap = false;
    Pixmap aPixmap = None;
    if (rEnterArgs.getLength() == 3)
    {
        rEnterArgs[0] >>= bFreePixmap;
        long pixmapHandle = None;
        rEnterArgs[1] >>= pixmapHandle;
        aPixmap = pixmapHandle;
    }

    XSetWindowBackgroundPixmap(pDisp, aObjectParent, aPixmap);
    if (bFreePixmap)
        XFreePixmap(pDisp, aPixmap);

    bFreePixmap = false;
    aPixmap = None;
    if (rLeaveArgs.getLength() == 3)
    {
        rLeaveArgs[0] >>= bFreePixmap;
        long pixmapHandle = None;
        rLeaveArgs[1] >>= pixmapHandle;
        aPixmap = pixmapHandle;
    }

    XSetWindowBackgroundPixmap(pDisp, maSecondary, aPixmap);
    if (bFreePixmap)
        XFreePixmap(pDisp, aPixmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
