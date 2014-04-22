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

#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/svapp.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/region.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>
#include <svsys.h>
#include <vcl/sysdata.hxx>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <salframe.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <svdata.hxx>
#include <window.h>
#include <outdev.h>
#include <outdata.hxx>
#include "PhysicalFontCollection.hxx"

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/CanvasFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <numeric>

void OutputDevice::ClipToPaintRegion(Rectangle& /*rDstRect*/)
{
}

bool OutputDevice::ImplSelectClipRegion( const Region& rRegion, SalGraphics* pGraphics )
{
    DBG_TESTSOLARMUTEX();

    if( !pGraphics )
    {
        if( !mpGraphics )
            if( !AcquireGraphics() )
                return false;
        pGraphics = mpGraphics;
    }

    bool bClipRegion = pGraphics->SetClipRegion( rRegion, this );
    OSL_ENSURE( bClipRegion, "OutputDevice::ImplSelectClipRegion() - can't create region" );
    return bClipRegion;
}

// TODO: fdo#74424 - this needs to be moved out of OutputDevice and into the
// Window, VirtualDevice and Printer classes
void OutputDevice::ImplInitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;
        Region  aRegion;

        // Put back backed up background
        if ( pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin )
            pWindow->ImplInvalidateAllOverlapBackgrounds();
        if ( pWindow->mpWindowImpl->mbInPaint )
            aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
        else
        {
            aRegion = *(pWindow->ImplGetWinChildClipRegion());
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
            ImplSelectClipRegion( aRegion );
        }
        mbClipRegionSet = true;
    }
    else
    {
        if ( mbClipRegion )
        {
            if ( maRegion.IsEmpty() )
                mbOutputClipped = true;
            else
            {
                mbOutputClipped = false;

                // #102532# Respect output offset also for clip region
                Region aRegion( ImplPixelToDevicePixel( maRegion ) );
                const bool bClipDeviceBounds( ! GetPDFWriter()
                                              && GetOutDevType() != OUTDEV_PRINTER );
                if( bClipDeviceBounds )
                {
                    // Perform actual rect clip against outdev
                    // dimensions, to generate empty clips whenever one of the
                    // values is completely off the device.
                    Rectangle aDeviceBounds( mnOutOffX, mnOutOffY,
                                             mnOutOffX+GetOutputWidthPixel()-1,
                                             mnOutOffY+GetOutputHeightPixel()-1 );
                    aRegion.Intersect( aDeviceBounds );
                }

                if ( aRegion.IsEmpty() )
                {
                    mbOutputClipped = true;
                }
                else
                {
                    mbOutputClipped = false;
                    ImplSelectClipRegion( aRegion );
                }
            }

            mbClipRegionSet = true;
        }
        else
        {
            if ( mbClipRegionSet )
            {
                mpGraphics->ResetClipRegion();
                mbClipRegionSet = false;
            }

            mbOutputClipped = false;
        }
    }

    mbInitClipRegion = false;
}

void OutputDevice::ImplSetClipRegion( const Region* pRegion )
{
    DBG_TESTSOLARMUTEX();

    if ( !pRegion )
    {
        if ( mbClipRegion )
        {
            maRegion            = Region(true);
            mbClipRegion        = false;
            mbInitClipRegion    = true;
        }
    }
    else
    {
        maRegion            = *pRegion;
        mbClipRegion        = true;
        mbInitClipRegion    = true;
    }
}

void OutputDevice::SetClipRegion()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( Region(), false ) );

    ImplSetClipRegion( NULL );

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion();
}

void OutputDevice::SetClipRegion( const Region& rRegion )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( rRegion, true ) );

    if ( rRegion.IsNull() )
    {
        ImplSetClipRegion( NULL );
    }
    else
    {
        Region aRegion = LogicToPixel( rRegion );
        ImplSetClipRegion( &aRegion );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion( rRegion );
}

Region OutputDevice::GetClipRegion() const
{

    return PixelToLogic( maRegion );
}

Region OutputDevice::GetActiveClipRegion() const
{

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Region aRegion(true);
        Window* pWindow = (Window*)this;
        if ( pWindow->mpWindowImpl->mbInPaint )
        {
            aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
            aRegion.Move( -mnOutOffX, -mnOutOffY );
        }
        if ( mbClipRegion )
            aRegion.Intersect( maRegion );
        return PixelToLogic( aRegion );
    }
    else
        return GetClipRegion();
}

void OutputDevice::MoveClipRegion( long nHorzMove, long nVertMove )
{

    if ( mbClipRegion )
    {
        if( mpMetaFile )
            mpMetaFile->AddAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );

        maRegion.Move( ImplLogicWidthToDevicePixel( nHorzMove ),
                       ImplLogicHeightToDevicePixel( nVertMove ) );
        mbInitClipRegion = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->MoveClipRegion( nHorzMove, nVertMove );
}

void OutputDevice::IntersectClipRegion( const Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaISectRectClipRegionAction( rRect ) );

    Rectangle aRect = LogicToPixel( rRect );
    maRegion.Intersect( aRect );
    mbClipRegion        = true;
    mbInitClipRegion    = true;

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRect );
}

void OutputDevice::IntersectClipRegion( const Region& rRegion )
{

    if(!rRegion.IsNull())
    {
        if ( mpMetaFile )
            mpMetaFile->AddAction( new MetaISectRegionClipRegionAction( rRegion ) );

        Region aRegion = LogicToPixel( rRegion );
        maRegion.Intersect( aRegion );
        mbClipRegion        = true;
        mbInitClipRegion    = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRegion );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
