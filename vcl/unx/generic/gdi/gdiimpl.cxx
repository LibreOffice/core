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

#include <memory>
#include <numeric>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xproto.h>

#include "gdiimpl.hxx"

#include <vcl/gradient.hxx>
#include <sal/log.hxx>

#include <unx/saldisp.hxx>
#include <unx/salbmp.h>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>
#include <salframe.hxx>
#include <unx/x11/xrender_peer.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/utils/systemdependentdata.hxx>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

X11SalGraphicsImpl::X11SalGraphicsImpl(X11SalGraphics& rParent):
    mrParent(rParent),
    mbCopyGC(false),
    mbInvertGC(false),
    mbStippleGC(false),
    mbXORMode(false),
    mpCopyGC(nullptr),
    mpInvertGC(nullptr),
    mpStippleGC(nullptr)
{
}

X11SalGraphicsImpl::~X11SalGraphicsImpl()
{
}

void X11SalGraphicsImpl::Init()
{
}

XID X11SalGraphicsImpl::GetXRenderPicture()
{
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();

    if( !mrParent.m_aXRenderPicture )
    {
        // check xrender support for matching visual
        XRenderPictFormat* pXRenderFormat = mrParent.GetXRenderFormat();
        if( !pXRenderFormat )
            return 0;
        // get the matching xrender target for drawable
        mrParent.m_aXRenderPicture = rRenderPeer.CreatePicture( mrParent.GetDrawable(), pXRenderFormat, 0, nullptr );
    }

    {
        // reset clip region
        // TODO: avoid clip reset if already done
        XRenderPictureAttributes aAttr;
        aAttr.clip_mask = None;
        rRenderPeer.ChangePicture( mrParent.m_aXRenderPicture, CPClipMask, &aAttr );
    }

    return mrParent.m_aXRenderPicture;
}

static void freeGC(Display *pDisplay, GC& rGC)
{
    if( rGC )
    {
        XFreeGC( pDisplay, rGC );
        rGC = None;
    }
}

void X11SalGraphicsImpl::freeResources()
{
    Display *pDisplay = mrParent.GetXDisplay();

    freeGC( pDisplay, mpCopyGC );
    freeGC( pDisplay, mpInvertGC );
    freeGC( pDisplay, mpStippleGC );
    mbCopyGC = mbInvertGC = mbStippleGC = false;
}

GC X11SalGraphicsImpl::CreateGC( Drawable hDrawable, unsigned long nMask )
{
    XGCValues values;

    values.graphics_exposures   = False;
    values.foreground           = mrParent.GetColormap().GetBlackPixel()
                                  ^ mrParent.GetColormap().GetWhitePixel();
    values.function             = GXxor;
    values.line_width           = 1;
    values.fill_style           = FillStippled;
    values.stipple              = mrParent.GetDisplay()->GetInvert50( mrParent.m_nXScreen );
    values.subwindow_mode       = ClipByChildren;

    return XCreateGC( mrParent.GetXDisplay(), hDrawable, nMask | GCSubwindowMode, &values );
}

inline GC X11SalGraphicsImpl::GetCopyGC()
{
    if( mbXORMode ) return GetInvertGC();

    if( !mpCopyGC )
        mpCopyGC = CreateGC( mrParent.GetDrawable() );

    if( !mbCopyGC )
    {
        mrParent.SetClipRegion( mpCopyGC );
        mbCopyGC = true;
    }
    return mpCopyGC;
}

GC X11SalGraphicsImpl::GetInvertGC()
{
    if( !mpInvertGC )
        mpInvertGC = CreateGC( mrParent.GetDrawable(),
                               GCGraphicsExposures
                               | GCForeground
                               | GCFunction
                               | GCLineWidth );

    if( !mbInvertGC )
    {
        mrParent.SetClipRegion( mpInvertGC );
        mbInvertGC = true;
    }
    return mpInvertGC;
}

inline GC X11SalGraphicsImpl::GetStippleGC()
{
    if( !mpStippleGC )
        mpStippleGC = CreateGC( mrParent.GetDrawable(),
                                GCGraphicsExposures
                                | GCFillStyle
                                | GCLineWidth );

    if( !mbStippleGC )
    {
        XSetFunction( mrParent.GetXDisplay(), mpStippleGC, mbXORMode ? GXxor : GXcopy );
        mrParent.SetClipRegion( mpStippleGC );
        mbStippleGC = true;
    }

    return mpStippleGC;
}

void X11SalGraphicsImpl::ResetClipRegion()
{
    if( !mrParent.mpClipRegion )
        return;

    mbCopyGC        = false;
    mbInvertGC      = false;
    mbStippleGC     = false;

    XDestroyRegion( mrParent.mpClipRegion );
    mrParent.mpClipRegion    = nullptr;
}

void X11SalGraphicsImpl::setClipRegion( const vcl::Region& i_rClip )
{
    if( mrParent.mpClipRegion )
        XDestroyRegion( mrParent.mpClipRegion );
    mrParent.mpClipRegion = XCreateRegion();

    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    for (auto const& rectangle : aRectangles)
    {
        const tools::Long nW(rectangle.GetWidth());

        if(nW)
        {
            const tools::Long nH(rectangle.GetHeight());

            if(nH)
            {
                XRectangle aRect;

                aRect.x = static_cast<short>(rectangle.Left());
                aRect.y = static_cast<short>(rectangle.Top());
                aRect.width = static_cast<unsigned short>(nW);
                aRect.height = static_cast<unsigned short>(nH);
                XUnionRectWithRegion(&aRect, mrParent.mpClipRegion, mrParent.mpClipRegion);
            }
        }
    }

    //ImplRegionInfo aInfo;
    //long nX, nY, nW, nH;
    //bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
    //while( bRegionRect )
    //{
    //    if ( nW && nH )
    //    {
    //        XRectangle aRect;
    //        aRect.x           = (short)nX;
    //        aRect.y           = (short)nY;
    //        aRect.width       = (unsigned short)nW;
    //        aRect.height  = (unsigned short)nH;

    //        XUnionRectWithRegion( &aRect, mrParent.mpClipRegion, mrParent.mpClipRegion );
    //    }
    //    bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
    //}

    // done, invalidate GCs
    mbCopyGC        = false;
    mbInvertGC      = false;
    mbStippleGC     = false;

    if( XEmptyRegion( mrParent.mpClipRegion ) )
    {
        XDestroyRegion( mrParent.mpClipRegion );
        mrParent.mpClipRegion= nullptr;
    }
}

void X11SalGraphicsImpl::SetXORMode( bool bSet, bool )
{
    if (mbXORMode != bSet)
    {
        mbXORMode   = bSet;
        mbCopyGC        = false;
        mbInvertGC  = false;
        mbStippleGC = false;
    }
}

tools::Long X11SalGraphicsImpl::GetGraphicsHeight() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.height();
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetHeight();
    else
        return 0;
}

sal_uInt16 X11SalGraphicsImpl::GetBitCount() const
{
    return mrParent.GetVisual().GetDepth();
}

tools::Long X11SalGraphicsImpl::GetGraphicsWidth() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.width();
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetWidth();
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
