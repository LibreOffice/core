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

#include <sal/config.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <o3tl/any.hxx>
#include <o3tl/temporary.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlexp.hxx>
#include <tools/fontenum.hxx>
#include <vcl/vclenum.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

void SvxXMLNumRuleExport::exportLevelStyles( const uno::Reference< css::container::XIndexReplace > & xNumRule,
                                             bool bOutline )
{
    sal_Int32 nCount = xNumRule ->getCount();
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        uno::Any aEntry( xNumRule->getByIndex( i ) );
        uno::Sequence<beans::PropertyValue> aSeq;
        if( aEntry >>= aSeq )
        {
            exportLevelStyle( i, aSeq, bOutline );
        }
    }
}

void SvxXMLNumRuleExport::exportLevelStyle( sal_Int32 nLevel,
                                    const uno::Sequence<beans::PropertyValue>& rProps,
                                    bool bOutline )
{
    sal_Int16 eType = NumberingType::CHAR_SPECIAL;

    sal_Int16 eAdjust = HoriOrientation::LEFT;
    OUString sPrefix, sSuffix;
    OUString sTextStyleName;
    bool bHasColor = false;
    sal_Int32 nColor = 0;
    sal_Int32 nSpaceBefore = 0, nMinLabelWidth = 0, nMinLabelDist = 0;

    sal_Int16 nStartValue = 1, nDisplayLevels = 1, nBullRelSize = 0;

    sal_UCS4 cBullet = 0xf095;
    OUString sBulletFontName, sBulletFontStyleName ;
    FontFamily eBulletFontFamily = FAMILY_DONTKNOW;
    FontPitch eBulletFontPitch = PITCH_DONTKNOW;
    rtl_TextEncoding eBulletFontEncoding = RTL_TEXTENCODING_DONTKNOW;

    uno::Reference<graphic::XGraphic> xGraphic;

    sal_Int32 nImageWidth = 0, nImageHeight = 0;
    sal_Int16 eImageVertOrient = VertOrientation::LINE_CENTER;

    sal_Int16 ePosAndSpaceMode = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
    sal_Int16 eLabelFollowedBy = LabelFollow::LISTTAB;
    sal_Int32 nListtabStopPosition( 0 );
    sal_Int32 nFirstLineIndent( 0 );
    sal_Int32 nIndentAt( 0 );

    for( const beans::PropertyValue& rProp : rProps )
    {
        if( rProp.Name == "NumberingType" )
        {
            rProp.Value >>= eType;
        }
        else if( rProp.Name == "Prefix" )
        {
            rProp.Value >>= sPrefix;
        }
        else if( rProp.Name == "Suffix" )
        {
            rProp.Value >>= sSuffix;
        }
        else if( rProp.Name == "BulletChar" )
        {
            OUString sValue;
            rProp.Value >>= sValue;
            if( !sValue.isEmpty() )
            {
                cBullet = sValue.iterateCodePoints(&o3tl::temporary(sal_Int32(0)));
            }
        }
        else if( rProp.Name == "BulletRelSize" )
        {
            rProp.Value >>= nBullRelSize;
        }
        else if( rProp.Name == "Adjust" )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eAdjust = nValue;
        }
        else if( rProp.Name == "BulletFont" )
        {
            awt::FontDescriptor rFDesc;
            if( rProp.Value >>= rFDesc )
            {
                sBulletFontName = rFDesc.Name;
                sBulletFontStyleName = rFDesc.StyleName;
                eBulletFontFamily = static_cast< FontFamily >( rFDesc.Family );
                eBulletFontPitch = static_cast< FontPitch >( rFDesc.Pitch );
                eBulletFontEncoding = static_cast<rtl_TextEncoding>(rFDesc.CharSet);
            }
        }
        else if( rProp.Name == "GraphicBitmap" )
        {
            uno::Reference<awt::XBitmap> xBitmap;
            rProp.Value >>= xBitmap;
            xGraphic.set(xBitmap, uno::UNO_QUERY);
        }
        else if( rProp.Name == "BulletColor" )
        {
            rProp.Value >>= nColor;
            bHasColor = true;
        }
        else  if( rProp.Name == "StartWith" )
        {
            rProp.Value >>= nStartValue;
        }
        else  if( rProp.Name == "LeftMargin" )
        {
            rProp.Value >>= nSpaceBefore;
        }
        else  if( rProp.Name == "FirstLineOffset" )
        {
            rProp.Value >>= nMinLabelWidth;
        }
        else  if( rProp.Name == "SymbolTextDistance" )
        {
            rProp.Value >>= nMinLabelDist;
        }
        else if( rProp.Name == "ParentNumbering" )
        {
            rProp.Value >>= nDisplayLevels;
            if( nDisplayLevels > nLevel+1 )
                nDisplayLevels = static_cast<sal_Int16>( nLevel )+1;
        }
        else if( rProp.Name == "CharStyleName" )
        {
            rProp.Value >>= sTextStyleName;
        }
        else if( rProp.Name == "GraphicSize" )
        {
            awt::Size aSize;
            if( rProp.Value >>= aSize )
            {
                nImageWidth = aSize.Width;
                nImageHeight = aSize.Height;
            }
        }
        else if( rProp.Name == "VertOrient" )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eImageVertOrient = nValue;
        }
        else if( rProp.Name == "PositionAndSpaceMode" )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            ePosAndSpaceMode = nValue;
        }
        else if( rProp.Name == "LabelFollowedBy" )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eLabelFollowedBy = nValue;
        }
        else if( rProp.Name == "ListtabStopPosition" )
        {
            rProp.Value >>= nListtabStopPosition;
        }
        else if( rProp.Name == "FirstLineIndent" )
        {
            rProp.Value >>= nFirstLineIndent;
        }
        else if( rProp.Name == "IndentAt" )
        {
            rProp.Value >>= nIndentAt;
        }
    }

    if( bOutline && (NumberingType::CHAR_SPECIAL == eType ||
                     NumberingType::BITMAP == eType) )
    {
        SAL_WARN_IF( bOutline, "xmloff",
           "SvxXMLNumRuleExport::exportLevelStyle: invalid style for outline" );
        return;
    }

    GetExport().CheckAttrList();

    // text:level
    OUStringBuffer sTmp;
    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_LEVEL, OUString::number( nLevel + 1 ) );
    // #i110694#: no style-name on list-level-style-image
    // #i116149#: neither prefix/suffix
    if (NumberingType::BITMAP != eType)
    {
        if (!sTextStyleName.isEmpty())
        {
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                    GetExport().EncodeStyleName( sTextStyleName ) );
        }
        if (!sPrefix.isEmpty())
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_PREFIX,
                    sPrefix );
        }
        if (!sSuffix.isEmpty())
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_SUFFIX,
                    sSuffix );
        }
    }

    enum XMLTokenEnum eElem = XML_LIST_LEVEL_STYLE_NUMBER;
    if( NumberingType::CHAR_SPECIAL == eType )
    {
        // <text:list-level-style-bullet>
        eElem = XML_LIST_LEVEL_STYLE_BULLET;

        if( cBullet )
        {
            if( cBullet < ' ' )
            {
                cBullet = 0xF000 + 149;
            }
            // text:bullet-char="..."
            sTmp.append(OUString(&cBullet, 1));
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_BULLET_CHAR,
                          sTmp.makeStringAndClear() );
        }
        else
        {
            // If 'cBullet' is zero, XML_BULLET_CHAR must exist with blank.
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_BULLET_CHAR, "");
        }
    }
    else if( NumberingType::BITMAP == eType )
    {
        // <text:list-level-style-image>

        eElem = XML_LIST_LEVEL_STYLE_IMAGE;

        if (xGraphic.is())
        {
            OUString sUsedMimeType;
            OUString sInternalURL = GetExport().AddEmbeddedXGraphic(xGraphic, sUsedMimeType);
            if (!sInternalURL.isEmpty())
            {
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }
        }
        else
        {
            SAL_WARN_IF(xGraphic.is(), "xmloff", "embedded images are not supported by now");
        }
    }
    else
    {
        // <text:list-level-style-number> or <text:outline-level-style>
        if( bOutline )
            eElem = XML_OUTLINE_LEVEL_STYLE;
        else
            eElem = XML_LIST_LEVEL_STYLE_NUMBER;

        GetExport().GetMM100UnitConverter().convertNumFormat( sTmp, eType );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                       sTmp.makeStringAndClear() );
        SvXMLUnitConverter::convertNumLetterSync( sTmp, eType );
        if( !sTmp.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_NUM_LETTER_SYNC,
                                           sTmp.makeStringAndClear() );

        if( nStartValue != 1 )
        {
            sTmp.append( static_cast<sal_Int32>(nStartValue) );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                          sTmp.makeStringAndClear() );
        }
        if( nDisplayLevels > 1 && NumberingType::NUMBER_NONE != eType )
        {
            sTmp.append( static_cast<sal_Int32>(nDisplayLevels) );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_DISPLAY_LEVELS,
                          sTmp.makeStringAndClear() );
        }
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, eElem,
                                  true, true );

        if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION )
        {
            nSpaceBefore += nMinLabelWidth;
            nMinLabelWidth = -nMinLabelWidth;
            if( nSpaceBefore != 0 )
            {
                OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        nSpaceBefore );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_SPACE_BEFORE, sAttr );
            }
            if( nMinLabelWidth != 0 )
            {
                OUString s = GetExport().GetMM100UnitConverter().convertMeasureToXML( nMinLabelWidth );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH, s);
            }
            if( nMinLabelDist > 0 )
            {
                OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        nMinLabelDist );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE, sAttr);
            }
        }
        /* Check, if properties for position-and-space-mode LABEL_ALIGNMENT
           are allowed to be exported. (#i89178#)
        */
        else if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_ALIGNMENT &&
                  mbExportPositionAndSpaceModeLabelAlignment )
        {
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      XML_LIST_LEVEL_POSITION_AND_SPACE_MODE,
                                      XML_LABEL_ALIGNMENT );
        }
        if( HoriOrientation::LEFT != eAdjust )
        {
            enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
            switch( eAdjust )
            {
            case HoriOrientation::RIGHT:    eValue = XML_END;   break;
            case HoriOrientation::CENTER:   eValue = XML_CENTER;    break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_TEXT_ALIGN, eValue );
        }

        if( NumberingType::BITMAP == eType )
        {
            enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
            switch( eImageVertOrient )
            {
            case VertOrientation::BOTTOM:   // yes, it's OK: BOTTOM means that the baseline
                                    // hits the frame at its topmost position
            case VertOrientation::LINE_TOP:
            case VertOrientation::CHAR_TOP:
                eValue = XML_TOP;
                break;
            case VertOrientation::CENTER:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::CHAR_CENTER:
                eValue = XML_MIDDLE;
                break;
            case VertOrientation::TOP:      // yes, it's OK: TOP means that the baseline
                                    // hits the frame at its bottommost position
            case VertOrientation::LINE_BOTTOM:
            case VertOrientation::CHAR_BOTTOM:
                eValue = XML_BOTTOM;
                break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_VERTICAL_POS, eValue );

            eValue = XML_TOKEN_INVALID;
            switch( eImageVertOrient )
            {
            case VertOrientation::TOP:
            case VertOrientation::CENTER:
            case VertOrientation::BOTTOM:
                eValue = XML_BASELINE;
                break;
            case VertOrientation::LINE_TOP:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::LINE_BOTTOM:
                eValue = XML_LINE;
                break;
            case VertOrientation::CHAR_TOP:
            case VertOrientation::CHAR_CENTER:
            case VertOrientation::CHAR_BOTTOM:
                eValue = XML_CHAR;
                break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_VERTICAL_REL, eValue );

            if( nImageWidth > 0 )
            {
                OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        nImageWidth );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_WIDTH, sAttr );
            }

            if( nImageHeight > 0 )
            {
                OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        nImageHeight );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_HEIGHT, sAttr );
            }
        }

        {
            SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_LIST_LEVEL_PROPERTIES, true, true );

            /* Check, if properties for position-and-space-mode LABEL_ALIGNMENT
               are allowed to be exported. (#i89178#)
            */
            if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_ALIGNMENT &&
                 mbExportPositionAndSpaceModeLabelAlignment )
            {
                enum XMLTokenEnum eValue = XML_LISTTAB;
                if ( eLabelFollowedBy == LabelFollow::SPACE )
                {
                    eValue = XML_SPACE;
                }
                else if ( eLabelFollowedBy == LabelFollow::NOTHING )
                {
                    eValue = XML_NOTHING;
                }
                GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                          XML_LABEL_FOLLOWED_BY, eValue );

                if (eLabelFollowedBy == LabelFollow::NEWLINE)
                {
                    eValue = XML_NEWLINE;
                    GetExport().AddAttribute( XML_NAMESPACE_LO_EXT,
                                          XML_LABEL_FOLLOWED_BY, eValue );
                }

                if ( eLabelFollowedBy == LabelFollow::LISTTAB &&
                     nListtabStopPosition > 0 )
                {
                    OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            nListtabStopPosition );
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              XML_LIST_TAB_STOP_POSITION,
                                              sAttr );
                }

                if ( nFirstLineIndent != 0 )
                {
                    OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            nFirstLineIndent );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_TEXT_INDENT,
                                              sAttr );
                }

                if ( nIndentAt != 0 )
                {
                    OUString sAttr = GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            nIndentAt );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_MARGIN_LEFT,
                                              sAttr );
                }

                SvXMLElementExport aLabelAlignmentElement( GetExport(), XML_NAMESPACE_STYLE,
                                             XML_LIST_LEVEL_LABEL_ALIGNMENT,
                                             true, true );
            }
        }

        if( NumberingType::CHAR_SPECIAL == eType )
        {
            if( !sBulletFontName.isEmpty() )
            {
                OUString sStyleName =
                    GetExport().GetFontAutoStylePool()->Find(
                        sBulletFontName, sBulletFontStyleName,
                        eBulletFontFamily, eBulletFontPitch,
                        eBulletFontEncoding );

                if( !sStyleName.isEmpty() )
                {
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_NAME,
                                                  sStyleName );
                }
                else
                {
                    OUString sTemp;

                    const SvXMLUnitConverter& rUnitConv =
                        GetExport().GetMM100UnitConverter();
                    XMLFontFamilyNamePropHdl aFamilyNameHdl;
                    if( aFamilyNameHdl.exportXML( sTemp, Any(sBulletFontName), rUnitConv ) )
                        GetExport().AddAttribute( XML_NAMESPACE_FO,
                                                  XML_FONT_FAMILY, sTemp );

                    if( !sBulletFontStyleName.isEmpty() )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_STYLE_NAME,
                                                  sBulletFontStyleName );

                    XMLFontFamilyPropHdl aFamilyHdl;
                    if( aFamilyHdl.exportXML( sTemp, Any(static_cast<sal_Int16>(eBulletFontFamily)), rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_FAMILY_GENERIC,
                                                  sTemp );

                    XMLFontPitchPropHdl aPitchHdl;
                    if( aPitchHdl.exportXML( sTemp, Any(static_cast<sal_Int16>(eBulletFontPitch)), rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_PITCH, sTemp );

                    XMLFontEncodingPropHdl aEncHdl;
                    if( aEncHdl.exportXML( sTemp, Any(static_cast<sal_Int16>(eBulletFontEncoding)), rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_CHARSET, sTemp );
                }
            }
        }
        if( NumberingType::BITMAP != eType )
        {
            // fo:color = "#..."
            if( bHasColor )
            {
                if (0xffffffff == static_cast<sal_uInt32>(nColor))
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_USE_WINDOW_FONT_COLOR, XML_TRUE );
                }
                else
                {
                    OUStringBuffer sBuffer;
                    ::sax::Converter::convertColor( sBuffer, nColor );
                    GetExport().AddAttribute( XML_NAMESPACE_FO, XML_COLOR,
                                  sBuffer.makeStringAndClear() );
                }
            }
            // fo:height="...%"
            if( nBullRelSize )
            {
                ::sax::Converter::convertPercent( sTmp, nBullRelSize );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_FONT_SIZE,
                              sTmp.makeStringAndClear() );
            }
        }
        if( GetExport().GetAttrList().getLength() > 0 )
        {
            SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_TEXT_PROPERTIES, true, true );
        }
        if (xGraphic.is() && NumberingType::BITMAP == eType)
        {
            // optional office:binary-data
            GetExport().AddEmbeddedXGraphicAsBase64(xGraphic);
        }
    }
}


constexpr OUStringLiteral gsNumberingRules( u"NumberingRules" );
constexpr OUStringLiteral gsIsPhysical( u"IsPhysical" );
constexpr OUStringLiteral gsIsContinuousNumbering( u"IsContinuousNumbering" );

SvxXMLNumRuleExport::SvxXMLNumRuleExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    // Let list style creation depend on Load/Save option "ODF format version" (#i89178#)
    mbExportPositionAndSpaceModeLabelAlignment( true )
{
    switch (GetExport().getSaneDefaultVersion())
    {
        case SvtSaveOptions::ODFSVER_010:
        case SvtSaveOptions::ODFSVER_011:
        {
            mbExportPositionAndSpaceModeLabelAlignment = false;
        }
        break;
        default: // >= ODFSVER_012
        {
            mbExportPositionAndSpaceModeLabelAlignment = true;
        }
    }
}

SvxXMLNumRuleExport::~SvxXMLNumRuleExport()
{
}

void SvxXMLNumRuleExport::exportNumberingRule(
        const OUString& rName, bool bIsHidden,
        const Reference< XIndexReplace >& rNumRule )
{
    Reference< XPropertySet > xPropSet( rNumRule, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo;
    if( xPropSet.is() )
           xPropSetInfo = xPropSet->getPropertySetInfo();

    GetExport().CheckAttrList();

    // style:name="..."
    if( !rName.isEmpty() )
    {
        bool bEncoded = false;
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                          GetExport().EncodeStyleName( rName, &bEncoded ) );
        if( bEncoded )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                                 rName);
    }

    // style:hidden="..."
    if (bIsHidden
        && GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
    {
        GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_HIDDEN, "true");
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_HIDDEN, "true"); // FIXME for compatibility
    }

    // text:consecutive-numbering="..."
    bool bContNumbering = false;
    if( xPropSetInfo.is() &&
        xPropSetInfo->hasPropertyByName( gsIsContinuousNumbering ) )
    {
        Any aAny( xPropSet->getPropertyValue( gsIsContinuousNumbering ) );
        bContNumbering = *o3tl::doAccess<bool>(aAny);
    }
    if( bContNumbering )
        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                  XML_CONSECUTIVE_NUMBERING, XML_TRUE );

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_LIST_STYLE ,
                                  true, true );
        exportLevelStyles( rNumRule );
    }
}

