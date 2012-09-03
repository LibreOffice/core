/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "aqua/common.h"

#include "aqua/salframe.h"

#include "aqua/coretext/salgdi.h"
#include "aqua/coretext/salcoretextstyle.hxx"
#include "aqua/coretext/salcoretextlayout.hxx"

AquaSalGraphics::AquaSalGraphics()
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
    , m_font_face( NULL )
    , mbNonAntialiasedText( false )
    , mbPrinter( false )
    , mbVirDev( false )
    , mbWindow( false )
{
    msgs_debug(gr,"-->");
    m_style = new CoreTextStyleInfo();
    msgs_debug(gr,"m_style=%p <--", m_style);
}

AquaSalGraphics::~AquaSalGraphics()
{
    msgs_debug(gr,"-->");
    if(m_style)
    {
        delete m_style;
        m_style = NULL;
    }
    msgs_debug(gr,"<--");
}

inline bool AquaSalGraphics::AddTempDevFont( ImplDevFontList*,
                                             const rtl::OUString& ,
                                             const rtl::OUString& )
{
    OSL_ASSERT( FALSE );
    return false;
}

void AquaSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

void AquaSalGraphics::FreeEmbedFontData( const void* pData, long /*nDataLen*/ )
{
    // TODO: implementing this only makes sense when the implementation of
    //      AquaSalGraphics::GetEmbedFontData() returns non-NULL
    (void)pData;
    DBG_ASSERT( (pData!=NULL), "AquaSalGraphics::FreeEmbedFontData() is not implemented\n");
}

void AquaSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( pFontList, "AquaSalGraphics::GetDevFontList(NULL) !");

    SalData* pSalData = GetSalData();
    if (pSalData->mpFontList == NULL)
    {
        pSalData->mpFontList = new SystemFontList();
    }
    // Copy all PhysicalFontFace objects contained in the SystemFontList
    pSalData->mpFontList->AnnounceFonts( *pFontList );
}

void AquaSalGraphics::ClearDevFontCache()
{
    SalData* pSalData = GetSalData();
    delete pSalData->mpFontList;
    pSalData->mpFontList = NULL;
}

void AquaSalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // nothing to do since there are no device-specific fonts on Aqua
}

const void* AquaSalGraphics::GetEmbedFontData( const PhysicalFontFace*,
                              const sal_Ucs* /*pUnicodes*/,
                              sal_Int32* /*pWidths*/,
                              FontSubsetInfo&,
                              long* /*pDataLen*/ )
{
    return NULL;
}

const Ucs2SIntMap* AquaSalGraphics::GetFontEncodingVector(const PhysicalFontFace*,
                                                          const Ucs2OStrMap** /*ppNonEncoded*/ )
{
    return NULL;
}

void AquaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
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

sal_Bool AquaSalGraphics::GetGlyphBoundRect( sal_GlyphId /*nGlyphId*/, Rectangle& /*rRect*/ )
{
    /* TODO: create a Ghyph iterator to keep track ot 'state' between call */
    return false;
}

sal_Bool AquaSalGraphics::GetGlyphOutline( sal_GlyphId /*nGlyphId*/, basegfx::B2DPolyPolygon& /*rPolyPoly*/ )
{
    /* TODO */
    return false;
}

void AquaSalGraphics::GetGlyphWidths( const PhysicalFontFace* /*pFontData*/, bool /*bVertical*/,
                                      Int32Vector& /*rGlyphWidths*/, Ucs2UIntMap& /*rUnicodeEnc*/ )
{
}

sal_uLong AquaSalGraphics::GetKernPairs( sal_uLong, ImplKernPairData* )
{
    return 0;
}

bool AquaSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !m_font_face )
    {
        return false;
    }
    return m_font_face->GetImplFontCapabilities(rFontCapabilities);
}

const ImplFontCharMap* AquaSalGraphics::GetImplFontCharMap() const
{
    if( !m_font_face )
    {
        return ImplFontCharMap::GetDefaultMap();
    }
    return m_font_face->GetImplFontCharMap();
}

bool AquaSalGraphics::GetRawFontData( const PhysicalFontFace* pFontFace,
                     std::vector<unsigned char>& rBuffer, bool* pJustCFF )
{
    const CoreTextPhysicalFontFace* font_face = static_cast<const CoreTextPhysicalFontFace*>(pFontFace);

    return font_face->GetRawFontData(rBuffer, pJustCFF);
}

SystemFontData AquaSalGraphics::GetSysFontData( int /* nFallbacklevel */ ) const
{
    msgs_debug(gr,"-->");
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.bAntialias = true;

    CTFontRef font = CTFontCreateUIFontForLanguage(kCTFontSystemFontType, 0.0, NULL);
    font = (CTFontRef)CFRetain(font);
    aSysFontData.rCTFont = (void*)font;

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

SalLayout* AquaSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    msgs_debug(gr,"-->");
    CoreTextLayout* layout = new CoreTextLayout( this, m_style );
    msgs_debug(gr,"layout:%p <--", layout);
    return layout;
}

sal_uInt16 AquaSalGraphics::SetFont( FontSelectPattern* pReqFont, int /*nFallbackLevel*/ )
{
    msgs_debug(gr,"m_style=%p -->", m_style);
    m_style->SetFont(pReqFont);
    msgs_debug(gr,"<--");
    return 0;
}

void AquaSalGraphics::SetTextColor( SalColor nSalColor )
{
    msgs_debug(gr,"m_style=%p -->", m_style);
    m_style->SetColor(nSalColor);
    msgs_debug(gr,"<--");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
