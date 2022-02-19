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

#include <PropertyMapper.hxx>
#include <unonames.hxx>

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <tools/diagnose_ex.h>
#include <svx/unoshape.hxx>
#include <CharacterProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <TitleProperties.hxx>
#include <LabelProperties.hxx>
#include <DataPointProperties.hxx>

using namespace chart::LinePropertiesHelper;
using namespace chart::CharacterProperties;
using namespace chart::FillProperties;
using namespace chart::TitleProperties;
using namespace chart::LabelProperties;
using namespace chart::DataPointProperties;

namespace chart
{
using namespace ::com::sun::star;

void PropertyMapper::setMappedProperties(
          SvxShape& xTarget
        , const uno::Reference< beans::XPropertySet >& xSlowSource
        , const tPropertyNameMap2& rMap )
{
    if( !xSlowSource.is() )
        return;

    /**
     * Ideally we could change this function to take XFastPropertySet.
     * But.... that would change signatures all over the place, and everywhere else
     * really wants to use XPropertySet, and we have no way of making a type signature
     * for XPropertySet UNION XFastPropertySet.
     */

    uno::Reference< beans::XFastPropertySet > xSource(xSlowSource, uno::UNO_QUERY);
    assert(xSource && "we assume that everything supports XFastPropertySet here");

    sal_Int32 nPropertyCount = rMap.size();
    tNameSequence aNames(nPropertyCount);
    tAnySequence  aValues(nPropertyCount);
    auto pNames = aNames.getArray();
    auto pValues = aValues.getArray();
    sal_Int32 nN=0;

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        sal_Int32 rSource = elem.second;
        try
        {
            uno::Any aAny( xSource->getFastPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                pNames[nN]  = rTarget;
                pValues[nN] = std::move(aAny);
                ++nN;
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    if (nN == 0)
        return;
    //reduce to real property count
    aNames.realloc(nN);
    aValues.realloc(nN);

    uno::Reference< beans::XMultiPropertySet > xShapeMultiProp( xTarget, uno::UNO_QUERY_THROW );
    try
    {
        xShapeMultiProp->setPropertyValues( aNames, aValues );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" ); //if this occurs more often think of removing the XMultiPropertySet completely for better performance
    }
}

void PropertyMapper::setMappedProperties(
          const uno::Reference< beans::XPropertySet >& xTarget
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    if( !xTarget.is() || !xSource.is() )
        return;

    tNameSequence aNames;
    tAnySequence  aValues;
    sal_Int32 nN=0;
    sal_Int32 nPropertyCount = rMap.size();
    aNames.realloc(nPropertyCount);
    auto pNames = aNames.getArray();
    aValues.realloc(nPropertyCount);
    auto pValues = aValues.getArray();

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        const OUString & rSource = elem.second;
        try
        {
            uno::Any aAny( xSource->getPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                pNames[nN]  = rTarget;
                pValues[nN] = aAny;
                ++nN;
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    if (nN == 0)
        return;

    uno::Reference< beans::XMultiPropertySet > xShapeMultiProp( xTarget, uno::UNO_QUERY );
    if (xShapeMultiProp)
        try
        {
            //reduce to real property count
            aNames.realloc(nN);
            aValues.realloc(nN);
            xShapeMultiProp->setPropertyValues( aNames, aValues );
            return; // successful
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" ); //if this occurs more often think of removing the XMultiPropertySet completely for better performance
        }

    // fall back to one at a time
    try
    {
        for( sal_Int32 i = 0; i < nN; i++ )
        {
            try
            {
                xTarget->setPropertyValue( aNames[i], aValues[i] );
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void PropertyMapper::setMappedProperties(
          const uno::Reference< beans::XPropertySet >& xTarget
        , const uno::Reference< beans::XPropertySet >& xSlowSource
        , const tPropertyNameMap2& rMap )
{
    if( !xTarget.is() || !xSlowSource.is() )
        return;

    /**
     * Ideally we could change this function to take XFastPropertySet.
     * But.... that would change signatures all over the place, and everywhere else
     * really wants to use XPropertySet, and we have no way of making a type signature
     * for XPropertySet UNION XFastPropertySet.
     */

    uno::Reference< beans::XFastPropertySet > xSource(xSlowSource, uno::UNO_QUERY);
    assert(xSource && "we assume that everything supports XFastPropertySet here");

    tNameSequence aNames;
    tAnySequence  aValues;
    sal_Int32 nN=0;
    sal_Int32 nPropertyCount = rMap.size();
    aNames.realloc(nPropertyCount);
    auto pNames = aNames.getArray();
    aValues.realloc(nPropertyCount);
    auto pValues = aValues.getArray();

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        const sal_Int32 & rSource = elem.second;
        try
        {
            uno::Any aAny( xSource->getFastPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                pNames[nN]  = rTarget;
                pValues[nN] = std::move(aAny);
                ++nN;
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    if (nN == 0)
        return;

    uno::Reference< beans::XMultiPropertySet > xShapeMultiProp( xTarget, uno::UNO_QUERY );
    if (xShapeMultiProp)
        try
        {
            //reduce to real property count
            aNames.realloc(nN);
            aValues.realloc(nN);
            xShapeMultiProp->setPropertyValues( aNames, aValues );
            return; // successful
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" ); //if this occurs more often think of removing the XMultiPropertySet completely for better performance
        }

    // fall back to one at a time
    try
    {
        for( sal_Int32 i = 0; i < nN; i++ )
        {
            try
            {
                xTarget->setPropertyValue( aNames[i], aValues[i] );
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void PropertyMapper::getValueMap(
                  tPropertyNameValueMap& rValueMap
                , const tPropertyNameMap& rNameMap
                , const uno::Reference< beans::XPropertySet >& xSourceProp
                )
{
    uno::Reference< beans::XMultiPropertySet > xMultiPropSet(xSourceProp, uno::UNO_QUERY);
    if((false) && xMultiPropSet.is())
    {
        uno::Sequence< OUString > aPropSourceNames(rNameMap.size());
        auto aPropSourceNamesRange = asNonConstRange(aPropSourceNames);
        uno::Sequence< OUString > aPropTargetNames(rNameMap.size());
        auto aPropTargetNamesRange = asNonConstRange(aPropTargetNames);
        sal_Int32 i = 0;
        for (auto const& elem : rNameMap)
        {
            aPropTargetNamesRange[i] = elem.first;
            aPropSourceNamesRange[i] = elem.second;
            ++i;
        }

        uno::Sequence< uno::Any > xValues = xMultiPropSet->getPropertyValues(aPropSourceNames);
        sal_Int32 n = rNameMap.size();
        for(i = 0;i < n; ++i)
        {
            if( xValues[i].hasValue() )
                rValueMap.emplace(  aPropTargetNames[i], xValues[i] );
        }
    }
    else
    {
        for (auto const& elem : rNameMap)
        {
            const OUString & rTarget = elem.first;
            const OUString & rSource = elem.second;
            try
            {
                uno::Any aAny( xSourceProp->getPropertyValue(rSource) );
                if( aAny.hasValue() )
                    rValueMap.emplace(  rTarget, aAny );
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }
}

void PropertyMapper::getValueMap(
                  tPropertyNameValueMap& rValueMap
                , const tPropertyNameMap2& rNameMap
                , const uno::Reference< beans::XPropertySet >& xSlowSourceProp
                )
{
    /**
     * Ideally we could change this function to take XFastPropertySet.
     * But.... that would change signatures all over the place, and everywhere else
     * really wants to use XPropertySet, and we have no way of making a type signature
     * for XPropertySet UNION XFastPropertySet.
     */

    uno::Reference< beans::XFastPropertySet > xSourceProp(xSlowSourceProp, uno::UNO_QUERY);
    assert(xSourceProp && "we assume that everything supports XFastPropertySet here");

    for (auto const& elem : rNameMap)
    {
        const OUString & rTarget = elem.first;
        const sal_Int32 & rSource = elem.second;
        try
        {
            uno::Any aAny( xSourceProp->getFastPropertyValue(rSource) );
            if( aAny.hasValue() )
                rValueMap.emplace(  rTarget, std::move(aAny) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
}

void PropertyMapper::getMultiPropertyListsFromValueMap(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const tPropertyNameValueMap& rValueMap
                )
{
    sal_Int32 nPropertyCount = rValueMap.size();
    rNames.realloc(nPropertyCount);
    auto pNames = rNames.getArray();
    rValues.realloc(nPropertyCount);
    auto pValues = rValues.getArray();

    //fill sequences
    sal_Int32 nN=0;
    for (auto const& elem : rValueMap)
    {
        const uno::Any& rAny = elem.second;
        if( rAny.hasValue() )
        {
            //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
            pNames[nN]  = elem.first;
            pValues[nN] = rAny;
            ++nN;
        }
    }
    //reduce to real property count
    rNames.realloc(nN);
    rValues.realloc(nN);
}

uno::Any* PropertyMapper::getValuePointer( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , std::u16string_view rPropName )
{
    sal_Int32 nCount = rPropNames.getLength();
    for( sal_Int32 nN = 0; nN < nCount; nN++ )
    {
        if(rPropNames[nN] == rPropName)
            return &rPropValues.getArray()[nN];
    }
    return nullptr;
}

uno::Any* PropertyMapper::getValuePointerForLimitedSpace( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , bool bLimitedHeight)
{
    return PropertyMapper::getValuePointer( rPropValues, rPropNames
        , bLimitedHeight ? OUString("TextMaximumFrameHeight") : OUString("TextMaximumFrameWidth") );
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForCharacterProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForCharacterProperties{
        {"CharColor",                PROP_CHAR_COLOR},
        {"CharContoured",            PROP_CHAR_CONTOURED},
        {"CharEmphasis",             PROP_CHAR_EMPHASIS},//the service style::CharacterProperties  describes a property called 'CharEmphasize' which is nowhere implemented

        {"CharFontFamily",           PROP_CHAR_FONT_FAMILY},
        {"CharFontFamilyAsian",      PROP_CHAR_ASIAN_FONT_FAMILY},
        {"CharFontFamilyComplex",    PROP_CHAR_COMPLEX_FONT_FAMILY},
        {"CharFontCharSet",          PROP_CHAR_FONT_CHAR_SET},
        {"CharFontCharSetAsian",     PROP_CHAR_ASIAN_CHAR_SET},
        {"CharFontCharSetComplex",   PROP_CHAR_COMPLEX_CHAR_SET},
        {"CharFontName",             PROP_CHAR_FONT_NAME},
        {"CharFontNameAsian",        PROP_CHAR_ASIAN_FONT_NAME},
        {"CharFontNameComplex",      PROP_CHAR_COMPLEX_FONT_NAME},
        {"CharFontPitch",            PROP_CHAR_FONT_PITCH},
        {"CharFontPitchAsian",       PROP_CHAR_ASIAN_FONT_PITCH},
        {"CharFontPitchComplex",     PROP_CHAR_COMPLEX_FONT_PITCH},
        {"CharFontStyleName",        PROP_CHAR_FONT_STYLE_NAME},
        {"CharFontStyleNameAsian",   PROP_CHAR_ASIAN_FONT_STYLE_NAME},
        {"CharFontStyleNameComplex", PROP_CHAR_COMPLEX_FONT_STYLE_NAME},

        {"CharHeight",               PROP_CHAR_CHAR_HEIGHT},
        {"CharHeightAsian",          PROP_CHAR_ASIAN_CHAR_HEIGHT},
        {"CharHeightComplex",        PROP_CHAR_COMPLEX_CHAR_HEIGHT},
        {"CharKerning",              PROP_CHAR_KERNING},
        {"CharLocale",               PROP_CHAR_LOCALE},
        {"CharLocaleAsian",          PROP_CHAR_ASIAN_LOCALE},
        {"CharLocaleComplex",        PROP_CHAR_COMPLEX_LOCALE},
        {"CharPosture",              PROP_CHAR_POSTURE},
        {"CharPostureAsian",         PROP_CHAR_ASIAN_POSTURE},
        {"CharPostureComplex",       PROP_CHAR_COMPLEX_POSTURE},
        {"CharRelief",               PROP_CHAR_RELIEF},
        {"CharShadowed",             PROP_CHAR_SHADOWED},
        {"CharStrikeout",            PROP_CHAR_STRIKE_OUT},
        {"CharUnderline",            PROP_CHAR_UNDERLINE},
        {"CharUnderlineColor",       PROP_CHAR_UNDERLINE_COLOR},
        {"CharUnderlineHasColor",    PROP_CHAR_UNDERLINE_HAS_COLOR},
        {"CharOverline",             PROP_CHAR_OVERLINE},
        {"CharOverlineColor",        PROP_CHAR_OVERLINE_COLOR},
        {"CharOverlineHasColor",     PROP_CHAR_OVERLINE_HAS_COLOR},
        {"CharWeight",               PROP_CHAR_WEIGHT},
        {"CharWeightAsian",          PROP_CHAR_ASIAN_WEIGHT},
        {"CharWeightComplex",        PROP_CHAR_COMPLEX_WEIGHT},
        {"CharWordMode",             PROP_CHAR_WORD_MODE},

        {"WritingMode",              PROP_WRITING_MODE},

        {"ParaIsCharacterDistance",  PROP_PARA_IS_CHARACTER_DISTANCE}};

    return s_aShapePropertyMapForCharacterProperties;
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForParagraphProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForParagraphProperties{
        {"ParaAdjust",          PROP_TITLE_PARA_ADJUST},
        {"ParaBottomMargin",    PROP_TITLE_PARA_BOTTOM_MARGIN},
        {"ParaIsHyphenation",   PROP_TITLE_PARA_IS_HYPHENATION},
        {"ParaLastLineAdjust",  PROP_TITLE_PARA_LAST_LINE_ADJUST},
        {"ParaLeftMargin",      PROP_TITLE_PARA_LEFT_MARGIN},
        {"ParaRightMargin",     PROP_TITLE_PARA_RIGHT_MARGIN},
        {"ParaTopMargin",       PROP_TITLE_PARA_TOP_MARGIN}};
    return s_aShapePropertyMapForParagraphProperties;
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForFillProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForFillProperties{
        {"FillBackground",               PROP_FILL_BACKGROUND},
        {"FillBitmapName",               PROP_FILL_BITMAP_NAME},
        {"FillColor",                    PROP_FILL_COLOR},
        {"FillGradientName",             PROP_FILL_GRADIENT_NAME},
        {"FillGradientStepCount",        PROP_FILL_GRADIENT_STEPCOUNT},
        {"FillHatchName",                PROP_FILL_HATCH_NAME},
        {"FillStyle",                    PROP_FILL_STYLE},
        {"FillTransparence",             PROP_FILL_TRANSPARENCE},
        {"FillTransparenceGradientName", PROP_FILL_TRANSPARENCE_GRADIENT_NAME},
        //bitmap properties
        {"FillBitmapMode",               PROP_FILL_BITMAP_MODE},
        {"FillBitmapSizeX",              PROP_FILL_BITMAP_SIZEX},
        {"FillBitmapSizeY",              PROP_FILL_BITMAP_SIZEY},
        {"FillBitmapLogicalSize",        PROP_FILL_BITMAP_LOGICALSIZE},
        {"FillBitmapOffsetX",            PROP_FILL_BITMAP_OFFSETX},
        {"FillBitmapOffsetY",            PROP_FILL_BITMAP_OFFSETY},
        {"FillBitmapRectanglePoint",     PROP_FILL_BITMAP_RECTANGLEPOINT},
        {"FillBitmapPositionOffsetX",    PROP_FILL_BITMAP_POSITION_OFFSETX},
        {"FillBitmapPositionOffsetY",    PROP_FILL_BITMAP_POSITION_OFFSETY}};
    return s_aShapePropertyMapForFillProperties;
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForLineProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForLineProperties{
        {"LineColor",              PROP_LINE_COLOR},
        {"LineDashName",           PROP_LINE_DASH_NAME},
        {"LineJoint",              PROP_LINE_JOINT},
        {"LineStyle",              PROP_LINE_STYLE},
        {"LineTransparence",       PROP_LINE_TRANSPARENCE},
        {"LineWidth",              PROP_LINE_WIDTH},
        {"LineCap",                PROP_LINE_CAP}};
    return s_aShapePropertyMapForLineProperties;
}

namespace {
    tPropertyNameMap2 getPropertyNameMapForFillAndLineProperties_() {
        auto map = PropertyMapper::getPropertyNameMapForFillProperties();
        auto const & add
            = PropertyMapper::getPropertyNameMapForLineProperties();
        map.insert(add.begin(), add.end());
        return map;
    }
}
const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForFillAndLineProperties()
{
    static tPropertyNameMap2 s_aShapePropertyMapForFillAndLineProperties
        = getPropertyNameMapForFillAndLineProperties_();
    return s_aShapePropertyMapForFillAndLineProperties;
}

namespace {
    tPropertyNameMap2 getPropertyNameMapForTextShapeProperties_() {
        auto map = PropertyMapper::getPropertyNameMapForCharacterProperties();
        auto const & add1
            = PropertyMapper::getPropertyNameMapForFillProperties();
        map.insert(add1.begin(), add1.end());
        auto const & add2
            = PropertyMapper::getPropertyNameMapForLineProperties();
        map.insert(add2.begin(), add2.end());
        return map;
    }
}
const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForTextShapeProperties()
{
    static tPropertyNameMap2 s_aShapePropertyMapForTextShapeProperties
        = getPropertyNameMapForTextShapeProperties_();
    return s_aShapePropertyMapForTextShapeProperties;
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForLineSeriesProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForLineSeriesProperties{
        {"LineColor",           PROP_FILL_COLOR},
        {"LineDashName",        PROP_LINE_DASH_NAME},
        {"LineStyle",           PROP_LINE_STYLE},
        {"LineTransparence",    PROP_FILL_TRANSPARENCE},
        {"LineWidth",           PROP_LINE_WIDTH},
        {"LineCap",             PROP_LINE_CAP}};
    return s_aShapePropertyMapForLineSeriesProperties;
}

namespace {
    tPropertyNameMap2 getPropertyNameMapForTextLabelProperties_() {
        auto map = PropertyMapper::getPropertyNameMapForCharacterProperties();
        map.insert({
            {"LineStyle", PROP_LABEL_BORDER_STYLE},
            {"LineWidth", PROP_LABEL_BORDER_WIDTH},
            {"LineColor", PROP_LABEL_BORDER_COLOR},
            {"LineTransparence", PROP_LABEL_BORDER_TRANS},
            {"FillStyle", PROP_LABEL_FILL_STYLE},
            {"FillColor", PROP_LABEL_FILL_COLOR},
            {"FillBackground", PROP_LABEL_FILL_BACKGROUND},
            {"FillHatchName", PROP_LABEL_FILL_HATCH_NAME},
            });
                // fix the spelling!
        return map;
    }
}
const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForTextLabelProperties()
{
    // target name (drawing layer) : source name (chart model)
    static tPropertyNameMap2 aMap = getPropertyNameMapForTextLabelProperties_();
    return aMap;
}

const tPropertyNameMap2& PropertyMapper::getPropertyNameMapForFilledSeriesProperties()
{
    //shape property -- chart model object property
    static tPropertyNameMap2 s_aShapePropertyMapForFilledSeriesProperties{
        {"FillBackground",               PROP_FILL_BACKGROUND},
        {"FillBitmapName",               PROP_FILL_BITMAP_NAME},
        {"FillColor",                    PROP_FILL_COLOR},
        {"FillGradientName",             PROP_FILL_GRADIENT_NAME},
        {"FillGradientStepCount",        PROP_FILL_GRADIENT_STEPCOUNT},
        {"FillHatchName",                PROP_FILL_HATCH_NAME},
        {"FillStyle",                    PROP_FILL_STYLE},
        {"FillTransparence",             PROP_FILL_TRANSPARENCE},
        {"FillTransparenceGradientName", PROP_FILL_TRANSPARENCE_GRADIENT_NAME},
        //bitmap properties
        {"FillBitmapMode",               PROP_FILL_BITMAP_MODE},
        {"FillBitmapSizeX",              PROP_FILL_BITMAP_SIZEX},
        {"FillBitmapSizeY",              PROP_FILL_BITMAP_SIZEY},
        {"FillBitmapLogicalSize",        PROP_FILL_BITMAP_LOGICALSIZE},
        {"FillBitmapOffsetX",            PROP_FILL_BITMAP_OFFSETX},
        {"FillBitmapOffsetY",            PROP_FILL_BITMAP_OFFSETY},
        {"FillBitmapRectanglePoint",     PROP_FILL_BITMAP_RECTANGLEPOINT},
        {"FillBitmapPositionOffsetX",    PROP_FILL_BITMAP_POSITION_OFFSETX},
        {"FillBitmapPositionOffsetY",    PROP_FILL_BITMAP_POSITION_OFFSETY},
        //line properties
        {"LineColor",                    PROP_DATAPOINT_BORDER_COLOR},
        {"LineDashName",                 PROP_DATAPOINT_BORDER_DASH_NAME},
        {"LineStyle",                    PROP_DATAPOINT_BORDER_STYLE},
        {"LineTransparence",             PROP_LINE_TRANSPARENCE},
        {"LineWidth",                    PROP_DATAPOINT_BORDER_WIDTH},
        {"LineCap",                      PROP_LINE_CAP}};
    return s_aShapePropertyMapForFilledSeriesProperties;
}

void PropertyMapper::setMultiProperties(
                  const tNameSequence& rNames
                , const tAnySequence&  rValues
                , SvxShape& xTarget )
{
    try
    {
        xTarget.setPropertyValues( rNames, rValues );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" ); //if this occurs more often think of removing the XMultiPropertySet completely for better performance
    }
}

void PropertyMapper::getTextLabelMultiPropertyLists(
    const uno::Reference< beans::XPropertySet >& xSourceProp
    , tNameSequence& rPropNames, tAnySequence& rPropValues
    , bool bName
    , sal_Int32 nLimitedSpace
    , bool bLimitedHeight
    , bool bSupportsLabelBorder)
{
    //fill character properties into the ValueMap
    tPropertyNameValueMap aValueMap;
    tPropertyNameMap2 const & aNameMap = bSupportsLabelBorder ? PropertyMapper::getPropertyNameMapForTextLabelProperties() : getPropertyNameMapForCharacterProperties();

    PropertyMapper::getValueMap(aValueMap, aNameMap, xSourceProp);

    //some more shape properties apart from character properties, position-matrix and label string
    aValueMap.emplace( "TextHorizontalAdjust", uno::Any(drawing::TextHorizontalAdjust_CENTER) ); // drawing::TextHorizontalAdjust - needs to be overwritten
    aValueMap.emplace( "TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_CENTER) ); //drawing::TextVerticalAdjust - needs to be overwritten
    aValueMap.emplace( "TextAutoGrowHeight", uno::Any(true) ); // sal_Bool
    aValueMap.emplace( "TextAutoGrowWidth", uno::Any(true) ); // sal_Bool
    aValueMap.emplace( "ParaAdjust", uno::Any(style::ParagraphAdjust_CENTER) ); // style::ParagraphAdjust_CENTER - needs to be overwritten
    if( bName )
        aValueMap.emplace( "Name", uno::Any( OUString() ) ); //CID OUString - needs to be overwritten for each point

    if( nLimitedSpace > 0 )
    {
        if(bLimitedHeight)
            aValueMap.emplace( "TextMaximumFrameHeight", uno::Any(nLimitedSpace) ); //sal_Int32
        else
            aValueMap.emplace( "TextMaximumFrameWidth", uno::Any(nLimitedSpace) ); //sal_Int32
        aValueMap.emplace( "ParaIsHyphenation", uno::Any(true) );
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
    aValueMap.emplace( "TextHorizontalAdjust", uno::Any( drawing::TextHorizontalAdjust_CENTER ));
    aValueMap.emplace( "TextVerticalAdjust", uno::Any( drawing::TextVerticalAdjust_CENTER ));
    aValueMap.emplace( "TextAutoGrowHeight", uno::Any( true ));
    aValueMap.emplace( "TextAutoGrowWidth", uno::Any( true ));

    // set some distance to the border, in case it is shown
    const sal_Int32 nWidthDist  = 250;
    const sal_Int32 nHeightDist = 125;
    aValueMap.emplace( "TextLeftDistance",  uno::Any( nWidthDist ));
    aValueMap.emplace( "TextRightDistance", uno::Any( nWidthDist ));
    aValueMap.emplace( "TextUpperDistance", uno::Any( nHeightDist ));
    aValueMap.emplace( "TextLowerDistance", uno::Any( nHeightDist ));

    // use a line-joint showing the border of thick lines like two rectangles
    // filled in between.
    aValueMap["LineJoint"] <<= drawing::LineJoint_ROUND;

    PropertyMapper::getMultiPropertyListsFromValueMap( rPropNames, rPropValues, aValueMap );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
