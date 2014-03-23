/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <boost/assert.hpp>
#include <vector>
#include <hash_map>
#include <set>

#include "salgdi.h"
#include "atsfonts.hxx"

#include "vcl/svapp.hxx"
#include "vcl/impfont.hxx"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"

typedef GlyphID ATSGlyphID;

// =======================================================================

// mac specific physically available font face
class AtsFontData
:   public ImplMacFontData
{
public:
    explicit                AtsFontData( const ImplDevFontAttributes&, ATSUFontID );
    virtual                 ~AtsFontData( void );
    virtual ImplFontData*   Clone( void ) const;

    virtual ImplMacTextStyle*   CreateMacTextStyle( const ImplFontSelectData& ) const;
    virtual ImplFontEntry*      CreateFontInstance( /*const*/ ImplFontSelectData& ) const;
    virtual int                 GetFontTable( const char pTagName[5], unsigned char* ) const;
};

// =======================================================================

class AtsFontList
:   public SystemFontList
{
public:
    explicit    AtsFontList( void );
    virtual     ~AtsFontList( void );

    virtual void            AnnounceFonts( ImplDevFontList& ) const;
    virtual ImplMacFontData* GetFontDataFromId( sal_IntPtr nFontId ) const;

private:
    typedef std::hash_map<sal_IntPtr,AtsFontData*> AtsFontContainer;
    AtsFontContainer maFontContainer;

    void InitGlyphFallbacks( void );
    ATSUFontFallbacks   maFontFallbacks;
};

// =======================================================================

AtsFontData::AtsFontData( const ImplDevFontAttributes& rDFA, ATSUFontID nFontId )
:   ImplMacFontData( rDFA, (sal_IntPtr)nFontId )
{}

// -----------------------------------------------------------------------

AtsFontData::~AtsFontData( void )
{}

// -----------------------------------------------------------------------

ImplFontData* AtsFontData::Clone( void ) const
{
    AtsFontData* pClone = new AtsFontData(*this);
    return pClone;
}

// -----------------------------------------------------------------------

ImplMacTextStyle* AtsFontData::CreateMacTextStyle( const ImplFontSelectData& rFSD ) const
{
    return new AtsTextStyle( rFSD );
}

// -----------------------------------------------------------------------

ImplFontEntry* AtsFontData::CreateFontInstance( /*const*/ ImplFontSelectData& rFSD ) const
{
    return new ImplFontEntry( rFSD );
}

// -----------------------------------------------------------------------

int AtsFontData::GetFontTable( const char pTagName[5], unsigned char* pResultBuf ) const
{
    DBG_ASSERT( aTagName[4]=='\0', "AtsFontData::GetFontTable with invalid tagname!\n" );

    const FourCharCode pTagCode = (pTagName[0]<<24) + (pTagName[1]<<16) + (pTagName[2]<<8) + (pTagName[3]<<0);

    // get the byte size of the raw table
    ATSFontRef rATSFont = FMGetATSFontRefFromFont( (ATSUFontID)mnFontId );
    ByteCount nBufSize = 0;
    OSStatus eStatus = ATSFontGetTable( rATSFont, pTagCode, 0, 0, NULL, &nBufSize );
    if( eStatus != noErr )
        return 0;

    // get the raw table data if requested
    if( pResultBuf && (nBufSize > 0))
    {
        ByteCount nRawLength = 0;
        eStatus = ATSFontGetTable( rATSFont, pTagCode, 0, nBufSize, (void*)pResultBuf, &nRawLength );
        if( eStatus != noErr )
            return 0;
        DBG_ASSERT( (nBufSize==nRawLength), "AtsFontData::GetFontTable ByteCount mismatch!\n");
    }

    return nBufSize;
}

// =======================================================================

