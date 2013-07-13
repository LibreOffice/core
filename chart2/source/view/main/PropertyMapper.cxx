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

#include "PropertyMapper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>

namespace chart
{
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
        OUString aTarget = aIt->first;
        OUString aSource = aIt->second;
        try
        {
            uno::Any aAny( xSourceProp->getPropertyValue(aSource) );
            if( aAny.hasValue() )
                rValueMap.insert( tPropertyNameValueMap::value_type( aTarget, aAny ) );
        }
        catch( const uno::Exception& e )
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
                         , const OUString& rPropName )
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
        , bLimitedHeight ? OUString("TextMaximumFrameHeight") : OUString("TextMaximumFrameWidth") );
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForCharacterProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForCharacterProperties =
        tMakePropertyNameMap
        ( "CharColor",                "CharColor" )
        ( "CharContoured",            "CharContoured" )
        ( "CharEmphasis",             "CharEmphasis" )//the service style::CharacterProperties  describes a property called 'CharEmphasize' which is nowhere implemented

        ( "CharFontFamily",           "CharFontFamily" )
        ( "CharFontFamilyAsian",      "CharFontFamilyAsian" )
        ( "CharFontFamilyComplex",    "CharFontFamilyComplex" )
        ( "CharFontCharSet",          "CharFontCharSet" )
        ( "CharFontCharSetAsian",     "CharFontCharSetAsian" )
        ( "CharFontCharSetComplex",   "CharFontCharSetComplex" )
        ( "CharFontName",             "CharFontName" )
        ( "CharFontNameAsian",        "CharFontNameAsian" )
        ( "CharFontNameComplex",      "CharFontNameComplex" )
        ( "CharFontPitch",            "CharFontPitch" )
        ( "CharFontPitchAsian",       "CharFontPitchAsian" )
        ( "CharFontPitchComplex",     "CharFontPitchComplex" )
        ( "CharFontStyleName",        "CharFontStyleName" )
        ( "CharFontStyleNameAsian",   "CharFontStyleNameAsian" )
        ( "CharFontStyleNameComplex", "CharFontStyleNameComplex" )

        ( "CharHeight",               "CharHeight" )
        ( "CharHeightAsian",          "CharHeightAsian" )
        ( "CharHeightComplex",        "CharHeightComplex" )
        ( "CharKerning",              "CharKerning" )
        ( "CharLocale",               "CharLocale" )
        ( "CharLocaleAsian",          "CharLocaleAsian" )
        ( "CharLocaleComplex",        "CharLocaleComplex" )
        ( "CharPosture",              "CharPosture" )
        ( "CharPostureAsian",         "CharPostureAsian" )
        ( "CharPostureComplex",       "CharPostureComplex" )
        ( "CharRelief",               "CharRelief" )
        ( "CharShadowed",             "CharShadowed" )
        ( "CharStrikeout",            "CharStrikeout" )
        ( "CharUnderline",            "CharUnderline" )
        ( "CharUnderlineColor",       "CharUnderlineColor" )
        ( "CharUnderlineHasColor",    "CharUnderlineHasColor" )
        ( "CharOverline",             "CharOverline" )
        ( "CharOverlineColor",        "CharOverlineColor" )
        ( "CharOverlineHasColor",     "CharOverlineHasColor" )
        ( "CharWeight",               "CharWeight" )
        ( "CharWeightAsian",          "CharWeightAsian" )
        ( "CharWeightComplex",        "CharWeightComplex" )
        ( "CharWordMode",             "CharWordMode" )

        ( "WritingMode",              "WritingMode" )

        ( "ParaIsCharacterDistance",  "ParaIsCharacterDistance" )
        ;
    return m_aShapePropertyMapForCharacterProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForParagraphProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForParagraphProperties =
        tMakePropertyNameMap
        ( "ParaAdjust",          "ParaAdjust" )
        ( "ParaBottomMargin",    "ParaBottomMargin" )
        ( "ParaIsHyphenation",   "ParaIsHyphenation" )
        ( "ParaLastLineAdjust",  "ParaLastLineAdjust" )
        ( "ParaLeftMargin",      "ParaLeftMargin" )
        ( "ParaRightMargin",     "ParaRightMargin" )
        ( "ParaTopMargin",       "ParaTopMargin" )
        ;
    return m_aShapePropertyMapForParagraphProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFillProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForFillProperties =
        tMakePropertyNameMap
        ( "FillBackground",               "FillBackground" )
        ( "FillBitmapName",               "FillBitmapName" )
        ( "FillColor",                    "FillColor" )
        ( "FillGradientName",             "FillGradientName" )
        ( "FillGradientStepCount",        "FillGradientStepCount" )
        ( "FillHatchName",                "FillHatchName" )
        ( "FillStyle",                    "FillStyle" )
        ( "FillTransparence",             "FillTransparence" )
        ( "FillTransparenceGradientName", "FillTransparenceGradientName" )
        //bitmap properties
        ( "FillBitmapMode",               "FillBitmapMode" )
        ( "FillBitmapSizeX",              "FillBitmapSizeX" )
        ( "FillBitmapSizeY",              "FillBitmapSizeY" )
        ( "FillBitmapLogicalSize",        "FillBitmapLogicalSize" )
        ( "FillBitmapOffsetX",            "FillBitmapOffsetX" )
        ( "FillBitmapOffsetY",            "FillBitmapOffsetY" )
        ( "FillBitmapRectanglePoint",     "FillBitmapRectanglePoint" )
        ( "FillBitmapPositionOffsetX",    "FillBitmapPositionOffsetX" )
        ( "FillBitmapPositionOffsetY",    "FillBitmapPositionOffsetY" )
        ;
    return m_aShapePropertyMapForFillProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForLineProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForLineProperties =
        tMakePropertyNameMap
        ( "LineColor",              "LineColor" )
        ( "LineDashName",           "LineDashName" )
        ( "LineJoint",              "LineJoint" )
        ( "LineStyle",              "LineStyle" )
        ( "LineTransparence",       "LineTransparence" )
        ( "LineWidth",              "LineWidth" )
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
        ( PropertyMapper::getPropertyNameMapForLineProperties() );

    return m_aShapePropertyMapForTextShapeProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForLineSeriesProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForLineSeriesProperties =
        tMakePropertyNameMap
        ( "LineColor",           "Color" )
        ( "LineDashName",        "LineDashName" )
        ( "LineStyle",           "LineStyle" )
        ( "LineTransparence",    "Transparency" )
        ( "LineWidth",           "LineWidth" )

        ;
    return m_aShapePropertyMapForLineSeriesProperties;
}

