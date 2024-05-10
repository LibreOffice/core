/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpsContext.hxx"
#include "WpgContext.hxx"
#include "WordprocessingCanvasContext.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <drawingml/customshapegeometry.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/fontworkhelpers.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textbodyproperties.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/drawingml/connectorshapecontext.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <svx/svdoashp.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <optional>

using namespace com::sun::star;

namespace
{
bool lcl_getTextPropsFromFrameText(const uno::Reference<text::XText>& xText,
                                   std::vector<beans::PropertyValue>& rTextPropVec)
{
    if (!xText.is())
        return false;
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoStart(false);
    xTextCursor->gotoEnd(true);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
    if (!paraEnumAccess.is())
        return false;
    uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());
    while (paraEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
        if (!runEnumAccess.is())
            continue;
        uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();
        while (runEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);
            if (xRun->getString().isEmpty())
                continue;
            uno::Reference<beans::XPropertySet> xRunPropSet(xRun, uno::UNO_QUERY);
            if (!xRunPropSet.is())
                continue;
            auto xRunPropSetInfo = xRunPropSet->getPropertySetInfo();
            if (!xRunPropSetInfo.is())
                continue;

            // We have found a non-empty run. Collect its properties.
            auto aRunPropInfoSequence = xRunPropSetInfo->getProperties();
            for (const beans::Property& aProp : aRunPropInfoSequence)
            {
                rTextPropVec.push_back(comphelper::makePropertyValue(
                    aProp.Name, xRunPropSet->getPropertyValue(aProp.Name)));
            }
            return true;
        }
    }
    return false;
}

// CharInteropGrabBag puts all attributes of an element into a property with Name="attributes" and
// Value being a sequence of the attributes. This methods finds the value of an individual rName
// attribute and puts it into rValue parameter. If it does not find it, rValue is unchanged and
// the method returns false, otherwise it returns true.
bool lcl_getAttributeAsString(const uno::Sequence<beans::PropertyValue>& aPropertyValueAsSeq,
                              const OUString& rName, OUString& rValue)
{
    comphelper::SequenceAsHashMap aPropertyValueAsMap(aPropertyValueAsSeq);
    uno::Sequence<beans::PropertyValue> aAttributesSeq;
    if (!((aPropertyValueAsMap.getValue(u"attributes"_ustr) >>= aAttributesSeq)
          && aAttributesSeq.hasElements()))
        return false;
    comphelper::SequenceAsHashMap aAttributesMap(aAttributesSeq);
    OUString sRet;
    if (!(aAttributesMap.getValue(rName) >>= sRet))
        return false;
    rValue = sRet;
    return true;
}

// Same as above for a number as attribute value
bool lcl_getAttributeAsNumber(const uno::Sequence<beans::PropertyValue>& rPropertyValueAsSeq,
                              const OUString& rName, sal_Int32& rValue)
{
    comphelper::SequenceAsHashMap aPropertyValueAsMap(rPropertyValueAsSeq);
    uno::Sequence<beans::PropertyValue> aAttributesSeq;
    if (!((aPropertyValueAsMap.getValue(u"attributes"_ustr) >>= aAttributesSeq)
          && aAttributesSeq.hasElements()))
        return false;
    comphelper::SequenceAsHashMap aAttributesMap(aAttributesSeq);
    sal_Int32 nRet;
    if (!(aAttributesMap.getValue(rName) >>= nRet))
        return false;
    rValue = nRet;
    return true;
}

void lcl_getColorTransformationsFromPropSeq(const uno::Sequence<beans::PropertyValue>& rPropSeq,
                                            oox::drawingml::Color& rColor)
{
    auto isValidPropName = [](const OUString& rName) -> bool {
        return rName == u"tint" || rName == u"shade" || rName == u"alpha" || rName == u"hueMod"
               || rName == u"sat" || rName == u"satOff" || rName == u"satMod" || rName == u"lum"
               || rName == u"lumOff" || rName == u"lumMod";
    };
    for (auto it = rPropSeq.begin(); it < rPropSeq.end(); ++it)
    {
        if (isValidPropName((*it).Name))
        {
            uno::Sequence<beans::PropertyValue> aValueSeq;
            sal_Int32 nNumber(0); // dummy value to make compiler happy, "val" should exist
            if (((*it).Value >>= aValueSeq)
                && lcl_getAttributeAsNumber(aValueSeq, u"val"_ustr, nNumber))
            {
                // char w14:alpha contains transparency, whereas shape fill a:alpha contains opacity.
                if ((*it).Name == u"alpha")
                    rColor.addTransformation(
                        oox::NMSP_dml | oox::AttributeConversion::decodeToken((*it).Name),
                        oox::drawingml::MAX_PERCENT - nNumber);
                else
                    rColor.addTransformation(
                        oox::NMSP_w14 | oox::AttributeConversion::decodeToken((*it).Name), nNumber);
            }
        }
    }
}

