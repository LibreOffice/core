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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <rtl/ustrbuf.hxx>

#include <tools/debug.hxx>
#include <tools/fontenum.hxx>
#include <tools/string.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/i18nmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/families.hxx>
#include <xmloff/maptype.hxx>

#include <xmloff/xmlnumi.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::xmloff::token;
using namespace ::com::sun::star::io;

using rtl::OUString;
using rtl::OUStringBuffer;

static sal_Char const XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE[] =
        "SymbolTextDistance";
static sal_Char const XML_UNO_NAME_NRULE_PARENT_NUMBERING[] =
        "ParentNumbering";
static sal_Char const XML_UNO_NAME_NRULE_CHAR_STYLE_NAME[] =
        "CharStyleName";
static sal_Char const XML_UNO_NAME_NRULE_BULLET_CHAR[] ="BulletChar";
static sal_Char const XML_UNO_NAME_NRULE_BULLET_RELSIZE[] = "BulletRelSize";
static sal_Char const XML_UNO_NAME_NRULE_GRAPHIC_SIZE[] =
        "GraphicSize";
static sal_Char const XML_UNO_NAME_NRULE_VERT_ORIENT[] ="VertOrient";

static sal_Char const XML_UNO_NAME_NRULE_NUMBERINGTYPE[] = "NumberingType";
static sal_Char const XML_UNO_NAME_NRULE_PREFIX[] = "Prefix";
static sal_Char const XML_UNO_NAME_NRULE_SUFFIX[] = "Suffix";
static sal_Char const XML_UNO_NAME_NRULE_ADJUST[] = "Adjust";
static sal_Char const XML_UNO_NAME_NRULE_LEFT_MARGIN[] = "LeftMargin";
static sal_Char const XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET[] =
    "FirstLineOffset";
static sal_Char const XML_UNO_NAME_NRULE_BULLET_FONT[] = "BulletFont";
static sal_Char const XML_UNO_NAME_NRULE_GRAPHICURL[] = "GraphicURL";
static sal_Char const XML_UNO_NAME_NRULE_START_WITH[] = "StartWith";
static sal_Char const XML_UNO_NAME_NRULE_BULLET_COLOR[] = "BulletColor";
static sal_Char const XML_UNO_NAME_NRULE_POSITION_AND_SPACE_MODE[] = "PositionAndSpaceMode";
static sal_Char const XML_UNO_NAME_NRULE_LABEL_FOLLOWED_BY[] = "LabelFollowedBy";
static sal_Char const XML_UNO_NAME_NRULE_LISTTAB_STOP_POSITION[] = "ListtabStopPosition";
static sal_Char const XML_UNO_NAME_NRULE_FIRST_LINE_INDENT[] = "FirstLineIndent";
static sal_Char const XML_UNO_NAME_NRULE_INDENT_AT[] = "IndentAt";

// ---------------------------------------------------------------------

class SvxXMLListLevelStyleContext_Impl;

class SvxXMLListLevelStyleAttrContext_Impl : public SvXMLImportContext
{
    SvxXMLListLevelStyleContext_Impl&   rListLevel;

public:

    SvxXMLListLevelStyleAttrContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
             const OUString& rLName,
              const Reference< xml::sax::XAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel   );
    virtual ~SvxXMLListLevelStyleAttrContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
};

// ---------------------------------------------------------------------

class SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl : public SvXMLImportContext
{
    SvxXMLListLevelStyleContext_Impl&   rListLevel;

public:

    SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel   );
    virtual ~SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl();
};

// ---------------------------------------------------------------------

enum SvxXMLTextListLevelStyleAttrTokens
{
    XML_TOK_TEXT_LEVEL_ATTR_LEVEL,
    XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME,
    XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR,
    XML_TOK_TEXT_LEVEL_ATTR_HREF,
    XML_TOK_TEXT_LEVEL_ATTR_TYPE,
    XML_TOK_TEXT_LEVEL_ATTR_SHOW,
    XML_TOK_TEXT_LEVEL_ATTR_ACTUATE,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC,
    XML_TOK_TEXT_LEVEL_ATTR_START_VALUE,
    XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS,

    XML_TOK_TEXT_LEVEL_ATTR_END=XML_TOK_UNKNOWN
};

static const SvXMLTokenMapEntry* lcl_getLevelAttrTokenMap()
{
    static SvXMLTokenMapEntry aLevelAttrTokenMap[] =
    {
        { XML_NAMESPACE_TEXT, XML_LEVEL, XML_TOK_TEXT_LEVEL_ATTR_LEVEL },
        { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME },
        { XML_NAMESPACE_TEXT, XML_BULLET_CHAR, XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR },
        { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_TEXT_LEVEL_ATTR_HREF },
        { XML_NAMESPACE_XLINK, XML_TYPE, XML_TOK_TEXT_LEVEL_ATTR_TYPE },
        { XML_NAMESPACE_XLINK, XML_SHOW, XML_TOK_TEXT_LEVEL_ATTR_SHOW },
        { XML_NAMESPACE_XLINK, XML_ACTUATE, XML_TOK_TEXT_LEVEL_ATTR_ACTUATE },

        { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT },
        { XML_NAMESPACE_STYLE, XML_NUM_PREFIX, XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX },
        { XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX },
        { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC, XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC },
        { XML_NAMESPACE_TEXT, XML_START_VALUE, XML_TOK_TEXT_LEVEL_ATTR_START_VALUE },
        { XML_NAMESPACE_TEXT, XML_DISPLAY_LEVELS, XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS },

        XML_TOKEN_MAP_END
    };
    return aLevelAttrTokenMap;
}

