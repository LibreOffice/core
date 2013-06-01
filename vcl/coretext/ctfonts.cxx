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

#include "impfont.hxx"
#include "outfont.hxx"
#include "sallayout.hxx"

#include "aqua/salinst.h"
#include "aqua/saldata.hxx"
#include "coretext/salgdi2.h"
#include "ctfonts.hxx"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"

#ifndef DISABLE_CORETEXT_DYNLOAD
#include <dlfcn.h>
#endif

// =======================================================================

// CoreText specific physically available font face
class CTFontData
:   public ImplMacFontData
{
public:
    explicit                CTFontData( const ImplDevFontAttributes&, sal_IntPtr nFontId );
    virtual                 ~CTFontData( void );
    virtual ImplFontData*   Clone( void ) const;

    virtual ImplMacTextStyle*   CreateMacTextStyle( const ImplFontSelectData& ) const;
    virtual ImplFontEntry*      CreateFontInstance( /*const*/ ImplFontSelectData& ) const;
    virtual int                 GetFontTable( const char pTagName[5], unsigned char* ) const;
};

// =======================================================================

class CTFontList
:   public SystemFontList
{
public:
    explicit    CTFontList( void );
    virtual     ~CTFontList( void );

    bool        Init( void );
    void        AddFont( CTFontData* );

    virtual void    AnnounceFonts( ImplDevFontList& ) const;
    virtual ImplMacFontData* GetFontDataFromId( sal_IntPtr ) const;

private:
    CTFontCollectionRef mpCTFontCollection;
    CFArrayRef mpCTFontArray;

    typedef std::hash_map<sal_IntPtr,CTFontData*> CTFontContainer;
    CTFontContainer maFontContainer;
};

// =======================================================================