AtsTextStyle::AtsTextStyle( const ImplFontSelectData& rFSD )
:   ImplMacTextStyle( rFSD )
{
    // create the style object for ATSUI font attributes
    ATSUCreateStyle( &maATSUStyle );
    const ImplFontSelectData* const pReqFont = &rFSD;

    mpFontData = (AtsFontData*)rFSD.mpFontData;

    // limit the ATS font size to avoid Fixed16.16 overflows
    double fScaledFontHeight = pReqFont->mfExactHeight;
    static const float fMaxFontHeight = 144.0;
    if( fScaledFontHeight > fMaxFontHeight )
    {
        mfFontScale = fScaledFontHeight / fMaxFontHeight;
        fScaledFontHeight = fMaxFontHeight;
    }

    // convert font rotation to radian
    mfFontRotation = pReqFont->mnOrientation * (M_PI / 1800.0);

    // determine if font stretching is needed
    if( (pReqFont->mnWidth != 0) && (pReqFont->mnWidth != pReqFont->mnHeight) )
    {
        mfFontStretch = (float)pReqFont->mnWidth / pReqFont->mnHeight;
        // set text style to stretching matrix
        CGAffineTransform aMatrix = CGAffineTransformMakeScale( mfFontStretch, 1.0F );
        const ATSUAttributeTag aMatrixTag = kATSUFontMatrixTag;
        const ATSUAttributeValuePtr aAttr = &aMatrix;
        const ByteCount aMatrixBytes = sizeof(aMatrix);
        /*OSStatus eStatus =*/ ATSUSetAttributes( maATSUStyle, 1, &aMatrixTag, &aMatrixBytes, &aAttr );
    }
}

// -----------------------------------------------------------------------

AtsTextStyle::~AtsTextStyle( void )
{
    ATSUDisposeStyle( maATSUStyle );
}

// -----------------------------------------------------------------------

void AtsTextStyle::GetFontMetric( float fDPIY, ImplFontMetricData& rMetric ) const
{
    // get the font metrics (in point units)
    // of the font that has eventually been size-limited

    // get the matching ATSU font handle
    ATSUFontID fontId;
    OSStatus err = ::ATSUGetAttribute( maATSUStyle, kATSUFontTag, sizeof(ATSUFontID), &fontId, 0 );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font id\n");

    ATSFontMetrics aMetrics;
    ATSFontRef rFont = FMGetATSFontRefFromFont( fontId );
    err = ATSFontGetHorizontalMetrics ( rFont, kATSOptionFlagsDefault, &aMetrics );
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font metrics\n");
    if( err != noErr )
        return;

    // all ATS fonts are scalable fonts
    rMetric.mbScalableFont = true;
    // TODO: check if any kerning is possible
    rMetric.mbKernableFont = true;

    // convert into VCL font metrics (in unscaled pixel units)

    Fixed ptSize;
    err = ATSUGetAttribute( maATSUStyle, kATSUSizeTag, sizeof(Fixed), &ptSize, 0);
    DBG_ASSERT( (err==noErr), "AquaSalGraphics::GetFontMetric() : could not get font size\n");
    const double fPointSize = Fix2X( ptSize );

    // convert quartz units to pixel units
    // please see the comment in AquaSalGraphics::SetFont() for details
    const double fPixelSize = (mfFontScale * fDPIY * fPointSize);
    rMetric.mnAscent       = static_cast<long>(+aMetrics.ascent  * fPixelSize + 0.5);
    rMetric.mnDescent      = static_cast<long>(-aMetrics.descent * fPixelSize + 0.5);
    const long nExtDescent = static_cast<long>((-aMetrics.descent + aMetrics.leading) * fPixelSize + 0.5);
    rMetric.mnExtLeading   = nExtDescent - rMetric.mnDescent;
    rMetric.mnIntLeading   = 0;
    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    rMetric.mnWidth        = static_cast<long>(mfFontStretch * fPixelSize + 0.5);
}

// -----------------------------------------------------------------------

