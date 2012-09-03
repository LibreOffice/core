/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "ios/common.h"

#include "ios/salframe.h"

#include "ios/salgdi.h"
#include "ios/salcoretextstyle.hxx"
#include "ios/salcoretextlayout.hxx"

IosSalGraphics::IosSalGraphics()
    : mpFrame( NULL )
    , mxLayer( NULL )
    , mrContext( NULL )
    , mpXorEmulation( NULL )
    , mnXorMode( 0 )
    , mnWidth( 0 )
    , mnHeight( 0 )
    , mnBitmapDepth( 0 )
    , mnRealDPIX( 0 )
    , mnRealDPIY( 0 )
    , mfFakeDPIScale( 1.0 )
    , mxClipPath( NULL )
    , maLineColor( COL_WHITE )
    , maFillColor( COL_BLACK )
    , m_pCoreTextFontData( NULL )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
    , mbWindow( false )
{
    msgs_debug(gr,"-->");
    m_style = new CoreTextStyleInfo();
    msgs_debug(gr,"m_style=%p <--", m_style);
}

IosSalGraphics::~IosSalGraphics()
{
    msgs_debug(gr,"-->");
    if(m_style)
    {
        delete m_style;
        m_style = NULL;
    }
    msgs_debug(gr,"<--");
}

inline bool IosSalGraphics::AddTempDevFont( ImplDevFontList*,
                                             const rtl::OUString& ,
                                             const rtl::OUString& )
{
    OSL_ASSERT( FALSE );
    return false;
}

void IosSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

void IosSalGraphics::FreeEmbedFontData( const void* pData, long /*nDataLen*/ )
{
    // TODO: implementing this only makes sense when the implementation of
    //      IosSalGraphics::GetEmbedFontData() returns non-NULL
    (void)pData;
    DBG_ASSERT( (pData!=NULL), "IosSalGraphics::FreeEmbedFontData() is not implemented\n");
}

void IosSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( pFontList, "IosSalGraphics::GetDevFontList(NULL) !");

    SalData* pSalData = GetSalData();
    if (pSalData->mpFontList == NULL)
    {
        pSalData->mpFontList = new SystemFontList();
    }
    // Copy all PhysicalFontFace objects contained in the SystemFontList
    pSalData->mpFontList->AnnounceFonts( *pFontList );
}

void IosSalGraphics::ClearDevFontList()
{
    SalData* pSalData = GetSalData();
    delete pSalData->mpFontList;
    pSalData->mpFontList = NULL;
}

void IosSalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // nothing to do since there are no device-specific fonts on Ios
}

const void* IosSalGraphics::GetEmbedFontData( const PhysicalFontFace*,
                              const sal_Ucs* /*pUnicodes*/,
                              sal_Int32* /*pWidths*/,
                              FontSubsetInfo&,
                              long* /*pDataLen*/ )
{
    return NULL;
}

const Ucs2SIntMap* IosSalGraphics::GetFontEncodingVector(const PhysicalFontFace*,
                                                          const Ucs2OStrMap** /*ppNonEncoded*/ )
{
    return NULL;
}

void IosSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    (void)nFallbackLevel; // glyph-fallback on CoreText is done differently -> no fallback level

    pMetric->mbScalableFont = true;
    pMetric->mbKernableFont = true;
    CTFontRef font = m_style->GetFont();
    DBG_ASSERT(font, "GetFontMetric without font set in style");

    pMetric->mnAscent = static_cast<long>( CTFontGetAscent(font) * mfFakeDPIScale + 0.5);
    pMetric->mnDescent = static_cast<long>(CTFontGetDescent(font) * mfFakeDPIScale + 0.5);
    const long nExtDescent  = static_cast<long>((CTFontGetLeading(font) + CTFontGetDescent(font)) *
                                                mfFakeDPIScale + 0.5);
    pMetric->mnExtLeading   = nExtDescent + pMetric->mnDescent;
    pMetric->mnIntLeading   = 0;
    pMetric->mnWidth = m_style->GetFontStretchedSize();
    msgs_debug(gr,"ascent=%ld, descent=%ld, extleading=%ld, intleading=%ld,w=%ld",
               pMetric->mnAscent, pMetric->mnDescent,
               pMetric->mnExtLeading,
               pMetric->mnIntLeading,
               pMetric->mnWidth);
}