// Expected: rPropSeq contains a property "schemeClr" or a property "srgbClr".
bool lcl_getColorFromPropSeq(const uno::Sequence<beans::PropertyValue>& rPropSeq,
                             oox::drawingml::Color& rColor)
{
    bool bColorFound = false;
    comphelper::SequenceAsHashMap aPropMap(rPropSeq);
    uno::Sequence<beans::PropertyValue> aColorDetailSeq;
    if (aPropMap.getValue(u"schemeClr"_ustr) >>= aColorDetailSeq)
    {
        OUString sColorString;
        bColorFound = lcl_getAttributeAsString(aColorDetailSeq, u"val"_ustr, sColorString);
        if (bColorFound)
        {
            sal_Int32 nColorToken = oox::AttributeConversion::decodeToken(sColorString);
            rColor.setSchemeClr(nColorToken);
            rColor.setSchemeName(sColorString);
        }
    }
    if (!bColorFound && (aPropMap.getValue(u"srgbClr"_ustr) >>= aColorDetailSeq))
    {
        OUString sColorString;
        bColorFound = lcl_getAttributeAsString(aColorDetailSeq, u"val"_ustr, sColorString);
        if (bColorFound)
        {
            sal_Int32 nColor = oox::AttributeConversion::decodeIntegerHex(sColorString);
            rColor.setSrgbClr(nColor);
        }
    }
    // Without color, color transformations are pointless.
    if (bColorFound)
        lcl_getColorTransformationsFromPropSeq(aColorDetailSeq, rColor);
    return bColorFound;
}

void lcl_getFillDetailsFromPropSeq(const uno::Sequence<beans::PropertyValue>& rTextFillSeq,
                                   oox::drawingml::FillProperties& rFillProperties)
{
    // rTextFillSeq should have an item containing either "noFill" or "solidFill" or "gradFill"
    // property.
    if (!rTextFillSeq.hasElements())
        return;
    comphelper::SequenceAsHashMap aTextFillMap(rTextFillSeq);
    if (aTextFillMap.contains(u"noFill"_ustr))
    {
        rFillProperties.moFillType = oox::XML_noFill;
        return;
    }

    uno::Sequence<beans::PropertyValue> aPropSeq;
    if ((aTextFillMap.getValue(u"solidFill"_ustr) >>= aPropSeq) && aPropSeq.hasElements())
    {
        rFillProperties.moFillType = oox::XML_solidFill;
        lcl_getColorFromPropSeq(aPropSeq, rFillProperties.maFillColor);
        return;
    }

    if ((aTextFillMap.getValue(u"gradFill"_ustr) >>= aPropSeq) && aPropSeq.hasElements())
    {
        rFillProperties.moFillType = oox::XML_gradFill;
        // aPropSeq should have two items. One is "gsLst" for the stop colors, the other is
        // either "lin" or "path" for the kind of gradient.
        // First get stop colors
        comphelper::SequenceAsHashMap aPropMap(aPropSeq);
        uno::Sequence<beans::PropertyValue> aGsLstSeq;
        if (aPropMap.getValue(u"gsLst"_ustr) >>= aGsLstSeq)
        {
            for (auto it = aGsLstSeq.begin(); it < aGsLstSeq.end(); ++it)
            {
                // (*it) is a bean::PropertyValue with Name="gs". Its Value is a property sequence.
                uno::Sequence<beans::PropertyValue> aColorStopSeq;
                if ((*it).Value >>= aColorStopSeq)
                {
                    // aColorStopSeq should have an item for the color and an item for the position
                    sal_Int32 nPos;
                    oox::drawingml::Color aColor;
                    if (lcl_getAttributeAsNumber(aColorStopSeq, u"pos"_ustr, nPos)
                        && lcl_getColorFromPropSeq(aColorStopSeq, aColor))
                    {
                        // The position in maGradientStops is relative, thus in range [0.0;1.0].
                        double fPos = nPos / 100000.0;
                        rFillProperties.maGradientProps.maGradientStops.insert({ fPos, aColor });
                    }
                }
            }
        }
        // Now determine kind of gradient.
        uno::Sequence<beans::PropertyValue> aKindSeq;
        if (aPropMap.getValue(u"lin"_ustr) >>= aKindSeq)
        {
            // aKindSeq contains the attributes "ang" and "scaled"
            sal_Int32 nAngle; // in 1/60000 deg
            if (lcl_getAttributeAsNumber(aKindSeq, u"ang"_ustr, nAngle))
                rFillProperties.maGradientProps.moShadeAngle = nAngle;
            OUString sScaledString;
            if (lcl_getAttributeAsString(aKindSeq, u"scaled"_ustr, sScaledString))
                rFillProperties.maGradientProps.moShadeScaled
                    = sScaledString == u"1" || sScaledString == u"true";
            return;
        }
        if (aPropMap.getValue(u"path"_ustr) >>= aKindSeq)
        {
            // aKindSeq contains the attribute "path" for the kind of path and a property "fillToRect"
            // which defines the center rectangle of the gradient. The property "a:tileRect" known from
            // fill of shapes does not exist in w14 namespace.
            OUString sKind;
            if (lcl_getAttributeAsString(aKindSeq, u"path"_ustr, sKind))
                rFillProperties.maGradientProps.moGradientPath
                    = oox::AttributeConversion::decodeToken(sKind);
            comphelper::SequenceAsHashMap aKindMap(aKindSeq);
            uno::Sequence<beans::PropertyValue> aFillToRectSeq;
            if (aKindMap.getValue(u"fillToRect"_ustr) >>= aFillToRectSeq)
            {
                // The values l, t, r and b are not coordinates, but determine an offset from the
                // edge of the bounding box of the shape. This unusual meaning of X1, Y1, X2 and
                // Y2 is needed for method pushToPropMap() of FillProperties.
                geometry::IntegerRectangle2D aRect;
                if (!lcl_getAttributeAsNumber(aFillToRectSeq, u"l"_ustr, aRect.X1))
                    aRect.X1 = 0;
                if (!lcl_getAttributeAsNumber(aFillToRectSeq, u"t"_ustr, aRect.Y1))
                    aRect.Y1 = 0;
                if (!lcl_getAttributeAsNumber(aFillToRectSeq, u"r"_ustr, aRect.X2))
                    aRect.X2 = 0;
                if (!lcl_getAttributeAsNumber(aFillToRectSeq, u"b"_ustr, aRect.Y2))
                    aRect.Y2 = 0;
                rFillProperties.maGradientProps.moFillToRect = aRect;
            }
        }
        return;
    }
}

