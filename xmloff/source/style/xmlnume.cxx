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


#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/ustrbuf.hxx>

#include <tools/debug.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlexp.hxx>
#include <tools/fontenum.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

void SvxXMLNumRuleExport::exportLevelStyles( const uno::Reference< ::com::sun::star::container::XIndexReplace > & xNumRule,
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

    sal_Unicode cBullet = 0xf095;
    OUString sBulletFontName, sBulletFontStyleName ;
    FontFamily eBulletFontFamily = FAMILY_DONTKNOW;
    FontPitch eBulletFontPitch = PITCH_DONTKNOW;
    rtl_TextEncoding eBulletFontEncoding = RTL_TEXTENCODING_DONTKNOW;

    OUString sImageURL;
    uno::Reference< ::com::sun::star::awt::XBitmap >  xBitmap;
    sal_Int32 nImageWidth = 0, nImageHeight = 0;
    sal_Int16 eImageVertOrient = VertOrientation::LINE_CENTER;

    sal_Int16 ePosAndSpaceMode = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
    sal_Int16 eLabelFollowedBy = LabelFollow::LISTTAB;
    sal_Int32 nListtabStopPosition( 0 );
    sal_Int32 nFirstLineIndent( 0 );
    sal_Int32 nIndentAt( 0 );

    const sal_Int32 nCount = rProps.getLength();
    const beans::PropertyValue* pPropArray = rProps.getConstArray();
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        const beans::PropertyValue& rProp = pPropArray[i];

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
                cBullet = (sal_Unicode)sValue[0];
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
                eBulletFontEncoding = (rtl_TextEncoding)rFDesc.CharSet;
            }
        }
        else if( rProp.Name == "GraphicURL" )
        {
            rProp.Value >>= sImageURL;
        }
        else if( rProp.Name == "GraphicBitmap" )
        {
            rProp.Value >>= xBitmap;
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
        DBG_ASSERT( !bOutline,
           "SvxXMLNumRuleExport::exportLevelStyle: invalid style for outline" );
        return;
    }

    GetExport().CheckAttrList();

    // text:level
    OUStringBuffer sTmp;
    sTmp.append( nLevel + 1 );
    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_LEVEL, sTmp.makeStringAndClear() );
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
            sTmp.append( cBullet );
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


        if( !sImageURL.isEmpty() )
        {
            OUString sURL( GetExport().AddEmbeddedGraphicObject( sImageURL ) );
            if( !sURL.isEmpty() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );

                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }
        }
        else
        {
            DBG_ASSERT( !xBitmap.is(),
                        "embedded images are not supported by now" );
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
            sTmp.append( (sal_Int32)nStartValue );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                          sTmp.makeStringAndClear() );
        }
        if( nDisplayLevels > 1 && NumberingType::NUMBER_NONE != eType )
        {
            sTmp.append( (sal_Int32)nDisplayLevels );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_DISPLAY_LEVELS,
                          sTmp.makeStringAndClear() );
        }
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, eElem,
                                  true, true );

        OUStringBuffer sBuffer;
        if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION )
        {
            nSpaceBefore += nMinLabelWidth;
            nMinLabelWidth = -nMinLabelWidth;
            if( nSpaceBefore != 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, nSpaceBefore );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_SPACE_BEFORE,
                              sBuffer.makeStringAndClear() );
            }
            if( nMinLabelWidth != 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, nMinLabelWidth );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH,
                              sBuffer.makeStringAndClear() );
            }
            if( nMinLabelDist > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, nMinLabelDist );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE,
                              sBuffer.makeStringAndClear() );
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
            case VertOrientation::BOTTOM:   // yes, its OK: BOTTOM means that the baseline
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
            case VertOrientation::TOP:      // yes, its OK: TOP means that the baseline
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
                GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, nImageWidth );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_WIDTH,
                              sBuffer.makeStringAndClear() );
            }

            if( nImageHeight > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, nImageHeight );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_HEIGHT,
                              sBuffer.makeStringAndClear() );
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

                if ( eLabelFollowedBy == LabelFollow::LISTTAB &&
                     nListtabStopPosition > 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            sBuffer, nListtabStopPosition );
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              XML_LIST_TAB_STOP_POSITION,
                                              sBuffer.makeStringAndClear() );
                }

                if ( nFirstLineIndent != 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            sBuffer, nFirstLineIndent );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_TEXT_INDENT,
                                              sBuffer.makeStringAndClear() );
                }

                if ( nIndentAt != 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasureToXML(
                            sBuffer, nIndentAt );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_MARGIN_LEFT,
                                              sBuffer.makeStringAndClear() );
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
                    if( aFamilyHdl.exportXML( sTemp, Any((sal_Int16)eBulletFontFamily), rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_FAMILY_GENERIC,
                                                  sTemp );

                    XMLFontPitchPropHdl aPitchHdl;
                    if( aPitchHdl.exportXML( sTemp, Any((sal_Int16)eBulletFontPitch), rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_PITCH, sTemp );

                    XMLFontEncodingPropHdl aEncHdl;
                    if( aEncHdl.exportXML( sTemp, Any((sal_Int16)eBulletFontEncoding), rUnitConv  ) )
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
        if( NumberingType::BITMAP == eType && !sImageURL.isEmpty() )
        {
            // optional office:binary-data
            GetExport().AddEmbeddedGraphicObjectAsBase64( sImageURL );
        }
    }
}