CTTextStyle::CTTextStyle( const ImplFontSelectData& rFSD )
:   ImplMacTextStyle( rFSD )
,   mpStyleDict( NULL )
{
    mpFontData = (CTFontData*)rFSD.mpFontData;
    const ImplFontSelectData* const pReqFont = &rFSD;

    double fScaledFontHeight = pReqFont->mfExactHeight;
#if 0 // TODO: does CoreText need font size limiting???
    static const float fMaxFontHeight = 144.0; // TODO: is there a limit for CoreText?
    if( fScaledFontHeight > fMaxFontHeight )
    {
        mfFontScale = fScaledFontHeight / fMaxFontHeight;
        fScaledFontHeight = fMaxFontHeight;
    }
#endif

    // convert font rotation to radian
    mfFontRotation = pReqFont->mnOrientation * (M_PI / 1800.0);

    // handle font stretching if any
    const CGAffineTransform* pMatrix = NULL;
    CGAffineTransform aMatrix;
    if( (pReqFont->mnWidth != 0) && (pReqFont->mnWidth != pReqFont->mnHeight) )
    {
        mfFontStretch = (float)pReqFont->mnWidth / pReqFont->mnHeight;
        aMatrix = CGAffineTransformMakeScale( mfFontStretch, 1.0F );
        pMatrix = &aMatrix;
    }

    // create the style object for CoreText font attributes
    static const CFIndex nMaxDictSize = 16; // TODO: does this really suffice?
    mpStyleDict = CFDictionaryCreateMutable( NULL, nMaxDictSize,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

    // set some default styles: no kerning, regular ligatures
    static const CGFloat fValZero = 0.0;
    CFNumberRef pCFFloatNumZero = CFNumberCreate( NULL, kCFNumberFloatType, &fValZero );
    CFDictionarySetValue( mpStyleDict, kCTKernAttributeName, pCFFloatNumZero );
    CFRelease( pCFFloatNumZero);
    static const int nValOne = 1;
    CFNumberRef pCFIntNumOne = CFNumberCreate( NULL, kCFNumberIntType, &nValOne );
    CFDictionarySetValue( mpStyleDict, kCTLigatureAttributeName, pCFIntNumOne );
    CFRelease( pCFIntNumOne);
    CFBooleanRef pCFVertBool = pReqFont->mbVertical ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue( mpStyleDict, kCTVerticalFormsAttributeName, pCFVertBool );

    CTFontDescriptorRef pFontDesc = (CTFontDescriptorRef)mpFontData->GetFontId();
    CTFontRef pNewCTFont = CTFontCreateWithFontDescriptor( pFontDesc, fScaledFontHeight, pMatrix );
    CFDictionarySetValue( mpStyleDict, kCTFontAttributeName, pNewCTFont );
    CFRelease( pNewCTFont);

#if 0 // LastResort is implicit in CoreText's font cascading
    const void* aGFBDescriptors[] = { CTFontDescriptorCreateWithNameAndSize( CFSTR("LastResort"), 0) }; // TODO: use the full GFB list
    const int nGfbCount = sizeof(aGFBDescriptors) / sizeof(*aGFBDescriptors);
    CFArrayRef pGfbList = CFArrayCreate( NULL, aGFBDescriptors, nGfbCount, &kCFTypeArrayCallBacks);
    CFDictionaryAddValue( mpStyleDict, kCTFontCascadeListAttribute, pGfbList);
    CFRelease( pGfbList);
#endif
}

// -----------------------------------------------------------------------

CTTextStyle::~CTTextStyle( void )
{
    if( mpStyleDict )
        CFRelease( mpStyleDict );
}

// -----------------------------------------------------------------------

void CTTextStyle::GetFontMetric( float fDPIY, ImplFontMetricData& rMetric ) const
{
    // get the matching CoreText font handle
    // TODO: is it worth it to cache the CTFontRef in SetFont() and reuse it here?
    CTFontRef aCTFontRef = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );

    const double fPixelSize = (mfFontScale * fDPIY);
    rMetric.mnAscent       = lrint( CTFontGetAscent( aCTFontRef ) * fPixelSize);
    rMetric.mnDescent      = lrint( CTFontGetDescent( aCTFontRef ) * fPixelSize);
    rMetric.mnIntLeading   = lrint( CTFontGetLeading( aCTFontRef ) * fPixelSize);
    rMetric.mnExtLeading   = 0;
    // since ImplFontMetricData::mnWidth is only used for stretching/squeezing fonts
    // setting this width to the pixel height of the fontsize is good enough
    // it also makes the calculation of the stretch factor simple
    rMetric.mnWidth        = lrint( CTFontGetSize( aCTFontRef ) * fPixelSize * mfFontStretch);

    // all CoreText fonts are scalable
    rMetric.mbScalableFont = true;
    // TODO: check if any kerning is supported
    rMetric.mbKernableFont = true;
}

// -----------------------------------------------------------------------

bool CTTextStyle::GetGlyphBoundRect( sal_GlyphId nGlyphId, Rectangle& rRect ) const
{
    const DynCoreTextSyms& rCT = DynCoreTextSyms::get();
    CGGlyph nCGGlyph = nGlyphId & GF_IDXMASK; // NOTE: CoreText handles glyph fallback itself
    CTFontRef aCTFontRef = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );

    const CTFontOrientation aFontOrientation = kCTFontDefaultOrientation; // TODO: horz/vert
    const CGRect aCGRect = rCT.FontGetBoundingRectsForGlyphs( aCTFontRef, aFontOrientation, &nCGGlyph, NULL, 1 );

    rRect.Left()   = lrint( mfFontScale * aCGRect.origin.x );
    rRect.Top()    = lrint( mfFontScale * aCGRect.origin.y );
    rRect.Right()  = lrint( mfFontScale * (aCGRect.origin.x + aCGRect.size.width) );
    rRect.Bottom() = lrint( mfFontScale * (aCGRect.origin.y + aCGRect.size.height) );
    return true;
}

// -----------------------------------------------------------------------

// callbacks from CTFontCreatePathForGlyph+CGPathApply for GetGlyphOutline()
struct GgoData { basegfx::B2DPolygon maPolygon; basegfx::B2DPolyPolygon* mpPolyPoly; };

