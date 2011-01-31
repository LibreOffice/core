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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "PropertyMapper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

namespace
{

void lcl_overwriteOrAppendValues(
    tPropertyNameValueMap &rMap, const tPropertyNameValueMap& rOverwriteMap )
{
    tPropertyNameValueMap::const_iterator aIt( rOverwriteMap.begin() );
    tPropertyNameValueMap::const_iterator aEnd( rOverwriteMap.end() );

    for( ; aIt != aEnd; ++aIt )
        rMap[ aIt->first ] = aIt->second;
}

} // anonymous namespace

void PropertyMapper::setMappedProperties(
          const uno::Reference< beans::XPropertySet >& xTarget
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap
        , tPropertyNameValueMap* pOverwriteMap )
{
    if( !xTarget.is() || !xSource.is() )
        return;

    tNameSequence aNames;
    tAnySequence  aValues;
    getMultiPropertyLists(aNames, aValues, xSource, rMap );
    if(pOverwriteMap && (aNames.getLength() == aValues.getLength()))
    {
        tPropertyNameValueMap aNewMap;
        for( sal_Int32 nI=0; nI<aNames.getLength(); ++nI )
            aNewMap[ aNames[nI] ] = aValues[nI];
        lcl_overwriteOrAppendValues( aNewMap, *pOverwriteMap );
        aNames = ContainerHelper::MapKeysToSequence( aNewMap );
        aValues = ContainerHelper::MapValuesToSequence( aNewMap );
    }

    PropertyMapper::setMultiProperties( aNames, aValues, xTarget );
}