void AtsTextStyle::SetTextColor( const RGBAColor& rColor )
{
    RGBColor aAtsColor;
    aAtsColor.red   = (unsigned short)( rColor.GetRed()   * 65535.0 );
    aAtsColor.green = (unsigned short)( rColor.GetGreen() * 65535.0 );
    aAtsColor.blue  = (unsigned short)( rColor.GetColor() * 65535.0 );

    ATSUAttributeTag aTag = kATSUColorTag;
    ByteCount aValueSize = sizeof( aAtsColor );
    ATSUAttributeValuePtr aValue = &aAtsColor;

    /*OSStatus err =*/ ATSUSetAttributes( maATSUStyle, 1, &aTag, &aValueSize, &aValue );
}

// -----------------------------------------------------------------------

bool AtsTextStyle::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect ) const
{
    ATSUStyle rATSUStyle = maATSUStyle; // TODO: handle glyph fallback
    ATSGlyphID aGlyphId = aGlyphId;
    ATSGlyphScreenMetrics aGlyphMetrics;
    const bool bNonAntialiasedText = false;
    OSStatus eStatus = ATSUGlyphGetScreenMetrics( rATSUStyle,
        1, &aGlyphId, 0, FALSE, !bNonAntialiasedText, &aGlyphMetrics );
    if( eStatus != noErr )
        return false;

    const long nMinX = (long)(+aGlyphMetrics.topLeft.x * mfFontScale - 0.5);
    const long nMaxX = (long)(aGlyphMetrics.width * mfFontScale + 0.5) + nMinX;
    const long nMinY = (long)(-aGlyphMetrics.topLeft.y * mfFontScale - 0.5);
    const long nMaxY = (long)(aGlyphMetrics.height * mfFontScale + 0.5) + nMinY;
    rRect = Rectangle( nMinX, nMinY, nMaxX, nMaxY );
    return true;
}

// -----------------------------------------------------------------------

// callbacks from ATSUGlyphGetCubicPaths() fore GetGlyphOutline()
struct GgoData { basegfx::B2DPolygon maPolygon; basegfx::B2DPolyPolygon* mpPolyPoly; };

static OSStatus GgoLineToProc( const Float32Point* pPoint, void* pData )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const basegfx::B2DPoint aB2DPoint( pPoint->x, pPoint->y );
    rPolygon.append( aB2DPoint );
    return noErr;
}

static OSStatus GgoCurveToProc( const Float32Point* pCP1, const Float32Point* pCP2,
    const Float32Point* pPoint, void* pData )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const sal_uInt32 nPointCount = rPolygon.count();
    const basegfx::B2DPoint aB2DControlPoint1( pCP1->x, pCP1->y );
    rPolygon.setNextControlPoint( nPointCount-1, aB2DControlPoint1 );
    const basegfx::B2DPoint aB2DEndPoint( pPoint->x, pPoint->y );
    rPolygon.append( aB2DEndPoint );
    const basegfx::B2DPoint aB2DControlPoint2( pCP2->x, pCP2->y );
    rPolygon.setPrevControlPoint( nPointCount, aB2DControlPoint2 );
    return noErr;
}

static OSStatus GgoClosePathProc( void* pData )
{
    GgoData* pGgoData = static_cast<GgoData*>(pData);
    basegfx::B2DPolygon& rPolygon = pGgoData->maPolygon;
    if( rPolygon.count() > 0 )
        pGgoData->mpPolyPoly->append( rPolygon );
    rPolygon.clear();
    return noErr;
}

static OSStatus GgoMoveToProc( const Float32Point* pPoint, void* pData )
{
    GgoClosePathProc( pData );
    OSStatus eStatus = GgoLineToProc( pPoint, pData );
    return eStatus;
}