class SvxXMLListLevelStyleContext_Impl : public SvXMLImportContext
{
    friend class SvxXMLListLevelStyleAttrContext_Impl;
    const OUString      sStarBats;
    const OUString      sStarMath;

    OUString            sPrefix;
    OUString            sSuffix;
    OUString            sTextStyleName;
    OUString            sNumFormat;
    OUString            sNumLetterSync;
    OUString            sBulletFontName;
    OUString            sBulletFontStyleName;
    OUString            sImageURL;

    Reference < XOutputStream > xBase64Stream;

    sal_Int32           nLevel;
    sal_Int32           nSpaceBefore;
    sal_Int32           nMinLabelWidth;
    sal_Int32           nMinLabelDist;
    sal_Int32           nImageWidth;
    sal_Int32           nImageHeight;
    sal_Int16           nNumStartValue;
    sal_Int16           nNumDisplayLevels;

    sal_Int16           eAdjust;
    sal_Int16           eBulletFontFamily;
    sal_Int16           eBulletFontPitch;
    rtl_TextEncoding    eBulletFontEncoding;
    sal_Int16           eImageVertOrient;

    sal_Unicode         cBullet;

    sal_Int16           nRelSize;
    sal_Int32           m_nColor;

    sal_Int16           ePosAndSpaceMode;
    sal_Int16           eLabelFollowedBy;
    sal_Int32           nListtabStopPosition;
    sal_Int32           nFirstLineIndent;
    sal_Int32           nIndentAt;

    sal_Bool            bBullet : 1;
    sal_Bool            bImage : 1;
    sal_Bool            bNum : 1;
    sal_Bool            bHasColor : 1;

    void SetRelSize( sal_Int16 nRel ) { nRelSize = nRel; }
    void SetColor( sal_Int32 nColor )
        { m_nColor = nColor; bHasColor = sal_True; }
    void SetSpaceBefore( sal_Int32 nSet ) { nSpaceBefore = nSet; }
    void SetMinLabelWidth( sal_Int32 nSet ) { nMinLabelWidth = nSet; }
    void SetMinLabelDist( sal_Int32 nSet ) { nMinLabelDist = nSet; }
    void SetAdjust( sal_Int16 eSet ) { eAdjust = eSet; }

    void SetBulletFontName( const OUString& rSet ) { sBulletFontName = rSet; }
    void SetBulletFontStyleName( const OUString& rSet )
         { sBulletFontStyleName = rSet; }
    void SetBulletFontFamily( sal_Int16 eSet ) { eBulletFontFamily = eSet; }
    void SetBulletFontPitch( sal_Int16 eSet ) { eBulletFontPitch = eSet; }
    void SetBulletFontEncoding( rtl_TextEncoding eSet )
         { eBulletFontEncoding = eSet; }

    void SetImageWidth( sal_Int32 nSet ) { nImageWidth = nSet; }
    void SetImageHeight( sal_Int32 nSet ) { nImageHeight = nSet; }
    void SetImageVertOrient( sal_Int16 eSet )
         { eImageVertOrient = eSet; }

public:

    SvxXMLListLevelStyleContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SvxXMLListLevelStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    sal_Int32 GetLevel() const { return nLevel; }
    Sequence<beans::PropertyValue> GetProperties(
            const SvI18NMap *pI18NMap=0 );

    inline void SetPosAndSpaceMode( sal_Int16 eValue )
    {
        ePosAndSpaceMode = eValue;
    }
    inline void SetLabelFollowedBy( sal_Int16 eValue )
    {
        eLabelFollowedBy = eValue;
    }
    inline void SetListtabStopPosition( sal_Int32 nValue )
    {
        nListtabStopPosition = nValue;
    }
    inline void SetFirstLineIndent( sal_Int32 nValue )
    {
        nFirstLineIndent = nValue;
    }
    inline void SetIndentAt( sal_Int32 nValue )
    {
        nIndentAt = nValue;
    }
};

SvxXMLListLevelStyleContext_Impl::SvxXMLListLevelStyleContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList )

:   SvXMLImportContext( rImport, nPrfx, rLName )
,   sStarBats( "StarBats"  )
,   sStarMath( "StarMath"  )
,   sNumFormat( OUString("1") )
,   nLevel( -1L )
,   nSpaceBefore( 0L )
,   nMinLabelWidth( 0L )
,   nMinLabelDist( 0L )
,   nImageWidth( 0L )
,   nImageHeight( 0L )
,   nNumStartValue( 1 )
,   nNumDisplayLevels( 1 )
,   eAdjust( HoriOrientation::LEFT )
,   eBulletFontFamily( FAMILY_DONTKNOW )
,   eBulletFontPitch( PITCH_DONTKNOW )
,   eBulletFontEncoding( RTL_TEXTENCODING_DONTKNOW )
,   eImageVertOrient(0)
,   cBullet( 0 )
,   nRelSize(0)
,   m_nColor(0)
,   ePosAndSpaceMode( PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION )
,   eLabelFollowedBy( LabelFollow::LISTTAB )
,   nListtabStopPosition( 0 )
,   nFirstLineIndent( 0 )
,   nIndentAt( 0 )
,   bBullet( sal_False )
,   bImage( sal_False )
,   bNum( sal_False )
,   bHasColor( sal_False )
{
    if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_NUMBER ) ||
        IsXMLToken( rLName, XML_OUTLINE_LEVEL_STYLE )        )
        bNum = sal_True;
    else if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_BULLET ) )
        bBullet = sal_True;
    else if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_IMAGE ) )
        bImage = sal_True;

    SvXMLTokenMap aTokenMap( lcl_getLevelAttrTokenMap() );
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_LEVEL_ATTR_LEVEL:
            nLevel = rValue.toInt32();
            if( nLevel >= 1L )
                nLevel--;
            else
                nLevel = 0;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME:
            sTextStyleName = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR:
            cBullet = rValue[0];
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_HREF:
            if( bImage )
                sImageURL = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_TYPE:
        case XML_TOK_TEXT_LEVEL_ATTR_SHOW:
        case XML_TOK_TEXT_LEVEL_ATTR_ACTUATE:
            // This properties will be ignored
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT:
            if( bNum )
                sNumFormat = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX:
            sPrefix = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX:
            sSuffix = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC:
            if( bNum )
                sNumLetterSync = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_START_VALUE:
            if( bNum )
            {
                sal_Int32 nTmp = rValue.toInt32();
                nNumStartValue =
                    (nTmp < 0L) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : (sal_Int16)nTmp );
            }
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS:
            if( bNum )
            {
                sal_Int32 nTmp = rValue.toInt32();
                nNumDisplayLevels =
                    (nTmp < 1L) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : (sal_Int16)nTmp );
            }
            break;
        }
    }
}

SvxXMLListLevelStyleContext_Impl::~SvxXMLListLevelStyleContext_Impl()
{
}

SvXMLImportContext *SvxXMLListLevelStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if( XML_NAMESPACE_STYLE == nPrefix &&
        ( IsXMLToken( rLocalName, XML_LIST_LEVEL_PROPERTIES ) ||
             IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) ) )
    {
        pContext = new SvxXMLListLevelStyleAttrContext_Impl( GetImport(),
                                                             nPrefix,
                                                             rLocalName,
                                                               xAttrList,
                                                             *this );
    }
    else if( (XML_NAMESPACE_OFFICE == nPrefix) && xmloff::token::IsXMLToken( rLocalName,
                                        xmloff::token::XML_BINARY_DATA ) )
    {
        if( bImage && sImageURL.isEmpty() && !xBase64Stream.is() )
        {
            xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( xBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    xBase64Stream );
        }
    }
    if( !pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

Sequence<beans::PropertyValue> SvxXMLListLevelStyleContext_Impl::GetProperties(
        const SvI18NMap *pI18NMap )
{
    sal_Int16 eType;

    sal_Int32 nCount = 0;
    if( bBullet )
    {
        eType = NumberingType::CHAR_SPECIAL;
        nCount = cBullet ? 15 : 14;
    }
    if( bImage )
    {
        eType = NumberingType::BITMAP;
        nCount = 15;

        if( !sImageURL.isEmpty() || xBase64Stream.is() )
            nCount++;
    }
    if( bNum )
    {
        eType = NumberingType::ARABIC;
        GetImport().GetMM100UnitConverter().convertNumFormat(
                eType, sNumFormat, sNumLetterSync, sal_True );
        nCount = 15;
    }

    if( ( bBullet || bNum ) && nRelSize )
        nCount++;

    if( !bImage && bHasColor )
    {
        nCount++;
    }

    if (bBullet && !sSuffix.isEmpty())
    {
        sal_uInt16 const nVersion(GetImport().getGeneratorVersion());
        sal_Int32 nUPD;
        sal_Int32 nBuildId;
        if (GetImport().getBuildIds(nUPD, nBuildId)
            && (   (SvXMLImport::OOo_1x == nVersion)
                || (SvXMLImport::OOo_2x == nVersion)
                || (310 == nUPD) || (320 == nUPD) || (330 == nUPD)
                || ((300 == nUPD) && (nBuildId <= 9573))))
        {
            // #i93908# OOo < 3.4 wrote a bogus suffix for bullet chars
            sSuffix = ::rtl::OUString(); // clear it
        }
    }

    Sequence<beans::PropertyValue> aPropSeq( nCount );
    if( nCount > 0 )
    {
        beans::PropertyValue *pProps = aPropSeq.getArray();
        sal_Int32 nPos = 0;
        pProps[nPos].Name =
                OUString(XML_UNO_NAME_NRULE_NUMBERINGTYPE );
        pProps[nPos++].Value <<= (sal_Int16)eType ;

        pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_PREFIX );
        pProps[nPos++].Value <<= sPrefix;

        pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_SUFFIX );
        pProps[nPos++].Value <<= sSuffix;

        pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_ADJUST );
        pProps[nPos++].Value <<= eAdjust;

        sal_Int32 nLeftMargin = nSpaceBefore + nMinLabelWidth;
        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_LEFT_MARGIN );
        pProps[nPos++].Value <<= (sal_Int32)nLeftMargin;

        sal_Int32 nFirstLineOffset = -nMinLabelWidth;

        pProps[nPos].Name =
                OUString(XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET );
        pProps[nPos++].Value <<= (sal_Int32)nFirstLineOffset;

        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE);
        pProps[nPos++].Value <<= (sal_Int16)nMinLabelDist;

        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_POSITION_AND_SPACE_MODE);
        pProps[nPos++].Value <<= (sal_Int16)ePosAndSpaceMode;
        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_LABEL_FOLLOWED_BY);
        pProps[nPos++].Value <<= (sal_Int16)eLabelFollowedBy;
        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_LISTTAB_STOP_POSITION);
        pProps[nPos++].Value <<= (sal_Int32)nListtabStopPosition;
        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_FIRST_LINE_INDENT);
        pProps[nPos++].Value <<= (sal_Int32)nFirstLineIndent;
        pProps[nPos].Name =
            OUString(XML_UNO_NAME_NRULE_INDENT_AT);
        pProps[nPos++].Value <<= (sal_Int32)nIndentAt;

        OUString sDisplayTextStyleName = GetImport().GetStyleDisplayName(
                                XML_STYLE_FAMILY_TEXT_TEXT, sTextStyleName  );
        OUString sStyleName = sDisplayTextStyleName;
        if( !sStyleName.isEmpty() && pI18NMap )
            sStyleName = pI18NMap->Get( SFX_STYLE_FAMILY_CHAR, sStyleName );
        pProps[nPos].Name =
                OUString(XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        pProps[nPos++].Value <<= sDisplayTextStyleName;

        if( bBullet )
        {
            awt::FontDescriptor aFDesc;
            aFDesc.Name = sBulletFontName;
            if( !sBulletFontName.isEmpty() )
            {
                aFDesc.StyleName = sBulletFontStyleName;
                aFDesc.Family = eBulletFontFamily;
                aFDesc.Pitch = eBulletFontPitch;
                aFDesc.CharSet = eBulletFontEncoding;
                aFDesc.Weight = WEIGHT_DONTKNOW;
                sal_Bool bStarSymbol = sal_False;
                if( aFDesc.Name.equalsIgnoreAsciiCase( sStarBats ) )
                {
                    cBullet = GetImport().ConvStarBatsCharToStarSymbol( cBullet );
                    bStarSymbol = sal_True;
                }
                else if( aFDesc.Name.equalsIgnoreAsciiCase( sStarMath ) )
                {
                    cBullet = GetImport().ConvStarMathCharToStarSymbol( cBullet );
                    bStarSymbol = sal_True;
                }
                if( bStarSymbol )
                    aFDesc.Name = OUString( "StarSymbol"  );
            }

            if( cBullet )
            {
                OUStringBuffer sTmp(1);
                sTmp.append( cBullet );
                pProps[nPos].Name =
                        OUString(XML_UNO_NAME_NRULE_BULLET_CHAR );
                pProps[nPos++].Value <<= sTmp.makeStringAndClear();
            }

            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_BULLET_FONT );
            pProps[nPos++].Value <<= aFDesc;

        }

        if( bImage )
        {
            OUString sStr( sImageURL );
            if( !sImageURL.isEmpty() )
            {
                sStr = GetImport().ResolveGraphicObjectURL( sImageURL,
                                                                 sal_False );
            }
            else if( xBase64Stream.is() )
            {
                sStr = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
            }

            if( !sStr.isEmpty() )
            {
                pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_GRAPHICURL );
                pProps[nPos++].Value <<= sStr;
            }

            awt::Size aSize( nImageWidth, nImageHeight );
            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_GRAPHIC_SIZE );
            pProps[nPos++].Value <<= aSize;

            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_VERT_ORIENT );
            pProps[nPos++].Value <<= (sal_Int16)eImageVertOrient;
        }

        if( bNum )
        {
            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_START_WITH );
            pProps[nPos++].Value <<= (sal_Int16)nNumStartValue;

            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_PARENT_NUMBERING);
            pProps[nPos++].Value <<= (sal_Int16)nNumDisplayLevels;
        }

        if( ( bNum || bBullet ) && nRelSize )
        {
            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_BULLET_RELSIZE );
            pProps[nPos++].Value <<= nRelSize;
        }

        if( !bImage && bHasColor )
        {
            pProps[nPos].Name = OUString(XML_UNO_NAME_NRULE_BULLET_COLOR );
            pProps[nPos++].Value <<= m_nColor;
        }

        DBG_ASSERT( nPos == nCount, "array under/overflow" );
    }

    return aPropSeq;
}

// ---------------------------------------------------------------------