static void MyCGPathApplierFunc( void* pData, const CGPathElement* pElement )
{
    basegfx::B2DPolygon& rPolygon = static_cast<GgoData*>(pData)->maPolygon;
    const int nPointCount = rPolygon.count();

    switch( pElement->type )
    {
    case kCGPathElementCloseSubpath:
    case kCGPathElementMoveToPoint:
        if( nPointCount > 0 ) {
            static_cast<GgoData*>(pData)->mpPolyPoly->append( rPolygon );
            rPolygon.clear();
        }
        // fall through for kCGPathElementMoveToPoint:
        if( pElement->type != kCGPathElementMoveToPoint )
            break;
    case kCGPathElementAddLineToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[0].x, -pElement->points[0].y ) );
        break;
    case kCGPathElementAddCurveToPoint:
        rPolygon.append( basegfx::B2DPoint( +pElement->points[2].x, -pElement->points[2].y ) );
        rPolygon.setNextControlPoint( nPointCount-1, basegfx::B2DPoint( pElement->points[0].x, -pElement->points[0].y ) );
        rPolygon.setPrevControlPoint( nPointCount+0, basegfx::B2DPoint( pElement->points[1].x, -pElement->points[1].y ) );
        break;
    case kCGPathElementAddQuadCurveToPoint: {
        const basegfx::B2DPoint aStartPt = rPolygon.getB2DPoint( nPointCount-1 );
        const basegfx::B2DPoint aCtrPt1( (aStartPt.getX() + 2* pElement->points[0].x) / 3.0,
                    (aStartPt.getY() - 2 * pElement->points[0].y) / 3.0 );
        const basegfx::B2DPoint aCtrPt2( (+2 * +pElement->points[0].x + pElement->points[1].x) / 3.0,
                (-2 * pElement->points[0].y - pElement->points[1].y) / 3.0 );
        rPolygon.append( basegfx::B2DPoint( +pElement->points[1].x, -pElement->points[1].y ) );
        rPolygon.setNextControlPoint( nPointCount-1, aCtrPt1 );
        rPolygon.setPrevControlPoint( nPointCount+0, aCtrPt2 );
        } break;
    }
}

bool CTTextStyle::GetGlyphOutline( sal_GlyphId nGlyphId, basegfx::B2DPolyPolygon& rResult ) const
{
    rResult.clear();

    const DynCoreTextSyms& rCT = DynCoreTextSyms::get();
    // TODO: GF_FONTMASK if using non-native glyph fallback
    CGGlyph nCGGlyph = nGlyphId & GF_IDXMASK;
    CTFontRef pCTFont = (CTFontRef)CFDictionaryGetValue( mpStyleDict, kCTFontAttributeName );
    CGPathRef xPath = rCT.FontCreatePathForGlyph( pCTFont, nCGGlyph, NULL );

    GgoData aGgoData;
    aGgoData.mpPolyPoly = &rResult;
    CGPathApply( xPath, (void*)&aGgoData, MyCGPathApplierFunc );
#if 0 // TODO: does OSX ensure that the last polygon is always closed?
    const CGPathElement aClosingElement = { kCGPathElementCloseSubpath, NULL };
    MyCGPathApplierFunc( (void*)&aGgoData, &aClosingElement );
#endif

    // apply the font scale
    if( mfFontScale != 1.0 ) {
        basegfx::B2DHomMatrix aScale;
        aScale.scale( +mfFontScale, +mfFontScale );
        rResult.transform( aScale );
    }

    return true;
}

// -----------------------------------------------------------------------

void CTTextStyle::SetTextColor( const RGBAColor& rColor )
{
#if (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
    CGColorRef pCGColor = CGColorCreateGenericRGB( rColor.GetRed(),
        rColor.GetGreen(), rColor.GetBlue(), rColor.GetAlpha() );
#else // for builds on OSX 10.4 SDK
    const CGColorSpaceRef pCGColorSpace = GetSalData()->mxRGBSpace;
    CGColorRef pCGColor = CGColorCreate( pCGColorSpace, rColor.AsArray() );
#endif
    CFDictionarySetValue( mpStyleDict, kCTForegroundColorAttributeName, pCGColor );
    CFRelease( pCGColor);
}

// =======================================================================

CTFontData::CTFontData( const ImplDevFontAttributes& rDFA, sal_IntPtr nFontId )
:   ImplMacFontData( rDFA, nFontId )
{}

