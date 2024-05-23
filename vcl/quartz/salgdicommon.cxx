/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <sal/log.hxx>

#include <cassert>
#include <cstring>
#include <numeric>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/endian.h>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <sal/types.h>
#include <tools/long.hxx>
#include <vcl/sysdata.hxx>

#include <quartz/salbmp.h>
#ifdef MACOSX
#include <quartz/salgdi.h>
#endif
#include <quartz/utils.h>
#ifdef IOS
#include <svdata.hxx>
#endif

using namespace vcl;

#ifndef IOS

void AquaSalGraphics::copyResolution( AquaSalGraphics& rGraphics )
{
    if (!rGraphics.mnRealDPIY && rGraphics.maShared.mbWindow && rGraphics.maShared.mpFrame)
    {
        rGraphics.initResolution(rGraphics.maShared.mpFrame->getNSWindow());
    }
    mnRealDPIX = rGraphics.mnRealDPIX;
    mnRealDPIY = rGraphics.mnRealDPIY;
}

#endif

SystemGraphicsData AquaSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.rCGContext = maShared.maContextHolder.get();
    return aRes;
}

#ifndef IOS

void AquaSalGraphics::initResolution(NSWindow* nsWindow)
{
    if (!nsWindow)
    {
        if (Application::IsBitmapRendering())
            mnRealDPIX = mnRealDPIY = 96;
        return;
    }

    // #i100617# read DPI only once; there is some kind of weird caching going on
    // if the main screen changes
    // FIXME: this is really unfortunate and needs to be investigated

    SalData* pSalData = GetSalData();
    if( pSalData->mnDPIX == 0 || pSalData->mnDPIY == 0 )
    {
        NSScreen* pScreen = nil;

        /* #i91301#
        many woes went into the try to have different resolutions
        on different screens. The result of these trials is that OOo is not ready
        for that yet, vcl and applications would need to be adapted.

        Unfortunately this is not possible in the 3.0 timeframe.
        So let's stay with one resolution for all Windows and VirtualDevices
        which is the resolution of the main screen

        This of course also means that measurements are exact only on the main screen.
        For activating different resolutions again just comment out the two lines below.

        if( pWin )
        pScreen = [pWin screen];
        */
        if( pScreen == nil )
        {
            NSArray* pScreens = [NSScreen screens];
            if( pScreens && [pScreens count] > 0)
            {
                pScreen = [pScreens objectAtIndex: 0];
            }
        }

        mnRealDPIX = mnRealDPIY = 96;
        if( pScreen )
        {
            NSDictionary* pDev = [pScreen deviceDescription];
            if( pDev )
            {
                NSNumber* pVal = [pDev objectForKey: @"NSScreenNumber"];
                if( pVal )
                {
                    // FIXME: casting a long to CGDirectDisplayID is evil, but
                    // Apple suggest to do it this way
                    const CGDirectDisplayID nDisplayID = static_cast<CGDirectDisplayID>([pVal longValue]);
                    const CGSize aSize = CGDisplayScreenSize( nDisplayID ); // => result is in millimeters
                    mnRealDPIX = static_cast<sal_Int32>((CGDisplayPixelsWide( nDisplayID ) * 25.4) / aSize.width);
                    mnRealDPIY = static_cast<sal_Int32>((CGDisplayPixelsHigh( nDisplayID ) * 25.4) / aSize.height);
                }
                else
                {
                    OSL_FAIL( "no resolution found in device description" );
                }
            }
            else
            {
                OSL_FAIL( "no device description" );
            }
        }
        else
        {
            OSL_FAIL( "no screen found" );
        }

        // #i107076# maintaining size-WYSIWYG-ness causes many problems for
        //           low-DPI, high-DPI or for mis-reporting devices
        //           => it is better to limit the calculation result then
        static const int nMinDPI = 72;
        if( (mnRealDPIX < nMinDPI) || (mnRealDPIY < nMinDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMinDPI;
        }
        // Note that on a Retina display, the "mnRealDPIX" as
        // calculated above is not the true resolution of the display,
        // but the "logical" one, or whatever the correct terminology
        // is. (For instance on a 5K 27in iMac, it's 108.)  So at
        // least currently, it won't be over 200. I don't know whether
        // this test is a "sanity check", or whether there is some
        // real reason to limit this to 200.
        static const int nMaxDPI = 200;
        if( (mnRealDPIX > nMaxDPI) || (mnRealDPIY > nMaxDPI) )
        {
            mnRealDPIX = mnRealDPIY = nMaxDPI;
        }
        // for OSX any anisotropy reported for the display resolution is best ignored (e.g. TripleHead2Go)
        mnRealDPIX = mnRealDPIY = (mnRealDPIX + mnRealDPIY + 1) / 2;

        pSalData->mnDPIX = mnRealDPIX;
        pSalData->mnDPIY = mnRealDPIY;
    }
    else
    {
        mnRealDPIX = pSalData->mnDPIX;
        mnRealDPIY = pSalData->mnDPIY;
    }
}

#endif

void AquaSharedAttributes::setState()
{
    maContextHolder.restoreState();
    maContextHolder.saveState();

    // setup clipping
    if (mxClipPath)
    {
        CGContextBeginPath(maContextHolder.get());            // discard any existing path
        CGContextAddPath(maContextHolder.get(), mxClipPath);  // set the current path to the clipping path
        CGContextClip(maContextHolder.get());                 // use it for clipping
    }

    // set RGB colorspace and line and fill colors
    CGContextSetFillColor(maContextHolder.get(), maFillColor.AsArray() );

    CGContextSetStrokeColor(maContextHolder.get(), maLineColor.AsArray() );
    CGContextSetShouldAntialias(maContextHolder.get(), false );
    if (mnXorMode == 2)
    {
        CGContextSetBlendMode(maContextHolder.get(), kCGBlendModeDifference );
    }
}

#ifndef IOS

void AquaSalGraphics::updateResolution()
{
    SAL_WARN_IF(!maShared.mbWindow, "vcl", "updateResolution on inappropriate graphics");

    initResolution((maShared.mbWindow && maShared.mpFrame) ? maShared.mpFrame->getNSWindow() : nil);
}

#endif

XorEmulation::XorEmulation()
  : m_xTargetLayer( nullptr )
  , m_xTargetContext( nullptr )
  , m_xMaskContext( nullptr )
  , m_xTempContext( nullptr )
  , m_pMaskBuffer( nullptr )
  , m_pTempBuffer( nullptr )
  , m_nBufferLongs( 0 )
  , m_bIsEnabled( false )
{
    SAL_INFO( "vcl.quartz", "XorEmulation::XorEmulation() this=" << this );
}

XorEmulation::~XorEmulation()
{
    SAL_INFO( "vcl.quartz", "XorEmulation::~XorEmulation() this=" << this );
    Disable();
    SetTarget( 0, 0, 0, nullptr, nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
