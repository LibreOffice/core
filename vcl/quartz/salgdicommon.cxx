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
#include <osl/file.hxx>
#include <sal/types.h>
#include <tools/long.hxx>
#include <vcl/sysdata.hxx>

#include <fontsubset.hxx>
#include <quartz/salbmp.h>
#ifdef MACOSX
#include <quartz/salgdi.h>
#endif
#include <quartz/utils.h>
#ifdef IOS
#include <svdata.hxx>
#endif
#include <sft.hxx>

using namespace vcl;

bool AquaSalGraphics::CreateFontSubset( const OUString& rToFile,
                                        const vcl::font::PhysicalFontFace* pFontData,
                                        const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                        sal_Int32* pGlyphWidths, const int nGlyphCount,
                                        FontSubsetInfo& rInfo )
{
    // TODO: move more of the functionality here into the generic subsetter code

    // prepare the requested file name for writing the font-subset file
    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return false;

    // get the raw-bytes from the font to be subset
    std::vector<unsigned char> aBuffer;
    bool bCffOnly = false;
    if( !GetRawFontData( pFontData, aBuffer, &bCffOnly ) )
        return false;
    const OString aToFile( OUStringToOString( aSysPath,
                                              osl_getThreadTextEncoding()));

    // handle CFF-subsetting
    // NOTE: assuming that all glyphids requested on Aqua are fully translated
    if (bCffOnly)
        return SalGraphics::CreateCFFfontSubset(aBuffer.data(), aBuffer.size(), aToFile, pGlyphIds,
                                                pEncoding, pGlyphWidths, nGlyphCount, rInfo);

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // prepare data for psprint's font subsetter
    TrueTypeFont* pSftFont = nullptr;
    if (::OpenTTFontBuffer( static_cast<void*>(aBuffer.data()), aBuffer.size(), 0, &pSftFont)
            != SFErrCodes::Ok)
        return false;

    // get details about the subsetted font
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    OUString aPSName(aTTInfo.psname, std::strlen(aTTInfo.psname), RTL_TEXTENCODING_UTF8);
    FillFontSubsetInfo(aTTInfo, aPSName, rInfo);

    // write subset into destination file
    bool bRet
        = SalGraphics::CreateTTFfontSubset(*pSftFont, aToFile, false /* use FontSelectPattern? */,
                                           pGlyphIds, pEncoding, pGlyphWidths, nGlyphCount);
    ::CloseTTFont(pSftFont);
    return bRet;
}

#ifndef IOS

void AquaSalGraphics::copyResolution( AquaSalGraphics& rGraphics )
{
    if (!rGraphics.mnRealDPIY && rGraphics.maShared.mbWindow && rGraphics.maShared.mpFrame)
        AquaSalGraphics::GetDPI(rGraphics.maShared.mpFrame->getNSWindow(), rGraphics.mnRealDPIX, rGraphics.mnRealDPIY);
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

void AquaSalGraphics::GetDPI(NSWindow* nsWindow, sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    // #i100617# read DPI only once; there is some kind of weird caching going on
    // if the main screen changes
    // FIXME: this is really unfortunate and needs to be investigated

    SalData* pSalData = GetSalData();
    if (pSalData->mnDPIX != 0 && pSalData->mnDPIY != 0)
    {
        rDPIX = pSalData->mnDPIX;
        rDPIY = pSalData->mnDPIY;
        return;
    }

    if (!nsWindow)
    {
        assert(Application::IsBitmapRendering());
        pSalData->mnDPIX = pSalData->mnDPIY = 96;
        return;
    }

    /* #i91301#
    many woes went into the try to have different resolutions
    on different screens. The result of these trials is that OOo is not ready
    for that yet, vcl and applications would need to be adapted.

    Unfortunately this is not possible in the 3.0 timeframe.
    So let's stay with one resolution for all Windows and VirtualDevices
    which is the resolution of the main screen
    */
    NSScreen* pScreen = nil;
    NSArray* pScreens = [NSScreen screens];
    if (pScreens && [pScreens count] > 0)
    {
        pScreen = [pScreens objectAtIndex: 0];
    }

    rDPIX = rDPIY = 96;
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
                rDPIX = static_cast<sal_Int32>((CGDisplayPixelsWide( nDisplayID ) * 25.4) / aSize.width);
                rDPIY = static_cast<sal_Int32>((CGDisplayPixelsHigh( nDisplayID ) * 25.4) / aSize.height);
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
    if( (rDPIX < nMinDPI) || (rDPIY < nMinDPI) )
    {
        rDPIX = rDPIY = nMinDPI;
    }
    // Note that on a Retina display, the "rDPIX" as
    // calculated above is not the true resolution of the display,
    // but the "logical" one, or whatever the correct terminology
    // is. (For instance on a 5K 27in iMac, it's 108.)  So at
    // least currently, it won't be over 200. I don't know whether
    // this test is a "sanity check", or whether there is some
    // real reason to limit this to 200.
    static const int nMaxDPI = 200;
    if( (rDPIX > nMaxDPI) || (rDPIY > nMaxDPI) )
    {
        rDPIX = rDPIY = nMaxDPI;
    }
    // for OSX any anisotropy reported for the display resolution is best ignored (e.g. TripleHead2Go)
    rDPIX = rDPIY = (rDPIX + rDPIY + 1) / 2;

    pSalData->mnDPIX = rDPIX;
    pSalData->mnDPIY = rDPIY;
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
    NSWindow* pWindow = (maShared.mbWindow && maShared.mpFrame) ? maShared.mpFrame->getNSWindow() : nil;
    AquaSalGraphics::GetDPI(pWindow, mnRealDPIX, mnRealDPIY);
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