// -----------------------------------------------------------------------

CTFontData::~CTFontData( void )
{
    // TODO: any resources to release?
}

// -----------------------------------------------------------------------

ImplFontData* CTFontData::Clone( void ) const
{
    return new CTFontData( *this);
}

// -----------------------------------------------------------------------

ImplMacTextStyle* CTFontData::CreateMacTextStyle( const ImplFontSelectData& rFSD ) const
{
    return new CTTextStyle( rFSD);
}

// -----------------------------------------------------------------------

ImplFontEntry* CTFontData::CreateFontInstance( /*const*/ ImplFontSelectData& rFSD ) const
{
    return new ImplFontEntry( rFSD);
}

// -----------------------------------------------------------------------

int CTFontData::GetFontTable( const char pTagName[5], unsigned char* pResultBuf ) const
{
    DBG_ASSERT( aTagName[4]=='\0', "CTFontData::GetFontTable with invalid tagname!\n" );

    const CTFontTableTag nTagCode = (pTagName[0]<<24) + (pTagName[1]<<16) + (pTagName[2]<<8) + (pTagName[3]<<0);

    // get the raw table length
    CTFontDescriptorRef pFontDesc = reinterpret_cast<CTFontDescriptorRef>( GetFontId());
    CTFontRef rCTFont = CTFontCreateWithFontDescriptor( pFontDesc, 0.0, NULL);
    CFDataRef pDataRef = CTFontCopyTable( rCTFont, nTagCode, kCTFontTableOptionExcludeSynthetic);
    CFRelease( rCTFont);
    if( !pDataRef)
        return 0;

    const CFIndex nByteLength = CFDataGetLength( pDataRef);

    // get the raw table data if requested
    if( pResultBuf && (nByteLength > 0))
    {
        const CFRange aFullRange = CFRangeMake( 0, nByteLength);
        CFDataGetBytes( pDataRef, aFullRange, (UInt8*)pResultBuf);
    }

    CFRelease( pDataRef);

    return (int)nByteLength;
}

// =======================================================================