bool AtsTextStyle::GetGlyphOutline( sal_GlyphId aGlyphId, basegfx::B2DPolyPolygon& rResult ) const
{
    GgoData aGgoData;
    aGgoData.mpPolyPoly = &rResult;
    rResult.clear();

    OSStatus eGgoStatus = noErr;
    OSStatus eStatus = ATSUGlyphGetCubicPaths( maATSUStyle, aGlyphId,
        GgoMoveToProc, GgoLineToProc, GgoCurveToProc, GgoClosePathProc,
        &aGgoData, &eGgoStatus );
    if( (eStatus != noErr) ) // TODO: why is (eGgoStatus!=noErr) when curves are involved?
        return false;

    GgoClosePathProc( &aGgoData );

    // apply the font scale
    if( mfFontScale != 1.0 ) {
        basegfx::B2DHomMatrix aScale;
        aScale.scale( +mfFontScale, +mfFontScale );
        rResult.transform( aScale );
    }

    return true;
}

// =======================================================================

static bool GetDevFontAttributes( ATSUFontID nFontID, ImplDevFontAttributes& rDFA )
{
    // all ATSU fonts are device fonts that can be directly rotated
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

    // reset the attributes
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.mePitch      = PITCH_VARIABLE;
    rDFA.meWidthType  = WIDTH_NORMAL;
    rDFA.meWeight     = WEIGHT_NORMAL;
    rDFA.meItalic     = ITALIC_NONE;
    rDFA.mbSymbolFlag = false;

    // ignore bitmap fonts
    ATSFontRef rATSFontRef = FMGetATSFontRefFromFont( nFontID );
    ByteCount nHeadLen = 0;
    OSStatus rc = ATSFontGetTable( rATSFontRef, 0x68656164/*head*/, 0, 0, NULL, &nHeadLen );
    if( (rc != noErr) || (nHeadLen <= 0) )
        return false;

    // all scalable fonts on this platform are subsettable
    rDFA.mbSubsettable  = true;
    rDFA.mbEmbeddable   = false;
    // TODO: these members are needed only for our X11 platform targets
    rDFA.meAntiAlias    = ANTIALIAS_DONTKNOW;
    rDFA.meEmbeddedBitmap = EMBEDDEDBITMAP_DONTKNOW;

    // prepare iterating over all name strings of the font
    ItemCount nFontNameCount = 0;
    rc = ATSUCountFontNames( nFontID, &nFontNameCount );
    if( rc != noErr )
        return false;
    int nBestNameValue = 0;
    int nBestStyleValue = 0;
    FontLanguageCode eBestLangCode = 0;
    const FontLanguageCode eUILangCode = Application::GetSettings().GetUILanguage();
    typedef std::vector<char> NameBuffer;
    NameBuffer aNameBuffer( 256 );

    // iterate over all available name strings of the font
    for( ItemCount nNameIndex = 0; nNameIndex < nFontNameCount; ++nNameIndex )
    {
        ByteCount nNameLength = 0;

        FontNameCode     eFontNameCode;
        FontPlatformCode eFontNamePlatform;
        FontScriptCode   eFontNameScript;
        FontLanguageCode eFontNameLanguage;
        rc = ATSUGetIndFontName( nFontID, nNameIndex, 0, NULL,
            &nNameLength, &eFontNameCode, &eFontNamePlatform, &eFontNameScript, &eFontNameLanguage );
        if( rc != noErr )
            continue;

        // ignore non-interesting name entries
        if( (eFontNameCode != kFontFamilyName)
        &&  (eFontNameCode != kFontStyleName)
        &&  (eFontNameCode != kFontPostscriptName) )
            continue;

        // heuristic to find the most common font name
        // prefering default language names or even better the names matching to the UI language
        int nNameValue = (eFontNameLanguage==eUILangCode) ? 0 : ((eFontNameLanguage==0) ? -10 : -20);
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_UNICODE;
        const int nPlatformEncoding = ((int)eFontNamePlatform << 8) + (int)eFontNameScript;
        switch( nPlatformEncoding )
        {
            case 0x000: nNameValue += 23; break;    // Unicode 1.0
            case 0x001: nNameValue += 24; break;    // Unicode 1.1
            case 0x002: nNameValue += 25; break;    // iso10646_1993
            case 0x003: nNameValue += 26; break;    // UCS-2
            case 0x301: nNameValue += 27; break;    // Win UCS-2
            case 0x004:                             // UCS-4
            case 0x30A: nNameValue += 0;            // Win-UCS-4
                eEncoding = RTL_TEXTENCODING_UCS4;
                break;
            case 0x100: nNameValue += 21;           // Mac Roman
                eEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                break;
            case 0x300: nNameValue =  0;            // Win Symbol encoded name!
                rDFA.mbSymbolFlag = true;           // (often seen for symbol fonts)
                break;
            default: nNameValue = 0;                // ignore other encodings
                break;
        }

        // ignore name entries with no useful encoding
        if( nNameValue <= 0 )
            continue;
        if( nNameLength >= aNameBuffer.size() )
            continue;

        // get the encoded name
        aNameBuffer.reserve( nNameLength+1 ); // extra byte helps for debugging
        rc = ATSUGetIndFontName( nFontID, nNameIndex, nNameLength, &aNameBuffer[0],
            &nNameLength, &eFontNameCode, &eFontNamePlatform, &eFontNameScript, &eFontNameLanguage );
        if( rc != noErr )
            continue;

        // convert to unicode name
        UniString aUtf16Name;
        if( eEncoding == RTL_TEXTENCODING_UNICODE ) // we are just interested in UTF16 encoded names
            aUtf16Name = UniString( (const sal_Unicode*)&aNameBuffer[0], nNameLength/2 );
        else if( eEncoding == RTL_TEXTENCODING_UCS4 )
            aUtf16Name = UniString(); // TODO
        else // assume the non-unicode encoded names are byte encoded
            aUtf16Name = UniString( &aNameBuffer[0], nNameLength, eEncoding );

        // ignore empty strings
        if( aUtf16Name.Len() <= 0 )
            continue;

        // handle the name depending on its namecode
        switch( eFontNameCode )
        {
        case kFontFamilyName:
            // ignore font names starting with '.'
            if( aUtf16Name.GetChar(0) == '.' )
                nNameValue = 0;
            else if( rDFA.maName.Len() )
            {
                // even if a family name is not the one we are looking for
                // it is still useful as a font name alternative
                if( rDFA.maMapNames.Len() )
                    rDFA.maMapNames += ';';
                rDFA.maMapNames += (nBestNameValue < nNameValue) ? rDFA.maName : aUtf16Name;
            }
            if( nBestNameValue < nNameValue )
            {
                // get the best family name
                nBestNameValue = nNameValue;
                eBestLangCode = eFontNameLanguage;
                rDFA.maName = aUtf16Name;
            }
            break;
        case kFontStyleName:
            // get a style name matching to the family name
            if( nBestStyleValue < nNameValue )
            {
                nBestStyleValue = nNameValue;
                rDFA.maStyleName = aUtf16Name;
            }
            break;
        case kFontPostscriptName:
            // use the postscript name to get some useful info
            UpdateAttributesFromPSName( aUtf16Name, rDFA );
            break;
        default:
            // TODO: use other name entries too?
            break;
        }
    }

    bool bRet = (rDFA.maName.Len() > 0);
    return bRet;
}