void PropertyMapper::getValueMap(
                  tPropertyNameValueMap& rValueMap
                , const tPropertyNameMap& rNameMap
                , const uno::Reference< beans::XPropertySet >& xSourceProp
                )
{
    tPropertyNameMap::const_iterator aIt( rNameMap.begin() );
    tPropertyNameMap::const_iterator aEnd( rNameMap.end() );

    for( ; aIt != aEnd; ++aIt )
    {
        rtl::OUString aTarget = aIt->first;
        rtl::OUString aSource = aIt->second;
        try
        {
            uno::Any aAny( xSourceProp->getPropertyValue(aSource) );
            if( aAny.hasValue() )
                rValueMap.insert( tPropertyNameValueMap::value_type( aTarget, aAny ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

void PropertyMapper::getMultiPropertyLists(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const uno::Reference< beans::XPropertySet >& xSourceProp
                , const tPropertyNameMap& rNameMap
                )
{
    tPropertyNameValueMap aValueMap;
    getValueMap( aValueMap, rNameMap, xSourceProp );
    getMultiPropertyListsFromValueMap( rNames, rValues, aValueMap );
}

void PropertyMapper::getMultiPropertyListsFromValueMap(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const tPropertyNameValueMap& rValueMap
                )
{
    sal_Int32 nPropertyCount = rValueMap.size();
    rNames.realloc(nPropertyCount);
    rValues.realloc(nPropertyCount);

    //fill sequences
    tPropertyNameValueMap::const_iterator aValueIt(  rValueMap.begin() );
    tPropertyNameValueMap::const_iterator aValueEnd( rValueMap.end()   );
    sal_Int32 nN=0;
    for( ; aValueIt != aValueEnd; ++aValueIt )
    {
        const uno::Any& rAny = aValueIt->second;
        if( rAny.hasValue() )
        {
            //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
            rNames[nN]  = aValueIt->first;
            rValues[nN] = rAny;
            ++nN;
        }
    }
    //reduce to real property count
    rNames.realloc(nN);
    rValues.realloc(nN);
}

uno::Any* PropertyMapper::getValuePointer( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , const rtl::OUString& rPropName )
{
    sal_Int32 nCount = rPropNames.getLength();
    for( sal_Int32 nN = 0; nN < nCount; nN++ )
    {
        if(rPropNames[nN].equals(rPropName))
            return &rPropValues[nN];
    }
    return NULL;
}

uno::Any* PropertyMapper::getValuePointerForLimitedSpace( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , bool bLimitedHeight)
{
    return PropertyMapper::getValuePointer( rPropValues, rPropNames
        , bLimitedHeight ? C2U("TextMaximumFrameHeight") : C2U("TextMaximumFrameWidth") );
}

/*
//set some properties from service style::CharacterProperties:
//-------- tabpage: Zeichen -----------
//Schriftart z.B. Albany            UNO_NAME_EDIT_CHAR_FONTNAME == UNO_NAME_EDIT_CHAR_FONTSTYLENAME    //UNO_NAME_CHAR_FONT
//Schriftschnitt z.B. kursiv        UNO_NAME_EDIT_CHAR_POSTURE    UNO_NAME_CHAR_POSTURE awt::FontSlant NONE OBLIQUE ITALIC DONTKNOW REVERSE_OBLIQUE REVERSE_ITALIC
//Schriftgrad (Punktgrösse z.B. 12) UNO_NAME_EDIT_CHAR_HEIGHT == UNO_NAME_CHAR_HEIGHT
        //? UNO_NAME_EDIT_CHAR_WEIGHT == UNO_NAME_CHAR_WEIGHT
//Sprache                           UNO_NAME_EDIT_CHAR_LOCALE lang::Locale

//-------- tabpage: Schrifteffekt -----------
//Unterstreichung                   UNO_NAME_CHAR_UNDERLINE sal_Int16 awt::FontUnderline_NONE _SINGLE _DOUBLE _DOTTED _DONTKNOW _DASH ...
//Unterstreichung-farbe             ??? 'CharUnderlineColor' + CharUnderlineHasColor
//Durchstreichung z.B. doppelt      "CharStrikeout" sal_Int16 awt::FontStrikeout_NONE _SINGLE _DOUBLE ...
//wortweise-Durchstreichung ja/nein "CharWordMode" bool
//Schriftfarbe                      UNO_NAME_EDIT_CHAR_COLOR sal_Int32      UNO_NAME_CHAR_COLOR
//ReliefArt ohne/erhaben/tief       "CharRelief" sal_Int16 text::FontRelief_NONE FontRelief_EMBOSSED FontRelief_ENGRAVED
//Kontur                            "CharContoured" bool
//Schatten                          UNO_NAME_CHAR_SHADOWED bool
*/

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForCharacterProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForCharacterProperties =
        tMakePropertyNameMap
//      ( C2U( "CharBackColor" ),           C2U("TextBackgroundColor") )
//      ( C2U( "CharCaseMap" ),             C2U("CaseMapping") )
        ( C2U( "CharColor" ),               C2U("CharColor") )
        ( C2U( "CharContoured" ),           C2U("CharContoured") )
/////// ( C2U( "CharCrossedOut" ),          C2U("CharCrossedOut") ) //setting this explicitly somehow conflicts with CharStrikeout
        ( C2U( "CharEmphasis" ),            C2U("CharEmphasis") )//the service style::CharacterProperties  describes a property called 'CharEmphasize' wich is nowhere implemented
//        ( C2U( "CharEscapement" ),          C2U("CharEscapement") ) //#i98344# @future: add these to properties again, if the user interface offers the possibility to change them; then make sure that older wrong files are corrected on import
//        ( C2U( "CharEscapementHeight" ),    C2U("CharEscapementHeight") ) //#i98344# @future: add these to properties again, if the user interface offers the possibility to change them; then make sure that older wrong files are corrected on import
//      ( C2U( "CharFlash" ),               C2U("Flashing") )

        ( C2U( "CharFontFamily" ),          C2U("CharFontFamily") )
        ( C2U( "CharFontFamilyAsian" ),     C2U("CharFontFamilyAsian") )
        ( C2U( "CharFontFamilyComplex" ),   C2U("CharFontFamilyComplex") )
        ( C2U( "CharFontCharSet" ),         C2U("CharFontCharSet") )
        ( C2U( "CharFontCharSetAsian" ),    C2U("CharFontCharSetAsian") )
        ( C2U( "CharFontCharSetComplex" ),  C2U("CharFontCharSetComplex") )
        ( C2U( "CharFontName" ),            C2U("CharFontName") )
        ( C2U( "CharFontNameAsian" ),       C2U("CharFontNameAsian") )
        ( C2U( "CharFontNameComplex" ),     C2U("CharFontNameComplex") )
        ( C2U( "CharFontPitch" ),           C2U("CharFontPitch") )
        ( C2U( "CharFontPitchAsian" ),      C2U("CharFontPitchAsian") )
        ( C2U( "CharFontPitchComplex" ),    C2U("CharFontPitchComplex") )
        ( C2U( "CharFontStyleName" ),       C2U("CharFontStyleName") )
        ( C2U( "CharFontStyleNameAsian" ),  C2U("CharFontStyleNameAsian") )
        ( C2U( "CharFontStyleNameComplex" ),C2U("CharFontStyleNameComplex") )

        ( C2U( "CharHeight" ),              C2U("CharHeight") )
        ( C2U( "CharHeightAsian" ),         C2U("CharHeightAsian") )
        ( C2U( "CharHeightComplex" ),       C2U("CharHeightComplex") )
        ( C2U( "CharKerning" ),             C2U("CharKerning") )
        ( C2U( "CharLocale" ),              C2U("CharLocale") )
        ( C2U( "CharLocaleAsian" ),         C2U("CharLocaleAsian") )
        ( C2U( "CharLocaleComplex" ),       C2U("CharLocaleComplex") )
//      ( C2U( "CharNoHyphenation" ),       C2U("InhibitHyphenation") )
        ( C2U( "CharPosture" ),             C2U("CharPosture") )
        ( C2U( "CharPostureAsian" ),        C2U("CharPostureAsian") )
        ( C2U( "CharPostureComplex" ),      C2U("CharPostureComplex") )
        ( C2U( "CharRelief" ),              C2U("CharRelief") )
//      ( C2U( "CharRotation" ),            C2U("Rotation") ) --> additional feature ...
//      ( C2U( "CharScaleWidth" ),          C2U("CharScaleWidth") )
        ( C2U( "CharShadowed" ),            C2U("CharShadowed") )
        ( C2U( "CharStrikeout" ),           C2U("CharStrikeout") )
        ( C2U( "CharUnderline" ),           C2U("CharUnderline") )
        ( C2U( "CharUnderlineColor" ),      C2U("CharUnderlineColor") )
        ( C2U( "CharUnderlineHasColor" ),   C2U("CharUnderlineHasColor") )
        ( C2U( "CharOverline" ),            C2U("CharOverline") )
        ( C2U( "CharOverlineColor" ),       C2U("CharOverlineColor") )
        ( C2U( "CharOverlineHasColor" ),    C2U("CharOverlineHasColor") )
        ( C2U( "CharWeight" ),              C2U("CharWeight") )
        ( C2U( "CharWeightAsian" ),         C2U("CharWeightAsian") )
        ( C2U( "CharWeightComplex" ),       C2U("CharWeightComplex") )
        ( C2U( "CharWordMode" ),            C2U("CharWordMode") )

        ( C2U( "WritingMode" ),             C2U("WritingMode") )

//      ( C2U( "RubyText" ),                C2U("RubyText") )
//      ( C2U( "RubyAdjust" ),              C2U("RubyAdjust") )
//      ( C2U( "RubyCharStyleName" ),       C2U("RubyStyleName") )
//      ( C2U( "RubyIsAbove" ),             C2U("RubyIsAbove") )
        ( C2U( "ParaIsCharacterDistance" ), C2U("ParaIsCharacterDistance") )
        ;
    return m_aShapePropertyMapForCharacterProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForParagraphProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForParagraphProperties =
        tMakePropertyNameMap
        ( C2U( "ParaAdjust" ),          C2U("ParaAdjust") )
        ( C2U( "ParaBottomMargin" ),    C2U("ParaBottomMargin") )
        ( C2U( "ParaIsHyphenation" ),   C2U("ParaIsHyphenation") )
        ( C2U( "ParaLastLineAdjust" ),  C2U("ParaLastLineAdjust") )
        ( C2U( "ParaLeftMargin" ),      C2U("ParaLeftMargin") )
        ( C2U( "ParaRightMargin" ),     C2U("ParaRightMargin") )
        ( C2U( "ParaTopMargin" ),       C2U("ParaTopMargin") )
        ;
    return m_aShapePropertyMapForParagraphProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFillProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForFillProperties =
        tMakePropertyNameMap
        ( C2U( "FillBackground" ),        C2U( "FillBackground" ) )
        ( C2U( "FillBitmapName" ),        C2U( "FillBitmapName" ) )
        ( C2U( "FillColor" ),             C2U( "FillColor" ) )
        ( C2U( "FillGradientName" ),      C2U( "FillGradientName" ) )
        ( C2U( "FillGradientStepCount" ), C2U( "FillGradientStepCount" ) )
        ( C2U( "FillHatchName" ),         C2U( "FillHatchName" ) )
        ( C2U( "FillStyle" ),             C2U( "FillStyle" ) )
        ( C2U( "FillTransparence" ),      C2U( "FillTransparence" ) )
        ( C2U( "FillTransparenceGradientName" ), C2U("FillTransparenceGradientName") )
        //bitmap properties
        ( C2U( "FillBitmapMode" ),        C2U( "FillBitmapMode" ) )
        ( C2U( "FillBitmapSizeX" ),       C2U( "FillBitmapSizeX" ) )
        ( C2U( "FillBitmapSizeY" ),       C2U( "FillBitmapSizeY" ) )
        ( C2U( "FillBitmapLogicalSize" ), C2U( "FillBitmapLogicalSize" ) )
        ( C2U( "FillBitmapOffsetX" ),     C2U( "FillBitmapOffsetX" ) )
        ( C2U( "FillBitmapOffsetY" ),     C2U( "FillBitmapOffsetY" ) )
        ( C2U( "FillBitmapRectanglePoint" ),C2U( "FillBitmapRectanglePoint" ) )
        ( C2U( "FillBitmapPositionOffsetX" ),C2U( "FillBitmapPositionOffsetX" ) )
        ( C2U( "FillBitmapPositionOffsetY" ),C2U( "FillBitmapPositionOffsetY" ) )
        ;
    return m_aShapePropertyMapForFillProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForLineProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForLineProperties =
        tMakePropertyNameMap
        ( C2U( "LineColor" ),             C2U( "LineColor" ) )
        ( C2U( "LineDashName" ),          C2U( "LineDashName" ) )
        ( C2U( "LineJoint" ),             C2U( "LineJoint" ) )
        ( C2U( "LineStyle" ),             C2U( "LineStyle" ) )
        ( C2U( "LineTransparence" ),      C2U( "LineTransparence" ) )
        ( C2U( "LineWidth" ),             C2U( "LineWidth" ) )
        ;
    return m_aShapePropertyMapForLineProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFillAndLineProperties()
{
    static tMakePropertyNameMap m_aShapePropertyMapForFillAndLineProperties =
        tMakePropertyNameMap
        ( PropertyMapper::getPropertyNameMapForFillProperties() )
        ( PropertyMapper::getPropertyNameMapForLineProperties() )
        ;

    return m_aShapePropertyMapForFillAndLineProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForTextShapeProperties()
{
    static tMakePropertyNameMap m_aShapePropertyMapForTextShapeProperties =
        tMakePropertyNameMap
        ( PropertyMapper::getPropertyNameMapForCharacterProperties() )
        ( PropertyMapper::getPropertyNameMapForFillProperties() )
        ( PropertyMapper::getPropertyNameMapForLineProperties() )
//         ( PropertyMapper::getPropertyNameMapForParagraphProperties() )
        // some text properties
//         ( C2U( "TextHorizontalAdjust" ),   C2U( "TextHorizontalAdjust" ) )
//         ( C2U( "TextVerticalAdjust" ),     C2U( "TextVerticalAdjust" ) )
//         ( C2U( "TextAutoGrowHeight" ),     C2U( "TextAutoGrowHeight" ) )
//         ( C2U( "TextAutoGrowWidth" ),      C2U( "TextAutoGrowWidth" ) )
//         ( C2U( "TextLeftDistance" ),       C2U( "TextLeftDistance" ) )
//         ( C2U( "TextRightDistance" ),      C2U( "TextRightDistance" ) )
//         ( C2U( "TextUpperDistance" ),      C2U( "TextUpperDistance" ) )
//         ( C2U( "TextLowerDistance" ),      C2U( "TextLowerDistance" ) )
        ;

    return m_aShapePropertyMapForTextShapeProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForLineSeriesProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForLineSeriesProperties =
        tMakePropertyNameMap
        ( C2U( "LineColor" ),           C2U("Color") )
        ( C2U( "LineDashName" ),        C2U("LineDashName") )
//      ( C2U( "LineJoint" ),           C2U("LineJoint") )
        ( C2U( "LineStyle" ),           C2U("LineStyle") )
        ( C2U( "LineTransparence" ),    C2U("Transparency") )
        ( C2U( "LineWidth" ),           C2U("LineWidth") )

        ;
    return m_aShapePropertyMapForLineSeriesProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFilledSeriesProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForFilledSeriesProperties =
        tMakePropertyNameMap
        ( C2U( "FillBackground"),       C2U("FillBackground") )
        ( C2U( "FillBitmapName" ),      C2U("FillBitmapName") )
        ( C2U( "FillColor" ),           C2U("Color") )
        ( C2U( "FillGradientName" ),    C2U("GradientName") )
        ( C2U( "FillGradientStepCount" ), C2U( "GradientStepCount" ) )
        ( C2U( "FillHatchName" ),       C2U("HatchName") )
        ( C2U( "FillStyle" ),           C2U("FillStyle") )
        ( C2U( "FillTransparence" ),    C2U("Transparency") )
        ( C2U( "FillTransparenceGradientName" ), C2U("TransparencyGradientName") )
        //bitmap properties
        ( C2U( "FillBitmapMode" ),        C2U( "FillBitmapMode" ) )
        ( C2U( "FillBitmapSizeX" ),       C2U( "FillBitmapSizeX" ) )
        ( C2U( "FillBitmapSizeY" ),       C2U( "FillBitmapSizeY" ) )
        ( C2U( "FillBitmapLogicalSize" ), C2U( "FillBitmapLogicalSize" ) )
        ( C2U( "FillBitmapOffsetX" ),     C2U( "FillBitmapOffsetX" ) )
        ( C2U( "FillBitmapOffsetY" ),     C2U( "FillBitmapOffsetY" ) )
        ( C2U( "FillBitmapRectanglePoint" ),C2U( "FillBitmapRectanglePoint" ) )
        ( C2U( "FillBitmapPositionOffsetX" ),C2U( "FillBitmapPositionOffsetX" ) )
        ( C2U( "FillBitmapPositionOffsetY" ),C2U( "FillBitmapPositionOffsetY" ) )
        //line properties
        ( C2U( "LineColor" ),           C2U("BorderColor") )
        ( C2U( "LineDashName" ),        C2U("BorderDashName") )
//      ( C2U( "LineJoint" ),           C2U("LineJoint") )
        ( C2U( "LineStyle" ),           C2U("BorderStyle") )
        ( C2U( "LineTransparence" ),    C2U("BorderTransparency") )
        ( C2U( "LineWidth" ),           C2U("BorderWidth") )
        ;
    return m_aShapePropertyMapForFilledSeriesProperties;
}

void PropertyMapper::setMultiProperties(
                  const tNameSequence& rNames
                , const tAnySequence&  rValues
                , const ::com::sun::star::uno::Reference<
                  ::com::sun::star::beans::XPropertySet >& xTarget )
{
    bool bSuccess = false;
    try
    {
        uno::Reference< beans::XMultiPropertySet > xShapeMultiProp( xTarget, uno::UNO_QUERY );
        if( xShapeMultiProp.is() )
        {
            xShapeMultiProp->setPropertyValues( rNames, rValues );
            bSuccess = true;
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e ); //if this occurs more often think of removing the XMultiPropertySet completly for better performance
    }

    if(!bSuccess)
    try
    {
        sal_Int32 nCount = std::max( rNames.getLength(), rValues.getLength() );
        rtl::OUString aPropName;
        uno::Any aValue;
        for( sal_Int32 nN = 0; nN < nCount; nN++ )
        {
            aPropName = rNames[nN];
            aValue = rValues[nN];

            try
            {
                xTarget->setPropertyValue( aPropName, aValue );
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void PropertyMapper::getTextLabelMultiPropertyLists(
    const uno::Reference< beans::XPropertySet >& xSourceProp
    , tNameSequence& rPropNames, tAnySequence& rPropValues
    , bool bName
    , sal_Int32 nLimitedSpace
    , bool bLimitedHeight )
{
    //fill character properties into the ValueMap
    tPropertyNameValueMap aValueMap;
    PropertyMapper::getValueMap( aValueMap
            , PropertyMapper::getPropertyNameMapForCharacterProperties()
            , xSourceProp );

    //some more shape properties apart from character properties, position-matrix and label string
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("LineStyle"), uno::makeAny(drawing::LineStyle_NONE) ) ); // drawing::LineStyle
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextHorizontalAdjust"), uno::makeAny(drawing::TextHorizontalAdjust_CENTER) ) ); // drawing::TextHorizontalAdjust - needs to be overwritten
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextVerticalAdjust"), uno::makeAny(drawing::TextVerticalAdjust_CENTER) ) ); //drawing::TextVerticalAdjust - needs to be overwritten
    //aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextWritingMode"), uno::makeAny(eWritingMode) ) ); //text::WritingMode
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowHeight"), uno::makeAny(sal_True) ) ); // sal_Bool
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowWidth"), uno::makeAny(sal_True) ) ); // sal_Bool
    if( bName )
        aValueMap.insert( tPropertyNameValueMap::value_type( C2U("Name"), uno::makeAny( rtl::OUString() ) ) ); //CID rtl::OUString - needs to be overwritten for each point

    if( nLimitedSpace > 0 )
    {
        if(bLimitedHeight)
            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextMaximumFrameHeight"), uno::makeAny(nLimitedSpace) ) ); //sal_Int32
        else
            aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextMaximumFrameWidth"), uno::makeAny(nLimitedSpace) ) ); //sal_Int32
        aValueMap.insert( tPropertyNameValueMap::value_type( C2U("ParaIsHyphenation"), uno::makeAny(sal_True) ) );
    }

    /*
    //@todo ?: add paragraph properties:
    //(uno::makeAny(eParaAdjust)) //ParaAdjust - style::ParagraphAdjust
    //(uno::makeAny( (sal_Bool)rAxisLabelProperties.bLineBreakAllowed )) //ParaIsHyphenation - sal_Bool
    style::ParagraphAdjust eParaAdjust( style::ParagraphAdjust_LEFT );
    if( eHorizontalAdjust == drawing::TextHorizontalAdjust_RIGHT )
        eParaAdjust = style::ParagraphAdjust_RIGHT;
    */

    PropertyMapper::getMultiPropertyListsFromValueMap( rPropNames, rPropValues, aValueMap );
}

void PropertyMapper::getPreparedTextShapePropertyLists(
    const uno::Reference< beans::XPropertySet >& xSourceProp
    , tNameSequence& rPropNames, tAnySequence& rPropValues )
{
    //fill character, line and fill properties into the ValueMap
    tPropertyNameValueMap aValueMap;
    PropertyMapper::getValueMap( aValueMap
            , PropertyMapper::getPropertyNameMapForTextShapeProperties()
            , xSourceProp );

    // auto-grow makes sure the shape has the correct size after setting text
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextHorizontalAdjust"), uno::makeAny( drawing::TextHorizontalAdjust_CENTER )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextVerticalAdjust"), uno::makeAny( drawing::TextVerticalAdjust_CENTER )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowHeight"), uno::makeAny( true )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowWidth"), uno::makeAny( true )));

    // set some distance to the border, in case it is shown
    const sal_Int32 nWidthDist  = 250;
    const sal_Int32 nHeightDist = 125;
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextLeftDistance"),  uno::makeAny( nWidthDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextRightDistance"), uno::makeAny( nWidthDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextUpperDistance"), uno::makeAny( nHeightDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextLowerDistance"), uno::makeAny( nHeightDist )));

    // use a line-joint showing the border of thick lines like two rectangles
    // filled in between.
    aValueMap[C2U("LineJoint")] <<= drawing::LineJoint_ROUND;

    PropertyMapper::getMultiPropertyListsFromValueMap( rPropNames, rPropValues, aValueMap );
}

//.............................................................................
} //namespace chart
//.............................................................................
