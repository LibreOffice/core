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

#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <vcl/vclenum.hxx>

#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmlimp.hxx"
#include "XMLBase64ImportContext.hxx"

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/style/XStyle.hpp>


#include "i18nmap.hxx"
#include "xmluconv.hxx"
#include "fonthdl.hxx"
#include "XMLFontStylesContext.hxx"
#include "maptype.hxx"


#include "xmlnumi.hxx"

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::binfilter::xmloff::token;
using namespace ::com::sun::star::io;

using rtl::OUString;
using rtl::OUStringBuffer;

static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE[] =
        "SymbolTextDistance";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PARENT_NUMBERING[] =
        "ParentNumbering";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_CHAR_STYLE_NAME[] =
        "CharStyleName";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_CHAR[] ="BulletChar";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_RELSIZE[] = "BulletRelSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_SIZE[] =
        "GraphicSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_VERT_ORIENT[] ="VertOrient";

static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_NUMBERINGTYPE[] = "NumberingType";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PREFIX[] = "Prefix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SUFFIX[] = "Suffix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_ADJUST[] = "Adjust";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_LEFT_MARGIN[] = "LeftMargin";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET[] =
    "FirstLineOffset";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_FONT[] = "BulletFont";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHICURL[] = "GraphicURL";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_START_WITH[] = "StartWith";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_COLOR[] = "BulletColor";

// ---------------------------------------------------------------------

class SvxXMLListLevelStyleContext_Impl;

class SvxXMLListLevelStyleAttrContext_Impl : public SvXMLImportContext
{
    SvxXMLListLevelStyleContext_Impl&	rListLevel;

public:

    SvxXMLListLevelStyleAttrContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
             const OUString& rLName,
              const Reference< xml::sax::XAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel	);
    virtual ~SvxXMLListLevelStyleAttrContext_Impl();
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

static __FAR_DATA SvXMLTokenMapEntry aLevelAttrTokenMap[] =
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

class SvxXMLListLevelStyleContext_Impl : public SvXMLImportContext
{
    friend class SvxXMLListLevelStyleAttrContext_Impl;
#ifdef CONV_STAR_FONTS
    const OUString 		sStarBats;
    const OUString 		sStarMath;
#endif

    OUString 			sPrefix;
    OUString			sSuffix;
    OUString			sTextStyleName;
    OUString			sNumFormat;
    OUString			sNumLetterSync;
    OUString			sBulletFontName;
    OUString			sBulletFontStyleName;
    OUString			sImageURL;

    Reference < XOutputStream > xBase64Stream;

    sal_Int32			nLevel;
    sal_Int32			nSpaceBefore;
    sal_Int32			nMinLabelWidth;
    sal_Int32			nMinLabelDist;
    sal_Int32			nImageWidth;
    sal_Int32			nImageHeight;
    sal_Int16			nNumStartValue;
    sal_Int16			nNumDisplayLevels;

    sal_Int16 			eAdjust;
    sal_Int16			eBulletFontFamily;
    sal_Int16			eBulletFontPitch;
    rtl_TextEncoding	eBulletFontEncoding;
    sal_Int16			eImageVertOrient;

    sal_Unicode			cBullet;

    sal_Int16			nRelSize;
    Color				aColor;

    sal_Bool			bBullet : 1;
    sal_Bool			bImage : 1;
    sal_Bool			bNum : 1;
    sal_Bool			bHasColor : 1;

    void SetRelSize( sal_Int16 nRel ) { nRelSize = nRel; }
    void SetColor( sal_Int32 _aColor ) { aColor = _aColor; bHasColor = sal_True; }
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
};

SvxXMLListLevelStyleContext_Impl::SvxXMLListLevelStyleContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :

    SvXMLImportContext( rImport, nPrfx, rLName ),
    sNumFormat( OUString::createFromAscii( "1" ) ),
    nLevel( -1L ),
    nSpaceBefore( 0L ),
    nMinLabelWidth( 0L ),
    nMinLabelDist( 0L ),
    nImageWidth( 0L ),
    nImageHeight( 0L ),
    nNumStartValue( 1 ),
    nNumDisplayLevels( 1 ),
    eAdjust( HoriOrientation::LEFT ),
    eBulletFontFamily( FAMILY_DONTKNOW ),
    eBulletFontPitch( PITCH_DONTKNOW ),
    eBulletFontEncoding( RTL_TEXTENCODING_DONTKNOW ),
    cBullet( 0 ),
    nRelSize(0),
    bNum( sal_False ),
    bBullet( sal_False ),
    bImage( sal_False ),
    bHasColor( sal_False ),
    aColor( 0 )
#ifdef CONV_STAR_FONTS
    ,sStarBats( RTL_CONSTASCII_USTRINGPARAM( "StarBats" ) ),
    sStarMath( RTL_CONSTASCII_USTRINGPARAM( "StarMath" ) )
#endif
{
    if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_NUMBER ) ||
        IsXMLToken( rLName, XML_OUTLINE_LEVEL_STYLE )        )
        bNum = sal_True;
    else if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_BULLET ) )
        bBullet = sal_True;
    else if( IsXMLToken( rLName, XML_LIST_LEVEL_STYLE_IMAGE ) )
        bImage = sal_True;

    SvXMLTokenMap aTokenMap( aLevelAttrTokenMap );
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
                nLevel = 0L;
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
        IsXMLToken( rLocalName, XML_PROPERTIES ) )
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
        if( bImage && !sImageURL.getLength() && !xBase64Stream.is() )
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
    sal_Int16 eType = 0;

    sal_Int32 nCount = 0L;
    if( bBullet )
    {
        eType = NumberingType::CHAR_SPECIAL;
        nCount = cBullet ? 10 : 9;
    }
    if( bImage )
    {
        eType = NumberingType::BITMAP;
        nCount = (sal_Int32) 10L;

        if( (sImageURL.getLength() > 0L) || xBase64Stream.is() )
            nCount++;
    }
    if( bNum )
    {
        eType = NumberingType::ARABIC;
        GetImport().GetMM100UnitConverter().convertNumFormat(
                eType, sNumFormat, sNumLetterSync, sal_True );
        nCount = (sal_Int32) 10L;
    }

    if( ( bBullet || bNum ) && nRelSize )
        nCount++;

    if( !bImage && bHasColor )
    {
        nCount++;
    }

    Sequence<beans::PropertyValue> aPropSeq( nCount );
    if( nCount > 0 )
    {
        beans::PropertyValue *pProps = aPropSeq.getArray();
        sal_Int32 nPos = 0L;
        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_NUMBERINGTYPE );
        pProps[nPos++].Value <<= (sal_Int16)eType ;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_PREFIX );
        pProps[nPos++].Value <<= sPrefix;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_SUFFIX );
        pProps[nPos++].Value <<= sSuffix;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_ADJUST );
        pProps[nPos++].Value <<= eAdjust;

        sal_Int32 nLeftMargin = nSpaceBefore + nMinLabelWidth;
        if( nLeftMargin < 0 )
            nLeftMargin = 0;
        pProps[nPos].Name =
            OUString::createFromAscii( XML_UNO_NAME_NRULE_LEFT_MARGIN );
        pProps[nPos++].Value <<= (sal_Int32)nLeftMargin;

        sal_Int32 nFirstLineOffset =
                nMinLabelWidth <= nLeftMargin ? -nMinLabelWidth : -nLeftMargin;

        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET );
        pProps[nPos++].Value <<= (sal_Int32)nFirstLineOffset;

        pProps[nPos].Name =
            OUString::createFromAscii(XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE);
        pProps[nPos++].Value <<= (sal_Int16)nMinLabelDist;

        OUString sStyleName = sTextStyleName;
        if( sStyleName.getLength() && pI18NMap )
            sStyleName = pI18NMap->Get( SFX_STYLE_FAMILY_CHAR, sStyleName );
        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        pProps[nPos++].Value <<= sTextStyleName;

        if( bBullet )
        {
            awt::FontDescriptor aFDesc;
            aFDesc.Name = sBulletFontName;
            if( sBulletFontName.getLength() )
            {
                aFDesc.StyleName = sBulletFontStyleName;
                aFDesc.Family = eBulletFontFamily;
                aFDesc.Pitch = eBulletFontPitch;
                aFDesc.CharSet = eBulletFontEncoding;
                aFDesc.Weight = WEIGHT_DONTKNOW;
                //aFDesc.Transparant = sal_True;
#ifdef CONV_STAR_FONTS
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
                    aFDesc.Name =
                        OUString( RTL_CONSTASCII_USTRINGPARAM("StarSymbol" ) );
#endif
            }

            if( cBullet )
            {
                OUStringBuffer sTmp(1);
                sTmp.append( cBullet );
                pProps[nPos].Name =
                        OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_CHAR );
                pProps[nPos++].Value <<= sTmp.makeStringAndClear();
            }

            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_FONT );
            pProps[nPos++].Value <<= aFDesc;

        }

        if( bImage )
        {
            OUString sStr( sImageURL );
            if( sImageURL.getLength() )
            {
                sStr = GetImport().ResolveGraphicObjectURL( sImageURL,
                                                                 sal_False );
            }
            else if( xBase64Stream.is() )
            {
                sStr = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
//				xBase64Stream = 0;
            }

            if( sStr.getLength() )
            {
                pProps[nPos].Name =
                        OUString::createFromAscii( XML_UNO_NAME_NRULE_GRAPHICURL );
                pProps[nPos++].Value <<= sStr;
            }

            awt::Size aSize( nImageWidth, nImageHeight );
            pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_GRAPHIC_SIZE );
            pProps[nPos++].Value <<= aSize;

            pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_VERT_ORIENT );
            pProps[nPos++].Value <<= (sal_Int16)eImageVertOrient;
        }

        if( bNum )
        {
            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_START_WITH );
            pProps[nPos++].Value <<= (sal_Int16)nNumStartValue;

            pProps[nPos].Name =
                OUString::createFromAscii(XML_UNO_NAME_NRULE_PARENT_NUMBERING);
            pProps[nPos++].Value <<= (sal_Int16)nNumDisplayLevels;
        }

        if( ( bNum || bBullet ) && nRelSize )
        {
            pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_RELSIZE );
            pProps[nPos++].Value <<= nRelSize;
        }

        if( !bImage && bHasColor )
        {
            pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_COLOR );
            pProps[nPos++].Value <<= (sal_Int32)aColor.GetColor();
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

    XML_TOK_STYLE_ATTRIBUTES_ATTR_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aStyleAttributesAttrTokenMap[] =
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

    XML_TOKEN_MAP_END
};
SvxXMLListLevelStyleAttrContext_Impl::SvxXMLListLevelStyleAttrContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rListLevel( rLLevel )
{
    SvXMLTokenMap aTokenMap( aStyleAttributesAttrTokenMap );
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
            if( rUnitConv.convertMeasure( nVal, rValue, 0, USHRT_MAX ) )
                rListLevel.SetSpaceBefore( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH:
            if( rUnitConv.convertMeasure( nVal, rValue, 0, SHRT_MAX ) )
                rListLevel.SetMinLabelWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST:
            if( rUnitConv.convertMeasure( nVal, rValue, 0, USHRT_MAX ) )
                rListLevel.SetMinLabelDist( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN:
            if( rValue.getLength() )
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
            if( rUnitConv.convertMeasure( nVal, rValue, 0, SAL_MAX_INT32 ) )
                rListLevel.SetImageWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT:
            if( rUnitConv.convertMeasure( nVal, rValue, 0, SAL_MAX_INT32 ) )
                rListLevel.SetImageHeight( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_COLOR:
            {
                Color aColor;
                if( SvXMLUnitConverter::convertColor( aColor, rValue ) )
                    rListLevel.SetColor( (sal_Int32)aColor.GetColor() );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_WINDOW_FONT_COLOR:
            {
                if( IsXMLToken( rValue, XML_TRUE ) )
                    rListLevel.SetColor( (sal_Int32)0xffffffff );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_SIZE:
            if(SvXMLUnitConverter::convertPercent( nVal, rValue ) )
                rListLevel.SetRelSize( (sal_Int16)nVal );
            break;
        }
    }

    if( sFontName.getLength() )
    {
        const XMLFontStylesContext *pFontDecls =
            GetImport().GetTextImport()->GetFontDecls();
        if( pFontDecls )
        {
            ::std::vector < XMLPropertyState > aProps;
            if( pFontDecls->FillProperties( sFontName, aProps, 0, 1, 2, 3, 4 ) )
            {
                OUString sTmp;
                sal_Int16 nTmp;
                ::std::vector< XMLPropertyState >::iterator i;
                for( i = aProps.begin(); i != aProps.end(); i++ )
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
    if( sFontFamily.getLength() )
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
        if( sFontFamilyGeneric.getLength() &&
            aFamilyHdl.importXML( sFontFamilyGeneric, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontFamily( nTmp );
        }

        if( sFontStyleName.getLength() )
            rListLevel.SetBulletFontStyleName( sFontStyleName );

        XMLFontPitchPropHdl aPitchHdl;
        if( sFontPitch.getLength() &&
            aPitchHdl.importXML( sFontPitch, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontPitch( nTmp );
        }

        XMLFontEncodingPropHdl aEncHdl;
        if( sFontCharset.getLength() &&
            aEncHdl.importXML( sFontCharset, aAny, rUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontEncoding( nTmp );
        }
    }

    sal_Int16 eVertOrient = VertOrientation::LINE_CENTER;
    if( sVerticalPos.getLength() )
    {
        if( IsXMLToken( sVerticalPos, XML_TOP ) )
            eVertOrient = VertOrientation::LINE_TOP;
        else if( IsXMLToken( sVerticalPos, XML_BOTTOM ) )
            eVertOrient = VertOrientation::LINE_BOTTOM;
    }
    if( sVerticalRel.getLength() )
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

// ---------------------------------------------------------------------

typedef SvxXMLListLevelStyleContext_Impl *SvxXMLListLevelStyleContext_ImplPtr;
SV_DECL_PTRARR( SvxXMLListStyle_Impl, SvxXMLListLevelStyleContext_ImplPtr,
                10, 5 )

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
        sal_Bool bOutl ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, bOutl ? XML_STYLE_FAMILY_TEXT_OUTLINE : XML_STYLE_FAMILY_TEXT_LIST ),
    pLevelStyles( 0 ),
    nLevels( 0 ),
    bConsecutive( sal_False ),
    bOutline( bOutl ),
//	bUsed( sal_False ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sNumberingRules( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ),
    sName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ),
    sIsContinuousNumbering( RTL_CONSTASCII_USTRINGPARAM( "IsContinuousNumbering" ) )
{
}

SvxXMLListStyleContext::~SvxXMLListStyleContext()
{
    if( pLevelStyles )
    {
        while( pLevelStyles->Count() )
        {
            sal_uInt16 n = pLevelStyles->Count() - 1;
            SvxXMLListLevelStyleContext_Impl *pStyle = (*pLevelStyles)[n];
            pLevelStyles->Remove( n, 1 );
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
        pLevelStyles->Insert( pLevelStyle, pLevelStyles->Count() );
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
    if( pLevelStyles )
    {
        sal_uInt16 nCount = pLevelStyles->Count();
        sal_Int32 nLevels = rNumRule->getCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            SvxXMLListLevelStyleContext_Impl *pLevelStyle =
                (*pLevelStyles)[i];
            sal_Int32 nLevel = pLevelStyle->GetLevel();
            if( nLevel >= 0 && nLevel < nLevels )
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
        const OUString& rName = GetName();
        if( 0 == rName.getLength() )
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

            Reference < XInterface > xIfc = xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.NumberingStyle")) );
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
    if( bOutline || xNumRules.is() || 0 == rName.getLength() )
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

    Reference < XInterface > xIfc = xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.NumberingRules")) );
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
                OUString::createFromAscii( XML_UNO_NAME_NRULE_NUMBERINGTYPE );
    (pProps++)->Value <<= (sal_Int16)(bOrdered ? NumberingType::ARABIC
                                                 : NumberingType::CHAR_SPECIAL );
    if( !bOrdered )
    {
        // TODO: Bullet-Font
        //aNumFmt.SetBulletFont( &SwNumRule::GetDefBulletFont() );
        awt::FontDescriptor aFDesc;
        aFDesc.Name = OUString( 
#if defined UNX
                        RTL_CONSTASCII_USTRINGPARAM( "starbats" )
#else
                        RTL_CONSTASCII_USTRINGPARAM( "StarBats" )
#endif
                                        );
        aFDesc.Family = FAMILY_DONTKNOW ;
        aFDesc.Pitch = PITCH_DONTKNOW ;
        aFDesc.CharSet = RTL_TEXTENCODING_SYMBOL ;
        aFDesc.Weight = WEIGHT_DONTKNOW;
        //aFDesc.Transparant = sal_True;
        pProps->Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_FONT );
        (pProps++)->Value <<= aFDesc;

        OUStringBuffer sTmp(1);
        sTmp.append( (sal_Unicode)(0xF000 + 149) );
        pProps->Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_CHAR );
        (pProps++)->Value <<= sTmp.makeStringAndClear();
        pProps->Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        (pProps++)->Value <<=
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Numbering Symbols" ) );
    }

    Any aAny;
    aAny <<= aPropSeq;
    rNumRule->replaceByIndex( nLevel, aAny );
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
