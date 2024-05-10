/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <comphelper/sequence.hxx>

#include <drawingml/customshapeproperties.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokenmap.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <o3tl/string_view.hxx>

using namespace ::com::sun::star;

namespace
{
// Parses a string like: Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj"
void lcl_parseAdjustmentValue(
    std::vector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues,
    std::string_view rValue)
{
    sal_Int32 nIndex = 0;
    drawing::EnhancedCustomShapeAdjustmentValue aAdjustmentValue;
    do
    {
        std::string_view aToken(o3tl::trim(o3tl::getToken(rValue, 0, ',', nIndex)));
        static const char aNamePrefix[] = "Name = \"";
        static const char aValuePrefix[] = "Value = (any) { (long) ";
        if (o3tl::starts_with(aToken, aNamePrefix))
        {
            std::string_view aName = aToken.substr(
                strlen(aNamePrefix), aToken.size() - strlen(aNamePrefix) - strlen("\""));
            aAdjustmentValue.Name = OUString::fromUtf8(aName);
        }
        else if (o3tl::starts_with(aToken, aValuePrefix))
        {
            std::string_view aValue = aToken.substr(
                strlen(aValuePrefix), aToken.size() - strlen(aValuePrefix) - strlen(" }"));
            aAdjustmentValue.Value <<= o3tl::toInt32(aValue);
        }
        else if (!o3tl::starts_with(aToken, "State = "))
            SAL_WARN("oox", "lcl_parseAdjustmentValue: unexpected prefix: " << aToken);
    } while (nIndex >= 0);
    rAdjustmentValues.push_back(aAdjustmentValue);
}

// Parses a string like: { Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj" }, { Value = ..., State = ..., Name = ... }
void lcl_parseAdjustmentValues(
    std::vector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues,
    std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
            {
                lcl_parseAdjustmentValue(
                    rAdjustmentValues,
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },")));
            }
        }
    }
}

drawing::EnhancedCustomShapeParameterPair
lcl_parseEnhancedCustomShapeParameterPair(std::string_view rValue)
{
    drawing::EnhancedCustomShapeParameterPair aPair;
    // We expect the following here: First.Value, First.Type, Second.Value, Second.Type
    static const char aExpectedFVPrefix[]
        = "First = (com.sun.star.drawing.EnhancedCustomShapeParameter) { Value = (any) { (long) ";
    assert(o3tl::starts_with(rValue, aExpectedFVPrefix));
    sal_Int32 nIndex = strlen(aExpectedFVPrefix);
    aPair.First.Value
        <<= static_cast<sal_uInt32>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));

    static const char aExpectedFTPrefix[] = ", Type = (short) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedFTPrefix));
    nIndex += strlen(aExpectedFTPrefix);
    aPair.First.Type
        = static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));

    static const char aExpectedSVPrefix[] = ", Second = "
                                            "(com.sun.star.drawing.EnhancedCustomShapeParameter) { "
                                            "Value = (any) { (long) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedSVPrefix));
    nIndex += strlen(aExpectedSVPrefix);
    aPair.Second.Value
        <<= static_cast<sal_uInt32>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));

    static const char aExpectedSTPrefix[] = ", Type = (short) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedSTPrefix));
    nIndex += strlen(aExpectedSTPrefix);
    aPair.Second.Type
        = static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));
    return aPair;
}

drawing::EnhancedCustomShapeSegment lcl_parseEnhancedCustomShapeSegment(std::string_view rValue)
{
    drawing::EnhancedCustomShapeSegment aSegment;
    // We expect the following here: Command, Count
    static const char aExpectedCommandPrefix[] = "Command = (short) ";
    assert(o3tl::starts_with(rValue, aExpectedCommandPrefix));
    sal_Int32 nIndex = strlen(aExpectedCommandPrefix);
    aSegment.Command
        = static_cast<sal_Int16>(o3tl::toInt32(o3tl::getToken(rValue, 0, ',', nIndex)));

    static const char aExpectedCountPrefix[] = " Count = (short) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedCountPrefix));
    nIndex += strlen(aExpectedCountPrefix);
    aSegment.Count = static_cast<sal_Int16>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));
    return aSegment;
}

awt::Rectangle lcl_parseRectangle(std::string_view rValue)
{
    awt::Rectangle aRectangle;
    // We expect the following here: X, Y, Width, Height
    static const char aExpectedXPrefix[] = "X = (long) ";
    assert(o3tl::starts_with(rValue, aExpectedXPrefix));
    sal_Int32 nIndex = strlen(aExpectedXPrefix);
    aRectangle.X = o3tl::toInt32(o3tl::getToken(rValue, 0, ',', nIndex));

    static const char aExpectedYPrefix[] = " Y = (long) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedYPrefix));
    nIndex += strlen(aExpectedYPrefix);
    aRectangle.Y = o3tl::toInt32(o3tl::getToken(rValue, 0, ',', nIndex));

    static const char aExpectedWidthPrefix[] = " Width = (long) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedWidthPrefix));
    nIndex += strlen(aExpectedWidthPrefix);
    aRectangle.Width = o3tl::toInt32(o3tl::getToken(rValue, 0, ',', nIndex));

    static const char aExpectedHeightPrefix[] = " Height = (long) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedHeightPrefix));
    nIndex += strlen(aExpectedHeightPrefix);
    aRectangle.Height = o3tl::toInt32(rValue.substr(nIndex));

    return aRectangle;
}

awt::Size lcl_parseSize(std::string_view rValue)
{
    awt::Size aSize;
    // We expect the following here: Width, Height
    static const char aExpectedWidthPrefix[] = "Width = (long) ";
    assert(o3tl::starts_with(rValue, aExpectedWidthPrefix));
    sal_Int32 nIndex = strlen(aExpectedWidthPrefix);
    aSize.Width = o3tl::toInt32(o3tl::getToken(rValue, 0, ',', nIndex));

    static const char aExpectedHeightPrefix[] = " Height = (long) ";
    assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedHeightPrefix));
    nIndex += strlen(aExpectedHeightPrefix);
    aSize.Height = o3tl::toInt32(rValue.substr(nIndex));

    return aSize;
}

drawing::EnhancedCustomShapeTextFrame lcl_parseEnhancedCustomShapeTextFrame(std::string_view rValue)
{
    drawing::EnhancedCustomShapeTextFrame aTextFrame;
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "TopLeft = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" }"));
                aTextFrame.TopLeft = lcl_parseEnhancedCustomShapeParameterPair(aToken);
            }
            else
                SAL_WARN("oox",
                         "lcl_parseEnhancedCustomShapeTextFrame: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }

    std::string_view aToken = rValue.substr(nStart);
    static const char aExpectedPrefix[]
        = "BottomRight = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
    if (o3tl::starts_with(aToken, aExpectedPrefix))
    {
        aToken = aToken.substr(strlen(aExpectedPrefix),
                               aToken.size() - strlen(aExpectedPrefix) - strlen(" }"));
        aTextFrame.BottomRight = lcl_parseEnhancedCustomShapeParameterPair(aToken);
    }
    else
        SAL_WARN("oox",
                 "lcl_parseEnhancedCustomShapeTextFrame: unexpected token at the end: " << aToken);

    return aTextFrame;
}

// Parses a string like: Name = "Position", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parseHandlePosition(std::vector<beans::PropertyValue>& rHandle, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = "Position";
                aPropertyValue.Value <<= lcl_parseEnhancedCustomShapeParameterPair(aToken);
                rHandle.push_back(aPropertyValue);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parseHandlePosition: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

// Parses a string like: Name = "RangeYMaximum", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parseHandleRange(std::vector<beans::PropertyValue>& rHandle, std::string_view rValue,
                          const OUString& rName)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            static const char aExpectedPrefix[]
                = "Value = (any) { (com.sun.star.drawing.EnhancedCustomShapeParameter) { ";
            if (o3tl::starts_with(rValue.substr(nStart), aExpectedPrefix))
            {
                drawing::EnhancedCustomShapeParameter aParameter;
                sal_Int32 nIndex{ nStart + static_cast<sal_Int32>(strlen(aExpectedPrefix)) };
                // We expect the following here: Value and Type
                static const char aExpectedVPrefix[] = "Value = (any) { (long) ";
                assert(o3tl::starts_with(rValue.substr(nIndex), aExpectedVPrefix));
                nIndex += strlen(aExpectedVPrefix);
                aParameter.Value <<= o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex));

                static const char aExpectedTPrefix[] = ", Type = (short) ";
                assert(nIndex >= 0 && o3tl::starts_with(rValue.substr(nIndex), aExpectedTPrefix));
                nIndex += strlen(aExpectedTPrefix);
                aParameter.Type
                    = static_cast<sal_Int16>(o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex)));

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = rName;
                aPropertyValue.Value <<= aParameter;
                rHandle.push_back(aPropertyValue);
            }
            else if (!o3tl::starts_with(rValue.substr(nStart), "Name =")
                     && !o3tl::starts_with(rValue.substr(nStart), "Handle ="))
                SAL_WARN("oox", "lcl_parseHandleRange: unexpected token: "
                                    << rValue.substr(nStart, i - nStart));
            nStart = i + strlen(", ");
        }
    }
}