static void CTFontEnumCallBack( const void* pValue, void* pContext )
{
    CTFontDescriptorRef pFD = static_cast<CTFontDescriptorRef>(pValue);

    // all CoreText fonts are device fonts that can rotate just fine
    ImplDevFontAttributes rDFA;
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

    // reset the font attributes
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.mePitch      = PITCH_VARIABLE;
    rDFA.meWidthType  = WIDTH_NORMAL;
    rDFA.meWeight     = WEIGHT_NORMAL;
    rDFA.meItalic     = ITALIC_NONE;
    rDFA.mbSymbolFlag = false;

    // all scalable fonts on this platform are subsettable
    rDFA.mbEmbeddable = false;
    rDFA.mbSubsettable = true;

    // get font name
    // TODO: use kCTFontDisplayNameAttribute instead???
    CFStringRef pFamilyName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute );
    rDFA.maName = GetOUString( pFamilyName );
    // get font style
    CFStringRef pStyleName = (CFStringRef)CTFontDescriptorCopyAttribute( pFD, kCTFontStyleNameAttribute );
    rDFA.maStyleName = GetOUString( pStyleName );

    // get font-enabled status
    int bFontEnabled = FALSE;
    CFNumberRef pFontEnabled = (CFNumberRef)CTFontDescriptorCopyAttribute( pFD, kCTFontEnabledAttribute );
    CFNumberGetValue( pFontEnabled, kCFNumberIntType, &bFontEnabled );

    // get font attributes
    CFDictionaryRef pAttrDict = (CFDictionaryRef)CTFontDescriptorCopyAttribute( pFD, kCTFontTraitsAttribute );

    // get symbolic trait
    // TODO: use other traits such as MonoSpace/Condensed/Expanded or Vertical too
    SInt64 nSymbolTrait = 0;
    CFNumberRef pSymbolNum = NULL;
    if( CFDictionaryGetValueIfPresent( pAttrDict, kCTFontSymbolicTrait, (const void**)&pSymbolNum ) ) {
        CFNumberGetValue( pSymbolNum, kCFNumberSInt64Type, &nSymbolTrait );
        rDFA.mbSymbolFlag = ((nSymbolTrait & kCTFontClassMaskTrait) == kCTFontSymbolicClass);
    }

    // get the font weight
    double fWeight = 0;
    CFNumberRef pWeightNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontWeightTrait );
    CFNumberGetValue( pWeightNum, kCFNumberDoubleType, &fWeight );
    int nInt = WEIGHT_NORMAL;
    if( fWeight > 0 ) {
        nInt = rint(WEIGHT_NORMAL + fWeight * ((WEIGHT_BLACK - WEIGHT_NORMAL)/0.68));
        if( nInt > WEIGHT_BLACK )
            nInt = WEIGHT_BLACK;
    } else if( fWeight < 0 ) {
        nInt = rint(WEIGHT_NORMAL + fWeight * ((WEIGHT_NORMAL - WEIGHT_THIN)/0.9));
        if( nInt < WEIGHT_THIN )
            nInt = WEIGHT_THIN;
    }
    rDFA.meWeight = (FontWeight)nInt;

    // get the font slant
    double fSlant = 0;
    CFNumberRef pSlantNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontSlantTrait );
    CFNumberGetValue( pSlantNum, kCFNumberDoubleType, &fSlant );
    if( fSlant >= 0.035 )
        rDFA.meItalic = ITALIC_NORMAL;

    // get width trait
    double fWidth = 0;
    CFNumberRef pWidthNum = (CFNumberRef)CFDictionaryGetValue( pAttrDict, kCTFontWidthTrait );
    CFNumberGetValue( pWidthNum, kCFNumberDoubleType, &fWidth );
    nInt = WIDTH_NORMAL;
    if( fWidth > 0 ) {
        nInt = rint( WIDTH_NORMAL + fWidth * ((WIDTH_ULTRA_EXPANDED - WIDTH_NORMAL)/0.4));
        if( nInt > WIDTH_ULTRA_EXPANDED )
            nInt = WIDTH_ULTRA_EXPANDED;
    } else if( fWidth < 0 ) {
        nInt = rint( WIDTH_NORMAL + fWidth * ((WIDTH_NORMAL - WIDTH_ULTRA_CONDENSED)/0.5));
        if( nInt < WIDTH_ULTRA_CONDENSED )
            nInt = WIDTH_ULTRA_CONDENSED;
    }
    rDFA.meWidthType = (FontWidth)nInt;

    // release the attribute dict that we had copied
    CFRelease( pAttrDict );

    // TODO? also use the HEAD table if available to get more attributes
//  CFDataRef CTFontCopyTable( CTFontRef, kCTFontTableHead, /*kCTFontTableOptionNoOptions*/kCTFontTableOptionExcludeSynthetic );

#if (OSL_DEBUG_LEVEL >= 1)
    // update font attributes using the font's postscript name
    ImplDevFontAttributes rDFA2;
    CTFontRef pFont = CTFontCreateWithFontDescriptor( pFD, 0.0, NULL );
    CFStringRef pPSName = CTFontCopyPostScriptName( pFont );
    const String aPSName = GetOUString( pPSName );

    rDFA2.mbSymbolFlag = false;
    rDFA2.mePitch      = PITCH_VARIABLE;
    rDFA2.meWidthType  = WIDTH_NORMAL;
    rDFA2.meWeight     = WEIGHT_NORMAL;
    rDFA2.meItalic     = ITALIC_NONE;

    UpdateAttributesFromPSName( aPSName, rDFA2 );
    CFRelease( pPSName );
    CFRelease( pFont );

    // show the font details and compare the CTFontDescriptor vs. PSName traits
    char cMatch = (rDFA.mbSymbolFlag==rDFA2.mbSymbolFlag);
    cMatch &= (rDFA.meWeight==rDFA2.meWeight);
    cMatch &= ((rDFA.meItalic==ITALIC_NONE) == (rDFA2.meItalic==ITALIC_NONE));
    cMatch &= (rDFA.meWidthType==rDFA2.meWidthType);
    cMatch = cMatch ? '.' : '#';

    char aFN[256], aSN[256];
    CFStringGetCString( pFamilyName, aFN, sizeof(aFN), kCFStringEncodingUTF8 );
    CFStringGetCString( pStyleName, aSN, sizeof(aSN), kCFStringEncodingUTF8 );

    const ByteString aPSCName( aPSName, RTL_TEXTENCODING_UTF8 );
    const char* aPN = aPSCName.GetBuffer();
    printf("\tCTFont_%d%x%d%d_%c_%d%x%d%d ena=%d s=%02d b=%+.2f i=%+.2f w=%+.2f (\"%s\", \"%s\", \"%s\")\n",
        (int)rDFA.mbSymbolFlag,(int)rDFA.meWeight,(int)rDFA.meItalic,(int)rDFA.meWidthType,
        cMatch,
        (int)rDFA2.mbSymbolFlag,(int)rDFA2.meWeight,(int)rDFA2.meItalic,(int)rDFA2.meWidthType,
        bFontEnabled,
        (int)(nSymbolTrait>>kCTFontClassMaskShift),fWeight,fSlant,fWidth,aFN,aSN,aPN);
