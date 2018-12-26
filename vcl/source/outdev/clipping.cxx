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

#include <sal/config.h>
#include <osl/diagnose.h>

#include <tools/debug.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>

#include <salgdi.hxx>
#include <salframe.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <window.h>

#include <numeric>

vcl::Region OutputDevice::GetClipRegion() const
{

    return PixelToLogic( maRegion );
}

void OutputDevice::SetClipRegion()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( vcl::Region(), false ) );

    SetDeviceClipRegion( nullptr );

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion();
}

void OutputDevice::SetClipRegion( const vcl::Region& rRegion )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( rRegion, true ) );

    if ( rRegion.IsNull() )
    {
        SetDeviceClipRegion( nullptr );
    }
    else
    {
        vcl::Region aRegion = LogicToPixel( rRegion );
        SetDeviceClipRegion( &aRegion );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion( rRegion );
}

bool OutputDevice::SelectClipRegion( const vcl::Region& rRegion, SalGraphics* pGraphics )
{
    DBG_TESTSOLARMUTEX();

    if( !pGraphics )
    {
        if( !mpGraphics && !AcquireGraphics() )
            return false;
        pGraphics = mpGraphics;
    }

    bool bClipRegion = pGraphics->SetClipRegion( rRegion, this );
    OSL_ENSURE( bClipRegion, "OutputDevice::SelectClipRegion() - can't create region" );
    return bClipRegion;
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

void OutputDevice::IntersectClipRegion( const tools::Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaISectRectClipRegionAction( rRect ) );

    tools::Rectangle aRect = LogicToPixel( rRect );
    maRegion.Intersect( aRect );
    mbClipRegion        = true;
    mbInitClipRegion    = true;

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRect );
}

void OutputDevice::IntersectClipRegion( const vcl::Region& rRegion )
{

    if(!rRegion.IsNull())
    {
        if ( mpMetaFile )
            mpMetaFile->AddAction( new MetaISectRegionClipRegionAction( rRegion ) );

        vcl::Region aRegion = LogicToPixel( rRegion );
        maRegion.Intersect( aRegion );
        mbClipRegion        = true;
        mbInitClipRegion    = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRegion );
}

void OutputDevice::InitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    if ( mbClipRegion )
    {
        if ( maRegion.IsEmpty() )
            mbOutputClipped = true;
        else
        {
            mbOutputClipped = false;

            // #102532# Respect output offset also for clip region
            vcl::Region aRegion( ImplPixelToDevicePixel( maRegion ) );
            const bool bClipDeviceBounds((OUTDEV_PDF != GetOutDevType())
                                          && (OUTDEV_PRINTER != GetOutDevType()));
            if( bClipDeviceBounds )
            {
                // Perform actual rect clip against outdev
                // dimensions, to generate empty clips whenever one of the
                // values is completely off the device.
                tools::Rectangle aDeviceBounds( mnOutOffX, mnOutOffY,
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
                SelectClipRegion( aRegion );
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

    mbInitClipRegion = false;
}

vcl::Region OutputDevice::GetActiveClipRegion() const
{
    return GetClipRegion();
}

void OutputDevice::ClipToPaintRegion(tools::Rectangle& /*rDstRect*/)
{
    // this is only used in Window, but we still need it as it's called
    // on in other clipping functions
}

void OutputDevice::SetDeviceClipRegion( const vcl::Region* pRegion )
{
    DBG_TESTSOLARMUTEX();

    if ( !pRegion )
    {
        if ( mbClipRegion )
        {
            maRegion            = vcl::Region(true);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
