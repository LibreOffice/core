/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2001-02-22 14:13:05 $
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
#ifdef USE_MOTIF
#include <Xm/DrawingA.h>
#else
#include <X11/Xaw/Label.h>
#endif
#include <postx.h>

#include <salunx.h>
#include <salstd.hxx>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif

#ifdef USE_MOTIF
#define OBJECT_WIDGET_CLASS xmDrawingAreaWidgetClass
#else
#define OBJECT_WIDGET_CLASS labelWidgetClass
#endif

SalObjectList SalObjectData::aAllObjects;

// =======================================================================

long ImplSalObjCallbackDummy( void*, SalObject*, USHORT, const void* )
{
    return 0;
}

// =======================================================================
// SalInstance memberfkts to create and destroy a SalObject

SalObject* SalInstance::CreateObject( SalFrame* pParent )
{
    int error_base, event_base;
    SalObject*       pObject  = new SalObject;
    SystemChildData* pObjData = const_cast<SystemChildData*>(pObject->GetSystemData());

    if ( ! XShapeQueryExtension( (Display*)pObjData->pDisplay,
                                  &event_base, &error_base ) )
    {
        delete pObject;
        return NULL;
    }

    SalDisplay* pSalDisp = pParent->maFrameData.GetDisplay();
    Widget pWidgetParent = pParent->maFrameData.GetWidget();

    pObject->maObjectData.maPrimary =
        XtVaCreateWidget( "salobject primary",
                          OBJECT_WIDGET_CLASS,
                          pWidgetParent,
                          NULL );
    pObject->maObjectData.maSecondary =
        XtVaCreateWidget( "salobject secondary",
                          OBJECT_WIDGET_CLASS,
                          pObject->maObjectData.maPrimary,
                          NULL );
    XtRealizeWidget( pObject->maObjectData.maPrimary );
    XtRealizeWidget( pObject->maObjectData.maSecondary );

    pObjData->pDisplay      = XtDisplay( pObject->maObjectData.maPrimary );
    pObjData->aWindow       = XtWindow( pObject->maObjectData.maSecondary );
    pObjData->pWidget       = pObject->maObjectData.maSecondary;
    pObjData->pVisual       = pSalDisp->GetVisual()->GetVisual();
    pObjData->nDepth        = pSalDisp->GetVisual()->GetDepth();
    pObjData->aColormap     = pSalDisp->GetColormap().GetXColormap();
    pObjData->pAppContext   = pSalDisp->GetXLib()->GetAppContext();
    return pObject;
}


void SalInstance::DestroyObject( SalObject* pObject )
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


SalObject::SalObject()
{
    maObjectData.maSystemChildData.nSize        = sizeof( SystemChildData );
    maObjectData.maSystemChildData.pDisplay     = GetSalData()->GetDefDisp()->GetDisplay();
    maObjectData.maSystemChildData.aWindow      = None;
    maObjectData.maSystemChildData.pSalFrame    = 0;
    maObjectData.maSystemChildData.pWidget      = 0;
    maObjectData.maSystemChildData.pVisual      = 0;
    maObjectData.maSystemChildData.nDepth       = 0;
    maObjectData.maSystemChildData.aColormap    = 0;
    maObjectData.maSystemChildData.pAppContext  = NULL;
    maObjectData.maSystemChildData.aShellWindow = 0;
    maObjectData.maSystemChildData.pShellWidget = NULL;

    maObjectData.mpInst                         = NULL;
    maObjectData.mpProc                         = ImplSalObjCallbackDummy;
    maObjectData.maPrimary                      = NULL;
    maObjectData.maSecondary                    = NULL;

    SalObjectData::aAllObjects.Insert( this, LIST_APPEND );
}


SalObject::~SalObject()
{
    SalObjectData::aAllObjects.Remove( this );
    if ( maObjectData.maPrimary )
        XtDestroyWidget ( maObjectData.maPrimary );
    if ( maObjectData.maSecondary )
        XtDestroyWidget ( maObjectData.maSecondary );
}


