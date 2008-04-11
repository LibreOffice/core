/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdi.cxx,v $
 * $Revision: 1.71 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "salconst.h"
#include "salgdi.h"
#include "salbmp.h"
#include "salcolorutils.hxx"
#include "vcl/impfont.hxx"
#include "psprint/list.h"
#include "psprint/sft.h"
#include "osl/file.hxx"
#include "vos/mutex.hxx"
#include "osl/process.h"

#include "vcl/sallayout.hxx"
#include "salatsuifontutils.hxx"
#include "vcl/svapp.hxx"

#include "basegfx/range/b2drectangle.hxx"
#include "basegfx/range/b2irange.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"

#include "basebmp/color.hxx"

#include <boost/assert.hpp>
#include <algorithm>

using namespace std;


typedef unsigned char Boolean; // copied from MacTypes.h, should be properly included
typedef std::vector<unsigned char> ByteVector;


// =======================================================================

ImplMacFontData::ImplMacFontData( const ImplDevFontAttributes& rDFA, ATSUFontID nFontId )
:   ImplFontData( rDFA, 0 )
,   mnFontId( nFontId )
,   mpCharMap( NULL )
,   mbOs2Read( false )
,   mbHasOs2Table( false )
,   mbCmapEncodingRead( false )
,   mbHasCJKSupport( false )
{}

// -----------------------------------------------------------------------

ImplMacFontData::~ImplMacFontData()
{
    if( mpCharMap )
        mpCharMap->DeReference();
}

// -----------------------------------------------------------------------

sal_IntPtr ImplMacFontData::GetFontId() const
{
    return (sal_IntPtr)mnFontId;
}

// -----------------------------------------------------------------------

ImplFontData* ImplMacFontData::Clone() const
{
    ImplMacFontData* pClone = new ImplMacFontData(*this);
    if( mpCharMap )
        mpCharMap->AddReference();
    return pClone;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplMacFontData::CreateFontInstance(ImplFontSelectData& rFSD) const
{
    return new ImplFontEntry(rFSD);
}

// -----------------------------------------------------------------------

inline FourCharCode GetTag(const char aTagName[5])
{
    return (aTagName[0]<<24)+(aTagName[1]<<16)+(aTagName[2]<<8)+(aTagName[3]);
}

static unsigned GetUShort( const unsigned char* p ){return((p[0]<<8)+p[1]);}
static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}

ImplFontCharMap* ImplMacFontData::GetImplFontCharMap() const
{
    if( mpCharMap )
    {
        // return the cached charmap
        mpCharMap->AddReference();
        return mpCharMap;
    }

    // set the default charmap
    mpCharMap = ImplFontCharMap::GetDefaultMap();

    // get the CMAP byte size
    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::GetImplFontCharMap : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return mpCharMap;

    // allocate a buffer for the CMAP raw data
    ByteVector aBuffer( nBufSize );

    // get the CMAP raw data
    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::GetImplFontCharMap : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
        return mpCharMap;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::GetImplFontCharMap : ByteCount mismatch!\n");

    // parse the CMAP
    CmapResult aCmapResult;
    if( !ParseCMAP( &aBuffer[0], nRawLength, aCmapResult ) )
        return mpCharMap;

    mpCharMap = new ImplFontCharMap( aCmapResult.mnPairCount, aCmapResult.mpPairCodes, aCmapResult.mpStartGlyphs );
    return mpCharMap;
}

// -----------------------------------------------------------------------