// =======================================================================

SystemFontList* GetAtsFontList( void )
{
    return new AtsFontList();
}

// =======================================================================

AtsFontList::AtsFontList()
{
    // count available system fonts
    ItemCount nATSUICompatibleFontsAvailable = 0;
    if( ATSUFontCount(&nATSUICompatibleFontsAvailable) != noErr )
        return;
    if( nATSUICompatibleFontsAvailable <= 0 )
        return;

    // enumerate available system fonts
    typedef std::vector<ATSUFontID> AtsFontIDVector;
    AtsFontIDVector aFontIDVector( nATSUICompatibleFontsAvailable );
    ItemCount nFontItemsCount = 0;
    if( ATSUGetFontIDs( &aFontIDVector[0], aFontIDVector.capacity(), &nFontItemsCount ) != noErr )
        return;

    BOOST_ASSERT(nATSUICompatibleFontsAvailable == nFontItemsCount && "Strange I would expect them to be equal");

    // prepare use of the available fonts
    AtsFontIDVector::const_iterator it = aFontIDVector.begin();
    for(; it != aFontIDVector.end(); ++it )
    {
        const ATSUFontID nFontID = *it;
        ImplDevFontAttributes aDevFontAttr;
        if( !GetDevFontAttributes( nFontID, aDevFontAttr ) )
            continue;
        AtsFontData* pFontData = new AtsFontData( aDevFontAttr, nFontID );
        maFontContainer[ nFontID ] = pFontData;
    }

    InitGlyphFallbacks();
}