void SvxXMLNumRuleExport::exportStyle( const Reference< XStyle >& rStyle )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( gsIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( gsIsPhysical );
        if( !*o3tl::doAccess<bool>(aAny) )
            return;
    }

    aAny = xPropSet->getPropertyValue( gsNumberingRules );
    Reference<XIndexReplace> xNumRule;
    aAny >>= xNumRule;

    OUString sName = rStyle->getName();

    bool bHidden = false;
    if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
    {
        aAny = xPropSet->getPropertyValue( "Hidden" );
        aAny >>= bHidden;
    }

    exportNumberingRule( sName, bHidden, xNumRule );
}

void SvxXMLNumRuleExport::exportOutline()
{
    Reference< XChapterNumberingSupplier > xCNSupplier( GetExport().GetModel(),
                                                        UNO_QUERY );
    SAL_WARN_IF( !xCNSupplier.is(), "xmloff", "no chapter numbering supplier" );

    if( !xCNSupplier.is() )
        return;

    Reference< XIndexReplace > xNumRule( xCNSupplier->getChapterNumberingRules() );
    SAL_WARN_IF( !xNumRule.is(), "xmloff", "no chapter numbering rules" );

    if( !xNumRule.is() )
        return;

    /* Outline style has property style:name since ODF 1.2
       Thus, export this property and adjust fix for issue #i69627# (#i90780#)
    */
    OUString sOutlineStyleName;
    {
        Reference<XPropertySet> xNumRulePropSet(
            xCNSupplier->getChapterNumberingRules(), UNO_QUERY );
        if (xNumRulePropSet.is())
        {
            xNumRulePropSet->getPropertyValue( "Name" ) >>= sOutlineStyleName;
        }
    }
    const SvtSaveOptions::ODFSaneDefaultVersion nODFVersion =
                                        GetExport().getSaneDefaultVersion();
    if ((nODFVersion == SvtSaveOptions::ODFSVER_010 ||
         nODFVersion == SvtSaveOptions::ODFSVER_011)
        && GetExport().writeOutlineStyleAsNormalListStyle())
    {
        exportNumberingRule( sOutlineStyleName, false, xNumRule );
    }
    else
    {
        if (nODFVersion != SvtSaveOptions::ODFSVER_010 &&
            nODFVersion != SvtSaveOptions::ODFSVER_011)
        {
            // style:name="..."
            GetExport().CheckAttrList();
            if ( !sOutlineStyleName.isEmpty() )
             {
                bool bEncoded = false;
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                                GetExport().EncodeStyleName( sOutlineStyleName,
                                                             &bEncoded ) );
                if( bEncoded )
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_DISPLAY_NAME,
                                              sOutlineStyleName );
            }
        }
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                  XML_OUTLINE_STYLE, true, true );
        exportLevelStyles( xNumRule, true );
    }
}

void SvxXMLNumRuleExport::exportStyles( bool bUsed,
                                        XMLTextListAutoStylePool *pPool,
                                        bool bExportChapterNumbering )
{
    if( bExportChapterNumbering )
        exportOutline();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(), UNO_QUERY );
    SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff", "No XStyleFamiliesSupplier from XModel for export!" );
    if( !xFamiliesSupp.is() )
        return;

    Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
    SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff", "getStyleFamilies() from XModel failed for export!" );

    if( !xFamilies.is() )
        return;

    static const OUStringLiteral aNumberStyleName( u"NumberingStyles" );

    Reference< XIndexAccess > xStyles;
    if( !xFamilies->hasByName( aNumberStyleName ) )
        return;

    xFamilies->getByName( aNumberStyleName ) >>= xStyles;

    SAL_WARN_IF( !xStyles.is(), "xmloff", "Style not found for export!" );

    if( !xStyles.is() )
        return;

    const sal_Int32 nStyles = xStyles->getCount();

    for( sal_Int32 i=0; i < nStyles; i++ )
    {
        Reference< XStyle > xStyle;
        xStyles->getByIndex( i ) >>= xStyle;

        if( !bUsed || xStyle->isInUse() )
        {
            exportStyle( xStyle );
            if( pPool )
                pPool->RegisterName( xStyle->getName() );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