void lcl_getLineDetailsFromPropSeq(const uno::Sequence<beans::PropertyValue>& rTextOutlineSeq,
                                   oox::drawingml::LineProperties& rLineProperties)
{
    if (!rTextOutlineSeq.hasElements())
    {
        rLineProperties.maLineFill.moFillType = oox::XML_noFill; // MS Office default
        return;
    }
    // aTextOulineSeq contains e.g. "attributes" {w, cap, cmpd, ctr}, either
    // "solidFill" or "gradFill or "noFill", and "prstDash" and "lineJoint" properties.

    // Fill
    lcl_getFillDetailsFromPropSeq(rTextOutlineSeq, rLineProperties.maLineFill);

    // LineJoint
    comphelper::SequenceAsHashMap aTextOutlineMap(rTextOutlineSeq);
    if (aTextOutlineMap.contains(u"bevel"_ustr))
        rLineProperties.moLineJoint = oox::XML_bevel;
    else if (aTextOutlineMap.contains(u"round"_ustr))
        rLineProperties.moLineJoint = oox::XML_round;
    else if (aTextOutlineMap.contains(u"miter"_ustr))
    {
        // LineProperties has no member to store a miter limit. Therefore some heuristic is
        // added here. 0 is default for attribute "lim" in MS Office. It is rendered same as bevel.
        sal_Int32 nMiterLimit
            = aTextOutlineMap.getUnpackedValueOrDefault(u"lim"_ustr, sal_Int32(0));
        if (nMiterLimit == 0)
            rLineProperties.moLineJoint = oox::XML_bevel;
        else
            rLineProperties.moLineJoint = oox::XML_miter;
    }

    // Dash
    uno::Sequence<beans::PropertyValue> aDashSeq;
    if (aTextOutlineMap.getValue(u"prstDash"_ustr) >>= aDashSeq)
    {
        // aDashSeq contains the attribute "val" with the kind of dash, e.g. "sysDot"
        OUString sDashKind;
        if (lcl_getAttributeAsString(aDashSeq, u"val"_ustr, sDashKind))
            rLineProperties.moPresetDash = oox::AttributeConversion::decodeToken(sDashKind);
    }
    OUString sCapKind;
    if (lcl_getAttributeAsString(rTextOutlineSeq, u"cap"_ustr, sCapKind))
        rLineProperties.moLineCap = oox::AttributeConversion::decodeToken(sCapKind);

    // Width
    sal_Int32 nWidth; // EMU
    if (lcl_getAttributeAsNumber(rTextOutlineSeq, u"w"_ustr, nWidth))
        rLineProperties.moLineWidth = nWidth;

    // Compound. LineProperties has a member for it, however Fontwork can currently only render "sng".
    OUString sCompoundKind;
    if (lcl_getAttributeAsString(rTextOutlineSeq, u"cmpd"_ustr, sCompoundKind))
        rLineProperties.moLineCompound = oox::AttributeConversion::decodeToken(sCompoundKind);

    // Align. LineProperties has no member for attribute "algn".

    return;
}

oox::drawingml::LineProperties
lcl_generateLinePropertiesFromTextProps(const comphelper::SequenceAsHashMap& aTextPropMap)
{
    oox::drawingml::LineProperties aLineProperties;
    aLineProperties.maLineFill.moFillType = oox::XML_noFill; // default

    // Get property "textOutline" from aTextPropMap
    uno::Sequence<beans::PropertyValue> aCharInteropGrabBagSeq;
    if (!(aTextPropMap.getValue(u"CharInteropGrabBag"_ustr) >>= aCharInteropGrabBagSeq))
        return aLineProperties;
    if (!aCharInteropGrabBagSeq.hasElements())
        return aLineProperties;
    comphelper::SequenceAsHashMap aCharInteropGrabBagMap(aCharInteropGrabBagSeq);
    beans::PropertyValue aProp;
    if (!(aCharInteropGrabBagMap.getValue(u"CharTextOutlineTextEffect"_ustr) >>= aProp))
        return aLineProperties;
    uno::Sequence<beans::PropertyValue> aTextOutlineSeq;
    if (!(aProp.Name == "textOutline" && (aProp.Value >>= aTextOutlineSeq)
          && aTextOutlineSeq.hasElements()))
        return aLineProperties;

    // Copy line properties from aTextOutlineSeq to aLineProperties
    lcl_getLineDetailsFromPropSeq(aTextOutlineSeq, aLineProperties);
    return aLineProperties;
}