const tMakePropertyNameMap& PropertyMapper::getPropertyNameMapForFilledSeriesProperties()
{
    //shape property -- chart model object property
    static tMakePropertyNameMap m_aShapePropertyMapForFilledSeriesProperties =
        tMakePropertyNameMap
        ( "FillBackground",               "FillBackground" )
        ( "FillBitmapName",               "FillBitmapName" )
        ( "FillColor",                    "Color" )
        ( "FillGradientName",             "GradientName" )
        ( "FillGradientStepCount",        "GradientStepCount" )
        ( "FillHatchName",                "HatchName" )
        ( "FillStyle",                    "FillStyle" )
        ( "FillTransparence",             "Transparency" )
        ( "FillTransparenceGradientName", "TransparencyGradientName" )
        //bitmap properties
        ( "FillBitmapMode",               "FillBitmapMode" )
        ( "FillBitmapSizeX",              "FillBitmapSizeX" )
        ( "FillBitmapSizeY",              "FillBitmapSizeY" )
        ( "FillBitmapLogicalSize",        "FillBitmapLogicalSize" )
        ( "FillBitmapOffsetX",            "FillBitmapOffsetX" )
        ( "FillBitmapOffsetY",            "FillBitmapOffsetY" )
        ( "FillBitmapRectanglePoint",     "FillBitmapRectanglePoint" )
        ( "FillBitmapPositionOffsetX",    "FillBitmapPositionOffsetX" )
        ( "FillBitmapPositionOffsetY",    "FillBitmapPositionOffsetY" )
        //line properties
        ( "LineColor",                    "BorderColor" )
        ( "LineDashName",                 "BorderDashName" )
        ( "LineStyle",                    "BorderStyle" )
        ( "LineTransparence",             "BorderTransparency" )
        ( "LineWidth",                    "BorderWidth" )
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
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e ); //if this occurs more often think of removing the XMultiPropertySet completely for better performance
    }

    if(!bSuccess)
    try
    {
        sal_Int32 nCount = std::max( rNames.getLength(), rValues.getLength() );
        OUString aPropName;
        uno::Any aValue;
        for( sal_Int32 nN = 0; nN < nCount; nN++ )
        {
            aPropName = rNames[nN];
            aValue = rValues[nN];

            try
            {
                xTarget->setPropertyValue( aPropName, aValue );
            }
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }
    catch( const uno::Exception& e )
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
    aValueMap.insert( tPropertyNameValueMap::value_type( "LineStyle", uno::makeAny(drawing::LineStyle_NONE) ) ); // drawing::LineStyle
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextHorizontalAdjust", uno::makeAny(drawing::TextHorizontalAdjust_CENTER) ) ); // drawing::TextHorizontalAdjust - needs to be overwritten
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextVerticalAdjust", uno::makeAny(drawing::TextVerticalAdjust_CENTER) ) ); //drawing::TextVerticalAdjust - needs to be overwritten
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowHeight", uno::makeAny(sal_True) ) ); // sal_Bool
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowWidth", uno::makeAny(sal_True) ) ); // sal_Bool
    if( bName )
        aValueMap.insert( tPropertyNameValueMap::value_type( "Name", uno::makeAny( OUString() ) ) ); //CID OUString - needs to be overwritten for each point

    if( nLimitedSpace > 0 )
    {
        if(bLimitedHeight)
            aValueMap.insert( tPropertyNameValueMap::value_type( "TextMaximumFrameHeight", uno::makeAny(nLimitedSpace) ) ); //sal_Int32
        else
            aValueMap.insert( tPropertyNameValueMap::value_type( "TextMaximumFrameWidth", uno::makeAny(nLimitedSpace) ) ); //sal_Int32
        aValueMap.insert( tPropertyNameValueMap::value_type( "ParaIsHyphenation", uno::makeAny(sal_True) ) );
    }

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
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextHorizontalAdjust", uno::makeAny( drawing::TextHorizontalAdjust_CENTER )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextVerticalAdjust", uno::makeAny( drawing::TextVerticalAdjust_CENTER )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowHeight", uno::makeAny( true )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowWidth", uno::makeAny( true )));

    // set some distance to the border, in case it is shown
    const sal_Int32 nWidthDist  = 250;
    const sal_Int32 nHeightDist = 125;
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextLeftDistance",  uno::makeAny( nWidthDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextRightDistance", uno::makeAny( nWidthDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextUpperDistance", uno::makeAny( nHeightDist )));
    aValueMap.insert( tPropertyNameValueMap::value_type( "TextLowerDistance", uno::makeAny( nHeightDist )));

    // use a line-joint showing the border of thick lines like two rectangles
    // filled in between.
    aValueMap["LineJoint"] <<= drawing::LineJoint_ROUND;

    PropertyMapper::getMultiPropertyListsFromValueMap( rPropNames, rPropValues, aValueMap );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