#endif // (OSL_DEBUG_LEVEL >= 1)

    if( bFontEnabled)
    {
        const sal_IntPtr nFontId = (sal_IntPtr)pValue;
        CTFontData* pFontData = new CTFontData( rDFA, nFontId );
        CTFontList* pFontList = (CTFontList*)pContext;
        pFontList->AddFont( pFontData );
    }
}

// =======================================================================

CTFontList::CTFontList()
:   mpCTFontCollection( NULL )
,   mpCTFontArray( NULL )
{}

// -----------------------------------------------------------------------

CTFontList::~CTFontList()
{
    CTFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        delete (*it).second;
    maFontContainer.clear();

    if( mpCTFontArray )
        CFRelease( mpCTFontArray );
    if( mpCTFontCollection )
        CFRelease( mpCTFontCollection );
}

// -----------------------------------------------------------------------

void CTFontList::AddFont( CTFontData* pFontData )
{
    sal_IntPtr nFontId = pFontData->GetFontId();
    maFontContainer[ nFontId ] = pFontData;
}

// -----------------------------------------------------------------------

void CTFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
    CTFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        rFontList.Add( (*it).second->Clone() );
}

// -----------------------------------------------------------------------

ImplMacFontData* CTFontList::GetFontDataFromId( sal_IntPtr nFontId ) const
{
    CTFontContainer::const_iterator it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
        return NULL;
    return (*it).second;
}

// -----------------------------------------------------------------------

bool CTFontList::Init( void )
{
#ifndef DISABLE_CORETEXT_DYNLOAD
    // check availability of the CoreText API
    const DynCoreTextSyms& rCT = DynCoreTextSyms::get();
    if( !rCT.IsActive() )
        return false;
#endif // DISABLE_CORETEXT_DYNLOAD

    // enumerate available system fonts
    static const int nMaxDictEntries = 8;
    CFMutableDictionaryRef pCFDict = CFDictionaryCreateMutable( NULL,
        nMaxDictEntries, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    CFDictionaryAddValue( pCFDict, kCTFontCollectionRemoveDuplicatesOption, kCFBooleanTrue );
    mpCTFontCollection = rCT.FontCollectionCreateFromAvailableFonts( pCFDict );
    CFRelease( pCFDict );

    mpCTFontArray = rCT.FontCollectionCreateMatchingFontDescriptors( mpCTFontCollection );
    const int nFontCount = CFArrayGetCount( mpCTFontArray );
    const CFRange aFullRange = CFRangeMake( 0, nFontCount );
    CFArrayApplyFunction( mpCTFontArray, aFullRange, CTFontEnumCallBack, this );

    return true;
}

// =======================================================================

#ifndef DISABLE_CORETEXT_DYNLOAD

DynCoreTextSyms::DynCoreTextSyms( void )
{
    mbIsActive = false;

    // check if CoreText has been explicitely disabled
    const char* pEnvStr = getenv( "SAL_DISABLE_CORETEXT");
    if( pEnvStr && (pEnvStr[0] != '0') )
        return;

    // check CoreText version
    GetCoreTextVersion = (uint32_t(*)(void))dlsym( RTLD_DEFAULT, "CTGetCoreTextVersion");
    if( !GetCoreTextVersion) return;

    const uint32_t nCTVersion = GetCoreTextVersion();
    static const uint32_t mykCTVersionNumber10_5 = 0x00020000;
    if( nCTVersion < mykCTVersionNumber10_5)
        return;

    // load CoreText symbols dynamically
    LineGetTrailingWhitespaceWidth = (double(*)(CTLineRef))dlsym( RTLD_DEFAULT, "CTLineGetTrailingWhitespaceWidth");
    if( !LineGetTrailingWhitespaceWidth) return;

    LineCreateJustifiedLine = (CTLineRef(*)(CTLineRef,CGFloat,double))dlsym( RTLD_DEFAULT, "CTLineCreateJustifiedLine");
    if( !LineCreateJustifiedLine) return;

    LineGetOffsetForStringIndex = (CGFloat(*)(CTLineRef,CFIndex,CGFloat*))dlsym( RTLD_DEFAULT, "CTLineGetOffsetForStringIndex");
    if( !LineGetOffsetForStringIndex) return;

    LineGetGlyphRuns = (CFArrayRef(*)(CTLineRef))dlsym( RTLD_DEFAULT, "CTLineGetGlyphRuns");
    if( !LineGetGlyphRuns) return;

    RunGetGlyphCount = (CFIndex(*)(CTRunRef))dlsym( RTLD_DEFAULT, "CTRunGetGlyphCount");
    if( !RunGetGlyphCount) return;

    RunGetGlyphsPtr = (const CGGlyph*(*)(CTRunRef))dlsym( RTLD_DEFAULT, "CTRunGetGlyphsPtr");
    if( !RunGetGlyphsPtr) return;

    RunGetPositionsPtr = (const CGPoint*(*)(CTRunRef))dlsym( RTLD_DEFAULT, "CTRunGetPositionsPtr");
    if( !RunGetPositionsPtr) return;

    RunGetAdvancesPtr = (const CGSize*(*)(CTRunRef))dlsym( RTLD_DEFAULT, "CTRunGetAdvancesPtr");
    if( !RunGetAdvancesPtr) return;

    RunGetStringIndicesPtr = (const CFIndex*(*)(CTRunRef))dlsym( RTLD_DEFAULT, "CTRunGetStringIndicesPtr");
    if( !RunGetStringIndicesPtr) return;

    FontCollectionCreateFromAvailableFonts = (CTFontCollectionRef(*)(CFDictionaryRef))dlsym( RTLD_DEFAULT, "CTFontCollectionCreateFromAvailableFonts");
    if( !FontCollectionCreateFromAvailableFonts) return;

    FontCollectionCreateMatchingFontDescriptors = (CFArrayRef(*)(CTFontCollectionRef))dlsym( RTLD_DEFAULT, "CTFontCollectionCreateMatchingFontDescriptors");
    if( !FontCollectionCreateMatchingFontDescriptors) return;

    FontCreatePathForGlyph = (CGPathRef(*)(CTFontRef,CGGlyph,const CGAffineTransform*))dlsym( RTLD_DEFAULT, "CTFontCreatePathForGlyph");
    if( !FontCreatePathForGlyph) return;

    FontGetBoundingRectsForGlyphs = (CGRect(*)(CTFontRef,CTFontOrientation,CGGlyph*,CGRect*,CFIndex))dlsym( RTLD_DEFAULT, "CTFontGetBoundingRectsForGlyphs");
    if( !FontGetBoundingRectsForGlyphs) return;

    mbIsActive = true;
}

// -----------------------------------------------------------------------

const DynCoreTextSyms& DynCoreTextSyms::get( void )
{
    static DynCoreTextSyms aCT;
    return aCT;
}

#endif // DISABLE_CORETEXT_DYNLOAD

// =======================================================================

SystemFontList* GetCoretextFontList( void )
{
    CTFontList* pList = new CTFontList();
    if( !pList->Init() ) {
        delete pList;
        return NULL;
    }

    return pList;
}

// =======================================================================