void ImplMacFontData::ReadOs2Table( void ) const
{
    // read this only once per font
    if( mbOs2Read )
        return;
    mbOs2Read = true;

    // prepare to get the OS/2 table raw data
    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadOs2Table : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return;

    // allocate a buffer for the OS/2 raw data
    ByteVector aBuffer;
    aBuffer.resize( nBufSize );

    // get the OS/2 raw data
    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("OS/2"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadOs2Table : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::ReadOs2Table : ByteCount mismatch!\n");
    mbHasOs2Table = true;

    // parse the OS/2 raw data
    // TODO: also analyze panose info, etc.

    // check if the fonts needs the "CJK extra leading" heuristic
    const unsigned char* pOS2map = &aBuffer[0];
    const sal_uInt32 nVersion = GetUShort( pOS2map );
    if( nVersion >= 0x0001 )
    {
        sal_uInt32 ulUnicodeRange2 = GetUInt( pOS2map + 46 );
        if( ulUnicodeRange2 & 0x2DF00000 )
            mbHasCJKSupport = true;
    }
}

void ImplMacFontData::ReadMacCmapEncoding( void ) const
{
    // read this only once per font
    if( mbCmapEncodingRead )
        return;
    mbCmapEncodingRead = true;

    ATSFontRef rFont = FMGetATSFontRefFromFont( mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nBufSize );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadMacCmapEncoding : ATSFontGetTable1 failed!\n");
    if( eStatus != noErr )
        return;

    ByteVector aBuffer;
    aBuffer.resize( nBufSize );

    ByteCount nRawLength = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nBufSize, (void*)&aBuffer[0], &nRawLength );
    DBG_ASSERT( (eStatus==noErr), "ImplMacFontData::ReadMacCmapEncoding : ATSFontGetTable2 failed!\n");
    if( eStatus != noErr )
        return;
    DBG_ASSERT( (nBufSize==nRawLength), "ImplMacFontData::ReadMacCmapEncoding : ByteCount mismatch!\n");

    const unsigned char* pCmap = &aBuffer[0];

    if (nRawLength < 24 )
        return;
    if( GetUShort( pCmap ) != 0x0000 )
        return;

    // check if the fonts needs the "CJK extra leading" heuristic
    int nSubTables = GetUShort( pCmap + 2 );

    for( const unsigned char* p = pCmap + 4; --nSubTables >= 0; p += 8 )
    {
        int nPlatform = GetUShort( p );
        if( nPlatform == kFontMacintoshPlatform ) {
            int nEncoding = GetUShort (p + 2 );
            if( nEncoding == kFontJapaneseScript ||
                nEncoding == kFontTraditionalChineseScript ||
                nEncoding == kFontKoreanScript ||
                nEncoding == kFontSimpleChineseScript )
            {
                mbHasCJKSupport = true;
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

bool ImplMacFontData::HasCJKSupport( void ) const
{
    ReadOs2Table();
    if( !mbHasOs2Table )
        ReadMacCmapEncoding();

    return mbHasCJKSupport;
}

// =======================================================================

AquaSalGraphics::AquaSalGraphics()
    : mpFrame( NULL )
    , mrContext( 0 )
    , mnRealDPIX( 0 )
    , mnRealDPIY( 0 )
    , mfFakeDPIScale( 1.0 )
    , mrClippingPath( 0 )
    , mbXORMode( false )
    , mpMacFontData( NULL )
    , mnATSUIRotation( 0 )
    , mfFontScale( 1.0 )
    , mfFontStretch( 1.0 )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
    , mbWindow( false )
{
    long nDummyX, nDummyY;
    GetResolution( nDummyX, nDummyY );

    // init colors
    for(int i=0; i<3; i++)
    {
        mpFillColor[i] = 0.0;   // white
        mpLineColor[i] = 1.0;   // black
    }
    mpFillColor[3] = mpLineColor[3] = 1.0; // opaque colors

    // create the style object for font attributes
    ATSUCreateStyle( &maATSUStyle );
}

// -----------------------------------------------------------------------

AquaSalGraphics::~AquaSalGraphics()
{
/*
    if( mnUpdateGraphicsEvent )
    {
        Application::RemoveUserEvent( mnUpdateGraphicsEvent );
    }
*/
    CGPathRelease( mrClippingPath );
    ATSUDisposeStyle( maATSUStyle );

    if( mrContext && mbWindow )
    {
        // destroy backbuffer bitmap context that we created ourself
        CFRelease( mrContext );
        mrContext = 0;
        // memory is freed automatically by maOwnContextMemory
    }
}

bool AquaSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    bool bRet = false;
    switch( eType )
    {
    case OutDevSupport_TransparentRect:
        bRet = true;
        break;
    default: break;
    }
    return bRet;
}

// =======================================================================

/** returns the display id this window is mostly visible on */
CGDirectDisplayID AquaSalGraphics::GetWindowDisplayID() const
{
    // TODO: Find the correct display!
    return CGMainDisplayID();
}

// ----------------------------------------------

static void GetDisplayResolution( long& rDPIX, long& rDPIY )
{
    // calculate resolution from physical size and pixel count
    const CGDirectDisplayID nDisplayID = CGMainDisplayID();
    const CGSize aSize = CGDisplayScreenSize( nDisplayID ); // => result is in millimeters
    rDPIX = static_cast<long>((CGDisplayPixelsWide( nDisplayID ) * 25.4) / aSize.width);
    rDPIY = static_cast<long>((CGDisplayPixelsHigh( nDisplayID ) * 25.4) / aSize.height);

    // equalize x- and y-resolution if they are close enough to prevent unneeded font stretching
    if( (rDPIX != rDPIY)
    &&  (10*rDPIX < 13*rDPIY) && (13*rDPIX > 10*rDPIY) )
    {
        rDPIX = rDPIY = (rDPIX + rDPIY + 1) / 2;
    }
}

void AquaSalGraphics::updateResolution()
{
    DBG_ASSERT( mbWindow, "updateResolution on inappropriate graphics" );
    GetDisplayResolution( mnRealDPIX, mnRealDPIY );
    mfFakeDPIScale = 1.0;
}

void AquaSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    if( !mnRealDPIY )
    {
        GetDisplayResolution( mnRealDPIX, mnRealDPIY );
        mfFakeDPIScale = 1.0;
    }

    rDPIX = static_cast<long>(mfFakeDPIScale * mnRealDPIX);
    rDPIY = static_cast<long>(mfFakeDPIScale * mnRealDPIY);
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    GetResolution( rDPIX, rDPIY );

    // the screen font resolution should equal the real resolution
    // but to satisfy the quite insane heuristics in Window::ImplUpdateGlobalSettings()
    // it needs to be tweaked
    // TODO: remove the tweaking below if it becomes possible
    if( (rDPIX < 72) || (rDPIY < 72) )
    {
        const long nMinDPI = (rDPIX <= rDPIY) ? rDPIX : rDPIY;
        rDPIX = (72 * rDPIX + (nMinDPI/2)) / nMinDPI;
        rDPIY = (72 * rDPIY + (nMinDPI/2)) / nMinDPI;
    }
}

// -----------------------------------------------------------------------

USHORT AquaSalGraphics::GetBitCount()
{
    USHORT nBits = 0;
    if( CheckContext() )
    {
        if( mbPrinter )
            nBits = 24;
        else
            nBits = static_cast<USHORT>(CGBitmapContextGetBitsPerPixel(mrContext));
    }
    return nBits;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::ResetClipRegion()
{
    // release old path and indicate no clipping
    CGPathRelease( mrClippingPath );
    mrClippingPath = NULL;
    maXORDevice.reset();
    maXORClipMask.reset();
    if( CheckContext() )
        SetState();
}

// -----------------------------------------------------------------------

void AquaSalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
    // release old path
    if( mrClippingPath )
    {
        CGPathRelease( mrClippingPath );
        mrClippingPath = NULL;
    }
    maXORDevice.reset();
    maXORClipMask.reset();

    if( maClippingRects.size() > SAL_CLIPRECT_COUNT && nRectCount < maClippingRects.size() )
    {
        std::vector<CGRect> aEmptyVec;
        maClippingRects.swap( aEmptyVec );
    }
    maClippingRects.clear();
    maClippingRects.reserve( nRectCount );

}

// -----------------------------------------------------------------------

BOOL AquaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( nWidth && nHeight )
        maClippingRects.push_back( CGRectMake(nX, nY, nWidth, nHeight) );

    return TRUE;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::EndSetClipRegion()
{
    if( ! maClippingRects.empty() )
    {
        mrClippingPath = CGPathCreateMutable();
        CGPathAddRects( mrClippingPath, NULL, &maClippingRects[0], maClippingRects.size() );
    }
    if( CheckContext() )
        SetState();
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetLineColor()
{
    mpLineColor[3] = 0.0;   // set alpha component to 0

    CGContextSetStrokeColor( mrContext, mpLineColor );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetLineColor( SalColor nSalColor )
{
    mpLineColor[0] = (float) SALCOLOR_RED(nSalColor) / 255.0;
    mpLineColor[1] = (float) SALCOLOR_GREEN(nSalColor) / 255.0;
    mpLineColor[2] = (float) SALCOLOR_BLUE(nSalColor) / 255.0;
    mpLineColor[3] = 1.0;   // opaque

    CGContextSetStrokeColor( mrContext, mpLineColor );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetFillColor()
{
    mpFillColor[3] = 0.0;   // set alpha component to 0

    CGContextSetFillColor( mrContext, mpFillColor );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetFillColor( SalColor nSalColor )
{
    mpFillColor[0] = (float) SALCOLOR_RED(nSalColor) / 255.0;
    mpFillColor[1] = (float) SALCOLOR_GREEN(nSalColor) / 255.0;
    mpFillColor[2] = (float) SALCOLOR_BLUE(nSalColor) / 255.0;
    mpFillColor[3] = 1.0;   // opaque

    CGContextSetFillColor( mrContext, mpFillColor );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetXORMode( BOOL bSet )
{
    if( ! mbPrinter )
    {
        mbXORMode = bSet;
        maXORDevice.reset();
        maXORClipMask.reset();
        if( CheckContext() )
            CGContextSetBlendMode(mrContext, bSet ? kCGBlendModeDifference : kCGBlendModeNormal );
    }
}

// -----------------------------------------------------------------------

static SalColor ImplGetROPSalColor( SalROPColor nROPColor )
{
    SalColor nSalColor;
    if ( nROPColor == SAL_ROP_0 )
        nSalColor = MAKE_SALCOLOR( 0, 0, 0 );
    else
        nSalColor = MAKE_SALCOLOR( 255, 255, 255 );
    return nSalColor;
}

void AquaSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    if( ! mbPrinter )
        SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::ImplDrawPixel( long nX, long nY, float pColor[] )
{
    if ( CheckContext() )
    {
        if( mbXORMode )
        {
            basegfx::B2DRectangle aRect( nX, nY, nX+1, nY+1 );
            maXORDevice->fillPolyPolygon( basegfx::B2DPolyPolygon( basegfx::tools::createPolygonFromRect( aRect ) ),
                                      basebmp::Color( static_cast<unsigned int>(pColor[0]*255),
                                                      static_cast<unsigned int>(pColor[1]*255),
                                                      static_cast<unsigned int>(pColor[2]*255) ),
                                      basebmp::DrawMode_XOR,
                                      maXORClipMask
                                      );
        }
        else
        {
            CGContextSetFillColor( mrContext, pColor );
            // draw 1x1 rect, there is no pixel drawing in Quartz
            CGContextFillRect( mrContext, CGRectMake (nX, nY, 1, 1) );
            CGContextSetFillColor( mrContext, mpFillColor );
        }
    }
}

void AquaSalGraphics::drawPixel( long nX, long nY )
{
    // draw pixel with current line color
    ImplDrawPixel( nX, nY, mpLineColor );
    RefreshRect( nX, nY, 1, 1 );
}

void AquaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    // draw pixel with specified color
    float pColor[] =
    {
        (float) SALCOLOR_RED(nSalColor) / 255.0,
        (float) SALCOLOR_GREEN(nSalColor) / 255.0,
        (float) SALCOLOR_BLUE(nSalColor) / 255.0,
        1.0   // opaque
    };

    ImplDrawPixel(nX, nY, pColor);
    RefreshRect( nX, nY, 1, 1 );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if ( CheckContext() )
    {
        if( mbXORMode )
        {
            maXORDevice->drawLine( basegfx::B2IPoint( nX1, nY1 ),
                                   basegfx::B2IPoint( nX2, nY2 ),
                                   basebmp::Color( static_cast<unsigned int>(mpLineColor[0]*255),
                                                   static_cast<unsigned int>(mpLineColor[1]*255),
                                                   static_cast<unsigned int>(mpLineColor[2]*255) ),
                                   basebmp::DrawMode_XOR,
                                   maXORClipMask
                                   );
        }
        else
        {
            CGContextBeginPath( mrContext );
            CGContextMoveToPoint( mrContext, static_cast<float>(nX1)+0.5, static_cast<float>(nY1)+0.5 );
            CGContextAddLineToPoint( mrContext, static_cast<float>(nX2)+0.5, static_cast<float>(nY2)+0.5 );
            CGContextDrawPath( mrContext, kCGPathStroke );
        }
    }

    Rectangle aRefreshRect( nX1, nY1, nX2, nY2 );
    aRefreshRect.Justify();
    // magical offsets: protect against rounding issues between context and real pixels
    RefreshRect( aRefreshRect.Left()-1, aRefreshRect.Top()-1, aRefreshRect.GetWidth()+1, aRefreshRect.GetHeight()+1 );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if ( CheckContext() )
    {
        if( mbXORMode )
        {
            basegfx::B2DRectangle aRect( nX, nY, nX+nWidth, nY+nHeight );
            if( mpFillColor[3] > 0.0 )
            {
                maXORDevice->fillPolyPolygon( basegfx::B2DPolyPolygon( basegfx::tools::createPolygonFromRect( aRect ) ),
                                          basebmp::Color( static_cast<unsigned int>(mpFillColor[0]*255),
                                                          static_cast<unsigned int>(mpFillColor[1]*255),
                                                          static_cast<unsigned int>(mpFillColor[2]*255) ),
                                          basebmp::DrawMode_XOR,
                                          maXORClipMask
                                          );
            }
            if( mpLineColor[3] > 0.0 )
            {
                maXORDevice->drawPolygon( basegfx::B2DPolygon( basegfx::tools::createPolygonFromRect( aRect ) ),
                                          basebmp::Color( static_cast<unsigned int>(mpLineColor[0]*255),
                                                          static_cast<unsigned int>(mpLineColor[1]*255),
                                                          static_cast<unsigned int>(mpLineColor[2]*255) ),
                                          basebmp::DrawMode_XOR,
                                          maXORClipMask
                                          );
            }
        }
        else
        {
            CGRect aRect( CGRectMake(nX, nY, nWidth, nHeight) );
            if( ! IsPenTransparent() )
            {
                aRect.origin.x      += 0.5;
                aRect.origin.y      += 0.5;
                aRect.size.width    -= 1;
                aRect.size.height -= 1;
            }

            if( ! IsBrushTransparent() )
                CGContextFillRect( mrContext, aRect );

            if( ! IsPenTransparent() )
                CGContextStrokeRect( mrContext, aRect );
        }

        RefreshRect( nX, nY, nWidth, nHeight );
    }
}

// -----------------------------------------------------------------------

static void getBoundRect( ULONG nPoints, const SalPoint *pPtAry, long &rX, long& rY, long& rWidth, long& rHeight )
{
    long nX1 = pPtAry->mnX;
    long nX2 = nX1;
    long nY1 = pPtAry->mnY;
    long nY2 = nY1;
    for( ULONG n = 1; n < nPoints; n++ )
    {
        if( pPtAry[n].mnX < nX1 )
            nX1 = pPtAry[n].mnX;
        else if( pPtAry[n].mnX > nX2 )
            nX2 = pPtAry[n].mnX;

        if( pPtAry[n].mnY < nY1 )
            nY1 = pPtAry[n].mnY;
        else if( pPtAry[n].mnY > nY2 )
            nY2 = pPtAry[n].mnY;
    }
    rX = nX1;
    rY = nY1;
    rWidth = nX2 - nX1 + 1;
    rHeight = nY2 - nY1 + 1;
}

static inline void alignLinePoint( const SalPoint* i_pIn, float& o_fX, float& o_fY )
{
    o_fX = static_cast<float>(i_pIn->mnX ) + 0.5;
    o_fY = static_cast<float>(i_pIn->mnY ) + 0.5;
}

void AquaSalGraphics::drawPolyLine( ULONG nPoints, const SalPoint *pPtAry )
{
    if( (nPoints > 1) && CheckContext() )
    {
        long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
        getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );
        if( mbXORMode )
        {
            // build polygon
            basegfx::B2DPolygon aLine;
            for( ULONG i = 0; i < nPoints; i++ )
                aLine.append( basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );

            maXORDevice->drawPolygon( aLine,
                                      basebmp::Color( static_cast<unsigned int>(mpLineColor[0]*255),
                                                      static_cast<unsigned int>(mpLineColor[1]*255),
                                                      static_cast<unsigned int>(mpLineColor[2]*255) ),
                                      basebmp::DrawMode_XOR,
                                      maXORClipMask
                                      );
        }
        else
        {
            float fX, fY;

            CGContextBeginPath( mrContext );
            alignLinePoint( pPtAry, fX, fY );
            CGContextMoveToPoint( mrContext, fX, fY );
            pPtAry++;
            for( ULONG nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
            {
                alignLinePoint( pPtAry, fX, fY );
                CGContextAddLineToPoint( mrContext, fX, fY );
            }
            CGContextDrawPath( mrContext, kCGPathStroke );
        }

        RefreshRect( nX, nY, nWidth, nHeight );
    }
}

// -----------------------------------------------------------------------

static void ImplDrawPolygon( CGContextRef& xContext, ULONG nPoints, const SalPoint *pPtAry, float* pFillColor, float* pLineColor )
{
    CGPathDrawingMode eMode;
    if( pFillColor[3] > 0.0 && pLineColor[3] > 0.0 )
        eMode = kCGPathEOFillStroke;
    else if( pLineColor[3] > 0.0 )
        eMode = kCGPathStroke;
    else if( pFillColor[3] > 0.0 )
        eMode = kCGPathEOFill;
    else
        return;

    CGContextBeginPath( xContext );

    if( pLineColor[3] > 0.0 )
    {
        float fX, fY;
        alignLinePoint( pPtAry, fX, fY );
        CGContextMoveToPoint( xContext, fX, fY );
        pPtAry++;
        for( ULONG nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
        {
            alignLinePoint( pPtAry, fX, fY );
            CGContextAddLineToPoint( xContext, fX, fY );
        }
    }
    else
    {
        CGContextMoveToPoint( xContext, pPtAry->mnX, pPtAry->mnY );
        pPtAry++;
        for( ULONG nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
            CGContextAddLineToPoint( xContext, pPtAry->mnX, pPtAry->mnY );
    }

    CGContextDrawPath( xContext, eMode );
}

void AquaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint *pPtAry )
{
    if( (nPoints > 1) && CheckContext() )
    {
        long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
        getBoundRect( nPoints, pPtAry, nX, nY, nWidth, nHeight );
        if( mbXORMode )
        {
            // build polygon
            basegfx::B2DPolygon aPoly;
            for( ULONG i = 0; i < nPoints; i++ )
                aPoly.append( basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );

            if( mpFillColor[3] > 0.0 )
            {
                maXORDevice->fillPolyPolygon( basegfx::B2DPolyPolygon( aPoly ),
                                              basebmp::Color( static_cast<unsigned int>(mpFillColor[0]*255),
                                                              static_cast<unsigned int>(mpFillColor[1]*255),
                                                              static_cast<unsigned int>(mpFillColor[2]*255) ),
                                              basebmp::DrawMode_XOR,
                                              maXORClipMask
                                              );
            }
            if( mpLineColor[3] > 0.0 )
            {
                maXORDevice->drawPolygon( aPoly,
                                          basebmp::Color( static_cast<unsigned int>(mpLineColor[0]*255),
                                                          static_cast<unsigned int>(mpLineColor[1]*255),
                                                          static_cast<unsigned int>(mpLineColor[2]*255) ),
                                          basebmp::DrawMode_XOR,
                                          maXORClipMask
                                          );
            }
        }
        else
            ImplDrawPolygon( mrContext, nPoints, pPtAry, mpFillColor, mpLineColor );

        RefreshRect( nX, nY, nWidth, nHeight );
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawPolyPolygon( ULONG nPolyCount, const ULONG *pPoints, PCONSTSALPOINT  *ppPtAry )
{
    if( nPolyCount > 0 && CheckContext() )
    {
        // find bound rect
        long leftX = 0, topY = 0, maxWidth = 0, maxHeight = 0;
        getBoundRect( pPoints[0], ppPtAry[0], leftX, topY, maxWidth, maxHeight );
        for( ULONG n = 1; n < nPolyCount; n++ )
        {
            long nX = leftX, nY = topY, nW = maxWidth, nH = maxHeight;
            getBoundRect( pPoints[n], ppPtAry[n], nX, nY, nW, nH );
            if( nX < leftX )
            {
                maxWidth += leftX - nX;
                leftX = nX;
            }
            if( nY < topY )
            {
                maxHeight += topY - nY;
                topY = nY;
            }
            if( nX + nW > leftX + maxWidth )
                maxWidth = nX + nW - leftX;
            if( nY + nH > topY + maxHeight )
                maxHeight = nY + nH - topY;
        }

        if( mbXORMode )
        {
            // build Path
            basegfx::B2DPolyPolygon aPolyPoly;
            for( ULONG nPoly = 0; nPoly < nPolyCount; nPoly++ )
            {
                const ULONG nPoints = pPoints[nPoly];
                if( nPoints > 1 )
                {
                    const SalPoint *pPtAry = ppPtAry[nPoly];
                    basegfx::B2DPolygon aPoly;
                    for( ULONG i = 0; i < nPoints; i++ )
                        aPoly.append( basegfx::B2DPoint( pPtAry[i].mnX, pPtAry[i].mnY ) );
                    aPoly.setClosed( pPtAry[0].mnX == pPtAry[nPoints-1].mnX &&
                                     pPtAry[0].mnY == pPtAry[nPoints-1].mnY );
                    aPolyPoly.append( aPoly );
                }
            }
            if( mpFillColor[3] > 0.0 )
            {
                maXORDevice->fillPolyPolygon( aPolyPoly,
                                              basebmp::Color( static_cast<unsigned int>(mpFillColor[0]*255),
                                                              static_cast<unsigned int>(mpFillColor[1]*255),
                                                              static_cast<unsigned int>(mpFillColor[2]*255) ),
                                              basebmp::DrawMode_XOR,
                                              maXORClipMask
                                              );
            }
            if( mpLineColor[3] > 0.0 )
            {
                for( ULONG nPoly = 0; nPoly < nPolyCount; nPoly++ )
                {
                    maXORDevice->drawPolygon( aPolyPoly.getB2DPolygon( nPoly ),
                                              basebmp::Color( static_cast<unsigned int>(mpLineColor[0]*255),
                                                              static_cast<unsigned int>(mpLineColor[1]*255),
                                                              static_cast<unsigned int>(mpLineColor[2]*255) ),
                                              basebmp::DrawMode_XOR,
                                              maXORClipMask
                                              );
                }
            }
        }
        else
        {
            CGPathDrawingMode eMode;
            if( mpFillColor[3] > 0.0 && mpLineColor[3] > 0.0 )
                eMode = kCGPathEOFillStroke;
            else if( mpLineColor[3] > 0.0 )
                eMode = kCGPathStroke;
            else if( mpFillColor[3] > 0.0 )
                eMode = kCGPathEOFill;
            else
                return;

            CGContextBeginPath( mrContext );
            if( mpLineColor[ 3 ] > 0.0 )
            {
                for( ULONG nPoly = 0; nPoly < nPolyCount; nPoly++ )
                {
                    const ULONG nPoints = pPoints[nPoly];
                    if( nPoints > 1 )
                    {
                        const SalPoint *pPtAry = ppPtAry[nPoly];
                        float fX, fY;
                        alignLinePoint( pPtAry, fX, fY );
                        CGContextMoveToPoint( mrContext, fX, fY );
                        pPtAry++;
                        for( ULONG nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                        {
                            alignLinePoint( pPtAry, fX, fY );
                            CGContextAddLineToPoint( mrContext, fX, fY );
                        }
                        CGContextClosePath(mrContext);
                    }
                }
            }
            else
            {
                for( ULONG nPoly = 0; nPoly < nPolyCount; nPoly++ )
                {
                    const ULONG nPoints = pPoints[nPoly];
                    if( nPoints > 1 )
                    {
                        const SalPoint *pPtAry = ppPtAry[nPoly];
                        CGContextMoveToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                        pPtAry++;
                        for( ULONG nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++ )
                            CGContextAddLineToPoint( mrContext, pPtAry->mnX, pPtAry->mnY );
                        CGContextClosePath(mrContext);
                    }
                }
            }

            CGContextDrawPath( mrContext, eMode );
        }

        RefreshRect( leftX, topY, maxWidth, maxHeight );
    }
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalGraphics::drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
    return sal_False;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::copyBits( const SalTwoRect *pPosAry, SalGraphics *pSrcGraphics )
{
    if( !pSrcGraphics )
        pSrcGraphics = this;

    //from unix salgdi2.cxx
    //[FIXME] find a better way to prevent calc from crashing when width and height are negative
    if( pPosAry->mnSrcWidth <= 0
        || pPosAry->mnSrcHeight <= 0
        || pPosAry->mnDestWidth <= 0
        || pPosAry->mnDestHeight <= 0 )
    {
        return;
    }

    SalBitmap* pBitmap = static_cast<AquaSalGraphics*>(pSrcGraphics)->getBitmap( pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight );

    if( pBitmap )
    {
        SalTwoRect aPosAry( *pPosAry );
        aPosAry.mnSrcX = 0;
        aPosAry.mnSrcY = 0;
        drawBitmap( &aPosAry, *pBitmap );
        delete pBitmap;
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::copyArea( long nDstX, long nDstY,long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, USHORT nFlags )
{
    SalBitmap* pBitmap = getBitmap( nSrcX, nSrcY, nSrcWidth, nSrcHeight );
    if( pBitmap )
    {
        SalTwoRect aPosAry;
        aPosAry.mnSrcX = 0;
        aPosAry.mnSrcY = 0;
        aPosAry.mnSrcWidth = nSrcWidth;
        aPosAry.mnSrcHeight = nSrcHeight;
        aPosAry.mnDestX = nDstX;
        aPosAry.mnDestY = nDstY;
        aPosAry.mnDestWidth = nSrcWidth;
        aPosAry.mnDestHeight = nSrcHeight;
        drawBitmap( &aPosAry, *pBitmap );
        delete pBitmap;
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    const AquaSalBitmap& rBitmap = static_cast<const AquaSalBitmap&>(rSalBitmap);
    if( CheckContext() )
    {
        if( mbXORMode )
        {
            basebmp::BitmapDeviceSharedPtr aBmp( rBitmap.getBitmapDevice() );
            maXORDevice->drawBitmap( aBmp,
                basegfx::B2IRange( pPosAry->mnSrcX, pPosAry->mnSrcY,
                                   pPosAry->mnSrcX+pPosAry->mnSrcWidth, pPosAry->mnSrcY+pPosAry->mnSrcHeight ),
                basegfx::B2IRange( pPosAry->mnDestX, pPosAry->mnDestY,
                                   pPosAry->mnDestX+pPosAry->mnDestWidth, pPosAry->mnDestY+pPosAry->mnDestHeight ),
                basebmp::DrawMode_XOR,
                maXORClipMask );
        }
        else
        {
            CGImageRef xImage = const_cast< AquaSalBitmap& >( rBitmap ).CreateCroppedImage( (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY, (int)pPosAry->mnSrcWidth, (int)pPosAry->mnSrcHeight );
            CGContextDrawImage(mrContext, CGRectMake ((int)pPosAry->mnDestX, (int)pPosAry->mnDestY, (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight), xImage);
            CGImageRelease(xImage);
        }
        RefreshRect((int)pPosAry->mnDestX, (int)pPosAry->mnDestY, (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight);
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,SalColor nTransparentColor )
{
    DBG_ERROR("not implemented for color masking!");
    drawBitmap( pPosAry, rSalBitmap );
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rTransparentBitmap )
{
    AquaSalBitmap& rBitmap = const_cast< AquaSalBitmap& >( static_cast<const AquaSalBitmap&>(rSalBitmap) );
    const AquaSalBitmap& rMask = static_cast<const AquaSalBitmap&>(rTransparentBitmap);
    if( CheckContext() )
    {
        // FIXME: XOR ?

        CGImageRef xMaskedImage( rBitmap.CreateWithMask( rMask, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight ) );
        if( xMaskedImage )
        {
            CGContextDrawImage(mrContext, CGRectMake (pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight), xMaskedImage);
            RefreshRect((int)pPosAry->mnDestX, (int)pPosAry->mnDestY, (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight);
            CGImageRelease(xMaskedImage);
        }
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::drawMask( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor )
{
    const AquaSalBitmap& rBitmap = static_cast<const AquaSalBitmap&>(rSalBitmap);

    if ( CheckContext() )
    {
        if( mbXORMode )
        {
            basebmp::BitmapDeviceSharedPtr aMask( rBitmap.getBitmapDevice() );
            maXORDevice->drawMaskedColor( basebmp::Color( nMaskColor ),
                                          aMask,
                                          basegfx::B2IRange( pPosAry->mnSrcX, pPosAry->mnSrcY,
                                              pPosAry->mnSrcX+pPosAry->mnSrcWidth, pPosAry->mnSrcY+pPosAry->mnSrcHeight ),
                                          basegfx::B2IPoint( pPosAry->mnDestX, pPosAry->mnDestY ),
                                          maXORClipMask );

        }
        else
        {
            CGImageRef xImage = rBitmap.CreateColorMask( pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, nMaskColor );
            if( xImage )
            {
                CGContextDrawImage(mrContext, CGRectMake (pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight), xImage);
                CGImageRelease(xImage);
            }
        }
        RefreshRect((int)pPosAry->mnDestX, (int)pPosAry->mnDestY, (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight);
    }
}

// -----------------------------------------------------------------------

SalBitmap* AquaSalGraphics::getBitmap( long  nX, long  nY, long  nDX, long  nDY )
{
    AquaSalBitmap* pBitmap = NULL;

    if( mrContext )
    {
        pBitmap = new AquaSalBitmap;
        if( !pBitmap->Create( mrContext, nX, nY, nDX, nDY, ! mbWindow ) )
        {
            delete pBitmap;
            pBitmap = 0;
        }
    }

    return pBitmap;
}

// -----------------------------------------------------------------------

SalColor AquaSalGraphics::getPixel( long nX, long nY )
{
    SalColor  nSalColor = 0;
    return nSalColor;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if ( CheckContext() )
    {
        CGRect aCGRect = CGRectMake( nX, nY, nWidth, nHeight);
        CGContextSaveGState(mrContext);

        if ( nFlags & SAL_INVERT_TRACKFRAME )
        {
            const float dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            CGContextStrokeRect ( mrContext, aCGRect );
        }
        else if ( nFlags & SAL_INVERT_50 )
        {
            //workaround
            // no xor in Core Graphics, so just invert
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextSetRGBFillColor( mrContext,1.0, 1.0, 1.0 , 1.0 );
            CGContextFillRect ( mrContext, aCGRect );
        }
        else // just invert
        {
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextSetRGBFillColor ( mrContext,1.0, 1.0, 1.0 , 1.0 );
            CGContextFillRect ( mrContext, aCGRect );
        }
        CGContextRestoreGState( mrContext);
        RefreshRect( nX, nY, nWidth, nHeight );
    }
}

// -----------------------------------------------------------------------

void AquaSalGraphics::invert( ULONG nPoints, const SalPoint*  pPtAry, SalInvert nSalFlags )
{
    CGPoint* CGpoints ;
    if ( CheckContext() )
    {
        CGContextSaveGState(mrContext);
        CGpoints = makeCGptArray(nPoints,pPtAry);
        CGContextAddLines ( mrContext, CGpoints, nPoints );
        if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        {
            const float dashLengths[2]  = { 4.0, 4.0 };     // for drawing dashed line
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBStrokeColor ( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextSetLineDash ( mrContext, 0, dashLengths, 2 );
            CGContextSetLineWidth( mrContext, 2.0);
            CGContextStrokePath ( mrContext );
        }
        else if ( nSalFlags & SAL_INVERT_50 )
        {
            // workaround
            // no xor in Core Graphics, so just invert
            CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
            CGContextSetRGBFillColor( mrContext,1.0, 1.0, 1.0 , 1.0 );
            CGContextFillPath( mrContext );
        }
        else // just invert
        {
            CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
            CGContextSetRGBFillColor( mrContext, 1.0, 1.0, 1.0, 1.0 );
            CGContextFillPath( mrContext );
        }
        CGRect pRect = CGContextGetClipBoundingBox(mrContext);
        CGContextRestoreGState( mrContext);
        delete []  CGpoints;
        RefreshRect(pRect.origin.x, pRect.origin.y, pRect.size.width, pRect.size.height);
    }
}

// -----------------------------------------------------------------------

BOOL AquaSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void*  pPtr, ULONG nSize )
{
    return FALSE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool AquaSalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap, const SalBitmap& rAlphaBmp )
{

    // An image mask can't have a depth > 8 bits (should be 1 to 8 bits)
    if( rAlphaBmp.GetBitCount() > 8 )
        return false;

    // are these two tests really necessary? (see vcl/unx/source/gdi/salgdi2.cxx)
    // horizontal/vertical mirroring not implemented yet
    if( rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0 )
        return false;

    // stretched conversion is not implemented yet
    if( rTR.mnDestWidth != rTR.mnSrcWidth )
        return false;
    if( rTR.mnDestHeight!= rTR.mnSrcHeight )
        return false;

    AquaSalBitmap& pSrcSalBmp = const_cast< AquaSalBitmap& >( static_cast<const AquaSalBitmap&>(rSrcBitmap));
    const AquaSalBitmap& pMaskSalBmp = static_cast<const AquaSalBitmap &>(rAlphaBmp);

    CGImageRef xMaskedImage=pSrcSalBmp.CreateWithMask( pMaskSalBmp, rTR.mnSrcX, rTR.mnSrcY, rTR.mnSrcWidth, rTR.mnSrcHeight );

    if(!xMaskedImage) return FALSE;

    if ( CheckContext() )
    {
        CGContextDrawImage(mrContext, CGRectMake( rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight), xMaskedImage );
        RefreshRect(rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);
    }

    CGImageRelease(xMaskedImage);

    return true;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool AquaSalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                     long nHeight, sal_uInt8 nTransparency )
{
    if ( CheckContext() )
    {
        CGPathDrawingMode eMode;
        if( mpFillColor[3] > 0.0 && mpLineColor[3] > 0.0 )
            eMode = kCGPathEOFillStroke;
        else if( mpLineColor[3] > 0.0 )
            eMode = kCGPathStroke;
        else if( mpFillColor[3] > 0.0 )
            eMode = kCGPathEOFill;
        else
            return false;

        CGRect aRect;
        aRect.origin.x    = static_cast<float>(nX);
        aRect.origin.y    = static_cast<float>(nY);
        aRect.size.width  = nWidth-1;
        aRect.size.height = nHeight-1;

        // save the current state
        CGContextSaveGState(mrContext);

        // set transparent fill/line colors
        if( mpFillColor[3] > 0.0 )
        {
            float fOldFillAlpha = mpFillColor[3]; //  save the old alpha
            // OOo transparency value of 0 corresponds to the Quartz transparency value of 1,
            // and the OOo transparency value of 100 corresponds to the quartz transparency value of 0
            mpFillColor[3] =  ((float)100-nTransparency) / 100; //  set the new alpha
            CGContextSetFillColor( mrContext, mpFillColor ); // using our color and new alpha
            mpFillColor[3] = fOldFillAlpha; // reset the old value
        }
        if( mpLineColor[3] )
        {
            float fOldLineAlpha = mpLineColor[3];
            mpLineColor[3] = ((float)100-nTransparency) / 100;
            CGContextSetStrokeColor( mrContext, mpLineColor );
            mpLineColor[3] = fOldLineAlpha;

            aRect.origin.x += 0.5;
            aRect.origin.y += 0.5;
        }

        CGContextBeginPath( mrContext );
        CGContextAddRect( mrContext, aRect );
        CGContextDrawPath( mrContext, eMode );

        // restore state
        CGContextRestoreGState(mrContext);
        RefreshRect(nX, nY, nWidth, nHeight);
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::SetTextColor( SalColor nSalColor )
{
    RGBColor color;
    color.red     = (unsigned short) ( SALCOLOR_RED(nSalColor)   * 65535.0 / 255.0 );
    color.green   = (unsigned short) ( SALCOLOR_GREEN(nSalColor) * 65535.0 / 255.0 );
    color.blue    = (unsigned short) ( SALCOLOR_BLUE(nSalColor)  * 65535.0 / 255.0 );

    ATSUAttributeTag aTag = kATSUColorTag;
    ByteCount aValueSize = sizeof( color );
    ATSUAttributeValuePtr aValue = &color;

    OSStatus err = ATSUSetAttributes( maATSUStyle, 1, &aTag, &aValueSize, &aValue );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::SetTextColor() : Could not set font attributes!\n");
    if( err != noErr )
        return;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    // get the ATSU font metrics (in point units)
    // of the font that has eventually been size-limited

    ATSUFontID fontId;
    OSStatus err = ATSUGetAttribute( maATSUStyle, kATSUFontTag, sizeof(ATSUFontID), &fontId, 0 );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font id\n");

    ATSFontMetrics aMetrics;
    ATSFontRef rFont = FMGetATSFontRefFromFont( fontId );
    err = ATSFontGetHorizontalMetrics ( rFont, kATSOptionFlagsDefault, &aMetrics );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font metrics\n");
    if( err != noErr )
        return;

    // all ATS fonts are scalable fonts
    pMetric->mbScalableFont = true;
    // TODO: check if any kerning is possible
    pMetric->mbKernableFont = true;

    // convert into VCL font metrics (in unscaled pixel units)

    Fixed ptSize;
    err = ATSUGetAttribute( maATSUStyle, kATSUSizeTag, sizeof(Fixed), &ptSize, 0);
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font size\n");
    const double fPointSize = Fix2X( ptSize );

    // convert quartz units to pixel units
    // please see the comment in AquaSalGraphics::SetFont() for details
    const double fPixelSize = (mfFontScale * mfFakeDPIScale * fPointSize);
    pMetric->mnAscent       = static_cast<long>(+aMetrics.ascent  * fPixelSize + 0.5);
    pMetric->mnDescent      = static_cast<long>((-aMetrics.descent + aMetrics.leading) * fPixelSize + 0.5);
    pMetric->mnIntLeading   = 0;
    pMetric->mnExtLeading   = 0;
    // ATSFontMetrics.avgAdvanceWidth is obsolete, so it is usually set to zero
    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    pMetric->mnWidth        = static_cast<long>(mfFontStretch * fPixelSize + 0.5);

    // apply the "CJK needs extra leading" heuristic if needed
#if 0 // #i85422# on MacOSX the CJK fonts have good metrics even without the heuristics below
    if( mpMacFontData->HasCJKSupport() )
    {
        pMetric->mnIntLeading   += pMetric->mnExtLeading;

        const long nHalfTmpExtLeading = pMetric->mnExtLeading / 2;
        const long nOtherHalfTmpExtLeading = pMetric->mnExtLeading - nHalfTmpExtLeading;

        long nCJKExtLeading = static_cast<long>(0.30 * (pMetric->mnAscent + pMetric->mnDescent));
        nCJKExtLeading -= pMetric->mnExtLeading;
        pMetric->mnExtLeading = (nCJKExtLeading > 0) ? nCJKExtLeading : 0;

        pMetric->mnAscent   += nHalfTmpExtLeading;
        pMetric->mnDescent  += nOtherHalfTmpExtLeading;
    }
#endif
}

// -----------------------------------------------------------------------

ULONG AquaSalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData*  pKernPairs )
{
    return 0;
}

// -----------------------------------------------------------------------

static bool AddTempFontDir( void )
{
    static bool bFirst = true;
    if( !bFirst )
        return false;
    bFirst = false;

    // determine path for the OOo temporary font files
    // since we are only interested in fonts that could not be
    // registered before because of missing administration rights
    // only the font path of the user installation is needed
    // TODO: is there an easier way to find the friggin directory name???
    ::rtl::OUString aExecPath;
    osl_getExecutableFile( &aExecPath.pData );
    ::rtl::OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( aExecPath, aUSystemPath ) );
    ::rtl::OString aPathName = rtl::OUStringToOString( aUSystemPath, RTL_TEXTENCODING_UTF8 );
    aPathName = aPathName.copy( 0, aPathName.lastIndexOf('/') );
    aPathName += "/../share/fonts/truetype/";

    FSRef aPathFSRef;
    Boolean bIsDirectory = true;
    OSStatus eStatus = FSPathMakeRef( (UInt8*)aPathName.getStr(),
        &aPathFSRef, &bIsDirectory );
    if( eStatus != noErr )
        return false;

    FSSpec aPathFSSpec;
    eStatus = ::FSGetCatalogInfo( &aPathFSRef, kFSCatInfoNone,
        NULL, NULL, &aPathFSSpec, NULL );
    if( eStatus != noErr )
        return false;

    // TODO: deactivate ATSFontContainerRef when closing app
    ATSFontContainerRef aATSFontContainer;

    const ATSFontContext eContext = kATSFontContextLocal; // TODO: *Global???
    eStatus = ::ATSFontActivateFromFileSpecification( &aPathFSSpec,
        eContext, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault,
        &aATSFontContainer );
    if( eStatus != noErr )
        return false;

    return true;
}

void AquaSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( pFontList, "AquaSalGraphics::GetDevFontList(NULL) !");

    AddTempFontDir();

    // The idea is to cache the list of system fonts once it has been generated.
    // SalData seems to be a good place for this caching. However we have to
    // carefully make the access to the font list thread-safe. If we register
    // a font-change event handler to update the font list in case fonts have
    // changed on the system we have to lock access to the list.
    ::vos::OMutex aMutex;
    aMutex.acquire();
    SalData* pSalData = GetSalData();
    if (pSalData->mpFontList == NULL)
        pSalData->mpFontList = new SystemFontList();
    aMutex.release();

    // Copy all ImplFontData objects contained in the SystemFontList
    pSalData->mpFontList->AnnounceFonts( *pFontList );
}

// -----------------------------------------------------------------------

bool AquaSalGraphics::AddTempDevFont( ImplDevFontList* pFontList,
    const String& rFontFileURL, const String& rFontName )
{
    ::rtl::OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    FSRef aNewRef;
    Boolean bIsDirectory = true;
    ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, RTL_TEXTENCODING_UTF8 );
    OSStatus eStatus = FSPathMakeRef( (UInt8*)aCFileName.getStr(), &aNewRef, &bIsDirectory );
    if( eStatus != noErr )
        return false;

    FSSpec aFontFSSpec;
    eStatus = ::FSGetCatalogInfo( &aNewRef, kFSCatInfoNone,
        NULL, NULL, &aFontFSSpec, NULL );
    if( eStatus != noErr )
        return false;

    ATSFontContainerRef oContainer;

    const ATSFontContext eContext = kATSFontContextLocal; // TODO: *Global???
    eStatus = ::ATSFontActivateFromFileSpecification( &aFontFSSpec,
        eContext, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault,
        &oContainer );
    if( eStatus != noErr )
        return false;

    // TODO: ATSFontDeactivate( oContainer ) when fonts are no longer needed
    // TODO: register new ImplMacFontdata in pFontList
    return true;
}

// -----------------------------------------------------------------------

BOOL AquaSalGraphics::GetGlyphOutline( long nIndex, basegfx::B2DPolyPolygon& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

long AquaSalGraphics::GetGraphicsWidth() const
{
    if( mrContext && (mbWindow || mbVirDev) )
    {
        return CGBitmapContextGetWidth( mrContext );
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL AquaSalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& )
{
    return sal_False;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // nothing to do since there are no device-specific fonts on Aqua
}

// -----------------------------------------------------------------------

void AquaSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

// -----------------------------------------------------------------------

USHORT AquaSalGraphics::SetFont( ImplFontSelectData* pReqFont, int nFallbackLevel )
{
    if( !pReqFont )
    {
        ATSUClearStyle( maATSUStyle );
        mpMacFontData = NULL;
        return 0;
    }

    // store the requested device font entry
    const ImplMacFontData* pMacFont = static_cast<const ImplMacFontData*>( pReqFont->mpFontData );
    mpMacFontData = pMacFont;

    // convert pixel units (as seen by upper layers) to typographic point units
    double fScaledAtsHeight = pReqFont->mfExactHeight;
    // avoid Fixed16.16 overflows by limiting the ATS font size
    static const float fMaxAtsHeight = 144.0;
    if( fScaledAtsHeight <= fMaxAtsHeight )
        mfFontScale = 1.0;
    else
    {
        mfFontScale = fScaledAtsHeight / fMaxAtsHeight;
        fScaledAtsHeight = fMaxAtsHeight;
    }
    Fixed fFixedSize = FloatToFixed( fScaledAtsHeight );
    // enable bold-emulation if needed
    Boolean bFakeBold = FALSE;
    if( (pReqFont->GetWeight() >= WEIGHT_BOLD)
    &&  (pMacFont->GetWeight() < WEIGHT_SEMIBOLD) )
        bFakeBold = TRUE;
    // enable italic-emulation if needed
    Boolean bFakeItalic = FALSE;
    if( ((pReqFont->GetSlant() == ITALIC_NORMAL) || (pReqFont->GetSlant() == ITALIC_OBLIQUE))
    && !((pMacFont->GetSlant() == ITALIC_NORMAL) || (pMacFont->GetSlant() == ITALIC_OBLIQUE)) )
        bFakeItalic = TRUE;

    // enable/disable antialiased text
    mbNonAntialiasedText = pReqFont->mbNonAntialiased;
    UInt32 nStyleRenderingOptions = kATSStyleNoOptions;
    if( pReqFont->mbNonAntialiased )
        nStyleRenderingOptions |= kATSStyleNoAntiAliasing;

    // prepare ATS-fontid as type matching to the kATSUFontTag request
    ATSUFontID nFontID = static_cast<ATSUFontID>(pMacFont->GetFontId());

    // update ATSU style attributes with requested font parameters

    const ATSUAttributeTag aTag[] =
    {
        kATSUFontTag,
        kATSUSizeTag,
        kATSUQDBoldfaceTag,
        kATSUQDItalicTag,
        kATSUStyleRenderingOptionsTag
    };

    const ByteCount aValueSize[] =
    {
        sizeof(ATSUFontID),
        sizeof(fFixedSize),
        sizeof(bFakeBold),
        sizeof(bFakeItalic),
        sizeof(nStyleRenderingOptions)
    };

    const ATSUAttributeValuePtr aValue[] =
    {
        &nFontID,
        &fFixedSize,
        &bFakeBold,
        &bFakeItalic,
        &nStyleRenderingOptions
    };

    OSStatus eStatus = ATSUSetAttributes( maATSUStyle,
                             sizeof(aTag) / sizeof(ATSUAttributeTag),
                             aTag, aValueSize, aValue );
    DBG_ASSERT( (eStatus==noErr), "AquaSalGraphics::SetFont() : Could not set font attributes!\n");

    // prepare font stretching
    const ATSUAttributeTag aMatrixTag = kATSUFontMatrixTag;
    if( (pReqFont->mnWidth == 0) || (pReqFont->mnWidth == pReqFont->mnHeight) )
    {
        mfFontStretch = 1.0;
        ATSUClearAttributes( maATSUStyle, 1, &aMatrixTag );
    }
    else
    {
        mfFontStretch = (float)pReqFont->mnWidth / pReqFont->mnHeight;
        CGAffineTransform aMatrix = CGAffineTransformMakeScale( mfFontStretch, 1.0F );
        const ATSUAttributeValuePtr aAttr = &aMatrix;
        const ByteCount aMatrixBytes = sizeof(aMatrix);
        eStatus = ATSUSetAttributes( maATSUStyle, 1, &aMatrixTag, &aMatrixBytes, &aAttr );
        DBG_ASSERT( (eStatus==noErr), "AquaSalGraphics::SetFont() : Could not set font matrix\n");
    }

    // prepare font rotation
    mnATSUIRotation = FloatToFixed( pReqFont->mnOrientation / 10.0 );

#if OSL_DEBUG_LEVEL > 3
    fprintf( stderr, "SetFont to (\"%s\", \"%s\", fontid=%d) for (\"%s\" \"%s\" weight=%d, slant=%d size=%dx%d orientation=%d)\n",
             ::rtl::OUStringToOString( pMacFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             ::rtl::OUStringToOString( pMacFont->GetStyleName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             (int)nFontID,
             ::rtl::OUStringToOString( pReqFont->GetFamilyName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             ::rtl::OUStringToOString( pReqFont->GetStyleName(), RTL_TEXTENCODING_UTF8 ).getStr(),
             pReqFont->GetWeight(),
             pReqFont->GetSlant(),
             pReqFont->mnHeight,
             pReqFont->mnWidth,
             pReqFont->mnOrientation);
#endif

    return 0;
}

// -----------------------------------------------------------------------

ImplFontCharMap* AquaSalGraphics::GetImplFontCharMap() const
{
    if( !mpMacFontData )
        return ImplFontCharMap::GetDefaultMap();

    return mpMacFontData->GetImplFontCharMap();
}

// -----------------------------------------------------------------------

// fake a SFNT font directory entry for a font table
// see http://developer.apple.com/textfonts/TTRefMan/RM06/Chap6.html#Directory
static void FakeDirEntry( FourCharCode eFCC, ByteCount nOfs, ByteCount nLen,
    const unsigned char* /*pData*/, unsigned char*& rpDest )
{
    // write entry tag
    rpDest[ 0] = (char)(eFCC >> 24);
    rpDest[ 1] = (char)(eFCC >> 16);
    rpDest[ 2] = (char)(eFCC >>  8);
    rpDest[ 3] = (char)(eFCC >>  0);
    // TODO: get entry checksum and write it
    //      not too important since the subsetter doesn't care currently
    //      for( pData+nOfs ... pData+nOfs+nLen )
    // write entry offset
    rpDest[ 8] = (char)(nOfs >> 24);
    rpDest[ 9] = (char)(nOfs >> 16);
    rpDest[10] = (char)(nOfs >>  8);
    rpDest[11] = (char)(nOfs >>  0);
    // write entry length
    rpDest[12] = (char)(nLen >> 24);
    rpDest[13] = (char)(nLen >> 16);
    rpDest[14] = (char)(nLen >>  8);
    rpDest[15] = (char)(nLen >>  0);
    // advance to next entry
    rpDest += 16;
}

static bool GetRawFontData( const ImplFontData* pFontData,
    ByteVector& rBuffer )
{
    const ImplMacFontData* pMacFont = static_cast<const ImplMacFontData*>(pFontData);
    const ATSUFontID nFontId = static_cast<ATSUFontID>(pMacFont->GetFontId());
    ATSFontRef rFont = FMGetATSFontRefFromFont( nFontId );

    // get font table availability and size in bytes
    ByteCount nHeadLen  = 0;
    OSStatus eStatus = ATSFontGetTable( rFont, GetTag("head"), 0, 0, NULL, &nHeadLen);
    if( (eStatus != noErr) || (nHeadLen <= 0) )
        return false;
    ByteCount nMaxpLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("maxp"), 0, 0, NULL, &nMaxpLen);
    if( (eStatus != noErr) || (nMaxpLen <= 0) )
        return false;
    ByteCount nCmapLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, 0, NULL, &nCmapLen);
    if( (eStatus != noErr) || (nCmapLen <= 0) )
        return false;
    ByteCount nLocaLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("loca"), 0, 0, NULL, &nLocaLen);
    if( (eStatus != noErr) || (nLocaLen <= 0) )
        return false;
    ByteCount nGlyfLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("glyf"), 0, 0, NULL, &nGlyfLen);
    if( (eStatus != noErr) || (nGlyfLen <= 0) )
        return false;
    ByteCount nNameLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("name"), 0, 0, NULL, &nNameLen);
    if( (eStatus != noErr) || (nNameLen <= 0) )
        return false;
    ByteCount nHheaLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("hhea"), 0, 0, NULL, &nHheaLen);
    if( (eStatus != noErr) || (nHheaLen <= 0) )
        return false;
    ByteCount nHmtxLen  = 0;
    eStatus = ATSFontGetTable( rFont, GetTag("hmtx"), 0, 0, NULL, &nHmtxLen);
    if( (eStatus != noErr) || (nHmtxLen <= 0) )
        return false;

    ByteCount nPrepLen=0, nCvtLen=0, nFpgmLen=0;
    if( 1 ) // TODO: reduce PDF size by making hint subsetting optional
    {
        eStatus = ATSFontGetTable( rFont, GetTag("prep"), 0, 0, NULL, &nPrepLen);
        eStatus = ATSFontGetTable( rFont, GetTag("cvt "), 0, 0, NULL, &nCvtLen);
        eStatus = ATSFontGetTable( rFont, GetTag("fpgm"), 0, 0, NULL, &nFpgmLen);
    }

    // prepare a byte buffer for a fake font
    int nTableCount = 8;
    nTableCount += (nPrepLen>0) + (nCvtLen>0) + (nFpgmLen>0);
    const ByteCount nFdirLen = 12 + 16*nTableCount;
    ByteCount nTotalLen = nFdirLen;
    nTotalLen += nHeadLen + nMaxpLen + nNameLen + nCmapLen + nLocaLen + nGlyfLen;
    nTotalLen += nHheaLen + nHmtxLen;
    nTotalLen += nPrepLen + nCvtLen + nFpgmLen;
    rBuffer.resize( nTotalLen );

    // fake a SFNT font directory header
    if( nTableCount < 16 )
    {
        int nLog2 = 0;
        while( (nTableCount >> nLog2) > 1 ) ++nLog2;
        rBuffer[ 1] = 1;                        // Win-TTF style scaler
        rBuffer[ 5] = nTableCount;              // table count
        rBuffer[ 7] = nLog2*16;                 // searchRange
        rBuffer[ 9] = nLog2;                    // entrySelector
        rBuffer[11] = (nTableCount-nLog2)*16;   // rangeShift
    }

    // get font table raw data and update the fake directory entries
    ByteCount nOfs = nFdirLen;
    unsigned char* pFakeEntry = &rBuffer[12];
    eStatus = ATSFontGetTable( rFont, GetTag("cmap"), 0, nCmapLen, (void*)&rBuffer[nOfs], &nCmapLen);
    FakeDirEntry( GetTag("cmap"), nOfs, nCmapLen, &rBuffer[0], pFakeEntry );
    nOfs += nCmapLen;
    if( nCvtLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("cvt "), 0, nCvtLen, (void*)&rBuffer[nOfs], &nCvtLen);
        FakeDirEntry( GetTag("cvt "), nOfs, nCvtLen, &rBuffer[0], pFakeEntry );
        nOfs += nCvtLen;
    }
    if( nFpgmLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("fpgm"), 0, nFpgmLen, (void*)&rBuffer[nOfs], &nFpgmLen);
        FakeDirEntry( GetTag("fpgm"), nOfs, nFpgmLen, &rBuffer[0], pFakeEntry );
        nOfs += nFpgmLen;
    }
    eStatus = ATSFontGetTable( rFont, GetTag("glyf"), 0, nGlyfLen, (void*)&rBuffer[nOfs], &nGlyfLen);
    FakeDirEntry( GetTag("glyf"), nOfs, nGlyfLen, &rBuffer[0], pFakeEntry );
    nOfs += nGlyfLen;
    eStatus = ATSFontGetTable( rFont, GetTag("head"), 0, nHeadLen, (void*)&rBuffer[nOfs], &nHeadLen);
    FakeDirEntry( GetTag("head"), nOfs, nHeadLen, &rBuffer[0], pFakeEntry );
    nOfs += nHeadLen;
    eStatus = ATSFontGetTable( rFont, GetTag("hhea"), 0, nHheaLen, (void*)&rBuffer[nOfs], &nHheaLen);
    FakeDirEntry( GetTag("hhea"), nOfs, nHheaLen, &rBuffer[0], pFakeEntry );
    nOfs += nHheaLen;
    eStatus = ATSFontGetTable( rFont, GetTag("hmtx"), 0, nHmtxLen, (void*)&rBuffer[nOfs], &nHmtxLen);
    FakeDirEntry( GetTag("hmtx"), nOfs, nHmtxLen, &rBuffer[0], pFakeEntry );
    nOfs += nHmtxLen;
    eStatus = ATSFontGetTable( rFont, GetTag("loca"), 0, nLocaLen, (void*)&rBuffer[nOfs], &nLocaLen);
    FakeDirEntry( GetTag("loca"), nOfs, nLocaLen, &rBuffer[0], pFakeEntry );
    nOfs += nLocaLen;
    eStatus = ATSFontGetTable( rFont, GetTag("maxp"), 0, nMaxpLen, (void*)&rBuffer[nOfs], &nMaxpLen);
    FakeDirEntry( GetTag("maxp"), nOfs, nMaxpLen, &rBuffer[0], pFakeEntry );
    nOfs += nMaxpLen;
    eStatus = ATSFontGetTable( rFont, GetTag("name"), 0, nNameLen, (void*)&rBuffer[nOfs], &nNameLen);
    FakeDirEntry( GetTag("name"), nOfs, nNameLen, &rBuffer[0], pFakeEntry );
    nOfs += nNameLen;
    if( nPrepLen ) {
        eStatus = ATSFontGetTable( rFont, GetTag("prep"), 0, nPrepLen, (void*)&rBuffer[nOfs], &nPrepLen);
        FakeDirEntry( GetTag("prep"), nOfs, nPrepLen, &rBuffer[0], pFakeEntry );
        nOfs += nPrepLen;
    }

    DBG_ASSERT( (nOfs==nTotalLen), "AquaSalGraphics::CreateFontSubset (nOfs!=nTotalLen)");

    return true;
}

BOOL AquaSalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
    const ImplFontData* pFontData, long* pGlyphIDs, sal_uInt8* pEncoding,
    sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo )
{
    ByteVector aBuffer;
    if( !GetRawFontData( pFontData, aBuffer ) )
        return sal_False;

    // TODO: modernize psprint's horrible fontsubset C-API
    // this probably only makes sense after the switch to another SCM
    // that can preserve change history after file renames

    // prepare data for psprint's font subsetter
    TrueTypeFont* pSftFont = NULL;
    int nRC = ::OpenTTFontBuffer( (void*)&aBuffer[0], aBuffer.size(), 0, &pSftFont);
    if( nRC != SF_OK )
        return sal_False;

    // get details about the subsetted font
    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftFont, &aTTInfo );
    rInfo.m_nFontType   = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;
    rInfo.m_aPSName     = String( aTTInfo.psname, RTL_TEXTENCODING_UTF8 );
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; // Well ...
    rInfo.m_nAscent     = +aTTInfo.winAscent;
    rInfo.m_nDescent    = -aTTInfo.winDescent;
    // mac fonts usually do not have an OS2-table
    // => get valid ascent/descent values from other tables
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.typoAscender;
    if( !rInfo.m_nAscent )
        rInfo.m_nAscent = +aTTInfo.ascender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = +aTTInfo.typoDescender;
    if( !rInfo.m_nDescent )
        rInfo.m_nDescent = -aTTInfo.descender;

    // subset glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphCount;
    USHORT    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef = -1;
    for( int i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_uInt32 nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
        if( pGlyphIDs[i] & GF_ISCHAR )
        {
            bool bVertical = (pGlyphIDs[i] & GF_ROTMASK) != 0;
            nGlyphIdx = ::MapChar( pSftFont, static_cast<sal_uInt16>(nGlyphIdx), bVertical );
            if( nGlyphIdx == 0 && pFontData->IsSymbolFont() )
            {
                // #i12824# emulate symbol aliasing U+FXXX <-> U+0XXX
                nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
                nGlyphIdx = (nGlyphIdx & 0xF000) ? (nGlyphIdx & 0x00FF) : (nGlyphIdx | 0xF000 );
                nGlyphIdx = ::MapChar( pSftFont, static_cast<sal_uInt16>(nGlyphIdx), bVertical );
            }
        }
        aShortIDs[i] = static_cast<USHORT>( nGlyphIdx );
        if( !nGlyphIdx )
            if( nNotDef < 0 )
                nNotDef = i; // first NotDef glyph found
    }

    if( nNotDef != 0 )
    {
        // add fake NotDef glyph if needed
        if( nNotDef < 0 )
            nNotDef = nGlyphCount++;

        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }
    DBG_ASSERT( nGlyphCount < 257, "too many glyphs for subsetting" );

    // TODO: where to get bVertical?
    const bool bVertical = false;

    // fill the pGlyphWidths array
    // while making sure that the NotDef glyph is at index==0
    TTSimpleGlyphMetrics* pGlyphMetrics =
        ::GetTTSimpleGlyphMetrics( pSftFont, aShortIDs, nGlyphCount, bVertical );
    if( !pGlyphMetrics )
        return FALSE;
    sal_uInt16 nNotDefAdv       = pGlyphMetrics[0].adv;
    pGlyphMetrics[0].adv        = pGlyphMetrics[nNotDef].adv;
    pGlyphMetrics[nNotDef].adv  = nNotDefAdv;
    for( int i = 0; i < nOrigCount; ++i )
        pGlyphWidths[i] = pGlyphMetrics[i].adv;
    free( pGlyphMetrics );

    // write subset into destination file
    rtl::OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return FALSE;
    rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    ByteString aToFile( rtl::OUStringToOString( aSysPath, aThreadEncoding ) );
    nRC = ::CreateTTFromTTGlyphs( pSftFont, aToFile.GetBuffer(), aShortIDs,
            aTempEncs, nGlyphCount, 0, NULL, 0 );
    ::CloseTTFont(pSftFont);
    return (nRC == SF_OK);
}

// -----------------------------------------------------------------------

void AquaSalGraphics::GetGlyphWidths( const ImplFontData* pFontData, bool bVertical,
    Int32Vector& rGlyphWidths, Ucs2UIntMap& rUnicodeEnc )
{
    rGlyphWidths.clear();
    rUnicodeEnc.clear();

    if( pFontData->IsSubsettable() )
    {
        ByteVector aBuffer;
        if( !GetRawFontData( pFontData, aBuffer ) )
            return;

        // TODO: modernize psprint's horrible fontsubset C-API
        // this probably only makes sense after the switch to another SCM
        // that can preserve change history after file renames

        // use the font subsetter to get the widths
        TrueTypeFont* pSftFont = NULL;
        int nRC = ::OpenTTFontBuffer( (void*)&aBuffer[0], aBuffer.size(), 0, &pSftFont);
        if( nRC != SF_OK )
            return;

        const int nGlyphCount = ::GetTTGlyphCount( pSftFont );
        if( nGlyphCount > 0 )
        {
            // get glyph metrics
            rGlyphWidths.resize(nGlyphCount);
            std::vector<sal_uInt16> aGlyphIds(nGlyphCount);
            for( int i = 0; i < nGlyphCount; i++ )
                aGlyphIds[i] = static_cast<sal_uInt16>(i);
            const TTSimpleGlyphMetrics* pGlyphMetrics = ::GetTTSimpleGlyphMetrics(
                pSftFont, &aGlyphIds[0], nGlyphCount, bVertical );
            if( pGlyphMetrics )
            {
                for( int i = 0; i < nGlyphCount; ++i )
                    rGlyphWidths[i] = pGlyphMetrics[i].adv;
                free( (void*)pGlyphMetrics );
            }

            const ImplFontCharMap* pMap = mpMacFontData->GetImplFontCharMap();
            DBG_ASSERT( pMap && pMap->GetCharCount(), "no charmap" );

            // get unicode<->glyph encoding
            int nCharCount = pMap->GetCharCount();
            sal_uInt32 nChar = pMap->GetFirstChar();
            for(; --nCharCount >= 0; nChar = pMap->GetNextChar( nChar ) )
            {
                if( nChar > 0xFFFF ) // TODO: allow UTF-32 chars
                    break;
                sal_Ucs nUcsChar = static_cast<sal_Ucs>(nChar);
                sal_uInt32 nGlyph = ::MapChar( pSftFont, nUcsChar, bVertical );
                if( nGlyph > 0 )
                    rUnicodeEnc[ nUcsChar ] = nGlyph;
            }
        }

        ::CloseTTFont( pSftFont );
    }
    else if( pFontData->IsEmbeddable() )
    {
        // get individual character widths
#if 0 // FIXME
        rWidths.reserve( 224 );
        for( sal_Unicode i = 32; i < 256; ++i )
        {
            int nCharWidth = 0;
            if( ::GetCharWidth32W( mhDC, i, i, &nCharWidth ) )
            {
                rUnicodeEnc[ i ] = rWidths.size();
                rWidths.push_back( nCharWidth );
            }
        }
#else
        DBG_ERROR("not implemented for non-subsettable fonts!\n");
#endif
    }
}

// -----------------------------------------------------------------------

const Ucs2SIntMap* AquaSalGraphics::GetFontEncodingVector(
    const ImplFontData* pFontData, const Ucs2OStrMap** ppNonEncoded )
{
    return NULL;
}

// -----------------------------------------------------------------------

const void* AquaSalGraphics::GetEmbedFontData( const ImplFontData* pFontData,
                              const sal_Ucs* pUnicodes,
                              sal_Int32* pWidths,
                              FontSubsetInfo& rInfo,
                              long* pDataLen )
{
    // TODO: are the non-subsettable fonts on OSX that are embeddable?
    return NULL;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::FreeEmbedFontData( const void* pData, long nDataLen )
{
    // TODO: implementing this only makes sense when the implementation of
    //      AquaSalGraphics::GetEmbedFontData() returns non-NULL
    DBG_ASSERT( (pData!=NULL), "AquaSalGraphics::FreeEmbedFontData() is not implemented\n");
}

// -----------------------------------------------------------------------

