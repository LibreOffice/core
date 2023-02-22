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
#include <comphelper/sequenceashashmap.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/presetgeometrynames.hxx>
#include <oox/helper/grabbagstack.hxx>
#include <svx/msdffdef.hxx>
#include <tools/color.hxx>
#include <tools/helpers.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/DashStyle.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/XThemeColor.hpp>

#include <array>
#include <map>

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

OString FontworkHelpers::GetVMLFontworkShapetypeMarkup(const MSO_SPT eShapeType)
{
    // The markup is taken from VML in DOCX documents. Using the generated 'vml-shape-types' file
    // does not work.

    static const std::map<MSO_SPT, OString> aTypeToMarkupMap{
        { mso_sptTextSimple,
          "<v:shapetype id=\"_x0000_t24\" coordsize=\"21600,21600\" o:spt=\"24\" adj=\"10800\" "
          "path=\"m@7,l@8,m@5,21600l@6,21600e\"><v:formulas><v:f eqn=\"sum #0 0 10800\"/><v:f "
          "eqn=\"prod #0 2 1\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 0 0 @2\"/><v:f "
          "eqn=\"sum 21600 0 @3\"/><v:f eqn=\"if @0 @3 0\"/><v:f eqn=\"if @0 21600 @1\"/><v:f "
          "eqn=\"if @0 0 @2\"/><v:f eqn=\"if @0 @4 21600\"/><v:f eqn=\"mid @5 @6\"/><v:f eqn=\"mid "
          "@8 @5\"/><v:f eqn=\"mid @7 @8\"/><v:f eqn=\"mid @6 @7\"/><v:f eqn=\"sum @6 0 "
          "@5\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@9,0;@10,10800;@11,21600;@12,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"#0,bottomRight\" xrange=\"6629,14971\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextOctagon,
          "<v:shapetype id=\"_x0000_t25\" coordsize=\"21600,21600\" o:spt=\"25\" adj=\"4800\" "
          "path=\"m0@0l7200,r7200,l21600@0m0@1l7200,21600r7200,l21600@1e\"><v:formulas><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"sum 21600 0 @0\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"rect\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"3086,10800\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextHexagon,
          "<v:shapetype id=\"_x0000_t26\" coordsize=\"21600,21600\" o:spt=\"26\" adj=\"10800\" "
          "path=\"m0@0l10800,,21600@0m,21600r10800,l21600,21600e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @1 10800 0\"/><v:f eqn=\"sum 21600 0 "
          "@1\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;5400,@1;10800,21600;16200,@1\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCurve,
          "<v:shapetype id=\"_x0000_t27\" coordsize=\"21600,21600\" o:spt=\"27\" adj=\"3086\" "
          "path=\"m,qy10800@0,21600,m0@1qy10800,21600,21600@1e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"prod @1 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@0;0,@2;10800,21600;21600,@2\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"center,#0\" yrange=\"0,7200\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextWave,
          "<v:shapetype id=\"_x0000_t28\" coordsize=\"21600,21600\" o:spt=\"28\" "
          "adj=\"2809,10800\" "
          "path=\"m@25@0c@26@3@27@1@28@0m@21@4c@22@5@23@6@24@4e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod @0 41 9\"/><v:f eqn=\"prod @0 23 9\"/><v:f eqn=\"sum 0 0 "
          "@2\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 21600 0 "
          "@3\"/><v:f eqn=\"sum #1 0 10800\"/><v:f eqn=\"sum 21600 0 #1\"/><v:f eqn=\"prod @8 2 "
          "3\"/><v:f eqn=\"prod @8 4 3\"/><v:f eqn=\"prod @8 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@9\"/><v:f eqn=\"sum 21600 0 @10\"/><v:f eqn=\"sum 21600 0 @11\"/><v:f eqn=\"prod #1 2 "
          "3\"/><v:f eqn=\"prod #1 4 3\"/><v:f eqn=\"prod #1 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@15\"/><v:f eqn=\"sum 21600 0 @16\"/><v:f eqn=\"sum 21600 0 @17\"/><v:f eqn=\"if @7 @14 "
          "0\"/><v:f eqn=\"if @7 @13 @15\"/><v:f eqn=\"if @7 @12 @16\"/><v:f eqn=\"if @7 21600 "
          "@17\"/><v:f eqn=\"if @7 0 @20\"/><v:f eqn=\"if @7 @9 @19\"/><v:f eqn=\"if @7 @10 "
          "@18\"/><v:f eqn=\"if @7 @11 21600\"/><v:f eqn=\"sum @24 0 @21\"/><v:f eqn=\"sum @4 0 "
          "@0\"/><v:f eqn=\"max @21 @25\"/><v:f eqn=\"min @24 @28\"/><v:f eqn=\"prod @0 2 "
          "1\"/><v:f eqn=\"sum 21600 0 @33\"/><v:f eqn=\"mid @26 @27\"/><v:f eqn=\"mid @24 "
          "@28\"/><v:f eqn=\"mid @22 @23\"/><v:f eqn=\"mid @21 @25\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@35,@0;@38,10800;@37,@4;@36,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" yrange=\"0,4459\"/><v:h "
          "position=\"#1,bottomRight\" xrange=\"8640,12960\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextRing,
          "<v:shapetype id=\"_x0000_t29\" coordsize=\"21600,21600\" o:spt=\"29\" "
          "adj=\"11796480,5400\" "
          "path=\"al10800,10800,10800,10800@2@14al10800,10800@0@0@2@14e\"><v:formulas><v:f "
          "eqn=\"val #1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 "
          "180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 "
          "90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 "
          "90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 "
          "@10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 "
          "@14\"/><v:f eqn=\"val 10800\"/><v:f eqn=\"sum 10800 0 #1\"/><v:f eqn=\"prod #1 1 "
          "2\"/><v:f eqn=\"sum @18 5400 0\"/><v:f eqn=\"cos @19 #0\"/><v:f eqn=\"sin @19 "
          "#0\"/><v:f eqn=\"sum @20 10800 0\"/><v:f eqn=\"sum @21 10800 0\"/><v:f eqn=\"sum 10800 "
          "0 @20\"/><v:f eqn=\"sum #1 10800 0\"/><v:f eqn=\"if @9 @17 @25\"/><v:f eqn=\"if @9 0 "
          "21600\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@27;@22,@23;10800,@26;@24,@23\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"#1,#0\" polar=\"10800,10800\" "
          "radiusrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextOnCurve,
          "<v:shapetype id=\"_x0000_t30\" coordsize=\"21600,21600\" o:spt=\"30\" adj=\"3086\" "
          "path=\"m,qy10800@0,21600,m0@1qy10800,21600,21600@1e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"prod @1 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@0;0,@2;10800,21600;21600,@2\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"center,#0\" yrange=\"0,7200\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextOnRing,
          "<v:shapetype id=\"_x0000_t31\" coordsize=\"21600,21600\" o:spt=\"31\" adj=\"11796480\" "
          "path=\"al10800,10800,10800,10800@2@14e\"><v:formulas><v:f eqn=\"val #1\"/><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 180\"/><v:f "
          "eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 90 0\"/><v:f "
          "eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 90\"/><v:f eqn=\"if "
          "@9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 @10\"/><v:f "
          "eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 @14\"/><v:f "
          "eqn=\"val 10800\"/><v:f eqn=\"cos 10800 #0\"/><v:f eqn=\"sin 10800 #0\"/><v:f eqn=\"sum "
          "@17 10800 0\"/><v:f eqn=\"sum @18 10800 0\"/><v:f eqn=\"sum 10800 0 @17\"/><v:f "
          "eqn=\"if @9 0 21600\"/><v:f eqn=\"sum 10800 0 @18\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@22;@19,@20;@21,@20\"/><v:textpath on=\"t\" "
          "style=\"v-text-kern:t\" fitpath=\"t\"/><v:handles><v:h position=\"@16,#0\" "
          "polar=\"10800,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextPlainText,
          "<v:shapetype id=\"_x0000_t136\" coordsize=\"21600,21600\" o:spt=\"136\" adj=\"10800\" "
          "path=\"m@7,l@8,m@5,21600l@6,21600e\"><v:formulas><v:f eqn=\"sum #0 0 10800\"/><v:f "
          "eqn=\"prod #0 2 1\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 0 0 @2\"/><v:f "
          "eqn=\"sum 21600 0 @3\"/><v:f eqn=\"if @0 @3 0\"/><v:f eqn=\"if @0 21600 @1\"/><v:f "
          "eqn=\"if @0 0 @2\"/><v:f eqn=\"if @0 @4 21600\"/><v:f eqn=\"mid @5 @6\"/><v:f eqn=\"mid "
          "@8 @5\"/><v:f eqn=\"mid @7 @8\"/><v:f eqn=\"mid @6 @7\"/><v:f eqn=\"sum @6 0 "
          "@5\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@9,0;@10,10800;@11,21600;@12,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"#0,bottomRight\" xrange=\"6629,14971\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextStop,
          "<v:shapetype id=\"_x0000_t137\" coordsize=\"21600,21600\" o:spt=\"137\" adj=\"4800\" "
          "path=\"m0@0l7200,r7200,l21600@0m0@1l7200,21600r7200,l21600@1e\"><v:formulas><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"sum 21600 0 @0\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"rect\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"3086,10800\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextTriangle,
          "<v:shapetype id=\"_x0000_t138\" coordsize=\"21600,21600\" o:spt=\"138\" adj=\"10800\" "
          "path=\"m0@0l10800,,21600@0m,21600r10800,l21600,21600e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @1 10800 0\"/><v:f eqn=\"sum 21600 0 "
          "@1\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;5400,@1;10800,21600;16200,@1\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextTriangleInverted,
          "<v:shapetype id=\"_x0000_t139\" coordsize=\"21600,21600\" o:spt=\"139\" adj=\"10800\" "
          "path=\"m,l10800,,21600,m0@0l10800,21600,21600@0e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @1 10800 0\"/><v:f eqn=\"sum 21600 0 "
          "@1\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;5400,@2;10800,21600;16200,@2\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextChevron,
          "<v:shapetype id=\"_x0000_t140\" coordsize=\"21600,21600\" o:spt=\"140\" adj=\"5400\" "
          "path=\"m0@0l10800,,21600@0m,21600l10800@1,21600,21600e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;0,@3;10800,@1;21600,@3\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextChevronInverted,
          "<v:shapetype id=\"_x0000_t141\" coordsize=\"21600,21600\" o:spt=\"141\" adj=\"16200\" "
          "path=\"m,l10800@1,21600,m0@0l10800,21600,21600@0e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@1;0,@2;10800,21600;21600,@2\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"10800,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextRingInside,
          "<v:shapetype id=\"_x0000_t142\" coordsize=\"21600,21600\" o:spt=\"142\" adj=\"13500\" "
          "path=\"m0@1qy10800,,21600@1,10800@0,0@1m0@2qy10800@3,21600@2,10800,21600,0@2e\"><v:"
          "formulas><v:f eqn=\"val #0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum height 0 "
          "@1\"/><v:f eqn=\"sum height 0 #0\"/><v:f eqn=\"sum @2 0 @1\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;10800,@0;0,10800;10800,21600;10800,@3;21600,10800\" "
          "o:connectangles=\"270,270,180,90,90,0\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"10800,21600\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextRingOutside,
          "<v:shapetype id=\"_x0000_t143\" coordsize=\"21600,21600\" o:spt=\"143\" adj=\"13500\" "
          "path=\"m0@1qy10800@0,21600@1,10800,,0@1m0@2qy10800,21600,21600@2,10800@3,0@2e\"><v:"
          "formulas><v:f eqn=\"val #0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum height 0 "
          "@1\"/><v:f eqn=\"sum height 0 #0\"/><v:f eqn=\"sum @2 0 @1\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;10800,@0;0,10800;10800,21600;10800,@3;21600,10800\" "
          "o:connectangles=\"270,270,180,90,90,0\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"10800,21600\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextArchUpCurve,
          "<v:shapetype id=\"_x0000_t144\" coordsize=\"21600,21600\" o:spt=\"144\" "
          "adj=\"11796480\" path=\"al10800,10800,10800,10800@2@14e\"><v:formulas><v:f eqn=\"val "
          "#1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 "
          "180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 "
          "90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 "
          "90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 "
          "@10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 "
          "@14\"/><v:f eqn=\"val 10800\"/><v:f eqn=\"cos 10800 #0\"/><v:f eqn=\"sin 10800 "
          "#0\"/><v:f eqn=\"sum @17 10800 0\"/><v:f eqn=\"sum @18 10800 0\"/><v:f eqn=\"sum 10800 "
          "0 @17\"/><v:f eqn=\"if @9 0 21600\"/><v:f eqn=\"sum 10800 0 @18\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@22;@19,@20;@21,@20\"/><v:textpath on=\"t\" "
          "style=\"v-text-kern:t\" fitpath=\"t\"/><v:handles><v:h position=\"@16,#0\" "
          "polar=\"10800,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextArchDownCurve,
          "<v:shapetype id=\"_x0000_t145\" coordsize=\"21600,21600\" o:spt=\"145\" "
          "path=\"al10800,10800,10800,10800@3@15e\"><v:formulas><v:f eqn=\"val #1\"/><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 180\"/><v:f "
          "eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 90 0\"/><v:f "
          "eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 90\"/><v:f eqn=\"if "
          "@9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 @10\"/><v:f "
          "eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 @14\"/><v:f "
          "eqn=\"val 10800\"/><v:f eqn=\"cos 10800 #0\"/><v:f eqn=\"sin 10800 #0\"/><v:f eqn=\"sum "
          "@17 10800 0\"/><v:f eqn=\"sum @18 10800 0\"/><v:f eqn=\"sum 10800 0 @17\"/><v:f "
          "eqn=\"if @9 0 21600\"/><v:f eqn=\"sum 10800 0 @18\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@22;@19,@20;@21,@20\"/><v:textpath on=\"t\" "
          "style=\"v-text-kern:t\" fitpath=\"t\"/><v:handles><v:h position=\"@16,#0\" "
          "polar=\"10800,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCircleCurve,
          "<v:shapetype id=\"_x0000_t146\" coordsize=\"21600,21600\" o:spt=\"146\" "
          "adj=\"-11730944\" path=\"al10800,10800,10800,10800@2@5e\"><v:formulas><v:f eqn=\"val "
          "#1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"prod #0 2 1\"/><v:f "
          "eqn=\"sumangle @3 0 360\"/><v:f eqn=\"if @3 @4 @3\"/><v:f eqn=\"val 10800\"/><v:f "
          "eqn=\"cos 10800 #0\"/><v:f eqn=\"sin 10800 #0\"/><v:f eqn=\"sum @7 10800 0\"/><v:f "
          "eqn=\"sum @8 10800 0\"/><v:f eqn=\"sum 10800 0 @8\"/><v:f eqn=\"if #0 0 "
          "21600\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@12,10800;@9,@10;@9,@11\"/><v:textpath on=\"t\" style=\"v-text-kern:t\" "
          "fitpath=\"t\"/><v:handles><v:h position=\"@6,#0\" "
          "polar=\"10800,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextButtonCurve,
          "<v:shapetype id=\"_x0000_t147\" coordsize=\"21600,21600\" o:spt=\"147\" "
          "adj=\"11796480\" "
          "path=\"al10800,10800,10800,10800@2@14m,10800r21600,al10800,10800,10800,10800@1@15e\"><v:"
          "formulas><v:f eqn=\"val #1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f "
          "eqn=\"sumangle #0 0 180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 "
          "1\"/><v:f eqn=\"sumangle #0 90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f "
          "eqn=\"sumangle @8 0 90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 "
          "360\"/><v:f eqn=\"if @10 @11 @10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 "
          "@13 @12\"/><v:f eqn=\"sum 0 0 @14\"/><v:f eqn=\"val 10800\"/><v:f eqn=\"cos 10800 "
          "#0\"/><v:f eqn=\"sin 10800 #0\"/><v:f eqn=\"sum @17 10800 0\"/><v:f eqn=\"sum @18 10800 "
          "0\"/><v:f eqn=\"sum 10800 0 @17\"/><v:f eqn=\"if @9 0 21600\"/><v:f eqn=\"sum 10800 0 "
          "@18\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;@19,@20;@21,@20;10800,10800;0,10800;21600,10800;10800,21600;@19,"
          "@23;@21,@23\"/><v:textpath on=\"t\" style=\"v-text-kern:t\" "
          "fitpath=\"t\"/><v:handles><v:h position=\"@16,#0\" "
          "polar=\"10800,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextArchUpPour,
          "<v:shapetype id=\"_x0000_t148\" coordsize=\"21600,21600\" o:spt=\"148\" "
          "adj=\"11796480,5400\" "
          "path=\"al10800,10800,10800,10800@2@14al10800,10800@0@0@2@14e\"><v:formulas><v:f "
          "eqn=\"val #1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 "
          "180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 "
          "90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 "
          "90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 "
          "@10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 "
          "@14\"/><v:f eqn=\"val 10800\"/><v:f eqn=\"sum 10800 0 #1\"/><v:f eqn=\"prod #1 1 "
          "2\"/><v:f eqn=\"sum @18 5400 0\"/><v:f eqn=\"cos @19 #0\"/><v:f eqn=\"sin @19 "
          "#0\"/><v:f eqn=\"sum @20 10800 0\"/><v:f eqn=\"sum @21 10800 0\"/><v:f eqn=\"sum 10800 "
          "0 @20\"/><v:f eqn=\"sum #1 10800 0\"/><v:f eqn=\"if @9 @17 @25\"/><v:f eqn=\"if @9 0 "
          "21600\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@27;@22,@23;10800,@26;@24,@23\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"#1,#0\" polar=\"10800,10800\" "
          "radiusrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextArchDownPour,
          "<v:shapetype id=\"_x0000_t149\" coordsize=\"21600,21600\" o:spt=\"149\" adj=\",5400\" "
          "path=\"al10800,10800@0@0@3@15al10800,10800,10800,10800@3@15e\"><v:formulas><v:f "
          "eqn=\"val #1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 "
          "180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 "
          "90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 "
          "90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 "
          "@10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 "
          "@14\"/><v:f eqn=\"val 10800\"/><v:f eqn=\"sum 10800 0 #1\"/><v:f eqn=\"prod #1 1 "
          "2\"/><v:f eqn=\"sum @18 5400 0\"/><v:f eqn=\"cos @19 #0\"/><v:f eqn=\"sin @19 "
          "#0\"/><v:f eqn=\"sum @20 10800 0\"/><v:f eqn=\"sum @21 10800 0\"/><v:f eqn=\"sum 10800 "
          "0 @20\"/><v:f eqn=\"sum #1 10800 0\"/><v:f eqn=\"if @9 @17 @25\"/><v:f eqn=\"if @9 0 "
          "21600\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@27;@22,@23;10800,@26;@24,@23\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"#1,#0\" polar=\"10800,10800\" "
          "radiusrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCirclePour,
          "<v:shapetype id=\"_x0000_t150\" coordsize=\"21600,21600\" o:spt=\"150\" "
          "adj=\"-11730944,5400\" "
          "path=\"al10800,10800,10800,10800@2@5al10800,10800@0@0@2@5e\"><v:formulas><v:f eqn=\"val "
          "#1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"prod #0 2 1\"/><v:f "
          "eqn=\"sumangle @3 0 360\"/><v:f eqn=\"if @3 @4 @3\"/><v:f eqn=\"val 10800\"/><v:f "
          "eqn=\"sum 10800 0 #1\"/><v:f eqn=\"prod #1 1 2\"/><v:f eqn=\"sum @8 5400 0\"/><v:f "
          "eqn=\"cos @9 #0\"/><v:f eqn=\"sin @9 #0\"/><v:f eqn=\"sum @10 10800 0\"/><v:f eqn=\"sum "
          "@11 10800 0\"/><v:f eqn=\"sum 10800 0 @11\"/><v:f eqn=\"sum #1 10800 0\"/><v:f eqn=\"if "
          "#0 @7 @15\"/><v:f eqn=\"if #0 0 21600\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" "
          "o:connectlocs=\"@17,10800;@12,@13;@16,10800;@12,@14\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"#1,#0\" polar=\"10800,10800\" "
          "radiusrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextButtonPour,
          "<v:shapetype id=\"_x0000_t151\" coordsize=\"21600,21600\" o:spt=\"151\" "
          "adj=\"11796480,5400\" "
          "path=\"al10800,10800,10800,10800@2@14al10800,10800@0@0@2@14m@25@17l@26@17m@25@18l@26@"
          "18al10800,10800@0@0@1@15al10800,10800,10800,10800@1@15e\"><v:formulas><v:f eqn=\"val "
          "#1\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum 0 0 #0\"/><v:f eqn=\"sumangle #0 0 "
          "180\"/><v:f eqn=\"sumangle #0 0 90\"/><v:f eqn=\"prod @4 2 1\"/><v:f eqn=\"sumangle #0 "
          "90 0\"/><v:f eqn=\"prod @6 2 1\"/><v:f eqn=\"abs #0\"/><v:f eqn=\"sumangle @8 0 "
          "90\"/><v:f eqn=\"if @9 @7 @5\"/><v:f eqn=\"sumangle @10 0 360\"/><v:f eqn=\"if @10 @11 "
          "@10\"/><v:f eqn=\"sumangle @12 0 360\"/><v:f eqn=\"if @12 @13 @12\"/><v:f eqn=\"sum 0 0 "
          "@14\"/><v:f eqn=\"sum #1 10800 0\"/><v:f eqn=\"prod @16 1 2\"/><v:f eqn=\"sum 21600 0 "
          "@17\"/><v:f eqn=\"sum 10800 0 #1\"/><v:f eqn=\"prod @19 1 2\"/><v:f eqn=\"prod @20 @20 "
          "1\"/><v:f eqn=\"prod #1 #1 1\"/><v:f eqn=\"sum @22 0 @21\"/><v:f eqn=\"sqrt @23\"/><v:f "
          "eqn=\"sum 10800 0 @24\"/><v:f eqn=\"sum @24 10800 0\"/><v:f eqn=\"val 10800\"/><v:f "
          "eqn=\"cos @17 #0\"/><v:f eqn=\"sin @17 #0\"/><v:f eqn=\"sum @28 10800 0\"/><v:f "
          "eqn=\"sum @29 10800 0\"/><v:f eqn=\"sum 10800 0 @28\"/><v:f eqn=\"sum 10800 0 "
          "@29\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;@30,@31;10800,@19;@32,@31;10800,@17;@25,10800;10800,@18;@26,"
          "10800;10800,@16;@30,@33;10800,21600;@32,@33\"/><v:textpath on=\"t\" "
          "fitshape=\"t\"/><v:handles><v:h position=\"#1,#0\" polar=\"10800,10800\" "
          "radiusrange=\"4320,10800\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCurveUp,
          "<v:shapetype id=\"_x0000_t152\" coordsize=\"21600,21600\" o:spt=\"152\" adj=\"9931\" "
          "path=\"m0@0c7200@2,14400@1,21600,m0@5c7200@6,14400@6,21600@5e\"><v:formulas><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"prod #0 3 4\"/><v:f eqn=\"prod #0 5 4\"/><v:f eqn=\"prod #0 "
          "3 8\"/><v:f eqn=\"prod #0 1 8\"/><v:f eqn=\"sum 21600 0 @3\"/><v:f eqn=\"sum @4 21600 "
          "0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"prod @5 1 2\"/><v:f eqn=\"sum @7 @8 0\"/><v:f "
          "eqn=\"prod #0 7 8\"/><v:f eqn=\"prod @5 1 3\"/><v:f eqn=\"sum @1 @2 0\"/><v:f eqn=\"sum "
          "@12 @0 0\"/><v:f eqn=\"prod @13 1 4\"/><v:f eqn=\"sum @11 14400 "
          "@14\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@10;0,@9;10800,21600;21600,@8\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" "
          "yrange=\"0,12169\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCurveDown,
          "<v:shapetype id=\"_x0000_t153\" coordsize=\"21600,21600\" o:spt=\"153\" adj=\"9391\" "
          "path=\"m,c7200@1,14400@2,21600@0m0@5c7200@6,14400@6,21600@5e\"><v:formulas><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"prod #0 3 4\"/><v:f eqn=\"prod #0 5 4\"/><v:f eqn=\"prod #0 "
          "3 8\"/><v:f eqn=\"prod #0 1 8\"/><v:f eqn=\"sum 21600 0 @3\"/><v:f eqn=\"sum @4 21600 "
          "0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"prod @5 1 2\"/><v:f eqn=\"sum @7 @8 0\"/><v:f "
          "eqn=\"prod #0 7 8\"/><v:f eqn=\"prod @5 1 3\"/><v:f eqn=\"sum @1 @2 0\"/><v:f eqn=\"sum "
          "@12 @0 0\"/><v:f eqn=\"prod @13 1 4\"/><v:f eqn=\"sum @11 14400 "
          "@14\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@10;0,@8;10800,21600;21600,@9\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"bottomRight,#0\" "
          "yrange=\"0,11368\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCascadeUp,
          "<v:shapetype id=\"_x0000_t154\" coordsize=\"21600,21600\" o:spt=\"154\" adj=\"9600\" "
          "path=\"m0@2l21600,m,21600l21600@0e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 #0\"/><v:f eqn=\"prod @1 1 4\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"prod @2 1 "
          "2\"/><v:f eqn=\"sum @3 10800 0\"/><v:f eqn=\"sum @4 10800 0\"/><v:f eqn=\"sum @0 21600 "
          "@2\"/><v:f eqn=\"prod @7 1 2\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"10800,@4;0,@6;10800,@5;21600,@3\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"bottomRight,#0\" yrange=\"6171,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCascadeDown,
          "<v:shapetype id=\"_x0000_t155\" coordsize=\"21600,21600\" o:spt=\"155\" adj=\"9600\" "
          "path=\"m,l21600@2m0@0l21600,21600e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 #0\"/><v:f eqn=\"prod @1 1 4\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"prod @2 1 "
          "2\"/><v:f eqn=\"sum @3 10800 0\"/><v:f eqn=\"sum @4 10800 0\"/><v:f eqn=\"sum @0 21600 "
          "@2\"/><v:f eqn=\"prod @7 1 2\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"10800,@4;0,@3;10800,@5;21600,@6\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"6171,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextWave1,
          "<v:shapetype id=\"_x0000_t156\" coordsize=\"21600,21600\" o:spt=\"156\" "
          "adj=\"2809,10800\" "
          "path=\"m@25@0c@26@3@27@1@28@0m@21@4c@22@5@23@6@24@4e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod @0 41 9\"/><v:f eqn=\"prod @0 23 9\"/><v:f eqn=\"sum 0 0 "
          "@2\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 21600 0 "
          "@3\"/><v:f eqn=\"sum #1 0 10800\"/><v:f eqn=\"sum 21600 0 #1\"/><v:f eqn=\"prod @8 2 "
          "3\"/><v:f eqn=\"prod @8 4 3\"/><v:f eqn=\"prod @8 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@9\"/><v:f eqn=\"sum 21600 0 @10\"/><v:f eqn=\"sum 21600 0 @11\"/><v:f eqn=\"prod #1 2 "
          "3\"/><v:f eqn=\"prod #1 4 3\"/><v:f eqn=\"prod #1 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@15\"/><v:f eqn=\"sum 21600 0 @16\"/><v:f eqn=\"sum 21600 0 @17\"/><v:f eqn=\"if @7 @14 "
          "0\"/><v:f eqn=\"if @7 @13 @15\"/><v:f eqn=\"if @7 @12 @16\"/><v:f eqn=\"if @7 21600 "
          "@17\"/><v:f eqn=\"if @7 0 @20\"/><v:f eqn=\"if @7 @9 @19\"/><v:f eqn=\"if @7 @10 "
          "@18\"/><v:f eqn=\"if @7 @11 21600\"/><v:f eqn=\"sum @24 0 @21\"/><v:f eqn=\"sum @4 0 "
          "@0\"/><v:f eqn=\"max @21 @25\"/><v:f eqn=\"min @24 @28\"/><v:f eqn=\"prod @0 2 "
          "1\"/><v:f eqn=\"sum 21600 0 @33\"/><v:f eqn=\"mid @26 @27\"/><v:f eqn=\"mid @24 "
          "@28\"/><v:f eqn=\"mid @22 @23\"/><v:f eqn=\"mid @21 @25\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@35,@0;@38,10800;@37,@4;@36,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" yrange=\"0,4459\"/><v:h "
          "position=\"#1,bottomRight\" xrange=\"8640,12960\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextWave2,
          "<v:shapetype id=\"_x0000_t157\" coordsize=\"21600,21600\" o:spt=\"157\" "
          "adj=\"2809,10800\" "
          "path=\"m@25@0c@26@1@27@3@28@0m@21@4c@22@6@23@5@24@4e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod @0 41 9\"/><v:f eqn=\"prod @0 23 9\"/><v:f eqn=\"sum 0 0 "
          "@2\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 21600 0 "
          "@3\"/><v:f eqn=\"sum #1 0 10800\"/><v:f eqn=\"sum 21600 0 #1\"/><v:f eqn=\"prod @8 2 "
          "3\"/><v:f eqn=\"prod @8 4 3\"/><v:f eqn=\"prod @8 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@9\"/><v:f eqn=\"sum 21600 0 @10\"/><v:f eqn=\"sum 21600 0 @11\"/><v:f eqn=\"prod #1 2 "
          "3\"/><v:f eqn=\"prod #1 4 3\"/><v:f eqn=\"prod #1 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@15\"/><v:f eqn=\"sum 21600 0 @16\"/><v:f eqn=\"sum 21600 0 @17\"/><v:f eqn=\"if @7 @14 "
          "0\"/><v:f eqn=\"if @7 @13 @15\"/><v:f eqn=\"if @7 @12 @16\"/><v:f eqn=\"if @7 21600 "
          "@17\"/><v:f eqn=\"if @7 0 @20\"/><v:f eqn=\"if @7 @9 @19\"/><v:f eqn=\"if @7 @10 "
          "@18\"/><v:f eqn=\"if @7 @11 21600\"/><v:f eqn=\"sum @24 0 @21\"/><v:f eqn=\"sum @4 0 "
          "@0\"/><v:f eqn=\"max @21 @25\"/><v:f eqn=\"min @24 @28\"/><v:f eqn=\"prod @0 2 "
          "1\"/><v:f eqn=\"sum 21600 0 @33\"/><v:f eqn=\"mid @26 @27\"/><v:f eqn=\"mid @24 "
          "@28\"/><v:f eqn=\"mid @22 @23\"/><v:f eqn=\"mid @21 @25\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"@35,@0;@38,10800;@37,@4;@36,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" yrange=\"0,4459\"/><v:h "
          "position=\"#1,bottomRight\" xrange=\"8640,12960\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextWave3,
          "<v:shapetype id=\"_x0000_t158\" coordsize=\"21600,21600\" o:spt=\"158\" "
          "adj=\"1404,10800\" "
          "path=\"m@37@0c@38@3@39@1@40@0@41@3@42@1@43@0m@30@4c@31@5@32@6@33@4@34@5@35@6@36@4e\"><v:"
          "formulas><v:f eqn=\"val #0\"/><v:f eqn=\"prod @0 41 9\"/><v:f eqn=\"prod @0 23 "
          "9\"/><v:f eqn=\"sum 0 0 @2\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"sum 21600 0 "
          "@1\"/><v:f eqn=\"sum 21600 0 @3\"/><v:f eqn=\"sum #1 0 10800\"/><v:f eqn=\"sum 21600 0 "
          "#1\"/><v:f eqn=\"prod @8 1 3\"/><v:f eqn=\"prod @8 2 3\"/><v:f eqn=\"prod @8 4 "
          "3\"/><v:f eqn=\"prod @8 5 3\"/><v:f eqn=\"prod @8 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@9\"/><v:f eqn=\"sum 21600 0 @10\"/><v:f eqn=\"sum 21600 0 @8\"/><v:f eqn=\"sum 21600 0 "
          "@11\"/><v:f eqn=\"sum 21600 0 @12\"/><v:f eqn=\"sum 21600 0 @13\"/><v:f eqn=\"prod #1 1 "
          "3\"/><v:f eqn=\"prod #1 2 3\"/><v:f eqn=\"prod #1 4 3\"/><v:f eqn=\"prod #1 5 3\"/><v:f "
          "eqn=\"prod #1 2 1\"/><v:f eqn=\"sum 21600 0 @20\"/><v:f eqn=\"sum 21600 0 @21\"/><v:f "
          "eqn=\"sum 21600 0 @22\"/><v:f eqn=\"sum 21600 0 @23\"/><v:f eqn=\"sum 21600 0 "
          "@24\"/><v:f eqn=\"if @7 @19 0\"/><v:f eqn=\"if @7 @18 @20\"/><v:f eqn=\"if @7 @17 "
          "@21\"/><v:f eqn=\"if @7 @16 #1\"/><v:f eqn=\"if @7 @15 @22\"/><v:f eqn=\"if @7 @14 "
          "@23\"/><v:f eqn=\"if @7 21600 @24\"/><v:f eqn=\"if @7 0 @29\"/><v:f eqn=\"if @7 @9 "
          "@28\"/><v:f eqn=\"if @7 @10 @27\"/><v:f eqn=\"if @7 @8 @8\"/><v:f eqn=\"if @7 @11 "
          "@26\"/><v:f eqn=\"if @7 @12 @25\"/><v:f eqn=\"if @7 @13 21600\"/><v:f eqn=\"sum @36 0 "
          "@30\"/><v:f eqn=\"sum @4 0 @0\"/><v:f eqn=\"max @30 @37\"/><v:f eqn=\"min @36 "
          "@43\"/><v:f eqn=\"prod @0 2 1\"/><v:f eqn=\"sum 21600 0 @48\"/><v:f eqn=\"mid @36 "
          "@43\"/><v:f eqn=\"mid @30 @37\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"@40,@0;@51,10800;@33,@4;@50,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" yrange=\"0,2229\"/><v:h "
          "position=\"#1,bottomRight\" xrange=\"8640,12960\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextWave4,
          "<v:shapetype id=\"_x0000_t159\" coordsize=\"21600,21600\" o:spt=\"159\" "
          "adj=\"1404,10800\" "
          "path=\"m@37@0c@38@1@39@3@40@0@41@1@42@3@43@0m@30@4c@31@6@32@5@33@4@34@6@35@5@36@4e\"><v:"
          "formulas><v:f eqn=\"val #0\"/><v:f eqn=\"prod @0 41 9\"/><v:f eqn=\"prod @0 23 "
          "9\"/><v:f eqn=\"sum 0 0 @2\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"sum 21600 0 "
          "@1\"/><v:f eqn=\"sum 21600 0 @3\"/><v:f eqn=\"sum #1 0 10800\"/><v:f eqn=\"sum 21600 0 "
          "#1\"/><v:f eqn=\"prod @8 1 3\"/><v:f eqn=\"prod @8 2 3\"/><v:f eqn=\"prod @8 4 "
          "3\"/><v:f eqn=\"prod @8 5 3\"/><v:f eqn=\"prod @8 2 1\"/><v:f eqn=\"sum 21600 0 "
          "@9\"/><v:f eqn=\"sum 21600 0 @10\"/><v:f eqn=\"sum 21600 0 @8\"/><v:f eqn=\"sum 21600 0 "
          "@11\"/><v:f eqn=\"sum 21600 0 @12\"/><v:f eqn=\"sum 21600 0 @13\"/><v:f eqn=\"prod #1 1 "
          "3\"/><v:f eqn=\"prod #1 2 3\"/><v:f eqn=\"prod #1 4 3\"/><v:f eqn=\"prod #1 5 3\"/><v:f "
          "eqn=\"prod #1 2 1\"/><v:f eqn=\"sum 21600 0 @20\"/><v:f eqn=\"sum 21600 0 @21\"/><v:f "
          "eqn=\"sum 21600 0 @22\"/><v:f eqn=\"sum 21600 0 @23\"/><v:f eqn=\"sum 21600 0 "
          "@24\"/><v:f eqn=\"if @7 @19 0\"/><v:f eqn=\"if @7 @18 @20\"/><v:f eqn=\"if @7 @17 "
          "@21\"/><v:f eqn=\"if @7 @16 #1\"/><v:f eqn=\"if @7 @15 @22\"/><v:f eqn=\"if @7 @14 "
          "@23\"/><v:f eqn=\"if @7 21600 @24\"/><v:f eqn=\"if @7 0 @29\"/><v:f eqn=\"if @7 @9 "
          "@28\"/><v:f eqn=\"if @7 @10 @27\"/><v:f eqn=\"if @7 @8 @8\"/><v:f eqn=\"if @7 @11 "
          "@26\"/><v:f eqn=\"if @7 @12 @25\"/><v:f eqn=\"if @7 @13 21600\"/><v:f eqn=\"sum @36 0 "
          "@30\"/><v:f eqn=\"sum @4 0 @0\"/><v:f eqn=\"max @30 @37\"/><v:f eqn=\"min @36 "
          "@43\"/><v:f eqn=\"prod @0 2 1\"/><v:f eqn=\"sum 21600 0 @48\"/><v:f eqn=\"mid @36 "
          "@43\"/><v:f eqn=\"mid @30 @37\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"@40,@0;@51,10800;@33,@4;@50,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" yrange=\"0,2229\"/><v:h "
          "position=\"#1,bottomRight\" xrange=\"8640,12960\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextInflate,
          "<v:shapetype id=\"_x0000_t160\" coordsize=\"21600,21600\" o:spt=\"160\" adj=\"2945\" "
          "path=\"m0@0c7200@2,14400@2,21600@0m0@3c7200@4,14400@4,21600@3e\"><v:formulas><v:f "
          "eqn=\"val #0\"/><v:f eqn=\"prod #0 1 3\"/><v:f eqn=\"sum 0 0 @1\"/><v:f eqn=\"sum 21600 "
          "0 #0\"/><v:f eqn=\"sum 21600 0 @2\"/><v:f eqn=\"prod #0 2 3\"/><v:f eqn=\"sum 21600 0 "
          "@5\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"rect\"/><v:textpath "
          "on=\"t\" fitshape=\"t\" xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" "
          "yrange=\"0,4629\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextDeflate,
          "<v:shapetype id=\"_x0000_t161\" coordsize=\"21600,21600\" o:spt=\"161\" adj=\"4050\" "
          "path=\"m,c7200@0,14400@0,21600,m,21600c7200@1,14400@1,21600,21600e\"><v:formulas><v:f "
          "eqn=\"prod #0 4 3\"/><v:f eqn=\"sum 21600 0 @0\"/><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 #0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@2;0,10800;10800,@3;21600,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"0,8100\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextInflateBottom,
          "<v:shapetype id=\"_x0000_t162\" coordsize=\"21600,21600\" o:spt=\"162\" adj=\"14706\" "
          "path=\"m,l21600,m0@0c7200@2,14400@2,21600@0e\"><v:formulas><v:f eqn=\"val #0\"/><v:f "
          "eqn=\"prod #0 1 3\"/><v:f eqn=\"sum 28800 0 @1\"/><v:f eqn=\"prod #0 1 2\"/><v:f "
          "eqn=\"sum @1 7200 0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;0,@3;10800,21600;21600,@3\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" "
          "yrange=\"11148,21600\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextDeflateBottom,
          "<v:shapetype id=\"_x0000_t163\" coordsize=\"21600,21600\" o:spt=\"163\" adj=\"11475\" "
          "path=\"m,l21600,m,21600c7200@1,14400@1,21600,21600e\"><v:formulas><v:f eqn=\"prod #0 4 "
          "3\"/><v:f eqn=\"sum @0 0 7200\"/><v:f eqn=\"val #0\"/><v:f eqn=\"prod #0 2 3\"/><v:f "
          "eqn=\"sum @3 7200 0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;0,10800;10800,@2;21600,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"1350,21600\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextInflateTop,
          "<v:shapetype id=\"_x0000_t164\" coordsize=\"21600,21600\" o:spt=\"164\" adj=\"6894\" "
          "path=\"m0@0c7200@2,14400@2,21600@0m,21600r21600,e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"prod #0 1 3\"/><v:f eqn=\"sum 0 0 @1\"/><v:f eqn=\"prod #0 1 2\"/><v:f "
          "eqn=\"sum @3 10800 0\"/><v:f eqn=\"sum 21600 0 @1\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;0,@4;10800,21600;21600,@4\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"topLeft,#0\" "
          "yrange=\"0,10452\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextDeflateTop,
          "<v:shapetype id=\"_x0000_t165\" coordsize=\"21600,21600\" o:spt=\"165\" adj=\"10125\" "
          "path=\"m,c7200@0,14400@0,21600,m,21600r21600,e\"><v:formulas><v:f eqn=\"prod #0 4 "
          "3\"/><v:f eqn=\"val #0\"/><v:f eqn=\"prod #0 2 3\"/><v:f eqn=\"sum 21600 0 "
          "@2\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@1;0,10800;10800,21600;21600,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"0,20250\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextDeflateInflate,
          "<v:shapetype id=\"_x0000_t166\" coordsize=\"21600,21600\" o:spt=\"166\" adj=\"6054\" "
          "path=\"m,l21600,m,10125c7200@1,14400@1,21600,10125m,11475c7200@2,14400@2,21600,11475m,"
          "21600r21600,e\"><v:formulas><v:f eqn=\"prod #0 4 3\"/><v:f eqn=\"sum @0 0 4275\"/><v:f "
          "eqn=\"sum @0 0 2925\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"rect\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"1308,20292\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextDeflateInflateDeflate,
          "<v:shapetype id=\"_x0000_t167\" coordsize=\"21600,21600\" o:spt=\"167\" adj=\"6054\" "
          "path=\"m,l21600,m,6609c7200@1,14400@1,21600,6609m,7491c7200@2,14400@2,21600,7491m,"
          "14109c7200@4,14400@4,21600,14109m,14991c7200@3,14400@3,21600,14991m,21600r21600,e\"><v:"
          "formulas><v:f eqn=\"prod #0 4 3\"/><v:f eqn=\"sum @0 0 2791\"/><v:f eqn=\"sum @0 0 "
          "1909\"/><v:f eqn=\"sum 21600 0 @1\"/><v:f eqn=\"sum 21600 0 @2\"/></v:formulas><v:path "
          "textpathok=\"t\" o:connecttype=\"rect\"/><v:textpath on=\"t\" fitshape=\"t\" "
          "xscale=\"t\"/><v:handles><v:h position=\"center,#0\" "
          "yrange=\"854,9525\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextFadeRight,
          "<v:shapetype id=\"_x0000_t168\" coordsize=\"21600,21600\" o:spt=\"168\" adj=\"7200\" "
          "path=\"m,l21600@0m,21600l21600@1e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum 21600 0 @2\"/><v:f eqn=\"sum @1 "
          "21600 @0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@2;0,10800;10800,@3;21600,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"bottomRight,#0\" yrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextFadeLeft,
          "<v:shapetype id=\"_x0000_t169\" coordsize=\"21600,21600\" o:spt=\"169\" adj=\"7200\" "
          "path=\"m0@0l21600,m0@1l21600,21600e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum 21600 0 @2\"/><v:f eqn=\"sum @1 "
          "21600 @0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@2;0,10800;10800,@3;21600,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,10800\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextFadeUp,
          "<v:shapetype id=\"_x0000_t170\" coordsize=\"21600,21600\" o:spt=\"170\" adj=\"7200\" "
          "path=\"m@0,l@1,m,21600r21600,e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum 21600 "
          "0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum 21600 0 @2\"/><v:f eqn=\"sum @1 21600 "
          "@0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;@2,10800;10800,21600;@3,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"#0,topLeft\" xrange=\"0,10792\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextFadeDown,
          "<v:shapetype id=\"_x0000_t171\" coordsize=\"21600,21600\" o:spt=\"171\" adj=\"7200\" "
          "path=\"m,l21600,m@0,21600l@1,21600e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum 21600 0 @2\"/><v:f eqn=\"sum @1 "
          "21600 @0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;@2,10800;10800,21600;@3,10800\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"#0,bottomRight\" xrange=\"0,10792\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextSlantUp,
          "<v:shapetype id=\"_x0000_t172\" coordsize=\"21600,21600\" o:spt=\"172\" adj=\"12000\" "
          "path=\"m0@0l21600,m,21600l21600@1e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @2 10800 0\"/><v:f eqn=\"prod @1 "
          "1 2\"/><v:f eqn=\"sum @4 10800 0\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"10800,@2;0,@3;10800,@5;21600,@4\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"0,15429\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextSlantDown,
          "<v:shapetype id=\"_x0000_t173\" coordsize=\"21600,21600\" o:spt=\"173\" adj=\"9600\" "
          "path=\"m,l21600@1m0@0l21600,21600e\"><v:formulas><v:f eqn=\"val #0\"/><v:f eqn=\"sum "
          "21600 0 @0\"/><v:f eqn=\"prod #0 1 2\"/><v:f eqn=\"sum @2 10800 0\"/><v:f eqn=\"prod @1 "
          "1 2\"/><v:f eqn=\"sum @4 10800 0\"/></v:formulas><v:path textpathok=\"t\" "
          "o:connecttype=\"custom\" o:connectlocs=\"10800,@4;0,@2;10800,@3;21600,@5\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"topLeft,#0\" yrange=\"6171,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCanUp,
          "<v:shapetype id=\"_x0000_t174\" coordsize=\"21600,21600\" o:spt=\"174\" adj=\"18514\" "
          "path=\"m0@1qy10800,,21600@1m,21600qy10800@0,21600,21600e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"prod @1 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,0;0,@3;10800,@0;21600,@3\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"center,#0\" yrange=\"14400,21600\"/></v:handles><o:lock v:ext=\"edit\" "
          "text=\"t\" shapetype=\"t\"/></v:shapetype>" },
        { mso_sptTextCanDown,
          "<v:shapetype id=\"_x0000_t175\" coordsize=\"21600,21600\" o:spt=\"175\" adj=\"3086\" "
          "path=\"m,qy10800@0,21600,m0@1qy10800,21600,21600@1e\"><v:formulas><v:f eqn=\"val "
          "#0\"/><v:f eqn=\"sum 21600 0 #0\"/><v:f eqn=\"prod @1 1 2\"/><v:f eqn=\"sum @2 10800 "
          "0\"/></v:formulas><v:path textpathok=\"t\" o:connecttype=\"custom\" "
          "o:connectlocs=\"10800,@0;0,@2;10800,21600;21600,@2\" "
          "o:connectangles=\"270,180,90,0\"/><v:textpath on=\"t\" fitshape=\"t\"/><v:handles><v:h "
          "position=\"center,#0\" yrange=\"0,7200\"/></v:handles><o:lock v:ext=\"edit\" text=\"t\" "
          "shapetype=\"t\"/></v:shapetype>" }
    };

    auto i(aTypeToMarkupMap.find(eShapeType));
    return i == aTypeToMarkupMap.end() ? OString() : i->second;
}