oox::drawingml::FillProperties
lcl_generateFillPropertiesFromTextProps(const comphelper::SequenceAsHashMap& rTextPropMap)
{
    oox::drawingml::FillProperties aFillProperties;
    aFillProperties.moFillType = oox::XML_solidFill; // default
    // Theme color supersedes direct color. textFill supersedes theme color. Theme color and textFill
    // are in CharInteropGrabBag.
    uno::Sequence<beans::PropertyValue> aCharInteropGrabBagSeq;
    if ((rTextPropMap.getValue(u"CharInteropGrabBag"_ustr) >>= aCharInteropGrabBagSeq)
        && aCharInteropGrabBagSeq.hasElements())
    {
        // Handle case textFill
        comphelper::SequenceAsHashMap aCharInteropGrabBagMap(aCharInteropGrabBagSeq);
        beans::PropertyValue aProp;
        if (aCharInteropGrabBagMap.getValue(u"CharTextFillTextEffect"_ustr) >>= aProp)
        {
            uno::Sequence<beans::PropertyValue> aTextFillSeq;
            if (aProp.Name == "textFill" && (aProp.Value >>= aTextFillSeq)
                && aTextFillSeq.hasElements())
            {
                // Copy fill properties from aTextFillSeq to aFillProperties
                lcl_getFillDetailsFromPropSeq(aTextFillSeq, aFillProperties);
                return aFillProperties;
            }
        }

        // no textFill, look for theme color, tint and shade
        bool bColorFound(false);
        OUString sColorString;
        if (aCharInteropGrabBagMap.getValue(u"CharThemeOriginalColor"_ustr) >>= sColorString)
        {
            sal_Int32 nThemeOrigColor = oox::AttributeConversion::decodeIntegerHex(sColorString);
            aFillProperties.maFillColor.setSrgbClr(nThemeOrigColor);
            bColorFound = true;
        }
        if (aCharInteropGrabBagMap.getValue(u"CharThemeColor"_ustr) >>= sColorString)
        {
            sal_Int32 nColorToken = oox::AttributeConversion::decodeToken(sColorString);
            aFillProperties.maFillColor.setSchemeClr(nColorToken);
            aFillProperties.maFillColor.setSchemeName(sColorString);
            bColorFound = true;
            // A character color has shade or tint, a shape color has lumMod and lumOff.
            OUString sTransformString;
            if (aCharInteropGrabBagMap.getValue(u"CharThemeColorTint"_ustr) >>= sTransformString)
            {
                double fTint = oox::AttributeConversion::decodeIntegerHex(sTransformString);
                fTint = fTint / 255.0 * oox::drawingml::MAX_PERCENT;
                aFillProperties.maFillColor.addTransformation(OOX_TOKEN(w14, lumMod),
                                                              static_cast<sal_Int32>(fTint + 0.5));
                double fOff = oox::drawingml::MAX_PERCENT - fTint;
                aFillProperties.maFillColor.addTransformation(OOX_TOKEN(w14, lumOff),
                                                              static_cast<sal_Int32>(fOff + 0.5));
            }
            else if (aCharInteropGrabBagMap.getValue(u"CharThemeColorShade"_ustr)
                     >>= sTransformString)
            {
                double fShade = oox::AttributeConversion::decodeIntegerHex(sTransformString);
                fShade = fShade / 255.0 * oox::drawingml::MAX_PERCENT;
                aFillProperties.maFillColor.addTransformation(OOX_TOKEN(w14, lumMod),
                                                              static_cast<sal_Int32>(fShade + 0.5));
            }
        }
        if (bColorFound)
            return aFillProperties;
    }

    // Neither textFill nor theme color. Look for direct color.
    sal_Int32 aCharColor = 0;
    if (rTextPropMap.getValue(u"CharColor"_ustr) >>= aCharColor)
        aFillProperties.maFillColor.setSrgbClr(aCharColor);
    else
        aFillProperties.maFillColor.setUnused();
    return aFillProperties;
}

void lcl_applyShapePropsToShape(const uno::Reference<beans::XPropertySet>& xShapePropertySet,
                                const oox::drawingml::ShapePropertyMap& rShapeProps)
{
    for (const auto& rProp : rShapeProps.makePropertyValueSequence())
    {
        xShapePropertySet->setPropertyValue(rProp.Name, rProp.Value);
    }
}

void lcl_setTextAnchorFromTextProps(const uno::Reference<beans::XPropertySet>& xShapePropertySet,
                                    const comphelper::SequenceAsHashMap& aTextPropMap)
{
    // Fontwork does not evaluate paragraph alignment but uses text anchor instead
    auto eHorzAdjust(drawing::TextHorizontalAdjust_CENTER);
    sal_Int16 nParaAlign = sal_Int16(drawing::TextHorizontalAdjust_CENTER);
    aTextPropMap.getValue(u"ParaAdjust"_ustr) >>= nParaAlign;
    switch (nParaAlign)
    {
        case sal_Int16(style::ParagraphAdjust_LEFT):
            eHorzAdjust = drawing::TextHorizontalAdjust_LEFT;
            break;
        case sal_Int16(style::ParagraphAdjust_RIGHT):
            eHorzAdjust = drawing::TextHorizontalAdjust_RIGHT;
            break;
        default:
            eHorzAdjust = drawing::TextHorizontalAdjust_CENTER;
    }
    xShapePropertySet->setPropertyValue(u"TextHorizontalAdjust"_ustr, uno::Any(eHorzAdjust));
    xShapePropertySet->setPropertyValue(u"TextVerticalAdjust"_ustr,
                                        uno::Any(drawing::TextVerticalAdjust_TOP));
}

void lcl_setTextPropsToShape(const uno::Reference<beans::XPropertySet>& xShapePropertySet,
                             std::vector<beans::PropertyValue>& aTextPropVec)
{
    auto xShapePropertySetInfo = xShapePropertySet->getPropertySetInfo();
    if (!xShapePropertySetInfo.is())
        return;
    for (size_t i = 0; i < aTextPropVec.size(); ++i)
    {
        if (xShapePropertySetInfo->hasPropertyByName(aTextPropVec[i].Name)
            && !(xShapePropertySetInfo->getPropertyByName(aTextPropVec[i].Name).Attributes
                 & beans::PropertyAttribute::READONLY)
            && aTextPropVec[i].Name != u"CharInteropGrabBag")
        {
            xShapePropertySet->setPropertyValue(aTextPropVec[i].Name, aTextPropVec[i].Value);
        }
    }
}

void lcl_applyUsedTextPropsToAllTextRuns(uno::Reference<text::XText>& xText,
                                         const std::vector<beans::PropertyValue>& aTextPropVec)
{
    if (!xText.is())
        return;
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoStart(false);
    xTextCursor->gotoEnd(true);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
    if (!paraEnumAccess.is())
        return;
    uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());
    while (paraEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
        if (!runEnumAccess.is())
            continue;
        uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();
        while (runEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);
            if (xRun->getString().isEmpty())
                continue;
            uno::Reference<beans::XPropertySet> xRunPropSet(xRun, uno::UNO_QUERY);
            if (!xRunPropSet.is())
                continue;
            auto xRunPropSetInfo = xRunPropSet->getPropertySetInfo();
            if (!xRunPropSetInfo.is())
                continue;

            for (size_t i = 0; i < aTextPropVec.size(); ++i)
            {
                if (xRunPropSetInfo->hasPropertyByName(aTextPropVec[i].Name)
                    && !(xRunPropSetInfo->getPropertyByName(aTextPropVec[i].Name).Attributes
                         & beans::PropertyAttribute::READONLY))
                    xRunPropSet->setPropertyValue(aTextPropVec[i].Name, aTextPropVec[i].Value);
            }
        }
    }
}
} // anonymous namespace

namespace oox::shape
{
WpsContext::WpsContext(ContextHandler2Helper const& rParent, uno::Reference<drawing::XShape> xShape,
                       const drawingml::ShapePtr& pMasterShapePtr,
                       const drawingml::ShapePtr& pShapePtr)
    : ShapeContext(rParent, pMasterShapePtr, pShapePtr)
    , mxShape(std::move(xShape))
{
    if (mpShapePtr)
        mpShapePtr->setWps(true);

    if (const auto pParent = dynamic_cast<const WpgContext*>(&rParent))
        m_bHasWPGParent = pParent->isFullWPGSupport();
    else if (dynamic_cast<const WordprocessingCanvasContext*>(&rParent))
        m_bHasWPGParent = true;
    else
        m_bHasWPGParent = false;

    if ((pMasterShapePtr && pMasterShapePtr->isInWordprocessingCanvas())
        || dynamic_cast<const WordprocessingCanvasContext*>(&rParent) != nullptr)
        pShapePtr->setWordprocessingCanvas(true);
}

WpsContext::~WpsContext() = default;

oox::core::ContextHandlerRef WpsContext::onCreateContext(sal_Int32 nElementToken,
                                                         const oox::AttributeList& rAttribs)
{
    switch (getBaseToken(nElementToken))
    {
        case XML_wsp:
            break;
        case XML_cNvCnPr:
        {
            // It might be a connector shape in a wordprocessing canvas
            // Replace the custom shape with a connector shape.
            if (!mpShapePtr || !mpShapePtr->isInWordprocessingCanvas() || !mpMasterShapePtr)
                break;
            // Generate new shape
            oox::drawingml::ShapePtr pShape = std::make_shared<oox::drawingml::Shape>(
                u"com.sun.star.drawing.ConnectorShape"_ustr, false);
            pShape->setConnectorShape(true);
            pShape->setWps(true);
            pShape->setWordprocessingCanvas(true);
            // ToDo: Can only copy infos from mpShapePtr to pShape for which getter available.
            pShape->setName(mpShapePtr->getName());
            pShape->setId(mpShapePtr->getId());
            pShape->setWPGChild(mpShapePtr->isWPGChild());
            // And actually replace the shape.
            mpShapePtr = pShape;
            mpMasterShapePtr->getChildren().pop_back();
            mpMasterShapePtr->getChildren().push_back(pShape);
            return new oox::drawingml::ConnectorShapePropertiesContext(
                *this, mpShapePtr, mpShapePtr->getConnectorShapeProperties());
        }
        case XML_bodyPr:
            if (mxShape.is())
            {
                // no evaluation of attribute XML_rot, because Word ignores it, as of 2022-07.

                uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                sal_Int32 nVert = rAttribs.getToken(XML_vert, XML_horz);
                if (nVert == XML_eaVert)
                {
                    xPropertySet->setPropertyValue(u"TextWritingMode"_ustr,
                                                   uno::Any(text::WritingMode_TB_RL));
                    xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                   uno::Any(text::WritingMode2::TB_RL));
                }
                else if (nVert == XML_mongolianVert)
                {
                    xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                   uno::Any(text::WritingMode2::TB_LR));
                }
                else if (nVert == XML_wordArtVert || nVert == XML_wordArtVertRtl)
                {
                    // Multiline wordArtVert is not implemented yet.
                    // It will render all the text in 1 line.
                    // Map 'wordArtVertRtl' to 'wordArtVert', as they are the same now.
                    xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                   uno::Any(text::WritingMode2::STACKED));
                }
                else if (nVert != XML_horz) // cases XML_vert and XML_vert270
                {
                    // Hack to get same rendering as after the fix for tdf#87924. If shape rotation
                    // plus text direction results in upright text, use horizontal text direction.
                    // Remove hack when frame is able to rotate.

                    // Need transformation matrix since RotateAngle does not contain flip.
                    drawing::HomogenMatrix3 aMatrix;
                    xPropertySet->getPropertyValue(u"Transformation"_ustr) >>= aMatrix;
                    basegfx::B2DHomMatrix aTransformation;
                    aTransformation.set(0, 0, aMatrix.Line1.Column1);
                    aTransformation.set(0, 1, aMatrix.Line1.Column2);
                    aTransformation.set(0, 2, aMatrix.Line1.Column3);
                    aTransformation.set(1, 0, aMatrix.Line2.Column1);
                    aTransformation.set(1, 1, aMatrix.Line2.Column2);
                    aTransformation.set(1, 2, aMatrix.Line2.Column3);
                    // For this to be a valid 2D transform matrix, the last row must be [0,0,1]
                    assert(aMatrix.Line3.Column1 == 0);
                    assert(aMatrix.Line3.Column2 == 0);
                    assert(aMatrix.Line3.Column3 == 1);
                    basegfx::B2DTuple aScale;
                    basegfx::B2DTuple aTranslate;
                    double fRotate = 0;
                    double fShearX = 0;
                    aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
                    auto nRotate(static_cast<sal_uInt16>(NormAngle360(basegfx::rad2deg(fRotate))));
                    if ((nVert == XML_vert && nRotate == 270)
                        || (nVert == XML_vert270 && nRotate == 90))
                    {
                        xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                       uno::Any(text::WritingMode2::LR_TB));
                        // ToDo: Remember original vert value and remove hack on export.
                    }
                    else if (nVert == XML_vert)
                        xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                       uno::Any(text::WritingMode2::TB_RL90));
                    else // nVert == XML_vert270
                        xPropertySet->setPropertyValue(u"WritingMode"_ustr,
                                                       uno::Any(text::WritingMode2::BT_LR));
                }

                if (bool bUpright = rAttribs.getBool(XML_upright, false))
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc(length + 1);
                    auto pGrabBag = aGrabBag.getArray();
                    pGrabBag[length].Name = "Upright";
                    pGrabBag[length].Value <<= bUpright;
                    xPropertySet->setPropertyValue(u"InteropGrabBag"_ustr, uno::Any(aGrabBag));
                }

                if (xServiceInfo.is())
                {
                    // Handle inset attributes for Writer textframes.
                    sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                    std::optional<sal_Int32> oInsets[4];
                    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aInsets); ++i)
                    {
                        std::optional<OUString> oValue = rAttribs.getString(aInsets[i]);
                        if (oValue.has_value())
                            oInsets[i] = oox::drawingml::GetCoordinate(oValue.value());
                        else
                            // Defaults from the spec: left/right: 91440 EMU, top/bottom: 45720 EMU
                            oInsets[i]
                                = (aInsets[i] == XML_lIns || aInsets[i] == XML_rIns) ? 254 : 127;
                    }
                    const OUString aShapeProps[]
                        = { u"TextLeftDistance"_ustr, u"TextUpperDistance"_ustr,
                            u"TextRightDistance"_ustr, u"TextLowerDistance"_ustr };
                    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aShapeProps); ++i)
                        if (oInsets[i])
                            xPropertySet->setPropertyValue(aShapeProps[i], uno::Any(*oInsets[i]));
                }

                // Handle text vertical adjustment inside a text frame
                if (rAttribs.hasAttribute(XML_anchor))
                {
                    drawing::TextVerticalAdjust eAdjust
                        = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                    xPropertySet->setPropertyValue(u"TextVerticalAdjust"_ustr, uno::Any(eAdjust));
                }

                // Apply character color of the shape to the shape's textbox.
                uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
                uno::Any xCharColor = xPropertySet->getPropertyValue(u"CharColor"_ustr);
                Color aColor = COL_AUTO;
                if ((xCharColor >>= aColor) && aColor != COL_AUTO)
                {
                    // tdf#135923 Apply character color of the shape to the textrun
                    //            when the character color of the textrun is default.
                    // tdf#153791 But only if the run has no background color (shd element in OOXML)
                    if (uno::Reference<container::XEnumerationAccess> paraEnumAccess{
                            xText, uno::UNO_QUERY })
                    {
                        uno::Reference<container::XEnumeration> paraEnum(
                            paraEnumAccess->createEnumeration());

                        while (paraEnum->hasMoreElements())
                        {
                            uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(),
                                                                        uno::UNO_QUERY);
                            uno::Reference<container::XEnumerationAccess> runEnumAccess(
                                xParagraph, uno::UNO_QUERY);
                            if (!runEnumAccess.is())
                                continue;
                            if (uno::Reference<beans::XPropertySet> xParaPropSet{ xParagraph,
                                                                                  uno::UNO_QUERY })
                                if ((xParaPropSet->getPropertyValue(u"ParaBackColor"_ustr)
                                     >>= aColor)
                                    && aColor != COL_AUTO)
                                    continue;

                            uno::Reference<container::XEnumeration> runEnum
                                = runEnumAccess->createEnumeration();

                            while (runEnum->hasMoreElements())
                            {
                                uno::Reference<text::XTextRange> xRun(runEnum->nextElement(),
                                                                      uno::UNO_QUERY);
                                const uno::Reference<beans::XPropertyState> xRunState(
                                    xRun, uno::UNO_QUERY);
                                if (!xRunState
                                    || xRunState->getPropertyState(u"CharColor"_ustr)
                                           == beans::PropertyState_DEFAULT_VALUE)
                                {
                                    uno::Reference<beans::XPropertySet> xRunPropSet(xRun,
                                                                                    uno::UNO_QUERY);
                                    if (!xRunPropSet)
                                        continue;
                                    if ((xRunPropSet->getPropertyValue(u"CharBackColor"_ustr)
                                         >>= aColor)
                                        && aColor != COL_AUTO)
                                        continue;
                                    if (!(xRunPropSet->getPropertyValue(u"CharColor"_ustr)
                                          >>= aColor)
                                        || aColor == COL_AUTO)
                                        xRunPropSet->setPropertyValue(u"CharColor"_ustr,
                                                                      xCharColor);
                                }
                            }
                        }
                    }
                }

                auto nWrappingType = rAttribs.getToken(XML_wrap, XML_square);
                xPropertySet->setPropertyValue(u"TextWordWrap"_ustr,
                                               uno::Any(nWrappingType == XML_square));

                return this;
            }
            else if (m_bHasWPGParent && mpShapePtr)
            {
                // this WPS context has to be inside a WPG shape, so the <BodyPr> element
                // cannot be applied to mxShape member, use mpShape instead, and after the
                // the parent shape finished, apply it for its children.
                mpShapePtr->setWPGChild(true);
                oox::drawingml::TextBodyPtr pTextBody;
                pTextBody.reset(new oox::drawingml::TextBody());

                if (rAttribs.hasAttribute(XML_anchor))
                {
                    drawing::TextVerticalAdjust eAdjust
                        = drawingml::GetTextVerticalAdjust(rAttribs.getToken(XML_anchor, XML_t));
                    pTextBody->getTextProperties().meVA = eAdjust;
                }

                sal_Int32 aInsets[] = { XML_lIns, XML_tIns, XML_rIns, XML_bIns };
                for (int i = 0; i < 4; ++i)
                {
                    if (rAttribs.hasAttribute(XML_lIns))
                    {
                        std::optional<OUString> oValue = rAttribs.getString(aInsets[i]);
                        if (oValue.has_value())
                            pTextBody->getTextProperties().moInsets[i]
                                = oox::drawingml::GetCoordinate(oValue.value());
                        else
                            // Defaults from the spec: left/right: 91440 EMU, top/bottom: 45720 EMU
                            pTextBody->getTextProperties().moInsets[i]
                                = (aInsets[i] == XML_lIns || aInsets[i] == XML_rIns) ? 254 : 127;
                    }
                }

                mpShapePtr->setTextBody(pTextBody);
            }
            break;
        case XML_noAutofit:
        case XML_spAutoFit:
        {
            uno::Reference<lang::XServiceInfo> xServiceInfo(mxShape, uno::UNO_QUERY);
            // We can't use oox::drawingml::TextBodyPropertiesContext here, as this
            // is a child context of bodyPr, so the shape is already sent: we need
            // to alter the XShape directly.
            uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                if (xServiceInfo->supportsService(u"com.sun.star.text.TextFrame"_ustr))
                    xPropertySet->setPropertyValue(
                        u"FrameIsAutomaticHeight"_ustr,
                        uno::Any(getBaseToken(nElementToken) == XML_spAutoFit));
                else
                    xPropertySet->setPropertyValue(
                        u"TextAutoGrowHeight"_ustr,
                        uno::Any(getBaseToken(nElementToken) == XML_spAutoFit));
            }
        }
        break;
        case XML_prstTxWarp:
            if (rAttribs.hasAttribute(XML_prst))
            {
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                if (xPropertySet.is())
                {
                    std::optional<OUString> presetShapeName = rAttribs.getString(XML_prst);
                    const OUString& preset = presetShapeName.value();
                    comphelper::SequenceAsHashMap aCustomShapeGeometry(
                        xPropertySet->getPropertyValue(u"CustomShapeGeometry"_ustr));
                    aCustomShapeGeometry[u"PresetTextWarp"_ustr] <<= preset;
                    xPropertySet->setPropertyValue(
                        u"CustomShapeGeometry"_ustr,
                        uno::Any(aCustomShapeGeometry.getAsConstPropertyValueList()));
                }
            }
            return new oox::drawingml::PresetTextShapeContext(
                *this, rAttribs, *(getShape()->getCustomShapeProperties()));
        case XML_txbx:
        {
            mpShapePtr->getCustomShapeProperties()->setShapeTypeOverride(true);
            mpShapePtr->setTextBox(true);
            //in case if the textbox is linked, save the attributes
            //for further processing.
            if (rAttribs.hasAttribute(XML_id))
            {
                std::optional<OUString> id = rAttribs.getString(XML_id);
                if (id.has_value())
                {
                    oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                    linkedTxtBoxAttr.id = id.value().toInt32();
                    mpShapePtr->setTxbxHasLinkedTxtBox(true);
                    mpShapePtr->setLinkedTxbxAttributes(linkedTxtBoxAttr);
                }
            }
            return this;
        }
        break;
        case XML_linkedTxbx:
        {
            //in case if the textbox is linked, save the attributes
            //for further processing.
            mpShapePtr->getCustomShapeProperties()->setShapeTypeOverride(true);
            mpShapePtr->setTextBox(true);
            std::optional<OUString> id = rAttribs.getString(XML_id);
            std::optional<OUString> seq = rAttribs.getString(XML_seq);
            if (id.has_value() && seq.has_value())
            {
                oox::drawingml::LinkedTxbxAttr linkedTxtBoxAttr;
                linkedTxtBoxAttr.id = id.value().toInt32();
                linkedTxtBoxAttr.seq = seq.value().toInt32();
                mpShapePtr->setTxbxHasLinkedTxtBox(true);
                mpShapePtr->setLinkedTxbxAttributes(linkedTxtBoxAttr);
            }
        }
        break;
        default:
            return ShapeContext::onCreateContext(nElementToken, rAttribs);
    }
    return nullptr;
}

void WpsContext::onEndElement()
{
    // Convert shape to Fontwork shape if necessary and meaningful.
    // Only at end of bodyPr all needed info is available.

    if (getBaseToken(getCurrentElement()) != XML_bodyPr)
        return;

    // Make sure all needed parts are available
    auto* pCustomShape
        = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(mxShape));
    if (!pCustomShape || !mpShapePtr || !mxShape.is())
        return;
    uno::Reference<beans::XPropertySet> xShapePropertySet(mxShape, uno::UNO_QUERY);
    if (!xShapePropertySet.is())
        return;
    // This is the text in the frame, associated with the shape
    uno::Reference<text::XText> xText(mxShape, uno::UNO_QUERY);
    if (!xText.is())
        return;

    OUString sMSPresetType;
    comphelper::SequenceAsHashMap aCustomShapeGeometry(
        xShapePropertySet->getPropertyValue(u"CustomShapeGeometry"_ustr));
    aCustomShapeGeometry[u"PresetTextWarp"_ustr] >>= sMSPresetType;
    if (sMSPresetType.isEmpty() || sMSPresetType == u"textNoShape")
        return;

    // Word can combine its "abc Transform" with a lot of shape types. LibreOffice can only render
    // the old kind WordArt, which is based on a rectangle. In case of non rectangular shape we keep
    // the shape and do not convert the text to Fontwork.
    OUString sType;
    aCustomShapeGeometry[u"Type"_ustr] >>= sType;
    if (sType != u"ooxml-rect")
        return;

    // Copy properties from frame text to have them available after the frame is removed.
    std::vector<beans::PropertyValue> aTextPropVec;
    if (!lcl_getTextPropsFromFrameText(xText, aTextPropVec))
        return;
    comphelper::SequenceAsHashMap aTextPropMap(comphelper::containerToSequence(aTextPropVec));

    // Copy text content from frame to shape. Since Fontwork uses simple text anyway, we can use
    // a string.
    OUString sFrameContent(xText->getString());
    pCustomShape->NbcSetText(sFrameContent);

    // Setting the property "TextBox" to false includes removing the attached frame from the shape.
    xShapePropertySet->setPropertyValue(u"TextBox"_ustr, uno::Any(false));

    // Set the shape into text path mode, so that the text is drawn as Fontwork. Word renders a legacy
    // "text on path" without the legacy stretching, therefore use false for bFromWordArt.
    mpShapePtr->getCustomShapeProperties()->setShapeTypeOverride(true);
    FontworkHelpers::putCustomShapeIntoTextPathMode(mxShape, getShape()->getCustomShapeProperties(),
                                                    sMSPresetType, /*bFromWordArt*/ false);

    // Apply the text props to the fontwork shape
    lcl_setTextPropsToShape(xShapePropertySet, aTextPropVec); // includes e.g. FontName
    lcl_setTextAnchorFromTextProps(xShapePropertySet, aTextPropMap);

    // Fontwork in LO uses fill and stroke of the shape and cannot style text portions individually.
    // "abc Transform" in Word uses fill and outline of the characters.
    // We need to copy the properties from a run to the shape.
    oox::drawingml::ShapePropertyMap aStrokeShapeProps(getFilter().getModelObjectHelper());
    oox::drawingml::LineProperties aCreatedLineProperties
        = lcl_generateLinePropertiesFromTextProps(aTextPropMap);
    aCreatedLineProperties.pushToPropMap(aStrokeShapeProps, getFilter().getGraphicHelper());
    lcl_applyShapePropsToShape(xShapePropertySet, aStrokeShapeProps);

    oox::drawingml::ShapePropertyMap aFillShapeProps(getFilter().getModelObjectHelper());
    oox::drawingml::FillProperties aCreatedFillProperties
        = lcl_generateFillPropertiesFromTextProps(aTextPropMap);
    aCreatedFillProperties.pushToPropMap(aFillShapeProps, getFilter().getGraphicHelper(),
                                         /*nShapeRotation*/ 0,
                                         /*nPhClr*/ API_RGB_TRANSPARENT,
                                         /*aShapeSize*/ css::awt::Size(0, 0), /*nPhClrTheme*/ -1,
                                         pCustomShape->IsMirroredX(), pCustomShape->IsMirroredY(),
                                         /*bIsCustomShape*/ true);
    lcl_applyShapePropsToShape(xShapePropertySet, aFillShapeProps);

    // Copying the text content from frame to shape as string has lost the styles. Apply the used text
    // properties back to all runs in the text.
    uno::Reference<text::XText> xNewText(pCustomShape->getUnoShape(), uno::UNO_QUERY);
    if (xNewText.is())
        lcl_applyUsedTextPropsToAllTextRuns(xNewText, aTextPropVec);

    // Fontwork stretches the text to the given path. So adapt shape size to text is nonsensical.
    xShapePropertySet->setPropertyValue(u"TextAutoGrowHeight"_ustr, uno::Any(false));
    xShapePropertySet->setPropertyValue(u"TextAutoGrowWidth"_ustr, uno::Any(false));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