sal_Bool IosSalGraphics::GetGlyphBoundRect( sal_GlyphId /*nGlyphId*/, Rectangle& /*rRect*/ )
{
    /* TODO: create a Ghyph iterator to keep track ot 'state' between call */
    return false;
}

sal_Bool IosSalGraphics::GetGlyphOutline( sal_GlyphId /*nGlyphId*/, basegfx::B2DPolyPolygon& /*rPolyPoly*/ )
{
    /* TODO */
    return false;
}

void IosSalGraphics::GetGlyphWidths( const PhysicalFontFace* /*pFontData*/, bool /*bVertical*/,
                                      Int32Vector& /*rGlyphWidths*/, Ucs2UIntMap& /*rUnicodeEnc*/ )
{
}

sal_uLong IosSalGraphics::GetKernPairs( sal_uLong, ImplKernPairData* )
{
    return 0;
}

bool IosSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !m_pCoreTextFontData )
    {
        return false;
    }
    return m_pCoreTextFontData->GetImplFontCapabilities(rFontCapabilities);
}

const ImplFontCharMap* IosSalGraphics::GetImplFontCharMap() const
{
    if( !m_pCoreTextFontData )
    {
        return ImplFontCharMap::GetDefaultMap();
    }
    return m_pCoreTextFontData->GetImplFontCharMap();
}

bool IosSalGraphics::GetRawFontData( const PhysicalFontFace* pFontData,
                     std::vector<unsigned char>& rBuffer, bool* pJustCFF )
{
    const ImplCoreTextFontData* font_data = static_cast<const ImplCoreTextFontData*>(pFontData);

    return font_data->GetRawFontData(rBuffer, pJustCFF);
}

SystemFontData IosSalGraphics::GetSysFontData( int /* nFallbacklevel */ ) const
{
    msgs_debug(gr,"-->");
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.bAntialias = true;

    CTFontRef font = CTFontCreateUIFontForLanguage(kCTFontSystemFontType, 0.0, NULL);
    font = (CTFontRef)CFRetain(font);
    aSysFontData.rCTFont = font;

    CTFontRef italic_font = CTFontCreateCopyWithSymbolicTraits( font,
                                                                0.0,
                                                                NULL,
                                                                kCTFontItalicTrait,
                                                                kCTFontItalicTrait + kCTFontBoldTrait);
    aSysFontData.bFakeItalic = italic_font ? false : true;
    SafeCFRelease(italic_font);

    CTFontRef bold_font = CTFontCreateCopyWithSymbolicTraits( font,
                                                              0.0,
                                                              NULL,
                                                              kCTFontBoldTrait,
                                                              kCTFontItalicTrait + kCTFontBoldTrait);
    aSysFontData.bFakeBold = bold_font ? false : true;
    SafeCFRelease(bold_font);

    CTFontRef vertical_font = CTFontCreateCopyWithSymbolicTraits( font,
                                                                  0.0,
                                                                  NULL,
                                                                  kCTFontVerticalTrait,
                                                                  kCTFontVerticalTrait);
    aSysFontData.bVerticalCharacterType = vertical_font ? true : false;
    SafeCFRelease(vertical_font);

    msgs_debug(gr,"<--");
    return aSysFontData;
}

SalLayout* IosSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    msgs_debug(gr,"-->");
    CoreTextLayout* layout = new CoreTextLayout( this, m_style );
    msgs_debug(gr,"layout:%p <--", layout);
    return layout;
}

sal_uInt16 IosSalGraphics::SetFont( FontSelectPattern* pReqFont, int /*nFallbackLevel*/ )
{
    msgs_debug(gr,"m_style=%p -->", m_style);
    m_style->SetFont(pReqFont);
    msgs_debug(gr,"<--");
    return 0;
}

void IosSalGraphics::SetTextColor( SalColor nSalColor )
{
    msgs_debug(gr,"m_style=%p -->", m_style);
    m_style->SetColor(nSalColor);
    msgs_debug(gr,"<--");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