void FontworkHelpers::collectCharColorProps(const uno::Reference<text::XText>& rXText,
                                            std::vector<beans::PropertyValue>& rCharPropVec)
{
    if (!rXText.is())
        return;
    uno::Reference<text::XTextCursor> rXTextCursor = rXText->createTextCursor();
    rXTextCursor->gotoStart(false);
    rXTextCursor->gotoEnd(true);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(rXText, uno::UNO_QUERY);
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

            // We have found a non-empty run. Collect its simple color properties.
            const std::array<OUString, 6> aNamesArray = { u"CharColor",
                                                          u"CharLumMod",
                                                          u"CharLumOff",
                                                          u"CharColorTheme",
                                                          u"CharColorThemeReference",
                                                          u"CharTransparence" };
            for (const auto& propName : aNamesArray)
            {
                if (xRunPropSetInfo->hasPropertyByName(propName))
                    rCharPropVec.push_back(comphelper::makePropertyValue(
                        propName, xRunPropSet->getPropertyValue(propName)));
            }
            return;
        }
    }
}

void FontworkHelpers::applyPropsToRuns(const std::vector<beans::PropertyValue>& rTextPropVec,
                                       uno::Reference<text::XText>& rXText)
{
    if (!rXText.is())
        return;
    uno::Reference<text::XTextCursor> xTextCursor = rXText->createTextCursor();
    xTextCursor->gotoStart(false);
    xTextCursor->gotoEnd(true);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(rXText, uno::UNO_QUERY);
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
            uno::Reference<beans::XPropertySet> xRunPropSet(xRun, uno::UNO_QUERY);
            if (!xRunPropSet.is())
                continue;
            auto xRunPropSetInfo = xRunPropSet->getPropertySetInfo();
            if (!xRunPropSetInfo.is())
                continue;

            for (const beans::PropertyValue& rProp : rTextPropVec)
            {
                if (xRunPropSetInfo->hasPropertyByName(rProp.Name)
                    && !(xRunPropSetInfo->getPropertyByName(rProp.Name).Attributes
                         & beans::PropertyAttribute::READONLY)
                    && rProp.Name != u"CharInteropGrabBag")
                {
                    xRunPropSet->setPropertyValue(rProp.Name, rProp.Value);
                }
            }
        }
    }
}