enum SvxXMLStyleAttributesAttrTokens
{
    XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_NAME,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_COLOR,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_WINDOW_FONT_COLOR,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_SIZE,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_POSITION_AND_SPACE_MODE,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_END=XML_TOK_UNKNOWN
};
static const SvXMLTokenMapEntry* lcl_getStyleAttributesAttrTokenMap()
{
    static SvXMLTokenMapEntry aStyleAttributesAttrTokenMap[] =
    {
        { XML_NAMESPACE_TEXT, XML_SPACE_BEFORE,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE },
        { XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH,
                  XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH },
        { XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST },
        { XML_NAMESPACE_FO, XML_TEXT_ALIGN,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN },
        { XML_NAMESPACE_STYLE, XML_FONT_NAME,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_NAME },
        { XML_NAMESPACE_FO, XML_FONT_FAMILY,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY },
        { XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC },
        { XML_NAMESPACE_STYLE, XML_FONT_STYLE_NAME,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME },
        { XML_NAMESPACE_STYLE, XML_FONT_PITCH,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH },
        { XML_NAMESPACE_STYLE, XML_FONT_CHARSET,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET },
        { XML_NAMESPACE_STYLE, XML_VERTICAL_POS,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS },
        { XML_NAMESPACE_STYLE, XML_VERTICAL_REL,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL },
        { XML_NAMESPACE_FO, XML_WIDTH,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH },
        { XML_NAMESPACE_FO, XML_HEIGHT,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT },
        { XML_NAMESPACE_FO, XML_COLOR,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_COLOR },
        { XML_NAMESPACE_STYLE, XML_USE_WINDOW_FONT_COLOR,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_WINDOW_FONT_COLOR },
        { XML_NAMESPACE_FO, XML_FONT_SIZE,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_SIZE },
        { XML_NAMESPACE_TEXT, XML_LIST_LEVEL_POSITION_AND_SPACE_MODE,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_POSITION_AND_SPACE_MODE },
        XML_TOKEN_MAP_END
    };
    return aStyleAttributesAttrTokenMap;
}
SvxXMLListLevelStyleAttrContext_Impl::SvxXMLListLevelStyleAttrContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rListLevel( rLLevel )
{
    SvXMLTokenMap aTokenMap( lcl_getStyleAttributesAttrTokenMap() );
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();

    OUString sFontName, sFontFamily, sFontStyleName, sFontFamilyGeneric,
             sFontPitch, sFontCharset;
    OUString sVerticalPos, sVerticalRel;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, SHRT_MIN, SHRT_MAX))
                rListLevel.SetSpaceBefore( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH:
            if (rUnitConv.convertMeasureToCore( nVal, rValue, 0, SHRT_MAX ))
                rListLevel.SetMinLabelWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST:
            if (rUnitConv.convertMeasureToCore( nVal, rValue, 0, USHRT_MAX ))
                rListLevel.SetMinLabelDist( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN:
            if( !rValue.isEmpty() )
            {
                sal_Int16 eAdjust = HoriOrientation::LEFT;
                if( IsXMLToken( rValue, XML_CENTER ) )
                    eAdjust = HoriOrientation::CENTER;
                else if( IsXMLToken( rValue, XML_END ) )
                    eAdjust = HoriOrientation::RIGHT;
                rListLevel.SetAdjust( eAdjust );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_NAME:
            sFontName = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY:
            sFontFamily = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC:
            sFontFamilyGeneric = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME:
            sFontStyleName = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH:
            sFontPitch = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET:
            sFontCharset = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS:
            sVerticalPos = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL:
            sVerticalRel = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, 0, SAL_MAX_INT32))
                rListLevel.SetImageWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, 0, SAL_MAX_INT32))
                rListLevel.SetImageHeight( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_COLOR:
            {
                sal_Int32 nColor(0);
                if (::sax::Converter::convertColor( nColor, rValue ))
                {
                    rListLevel.SetColor( nColor );
                }
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_WINDOW_FONT_COLOR:
            {
                if( IsXMLToken( rValue, XML_TRUE ) )
                    rListLevel.SetColor( (sal_Int32)0xffffffff );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_SIZE:
            if (::sax::Converter::convertPercent( nVal, rValue ))
                rListLevel.SetRelSize( (sal_Int16)nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_POSITION_AND_SPACE_MODE:
            {
                sal_Int16 ePosAndSpaceMode = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
                if( IsXMLToken( rValue, XML_LABEL_ALIGNMENT ) )
                    ePosAndSpaceMode = PositionAndSpaceMode::LABEL_ALIGNMENT;
                rListLevel.SetPosAndSpaceMode( ePosAndSpaceMode );
            }
            break;
        }
    }

    if( !sFontName.isEmpty() )
    {
        const XMLFontStylesContext *pFontDecls =
            GetImport().GetTextImport()->GetFontDecls();
        if( pFontDecls )
        {
            ::std::vector < XMLPropertyState > aProps;
            if( pFontDecls->FillProperties( sFontName, aProps, 0, 1, 2, 3, 4 ) )
            {
                OUString sTmp;
                sal_Int16 nTmp = 0;
                ::std::vector< XMLPropertyState >::iterator i;
                for( i = aProps.begin(); i != aProps.end(); ++i )
                {
                    switch( i->mnIndex )
                    {
                    case 0:
                        i->maValue >>= sTmp;
                        rListLevel.SetBulletFontName( sTmp);
                        break;
                    case 1:
                        i->maValue >>= sTmp;
                        rListLevel.SetBulletFontStyleName( sTmp );
                        break;
                    case 2:
                        i->maValue >>= nTmp;
                        rListLevel.SetBulletFontFamily( nTmp );
                        break;
                    case 3:
                        i->maValue >>= nTmp;
                        rListLevel.SetBulletFontPitch( nTmp );
                        break;
                    case 4:
                        i->maValue >>= nTmp;
                        rListLevel.SetBulletFontEncoding( nTmp );
                        break;
                    }
                }
            }
        }
    }
    if( !sFontFamily.isEmpty() )
    {
        String sEmpty;
        Any aAny;

        XMLFontFamilyNamePropHdl aFamilyNameHdl;
        if( aFamilyNameHdl.importXML( sFontFamily, aAny, rUnitConv ) )
        {
            OUString sTmp;
            aAny >>= sTmp;
            rListLevel.SetBulletFontName( sTmp);
        }

        XMLFontFamilyPropHdl aFamilyHdl;
        if( !sFontFamilyGeneric.isEmpty() &&
            aFamilyHdl.importXML( sFontFamilyGeneric, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp = 0;
            aAny >>= nTmp;
            rListLevel.SetBulletFontFamily( nTmp );
        }

        if( !sFontStyleName.isEmpty() )
            rListLevel.SetBulletFontStyleName( sFontStyleName );

        XMLFontPitchPropHdl aPitchHdl;
        if( !sFontPitch.isEmpty() &&
            aPitchHdl.importXML( sFontPitch, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp = 0;
            aAny >>= nTmp;
            rListLevel.SetBulletFontPitch( nTmp );
        }

        XMLFontEncodingPropHdl aEncHdl;
        if( !sFontCharset.isEmpty() &&
            aEncHdl.importXML( sFontCharset, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp = 0;
            aAny >>= nTmp;
            rListLevel.SetBulletFontEncoding( nTmp );
        }
    }

    sal_Int16 eVertOrient = VertOrientation::LINE_CENTER;
    if( !sVerticalPos.isEmpty() )
    {
        if( IsXMLToken( sVerticalPos, XML_TOP ) )
            eVertOrient = VertOrientation::LINE_TOP;
        else if( IsXMLToken( sVerticalPos, XML_BOTTOM ) )
            eVertOrient = VertOrientation::LINE_BOTTOM;
    }
    if( !sVerticalRel.isEmpty() )
    {
        if( IsXMLToken( sVerticalRel, XML_BASELINE ) )
        {
            // TOP and BOTTOM are exchanged for a baseline relation
            switch( eVertOrient  )
            {
            case VertOrientation::LINE_TOP:
                eVertOrient = VertOrientation::BOTTOM;
                break;
            case VertOrientation::LINE_CENTER:
                eVertOrient = VertOrientation::CENTER;
                break;
            case VertOrientation::LINE_BOTTOM:
                eVertOrient = VertOrientation::TOP;
                break;
            }
        }
        else if( IsXMLToken( sVerticalRel, XML_CHAR ) )
        {
            switch( eVertOrient  )
            {
            case VertOrientation::LINE_TOP:
                eVertOrient = VertOrientation::CHAR_TOP;
                break;
            case VertOrientation::LINE_CENTER:
                eVertOrient = VertOrientation::CHAR_CENTER;
                break;
            case VertOrientation::LINE_BOTTOM:
                eVertOrient = VertOrientation::CHAR_BOTTOM;
                break;
            }
        }
    }
    rListLevel.SetImageVertOrient( eVertOrient );
}

SvxXMLListLevelStyleAttrContext_Impl::~SvxXMLListLevelStyleAttrContext_Impl()
{
}

SvXMLImportContext* SvxXMLListLevelStyleAttrContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if ( XML_NAMESPACE_STYLE == nPrefix &&
         IsXMLToken( rLocalName, XML_LIST_LEVEL_LABEL_ALIGNMENT ) )
    {
        pContext = new SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl( GetImport(),
                                                             nPrefix,
                                                             rLocalName,
                                                             xAttrList,
                                                             rListLevel );
    }
    if( !pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// ---------------------------------------------------------------------

enum SvxXMLStyleAttributesLabelAlignmentAttrTokens
{
    XML_TOK_STYLE_ATTRIBUTES_ATTR_LABEL_FOLLOWED_BY,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_LISTTAB_STOP_POSITION,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FIRST_LINE_INDENT,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_INDENT_AT,

    XML_TOK_STYLE_ATTRIBUTES_LABEL_ALIGNMENT_ATTR_END=XML_TOK_UNKNOWN
};
static const SvXMLTokenMapEntry* lcl_getStyleAlignmentAttributesAttrTokenMap()
{
    static SvXMLTokenMapEntry aStyleAlignmentAttributesAttrTokenMap[] =
    {
        { XML_NAMESPACE_TEXT, XML_LABEL_FOLLOWED_BY,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_LABEL_FOLLOWED_BY },
        { XML_NAMESPACE_TEXT, XML_LIST_TAB_STOP_POSITION,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_LISTTAB_STOP_POSITION },
        { XML_NAMESPACE_FO, XML_TEXT_INDENT,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_FIRST_LINE_INDENT },
        { XML_NAMESPACE_FO, XML_MARGIN_LEFT,
                XML_TOK_STYLE_ATTRIBUTES_ATTR_INDENT_AT },

        XML_TOKEN_MAP_END
    };
    return aStyleAlignmentAttributesAttrTokenMap;
}
SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl::SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rListLevel( rLLevel )
{
    SvXMLTokenMap aTokenMap( lcl_getStyleAlignmentAttributesAttrTokenMap() );
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_LABEL_FOLLOWED_BY:
            {
                sal_Int16 eLabelFollowedBy = LabelFollow::LISTTAB;
                if( IsXMLToken( rValue, XML_SPACE ) )
                    eLabelFollowedBy = LabelFollow::SPACE;
                else if( IsXMLToken( rValue, XML_NOTHING ) )
                    eLabelFollowedBy = LabelFollow::NOTHING;
                rListLevel.SetLabelFollowedBy( eLabelFollowedBy );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_LISTTAB_STOP_POSITION:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, 0, SHRT_MAX))
                rListLevel.SetListtabStopPosition( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FIRST_LINE_INDENT:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, SHRT_MIN, SHRT_MAX))
                rListLevel.SetFirstLineIndent( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_INDENT_AT:
            if (rUnitConv.convertMeasureToCore(nVal, rValue, SHRT_MIN, SHRT_MAX))
                rListLevel.SetIndentAt( nVal );
            break;
        }
    }
}

SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl::~SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl()
{
}

// ---------------------------------------------------------------------

class SvxXMLListStyle_Impl : public std::vector<SvxXMLListLevelStyleContext_Impl *> {};

void SvxXMLListStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                           const OUString& rLocalName,
                                           const OUString& rValue )
{
    if( XML_NAMESPACE_TEXT == nPrefixKey &&
        IsXMLToken( rLocalName, XML_CONSECUTIVE_NUMBERING ) )
    {
        bConsecutive = IsXMLToken( rValue, XML_TRUE );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


SvxXMLListStyleContext::SvxXMLListStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bOutl )
:   SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, bOutl ? XML_STYLE_FAMILY_TEXT_OUTLINE : XML_STYLE_FAMILY_TEXT_LIST )
,   sIsPhysical( "IsPhysical"  )
,   sNumberingRules( "NumberingRules"  )
,   sName( "Name"  )
,   sIsContinuousNumbering( "IsContinuousNumbering"  )
,   pLevelStyles( 0 )
,   nLevels( 0 )
,   bConsecutive( sal_False )
,   bOutline( bOutl )
{
}

SvxXMLListStyleContext::~SvxXMLListStyleContext()
{
    if( pLevelStyles )
    {
        while( !pLevelStyles->empty() )
        {
            SvxXMLListLevelStyleContext_Impl *pStyle = pLevelStyles->back();
            pLevelStyles->pop_back();
            pStyle->ReleaseRef();
        }
    }

    delete pLevelStyles;
}

TYPEINIT1( SvxXMLListStyleContext, SvXMLStyleContext );

SvXMLImportContext *SvxXMLListStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_TEXT == nPrefix &&
        ( bOutline
              ? IsXMLToken( rLocalName, XML_OUTLINE_LEVEL_STYLE )
              : ( IsXMLToken( rLocalName, XML_LIST_LEVEL_STYLE_NUMBER ) ||
                IsXMLToken( rLocalName, XML_LIST_LEVEL_STYLE_BULLET ) ||
                 IsXMLToken( rLocalName, XML_LIST_LEVEL_STYLE_IMAGE )    ) ) )
    {
        SvxXMLListLevelStyleContext_Impl *pLevelStyle =
            new SvxXMLListLevelStyleContext_Impl( GetImport(), nPrefix,
                                                  rLocalName, xAttrList );
        if( !pLevelStyles )
            pLevelStyles = new SvxXMLListStyle_Impl;
        pLevelStyles->push_back( pLevelStyle );
        pLevelStyle->AddRef();

        pContext = pLevelStyle;
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SvxXMLListStyleContext::FillUnoNumRule(
        const Reference< container::XIndexReplace > & rNumRule,
        const SvI18NMap *pI18NMap ) const
{
    try
    {
        if( pLevelStyles && rNumRule.is() )
        {
            sal_uInt16 nCount = pLevelStyles->size();
            sal_Int32 l_nLevels = rNumRule->getCount();
            for( sal_uInt16 i=0; i < nCount; i++ )
            {
                SvxXMLListLevelStyleContext_Impl *pLevelStyle =
                    (*pLevelStyles)[i];
                sal_Int32 nLevel = pLevelStyle->GetLevel();
                if( nLevel >= 0 && nLevel < l_nLevels )
                {
                    Sequence<beans::PropertyValue> aProps =
                        pLevelStyle->GetProperties( pI18NMap );
                    Any aAny;
                    aAny <<= aProps;
                    rNumRule->replaceByIndex( nLevel, aAny );
                }
            }
        }

        Reference < XPropertySet > xPropSet( rNumRule, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo;
        if (xPropSet.is())
            xPropSetInfo = xPropSet->getPropertySetInfo();
        if( xPropSetInfo.is() &&
            xPropSetInfo->hasPropertyByName( sIsContinuousNumbering ) )
        {
            Any aAny;
            sal_Bool bTmp = bConsecutive;
            aAny.setValue( &bTmp, ::getBooleanCppuType() );
            xPropSet->setPropertyValue( sIsContinuousNumbering, aAny );
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "SvxXMLListStyleContext::FillUnoNumRule - Exception caught" );
    }
}

void SvxXMLListStyleContext::CreateAndInsertLate( sal_Bool bOverwrite )
{
    if( bOutline )
    {
        if( bOverwrite )
        {
            const Reference< XIndexReplace >& rNumRule =
                GetImport().GetTextImport()->GetChapterNumbering();
            // We don't set xNumberingRules here, to avoid using them
            // as numbering rules.
            if( rNumRule.is() )
                FillUnoNumRule( rNumRule, 0 );
        }
    }
    else
    {
        Reference < XStyle > xStyle;
        const OUString& rName = GetDisplayName();
        if( rName.isEmpty() )
        {
            SetValid( sal_False );
            return;
        }

        const Reference < XNameContainer >& rNumStyles =
                GetImport().GetTextImport()->GetNumberingStyles();
        if( !rNumStyles.is() )
        {
            SetValid( sal_False );
            return;
        }

        sal_Bool bNew = sal_False;
        if( rNumStyles->hasByName( rName ) )
        {
            Any aAny = rNumStyles->getByName( rName );
            aAny >>= xStyle;
        }
        else
        {
            Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                        UNO_QUERY );
            DBG_ASSERT( xFactory.is(), "no factory" );
            if( !xFactory.is() )
                return;

            Reference < XInterface > xIfc = xFactory->createInstance(OUString("com.sun.star.style.NumberingStyle") );
            if( !xIfc.is() )
                return;
            Reference < XStyle > xTmp( xIfc, UNO_QUERY );
            xStyle = xTmp;
            if( !xStyle.is() )
                return;

            Any aAny;
            aAny <<= xStyle;
            rNumStyles->insertByName( rName, aAny );
            bNew = sal_True;
        }

        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
        {
            Any aAny = xPropSet->getPropertyValue( sIsPhysical );
            bNew = !*(sal_Bool *)aAny.getValue();
        }
        if( rName != GetName() )
            GetImport().AddStyleDisplayName( XML_STYLE_FAMILY_TEXT_LIST,
                                             GetName(), rName );

        Any aAny = xPropSet->getPropertyValue( sNumberingRules );
        aAny >>= xNumRules;
        nLevels = xNumRules->getCount();
        if( bOverwrite || bNew )
        {
            FillUnoNumRule( xNumRules, 0 );
            aAny <<= xNumRules;
            xPropSet->setPropertyValue( sNumberingRules, aAny );
        }
        else
        {
            SetValid( sal_False );
        }

        SetNew( bNew );
    }
}

void SvxXMLListStyleContext::CreateAndInsertAuto() const
{
    DBG_ASSERT( !bOutline, "Outlines cannot be inserted here" );
    DBG_ASSERT( !xNumRules.is(), "Numbering Rule is existing already" );

    const OUString& rName = GetName();
    if( bOutline || xNumRules.is() || rName.isEmpty() )
    {
        ((SvxXMLListStyleContext *)this)->SetValid( sal_False );
        return;
    }

    ((SvxXMLListStyleContext *)this)->xNumRules = CreateNumRule(
        GetImport().GetModel() );
    ((SvxXMLListStyleContext *)this)->nLevels = xNumRules->getCount();

    FillUnoNumRule( xNumRules, 0 );
}

Reference < XIndexReplace > SvxXMLListStyleContext::CreateNumRule(
                                const Reference < XModel > & rModel )
{
    Reference<XIndexReplace> xNumRule;

    Reference< XMultiServiceFactory > xFactory( rModel, UNO_QUERY );
    DBG_ASSERT( xFactory.is(), "no factory" );
    if( !xFactory.is() )
        return xNumRule;

    Reference < XInterface > xIfc = xFactory->createInstance(OUString("com.sun.star.text.NumberingRules") );
    if( !xIfc.is() )
        return xNumRule;

    xNumRule = Reference<XIndexReplace>( xIfc, UNO_QUERY );
    DBG_ASSERT( xNumRule.is(), "go no numbering rule" );

    return xNumRule;
}

void SvxXMLListStyleContext::SetDefaultStyle(
        const Reference < XIndexReplace > & rNumRule,
        sal_Int16 nLevel,
        sal_Bool bOrdered )
{
    Sequence<beans::PropertyValue> aPropSeq( bOrdered ? 1 : 4  );
    beans::PropertyValue *pProps = aPropSeq.getArray();

    pProps->Name =
                OUString(XML_UNO_NAME_NRULE_NUMBERINGTYPE );
    (pProps++)->Value <<= (sal_Int16)(bOrdered ? NumberingType::ARABIC
                                                 : NumberingType::CHAR_SPECIAL );
    if( !bOrdered )
    {
        // TODO: Bullet-Font
        awt::FontDescriptor aFDesc;
        aFDesc.Name = OUString(
#if defined UNX
                        "starbats"
#else
                        "StarBats"
#endif
                                        );
        aFDesc.Family = FAMILY_DONTKNOW ;
        aFDesc.Pitch = PITCH_DONTKNOW ;
        aFDesc.CharSet = RTL_TEXTENCODING_SYMBOL ;
        aFDesc.Weight = WEIGHT_DONTKNOW;
        pProps->Name =
                    OUString(XML_UNO_NAME_NRULE_BULLET_FONT );
        (pProps++)->Value <<= aFDesc;

        OUStringBuffer sTmp(1);
        sTmp.append( (sal_Unicode)(0xF000 + 149) );
        pProps->Name =
                    OUString(XML_UNO_NAME_NRULE_BULLET_CHAR );
        (pProps++)->Value <<= sTmp.makeStringAndClear();
        pProps->Name =
                OUString(XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        (pProps++)->Value <<= OUString( "Numbering Symbols"  );
    }

    Any aAny;
    aAny <<= aPropSeq;
    rNumRule->replaceByIndex( nLevel, aAny );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
