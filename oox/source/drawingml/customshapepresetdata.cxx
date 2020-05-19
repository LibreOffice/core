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

using namespace ::com::sun::star;

namespace
{
// Parses a string like: Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj"
void lcl_parseAdjustmentValue(
    std::vector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues,
    const OString& rValue)
{
    sal_Int32 nIndex = 0;
    drawing::EnhancedCustomShapeAdjustmentValue aAdjustmentValue;
    do
    {
        OString aToken = rValue.getToken(0, ',', nIndex).trim();
        static const char aNamePrefix[] = "Name = \"";
        static const char aValuePrefix[] = "Value = (any) { (long) ";
        if (aToken.startsWith(aNamePrefix))
        {
            OString aName = aToken.copy(strlen(aNamePrefix),
                                        aToken.getLength() - strlen(aNamePrefix) - strlen("\""));
            aAdjustmentValue.Name = OUString::fromUtf8(aName);
        }
        else if (aToken.startsWith(aValuePrefix))
        {
            OString aValue = aToken.copy(strlen(aValuePrefix),
                                         aToken.getLength() - strlen(aValuePrefix) - strlen(" }"));
            aAdjustmentValue.Value <<= aValue.toInt32();
        }
        else if (!aToken.startsWith("State = "))
            SAL_WARN("oox", "lcl_parseAdjustmentValue: unexpected prefix: " << aToken);
    } while (nIndex >= 0);
    rAdjustmentValues.push_back(aAdjustmentValue);
}

// Parses a string like: { Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj" }, { Value = ..., State = ..., Name = ... }
void lcl_parseAdjustmentValues(
    std::vector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues,
    const OString& rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                    rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },")));
            }
        }
    }
}

drawing::EnhancedCustomShapeParameterPair
lcl_parseEnhancedCustomShapeParameterPair(const OString& rValue)
{
    drawing::EnhancedCustomShapeParameterPair aPair;
    // We expect the following here: First.Value, First.Type, Second.Value, Second.Type
    static const char aExpectedFVPrefix[]
        = "First = (com.sun.star.drawing.EnhancedCustomShapeParameter) { Value = (any) { (long) ";
    assert(rValue.startsWith(aExpectedFVPrefix));
    sal_Int32 nIndex = strlen(aExpectedFVPrefix);
    aPair.First.Value <<= static_cast<sal_uInt32>(rValue.getToken(0, '}', nIndex).toInt32());

    static const char aExpectedFTPrefix[] = ", Type = (short) ";
    assert(nIndex >= 0 && rValue.match(aExpectedFTPrefix, nIndex));
    nIndex += strlen(aExpectedFTPrefix);
    aPair.First.Type = static_cast<sal_uInt16>(rValue.getToken(0, '}', nIndex).toInt32());

    static const char aExpectedSVPrefix[] = ", Second = "
                                            "(com.sun.star.drawing.EnhancedCustomShapeParameter) { "
                                            "Value = (any) { (long) ";
    assert(nIndex >= 0 && rValue.match(aExpectedSVPrefix, nIndex));
    nIndex += strlen(aExpectedSVPrefix);
    aPair.Second.Value <<= static_cast<sal_uInt32>(rValue.getToken(0, '}', nIndex).toInt32());

    static const char aExpectedSTPrefix[] = ", Type = (short) ";
    assert(nIndex >= 0 && rValue.match(aExpectedSTPrefix, nIndex));
    nIndex += strlen(aExpectedSTPrefix);
    aPair.Second.Type = static_cast<sal_uInt16>(rValue.getToken(0, '}', nIndex).toInt32());
    return aPair;
}

drawing::EnhancedCustomShapeSegment lcl_parseEnhancedCustomShapeSegment(const OString& rValue)
{
    drawing::EnhancedCustomShapeSegment aSegment;
    // We expect the following here: Command, Count
    static const char aExpectedCommandPrefix[] = "Command = (short) ";
    assert(rValue.startsWith(aExpectedCommandPrefix));
    sal_Int32 nIndex = strlen(aExpectedCommandPrefix);
    aSegment.Command = static_cast<sal_Int16>(rValue.getToken(0, ',', nIndex).toInt32());

    static const char aExpectedCountPrefix[] = " Count = (short) ";
    assert(nIndex >= 0 && rValue.match(aExpectedCountPrefix, nIndex));
    nIndex += strlen(aExpectedCountPrefix);
    aSegment.Count = static_cast<sal_Int16>(rValue.getToken(0, '}', nIndex).toInt32());
    return aSegment;
}

awt::Rectangle lcl_parseRectangle(const OString& rValue)
{
    awt::Rectangle aRectangle;
    // We expect the following here: X, Y, Width, Height
    static const char aExpectedXPrefix[] = "X = (long) ";
    assert(rValue.startsWith(aExpectedXPrefix));
    sal_Int32 nIndex = strlen(aExpectedXPrefix);
    aRectangle.X = rValue.getToken(0, ',', nIndex).toInt32();

    static const char aExpectedYPrefix[] = " Y = (long) ";
    assert(nIndex >= 0 && rValue.match(aExpectedYPrefix, nIndex));
    nIndex += strlen(aExpectedYPrefix);
    aRectangle.Y = rValue.getToken(0, ',', nIndex).toInt32();

    static const char aExpectedWidthPrefix[] = " Width = (long) ";
    assert(nIndex >= 0 && rValue.match(aExpectedWidthPrefix, nIndex));
    nIndex += strlen(aExpectedWidthPrefix);
    aRectangle.Width = rValue.getToken(0, ',', nIndex).toInt32();

    static const char aExpectedHeightPrefix[] = " Height = (long) ";
    assert(nIndex >= 0 && rValue.match(aExpectedHeightPrefix, nIndex));
    nIndex += strlen(aExpectedHeightPrefix);
    aRectangle.Height = rValue.copy(nIndex).toInt32();

    return aRectangle;
}

awt::Size lcl_parseSize(const OString& rValue)
{
    awt::Size aSize;
    // We expect the following here: Width, Height
    static const char aExpectedWidthPrefix[] = "Width = (long) ";
    assert(rValue.startsWith(aExpectedWidthPrefix));
    sal_Int32 nIndex = strlen(aExpectedWidthPrefix);
    aSize.Width = rValue.getToken(0, ',', nIndex).toInt32();

    static const char aExpectedHeightPrefix[] = " Height = (long) ";
    assert(nIndex >= 0 && rValue.match(aExpectedHeightPrefix, nIndex));
    nIndex += strlen(aExpectedHeightPrefix);
    aSize.Height = rValue.copy(nIndex).toInt32();

    return aSize;
}

drawing::EnhancedCustomShapeTextFrame lcl_parseEnhancedCustomShapeTextFrame(const OString& rValue)
{
    drawing::EnhancedCustomShapeTextFrame aTextFrame;
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "TopLeft = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" }"));
                aTextFrame.TopLeft = lcl_parseEnhancedCustomShapeParameterPair(aToken);
            }
            else
                SAL_WARN("oox",
                         "lcl_parseEnhancedCustomShapeTextFrame: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }

    OString aToken = rValue.copy(nStart);
    static const char aExpectedPrefix[]
        = "BottomRight = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
    if (aToken.startsWith(aExpectedPrefix))
    {
        aToken = aToken.copy(strlen(aExpectedPrefix),
                             aToken.getLength() - strlen(aExpectedPrefix) - strlen(" }"));
        aTextFrame.BottomRight = lcl_parseEnhancedCustomShapeParameterPair(aToken);
    }
    else
        SAL_WARN("oox",
                 "lcl_parseEnhancedCustomShapeTextFrame: unexpected token at the end: " << aToken);

    return aTextFrame;
}

// Parses a string like: Name = "Position", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parseHandlePosition(std::vector<beans::PropertyValue>& rHandle, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" } }"));

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = "Position";
                aPropertyValue.Value <<= lcl_parseEnhancedCustomShapeParameterPair(aToken);
                rHandle.push_back(aPropertyValue);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parseHandlePosition: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