// -----------------------------------------------------------------------

AtsFontList::~AtsFontList()
{
    AtsFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        delete (*it).second;
    maFontContainer.clear();

    ATSUDisposeFontFallbacks( maFontFallbacks );
}

// -----------------------------------------------------------------------

void AtsFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
    AtsFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        rFontList.Add( (*it).second->Clone() );
}

// -----------------------------------------------------------------------

ImplMacFontData* AtsFontList::GetFontDataFromId( sal_IntPtr nFontId ) const
{
    AtsFontContainer::const_iterator it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
        return NULL;
    return (*it).second;
}

// -----------------------------------------------------------------------

// not all fonts are suitable for glyph fallback => sort them
struct GfbCompare{ bool operator()(const ImplMacFontData*, const ImplMacFontData*); };

inline bool GfbCompare::operator()( const ImplMacFontData* pA, const ImplMacFontData* pB )
{
    // use symbol fonts only as last resort
    bool bPreferA = !pA->IsSymbolFont();
    bool bPreferB = !pB->IsSymbolFont();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer scalable fonts
    bPreferA = pA->IsScalable();
    bPreferB = pB->IsScalable();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer non-slanted fonts
    bPreferA = (pA->GetSlant() == ITALIC_NONE);
    bPreferB = (pB->GetSlant() == ITALIC_NONE);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal weight fonts
    bPreferA = (pA->GetWeight() == WEIGHT_NORMAL);
    bPreferB = (pB->GetWeight() == WEIGHT_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal width fonts
    bPreferA = (pA->GetWidthType() == WIDTH_NORMAL);
    bPreferB = (pB->GetWidthType() == WIDTH_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    return false;
}

// -----------------------------------------------------------------------

void AtsFontList::InitGlyphFallbacks()
{
    // sort fonts for "glyph fallback"
    typedef std::multiset<const ImplMacFontData*,GfbCompare> FallbackSet;
    FallbackSet aFallbackSet;
    AtsFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
    {
        const ImplMacFontData* pIFD = (*it).second;
        // TODO: subsettable/embeddable glyph fallback only for PDF export?
        if( pIFD->IsSubsettable() || pIFD->IsEmbeddable() )
            aFallbackSet.insert( pIFD );
    }

    // tell ATSU about font preferences for "glyph fallback"
    typedef std::vector<ATSUFontID> AtsFontIDVector;
    AtsFontIDVector aFallbackVector;
    aFallbackVector.reserve( maFontContainer.size() );
    FallbackSet::const_iterator itFData = aFallbackSet.begin();
    for(; itFData != aFallbackSet.end(); ++itFData )
    {
        const ImplMacFontData* pFontData = (*itFData);
        ATSUFontID nFontID = (ATSUFontID)pFontData->GetFontId();
        aFallbackVector.push_back( nFontID );
    }

    ATSUCreateFontFallbacks( &maFontFallbacks );
    ATSUSetObjFontFallbacks( maFontFallbacks,
        aFallbackVector.size(), &aFallbackVector[0], kATSUSequentialFallbacksPreferred );
}

// =======================================================================