void SvxXMLNumRuleExport::AddListStyleAttributes()
{
}


SvxXMLNumRuleExport::SvxXMLNumRuleExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sNumberingRules( "NumberingRules" ),
    sIsPhysical( "IsPhysical" ),
    sIsContinuousNumbering( "IsContinuousNumbering" ),
    // Let list style creation depend on Load/Save option "ODF format version" (#i89178#)
    mbExportPositionAndSpaceModeLabelAlignment( true )
{
    switch ( GetExport().getDefaultVersion() )
    {
        case SvtSaveOptions::ODFVER_010:
        case SvtSaveOptions::ODFVER_011:
        {
            mbExportPositionAndSpaceModeLabelAlignment = false;
        }
        break;
        default: // ODFVER_UNKNOWN or ODFVER_012
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
    if ( bIsHidden && GetExport( ).getDefaultVersion( ) == SvtSaveOptions::ODFVER_LATEST )
        GetExport( ).AddAttribute( XML_NAMESPACE_STYLE, XML_HIDDEN, "true" );

    // text:consecutive-numbering="..."
    bool bContNumbering = false;
    if( xPropSetInfo.is() &&
        xPropSetInfo->hasPropertyByName( sIsContinuousNumbering ) )
    {
        Any aAny( xPropSet->getPropertyValue( sIsContinuousNumbering ) );
        bContNumbering = *static_cast<sal_Bool const *>(aAny.getValue());
    }
    if( bContNumbering )
        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                  XML_CONSECUTIVE_NUMBERING, XML_TRUE );

    // other application specific attributes
    AddListStyleAttributes();

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_LIST_STYLE ,
                                  true, true );
        exportLevelStyles( rNumRule );
    }
}

bool SvxXMLNumRuleExport::exportStyle( const Reference< XStyle >& rStyle )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*static_cast<sal_Bool const *>(aAny.getValue()) )
            return false;
    }

    aAny = xPropSet->getPropertyValue( sNumberingRules );
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

    return true;
}

void SvxXMLNumRuleExport::exportOutline()
{
    Reference< XChapterNumberingSupplier > xCNSupplier( GetExport().GetModel(),
                                                        UNO_QUERY );
    DBG_ASSERT( xCNSupplier.is(), "no chapter numbering supplier" );

    if( xCNSupplier.is() )
    {
        Reference< XIndexReplace > xNumRule( xCNSupplier->getChapterNumberingRules() );
        DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );

        if( xNumRule.is() )
        {
            /* Outline style has property style:name since ODF 1.2
               Thus, export this property and adjust fix for issue #i69627# (#i90780#)
            */
            OUString sOutlineStyleName;
            {
                Reference<XPropertySet> xNumRulePropSet(
                    xCNSupplier->getChapterNumberingRules(), UNO_QUERY );
                if (xNumRulePropSet.is())
                {
                    const OUString sName( "Name" );
                    xNumRulePropSet->getPropertyValue( sName ) >>= sOutlineStyleName;
                }
            }
            const SvtSaveOptions::ODFDefaultVersion nODFVersion =
                                                GetExport().getDefaultVersion();
            if ( ( nODFVersion == SvtSaveOptions::ODFVER_010 ||
                   nODFVersion == SvtSaveOptions::ODFVER_011 ) &&
                 GetExport().writeOutlineStyleAsNormalListStyle() )
            {
                exportNumberingRule( sOutlineStyleName, false, xNumRule );
            }
            else
            {
                if ( nODFVersion != SvtSaveOptions::ODFVER_010 &&
                     nODFVersion != SvtSaveOptions::ODFVER_011 )
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
    }
}

void SvxXMLNumRuleExport::exportStyles( bool bUsed,
                                        XMLTextListAutoStylePool *pPool,
                                        bool bExportChapterNumbering )
{
    if( bExportChapterNumbering )
        exportOutline();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(), UNO_QUERY );
    DBG_ASSERT( xFamiliesSupp.is(), "No XStyleFamiliesSupplier from XModel for export!" );
    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        DBG_ASSERT( xFamiliesSupp.is(), "getStyleFamilies() from XModel failed for export!" );

        if( xFamilies.is() )
        {
            const OUString aNumberStyleName( "NumberingStyles" );

            Reference< XIndexAccess > xStyles;
            if( xFamilies->hasByName( aNumberStyleName ) )
            {
                xFamilies->getByName( aNumberStyleName ) >>= xStyles;

                DBG_ASSERT( xStyles.is(), "Style not found for export!" );

                if( xStyles.is() )
                {
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
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
