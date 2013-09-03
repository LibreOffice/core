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

#include <prex.h>
#include <X11/extensions/shape.h>
#include <postx.h>
#include <tools/debug.hxx>

#include <vcl/keycodes.hxx>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/salinst.h>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salobj.h>

#include <salwtype.hxx>


// =======================================================================
// SalInstance member to create and destroy a SalObject

SalObject* X11SalInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow )
{
    return X11SalObject::CreateObject( pParent, pWindowData, bShow );
}

X11SalObject* X11SalObject::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow )
{
    int error_base, event_base;
    X11SalObject*       pObject  = new X11SalObject();
    SystemEnvData*    pObjData = const_cast<SystemEnvData*>(pObject->GetSystemData());

    if ( ! XShapeQueryExtension( (Display*)pObjData->pDisplay,
                                  &event_base, &error_base ) )
    {
        delete pObject;
        return NULL;
    }

    pObject->mpParent = pParent;

    SalDisplay* pSalDisp        = GetGenericData()->GetSalDisplay();
    const SystemEnvData* pEnv   = pParent->GetSystemData();
    Display* pDisp              = pSalDisp->GetDisplay();
    XLIB_Window aObjectParent   = (XLIB_Window)pEnv->aWindow;

    // find out on which screen that window is
    XWindowAttributes aParentAttr;
    XGetWindowAttributes( pDisp, aObjectParent, &aParentAttr );
    SalX11Screen nXScreen( XScreenNumberOfScreen( aParentAttr.screen ) );
    Visual* pVisual = (pWindowData && pWindowData->pVisual) ?
                      (Visual*)pWindowData->pVisual :
                      pSalDisp->GetVisual( nXScreen ).GetVisual();
    // get visual info
    VisualID aVisID = XVisualIDFromVisual( pVisual );
    XVisualInfo aTemplate;
    aTemplate.visualid = aVisID;
    int nVisuals = 0;
    XVisualInfo* pInfos = XGetVisualInfo( pDisp, VisualIDMask, &aTemplate, &nVisuals );
    // only one VisualInfo structure can match the visual id
    DBG_ASSERT( nVisuals == 1, "match count for visual id is not 1" );
    unsigned int nDepth     = pInfos->depth;
    XFree( pInfos );
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
        GetGenericData()->ErrorTrapPush();

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
        if( GetGenericData()->ErrorTrapPop( false ) )
        {
            pObject->maSecondary = None;
            delete pObject;
            return NULL;
        }
        XReparentWindow( pDisp, pObject->maSecondary, pObject->maPrimary, 0, 0 );
    }

    GetGenericData()->ErrorTrapPush();
    if( bShow ) {
        XMapWindow( pDisp, pObject->maSecondary );
        XMapWindow( pDisp, pObject->maPrimary );
    }

    pObjData->pDisplay      = pDisp;
    pObjData->aWindow       = pObject->maSecondary;
    pObjData->pWidget       = NULL;
    pObjData->pVisual       = pVisual;
    pObjData->nDepth        = nDepth;
    pObjData->aColormap     = aVisID == pSalDisp->GetVisual( nXScreen ).GetVisualId() ?
                              pSalDisp->GetColormap( nXScreen ).GetXColormap() : None;
    pObjData->pAppContext   = NULL;

    XSync(pDisp, False);
    if( GetGenericData()->ErrorTrapPop( false ) )
    {
        delete pObject;
        return NULL;
    }

    return pObject;
}


void X11SalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}


// ======================================================================
// SalClipRegion is a member of SalObject
// definition of SalClipRegion my be found in unx/inc/salobj.h


SalClipRegion::SalClipRegion()
{
    ClipRectangleList = NULL;
    numClipRectangles = 0;
    maxClipRectangles = 0;
    nClipRegionType   = SAL_OBJECT_CLIP_INCLUDERECTS;
}


SalClipRegion::~SalClipRegion()
{
    if ( ClipRectangleList )
        delete [] ClipRectangleList;
}


void
SalClipRegion::BeginSetClipRegion( sal_uLong nRects )
{
    if (ClipRectangleList)
        delete [] ClipRectangleList;

    ClipRectangleList = new XRectangle[nRects];
    numClipRectangles = 0;
    maxClipRectangles = nRects;
}


void
SalClipRegion::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if ( nWidth && nHeight && (numClipRectangles < maxClipRectangles) )
    {
        XRectangle *aRect = ClipRectangleList + numClipRectangles;

        aRect->x     = (short) nX;
        aRect->y     = (short) nY;
        aRect->width = (unsigned short) nWidth;
        aRect->height= (unsigned short) nHeight;

        numClipRectangles++;
    }
}


// =======================================================================
// SalObject Implementation


X11SalObject::X11SalObject()
{
    maSystemChildData.nSize     = sizeof( SystemEnvData );
    maSystemChildData.pDisplay  = GetGenericData()->GetSalDisplay()->GetDisplay();
    maSystemChildData.aWindow       = None;
    maSystemChildData.pSalFrame = 0;
    maSystemChildData.pWidget       = 0;
    maSystemChildData.pVisual       = 0;
    maSystemChildData.nDepth        = 0;
    maSystemChildData.aColormap = 0;
    maSystemChildData.pAppContext   = NULL;
    maSystemChildData.aShellWindow  = 0;
    maSystemChildData.pShellWidget  = NULL;
    maPrimary                       = 0;
    maSecondary                     = 0;
    maColormap                      = 0;

    std::list< SalObject* >& rObjects = GetGenericData()->GetSalDisplay()->getSalObjects();
    rObjects.push_back( this );
}


X11SalObject::~X11SalObject()
{
    std::list< SalObject* >& rObjects = GetGenericData()->GetSalDisplay()->getSalObjects();
    rObjects.remove( this );

    GetGenericData()->ErrorTrapPush();
    if ( maSecondary )
        XDestroyWindow( (Display*)maSystemChildData.pDisplay, maSecondary );
    if ( maPrimary )
        XDestroyWindow( (Display*)maSystemChildData.pDisplay, maPrimary );
    if ( maColormap )
        XFreeColormap((Display*)maSystemChildData.pDisplay, maColormap);
    XSync( (Display*)maSystemChildData.pDisplay, False );
    GetGenericData()->ErrorTrapPop();
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

    XLIB_Window aShapeWindow = maPrimary;

    XGetWindowAttributes ( (Display*)maSystemChildData.pDisplay,
                           aShapeWindow,
                           &win_attrib );

    win_size.x      = 0;
    win_size.y      = 0;
    win_size.width  = win_attrib.width;
    win_size.height = win_attrib.height;

    XShapeCombineRectangles ( (Display*)maSystemChildData.pDisplay,
                              aShapeWindow,
                              dest_kind,
                              0, 0,             // x_off, y_off
                              &win_size,        // list of rectangles
                              1,                // number of rectangles
                              op, ordering );
}


void
X11SalObject::BeginSetClipRegion( sal_uLong nRectCount )
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
    const int   nType       = maClipRegion.GetClipRegionType();
    const int   nRectangles = maClipRegion.GetRectangleCount();

    const int   dest_kind   = ShapeBounding;
    const int   ordering    = YSorted;
    int         op;

    switch ( nType )
    {
        case SAL_OBJECT_CLIP_INCLUDERECTS :
            op = ShapeSet;
            break;
        case SAL_OBJECT_CLIP_EXCLUDERECTS :
            op = ShapeSubtract;
            break;
        case SAL_OBJECT_CLIP_ABSOLUTE :
            op = ShapeSet;
            break;
        default :
            op = ShapeUnion;
    }

    XLIB_Window aShapeWindow = maPrimary;

    XShapeCombineRectangles ( (Display*)maSystemChildData.pDisplay,
                              aShapeWindow,
                              dest_kind,
                              0, 0, // x_off, y_off
                              pRectangles,
                              nRectangles,
                              op, ordering );
}


sal_uInt16
X11SalObject::GetClipRegionType()
{
    return maClipRegion.GetClipRegionType();
}

// -----------------------------------------------------------------------

void
X11SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    if ( maPrimary && maSecondary && nWidth && nHeight )
    {
        XMoveResizeWindow( (Display*)maSystemChildData.pDisplay,
                           maPrimary,
                            nX, nY, nWidth, nHeight );
        XMoveResizeWindow( (Display*)maSystemChildData.pDisplay,
                           maSecondary,
                            0, 0, nWidth, nHeight );
    }
}


void
X11SalObject::Show( sal_Bool bVisible )
{
    if  ( ! maSystemChildData.aWindow )
        return;

    if ( bVisible ) {
        XMapWindow( (Display*)maSystemChildData.pDisplay,
                    maSecondary );
        XMapWindow( (Display*)maSystemChildData.pDisplay,
                    maPrimary );
    } else {
        XUnmapWindow( (Display*)maSystemChildData.pDisplay,
                      maPrimary );
        XUnmapWindow( (Display*)maSystemChildData.pDisplay,
                      maSecondary );
    }
    mbVisible = bVisible;
}

void X11SalObject::GrabFocus()
{
    if( mbVisible )
         XSetInputFocus( (Display*)maSystemChildData.pDisplay,
                         maSystemChildData.aWindow,
                         RevertToNone,
                         CurrentTime );
}

// -----------------------------------------------------------------------

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

long X11SalObject::Dispatch( XEvent* pEvent )
{
    std::list< SalObject* >& rObjects = GetGenericData()->GetSalDisplay()->getSalObjects();

    for( std::list< SalObject* >::iterator it = rObjects.begin(); it != rObjects.end(); ++it )
    {
        X11SalObject* pObject = static_cast<X11SalObject*>(*it);
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
                const SystemEnvData* pParentData = pObject->mpParent->GetSystemData();
                int dest_x, dest_y;
                XLIB_Window aChild = None;
                XTranslateCoordinates( pEvent->xbutton.display,
                                       pEvent->xbutton.root,
                                       pParentData->aWindow,
                                       pEvent->xbutton.x_root,
                                       pEvent->xbutton.y_root,
                                       &dest_x, &dest_y,
                                       &aChild );
                aEvt.mnX        = dest_x;
                aEvt.mnY        = dest_y;
                aEvt.mnTime     = pEvent->xbutton.time;
                aEvt.mnCode     = sal_GetCode( pEvent->xbutton.state );
                aEvt.mnButton   = 0;
                sal_uInt16 nEvent = 0;
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
                             SALEVENT_MOUSEBUTTONDOWN :
                             SALEVENT_MOUSEBUTTONUP;
                }
                else if( pEvent->type == EnterNotify )
                    nEvent = SALEVENT_MOUSELEAVE;
                else
                    nEvent = SALEVENT_MOUSEMOVE;
                pObject->mpParent->CallCallback( nEvent, &aEvt );
            }
            else
            {
                switch( pEvent->type )
                {
                    case UnmapNotify:
                    pObject->mbVisible = sal_False;
                    return 1;
                    case MapNotify:
                    pObject->mbVisible = sal_True;
                    return 1;
                    case ButtonPress:
                    pObject->CallCallback( SALOBJ_EVENT_TOTOP, NULL );
                    return 1;
                    case FocusIn:
                    pObject->CallCallback( SALOBJ_EVENT_GETFOCUS, NULL );
                    return 1;
                    case FocusOut:
                    pObject->CallCallback( SALOBJ_EVENT_LOSEFOCUS, NULL );
                    return 1;
                    default: break;
                }
            }
            return 0;
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