// Parses a string like: Name = "RefY", Handle = (long) 0, Value = (any) { (long) 0 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
void lcl_parseHandleRef(std::vector<beans::PropertyValue>& rHandle, std::string_view rValue,
                        const OUString& rName)
{
    static constexpr std::string_view aPrefix = "\", Handle = (long) 0, Value = (any) { (long) ";
    const sal_Int32 nStart = SAL_N_ELEMENTS("Name = \"") - 1 + rName.getLength();

    if (rValue.substr(nStart, aPrefix.size()) == aPrefix)
    {
        sal_Int32 nIndex = nStart + aPrefix.size();
        beans::PropertyValue aPropertyValue;
        aPropertyValue.Name = rName;
        // We only expect a Value here
        aPropertyValue.Value <<= o3tl::toInt32(o3tl::getToken(rValue, 0, '}', nIndex));
        rHandle.push_back(aPropertyValue);
    }
    else
        SAL_WARN("oox", "lcl_parseHandleRef: unexpected value: " << rValue);
}

uno::Sequence<beans::PropertyValue> lcl_parseHandle(std::string_view rValue)
{
    std::vector<beans::PropertyValue> aRet;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
            {
                std::string_view aToken
                    = rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"));
                if (o3tl::starts_with(aToken, "Name = \"Position\""))
                    lcl_parseHandlePosition(aRet, aToken);
                else if (o3tl::starts_with(aToken, "Name = \"RangeXMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RangeXMaximum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RangeXMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RangeXMinimum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RangeYMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RangeYMaximum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RangeYMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RangeYMinimum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RadiusRangeMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RadiusRangeMaximum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RadiusRangeMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, u"RadiusRangeMinimum"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RefX\""))
                    lcl_parseHandleRef(aRet, aToken, u"RefX"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RefY\""))
                    lcl_parseHandleRef(aRet, aToken, u"RefY"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RefR\""))
                    lcl_parseHandleRef(aRet, aToken, u"RefR"_ustr);
                else if (o3tl::starts_with(aToken, "Name = \"RefAngle\""))
                    lcl_parseHandleRef(aRet, aToken, u"RefAngle"_ustr);
                else
                    SAL_WARN("oox", "lcl_parseHandle: unexpected token: " << aToken);
            }
        }
    }
    return comphelper::containerToSequence(aRet);
}

void lcl_parseHandles(std::vector<uno::Sequence<beans::PropertyValue>>& rHandles,
                      std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
            {
                uno::Sequence<beans::PropertyValue> aHandle = lcl_parseHandle(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },")));
                rHandles.push_back(aHandle);
            }
        }
    }
}

void lcl_parseEquations(std::vector<OUString>& rEquations, std::string_view rValue)
{
    bool bInString = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '"' && !bInString)
        {
            nStart = i;
            bInString = true;
        }
        else if (rValue[i] == '"' && bInString)
        {
            bInString = false;
            rEquations.push_back(OUString::fromUtf8(
                rValue.substr(nStart + strlen("\""), i - nStart - strlen("\""))));
        }
    }
}