void FontworkHelpers::createCharFillPropsFromShape(
    const uno::Reference<beans::XPropertySet>& rXPropSet,
    std::vector<beans::PropertyValue>& rCharPropVec)
{
    auto xPropSetInfo = rXPropSet->getPropertySetInfo();
    if (!xPropSetInfo.is())
        return;
    // CharColor contains the color including all color transformations
    // FillColor contains darken and lighten but not transparency
    sal_Int32 nColorRGB = 0;
    if (xPropSetInfo->hasPropertyByName(u"FillColor")
        && (rXPropSet->getPropertyValue(u"FillColor") >>= nColorRGB))
    {
        ::Color aColor(ColorTransparency, nColorRGB);
        sal_Int16 nTransPercent = 0;
        if (xPropSetInfo->hasPropertyByName(u"FillTransparence")
            && (rXPropSet->getPropertyValue(u"FillTransparence") >>= nTransPercent))
        {
            sal_uInt8 nAlpha = 255 - sal_uInt8(std::lround(double(nTransPercent) * 2.55));
            aColor.SetAlpha(nAlpha);
        }
        rCharPropVec.push_back(comphelper::makePropertyValue(u"CharColor", sal_Int32(aColor)));
    }

    const std::array<OUString, 5> aCharPropNames
        = { u"CharColorLumMod", u"CharColorLumOff", u"CharColorTheme", u"CharColorThemeReference",
            u"CharTransparence" };
    const std::array<OUString, 5> aShapePropNames
        = { u"FillColorLumMod", u"FillColorLumOff", u"FillColorTheme", u"FillColorThemeReference",
            u"FillTransparence" };
    for (size_t i = 0; i < 5; i++)
    {
        if (xPropSetInfo->hasPropertyByName(aShapePropNames[i]))
            rCharPropVec.push_back(comphelper::makePropertyValue(
                aCharPropNames[i], rXPropSet->getPropertyValue(aShapePropNames[i])));
    }
}

