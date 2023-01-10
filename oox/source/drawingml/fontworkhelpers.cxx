/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <drawingml/fontworkhelpers.hxx>

#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/presetgeometrynames.hxx>
#include <tools/helpers.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShape.hpp>

using namespace com::sun::star;

void FontworkHelpers::resetPropertyValueInVec(std::vector<beans::PropertyValue>& rPropVec,
                                              const OUString& rName)
{
    auto aIterator = std::find_if(
        rPropVec.begin(), rPropVec.end(),
        [rName](const beans::PropertyValue& rValue) { return rValue.Name == rName; });

    if (aIterator != rPropVec.end())
        rPropVec.erase(aIterator);
}

void FontworkHelpers::putCustomShapeIntoTextPathMode(
    const css::uno::Reference<drawing::XShape>& xShape,
    const oox::drawingml::CustomShapePropertiesPtr& pCustomShapePropertiesPtr,
    const OUString& sMSPresetType, const bool bFromWordArt)
{
    if (!xShape.is() || !pCustomShapePropertiesPtr || sMSPresetType == u"textNoShape")
        return;

    uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
    if (!xDefaulter.is())
        return;

    uno::Reference<beans::XPropertySet> xSet(xShape, uno::UNO_QUERY);
    if (!xSet.is())
        return;

    // The DrawingML shapes from the presetTextWarpDefinitions are mapped to the definitions
    // in svx/../EnhancedCustomShapeGeometry.cxx, which are used for WordArt shapes from
    // binary MS Office. Therefore all adjustment values need to be adapted.
    const OUString sFontworkType = PresetGeometryTypeNames::GetFontworkType(sMSPresetType);
    auto aAdjGdList = pCustomShapePropertiesPtr->getAdjustmentGuideList();
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment(
        !aAdjGdList.empty() ? aAdjGdList.size() : 1);
    auto pAdjustment = aAdjustment.getArray();
    int nIndex = 0;
    for (const auto& aEntry : aAdjGdList)
    {
        double fValue = aEntry.maFormula.toDouble();
        // then: polar-handle, else: XY-handle
        // There exist only 8 polar-handles at all in presetTextWarp.
        if ((sFontworkType == "fontwork-arch-down-curve")
            || (sFontworkType == "fontwork-arch-down-pour" && aEntry.maName == "adj1")
            || (sFontworkType == "fontwork-arch-up-curve")
            || (sFontworkType == "fontwork-arch-up-pour" && aEntry.maName == "adj1")
            || (sFontworkType == "fontwork-open-circle-curve")
            || (sFontworkType == "fontwork-open-circle-pour" && aEntry.maName == "adj1")
            || (sFontworkType == "fontwork-circle-curve")
            || (sFontworkType == "fontwork-circle-pour" && aEntry.maName == "adj1"))
        {
            // DrawingML has 1/60000 degree unit, but WordArt simple degree. Range [0..360[
            // or range ]-180..180] doesn't matter, because only cos(angle) and
            // sin(angle) are used.
            fValue = NormAngle360(fValue / 60000.0);
        }
        else
        {
            // DrawingML writes adjustment guides as relative value with 100% = 100000,
            // but WordArt definitions use values absolute in viewBox 0 0 21600 21600,
            // so scale with 21600/100000 = 0.216, with two exceptions:
            // X-handles of waves describe increase/decrease relative to horizontal center.
            // The gdRefR of pour-shapes is not relative to viewBox but to radius.
            if ((sFontworkType == "mso-spt158" && aEntry.maName == "adj2") // textDoubleWave1
                || (sFontworkType == "fontwork-wave" && aEntry.maName == "adj2") // textWave1
                || (sFontworkType == "mso-spt157" && aEntry.maName == "adj2") // textWave2
                || (sFontworkType == "mso-spt159" && aEntry.maName == "adj2")) // textWave4
            {
                fValue = (fValue + 50000.0) * 0.216;
            }
            else if ((sFontworkType == "fontwork-arch-down-pour" && aEntry.maName == "adj2")
                     || (sFontworkType == "fontwork-arch-up-pour" && aEntry.maName == "adj2")
                     || (sFontworkType == "fontwork-open-circle-pour" && aEntry.maName == "adj2")
                     || (sFontworkType == "fontwork-circle-pour" && aEntry.maName == "adj2"))
            {
                fValue *= 0.108;
            }
            else
            {
                fValue *= 0.216;
            }
        }

        pAdjustment[nIndex].Value <<= fValue;
        pAdjustment[nIndex++].State = css::beans::PropertyState_DIRECT_VALUE;
    }

    // Set attributes in CustomShapeGeometry
    xDefaulter->createCustomShapeDefaults(sFontworkType);

    auto aGeomPropSeq
        = xSet->getPropertyValue("CustomShapeGeometry").get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);

    // Reset old properties
    static const OUStringLiteral sTextPath(u"TextPath");
    static const OUStringLiteral sAdjustmentValues(u"AdjustmentValues");
    static const OUStringLiteral sPresetTextWarp(u"PresetTextWarp");

    resetPropertyValueInVec(aGeomPropVec, u"CoordinateSize");
    resetPropertyValueInVec(aGeomPropVec, u"Equations");
    resetPropertyValueInVec(aGeomPropVec, u"Path");
    resetPropertyValueInVec(aGeomPropVec, sAdjustmentValues);
    resetPropertyValueInVec(aGeomPropVec, u"ViewBox");
    resetPropertyValueInVec(aGeomPropVec, u"Handles");
    resetPropertyValueInVec(aGeomPropVec, sTextPath);
    resetPropertyValueInVec(aGeomPropVec, sPresetTextWarp);

    bool bScaleX(false);
    if (!bFromWordArt
        && (sMSPresetType == u"textArchDown" || sMSPresetType == u"textArchUp"
            || sMSPresetType == u"textCircle" || sMSPresetType == u"textButton"))
    {
        bScaleX = true;
    }

    // Apply new properties
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        { { sTextPath, uno::Any(true) },
          { u"TextPathMode", uno::Any(drawing::EnhancedCustomShapeTextPathMode_PATH) },
          { u"ScaleX", uno::Any(bScaleX) } }));
    aGeomPropVec.push_back(comphelper::makePropertyValue(sTextPath, aPropertyValues));

    aGeomPropVec.push_back(comphelper::makePropertyValue(sPresetTextWarp, sMSPresetType));

    if (!aAdjGdList.empty())
    {
        aGeomPropVec.push_back(comphelper::makePropertyValue(sAdjustmentValues, aAdjustment));
    }

    xSet->setPropertyValue(u"CustomShapeGeometry",
                           uno::Any(comphelper::containerToSequence(aGeomPropVec)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */