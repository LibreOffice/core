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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

#include <o3tl/any.hxx>
#include <o3tl/temporary.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <tools/fontenum.hxx>
#include <tools/color.hxx>

#include <sax/tools/converter.hxx>

#include <vcl/vclenum.hxx>

#include <xmloff/xmltkmap.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/families.hxx>
#include <xmloff/maptype.hxx>

#include <xmloff/xmlnumi.hxx>

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

class SvxXMLListLevelStyleContext_Impl;

namespace {

class SvxXMLListLevelStyleAttrContext_Impl : public SvXMLImportContext
{
    SvxXMLListLevelStyleContext_Impl&   rListLevel;

public:

    SvxXMLListLevelStyleAttrContext_Impl(
            SvXMLImport& rImport, sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel   );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl : public SvXMLImportContext
{
public:

    SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl(
            SvXMLImport& rImport, sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel   );
};

}

class SvxXMLListLevelStyleContext_Impl : public SvXMLImportContext
{
    friend SvxXMLListLevelStyleAttrContext_Impl;

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

    sal_UCS4            cBullet;

    sal_Int16           nRelSize;
    Color               m_nColor;

    sal_Int16           ePosAndSpaceMode;
    sal_Int16           eLabelFollowedBy;
    sal_Int32           nListtabStopPosition;
    sal_Int32           nFirstLineIndent;
    sal_Int32           nIndentAt;

    bool            bBullet : 1;
    bool            bImage : 1;
    bool            bNum : 1;
    bool            bHasColor : 1;

    void SetRelSize( sal_Int16 nRel ) { nRelSize = nRel; }
    void SetColor( Color nColor )
        { m_nColor = nColor; bHasColor = true; }
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
            SvXMLImport& rImport, sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList > & xAttrList );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    sal_Int32 GetLevel() const { return nLevel; }
    Sequence<beans::PropertyValue> GetProperties();

    void SetPosAndSpaceMode( sal_Int16 eValue )
    {
        ePosAndSpaceMode = eValue;
    }
    void SetLabelFollowedBy( sal_Int16 eValue )
    {
        eLabelFollowedBy = eValue;
    }
    void SetListtabStopPosition( sal_Int32 nValue )
    {
        nListtabStopPosition = nValue;
    }
    void SetFirstLineIndent( sal_Int32 nValue )
    {
        nFirstLineIndent = nValue;
    }
    void SetIndentAt( sal_Int32 nValue )
    {
        nIndentAt = nValue;
    }
};

constexpr OUStringLiteral gsStarBats( u"StarBats"  );
constexpr OUStringLiteral gsStarMath( u"StarMath"  );

SvxXMLListLevelStyleContext_Impl::SvxXMLListLevelStyleContext_Impl(
        SvXMLImport& rImport, sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList )

:   SvXMLImportContext( rImport )
,   sNumFormat( "1" )
,   nLevel( -1 )
,   nSpaceBefore( 0 )
,   nMinLabelWidth( 0 )
,   nMinLabelDist( 0 )
,   nImageWidth( 0 )
,   nImageHeight( 0 )
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
,   bBullet( false )
,   bImage( false )
,   bNum( false )
,   bHasColor( false )
{
    switch (nElement & TOKEN_MASK)
    {
        case XML_LIST_LEVEL_STYLE_NUMBER:
        case XML_OUTLINE_LEVEL_STYLE:
            bNum = true;
            break;
        case XML_LIST_LEVEL_STYLE_BULLET:
            bBullet = true;
            break;
        case XML_LIST_LEVEL_STYLE_IMAGE:
            bImage = true;
            break;
    }

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(TEXT, XML_LEVEL):
            nLevel = aIter.toInt32();
            if( nLevel >= 1 )
                nLevel--;
            else
                nLevel = 0;
            break;
        case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            sTextStyleName = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_BULLET_CHAR):
            if (!aIter.isEmpty())
            {
                cBullet = aIter.toString().iterateCodePoints(&o3tl::temporary(sal_Int32(0)));
            }
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            if( bImage )
                sImageURL = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_TYPE):
        case XML_ELEMENT(XLINK, XML_SHOW):
        case XML_ELEMENT(XLINK, XML_ACTUATE):
            // This properties will be ignored
            break;
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            if( bNum )
                sNumFormat = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_NUM_PREFIX):
            sPrefix = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_NUM_SUFFIX):
            sSuffix = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            if( bNum )
                sNumLetterSync = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_START_VALUE):
            if( bNum )
            {
                sal_Int32 nTmp = aIter.toInt32();
                nNumStartValue =
                    (nTmp < 0) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : static_cast<sal_Int16>(nTmp) );
            }
            break;
        case XML_ELEMENT(TEXT, XML_DISPLAY_LEVELS):
            if( bNum )
            {
                sal_Int32 nTmp = aIter.toInt32();
                nNumDisplayLevels =
                    (nTmp < 1) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : static_cast<sal_Int16>(nTmp) );
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvxXMLListLevelStyleContext_Impl::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_LIST_LEVEL_PROPERTIES) ||
        nElement == XML_ELEMENT(STYLE, XML_TEXT_PROPERTIES) )
    {
        return new SvxXMLListLevelStyleAttrContext_Impl( GetImport(),
                                                         nElement,
                                                         xAttrList,
                                                         *this );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA) )
    {
        if( bImage && sImageURL.isEmpty() && !xBase64Stream.is() )
        {
            xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( xBase64Stream.is() )
                return new XMLBase64ImportContext( GetImport(), xBase64Stream );
        }
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

Sequence<beans::PropertyValue> SvxXMLListLevelStyleContext_Impl::GetProperties()
{
    sal_Int16 eType = NumberingType::NUMBER_NONE;

    sal_Int32 nCount = 0;
    if( bBullet )
    {
        eType = NumberingType::CHAR_SPECIAL;
        nCount = 15; // 'cBullet' will be written anyway if 'bBullet' is true
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
                eType, sNumFormat, sNumLetterSync, true );
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
            sSuffix.clear(); // clear it
        }
    }

    Sequence<beans::PropertyValue> aPropSeq( nCount );
    if( nCount > 0 )
    {
        beans::PropertyValue *pProps = aPropSeq.getArray();
        sal_Int32 nPos = 0;
        pProps[nPos].Name = "NumberingType";
        pProps[nPos++].Value <<= eType ;

        pProps[nPos].Name = "Prefix";
        pProps[nPos++].Value <<= sPrefix;

        pProps[nPos].Name = "Suffix";
        pProps[nPos++].Value <<= sSuffix;

        pProps[nPos].Name = "Adjust";
        pProps[nPos++].Value <<= eAdjust;

        sal_Int32 nLeftMargin = nSpaceBefore + nMinLabelWidth;
        pProps[nPos].Name = "LeftMargin";
        pProps[nPos++].Value <<= nLeftMargin;

        sal_Int32 nFirstLineOffset = -nMinLabelWidth;

        pProps[nPos].Name = "FirstLineOffset";
        pProps[nPos++].Value <<= nFirstLineOffset;

        pProps[nPos].Name = "SymbolTextDistance";
        pProps[nPos++].Value <<= static_cast<sal_Int16>(nMinLabelDist);

        pProps[nPos].Name = "PositionAndSpaceMode";
        pProps[nPos++].Value <<= ePosAndSpaceMode;
        pProps[nPos].Name = "LabelFollowedBy";
        pProps[nPos++].Value <<= eLabelFollowedBy;
        pProps[nPos].Name = "ListtabStopPosition";
        pProps[nPos++].Value <<= nListtabStopPosition;
        pProps[nPos].Name = "FirstLineIndent";
        pProps[nPos++].Value <<= nFirstLineIndent;
        pProps[nPos].Name = "IndentAt";
        pProps[nPos++].Value <<= nIndentAt;

        OUString sDisplayTextStyleName = GetImport().GetStyleDisplayName(
                                XmlStyleFamily::TEXT_TEXT, sTextStyleName  );
        pProps[nPos].Name = "CharStyleName";
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
                bool bStarSymbol = false;
                if( aFDesc.Name.equalsIgnoreAsciiCase( gsStarBats ) )
                {
                    cBullet = GetImport().ConvStarBatsCharToStarSymbol( cBullet );
                    bStarSymbol = true;
                }
                else if( aFDesc.Name.equalsIgnoreAsciiCase( gsStarMath ) )
                {
                    cBullet = GetImport().ConvStarMathCharToStarSymbol( cBullet );
                    bStarSymbol = true;
                }
                if( bStarSymbol )
                    aFDesc.Name = "StarSymbol" ;
            }

            // Must append 'cBullet' even if it is zero
            // if 'bBullet' is true and 'cBullet' is zero - BulletChar property must be 0.
            pProps[nPos].Name = "BulletChar";
            pProps[nPos++].Value <<= OUString(&cBullet, 1);

            pProps[nPos].Name = "BulletFont";
            pProps[nPos++].Value <<= aFDesc;

        }

        if( bImage )
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            if (!sImageURL.isEmpty())
            {
                xGraphic = GetImport().loadGraphicByURL(sImageURL);
            }
            else if( xBase64Stream.is() )
            {
                xGraphic = GetImport().loadGraphicFromBase64(xBase64Stream);
            }

            uno::Reference<awt::XBitmap> xBitmap;
            if (xGraphic.is())
                xBitmap.set(xGraphic, uno::UNO_QUERY);

            if (xBitmap.is())
            {
                pProps[nPos].Name = "GraphicBitmap";
                pProps[nPos++].Value <<= xBitmap;
            }

            awt::Size aSize(nImageWidth, nImageHeight);
            pProps[nPos].Name = "GraphicSize";
            pProps[nPos++].Value <<= aSize;

            pProps[nPos].Name = "VertOrient";
            pProps[nPos++].Value <<= eImageVertOrient;
        }

        if( bNum )
        {
            pProps[nPos].Name = "StartWith";
            pProps[nPos++].Value <<= nNumStartValue;

            pProps[nPos].Name = "ParentNumbering";
            pProps[nPos++].Value <<= nNumDisplayLevels;
        }

        if( ( bNum || bBullet ) && nRelSize )
        {
            pProps[nPos].Name = "BulletRelSize";
            pProps[nPos++].Value <<= nRelSize;
        }

        if( !bImage && bHasColor )
        {
            pProps[nPos].Name = "BulletColor";
            pProps[nPos++].Value <<= m_nColor;
        }

        SAL_WARN_IF( nPos != nCount, "xmloff", "array under/overflow" );
    }

    return aPropSeq;
}