bool FontworkHelpers::createPrstDashFromLineDash(const drawing::LineDash& rLineDash,
                                                 const drawing::LineCap& rLineCap,
                                                 OUString& rsPrstDash)
{
    bool bIsConverted = false;

    bool bIsRelative(rLineDash.Style == drawing::DashStyle_RECTRELATIVE
                     || rLineDash.Style == drawing::DashStyle_ROUNDRELATIVE);
    if (bIsRelative && rLineDash.Dots == 1)
    { // The length were tweaked on import in case of prstDash. Revert it here.
        sal_uInt32 nDotLen = rLineDash.DotLen;
        sal_uInt32 nDashLen = rLineDash.DashLen;
        sal_uInt32 nDistance = rLineDash.Distance;
        if (rLineCap != drawing::LineCap_BUTT && nDistance >= 99)
        {
            nDistance -= 99;
            nDotLen += 99;
            if (nDashLen > 0)
                nDashLen += 99;
        }

        // LO uses length 0 for 100%, if the attribute is missing in ODF.
        // Other applications might write 100%. Make is unique for the conditions.
        if (nDotLen == 0)
            nDotLen = 100;
        if (nDashLen == 0 && rLineDash.Dashes > 0)
            nDashLen = 100;

        bIsConverted = true;
        if (nDotLen == 100 && rLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            rsPrstDash = u"dot";
        else if (nDotLen == 400 && rLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            rsPrstDash = u"dash";
        else if (nDotLen == 400 && rLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 300)
            rsPrstDash = u"dashDot";
        else if (nDotLen == 800 && rLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            rsPrstDash = u"lgDash";
        else if (nDotLen == 800 && rLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 300)
            rsPrstDash = u"lgDashDot";
        else if (nDotLen == 800 && rLineDash.Dashes == 2 && nDashLen == 100 && nDistance == 300)
            rsPrstDash = u"lgDashDotDot";
        else if (nDotLen == 100 && rLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 100)
            rsPrstDash = u"sysDot";
        else if (nDotLen == 300 && rLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 100)
            rsPrstDash = u"sysDash";
        else if (nDotLen == 300 && rLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 100)
            rsPrstDash = u"sysDashDot";
        else if (nDotLen == 300 && rLineDash.Dashes == 2 && nDashLen == 100 && nDistance == 100)
            rsPrstDash = "sysDashDotDot";
        else
            bIsConverted = false;
    }
    return bIsConverted;
}

bool FontworkHelpers::getThemeColorFromShape(
    OUString const& rPropertyName, const uno::Reference<beans::XPropertySet>& xPropertySet,
    model::ThemeColor& aThemeColor)
{
    auto xPropSetInfo = xPropertySet->getPropertySetInfo();
    if (!xPropSetInfo.is())
        return false;
    uno::Reference<util::XThemeColor> xThemeColor;
    if (xPropSetInfo->hasPropertyByName(rPropertyName)
        && (xPropertySet->getPropertyValue(rPropertyName) >>= xThemeColor) && xThemeColor.is())
    {
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        if (aThemeColor.getType() == model::ThemeColorType::Unknown)
            return false;
        else
            return true;
    }
    return false;
}

namespace
{
// Returns the string to be used in w14:schemeClr in case of w14:textOutline or w14:textFill
OUString lcl_getW14MarkupStringForThemeColor(const model::ThemeColor& rThemeColor)
{
    const std::array<OUString, 12> W14ColorNames
        = { u"tx1",     u"bg1",     u"tx2",     u"bg2",     u"accent1", u"accent2",
            u"accent3", u"accent4", u"accent5", u"accent6", u"hlink",   u"folHlink" };
    const sal_uInt8 nClrNameIndex = std::clamp<sal_uInt8>(
        sal_Int32(rThemeColor.getType()), sal_Int32(model::ThemeColorType::Dark1),
        sal_Int32(model::ThemeColorType::FollowedHyperlink));
    return W14ColorNames[nClrNameIndex];
}

// Returns the string to be used in w:themeColor. It is exported via CharThemeColor.
OUString lcl_getWMarkupStringForThemeColor(const model::ThemeColor& rThemeColor)
{
    const std::array<OUString, 12> WColorNames
        = { u"text1",   u"background1", u"text2",     u"background2",
            u"accent1", u"accent2",     u"accent3",   u"accent4",
            u"accent5", u"accent6",     u"hyperlink", u"followedHyperlink" };
    const sal_uInt8 nClrNameIndex = std::clamp<sal_uInt8>(
        sal_Int32(rThemeColor.getType()), sal_Int32(model::ThemeColorType::Dark1),
        sal_Int32(model::ThemeColorType::FollowedHyperlink));
    return WColorNames[nClrNameIndex];
}

// Puts the value of the first occurrence of rType in rThemeColor into rValue and returns true.
// If such does not exist, rValue is unchanged and the method returns false.
bool lcl_getThemeColorTransformationValue(const model::ThemeColor& rThemeColor,
                                          const model::TransformationType& rType, sal_Int16& rValue)
{
    const std::vector<model::Transformation> aTransVec(rThemeColor.getTransformations());
    auto bItemFound
        = [rType](const model::Transformation& rTrans) { return rType == rTrans.meType; };
    auto pIt = std::find_if(aTransVec.begin(), aTransVec.end(), bItemFound);
    if (pIt == aTransVec.end())
        return false;
    rValue = (*pIt).mnValue;
    return true;
}

// Adds the child elements 'lumMod' and/or 'lumOff' to 'schemeClr' maCurrentElement of
// pGrabStack, if such exist in rThemeColor. As of Feb 2023, 'alpha' is not contained in the
// the maTransformations of rThemeColor.
void lcl_addColorTransformationToGrabBagStack(const model::ThemeColor& rThemeColor,
                                              std::unique_ptr<oox::GrabBagStack>& pGrabBagStack)
{
    if (pGrabBagStack == nullptr)
        return;
    for (auto const& rColorTransform : rThemeColor.getTransformations())
    {
        switch (rColorTransform.meType)
        {
            case model::TransformationType::LumMod:
                pGrabBagStack->push("lumMod");
                pGrabBagStack->push("attributes");
                pGrabBagStack->addInt32("val", rColorTransform.mnValue * 10);
                pGrabBagStack->pop();
                pGrabBagStack->pop();
                break;
            case model::TransformationType::LumOff:
                pGrabBagStack->push("lumOff");
                pGrabBagStack->push("attributes");
                pGrabBagStack->addInt32("val", rColorTransform.mnValue * 10);
                pGrabBagStack->pop();
                pGrabBagStack->pop();
                break;
            default: // other child element can be added later if needed for Fontwork
                break;
        }
    }
}
} // end namespace

void FontworkHelpers::createCharInteropGrabBagUpdatesFromShapeProps(
    const uno::Reference<beans::XPropertySet>& rXPropSet,
    std::vector<beans::PropertyValue>& rUpdatePropVec)
{
    auto xPropSetInfo = rXPropSet->getPropertySetInfo();
    if (!xPropSetInfo.is())
        return;

    // GrabBagStack is a special tool for handling the hierarchy in a GrabBag
    std::unique_ptr<oox::GrabBagStack> pGrabBagStack;

    // CharTextFillTextEffect
    pGrabBagStack.reset(new oox::GrabBagStack("textFill"));
    drawing::FillStyle eFillStyle = drawing::FillStyle_SOLID;
    if (xPropSetInfo->hasPropertyByName(u"FillStyle"))
        rXPropSet->getPropertyValue(u"FillStyle") >>= eFillStyle;
    switch (eFillStyle)
    {
        case drawing::FillStyle_NONE:
        {
            pGrabBagStack->appendElement("noFill", uno::Any());
            break;
        }
        case drawing::FillStyle_GRADIENT: // ToDo
        {
            // fallback
            pGrabBagStack->push("solidFill");
            pGrabBagStack->push("srgbClr");
            pGrabBagStack->push("attributes");
            ::Color aColor(ColorTransparency, 7512015); // LO default fill
            pGrabBagStack->addString("val", aColor.AsRGBHexString());
            // pop() calls are in the final getRootProperty() method
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            pGrabBagStack->push("solidFill");
            model::ThemeColor aThemeColor;
            // It is either "schemeClr" or "srgbClr".
            if (FontworkHelpers::getThemeColorFromShape("FillColorThemeReference", rXPropSet,
                                                        aThemeColor))
            {
                pGrabBagStack->push("schemeClr");
                pGrabBagStack->push("attributes");
                pGrabBagStack->addString("val", lcl_getW14MarkupStringForThemeColor(aThemeColor));
                pGrabBagStack->pop(); // maCurrentElement:'schemeClr', maPropertyList:'attributes'
                lcl_addColorTransformationToGrabBagStack(aThemeColor, pGrabBagStack);
                // maCurrentElement:'schemeClr', maPropertyList:'attributes', maybe 'lumMod' and
                // maybe 'lumOff'
            }
            else
            {
                pGrabBagStack->push("srgbClr");
                sal_Int32 nFillColor(0);
                if (xPropSetInfo->hasPropertyByName(u"FillColor"))
                    rXPropSet->getPropertyValue(u"FillColor") >>= nFillColor;
                pGrabBagStack->push("attributes");
                ::Color aColor(ColorTransparency, nFillColor);
                pGrabBagStack->addString("val", aColor.AsRGBHexString());
                pGrabBagStack->pop();
                // maCurrentElement:'srgbClr', maPropertyList:'attributes'
            }

            sal_Int16 nFillTransparence(0);
            if (xPropSetInfo->hasPropertyByName(u"FillTransparence"))
                rXPropSet->getPropertyValue(u"FillTransparence") >>= nFillTransparence;
            if (nFillTransparence != 0)
            {
                pGrabBagStack->push("alpha");
                pGrabBagStack->push("attributes");
                pGrabBagStack->addInt32("val", nFillTransparence * 1000);
            }
            // all remaining pop() calls are in the final getRootProperty() method
            break;
        }
        default: // BITMAP is VML only export and does not arrive here. HATCH has to be VML only
                 // export too, but is not yet implemented.
            break;
    }
    // resolve the stack and put resulting PropertyValue into the update vector
    beans::PropertyValue aCharTextFillTextEffect;
    aCharTextFillTextEffect.Name = "CharTextFillTextEffect";
    aCharTextFillTextEffect.Value <<= pGrabBagStack->getRootProperty();
    rUpdatePropVec.push_back(aCharTextFillTextEffect);

    // CharTextOutlineTextEffect
    pGrabBagStack.reset(new oox::GrabBagStack("textOutline"));

    // attributes
    pGrabBagStack->push("attributes");
    // line width
    sal_Int32 nLineWidth(0);
    if (xPropSetInfo->hasPropertyByName(u"LineWidth"))
        rXPropSet->getPropertyValue(u"LineWidth") >>= nLineWidth;
    pGrabBagStack->addInt32("w", nLineWidth * 360);
    // cap for dashes
    drawing::LineCap eLineCap = drawing::LineCap_BUTT;
    if (xPropSetInfo->hasPropertyByName(u"LineCap"))
        rXPropSet->getPropertyValue(u"LineCap") >>= eLineCap;
    OUString sCap = u"flat";
    if (eLineCap == drawing::LineCap_ROUND)
        sCap = u"rnd";
    else if (eLineCap == drawing::LineCap_SQUARE)
        sCap = u"sq";
    pGrabBagStack->addString("cap", sCap);
    // LO has no compound lines and always centers the lines
    pGrabBagStack->addString("cmpd", u"sng");
    pGrabBagStack->addString("alng", u"ctr");
    pGrabBagStack->pop();
    // maCurrentElement:'textOutline', maPropertyList:'attributes'

    // style
    drawing::LineStyle eLineStyle = drawing::LineStyle_NONE;
    if (xPropSetInfo->hasPropertyByName(u"LineStyle"))
        rXPropSet->getPropertyValue(u"LineStyle") >>= eLineStyle;
    // 'dashed' is not a separate style in Word. Word has a style 'gradFill', but that is not yet
    // implemented in LO. So only 'noFill' and 'solidFill'.
    if (eLineStyle == drawing::LineStyle_NONE)
    {
        pGrabBagStack->appendElement("noFill", uno::Any());
    }
    else
    {
        pGrabBagStack->push("solidFill");
        // It is either "schemeClr" or "srgbClr".
        model::ThemeColor aThemeColor;
        if (FontworkHelpers::getThemeColorFromShape("LineColorThemeReference", rXPropSet,
                                                    aThemeColor))
        {
            pGrabBagStack->push("schemeClr");
            pGrabBagStack->push("attributes");
            pGrabBagStack->addString("val", lcl_getW14MarkupStringForThemeColor(aThemeColor));
            pGrabBagStack->pop();
            lcl_addColorTransformationToGrabBagStack(aThemeColor, pGrabBagStack);
            // maCurrentElement:'schemeClr', maPropertylist:'attributes'
        }
        else // not a theme color
        {
            pGrabBagStack->push("srgbClr");
            pGrabBagStack->push("attributes");
            sal_Int32 nLineColor(0);
            if (xPropSetInfo->hasPropertyByName(u"LineColor"))
                rXPropSet->getPropertyValue(u"LineColor") >>= nLineColor;
            ::Color aColor(ColorTransparency, nLineColor);
            pGrabBagStack->addString("val", aColor.AsRGBHexString());
            pGrabBagStack->pop();
            // maCurrentElement:'srgbClr', maPropertylist:'attributes'
        }

        sal_Int16 nLineTransparence(0);
        if (xPropSetInfo->hasPropertyByName(u"LineTransparence"))
            rXPropSet->getPropertyValue(u"LineTransparence") >>= nLineTransparence;
        if (nLineTransparence != 0)
        {
            pGrabBagStack->push("alpha");
            pGrabBagStack->push("attributes");
            pGrabBagStack->addInt32("val", nLineTransparence * 1000);
            pGrabBagStack->pop(); // maCurrentElement: 'alpha'
            pGrabBagStack->pop(); // maCurrentElement: 'srgbClr' or 'schemeClr'
        }
        pGrabBagStack->pop();
        // maCurrentElement:'solidFill', maPropertyList:either 'srgbClr' or 'schemeClr
        pGrabBagStack->pop();
    }
    // maCurrentElement:'textOutline', maPropertyList:'attributes' and either 'noFill' or 'solidFill'

    // prstDash
    if (eLineStyle == drawing::LineStyle_DASH)
    {
        pGrabBagStack->push("prstDash");
        OUString sPrstDash = u"sysDot";
        drawing::LineDash aLineDash;
        if (xPropSetInfo->hasPropertyByName(u"LineDash")
            && (rXPropSet->getPropertyValue(u"LineDash") >>= aLineDash))
        {
            // The outline of abc-transform in Word is not able to use custDash. But we know the line
            // is dashed. We keep "sysDot" as fallback in case no prstDash is detected.
            FontworkHelpers::createPrstDashFromLineDash(aLineDash, eLineCap, sPrstDash);
        }
        else
        {
            // ToDo: There may be a named dash style, but that is unlikely for Fontwork shapes. So
            // I skip it for now and use the "sysDot" fallback.
        }
        pGrabBagStack->push("attributes");
        pGrabBagStack->addString("val", sPrstDash);
        pGrabBagStack->pop(); // maCurrentElement:'prstDash'
        pGrabBagStack->pop(); // maCurrentElement:'textOutline'
    }
    // maCurrentElement:'textOutline', maPropertyList:'attributes', either 'noFill' or 'solidFill',
    // and  maybe 'prstDash'.

    // LineJoint, can be 'round', 'bevel' or 'miter' in Word
    drawing::LineJoint eLineJoint = drawing::LineJoint_NONE;
    if (xPropSetInfo->hasPropertyByName(u"LineJoint"))
        rXPropSet->getPropertyValue(u"LineJoint") >>= eLineJoint;
    if (eLineJoint == drawing::LineJoint_NONE || eLineJoint == drawing::LineJoint_BEVEL)
        pGrabBagStack->appendElement("bevel", uno::Any());
    else if (eLineJoint == drawing::LineJoint_ROUND)
        pGrabBagStack->appendElement("round", uno::Any());
    else // MITER or deprecated MIDDLE
    {
        pGrabBagStack->push("miter");
        pGrabBagStack->push("attributes");
        pGrabBagStack->addInt32("lim", 0); // As of Feb. 2023 LO cannot render other values.
        pGrabBagStack->pop(); // maCurrentElement:'attributes'
        pGrabBagStack->pop(); // maCurrentElement:'miter'
    }
    // maCurrentElement:'textOutline', maPropertyList:'attributes', either 'noFill' or
    // 'solidFill', maybe 'prstDash', and either 'bevel', 'round' or 'miter'.

    // resolve the stack and put resulting PropertyValue into the update vector
    beans::PropertyValue aCharTextOutlineTextEffect;
    aCharTextOutlineTextEffect.Name = "CharTextOutlineTextEffect";
    aCharTextOutlineTextEffect.Value <<= pGrabBagStack->getRootProperty();
    rUpdatePropVec.push_back(aCharTextOutlineTextEffect);

    // CharThemeOriginalColor, CharThemeColor, and CharThemeColorShade or CharThemeColorTint will be
    // used for <w:color> element. That is evaluated by applications, which do not understand w14
    // namespace, or if w14:textFill is omitted.
    model::ThemeColor aThemeColor;
    if (FontworkHelpers::getThemeColorFromShape("FillColorThemeReference", rXPropSet, aThemeColor))
    {
        // CharThemeColor
        beans::PropertyValue aCharThemeColor;
        aCharThemeColor.Name = u"CharThemeColor";
        aCharThemeColor.Value <<= lcl_getWMarkupStringForThemeColor(aThemeColor);
        rUpdatePropVec.push_back(aCharThemeColor);

        // CharThemeColorShade or CharThemeColorTint
        // MS Office uses themeTint and themeShade on the luminance in a HSL color space, see 2.1.72
        // in [MS-OI29500]. That is different from OOXML specification.
        // We made two assumption here: (1) If LumOff exists and is not zero, it is a 'tint'.
        // (2) LumMod + LumOff == 10000;
        sal_Int16 nLumMod;
        if (lcl_getThemeColorTransformationValue(aThemeColor, model::TransformationType::LumMod,
                                                 nLumMod))
        {
            sal_Int16 nLumOff;
            bool bIsTint = lcl_getThemeColorTransformationValue(
                               aThemeColor, model::TransformationType::LumOff, nLumOff)
                           && nLumOff != 0;
            sal_uInt8 nValue
                = std::clamp<sal_uInt8>(lround(double(nLumMod) * 255.0 / 10000.0), 0, 255);
            OUString sValue = OUString::number(nValue, 16);

            beans::PropertyValue aCharThemeTintOrShade;
            aCharThemeTintOrShade.Name = bIsTint ? u"CharThemeColorTint" : u"CharThemeColorShade";
            aCharThemeTintOrShade.Value <<= sValue;
            rUpdatePropVec.push_back(aCharThemeTintOrShade);
        }
    }
    // ToDo: Are FillColorLumMod, FillColorLumOff and FillColorTheme possible without
    // FillColorThemeReference? If yes, we need an 'else' part here.

    // CharThemeOriginalColor.
    beans::PropertyValue aCharThemeOriginalColor;
    sal_Int32 nFillColor(0);
    if (xPropSetInfo->hasPropertyByName(u"FillColor"))
        rXPropSet->getPropertyValue(u"FillColor") >>= nFillColor;
    aCharThemeOriginalColor.Name = u"CharThemeOriginalColor";
    ::Color aColor(ColorTransparency, nFillColor);
    aCharThemeOriginalColor.Value <<= aColor.AsRGBHEXString();
    rUpdatePropVec.push_back(aCharThemeOriginalColor);
}

void FontworkHelpers::applyUpdatesToCharInteropGrabBag(
    const std::vector<beans::PropertyValue>& rUpdatePropVec, uno::Reference<text::XText>& rXText)
{
    if (!rXText.is())
        return;
    uno::Reference<text::XTextCursor> rXTextCursor = rXText->createTextCursor();
    rXTextCursor->gotoStart(false);
    rXTextCursor->gotoEnd(true);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(rXText, uno::UNO_QUERY);
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

            // Now apply the updates to the CharInteropGrabBag of this run
            uno::Sequence<beans::PropertyValue> aCharInteropGrabBagSeq;
            if (xRunPropSetInfo->hasPropertyByName("CharInteropGrabBag"))
                xRunPropSet->getPropertyValue("CharInteropGrabBag") >>= aCharInteropGrabBagSeq;
            comphelper::SequenceAsHashMap aGrabBagMap(aCharInteropGrabBagSeq);
            for (const auto& rProp : rUpdatePropVec)
            {
                aGrabBagMap[rProp.Name] = rProp.Value; // [] inserts if not exists
            }
            xRunPropSet->setPropertyValue("CharInteropGrabBag",
                                          uno::Any(aGrabBagMap.getAsConstPropertyValueList()));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */