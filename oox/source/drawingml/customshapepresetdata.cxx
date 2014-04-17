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
#include <tools/stream.hxx>
#include <comphelper/sequenceasvector.hxx>

#include "oox/drawingml/customshapeproperties.hxx"
#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/tokenmap.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>

using namespace ::com::sun::star;

namespace
{

// Parses a string like: Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj"
void lcl_parseAdjustmentValue(comphelper::SequenceAsVector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues, const OString& rValue)
{
    sal_Int32 nIndex = 0;
    drawing::EnhancedCustomShapeAdjustmentValue aAdjustmentValue;
    do
    {
        OString aToken = rValue.getToken(0, ',', nIndex).trim();
        static const OString aNamePrefix("Name = \"");
        static const OString aValuePrefix("Value = (any) { (long) ");
        if (aToken.startsWith(aNamePrefix))
        {
            OString aName = aToken.copy(aNamePrefix.getLength(), aToken.getLength() - aNamePrefix.getLength() - strlen("\""));
            aAdjustmentValue.Name = OStringToOUString(aName, RTL_TEXTENCODING_UTF8);
        }
        else if (aToken.startsWith(aValuePrefix))
        {
            OString aValue = aToken.copy(aValuePrefix.getLength(), aToken.getLength() - aValuePrefix.getLength() - strlen(" }"));
            aAdjustmentValue.Value = uno::makeAny(aValue.toInt32());
        }
        else if (!aToken.startsWith("State = "))
            SAL_WARN("oox", "lcl_parseAdjustmentValue: unexpected prefix: " << aToken);
    }
    while (nIndex >= 0);
    rAdjustmentValues.push_back(aAdjustmentValue);
}

// Parses a string like: { Value = (any) { (long) 19098 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE, Name = "adj" }, { Value = ..., State = ..., Name = ... }
void lcl_parseAdjustmentValues(comphelper::SequenceAsVector<drawing::EnhancedCustomShapeAdjustmentValue>& rAdjustmentValues, const OString& rValue)
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
                lcl_parseAdjustmentValue(rAdjustmentValues, rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },")));
            }
        }
    }
}

drawing::EnhancedCustomShapeParameterPair lcl_parseEnhancedCustomShapeParameterPair(const OString& rValue)
{
    drawing::EnhancedCustomShapeParameterPair aPair;
    OString aToken = rValue;
    // We expect the followings here: First.Value, First.Type, Second.Value, Second.Type
    static const OString aExpectedFVPrefix = "First = (com.sun.star.drawing.EnhancedCustomShapeParameter) { Value = (any) { (long) ";
    assert(aToken.startsWith(aExpectedFVPrefix));
    sal_Int32 nIndex = aExpectedFVPrefix.getLength();
    aPair.First.Value = uno::makeAny(static_cast<sal_uInt32>(aToken.getToken(0, '}', nIndex).toInt32()));

    static const OString aExpectedFTPrefix = ", Type = (short) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedFTPrefix));
    nIndex = aExpectedFTPrefix.getLength();
    aPair.First.Type = static_cast<sal_uInt16>(aToken.getToken(0, '}', nIndex).toInt32());

    static const OString aExpectedSVPrefix = ", Second = (com.sun.star.drawing.EnhancedCustomShapeParameter) { Value = (any) { (long) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedSVPrefix));
    nIndex = aExpectedSVPrefix.getLength();
    aPair.Second.Value = uno::makeAny(static_cast<sal_uInt32>(aToken.getToken(0, '}', nIndex).toInt32()));

    static const OString aExpectedSTPrefix = ", Type = (short) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedSTPrefix));
    nIndex = aExpectedSTPrefix.getLength();
    aPair.Second.Type = static_cast<sal_uInt16>(aToken.getToken(0, '}', nIndex).toInt32());
    return aPair;
}

drawing::EnhancedCustomShapeSegment lcl_parseEnhancedCustomShapeSegment(const OString& rValue)
{
    drawing::EnhancedCustomShapeSegment aSegment;
    OString aToken = rValue;
    // We expect the followings here: Command, Count
    static const OString aExpectedCommandPrefix = "Command = (short) ";
    assert(aToken.startsWith(aExpectedCommandPrefix));
    sal_Int32 nIndex = aExpectedCommandPrefix.getLength();
    aSegment.Command = static_cast<sal_Int16>(aToken.getToken(0, ',', nIndex).toInt32());

    static const OString aExpectedCountPrefix = " Count = (short) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedCountPrefix));
    nIndex = aExpectedCountPrefix.getLength();
    aSegment.Count = static_cast<sal_Int16>(aToken.getToken(0, '}', nIndex).toInt32());
    return aSegment;
}

awt::Rectangle lcl_parseRectangle(const OString& rValue)
{
    awt::Rectangle aRectangle;
    OString aToken = rValue;
    // We expect the followings here: X, Y, Width, Height
    static const OString aExpectedXPrefix = "X = (long) ";
    assert(aToken.startsWith(aExpectedXPrefix));
    sal_Int32 nIndex = aExpectedXPrefix.getLength();
    aRectangle.X = static_cast<sal_Int32>(aToken.getToken(0, ',', nIndex).toInt32());

    static const OString aExpectedYPrefix = " Y = (long) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedYPrefix));
    nIndex = aExpectedYPrefix.getLength();
    aRectangle.Y = static_cast<sal_Int32>(aToken.getToken(0, ',', nIndex).toInt32());

    static const OString aExpectedWidthPrefix = " Width = (long) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedWidthPrefix));
    nIndex = aExpectedWidthPrefix.getLength();
    aRectangle.Width = static_cast<sal_Int32>(aToken.getToken(0, ',', nIndex).toInt32());

    static const OString aExpectedHeightPrefix = " Height = (long) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedHeightPrefix));
    nIndex = aExpectedHeightPrefix.getLength();
    aRectangle.Width = static_cast<sal_Int32>(aToken.copy(nIndex).toInt32());

    return aRectangle;
}

awt::Size lcl_parseSize(const OString& rValue)
{
    awt::Size aSize;
    OString aToken = rValue;
    // We expect the followings here: Width, Height
    static const OString aExpectedWidthPrefix = "Width = (long) ";
    assert(aToken.startsWith(aExpectedWidthPrefix));
    sal_Int32 nIndex = aExpectedWidthPrefix.getLength();
    aSize.Width = static_cast<sal_Int32>(aToken.getToken(0, ',', nIndex).toInt32());

    static const OString aExpectedHeightPrefix = " Height = (long) ";
    aToken = aToken.copy(nIndex);
    assert(aToken.startsWith(aExpectedHeightPrefix));
    nIndex = aExpectedHeightPrefix.getLength();
    aSize.Width = static_cast<sal_Int32>(aToken.copy(nIndex).toInt32());

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
            static const OString aExpectedPrefix("TopLeft = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                aTextFrame.TopLeft = lcl_parseEnhancedCustomShapeParameterPair(aToken);
            }
            else
                SAL_WARN("oox", "lcl_parseEnhancedCustomShapeTextFrame: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }

    OString aToken = rValue.copy(nStart, rValue.getLength() - nStart);
    static const OString aExpectedPrefix("BottomRight = (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ");
    if (aToken.startsWith(aExpectedPrefix))
    {
        aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
        aTextFrame.BottomRight = lcl_parseEnhancedCustomShapeParameterPair(aToken);
    }
    else
        SAL_WARN("oox", "lcl_parseEnhancedCustomShapeTextFrame: unexpected token at the end: " << aToken);

    return aTextFrame;
}

// Parses a string like: Name = "Position", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parseHandlePosition(comphelper::SequenceAsVector<beans::PropertyValue>& rHandle, const OString& rValue)
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
            static const OString aExpectedPrefix("Value = (any) { (com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = "Position";
                aPropertyValue.Value = uno::makeAny(lcl_parseEnhancedCustomShapeParameterPair(aToken));
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
void lcl_parseHandleRange(comphelper::SequenceAsVector<beans::PropertyValue>& rHandle, const OString& rValue, const OUString& rName)
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
            static const OString aExpectedPrefix("Value = (any) { (com.sun.star.drawing.EnhancedCustomShapeParameter) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                drawing::EnhancedCustomShapeParameter aParameter;
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));
                // We expect the followings here: Value and Type
                static const OString aExpectedVPrefix = "Value = (any) { (long) ";
                assert(aToken.startsWith(aExpectedVPrefix));
                sal_Int32 nIndex = aExpectedVPrefix.getLength();
                aParameter.Value = uno::makeAny(aToken.getToken(0, '}', nIndex).toInt32());

                static const OString aExpectedTPrefix = ", Type = (short) ";
                aToken = aToken.copy(nIndex);
                assert(aToken.startsWith(aExpectedTPrefix));
                nIndex = aExpectedTPrefix.getLength();
                aParameter.Type = static_cast<sal_Int16>(aToken.getToken(0, '}', nIndex).toInt32());

                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = rName;
                aPropertyValue.Value = uno::makeAny(aParameter);
                rHandle.push_back(aPropertyValue);

            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parseHandleRange: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

// Parses a string like: Name = "RefY", Handle = (long) 0, Value = (any) { (long) 0 }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
void lcl_parseHandleRef(comphelper::SequenceAsVector<beans::PropertyValue>& rHandle, const OString& rValue, const OUString& rName)
{
    static const OString aExpectedXPrefix("Name = \"RefX\", Handle = (long) 0, Value = (any) { (long) ");
    static const OString aExpectedYPrefix("Name = \"RefY\", Handle = (long) 0, Value = (any) { (long) ");
    if (rValue.startsWith(aExpectedXPrefix) || rValue.startsWith(aExpectedYPrefix))
    {
        sal_Int32 nIndex = aExpectedXPrefix.getLength();
        beans::PropertyValue aPropertyValue;
        aPropertyValue.Name = rName;
        // We only expect a Value here
        aPropertyValue.Value = uno::makeAny(rValue.getToken(0, '}', nIndex).toInt32());
        rHandle.push_back(aPropertyValue);
    }
    else
        SAL_WARN("oox", "lcl_parseHandleRef: unexpected value: " << rValue);
}

uno::Sequence<beans::PropertyValue> lcl_parseHandle(const OString& rValue)
{
    comphelper::SequenceAsVector<beans::PropertyValue> aRet;
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
                else
                    SAL_WARN("oox", "lcl_parseHandle: unexpected token: " << aToken);
            }
        }
    }
    return aRet.getAsConstList();
}

void lcl_parseHandles(comphelper::SequenceAsVector< uno::Sequence<beans::PropertyValue> >& rHandles, const OString& rValue)
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
                uno::Sequence<beans::PropertyValue> aHandle = lcl_parseHandle(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },")));
                rHandles.push_back(aHandle);
            }
        }
    }
}

void lcl_parseEquations(comphelper::SequenceAsVector<OUString>& rEquations, const OString& rValue)
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
            rEquations.push_back(OStringToOUString(rValue.copy(nStart + strlen("\""), i - nStart - strlen("\"")), RTL_TEXTENCODING_UTF8));
        }
    }
}

void lcl_parsePathCoordinateValues(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
{
    comphelper::SequenceAsVector<drawing::EnhancedCustomShapeParameterPair> aPairs;
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
                aPairs.push_back(lcl_parseEnhancedCustomShapeParameterPair(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Coordinates";
    aPropertyValue.Value = uno::makeAny(aPairs.getAsConstList());
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Coordinates", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathCoordinates(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
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
            static const OString aExpectedPrefix("Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeParameterPair) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));
                lcl_parsePathCoordinateValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathCoordinates: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSegmentValues(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
{
    comphelper::SequenceAsVector<drawing::EnhancedCustomShapeSegment> aSegments;
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
                aSegments.push_back(lcl_parseEnhancedCustomShapeSegment(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "Segments";
    aPropertyValue.Value = uno::makeAny(aSegments.getAsConstList());
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "Segments", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathSegments(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
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
            static const OString aExpectedPrefix("Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeSegment) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));
                lcl_parsePathSegmentValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathSegments: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathTextFrameValues(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
{
    comphelper::SequenceAsVector<drawing::EnhancedCustomShapeTextFrame> aTextFrames;
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
                aTextFrames.push_back(lcl_parseEnhancedCustomShapeTextFrame(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "TextFrames";
    aPropertyValue.Value = uno::makeAny(aTextFrames.getAsConstList());
    rPath.push_back(aPropertyValue);
}

// Parses a string like: Name = "TextFrames", Handle = (long) 0, Value = (any) { ... }, State = (com.sun.star.beans.PropertyState) DIRECT_VALUE
// where "{ ... }" may contain "," as well.
void lcl_parsePathTextFrames(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
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
            static const OString aExpectedPrefix("Value = (any) { ([]com.sun.star.drawing.EnhancedCustomShapeTextFrame) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));
                lcl_parsePathTextFrameValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathTextFrames: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePathSubViewSizeValues(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
{
    comphelper::SequenceAsVector<awt::Size> aSizes;
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
                aSizes.push_back(lcl_parseSize(rValue.copy(nStart + strlen("{ "), i - nStart - strlen(" },"))));
        }
    }

    beans::PropertyValue aPropertyValue;
    aPropertyValue.Name = "SubViewSize";
    aPropertyValue.Value = uno::makeAny(aSizes.getAsConstList());
    rPath.push_back(aPropertyValue);
}

void lcl_parsePathSubViewSize(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
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
            static const OString aExpectedPrefix("Value = (any) { ([]com.sun.star.awt.Size) { ");
            if (aToken.startsWith(aExpectedPrefix))
            {
                aToken = aToken.copy(aExpectedPrefix.getLength(), aToken.getLength() - aExpectedPrefix.getLength() - strlen(" } }"));
                lcl_parsePathSubViewSizeValues(rPath, aToken);
            }
            else if (!aToken.startsWith("Name =") && !aToken.startsWith("Handle ="))
                SAL_WARN("oox", "lcl_parsePathSubViewSize: unexpected token: " << aToken);
            nStart = i + strlen(", ");
        }
    }
}

void lcl_parsePath(comphelper::SequenceAsVector<beans::PropertyValue>& rPath, const OString& rValue)
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

namespace oox
{
namespace drawingml
{

void CustomShapeProperties::initializePresetDataMap()
{
    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/oox-drawingml-cs-presets");
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, STREAM_READ);
    if (aStream.GetError() != ERRCODE_NONE)
        SAL_WARN("oox", "failed to open oox-drawingml-cs-presets");
    OString aLine;
    OUString aName;
    bool bNotDone = aStream.ReadLine(aLine);
    PropertyMap aPropertyMap;
    bool bFirst = true;
    while (bNotDone)
    {
        static const OString aCommentPrefix("/* ");
        if (aLine.startsWith(aCommentPrefix))
        {
            if (bFirst)
                bFirst = false;
            else
                maPresetDataMap[StaticTokenMap::get().getTokenFromUnicode(aName)] = aPropertyMap;
            aName = OStringToOUString(aLine.copy(aCommentPrefix.getLength(), aLine.getLength() - aCommentPrefix.getLength() - strlen(" */")), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            if (aLine == "AdjustmentValues")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) {}")
                {
                    comphelper::SequenceAsVector<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
                    OString aExpectedPrefix("([]com.sun.star.drawing.EnhancedCustomShapeAdjustmentValue) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(), aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                    lcl_parseAdjustmentValues(aAdjustmentValues, aValue);
                    aPropertyMap.setProperty(PROP_AdjustmentValues, aAdjustmentValues.getAsConstList());
                }
                else
                    aPropertyMap.setProperty(PROP_AdjustmentValues, uno::Sequence<OUString>(0));
            }
            else if (aLine == "Equations")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([]string) {}")
                {
                    comphelper::SequenceAsVector<OUString> aEquations;
                    OString aExpectedPrefix("([]string) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(), aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                    lcl_parseEquations(aEquations, aValue);
                    aPropertyMap.setProperty(PROP_Equations, aEquations.getAsConstList());
                }
                else
                    aPropertyMap.setProperty(PROP_Equations, uno::Sequence<OUString>(0));
            }
            else if (aLine == "Handles")
            {
                aStream.ReadLine(aLine);
                if (aLine != "([][]com.sun.star.beans.PropertyValue) {}")
                {
                    comphelper::SequenceAsVector< uno::Sequence<beans::PropertyValue> > aHandles;
                    OString aExpectedPrefix("([][]com.sun.star.beans.PropertyValue) { ");
                    assert(aLine.startsWith(aExpectedPrefix));

                    OString aValue = aLine.copy(aExpectedPrefix.getLength(), aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                    lcl_parseHandles(aHandles, aValue);
                    aPropertyMap.setProperty(PROP_Handles, aHandles.getAsConstList());
                }
                else
                    aPropertyMap.setProperty(PROP_Handles, uno::Sequence<OUString>(0));
            }
            else if (aLine == "MirroredX")
            {
                aStream.ReadLine(aLine);
                if (aLine == "true" || aLine == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredX, sal_Bool(aLine == "true" ? sal_True : sal_False));
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected MirroredX parameter");
            }
            else if (aLine == "MirroredY")
            {
                aStream.ReadLine(aLine);
                if (aLine == "true" || aLine == "false")
                {
                    aPropertyMap.setProperty(PROP_MirroredY, sal_Bool(aLine == "true" ? sal_True : sal_False));
                }
                else
                    SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unexpected MirroredY parameter");
            }
            else if (aLine == "Path")
            {
                aStream.ReadLine(aLine);
                OString aExpectedPrefix("([]com.sun.star.beans.PropertyValue) { ");
                assert(aLine.startsWith(aExpectedPrefix));

                comphelper::SequenceAsVector<beans::PropertyValue> aPathValue;
                OString aValue = aLine.copy(aExpectedPrefix.getLength(), aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                lcl_parsePath(aPathValue, aValue);
                aPropertyMap.setProperty(PROP_Path, aPathValue.getAsConstList());
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

                OString aValue = aLine.copy(aExpectedPrefix.getLength(), aLine.getLength() - aExpectedPrefix.getLength() - strlen(" }"));
                aPropertyMap.setProperty(PROP_ViewBox, lcl_parseRectangle(aValue));
            }
            else
                SAL_WARN("oox", "CustomShapeProperties::initializePresetDataMap: unhandled line: " << aLine);
        }
        bNotDone = aStream.ReadLine(aLine);
    }
    maPresetDataMap[StaticTokenMap::get().getTokenFromUnicode(aName)] = aPropertyMap;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