SvxXMLListLevelStyleAttrContext_Impl::SvxXMLListLevelStyleAttrContext_Impl(
        SvXMLImport& rImport, sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport ),
    rListLevel( rLLevel )
{
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();

    OUString sFontName, sFontFamily, sFontStyleName, sFontFamilyGeneric,
             sFontPitch, sFontCharset;
    OUString sVerticalPos, sVerticalRel;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(TEXT, XML_SPACE_BEFORE):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView(), SHRT_MIN, SHRT_MAX))
                rListLevel.SetSpaceBefore( nVal );
            break;
        case XML_ELEMENT(TEXT, XML_MIN_LABEL_WIDTH):
            if (rUnitConv.convertMeasureToCore( nVal, aIter.toView(), 0, SHRT_MAX ))
                rListLevel.SetMinLabelWidth( nVal );
            break;
        case XML_ELEMENT(TEXT, XML_MIN_LABEL_DISTANCE):
            if (rUnitConv.convertMeasureToCore( nVal, aIter.toView(), 0, USHRT_MAX ))
                rListLevel.SetMinLabelDist( nVal );
            break;
        case XML_ELEMENT(FO, XML_TEXT_ALIGN):
        case XML_ELEMENT(FO_COMPAT, XML_TEXT_ALIGN):
            if( !aIter.isEmpty() )
            {
                sal_Int16 eAdjust = HoriOrientation::LEFT;
                if( IsXMLToken( aIter, XML_CENTER ) )
                    eAdjust = HoriOrientation::CENTER;
                else if( IsXMLToken( aIter, XML_END ) )
                    eAdjust = HoriOrientation::RIGHT;
                rListLevel.SetAdjust( eAdjust );
            }
            break;
        case XML_ELEMENT(STYLE, XML_FONT_NAME):
            sFontName = aIter.toString();
            break;
        case XML_ELEMENT(FO, XML_FONT_FAMILY):
        case XML_ELEMENT(FO_COMPAT, XML_FONT_FAMILY):
            sFontFamily = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_FONT_FAMILY_GENERIC):
            sFontFamilyGeneric = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_FONT_STYLE_NAME):
            sFontStyleName = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_FONT_PITCH):
            sFontPitch = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_FONT_CHARSET):
            sFontCharset = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_VERTICAL_POS):
            sVerticalPos = aIter.toString();
            break;
        case XML_ELEMENT(STYLE, XML_VERTICAL_REL):
            sVerticalRel = aIter.toString();
            break;
        case XML_ELEMENT(FO, XML_WIDTH):
        case XML_ELEMENT(FO_COMPAT, XML_WIDTH):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView()))
                rListLevel.SetImageWidth( nVal );
            break;
        case XML_ELEMENT(FO, XML_HEIGHT):
        case XML_ELEMENT(FO_COMPAT, XML_HEIGHT):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView()))
                rListLevel.SetImageHeight( nVal );
            break;
        case XML_ELEMENT(FO, XML_COLOR):
        case XML_ELEMENT(FO_COMPAT, XML_COLOR):
            {
                sal_Int32 nColor(0);
                if (::sax::Converter::convertColor( nColor, aIter.toView() ))
                {
                    rListLevel.SetColor( Color(nColor) );
                }
            }
            break;
        case XML_ELEMENT(STYLE, XML_USE_WINDOW_FONT_COLOR):
            {
                if( IsXMLToken( aIter, XML_TRUE ) )
                    rListLevel.SetColor(COL_AUTO);
            }
            break;
        case XML_ELEMENT(FO, XML_FONT_SIZE):
        case XML_ELEMENT(FO_COMPAT, XML_FONT_SIZE):
            if (::sax::Converter::convertPercent( nVal, aIter.toView() ))
                rListLevel.SetRelSize( static_cast<sal_Int16>(nVal) );
            break;
        case XML_ELEMENT(TEXT, XML_LIST_LEVEL_POSITION_AND_SPACE_MODE):
            {
                sal_Int16 ePosAndSpaceMode = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
                if( IsXMLToken( aIter, XML_LABEL_ALIGNMENT ) )
                    ePosAndSpaceMode = PositionAndSpaceMode::LABEL_ALIGNMENT;
                rListLevel.SetPosAndSpaceMode( ePosAndSpaceMode );
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !sFontName.isEmpty() )
    {
        const XMLFontStylesContext *pFontDecls =
            GetImport().GetFontDecls();
        if( pFontDecls )
        {
            ::std::vector < XMLPropertyState > aProps;
            if( pFontDecls->FillProperties( sFontName, aProps, 0, 1, 2, 3, 4 ) )
            {
                OUString sTmp;
                sal_Int16 nTmp = 0;
                for( const auto& rProp : aProps )
                {
                    switch( rProp.mnIndex )
                    {
                    case 0:
                        rProp.maValue >>= sTmp;
                        rListLevel.SetBulletFontName( sTmp);
                        break;
                    case 1:
                        rProp.maValue >>= sTmp;
                        rListLevel.SetBulletFontStyleName( sTmp );
                        break;
                    case 2:
                        rProp.maValue >>= nTmp;
                        rListLevel.SetBulletFontFamily( nTmp );
                        break;
                    case 3:
                        rProp.maValue >>= nTmp;
                        rListLevel.SetBulletFontPitch( nTmp );
                        break;
                    case 4:
                        rProp.maValue >>= nTmp;
                        rListLevel.SetBulletFontEncoding( nTmp );
                        break;
                    }
                }
            }
        }
    }
    if( !sFontFamily.isEmpty() )
    {
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

css::uno::Reference< css::xml::sax::XFastContextHandler > SvxXMLListLevelStyleAttrContext_Impl::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(STYLE, XML_LIST_LEVEL_LABEL_ALIGNMENT) )
    {
        return new SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl( GetImport(),
                                                             nElement,
                                                             xAttrList,
                                                             rListLevel );
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}


SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl::SvxXMLListLevelStyleLabelAlignmentAttrContext_Impl(
        SvXMLImport& rImport, sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport )
{
    SvXMLUnitConverter& rUnitConv = GetImport().GetMM100UnitConverter();

    sal_Int16 eLabelFollowedBy = LabelFollow::LISTTAB;
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(TEXT, XML_LABEL_FOLLOWED_BY):
        case XML_ELEMENT(LO_EXT, XML_LABEL_FOLLOWED_BY):
            {
                if( eLabelFollowedBy == LabelFollow::NEWLINE)
                    //NewLine from LO_EXT has precedence over other values of the Non LO_EXT namespace
                    break;
                if( IsXMLToken( aIter, XML_SPACE ) )
                    eLabelFollowedBy = LabelFollow::SPACE;
                else if( IsXMLToken( aIter, XML_NOTHING ) )
                    eLabelFollowedBy = LabelFollow::NOTHING;
                else if( IsXMLToken( aIter, XML_NEWLINE ) )
                    eLabelFollowedBy = LabelFollow::NEWLINE;
            }
            break;
        case XML_ELEMENT(TEXT, XML_LIST_TAB_STOP_POSITION):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView(), 0, SHRT_MAX))
                rLLevel.SetListtabStopPosition( nVal );
            break;
        case XML_ELEMENT(FO, XML_TEXT_INDENT):
        case XML_ELEMENT(FO_COMPAT, XML_TEXT_INDENT):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView(), SHRT_MIN, SHRT_MAX))
                rLLevel.SetFirstLineIndent( nVal );
            break;
        case XML_ELEMENT(FO, XML_MARGIN_LEFT):
        case XML_ELEMENT(FO_COMPAT, XML_MARGIN_LEFT):
            if (rUnitConv.convertMeasureToCore(nVal, aIter.toView(), SHRT_MIN, SHRT_MAX))
                rLLevel.SetIndentAt( nVal );
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    rLLevel.SetLabelFollowedBy( eLabelFollowedBy );
}

void SvxXMLListStyleContext::SetAttribute( sal_Int32 nElement,
                                           const OUString& rValue )
{
    if( nElement == XML_ELEMENT(TEXT, XML_CONSECUTIVE_NUMBERING) )
    {
        bConsecutive = IsXMLToken( rValue, XML_TRUE );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nElement, rValue );
    }
}

constexpr OUStringLiteral sIsPhysical( u"IsPhysical"  );
constexpr OUStringLiteral sNumberingRules( u"NumberingRules"  );
constexpr OUStringLiteral sIsContinuousNumbering( u"IsContinuousNumbering"  );

SvxXMLListStyleContext::SvxXMLListStyleContext( SvXMLImport& rImport,
        bool bOutl )
:   SvXMLStyleContext( rImport, bOutl ? XmlStyleFamily::TEXT_OUTLINE : XmlStyleFamily::TEXT_LIST )
,   bConsecutive( false )
,   bOutline( bOutl )
{
}

SvxXMLListStyleContext::~SvxXMLListStyleContext() {}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvxXMLListStyleContext::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( bOutline
        ? nElement == XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL_STYLE)
        : ( nElement == XML_ELEMENT(TEXT, XML_LIST_LEVEL_STYLE_NUMBER) ||
            nElement == XML_ELEMENT(TEXT, XML_LIST_LEVEL_STYLE_BULLET) ||
            nElement == XML_ELEMENT(TEXT, XML_LIST_LEVEL_STYLE_IMAGE )    ) )
    {
        rtl::Reference<SvxXMLListLevelStyleContext_Impl> xLevelStyle{
            new SvxXMLListLevelStyleContext_Impl( GetImport(), nElement, xAttrList )};
        if( !pLevelStyles )
            pLevelStyles = std::make_unique<SvxXMLListStyle_Impl>();
        pLevelStyles->push_back( xLevelStyle );

        return xLevelStyle.get();
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void SvxXMLListStyleContext::FillUnoNumRule(
        const Reference<container::XIndexReplace> & rNumRule) const
{
    try
    {
        if( pLevelStyles && rNumRule.is() )
        {
            sal_Int32 l_nLevels = rNumRule->getCount();
            for (const auto& pLevelStyle : *pLevelStyles)
            {
                sal_Int32 nLevel = pLevelStyle->GetLevel();
                if( nLevel >= 0 && nLevel < l_nLevels )
                {
                    Sequence<beans::PropertyValue> aProps =
                        pLevelStyle->GetProperties();
                    rNumRule->replaceByIndex( nLevel, Any(aProps) );
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
            xPropSet->setPropertyValue( sIsContinuousNumbering, Any(bConsecutive) );
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.style", "" );
    }
}

void SvxXMLListStyleContext::CreateAndInsertLate( bool bOverwrite )
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
                FillUnoNumRule(rNumRule);
        }
    }
    else
    {
        Reference < XStyle > xStyle;
        const OUString& rName = GetDisplayName();
        if( rName.isEmpty() )
        {
            SetValid( false );
            return;
        }

        const Reference < XNameContainer >& rNumStyles =
                GetImport().GetTextImport()->GetNumberingStyles();
        if( !rNumStyles.is() )
        {
            SetValid( false );
            return;
        }

        bool bNew = false;
        if( rNumStyles->hasByName( rName ) )
        {
            Any aAny = rNumStyles->getByName( rName );
            aAny >>= xStyle;
        }
        else
        {
            Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                        UNO_QUERY );
            SAL_WARN_IF( !xFactory.is(), "xmloff", "no factory" );
            if( !xFactory.is() )
                return;

            Reference < XInterface > xIfc = xFactory->createInstance("com.sun.star.style.NumberingStyle");
            if( !xIfc.is() )
                return;
            Reference < XStyle > xTmp( xIfc, UNO_QUERY );
            xStyle = xTmp;
            if( !xStyle.is() )
                return;

            rNumStyles->insertByName( rName, Any(xStyle) );
            bNew = true;
        }

        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
        {
            Any aAny = xPropSet->getPropertyValue( sIsPhysical );
            bNew = !*o3tl::doAccess<bool>(aAny);
        }

        if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
            xPropSet->setPropertyValue( "Hidden", uno::makeAny( IsHidden( ) ) );

        if( rName != GetName() )
            GetImport().AddStyleDisplayName( XmlStyleFamily::TEXT_LIST,
                                             GetName(), rName );

        Any aAny = xPropSet->getPropertyValue( sNumberingRules );
        aAny >>= xNumRules;
        if( bOverwrite || bNew )
        {
            FillUnoNumRule(xNumRules);
            xPropSet->setPropertyValue( sNumberingRules, Any(xNumRules) );
        }
        else
        {
            SetValid( false );
        }

        SetNew( bNew );
    }
}

void SvxXMLListStyleContext::CreateAndInsertAuto() const
{
    SAL_WARN_IF( bOutline, "xmloff", "Outlines cannot be inserted here" );
    SAL_WARN_IF( xNumRules.is(), "xmloff", "Numbering Rule is existing already" );

    const OUString& rName = GetName();
    if( bOutline || xNumRules.is() || rName.isEmpty() )
    {
        const_cast<SvxXMLListStyleContext *>(this)->SetValid( false );
        return;
    }

    const_cast<SvxXMLListStyleContext *>(this)->xNumRules = CreateNumRule(
        GetImport().GetModel() );

    FillUnoNumRule(xNumRules);
}

Reference < XIndexReplace > SvxXMLListStyleContext::CreateNumRule(
                                const Reference < XModel > & rModel )
{
    Reference<XIndexReplace> xNumRule;

    Reference< XMultiServiceFactory > xFactory( rModel, UNO_QUERY );
    SAL_WARN_IF( !xFactory.is(), "xmloff", "no factory" );
    if( !xFactory.is() )
        return xNumRule;

    Reference < XInterface > xIfc = xFactory->createInstance("com.sun.star.text.NumberingRules");
    if( !xIfc.is() )
        return xNumRule;

    xNumRule.set( xIfc, UNO_QUERY );
    SAL_WARN_IF( !xNumRule.is(), "xmloff", "go no numbering rule" );

    return xNumRule;
}

void SvxXMLListStyleContext::SetDefaultStyle(
        const Reference < XIndexReplace > & rNumRule,
        sal_Int16 nLevel,
        bool bOrdered )
{
    Sequence<beans::PropertyValue> aPropSeq( bOrdered ? 1 : 4  );
    beans::PropertyValue *pProps = aPropSeq.getArray();

    pProps->Name = "NumberingType";
    (pProps++)->Value <<= static_cast<sal_Int16>(bOrdered ? NumberingType::ARABIC
                                                 : NumberingType::CHAR_SPECIAL );
    if( !bOrdered )
    {
        // TODO: Bullet-Font
        awt::FontDescriptor aFDesc;
        aFDesc.Name =
#ifdef _WIN32
                        "StarBats"
#else
                        "starbats"
#endif
                                        ;
        aFDesc.Family = FAMILY_DONTKNOW ;
        aFDesc.Pitch = PITCH_DONTKNOW ;
        aFDesc.CharSet = RTL_TEXTENCODING_SYMBOL ;
        aFDesc.Weight = WEIGHT_DONTKNOW;
        pProps->Name = "BulletFont";
        (pProps++)->Value <<= aFDesc;

        pProps->Name = "BulletChar";
        (pProps++)->Value <<= OUString(sal_Unicode(0xF000 + 149));
        pProps->Name = "CharStyleName";
        (pProps++)->Value <<= OUString( "Numbering Symbols"  );
    }

    rNumRule->replaceByIndex( nLevel, Any(aPropSeq) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