// Parses a string like: Name = "RangeYMaximum", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parseHandleRange(std::vector<beans::PropertyValue>& rHandle, const OString& rValue,
                          const OUString& rName)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            if (rValue.match(aExpectedPrefix, nStart))
            {
                drawing::EnhancedCustomShapeParameter aParameter;
                sal_Int32 nIndex{ nStart + static_cast<sal_Int32>(strlen(aExpectedPrefix)) };
                // We expect the following here: Value and Type
                static const char aExpectedVPrefix[] = "Value = (any) { (long) ";
                assert(rValue.match(aExpectedVPrefix, nIndex));
                nIndex += strlen(aExpectedVPrefix);
                aParameter.Value <<= rValue.getToken(0, '}', nIndex).toInt32();

                static const char aExpectedTPrefix[] = ", Type = (short) ";
                assert(nIndex >= 0 && rValue.match(aExpectedTPrefix, nIndex));
                nIndex += strlen(aExpectedTPrefix);
                aParameter.Type = static_cast<sal_Int16>(rValue.getToken(0, '}', nIndex).toInt32());

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = rName;
                aPropertyValue.Value <<= aParameter;
                rHandle.push_back(aPropertyValue);
            }
            else if (!rValue.match("Name =", nStart) && !rValue.match("Handle =", nStart))
                SAL_WARN("oox", "lcl_parseHandleRange: unexpected token: "
                                    << rValue.copy(nStart, i - nStart));
            nStart = i + strlen(", ");
        }
    }
}

// Parses a string like: Name = "RefY", Handle = (long) 0, Value = (any) { (long) 0 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
void lcl_parseHandleRef(std::vector<beans::PropertyValue>& rHandle, const OString& rValue,
                        const OUString& rName)
{
    static const char aPrefix[] = "\", Handle = (long) 0, Value = (any) { (long) ";
    const sal_Int32 nCheck = SAL_N_ELEMENTS(aPrefix) - 1;
    const sal_Int32 nStart = SAL_N_ELEMENTS("Name = \"") - 1 + rName.getLength();

    if (rValue.copy(nStart, nCheck).equalsL(aPrefix, nCheck))
    {
        sal_Int32 nIndex = nStart + nCheck;
        beans::PropertyValue aPropertyValue;
        aPropertyValue.Name = rName;
        // We only expect a Value here
        aPropertyValue.Value <<= rValue.getToken(0, '}', nIndex).toInt32();
        rHandle.push_back(aPropertyValue);
    }
    else
        SAL_WARN("oox", "lcl_parseHandleRef: unexpected value: " << rValue);
}

uno::Sequence<beans::PropertyValue> lcl_parseHandle(const OString& rValue)
{
    std::vector<beans::PropertyValue> aRet;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                OString aToken = rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"));
                if (aToken.startsWith("Name = \"Position\""))
                    lcl_parseHandlePosition(aRet, aToken);
                else if (aToken.startsWith("Name = \"RangeXMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, "RangeXMaximum");
                else if (aToken.startsWith("Name = \"RangeXMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, "RangeXMinimum");
                else if (aToken.startsWith("Name = \"RangeYMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, "RangeYMaximum");
                else if (aToken.startsWith("Name = \"RangeYMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, "RangeYMinimum");
                else if (aToken.startsWith("Name = \"RadiusRangeMaximum\""))
                    lcl_parseHandleRange(aRet, aToken, "RadiusRangeMaximum");
                else if (aToken.startsWith("Name = \"RadiusRangeMinimum\""))
                    lcl_parseHandleRange(aRet, aToken, "RadiusRangeMinimum");
                else if (aToken.startsWith("Name = \"RefX\""))
                    lcl_parseHandleRef(aRet, aToken, "RefX");
                else if (aToken.startsWith("Name = \"RefY\""))
                    lcl_parseHandleRef(aRet, aToken, "RefY");
                else if (aToken.startsWith("Name = \"RefR\""))
                    lcl_parseHandleRef(aRet, aToken, "RefR");
                else if (aToken.startsWith("Name = \"RefAngle\""))
                    lcl_parseHandleRef(aRet, aToken, "RefAngle");
                else
                    SAL_WARN("oox", "lcl_parseHandle: unexpected token: " << aToken);
            }
        }
    }
    return comphelper::containerToSequence(aRet);
}

void lcl_parseHandles(std::vector<uno::Sequence<beans::PropertyValue>>& rHandles,
                      const OString& rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                    rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },")));
                rHandles.push_back(aHandle);
            }
        }
    }
}

void lcl_parseEquations(std::vector<OUString>& rEquations, const OString& rValue)
{
    bool bInString = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
    {
        if (rValue[i] == '"' && !bInString)
        {
            nStart = i;
            bInString = true;
        }
        else if (rValue[i] == '"' && bInString)
        {
            bInString = false;
            rEquations.push_back(
                OUString::fromUtf8(rValue.copy(nStart + strlen("\""), i - nStart - strlen("\""))));
        }
    }
}

void lcl_parsePathCoordinateValues(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    std::vector<drawing::EnhancedCustomShapeParameterPair> aPairs;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                    rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Coordinates";
    aPropertyValue.Value <<= comphelper::containerToSequence(aPairs);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Coordinates", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathCoordinates(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathCoordinateValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathCoordinates: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSegmentValues(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    std::vector<drawing::EnhancedCustomShapeSegment> aSegments;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                    rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Segments";
    aPropertyValue.Value <<= comphelper::containerToSequence(aSegments);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Segments", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathSegments(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeSegment) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathSegmentValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathSegments: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathTextFrameValues(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    std::vector<drawing::EnhancedCustomShapeTextFrame> aTextFrames;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                    rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "TextFrames";
    aPropertyValue.Value <<= comphelper::containerToSequence(aTextFrames);
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "TextFrames", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathTextFrames(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[]
                = "Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeTextFrame) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathTextFrameValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathTextFrames: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSubViewSizeValues(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    std::vector<awt::Size> aSizes;
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                aSizes.push_back(
                    lcl_parseSize(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "SubViewSize";
    aPropertyValue.Value <<= comphelper::containerToSequence(aSizes);
    rPath.push_back(aPropertyValue);
}

void lcl_parsePathSubViewSize(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    bool bIgnore = false;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
            OString aToken = rValue.copy(nStart, i - nStart);
            static const char aExpectedPrefix[] = "Value = (any) { ([]com.sun.star.awt.Size) { ";
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(strlen(aExpectedPrefix),
                                     aToken.getLength() - strlen(aExpectedPrefix) - strlen(" } }"));
                lcl_parsePathSubViewSizeValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathSubViewSize: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePath(std::vector<beans::PropertyValue>& rPath, const OString& rValue)
{
    sal_Int32 nLevel = 0;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < rValue.getLength(); ++i)
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
                OString aToken = rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"));
                if (aToken.startsWith("Name = \"Coordinates\""))
                    lcl_parsePathCoordinates(rPath, aToken);
                else if (aToken.startsWith("Name = \"Segments\""))
                    lcl_parsePathSegments(rPath, aToken);
                else if (aToken.startsWith("Name = \"TextFrames\""))
                    lcl_parsePathTextFrames(rPath, aToken);
                else if (aToken.startsWith("Name = \"SubViewSize\""))
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
    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/oox-drawingml-cs-presets");
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, StreamMode::READ);
    if (aStream.GetError() != ERRCODE_NONE)
        SAL_WARN("oox", "failed to open oox-drawingml-cs-presets");
    OString aLine;
    OUString aName;
    bool bNotDone = aStream.ReadLine(aLine);
    PropertyMap aPropertyMap;
    bool bFirst = true;
    while (bNotDone)
    {
        static const char aCommentPrefix[] = "/* ";
        if (aLine.startsWith(aCommentPrefix))
        {
            if (bFirst)
                bFirst = false;
            else
                maPresetDataMap[TokenMap::getTokenFromUnicode(aName)] = aPropertyMap;
            aName = OUString::fromUtf8(
                aLine.copy(strlen(aCommentPrefix),
                           aLine.getLength() - strlen(aCommentPrefix) - strlen(" */")));
        }
        else
        {
            if (aLine == "AdjustmentValues")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) {}")
                {
                    std::vector<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
                    OString aExpectedPrefix(
                        "([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(),
                                                aLine.getLength() - aExpectedPrefix.getLength()
                                                    - strlen(" }"));
                    lcl_parseAdjustmentValues(aAdjustmentValues, aValue);
                    aPropertyMap.setProperty(PROP_AdjustmentValues,
                                             comphelper::containerToSequence(aAdjustmentValues));
                }
                else
                    aPropertyMap.setProperty(PROP_AdjustmentValues, uno::Sequence<OUString>(0));
            }
            else if (aLine == "Equations")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([]string) {}")
                {
                    std::vector<OUString> aEquations;
                    OString aExpectedPrefix("([]string) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(),
                                                aLine.getLength() - aExpectedPrefix.getLength()
                                                    - strlen(" }"));
                    lcl_parseEquations(aEquations, aValue);
                    aPropertyMap.setProperty(PROP_Equations,
                                             comphelper::containerToSequence(aEquations));
                }
                else
                    aPropertyMap.setProperty(PROP_Equations, uno::Sequence<OUString>(0));
            }
            else if (aLine == "Handles")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([][]com.sun.star.beans.PropertyValue) {}")
                {
                    std::vector<uno::Sequence<beans::PropertyValue>> aHandles;
                    OString aExpectedPrefix("([][]com.sun.star.beans.PropertyValue) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(),
                                                aLine.getLength() - aExpectedPrefix.getLength()
                                                    - strlen(" }"));
                    lcl_parseHandles(aHandles, aValue);
                    aPropertyMap.setProperty(PROP_Handles,
                                             comphelper::containerToSequence(aHandles));
                }
                else
                    aPropertyMap.setProperty(PROP_Handles, uno::Sequence<OUString>(0));
            }
            else if (aLine == "MirroredX")
            {
                aStream.ReadLine(aLine);
                if (aLine == "true" || aLine == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredX, aLine == "true");
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected "
                                    "MirroredX parameter");
            }
            else if (aLine == "MirroredY")
            {
                aStream.ReadLine(aLine);
                if (aLine == "true" || aLine == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredY, aLine == "true");
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected "
                                    "MirroredY parameter");
            }
            else if (aLine == "Path")
            {
                aStream.ReadLine(aLine);
                OString aExpectedPrefix("([]com.sun.star.beans.PropertyValue) { ");
                assert(aLine.startsWith(aExpectedPrefix));

                std::vector<beans::PropertyValue> aPathValue;
                OString aValue
                    = aLine.copy(aExpectedPrefix.getLength(),
                                 aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                lcl_parsePath(aPathValue, aValue);
                aPropertyMap.setProperty(PROP_Path, comphelper::containerToSequence(aPathValue));
            }
            else if (aLine == "Type")
            {
                // Just ignore the line here, we already know the correct type.
                aStream.ReadLine(aLine);
                aPropertyMap.setProperty(PROP_Type, "ooxml-" + aName);
            }
            else if (aLine == "ViewBox")
            {
                aStream.ReadLine(aLine);
                OString aExpectedPrefix("(com.sun.star.awt.Rectangle) { ");
                assert(aLine.startsWith(aExpectedPrefix));

                OString aValue
                    = aLine.copy(aExpectedPrefix.getLength(),
                                 aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                aPropertyMap.setProperty(PROP_ViewBox, lcl_parseRectangle(aValue));
            }
            else
                SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unhandled line: "
                                    << aLine);
        }
        bNotDone = aStream.ReadLine(aLine);
    }
    maPresetDataMap[TokenMap::getTokenFromUnicode(aName)] = aPropertyMap;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
