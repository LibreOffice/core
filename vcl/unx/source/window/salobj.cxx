/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:47:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_SALOBJ_CXX
#include <prex.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <postx.h>

#include <salunx.h>
#include <salstd.hxx>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.h>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.h>
#endif
#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif

SalObjectList X11SalObject::aAllObjects;

// =======================================================================

long ImplSalObjCallbackDummy( void*, SalObject*, USHORT, const void* )
{
    return 0;
}

// =======================================================================
// SalInstance memberfkts to create and destroy a SalObject

SalObject* X11SalInstance::CreateObject( SalFrame* pParent )
{
    int error_base, event_base;
    X11SalObject*       pObject  = new X11SalObject();
    SystemChildData*    pObjData = const_cast<SystemChildData*>(pObject->GetSystemData());

    if ( ! XShapeQueryExtension( (Display*)pObjData->pDisplay,
                                  &event_base, &error_base ) )
    {
        delete pObject;
        return NULL;
    }

    SalDisplay* pSalDisp        = GetSalData()->GetDefDisp();
    const SystemEnvData* pEnv   = pParent->GetSystemData();
    Display* pDisp              = pSalDisp->GetDisplay();
    XLIB_Window aObjectParent   = (XLIB_Window)pEnv->aWindow;

    pObject->maPrimary =
        XCreateSimpleWindow( pDisp,
                             aObjectParent,
                             0, 0,
                             100, 100, 0,
                             pSalDisp->GetColormap().GetBlackPixel(),
                             pSalDisp->GetColormap().GetWhitePixel()
                             );
    pObject->maSecondary =
        XCreateSimpleWindow( pDisp,
                             pObject->maPrimary,
                             0, 0,
                             100, 100, 0,
                             pSalDisp->GetColormap().GetBlackPixel(),
                             pSalDisp->GetColormap().GetWhitePixel()
                             );
    XMapWindow( pDisp, pObject->maPrimary );
    XMapWindow( pDisp, pObject->maSecondary );

    pObjData->pDisplay      = pDisp;
    pObjData->aWindow       = pObject->maSecondary;
    pObjData->pWidget       = NULL;
    pObjData->pVisual       = pSalDisp->GetVisual()->GetVisual();
    pObjData->nDepth        = pSalDisp->GetVisual()->GetDepth();
    pObjData->aColormap     = pSalDisp->GetColormap().GetXColormap();
    pObjData->pAppContext   = NULL;
    XSync(pDisp, False);
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
        delete ClipRectangleList;
}


void
SalClipRegion::BeginSetClipRegion( ULONG nRects )
{
    if (ClipRectangleList)
        delete ClipRectangleList;

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
    maSystemChildData.nSize     = sizeof( SystemChildData );
    maSystemChildData.pDisplay  = GetSalData()->GetDefDisp()->GetDisplay();
    maSystemChildData.aWindow       = None;
    maSystemChildData.pSalFrame = 0;
    maSystemChildData.pWidget       = 0;
    maSystemChildData.pVisual       = 0;
    maSystemChildData.nDepth        = 0;
    maSystemChildData.aColormap = 0;
    maSystemChildData.pAppContext   = NULL;
    maSystemChildData.aShellWindow  = 0;
    maSystemChildData.pShellWidget  = NULL;

    mpInst                          = NULL;
    mpProc                          = ImplSalObjCallbackDummy;
    maPrimary                       = NULL;
    maSecondary                 = NULL;

    X11SalObject::aAllObjects.Insert( this, LIST_APPEND );
}


X11SalObject::~X11SalObject()
{
    X11SalObject::aAllObjects.Remove( this );
    if ( maSecondary )
        XDestroyWindow( (Display*)maSystemChildData.pDisplay, maSecondary );
    if ( maPrimary )
        XDestroyWindow( (Display*)maSystemChildData.pDisplay, maPrimary );
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
X11SalObject::BeginSetClipRegion( ULONG nRectCount )
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


USHORT
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
X11SalObject::Show( BOOL bVisible )
{
    if  ( ! maSystemChildData.aWindow )
        return;

    if ( bVisible )
        XMapWindow( (Display*)maSystemChildData.pDisplay,
                    maPrimary );
    else
        XUnmapWindow( (Display*)maSystemChildData.pDisplay,
                    maPrimary );

    mbVisible = bVisible;
}

// -----------------------------------------------------------------------

void X11SalObject::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void X11SalObject::GrabFocus()
{
    if( mbVisible )
         XSetInputFocus( (Display*)maSystemChildData.pDisplay,
                         maSystemChildData.aWindow,
                         RevertToNone,
                         CurrentTime );
}

// -----------------------------------------------------------------------

void X11SalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void X11SalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemChildData* X11SalObject::GetSystemData() const
{
    return &maSystemChildData;
}

long X11SalObject::Dispatch( XEvent* pEvent )
{
    for( int n= 0; n < aAllObjects.Count(); n++ )
    {
        X11SalObject* pObject = aAllObjects.GetObject( n );
        if( pEvent->xany.window == pObject->maPrimary ||
            pEvent->xany.window == pObject->maSecondary )
        {
            switch( pEvent->type )
            {
                case UnmapNotify:
                    pObject->mbVisible = FALSE;
                    return 1;
                case MapNotify:
                    pObject->mbVisible = TRUE;
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
            return 0;
        }
    }
    return 0;
}
