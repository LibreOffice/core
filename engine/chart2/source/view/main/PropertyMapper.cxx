/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>
#include <svx/unoshape.hxx>

namespace chart
{
using namespace ::com::sun::star;

void PropertyMapper::setMappedProperties(
          SvxShape& xTarget
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    if( !xSource.is() )
        return;

    sal_Int32 nPropertyCount = rMap.size();
    tNameSequence aNames(nPropertyCount);
    tAnySequence  aValues(nPropertyCount);
    auto pNames = aNames.getArray();
    auto pValues = aValues.getArray();
    sal_Int32 nN=0;
    uno::Reference< css::beans::XPropertySetInfo > xInfo = xSource->getPropertySetInfo();

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        const OUString & rSource = elem.second;
        if (xInfo->hasPropertyByName(rSource))
        {
            cpo::uno::Any aAny( xSource->getPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                pNames[nN]  = rTarget;
                pValues[nN] = std::move(aAny);
                ++nN;
            }
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
    catch( const cpo::uno::Exception& )
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
    uno::Reference< css::beans::XPropertySetInfo > xInfo = xSource->getPropertySetInfo();

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        const OUString & rSource = elem.second;
        if (xInfo->hasPropertyByName(rSource))
        {
            cpo::uno::Any aAny( xSource->getPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                pNames[nN]  = rTarget;
                pValues[nN] = std::move(aAny);
                ++nN;
            }
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
        catch( const cpo::uno::Exception& )
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
            catch( const cpo::uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }
    catch( const cpo::uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

cpo::uno::Sequence<css::beans::PropertyValue> PropertyMapper::getPropVals(
         const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    std::vector<css::beans::PropertyValue> aRet;
    aRet.reserve(rMap.size());
    uno::Reference< css::beans::XPropertySetInfo > xInfo = xSource->getPropertySetInfo();

    for (auto const& elem : rMap)
    {
        const OUString & rTarget = elem.first;
        const OUString & rSource = elem.second;
        if (xInfo->hasPropertyByName(rSource))
        {
            cpo::uno::Any aAny( xSource->getPropertyValue(rSource) );
            if( aAny.hasValue() )
            {
                //do not set empty anys because of performance (otherwise SdrAttrObj::ItemChange will take much longer)
                css::beans::PropertyValue aPropVal;
                aPropVal.Name = rTarget;
                aPropVal.Value = std::move(aAny);
                aRet.push_back(std::move(aPropVal));
            }
        }
    }
    return comphelper::containerToSequence(aRet);
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
        cpo::uno::Sequence< OUString > aPropSourceNames(rNameMap.size());
        auto aPropSourceNamesRange = asNonConstRange(aPropSourceNames);
        cpo::uno::Sequence< OUString > aPropTargetNames(rNameMap.size());
        auto aPropTargetNamesRange = asNonConstRange(aPropTargetNames);
        sal_Int32 i = 0;
        for (auto const& elem : rNameMap)
        {
            aPropTargetNamesRange[i] = elem.first;
            aPropSourceNamesRange[i] = elem.second;
            ++i;
        }

        cpo::uno::Sequence< cpo::uno::Any > xValues = xMultiPropSet->getPropertyValues(aPropSourceNames);
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
                cpo::uno::Any aAny( xSourceProp->getPropertyValue(rSource) );
                if( aAny.hasValue() )
                    rValueMap.emplace(  rTarget, aAny );
            }
            catch( const cpo::uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
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
        const cpo::uno::Any& rAny = elem.second;
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

cpo::uno::Any* PropertyMapper::getValuePointer( tAnySequence& rPropValues
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

cpo::uno::Any* PropertyMapper::getValuePointerForLimitedSpace( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , bool bLimitedHeight)
{
    return PropertyMapper::getValuePointer( rPropValues, rPropNames
        , bLimitedHeight ? u"TextMaximumFrameHeight"_ustr : u"TextMaximumFrameWidth"_ustr );
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForCharacterProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForCharacterProperties{
        {u"CharColor"_ustr,                u"CharColor"_ustr},
        {u"CharComplexColor"_ustr,         u"CharComplexColor"_ustr},
        {u"CharContoured"_ustr,            u"CharContoured"_ustr},
        {u"CharEmphasis"_ustr,             u"CharEmphasis"_ustr},//the service style::CharacterProperties  describes a property called 'CharEmphasize' which is nowhere implemented
        {u"CharEscapement"_ustr,           u"CharEscapement"_ustr},
        {u"CharEscapementHeight"_ustr,     u"CharEscapementHeight"_ustr},
        {u"CharFontFamily"_ustr,           u"CharFontFamily"_ustr},
        {u"CharFontFamilyAsian"_ustr,      u"CharFontFamilyAsian"_ustr},
        {u"CharFontFamilyComplex"_ustr,    u"CharFontFamilyComplex"_ustr},
        {u"CharFontCharSet"_ustr,          u"CharFontCharSet"_ustr},
        {u"CharFontCharSetAsian"_ustr,     u"CharFontCharSetAsian"_ustr},
        {u"CharFontCharSetComplex"_ustr,   u"CharFontCharSetComplex"_ustr},
        {u"CharFontName"_ustr,             u"CharFontName"_ustr},
        {u"CharFontNameAsian"_ustr,        u"CharFontNameAsian"_ustr},
        {u"CharFontNameComplex"_ustr,      u"CharFontNameComplex"_ustr},
        {u"CharFontPitch"_ustr,            u"CharFontPitch"_ustr},
        {u"CharFontPitchAsian"_ustr,       u"CharFontPitchAsian"_ustr},
        {u"CharFontPitchComplex"_ustr,     u"CharFontPitchComplex"_ustr},
        {u"CharFontStyleName"_ustr,        u"CharFontStyleName"_ustr},
        {u"CharFontStyleNameAsian"_ustr,   u"CharFontStyleNameAsian"_ustr},
        {u"CharFontStyleNameComplex"_ustr, u"CharFontStyleNameComplex"_ustr},

        {u"CharHeight"_ustr,               u"CharHeight"_ustr},
        {u"CharHeightAsian"_ustr,          u"CharHeightAsian"_ustr},
        {u"CharHeightComplex"_ustr,        u"CharHeightComplex"_ustr},
        {u"CharKerning"_ustr,              u"CharKerning"_ustr},
        {u"CharLocale"_ustr,               u"CharLocale"_ustr},
        {u"CharLocaleAsian"_ustr,          u"CharLocaleAsian"_ustr},
        {u"CharLocaleComplex"_ustr,        u"CharLocaleComplex"_ustr},
        {u"CharPosture"_ustr,              u"CharPosture"_ustr},
        {u"CharPostureAsian"_ustr,         u"CharPostureAsian"_ustr},
        {u"CharPostureComplex"_ustr,       u"CharPostureComplex"_ustr},
        {u"CharRelief"_ustr,               u"CharRelief"_ustr},
        {u"CharShadowed"_ustr,             u"CharShadowed"_ustr},
        {u"CharStrikeout"_ustr,            u"CharStrikeout"_ustr},
        {u"CharUnderline"_ustr,            u"CharUnderline"_ustr},
        {u"CharUnderlineColor"_ustr,       u"CharUnderlineColor"_ustr},
        {u"CharUnderlineComplexColor"_ustr, u"CharUnderlineComplexColor"_ustr},
        {u"CharUnderlineHasColor"_ustr,    u"CharUnderlineHasColor"_ustr},
        {u"CharOverline"_ustr,             u"CharOverline"_ustr},
        {u"CharOverlineColor"_ustr,        u"CharOverlineColor"_ustr},
        {u"CharOverlineComplexColor"_ustr, u"CharOverlineComplexColor"_ustr},
        {u"CharOverlineHasColor"_ustr,     u"CharOverlineHasColor"_ustr},
        {u"CharWeight"_ustr,               u"CharWeight"_ustr},
        {u"CharWeightAsian"_ustr,          u"CharWeightAsian"_ustr},
        {u"CharWeightComplex"_ustr,        u"CharWeightComplex"_ustr},
        {u"CharWordMode"_ustr,             u"CharWordMode"_ustr},

        {u"WritingMode"_ustr,              u"WritingMode"_ustr},

        {u"ParaIsCharacterDistance"_ustr,  u"ParaIsCharacterDistance"_ustr}};

    return s_aShapePropertyMapForCharacterProperties;
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForParagraphProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForParagraphProperties{
        {u"ParaAdjust"_ustr,          u"ParaAdjust"_ustr},
        {u"ParaBottomMargin"_ustr,    u"ParaBottomMargin"_ustr},
        {u"ParaIsHyphenation"_ustr,   u"ParaIsHyphenation"_ustr},
        {u"ParaLastLineAdjust"_ustr,  u"ParaLastLineAdjust"_ustr},
        {u"ParaLeftMargin"_ustr,      u"ParaLeftMargin"_ustr},
        {u"ParaRightMargin"_ustr,     u"ParaRightMargin"_ustr},
        {u"ParaTopMargin"_ustr,       u"ParaTopMargin"_ustr}};
    return s_aShapePropertyMapForParagraphProperties;
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForFillProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForFillProperties{
        {u"FillBackground"_ustr,               u"FillBackground"_ustr},
        {u"FillBitmapName"_ustr,               u"FillBitmapName"_ustr},
        {u"FillColor"_ustr,                    u"FillColor"_ustr},
        {u"FillComplexColor"_ustr,             u"FillComplexColor"_ustr},
        {u"FillGradientName"_ustr,             u"FillGradientName"_ustr},
        {u"FillGradientStepCount"_ustr,        u"FillGradientStepCount"_ustr},
        {u"FillHatchName"_ustr,                u"FillHatchName"_ustr},
        {u"FillStyle"_ustr,                    u"FillStyle"_ustr},
        {u"FillTransparence"_ustr,             u"FillTransparence"_ustr},
        {u"FillTransparenceGradientName"_ustr, u"FillTransparenceGradientName"_ustr},

        {u"GlowEffectColor"_ustr,              u"GlowEffectColor"_ustr},
        {u"GlowEffectRadius"_ustr,             u"GlowEffectRadius"_ustr},
        {u"GlowEffectTransparency"_ustr,       u"GlowEffectTransparency"_ustr},
        {u"SoftEdgeRadius"_ustr,               u"SoftEdgeRadius"_ustr},

        //bitmap properties
        {u"FillBitmapMode"_ustr,               u"FillBitmapMode"_ustr},
        {u"FillBitmapSizeX"_ustr,              u"FillBitmapSizeX"_ustr},
        {u"FillBitmapSizeY"_ustr,              u"FillBitmapSizeY"_ustr},
        {u"FillBitmapLogicalSize"_ustr,        u"FillBitmapLogicalSize"_ustr},
        {u"FillBitmapOffsetX"_ustr,            u"FillBitmapOffsetX"_ustr},
        {u"FillBitmapOffsetY"_ustr,            u"FillBitmapOffsetY"_ustr},
        {u"FillBitmapRectanglePoint"_ustr,     u"FillBitmapRectanglePoint"_ustr},
        {u"FillBitmapPositionOffsetX"_ustr,    u"FillBitmapPositionOffsetX"_ustr},
        {u"FillBitmapPositionOffsetY"_ustr,    u"FillBitmapPositionOffsetY"_ustr}};
    return s_aShapePropertyMapForFillProperties;
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForLineProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForLineProperties{
        {u"LineColor"_ustr,              u"LineColor"_ustr},
        {u"LineComplexColor"_ustr,       u"LineComplexColor"_ustr},
        {u"LineDashName"_ustr,           u"LineDashName"_ustr},
        {u"LineJoint"_ustr,              u"LineJoint"_ustr},
        {u"LineStyle"_ustr,              u"LineStyle"_ustr},
        {u"LineTransparence"_ustr,       u"LineTransparence"_ustr},
        {u"LineWidth"_ustr,              u"LineWidth"_ustr},
        {u"LineCap"_ustr,                u"LineCap"_ustr}};
    return s_aShapePropertyMapForLineProperties;
}

namespace {
    tPropertyNameMap getPropertyNameMapForFillAndLineProperties_() {
        auto map = PropertyMapper::getPropertyNameMapForFillProperties();
        auto const & add
            = PropertyMapper::getPropertyNameMapForLineProperties();
        map.insert(add.begin(), add.end());
        return map;
    }
}
const tPropertyNameMap& PropertyMapper::getPropertyNameMapForFillAndLineProperties()
{
    static tPropertyNameMap s_aShapePropertyMapForFillAndLineProperties
        = getPropertyNameMapForFillAndLineProperties_();
    return s_aShapePropertyMapForFillAndLineProperties;
}

namespace {
    tPropertyNameMap getPropertyNameMapForTextShapeProperties_() {
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
const tPropertyNameMap& PropertyMapper::getPropertyNameMapForTextShapeProperties()
{
    static tPropertyNameMap s_aShapePropertyMapForTextShapeProperties
        = getPropertyNameMapForTextShapeProperties_();
    return s_aShapePropertyMapForTextShapeProperties;
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForLineSeriesProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForLineSeriesProperties{
        {u"LineColor"_ustr,           u"Color"_ustr},
        {u"LineComplexColor"_ustr,    u"ComplexColor"_ustr},
        {u"LineDashName"_ustr,        u"LineDashName"_ustr},
        {u"LineStyle"_ustr,           u"LineStyle"_ustr},
        {u"LineTransparence"_ustr,    u"Transparency"_ustr},
        {u"LineWidth"_ustr,           u"LineWidth"_ustr},
        {u"LineCap"_ustr,             u"LineCap"_ustr}};
    return s_aShapePropertyMapForLineSeriesProperties;
}

namespace {
    tPropertyNameMap getPropertyNameMapForTextLabelProperties_() {
        auto map = PropertyMapper::getPropertyNameMapForCharacterProperties();
        map.insert({
            {u"LineStyle"_ustr, CHART_UNONAME_LABEL_BORDER_STYLE},
            {u"LineWidth"_ustr, CHART_UNONAME_LABEL_BORDER_WIDTH},
            {u"LineColor"_ustr, CHART_UNONAME_LABEL_BORDER_COLOR},
            {u"LineComplexColor"_ustr, CHART_UNONAME_LABEL_BORDER_COMPLEX_COLOR},
            {u"LineTransparence"_ustr, CHART_UNONAME_LABEL_BORDER_TRANS},
            {u"FillStyle"_ustr, CHART_UNONAME_LABEL_FILL_STYLE},
            {u"FillColor"_ustr, CHART_UNONAME_LABEL_FILL_COLOR},
            {u"FillComplexColor"_ustr, CHART_UNONAME_LABEL_FILL_COMPLEX_COLOR},
            {u"FillBackground"_ustr, CHART_UNONAME_LABEL_FILL_BACKGROUND},
            {u"FillHatchName"_ustr, CHART_UNONAME_LABEL_FILL_HATCH_NAME}
            });
                // fix the spelling!
        return map;
    }
}
const tPropertyNameMap& PropertyMapper::getPropertyNameMapForTextLabelProperties()
{
    // target name (drawing layer) : source name (chart model)
    static const tPropertyNameMap aMap = getPropertyNameMapForTextLabelProperties_();
    return aMap;
}

const tPropertyNameMap& PropertyMapper::getPropertyNameMapForFilledSeriesProperties()
{
    //shape property -- chart model object property
    static const tPropertyNameMap s_aShapePropertyMapForFilledSeriesProperties{
        {u"FillBackground"_ustr,               u"FillBackground"_ustr},
        {u"FillBitmapName"_ustr,               u"FillBitmapName"_ustr},
        {u"FillColor"_ustr,                    u"Color"_ustr},
        {u"FillComplexColor"_ustr,             u"ComplexColor"_ustr},
        {u"FillGradientName"_ustr,             u"GradientName"_ustr},
        {u"FillGradientStepCount"_ustr,        u"GradientStepCount"_ustr},
        {u"FillHatchName"_ustr,                u"HatchName"_ustr},
        {u"FillStyle"_ustr,                    u"FillStyle"_ustr},
        {u"FillTransparence"_ustr,             u"Transparency"_ustr},
        {u"FillTransparenceGradientName"_ustr, u"TransparencyGradientName"_ustr},
        //bitmap properties
        {u"FillBitmapMode"_ustr,               u"FillBitmapMode"_ustr},
        {u"FillBitmapSizeX"_ustr,              u"FillBitmapSizeX"_ustr},
        {u"FillBitmapSizeY"_ustr,              u"FillBitmapSizeY"_ustr},
        {u"FillBitmapLogicalSize"_ustr,        u"FillBitmapLogicalSize"_ustr},
        {u"FillBitmapOffsetX"_ustr,            u"FillBitmapOffsetX"_ustr},
        {u"FillBitmapOffsetY"_ustr,            u"FillBitmapOffsetY"_ustr},
        {u"FillBitmapRectanglePoint"_ustr,     u"FillBitmapRectanglePoint"_ustr},
        {u"FillBitmapPositionOffsetX"_ustr,    u"FillBitmapPositionOffsetX"_ustr},
        {u"FillBitmapPositionOffsetY"_ustr,    u"FillBitmapPositionOffsetY"_ustr},

        {u"GlowEffectColor"_ustr,              u"GlowEffectColor"_ustr},
        {u"GlowEffectRadius"_ustr,             u"GlowEffectRadius"_ustr},
        {u"GlowEffectTransparency"_ustr,       u"GlowEffectTransparency"_ustr},
        {u"SoftEdgeRadius"_ustr,               u"SoftEdgeRadius"_ustr},

        //line properties
        {u"LineColor"_ustr,                    u"BorderColor"_ustr},
        {u"LineComplexColor"_ustr,             u"BorderComplexColor"_ustr},
        {u"LineDashName"_ustr,                 u"BorderDashName"_ustr},
        {u"LineStyle"_ustr,                    u"BorderStyle"_ustr},
        {u"LineTransparence"_ustr,             u"BorderTransparency"_ustr},
        {u"LineWidth"_ustr,                    u"BorderWidth"_ustr},
        {u"LineCap"_ustr,                      u"LineCap"_ustr}};
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
    catch( const cpo::uno::Exception& )
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
    tPropertyNameMap const & aNameMap = bSupportsLabelBorder ? PropertyMapper::getPropertyNameMapForTextLabelProperties() : getPropertyNameMapForCharacterProperties();

    PropertyMapper::getValueMap(aValueMap, aNameMap, xSourceProp);

    //some more shape properties apart from character properties, position-matrix and label string
    aValueMap.emplace( "TextHorizontalAdjust", cpo::uno::Any(drawing::TextHorizontalAdjust_CENTER) ); // drawing::TextHorizontalAdjust - needs to be overwritten
    aValueMap.emplace( "TextVerticalAdjust", cpo::uno::Any(drawing::TextVerticalAdjust_CENTER) ); //drawing::TextVerticalAdjust - needs to be overwritten
    aValueMap.emplace( "TextAutoGrowHeight", cpo::uno::Any(true) ); // bool
    aValueMap.emplace( "TextAutoGrowWidth", cpo::uno::Any(true) ); // bool
    aValueMap.emplace( "ParaAdjust", cpo::uno::Any(style::ParagraphAdjust_CENTER) ); // style::ParagraphAdjust_CENTER - needs to be overwritten
    if( bName )
        aValueMap.emplace( "Name", cpo::uno::Any( OUString() ) ); //CID OUString - needs to be overwritten for each point

    if( nLimitedSpace > 0 )
    {
        if(bLimitedHeight)
            aValueMap.emplace( "TextMaximumFrameHeight", cpo::uno::Any(nLimitedSpace) ); //sal_Int32
        else
            aValueMap.emplace( "TextMaximumFrameWidth", cpo::uno::Any(nLimitedSpace) ); //sal_Int32
        aValueMap.emplace( "ParaIsHyphenation", cpo::uno::Any(true) );
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
    aValueMap.emplace( "TextHorizontalAdjust", cpo::uno::Any( drawing::TextHorizontalAdjust_CENTER ));
    aValueMap.emplace( "TextVerticalAdjust", cpo::uno::Any( drawing::TextVerticalAdjust_CENTER ));
    aValueMap.emplace( "TextAutoGrowHeight", cpo::uno::Any( true ));
    aValueMap.emplace( "TextAutoGrowWidth", cpo::uno::Any( true ));

    // set some distance to the border, in case it is shown
    const sal_Int32 nWidthDist  = 250;
    const sal_Int32 nHeightDist = 125;
    aValueMap.emplace( "TextLeftDistance",  cpo::uno::Any( nWidthDist ));
    aValueMap.emplace( "TextRightDistance", cpo::uno::Any( nWidthDist ));
    aValueMap.emplace( "TextUpperDistance", cpo::uno::Any( nHeightDist ));
    aValueMap.emplace( "TextLowerDistance", cpo::uno::Any( nHeightDist ));

    // use a line-joint showing the border of thick lines like two rectangles
    // filled in between.
    aValueMap[u"LineJoint"_ustr] <<= drawing::LineJoint_ROUND;

    PropertyMapper::getMultiPropertyListsFromValueMap( rPropNames, rPropValues, aValueMap );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