void
SalObject::ResetClipRegion()
{
    maObjectData.maClipRegion.ResetClipRegion();

    const int   dest_kind   = ShapeBounding;
    const int   op          = ShapeSet;
    const int   ordering    = YSorted;

    XWindowAttributes win_attrib;
    XRectangle        win_size;

    XGetWindowAttributes ( (Display*)maObjectData.maSystemChildData.pDisplay,
                           maObjectData.maSystemChildData.aWindow,
                           &win_attrib );

    win_size.x      = 0;
    win_size.y      = 0;
    win_size.width  = win_attrib.width;
    win_size.height = win_attrib.width;

    XShapeCombineRectangles ( (Display*)maObjectData.maSystemChildData.pDisplay,
                              maObjectData.maSystemChildData.aWindow,
                              dest_kind,
                              0, 0,             // x_off, y_off
                              &win_size,        // list of rectangles
                              1,                // number of rectangles
                              op, ordering );
}


void
SalObject::BeginSetClipRegion( ULONG nRectCount )
{
    maObjectData.maClipRegion.BeginSetClipRegion ( nRectCount );
}


void
SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    maObjectData.maClipRegion.UnionClipRegion ( nX, nY, nWidth, nHeight );
}


void
SalObject::EndSetClipRegion()
{
    XRectangle *pRectangles = maObjectData.maClipRegion.EndSetClipRegion ();
    const int   nType       = maObjectData.maClipRegion.GetClipRegionType();
    const int   nRectangles = maObjectData.maClipRegion.GetRectangleCount();

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

    XShapeCombineRectangles ( (Display*)maObjectData.maSystemChildData.pDisplay,
                              maObjectData.maSystemChildData.aWindow,
                              dest_kind,
                              0, 0, // x_off, y_off
                              pRectangles,
                              nRectangles,
                              op, ordering );
}


USHORT
SalObject::GetClipRegionType()
{
    return maObjectData.maClipRegion.GetClipRegionType();
}

// -----------------------------------------------------------------------

void
SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    if ( maObjectData.maPrimary && maObjectData.maSecondary && nWidth && nHeight )
    {
        XtConfigureWidget( maObjectData.maPrimary,
                            nX, nY, nWidth, nHeight, 0 );
        XtConfigureWidget( maObjectData.maSecondary,
                            0, 0, nWidth, nHeight, 0 );
    }
}


void
SalObject::Show( BOOL bVisible )
{
    if  ( ! maObjectData.maSystemChildData.aWindow )
        return;

    if ( bVisible )
        XtMapWidget( (Widget)maObjectData.maPrimary );
    else
        XtUnmapWidget( (Widget)maObjectData.maPrimary );

    maObjectData.mbVisible = bVisible;
}

// -----------------------------------------------------------------------

void SalObject::Enable( BOOL bEnable )
{
}

// -----------------------------------------------------------------------

void SalObject::GrabFocus()
{
    if( maObjectData.mbVisible )
         XSetInputFocus( (Display*)maObjectData.maSystemChildData.pDisplay,
                         maObjectData.maSystemChildData.aWindow,
                         RevertToNone,
                         CurrentTime );
}

// -----------------------------------------------------------------------

void SalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void SalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemChildData* SalObject::GetSystemData() const
{
    return &maObjectData.maSystemChildData;
}

// -----------------------------------------------------------------------

void SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
      maObjectData.mpInst = pInst;
      if ( pProc )
          maObjectData.mpProc = pProc;
      else
          maObjectData.mpProc = ImplSalObjCallbackDummy;
}

long SalObjectData::Dispatch( XEvent* pEvent )
{
    for( int n= 0; n < aAllObjects.Count(); n++ )
    {
        SalObject* pObject = aAllObjects.GetObject( n );
        if( pEvent->xany.window == XtWindow( pObject->maObjectData.maPrimary ) ||
            pEvent->xany.window == XtWindow( pObject->maObjectData.maSecondary ) )
        {
            switch( pEvent->type )
            {
                case UnmapNotify:
                    pObject->maObjectData.mbVisible = FALSE;
                    return 1;
                case MapNotify:
                    pObject->maObjectData.mbVisible = TRUE;
                    return 1;
                case ButtonPress:
                    pObject->maObjectData.mpProc(
                        pObject->maObjectData.mpInst,
                        pObject,
                        SALOBJ_EVENT_TOTOP,
                        NULL );
                    return 1;
                case FocusIn:
                    pObject->maObjectData.mpProc(
                        pObject->maObjectData.mpInst,
                        pObject,
                        SALOBJ_EVENT_GETFOCUS,
                        NULL );
                    return 1;
                case FocusOut:
                    pObject->maObjectData.mpProc(
                        pObject->maObjectData.mpInst,
                        pObject,
                        SALOBJ_EVENT_LOSEFOCUS,
                        NULL );
                    return 1;
                default: break;
            }
            return 0;
        }
    }
    return 0;
}
