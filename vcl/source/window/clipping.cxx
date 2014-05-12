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

#include <config_features.h>

#include <i18nlangtag/mslangid.hxx>

#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"

#include "unotools/fontcfg.hxx"
#include "unotools/confignode.hxx"

#include "vcl/layout.hxx"
#include "vcl/salgtype.hxx"
#include "vcl/event.hxx"
#include "vcl/fixed.hxx"
#include "vcl/help.hxx"
#include "vcl/cursor.hxx"
#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/syswin.hxx"
#include "vcl/syschild.hxx"
#include "vcl/dockwin.hxx"
#include "vcl/menu.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/wall.hxx"
#include "vcl/gradient.hxx"
#include "vcl/button.hxx"
#include "vcl/taskpanelist.hxx"
#include "vcl/dialog.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/popupmenuwindow.hxx"
#include "vcl/lazydelete.hxx"
#include "vcl/virdev.hxx"
#include "vcl/settings.hxx"

// declare system types in sysdata.hxx
#include "svsys.h"
#include "vcl/sysdata.hxx"

#include "salframe.hxx"
#include "salobj.hxx"
#include "salinst.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "dbggui.hxx"
#include "outfont.hxx"
#include "window.h"
#include "toolbox.h"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "brdwin.hxx"
#include "helpwin.hxx"
#include "sallayout.hxx"
#include "dndlcon.hxx"
#include "dndevdis.hxx"

#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XDisplayConnection.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/SystemClipboard.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/rendering/CanvasFactory.hpp"
#include "com/sun/star/rendering/XCanvas.hpp"
#include "com/sun/star/rendering/XSpriteCanvas.hpp"
#include "comphelper/processfactory.hxx"

#include <sal/macros.h>
#include <rtl/strbuf.hxx>

#include <set>
#include <typeinfo>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;
using namespace com::sun;

using ::com::sun::star::awt::XTopWindow;

void Window::InitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    Region  aRegion;

    // Put back backed up background
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();
    if ( mpWindowImpl->mbInPaint )
        aRegion = *(mpWindowImpl->mpPaintRegion);
    else
    {
        aRegion = *(ImplGetWinChildClipRegion());
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        // the mpWindowImpl->mpPaintRegion above is already correct (see ImplCallPaint()) !
        if( ImplIsAntiparallel() )
            ReMirror ( aRegion );
    }
    if ( mbClipRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
    if ( aRegion.IsEmpty() )
        mbOutputClipped = true;
    else
    {
        mbOutputClipped = false;
        SelectClipRegion( aRegion );
    }
    mbClipRegionSet = true;

    mbInitClipRegion = false;
}

Region Window::GetActiveClipRegion() const
{
    Region aRegion(true);

    if ( mpWindowImpl->mbInPaint )
    {
        aRegion = *(mpWindowImpl->mpPaintRegion);
        aRegion.Move( -mnOutOffX, -mnOutOffY );
    }

    if ( mbClipRegion )
        aRegion.Intersect( maRegion );

    return PixelToLogic( aRegion );
}

void Window::ClipToPaintRegion(Rectangle& rDstRect)
{
    const Region aPaintRgn(GetPaintRegion());

    if (!aPaintRgn.IsNull())
        rDstRect.Intersection(LogicToPixel(aPaintRgn.GetBoundRect()));
}

void Window::EnableClipSiblings( bool bClipSiblings )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mpWindowImpl->mbClipSiblings = bClipSiblings;
}


bool Window::ImplSysObjClip( const Region* pOldRegion )
{
    bool bUpdate = true;

    if ( mpWindowImpl->mpSysObj )
    {
        bool bVisibleState = mpWindowImpl->mbReallyVisible;

        if ( bVisibleState )
        {
            Region* pWinChildClipRegion = ImplGetWinChildClipRegion();

            if ( !pWinChildClipRegion->IsEmpty() )
            {
                if ( pOldRegion )
                {
                    Region aNewRegion = *pWinChildClipRegion;
                    pWinChildClipRegion->Intersect( *pOldRegion );
                    bUpdate = aNewRegion == *pWinChildClipRegion;
                }

                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aWinRectRegion( aWinRect );
                sal_uInt16      nClipFlags = mpWindowImpl->mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    mpWindowImpl->mpSysObj->ResetClipRegion();
                else
                {
                    if ( nClipFlags & SAL_OBJECT_CLIP_EXCLUDERECTS )
                    {
                        aWinRectRegion.Exclude( aRegion );
                        aRegion = aWinRectRegion;
                    }
                    if ( !(nClipFlags & SAL_OBJECT_CLIP_ABSOLUTE) )
                        aRegion.Move( -mnOutOffX, -mnOutOffY );

                    // ClipRegion setzen/updaten
                    RectangleVector aRectangles;
                    aRegion.GetRegionRectangles(aRectangles);
                    mpWindowImpl->mpSysObj->BeginSetClipRegion(aRectangles.size());

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                    {
                        mpWindowImpl->mpSysObj->UnionClipRegion(
                            aRectIter->Left(),
                            aRectIter->Top(),
                            aRectIter->GetWidth(),   // orig nWidth was ((R - L) + 1), same as GetWidth does
                            aRectIter->GetHeight()); // same for height
                    }

                    mpWindowImpl->mpSysObj->EndSetClipRegion();

                    //long                nX;
                    //long                nY;
                    //long                nWidth;
                    //long                nHeight;
                    //sal_uLong               nRectCount;
                    //ImplRegionInfo      aInfo;
                    //sal_Bool                bRegionRect;

                    //nRectCount = aRegion.GetRectCount();
                    //mpWindowImpl->mpSysObj->BeginSetClipRegion( nRectCount );
                    //bRegionRect = aRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                    //while ( bRegionRect )
                    //{
                    //    mpWindowImpl->mpSysObj->UnionClipRegion( nX, nY, nWidth, nHeight );
                    //    bRegionRect = aRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                    //}
                    //mpWindowImpl->mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = false;
        }

        // Visible-Status updaten
        mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

void Window::ImplUpdateSysObjChildrenClip()
{
    if ( mpWindowImpl->mpSysObj && mpWindowImpl->mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildrenClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildrenClip();

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjOverlapsClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateSysObjClip()
{
    if ( !ImplIsOverlapWindow() )
    {
        ImplUpdateSysObjChildrenClip();

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildrenClip();
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }
    }
    else
        mpWindowImpl->mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

bool Window::ImplSetClipFlagChildren( bool bSysObjOnlySmaller )
{
    bool bUpdate = true;
    if ( mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( mpWindowImpl->maWinClipRegion );

        mbInitClipRegion = true;
        mpWindowImpl->mbInitWinClipRegion = true;

        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                bUpdate = false;
            pWindow = pWindow->mpWindowImpl->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = true;
            mpWindowImpl->mbInitWinClipRegion = true;
            bUpdate = false;
        }

        delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = true;
    mpWindowImpl->mbInitWinClipRegion = true;

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
    }
    return bUpdate;
}

bool Window::ImplSetClipFlagOverlapWindows( bool bSysObjOnlySmaller )
{
    bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}

bool Window::ImplSetClipFlag( bool bSysObjOnlySmaller )
{
    if ( !ImplIsOverlapWindow() )
    {
        bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = true;
            pParent->mpWindowImpl->mbInitChildRegion = true;
        }

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                    bUpdate = false;
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpWindowImpl->mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}

void Window::ImplIntersectWindowClipRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbInitWinClipRegion )
        ImplInitWinClipRegion();

    rRegion.Intersect( mpWindowImpl->maWinClipRegion );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