void lcl_parsePathCoordinateValues(std::vector<beans::PropertyValue>& rPath,
                                   std::string_view rValue)
{
    std::vector<drawing::EnhancedCustomShapeParameterPair> aPairs;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                aPairs.push_back(lcl_parseEnhancedCustomShapeParameterPair(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Coordinates";
    aPropertyValue.Value <<= comphelper::containerToSequence(aPairs);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Coordinates", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathCoordinates(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathCoordinateValues(rPath, aToken);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parsePathCoordinates: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathGluePointsValues(std::vector<beans::PropertyValue>& rPath,
                                   std::string_view rValue)
{
    std::vector<drawing::EnhancedCustomShapeParameterPair> aPairs;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                aPairs.push_back(lcl_parseEnhancedCustomShapeParameterPair(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "GluePoints";
    aPropertyValue.Value <<= comphelper::containerToSequence(aPairs);
    rPath.push_back(aPropertyValue);
}

void lcl_parsePathGluePoints(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathGluePointsValues(rPath, aToken);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parsePathGluePoints: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSegmentValues(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    std::vector<drawing::EnhancedCustomShapeSegment> aSegments;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                aSegments.push_back(lcl_parseEnhancedCustomShapeSegment(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Segments";
    aPropertyValue.Value <<= comphelper::containerToSequence(aSegments);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Segments", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathSegments(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeSegment) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathSegmentValues(rPath, aToken);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parsePathSegments: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathTextFrameValues(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    std::vector<drawing::EnhancedCustomShapeTextFrame> aTextFrames;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                aTextFrames.push_back(lcl_parseEnhancedCustomShapeTextFrame(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "TextFrames";
    aPropertyValue.Value <<= comphelper::containerToSequence(aTextFrames);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "TextFrames", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathTextFrames(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeTextFrame) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathTextFrameValues(rPath, aToken);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parsePathTextFrames: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSubViewSizeValues(std::vector<beans::PropertyValue>& rPath,
                                    std::string_view rValue)
{
    std::vector<awt::Size> aSizes;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                aSizes.push_back(lcl_parseSize(
                    rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "SubViewSize";
    aPropertyValue.Value <<= comphelper::containerToSequence(aSizes);
    rPath.push_back(aPropertyValue);
}

void lcl_parsePathSubViewSize(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                bIgnore = true;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
                bIgnore = false;
        }
        else if (rValue[i] == ',' && !bIgnore)
        {
            std::string_view aToken = rValue.substr(nStart, i - nStart);
            static const char aExpectedPrefix[] = "Value = (any) { ([]com.sun.star.awt.Size) { ";
            if (o3tl::starts_with(aToken, aExpectedPrefix))
            {
                aToken = aToken.substr(strlen(aExpectedPrefix),
                                       aToken.size() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathSubViewSizeValues(rPath, aToken);
            }
            else if (!o3tl::starts_with(aToken, "Name =") && !o3tl::starts_with(aToken, "Handle ="))
                SAL_WARN("oox", "lcl_parsePathSubViewSize: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePath(std::vector<beans::PropertyValue>& rPath, std::string_view rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (size_t i = 0; i < rValue.size(); ++i)
    {
        if (rValue[i] == '{')
        {
            if (!nLevel)
                nStart = i;
            nLevel++;
        }
        else if (rValue[i] == '}')
        {
            nLevel--;
            if (!nLevel)
            {
                std::string_view aToken
                    = rValue.substr(nStart + strlen("{ "), i - nStart - strlen(" },"));
                if (o3tl::starts_with(aToken, "Name = \"Coordinates\""))
                    lcl_parsePathCoordinates(rPath, aToken);
                else if (o3tl::starts_with(aToken, "Name = \"GluePoints\""))
                    lcl_parsePathGluePoints(rPath, aToken);
                else if (o3tl::starts_with(aToken, "Name = \"Segments\""))
                    lcl_parsePathSegments(rPath, aToken);
                else if (o3tl::starts_with(aToken, "Name = \"TextFrames\""))
                    lcl_parsePathTextFrames(rPath, aToken);
                else if (o3tl::starts_with(aToken, "Name = \"SubViewSize\""))
                    lcl_parsePathSubViewSize(rPath, aToken);
                else
                    SAL_WARN("oox", "lcl_parsePath: unexpected token: " << aToken);
            }
        }
    }
}
}

namespace oox::drawingml
{
void CustomShapeProperties::initializePresetDataMap()
{
    OUString aPath(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/oox-drawingml-cs-presets"_ustr);
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, StreamMode::READ);
    if (aStream.GetError() != ERRCODE_NONE)
        SAL_WARN("oox", "failed to open oox-drawingml-cs-presets");
    OStringBuffer aLine;
    OUString aName;
    bool bNotDone = aStream.ReadLine(aLine);
    PropertyMap aPropertyMap;
    bool bFirst = true;
    while (bNotDone)
    {
        static const char aCommentPrefix[] = "/* ";
        if (o3tl::starts_with(aLine, aCommentPrefix))
        {
            if (bFirst)
                bFirst = false;
            else
                maPresetDataMap[TokenMap::getTokenFromUnicode(aName)] = aPropertyMap;
            aName = OUString::fromUtf8(std::string_view(aLine).substr(
                strlen(aCommentPrefix),
                aLine.getLength() - strlen(aCommentPrefix) - strlen(" */")));
        }
        else
        {
            if (std::string_view(aLine) == "AdjustmentValues")
            {
                aStream.ReadLine(aLine);
                if (std::string_view(aLine)
                    != "([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) {}")
                {
                    std::vector<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
                    static constexpr std::string_view aExpectedPrefix(
                        "([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) { ");
                    assert(o3tl::starts_with(aLine, aExpectedPrefix));

                    std::string_view aValue = std::string_view(aLine).substr(
                        aExpectedPrefix.size(),
                        aLine.getLength() - aExpectedPrefix.size() - strlen(" }"));
                    lcl_parseAdjustmentValues(aAdjustmentValues, aValue);
                    aPropertyMap.setProperty(PROP_AdjustmentValues,
                                             comphelper::containerToSequence(aAdjustmentValues));
                }
                else
                    aPropertyMap.setProperty(PROP_AdjustmentValues, uno::Sequence<OUString>(0));
            }
            else if (std::string_view(aLine) == "Equations")
            {
                aStream.ReadLine(aLine);
                if (std::string_view(aLine) != "([]string) {}")
                {
                    std::vector<OUString> aEquations;
                    static constexpr std::string_view aExpectedPrefix("([]string) { ");
                    assert(o3tl::starts_with(aLine, aExpectedPrefix));

                    std::string_view aValue = std::string_view(aLine).substr(
                        aExpectedPrefix.size(),
                        aLine.getLength() - aExpectedPrefix.size() - strlen(" }"));
                    lcl_parseEquations(aEquations, aValue);
                    aPropertyMap.setProperty(PROP_Equations,
                                             comphelper::containerToSequence(aEquations));
                }
                else
                    aPropertyMap.setProperty(PROP_Equations, uno::Sequence<OUString>(0));
            }
            else if (std::string_view(aLine) == "Handles")
            {
                aStream.ReadLine(aLine);
                if (std::string_view(aLine) != "([][]com.sun.star.beans.PropertyValue) {}")
                {
                    std::vector<uno::Sequence<beans::PropertyValue>> aHandles;
                    static constexpr std::string_view aExpectedPrefix(
                        "([][]com.sun.star.beans.PropertyValue) { ");
                    assert(o3tl::starts_with(aLine, aExpectedPrefix));

                    std::string_view aValue = std::string_view(aLine).substr(
                        aExpectedPrefix.size(),
                        aLine.getLength() - aExpectedPrefix.size() - strlen(" }"));
                    lcl_parseHandles(aHandles, aValue);
                    aPropertyMap.setProperty(PROP_Handles,
                                             comphelper::containerToSequence(aHandles));
                }
                else
                    aPropertyMap.setProperty(PROP_Handles, uno::Sequence<OUString>(0));
            }
            else if (std::string_view(aLine) == "MirroredX")
            {
                aStream.ReadLine(aLine);
                if (std::string_view(aLine) == "true" || std::string_view(aLine) == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredX, std::string_view(aLine) == "true");
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected "
                                    "MirroredX parameter");
            }
            else if (std::string_view(aLine) == "MirroredY")
            {
                aStream.ReadLine(aLine);
                if (std::string_view(aLine) == "true" || std::string_view(aLine) == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredY, std::string_view(aLine) == "true");
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected "
                                    "MirroredY parameter");
            }
            else if (std::string_view(aLine) == "Path")
            {
                aStream.ReadLine(aLine);
                static constexpr std::string_view aExpectedPrefix(
                    "([]com.sun.star.beans.PropertyValue) { ");
                assert(o3tl::starts_with(aLine, aExpectedPrefix));

                std::vector<beans::PropertyValue> aPathValue;
                std::string_view aValue = std::string_view(aLine).substr(
                    aExpectedPrefix.size(),
                    aLine.getLength() - aExpectedPrefix.size() - strlen(" }"));
                lcl_parsePath(aPathValue, aValue);
                aPropertyMap.setProperty(PROP_Path, comphelper::containerToSequence(aPathValue));
            }
            else if (std::string_view(aLine) == "Type")
            {
                // Just ignore the line here, we already know the correct type.
                aStream.ReadLine(aLine);
                aPropertyMap.setProperty(PROP_Type, "ooxml-" + aName);
            }
            else if (std::string_view(aLine) == "ViewBox")
            {
                aStream.ReadLine(aLine);
                static constexpr std::string_view aExpectedPrefix(
                    "(com.sun.star.awt.Rectangle) { ");
                assert(o3tl::starts_with(aLine, aExpectedPrefix));

                std::string_view aValue = std::string_view(aLine).substr(
                    aExpectedPrefix.size(),
                    aLine.getLength() - aExpectedPrefix.size() - strlen(" }"));
                aPropertyMap.setProperty(PROP_ViewBox, lcl_parseRectangle(aValue));
            }
            else
                SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unhandled line: "
                                    << std::string_view(aLine));
        }
        bNotDone = aStream.ReadLine(aLine);
    }
    maPresetDataMap[TokenMap::getTokenFromUnicode(aName)] = aPropertyMap;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
