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

#include <sal/config.h>

#include <algorithm>

#include <svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <svgnode.hxx>
#include <svgdocument.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <svggradientnode.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svgclippathnode.hxx>
#include <svgmasknode.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svgmarkernode.hxx>
#include <svgpatternnode.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagehierarchyprimitive2d.hxx>
#include <o3tl/unit_conversion.hxx>

const int nStyleDepthLimit = 1024;

namespace svgio::svgreader
{
        static basegfx::B2DLineJoin StrokeLinejoinToB2DLineJoin(StrokeLinejoin aStrokeLinejoin)
        {
            if(StrokeLinejoin::round == aStrokeLinejoin)
            {
                return basegfx::B2DLineJoin::Round;
            }
            else if(StrokeLinejoin::bevel == aStrokeLinejoin)
            {
                return basegfx::B2DLineJoin::Bevel;
            }

            return basegfx::B2DLineJoin::Miter;
        }

        static css::drawing::LineCap StrokeLinecapToDrawingLineCap(StrokeLinecap aStrokeLinecap)
        {
            switch(aStrokeLinecap)
            {
                default: /* StrokeLinecap::notset, StrokeLinecap::butt */
                {
                    return css::drawing::LineCap_BUTT;
                }
                case StrokeLinecap::round:
                {
                    return css::drawing::LineCap_ROUND;
                }
                case StrokeLinecap::square:
                {
                    return css::drawing::LineCap_SQUARE;
                }
            }
        }

        FontStretch getWider(FontStretch aSource)
        {
            switch(aSource)
            {
                case FontStretch::ultra_condensed: aSource = FontStretch::extra_condensed; break;
                case FontStretch::extra_condensed: aSource = FontStretch::condensed; break;
                case FontStretch::condensed: aSource = FontStretch::semi_condensed; break;
                case FontStretch::semi_condensed: aSource = FontStretch::normal; break;
                case FontStretch::normal: aSource = FontStretch::semi_expanded; break;
                case FontStretch::semi_expanded: aSource = FontStretch::expanded; break;
                case FontStretch::expanded: aSource = FontStretch::extra_expanded; break;
                case FontStretch::extra_expanded: aSource = FontStretch::ultra_expanded; break;
                default: break;
            }

            return aSource;
        }

        FontStretch getNarrower(FontStretch aSource)
        {
            switch(aSource)
            {
                case FontStretch::extra_condensed: aSource = FontStretch::ultra_condensed; break;
                case FontStretch::condensed: aSource = FontStretch::extra_condensed; break;
                case FontStretch::semi_condensed: aSource = FontStretch::condensed; break;
                case FontStretch::normal: aSource = FontStretch::semi_condensed; break;
                case FontStretch::semi_expanded: aSource = FontStretch::normal; break;
                case FontStretch::expanded: aSource = FontStretch::semi_expanded; break;
                case FontStretch::extra_expanded: aSource = FontStretch::expanded; break;
                case FontStretch::ultra_expanded: aSource = FontStretch::extra_expanded; break;
                default: break;
            }

            return aSource;
        }

        FontWeight getBolder(FontWeight aSource)
        {
            switch(aSource)
            {
                case FontWeight::N100: aSource = FontWeight::N200; break;
                case FontWeight::N200: aSource = FontWeight::N300; break;
                case FontWeight::N300: aSource = FontWeight::N400; break;
                case FontWeight::N400: aSource = FontWeight::N500; break;
                case FontWeight::N500: aSource = FontWeight::N600; break;
                case FontWeight::N600: aSource = FontWeight::N700; break;
                case FontWeight::N700: aSource = FontWeight::N800; break;
                case FontWeight::N800: aSource = FontWeight::N900; break;
                default: break;
            }

            return aSource;
        }

        FontWeight getLighter(FontWeight aSource)
        {
            switch(aSource)
            {
                case FontWeight::N200: aSource = FontWeight::N100; break;
                case FontWeight::N300: aSource = FontWeight::N200; break;
                case FontWeight::N400: aSource = FontWeight::N300; break;
                case FontWeight::N500: aSource = FontWeight::N400; break;
                case FontWeight::N600: aSource = FontWeight::N500; break;
                case FontWeight::N700: aSource = FontWeight::N600; break;
                case FontWeight::N800: aSource = FontWeight::N700; break;
                case FontWeight::N900: aSource = FontWeight::N800; break;
                default: break;
            }

            return aSource;
        }

        ::FontWeight getVclFontWeight(FontWeight aSource)
        {
            ::FontWeight nRetval(WEIGHT_NORMAL);

            switch(aSource)
            {
                case FontWeight::N100: nRetval = WEIGHT_ULTRALIGHT; break;
                case FontWeight::N200: nRetval = WEIGHT_LIGHT; break;
                case FontWeight::N300: nRetval = WEIGHT_SEMILIGHT; break;
                case FontWeight::N400: nRetval = WEIGHT_NORMAL; break;
                case FontWeight::N500: nRetval = WEIGHT_MEDIUM; break;
                case FontWeight::N600: nRetval = WEIGHT_SEMIBOLD; break;
                case FontWeight::N700: nRetval = WEIGHT_BOLD; break;
                case FontWeight::N800: nRetval = WEIGHT_ULTRABOLD; break;
                case FontWeight::N900: nRetval = WEIGHT_BLACK; break;
                default: break;
            }

            return nRetval;
        }

        void SvgStyleAttributes::readCssStyle(const OUString& rCandidate)
        {
            const sal_Int32 nLen(rCandidate.getLength());
            sal_Int32 nPos(0);

            while(nPos < nLen)
            {
                // get TokenName
                OUStringBuffer aTokenName;
                skip_char(rCandidate, u' ', nPos, nLen);
                copyString(rCandidate, nPos, aTokenName, nLen);

                if (aTokenName.isEmpty())
                {
                    // if no TokenName advance one by force to avoid death loop, continue
                    OSL_ENSURE(false, "Could not interpret on current position, advancing one byte (!)");
                    nPos++;
                    continue;
                }

                // get TokenValue
                OUStringBuffer aTokenValue;
                skip_char(rCandidate, u' ', u':', nPos, nLen);
                copyToLimiter(rCandidate, u';', nPos, aTokenValue, nLen);
                skip_char(rCandidate, u' ', u';', nPos, nLen);

                if (aTokenValue.isEmpty())
                {
                    // no value - continue
                    continue;
                }

                // generate OUStrings
                const OUString aOUTokenName(aTokenName.makeStringAndClear());
                OUString aOUTokenValue(aTokenValue.makeStringAndClear());

                // check for '!important' CssStyle mark, currently not supported
                // but needs to be extracted for correct parsing
                OUString aTokenImportant("!important");
                const sal_Int32 nIndexTokenImportant(aOUTokenValue.indexOf(aTokenImportant));

                if(-1 != nIndexTokenImportant)
                {
                    // if there currently just remove it and remove spaces to have the value only
                    OUString aNewOUTokenValue;

                    if(nIndexTokenImportant > 0)
                    {
                        // copy content before token
                        aNewOUTokenValue += aOUTokenValue.subView(0, nIndexTokenImportant);
                    }

                    if(aOUTokenValue.getLength() > nIndexTokenImportant + aTokenImportant.getLength())
                    {
                        // copy content after token
                        aNewOUTokenValue += aOUTokenValue.subView(nIndexTokenImportant + aTokenImportant.getLength());
                    }

                    // remove spaces
                    aOUTokenValue = aNewOUTokenValue.trim();
                }

                // valid token-value pair, parse it
                parseStyleAttribute(StrToSVGToken(aOUTokenName, true), aOUTokenValue, true);
            }
        }

        const SvgStyleAttributes* SvgStyleAttributes::getParentStyle() const
        {
            if(getCssStyleParent())
            {
                return getCssStyleParent();
            }

            if(mrOwner.supportsParentStyle() && mrOwner.getParent())
            {
                return mrOwner.getParent()->getSvgStyleAttributes();
            }

            return nullptr;
        }

        void SvgStyleAttributes::add_text(
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            drawinglayer::primitive2d::Primitive2DContainer&& rSource) const
        {
            if(rSource.empty())
                return;

            // at this point the primitives in rSource are of type TextSimplePortionPrimitive2D
            // or TextDecoratedPortionPrimitive2D and have the Fill Color (pAttributes->getFill())
            // set. When another fill is used and also evtl. stroke is set it gets necessary to
            // dismantle to geometry and add needed primitives
            const basegfx::BColor* pFill = getFill();
            const SvgGradientNode* pFillGradient = getSvgGradientNodeFill();
            const SvgPatternNode* pFillPattern = getSvgPatternNodeFill();
            const basegfx::BColor* pStroke = getStroke();
            const SvgGradientNode* pStrokeGradient = getSvgGradientNodeStroke();
            const SvgPatternNode* pStrokePattern = getSvgPatternNodeStroke();
            basegfx::B2DPolyPolygon aMergedArea;

            if(pFillGradient || pFillPattern || pStroke || pStrokeGradient || pStrokePattern)
            {
                // text geometry is needed, create
                // use neutral ViewInformation and create LineGeometryExtractor2D
                const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                drawinglayer::processor2d::TextAsPolygonExtractor2D aExtractor(aViewInformation2D);

                // process
                aExtractor.process(rSource);

                // get results
                const drawinglayer::processor2d::TextAsPolygonDataNodeVector& rResult = aExtractor.getTarget();
                const sal_uInt32 nResultCount(rResult.size());
                basegfx::B2DPolyPolygonVector aTextFillVector;
                aTextFillVector.reserve(nResultCount);

                for(sal_uInt32 a(0); a < nResultCount; a++)
                {
                    const drawinglayer::processor2d::TextAsPolygonDataNode& rCandidate = rResult[a];

                    if(rCandidate.getIsFilled())
                    {
                        aTextFillVector.push_back(rCandidate.getB2DPolyPolygon());
                    }
                }

                if(!aTextFillVector.empty())
                {
                    aMergedArea = basegfx::utils::mergeToSinglePolyPolygon(aTextFillVector);
                }
            }

            const bool bStrokeUsed(pStroke || pStrokeGradient || pStrokePattern);

            // add fill. Use geometry even for simple color fill when stroke
            // is used, else text rendering and the geometry-based stroke will
            // normally not really match optically due to diverse system text
            // renderers
            if(aMergedArea.count() && (pFillGradient || pFillPattern || bStrokeUsed))
            {
                // create text fill content based on geometry
                add_fill(aMergedArea, rTarget, aMergedArea.getB2DRange());
            }
            else if(pFill)
            {
                // add the already prepared primitives for single color fill
                rTarget.append(std::move(rSource));
            }

            // add stroke
            if(aMergedArea.count() && bStrokeUsed)
            {
                // create text stroke content
                add_stroke(aMergedArea, rTarget, aMergedArea.getB2DRange());
            }
        }

        void SvgStyleAttributes::add_fillGradient(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const SvgGradientNode& rFillGradient,
            const basegfx::B2DRange& rGeoRange) const
        {
            // create fill content
            drawinglayer::primitive2d::SvgGradientEntryVector aSvgGradientEntryVector;

            // get the color stops
            rFillGradient.collectGradientEntries(aSvgGradientEntryVector);

            if(aSvgGradientEntryVector.empty())
                return;

            basegfx::B2DHomMatrix aGeoToUnit;
            basegfx::B2DHomMatrix aGradientTransform;

            if(rFillGradient.getGradientTransform())
            {
                aGradientTransform = *rFillGradient.getGradientTransform();
            }

            if (SvgUnits::userSpaceOnUse == rFillGradient.getGradientUnits())
            {
                aGeoToUnit.translate(-rGeoRange.getMinX(), -rGeoRange.getMinY());
                aGeoToUnit.scale(1.0 / rGeoRange.getWidth(), 1.0 / rGeoRange.getHeight());
            }

            if(SVGToken::LinearGradient == rFillGradient.getType())
            {
                basegfx::B2DPoint aStart(0.0, 0.0);
                basegfx::B2DPoint aEnd(1.0, 0.0);

                if (SvgUnits::userSpaceOnUse == rFillGradient.getGradientUnits())
                {
                    // all possible units
                    aStart.setX(rFillGradient.getX1().solve(mrOwner, NumberType::xcoordinate));
                    aStart.setY(rFillGradient.getY1().solve(mrOwner, NumberType::ycoordinate));
                    aEnd.setX(rFillGradient.getX2().solve(mrOwner, NumberType::xcoordinate));
                    aEnd.setY(rFillGradient.getY2().solve(mrOwner, NumberType::ycoordinate));
                }
                else
                {
                    // fractions or percent relative to object bounds
                    const SvgNumber X1(rFillGradient.getX1());
                    const SvgNumber Y1(rFillGradient.getY1());
                    const SvgNumber X2(rFillGradient.getX2());
                    const SvgNumber Y2(rFillGradient.getY2());

                    aStart.setX(SvgUnit::percent == X1.getUnit() ? X1.getNumber() * 0.01 : X1.getNumber());
                    aStart.setY(SvgUnit::percent == Y1.getUnit() ? Y1.getNumber() * 0.01 : Y1.getNumber());
                    aEnd.setX(SvgUnit::percent == X2.getUnit() ? X2.getNumber() * 0.01 : X2.getNumber());
                    aEnd.setY(SvgUnit::percent == Y2.getUnit() ? Y2.getNumber() * 0.01 : Y2.getNumber());
                }

                if(!aGeoToUnit.isIdentity())
                {
                    aStart *= aGeoToUnit;
                    aEnd *= aGeoToUnit;
                }

                rTarget.push_back(
                    new drawinglayer::primitive2d::SvgLinearGradientPrimitive2D(
                        aGradientTransform,
                        rPath,
                        std::move(aSvgGradientEntryVector),
                        aStart,
                        aEnd,
                        SvgUnits::userSpaceOnUse != rFillGradient.getGradientUnits(),
                        rFillGradient.getSpreadMethod()));
            }
            else
            {
                basegfx::B2DPoint aStart(0.5, 0.5);
                basegfx::B2DPoint aFocal;
                double fRadius(0.5);
                const SvgNumber* pFx = rFillGradient.getFx();
                const SvgNumber* pFy = rFillGradient.getFy();
                const bool bFocal(pFx || pFy);

                if (SvgUnits::userSpaceOnUse == rFillGradient.getGradientUnits())
                {
                    // all possible units
                    aStart.setX(rFillGradient.getCx().solve(mrOwner, NumberType::xcoordinate));
                    aStart.setY(rFillGradient.getCy().solve(mrOwner, NumberType::ycoordinate));
                    fRadius = rFillGradient.getR().solve(mrOwner);

                    if(bFocal)
                    {
                        aFocal.setX(pFx ? pFx->solve(mrOwner, NumberType::xcoordinate) : aStart.getX());
                        aFocal.setY(pFy ? pFy->solve(mrOwner, NumberType::ycoordinate) : aStart.getY());
                    }
                }
                else
                {
                    // fractions or percent relative to object bounds
                    const SvgNumber Cx(rFillGradient.getCx());
                    const SvgNumber Cy(rFillGradient.getCy());
                    const SvgNumber R(rFillGradient.getR());

                    aStart.setX(SvgUnit::percent == Cx.getUnit() ? Cx.getNumber() * 0.01 : Cx.getNumber());
                    aStart.setY(SvgUnit::percent == Cy.getUnit() ? Cy.getNumber() * 0.01 : Cy.getNumber());
                    fRadius = (SvgUnit::percent == R.getUnit()) ? R.getNumber() * 0.01 : R.getNumber();

                    if(bFocal)
                    {
                        aFocal.setX(pFx ? (SvgUnit::percent == pFx->getUnit() ? pFx->getNumber() * 0.01 : pFx->getNumber()) : aStart.getX());
                        aFocal.setY(pFy ? (SvgUnit::percent == pFy->getUnit() ? pFy->getNumber() * 0.01 : pFy->getNumber()) : aStart.getY());
                    }
                }

                if(!aGeoToUnit.isIdentity())
                {
                    aStart *= aGeoToUnit;
                    fRadius = (aGeoToUnit * basegfx::B2DVector(fRadius, 0.0)).getLength();

                    if(bFocal)
                    {
                        aFocal *= aGeoToUnit;
                    }
                }

                rTarget.push_back(
                    new drawinglayer::primitive2d::SvgRadialGradientPrimitive2D(
                        aGradientTransform,
                        rPath,
                        std::move(aSvgGradientEntryVector),
                        aStart,
                        fRadius,
                        SvgUnits::userSpaceOnUse != rFillGradient.getGradientUnits(),
                        rFillGradient.getSpreadMethod(),
                        bFocal ? &aFocal : nullptr));
            }
        }

        void SvgStyleAttributes::add_fillPatternTransform(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const SvgPatternNode& rFillPattern,
            const basegfx::B2DRange& rGeoRange) const
        {
            // prepare fill polyPolygon with given pattern, check for patternTransform
            if(rFillPattern.getPatternTransform() && !rFillPattern.getPatternTransform()->isIdentity())
            {
                // PatternTransform is active; Handle by filling the inverse transformed
                // path and back-transforming the result
                basegfx::B2DPolyPolygon aPath(rPath);
                basegfx::B2DHomMatrix aInv(*rFillPattern.getPatternTransform());
                drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                aInv.invert();
                aPath.transform(aInv);
                add_fillPattern(aPath, aNewTarget, rFillPattern, aPath.getB2DRange());

                if(!aNewTarget.empty())
                {
                    rTarget.push_back(
                        new drawinglayer::primitive2d::TransformPrimitive2D(
                            *rFillPattern.getPatternTransform(),
                            std::move(aNewTarget)));
                }
            }
            else
            {
                // no patternTransform, create fillPattern directly
                add_fillPattern(rPath, rTarget, rFillPattern, rGeoRange);
            }
        }

        void SvgStyleAttributes::add_fillPattern(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const SvgPatternNode& rFillPattern,
            const basegfx::B2DRange& rGeoRange) const
        {
            // fill polyPolygon with given pattern
            const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives = rFillPattern.getPatternPrimitives();

            if(rPrimitives.empty())
                return;

            double fTargetWidth(rGeoRange.getWidth());
            double fTargetHeight(rGeoRange.getHeight());

            if(fTargetWidth <= 0.0 || fTargetHeight <= 0.0)
                return;

            // get relative values from pattern
            double fX(0.0);
            double fY(0.0);
            double fW(0.0);
            double fH(0.0);

            rFillPattern.getValuesRelative(fX, fY, fW, fH, rGeoRange, mrOwner);

            if(fW <= 0.0 || fH <= 0.0)
                return;

            // build the reference range relative to the rGeoRange
            const basegfx::B2DRange aReferenceRange(fX, fY, fX + fW, fY + fH);

            // find out how the content is mapped to the reference range
            basegfx::B2DHomMatrix aMapPrimitivesToUnitRange;
            const basegfx::B2DRange* pViewBox = rFillPattern.getViewBox();

            if(pViewBox)
            {
                // use viewBox/preserveAspectRatio
                const SvgAspectRatio& rRatio = rFillPattern.getSvgAspectRatio();
                const basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

                if(rRatio.isSet())
                {
                    // let mapping be created from SvgAspectRatio
                    aMapPrimitivesToUnitRange = rRatio.createMapping(aUnitRange, *pViewBox);
                }
                else
                {
                    // choose default mapping
                    aMapPrimitivesToUnitRange = SvgAspectRatio::createLinearMapping(aUnitRange, *pViewBox);
                }
            }
            else
            {
                // use patternContentUnits
                const SvgUnits aPatternContentUnits(rFillPattern.getPatternContentUnits() ? *rFillPattern.getPatternContentUnits() : SvgUnits::userSpaceOnUse);

                if (SvgUnits::userSpaceOnUse == aPatternContentUnits)
                {
                    // create relative mapping to unit coordinates
                    aMapPrimitivesToUnitRange.scale(1.0 / (fW * fTargetWidth), 1.0 / (fH * fTargetHeight));
                }
                else
                {
                    aMapPrimitivesToUnitRange.scale(1.0 / fW, 1.0 / fH);
                }
            }

            // apply aMapPrimitivesToUnitRange to content when used
            drawinglayer::primitive2d::Primitive2DContainer aPrimitives(rPrimitives);

            if(!aMapPrimitivesToUnitRange.isIdentity())
            {
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aMapPrimitivesToUnitRange,
                        std::move(aPrimitives)));

                aPrimitives = drawinglayer::primitive2d::Primitive2DContainer { xRef };
            }

            // embed in PatternFillPrimitive2D
            rTarget.push_back(
                new drawinglayer::primitive2d::PatternFillPrimitive2D(
                    rPath,
                    std::move(aPrimitives),
                    aReferenceRange));
        }

        void SvgStyleAttributes::add_fill(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::B2DRange& rGeoRange) const
        {
            const basegfx::BColor* pFill = getFill();
            const SvgGradientNode* pFillGradient = getSvgGradientNodeFill();
            const SvgPatternNode* pFillPattern = getSvgPatternNodeFill();

            if(!(pFill || pFillGradient || pFillPattern))
                return;

            const double fFillOpacity(getFillOpacity().solve(mrOwner));

            if(!basegfx::fTools::more(fFillOpacity, 0.0))
                return;

            drawinglayer::primitive2d::Primitive2DContainer aNewFill;

            if(pFillGradient)
            {
                // create fill content with SVG gradient primitive
                add_fillGradient(rPath, aNewFill, *pFillGradient, rGeoRange);
            }
            else if(pFillPattern)
            {
                // create fill content with SVG pattern primitive
                add_fillPatternTransform(rPath, aNewFill, *pFillPattern, rGeoRange);
            }
            else // if(pFill)
            {
                // create fill content
                aNewFill.resize(1);
                aNewFill[0] = new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    rPath,
                    *pFill);
            }

            if(aNewFill.empty())
                return;

            if(basegfx::fTools::less(fFillOpacity, 1.0))
            {
                // embed in UnifiedTransparencePrimitive2D
                rTarget.push_back(
                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                        std::move(aNewFill),
                        1.0 - fFillOpacity));
            }
            else
            {
                // append
                rTarget.append(aNewFill);
            }
        }

        void SvgStyleAttributes::add_stroke(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::B2DRange& rGeoRange) const
        {
            const basegfx::BColor* pStroke = getStroke();
            const SvgGradientNode* pStrokeGradient = getSvgGradientNodeStroke();
            const SvgPatternNode* pStrokePattern = getSvgPatternNodeStroke();

            if(!(pStroke || pStrokeGradient || pStrokePattern))
                return;

            drawinglayer::primitive2d::Primitive2DContainer aNewStroke;
            const double fStrokeOpacity(getStrokeOpacity().solve(mrOwner));

            if(!basegfx::fTools::more(fStrokeOpacity, 0.0))
                return;

            // get stroke width; SVG does not use 0.0 == hairline, so 0.0 is no line at all
            const double fStrokeWidth(getStrokeWidth().isSet() ? getStrokeWidth().solve(mrOwner) : 1.0);

            if(!basegfx::fTools::more(fStrokeWidth, 0.0))
                return;

            drawinglayer::primitive2d::Primitive2DReference aNewLinePrimitive;

            // if we have a line with two identical points it is not really a line,
            // but used by SVG sometimes to paint a single dot.In that case, create
            // the geometry for a single dot
            if(1 == rPath.count())
            {
                const basegfx::B2DPolygon& aSingle(rPath.getB2DPolygon(0));

                if(2 == aSingle.count() && aSingle.getB2DPoint(0).equal(aSingle.getB2DPoint(1)))
                {
                    aNewLinePrimitive = new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(
                            basegfx::utils::createPolygonFromCircle(
                                aSingle.getB2DPoint(0),
                                fStrokeWidth * (1.44 * 0.5))),
                        pStroke ? *pStroke : basegfx::BColor(0.0, 0.0, 0.0));
                }
            }

            if(!aNewLinePrimitive.is())
            {
                // get LineJoin, LineCap and stroke array
                const basegfx::B2DLineJoin aB2DLineJoin(StrokeLinejoinToB2DLineJoin(getStrokeLinejoin()));
                const css::drawing::LineCap aLineCap(StrokeLinecapToDrawingLineCap(getStrokeLinecap()));
                ::std::vector< double > aDashArray;

                if(!getStrokeDasharray().empty())
                {
                    aDashArray = solveSvgNumberVector(getStrokeDasharray(), mrOwner);
                }

                // convert svg:stroke-miterlimit to LineAttrute:mfMiterMinimumAngle
                // The default needs to be set explicitly, because svg default <> Draw default
                double fMiterMinimumAngle;
                if (getStrokeMiterLimit().isSet())
                {
                    fMiterMinimumAngle = 2.0 * asin(1.0/getStrokeMiterLimit().getNumber());
                }
                else
                {
                    fMiterMinimumAngle = 2.0 * asin(0.25); // 1.0/default 4.0
                }

                // todo: Handle getStrokeDashOffset()

                // prepare line attribute
                const drawinglayer::attribute::LineAttribute aLineAttribute(
                    pStroke ? *pStroke : basegfx::BColor(0.0, 0.0, 0.0),
                    fStrokeWidth,
                    aB2DLineJoin,
                    aLineCap,
                    fMiterMinimumAngle);

                if(aDashArray.empty())
                {
                    aNewLinePrimitive = new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
                        rPath,
                        aLineAttribute);
                }
                else
                {
                    const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(std::move(aDashArray));

                    aNewLinePrimitive = new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(
                        rPath,
                        aLineAttribute,
                        aStrokeAttribute);
                }
            }

            if(pStrokeGradient || pStrokePattern)
            {
                // put primitive into Primitive2DReference and Primitive2DSequence
                const drawinglayer::primitive2d::Primitive2DContainer aSeq { aNewLinePrimitive };

                // use neutral ViewInformation and create LineGeometryExtractor2D
                const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                drawinglayer::processor2d::LineGeometryExtractor2D aExtractor(aViewInformation2D);

                // process
                aExtractor.process(aSeq);

                // check for fill rsults
                const basegfx::B2DPolyPolygonVector& rLineFillVector(aExtractor.getExtractedLineFills());

                if(!rLineFillVector.empty())
                {
                    const basegfx::B2DPolyPolygon aMergedArea(
                        basegfx::utils::mergeToSinglePolyPolygon(
                            rLineFillVector));

                    if(aMergedArea.count())
                    {
                        if(pStrokeGradient)
                        {
                            // create fill content with SVG gradient primitive. Use original GeoRange,
                            // e.g. from circle without LineWidth
                            add_fillGradient(aMergedArea, aNewStroke, *pStrokeGradient, rGeoRange);
                        }
                        else // if(pStrokePattern)
                        {
                            // create fill content with SVG pattern primitive. Use GeoRange
                            // from the expanded data, e.g. circle with extended geo by half linewidth
                            add_fillPatternTransform(aMergedArea, aNewStroke, *pStrokePattern, aMergedArea.getB2DRange());
                        }
                    }
                }
            }
            else // if(pStroke)
            {
                aNewStroke.push_back(aNewLinePrimitive);
            }

            if(aNewStroke.empty())
                return;

            if(basegfx::fTools::less(fStrokeOpacity, 1.0))
            {
                // embed in UnifiedTransparencePrimitive2D
                rTarget.push_back(
                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                        std::move(aNewStroke),
                        1.0 - fStrokeOpacity));
            }
            else
            {
                // append
                rTarget.append(aNewStroke);
            }
        }

        bool SvgStyleAttributes::prepare_singleMarker(
            drawinglayer::primitive2d::Primitive2DContainer& rMarkerPrimitives,
            basegfx::B2DHomMatrix& rMarkerTransform,
            basegfx::B2DRange& rClipRange,
            const SvgMarkerNode& rMarker) const
        {
            // reset return values
            rMarkerTransform.identity();
            rClipRange.reset();

            // get marker primitive representation
            rMarkerPrimitives = rMarker.getMarkerPrimitives();

            if(!rMarkerPrimitives.empty())
            {
                basegfx::B2DRange aPrimitiveRange(0.0, 0.0, 1.0, 1.0);
                const basegfx::B2DRange* pViewBox = rMarker.getViewBox();

                if(pViewBox)
                {
                    aPrimitiveRange = *pViewBox;
                }

                if(aPrimitiveRange.getWidth() > 0.0 && aPrimitiveRange.getHeight() > 0.0)
                {
                    double fTargetWidth(rMarker.getMarkerWidth().isSet() ? rMarker.getMarkerWidth().solve(mrOwner, NumberType::xcoordinate) : 3.0);
                    double fTargetHeight(rMarker.getMarkerHeight().isSet() ? rMarker.getMarkerHeight().solve(mrOwner, NumberType::xcoordinate) : 3.0);
                    const bool bStrokeWidth(SvgMarkerNode::MarkerUnits::strokeWidth == rMarker.getMarkerUnits());
                    const double fStrokeWidth(getStrokeWidth().isSet() ? getStrokeWidth().solve(mrOwner) : 1.0);

                    if(bStrokeWidth)
                    {
                        // relative to strokeWidth
                        fTargetWidth *= fStrokeWidth;
                        fTargetHeight *= fStrokeWidth;
                    }

                    if(fTargetWidth > 0.0 && fTargetHeight > 0.0)
                    {
                        // create mapping
                        const basegfx::B2DRange aTargetRange(0.0, 0.0, fTargetWidth, fTargetHeight);
                        const SvgAspectRatio& rRatio = rMarker.getSvgAspectRatio();

                        if(rRatio.isSet())
                        {
                            // let mapping be created from SvgAspectRatio
                            rMarkerTransform = rRatio.createMapping(aTargetRange, aPrimitiveRange);

                            if(rRatio.isMeetOrSlice())
                            {
                                // need to clip
                                rClipRange = aPrimitiveRange;
                            }
                        }
                        else
                        {
                            if(!pViewBox)
                            {
                                if(bStrokeWidth)
                                {
                                    // adapt to strokewidth if needed
                                    rMarkerTransform.scale(fStrokeWidth, fStrokeWidth);
                                }
                            }
                            else
                            {
                                // choose default mapping
                                rMarkerTransform = SvgAspectRatio::createLinearMapping(aTargetRange, aPrimitiveRange);
                            }
                        }

                        // get and apply reference point. Initially it's in marker local coordinate system
                        basegfx::B2DPoint aRefPoint(
                            rMarker.getRefX().isSet() ? rMarker.getRefX().solve(mrOwner, NumberType::xcoordinate) : 0.0,
                            rMarker.getRefY().isSet() ? rMarker.getRefY().solve(mrOwner, NumberType::ycoordinate) : 0.0);

                        // apply MarkerTransform to have it in mapped coordinates
                        aRefPoint *= rMarkerTransform;

                        // apply by moving RepPoint to (0.0)
                        rMarkerTransform.translate(-aRefPoint.getX(), -aRefPoint.getY());

                        return true;
                    }
                }
            }

            return false;
        }

        void SvgStyleAttributes::add_markers(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::utils::PointIndexSet* pHelpPointIndices) const
        {
            // try to access linked markers
            const SvgMarkerNode* pStart = accessMarkerStartXLink();
            const SvgMarkerNode* pMid = accessMarkerMidXLink();
            const SvgMarkerNode* pEnd = accessMarkerEndXLink();

            if(!(pStart || pMid || pEnd))
                return;

            const sal_uInt32 nSubPathCount(rPath.count());

            if(!nSubPathCount)
                return;

            // remember prepared marker; pStart, pMid and pEnd may all be equal when
            // only 'marker' was used instead of 'marker-start', 'marker-mid' or 'marker-end',
            // see 'case SVGToken::Marker' in this file; thus in this case only one common
            // marker in primitive form will be prepared
            const SvgMarkerNode* pPrepared = nullptr;

            // values for the prepared marker, results of prepare_singleMarker
            drawinglayer::primitive2d::Primitive2DContainer aPreparedMarkerPrimitives;
            basegfx::B2DHomMatrix aPreparedMarkerTransform;
            basegfx::B2DRange aPreparedMarkerClipRange;

            for (sal_uInt32 a(0); a < nSubPathCount; a++)
            {
                // iterate over sub-paths
                const basegfx::B2DPolygon& aSubPolygonPath(rPath.getB2DPolygon(a));
                const sal_uInt32 nSubPolygonPointCount(aSubPolygonPath.count());
                const bool bSubPolygonPathIsClosed(aSubPolygonPath.isClosed());

                if(nSubPolygonPointCount)
                {
                    // for each sub-path, create one marker per point (when closed, two markers
                    // need to pe created for the 1st point)
                    const sal_uInt32 nTargetMarkerCount(bSubPolygonPathIsClosed ? nSubPolygonPointCount + 1 : nSubPolygonPointCount);

                    for (sal_uInt32 b(0); b < nTargetMarkerCount; b++)
                    {
                        const bool bIsFirstMarker(!a && !b);
                        const bool bIsLastMarker(nSubPathCount - 1 == a && nTargetMarkerCount - 1 == b);
                        const SvgMarkerNode* pNeeded = nullptr;

                        if(bIsFirstMarker)
                        {
                            // 1st point in 1st sub-polygon, use pStart
                            pNeeded = pStart;
                        }
                        else if(bIsLastMarker)
                        {
                            // last point in last sub-polygon, use pEnd
                            pNeeded = pEnd;
                        }
                        else
                        {
                            // anything in-between, use pMid
                            pNeeded = pMid;
                        }

                        if(pHelpPointIndices && !pHelpPointIndices->empty())
                        {
                            const basegfx::utils::PointIndexSet::const_iterator aFound(
                                pHelpPointIndices->find(basegfx::utils::PointIndex(a, b)));

                            if(aFound != pHelpPointIndices->end())
                            {
                                // this point is a pure helper point; do not create a marker for it
                                continue;
                            }
                        }

                        if(!pNeeded)
                        {
                            // no marker needs to be created for this point
                            continue;
                        }

                        if(pPrepared != pNeeded)
                        {
                            // if needed marker is not yet prepared, do it now
                            if(prepare_singleMarker(aPreparedMarkerPrimitives, aPreparedMarkerTransform, aPreparedMarkerClipRange, *pNeeded))
                            {
                                pPrepared = pNeeded;
                            }
                            else
                            {
                                // error: could not prepare given marker
                                OSL_ENSURE(false, "OOps, could not prepare given marker as primitives (!)");
                                pPrepared = nullptr;
                                continue;
                            }
                        }

                        // prepare complete transform
                        basegfx::B2DHomMatrix aCombinedTransform(aPreparedMarkerTransform);

                        // get rotation
                        if(pPrepared->getOrientAuto())
                        {
                            const sal_uInt32 nPointIndex(b % nSubPolygonPointCount);

                            // get entering and leaving tangents; this will search backward/forward
                            // in the polygon to find tangents unequal to zero, skipping empty edges
                            // see basegfx descriptions)
                            // Hint: Mozilla, Inkscape and others use only leaving tangent for start marker
                            // and entering tangent for end marker. To achieve this (if wanted) it is possible
                            // to make the fetch of aEntering/aLeaving dependent on bIsFirstMarker/bIsLastMarker.
                            // This is not done here, see comment 14 in task #1232379#
                            // or http://www.w3.org/TR/SVG/painting.html#OrientAttribute
                            basegfx::B2DVector aEntering(
                                basegfx::utils::getTangentEnteringPoint(
                                    aSubPolygonPath,
                                    nPointIndex));
                            basegfx::B2DVector aLeaving(
                                basegfx::utils::getTangentLeavingPoint(
                                    aSubPolygonPath,
                                    nPointIndex));
                            const bool bEntering(!aEntering.equalZero());
                            const bool bLeaving(!aLeaving.equalZero());

                            if(bEntering || bLeaving)
                            {
                                basegfx::B2DVector aSum(0.0, 0.0);

                                if(bEntering)
                                {
                                    aSum += aEntering.normalize();
                                }

                                if(bLeaving)
                                {
                                    aSum += aLeaving.normalize();
                                }

                                if(!aSum.equalZero())
                                {
                                    const double fAngle(atan2(aSum.getY(), aSum.getX()));

                                    // apply rotation
                                    aCombinedTransform.rotate(fAngle);
                                }
                            }
                        }
                        else
                        {
                            // apply rotation
                            aCombinedTransform.rotate(pPrepared->getAngle());
                        }

                        // get and apply target position
                        const basegfx::B2DPoint aPoint(aSubPolygonPath.getB2DPoint(b % nSubPolygonPointCount));

                        aCombinedTransform.translate(aPoint.getX(), aPoint.getY());

                        // prepare marker
                        drawinglayer::primitive2d::Primitive2DReference xMarker(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                aCombinedTransform,
                                drawinglayer::primitive2d::Primitive2DContainer(aPreparedMarkerPrimitives)));

                        if(!aPreparedMarkerClipRange.isEmpty())
                        {
                            // marker needs to be clipped, it's bigger as the mapping
                            basegfx::B2DPolyPolygon aClipPolygon(basegfx::utils::createPolygonFromRect(aPreparedMarkerClipRange));

                            aClipPolygon.transform(aCombinedTransform);
                            xMarker = new drawinglayer::primitive2d::MaskPrimitive2D(
                                aClipPolygon,
                                drawinglayer::primitive2d::Primitive2DContainer { xMarker });
                        }

                        // add marker
                        rTarget.push_back(xMarker);
                    }
                }
            }
        }

        void SvgStyleAttributes::add_path(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::utils::PointIndexSet* pHelpPointIndices) const
        {
            if(!rPath.count())
            {
                // no geometry at all
                return;
            }

            const basegfx::B2DRange aGeoRange(rPath.getB2DRange());

            if(aGeoRange.isEmpty())
            {
                // no geometry range
                return;
            }

            const double fOpacity(getOpacity().solve(mrOwner));

            if(basegfx::fTools::equalZero(fOpacity))
            {
                // not visible
                return;
            }

            // check if it's a line
            const bool bNoWidth(basegfx::fTools::equalZero(aGeoRange.getWidth()));
            const bool bNoHeight(basegfx::fTools::equalZero(aGeoRange.getHeight()));
            const bool bIsTwoPointLine(1 == rPath.count()
                && !rPath.areControlPointsUsed()
                && 2 == rPath.getB2DPolygon(0).count());
            const bool bIsLine(bIsTwoPointLine || bNoWidth || bNoHeight);

            if(!bIsLine)
            {
                // create fill
                basegfx::B2DPolyPolygon aPath(rPath);
                const bool bNeedToCheckClipRule(SVGToken::Path == mrOwner.getType() || SVGToken::Polygon == mrOwner.getType());
                const bool bClipPathIsNonzero(bNeedToCheckClipRule && mbIsClipPathContent && FillRule::nonzero == maClipRule);
                const bool bFillRuleIsNonzero(bNeedToCheckClipRule && !mbIsClipPathContent && FillRule::nonzero == getFillRule());

                if(bClipPathIsNonzero || bFillRuleIsNonzero)
                {
                    if(getFill() || getSvgGradientNodeFill() || getSvgPatternNodeFill()) {

                        // FIXME: By introducing appropriate fill rule, this should be changed
                        //       createNonzeroConform() is no longer needed, this should be
                        //       done by setting the appropriate FillRule

                        // nonzero is wanted, solve geometrically (see description on basegfx)
                        // basegfx::utils::createNonzeroConform() is expensive for huge paths
                        // and is only needed if path will be filled later on
                        aPath = basegfx::utils::createNonzeroConform(aPath);
                    }
                }

                add_fill(aPath, rTarget, aGeoRange);
            }

            // create stroke
            add_stroke(rPath, rTarget, aGeoRange);

            // Svg supports markers for path, polygon, polyline and line
            if(SVGToken::Path == mrOwner.getType() ||         // path
                SVGToken::Polygon == mrOwner.getType() ||     // polygon, polyline
                SVGToken::Line == mrOwner.getType())          // line
            {
                // try to add markers
                add_markers(rPath, rTarget, pHelpPointIndices);
            }
        }

        void SvgStyleAttributes::add_postProcess(
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            drawinglayer::primitive2d::Primitive2DContainer&& rSource,
            const std::optional<basegfx::B2DHomMatrix>& pTransform) const
        {
            if(rSource.empty())
                return;

            const double fOpacity(getOpacity().solve(mrOwner));

            if(basegfx::fTools::equalZero(fOpacity))
            {
                return;
            }

            drawinglayer::primitive2d::Primitive2DContainer aSource(std::move(rSource));

            if(basegfx::fTools::less(fOpacity, 1.0))
            {
                // embed in UnifiedTransparencePrimitive2D
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                        std::move(aSource),
                        1.0 - fOpacity));

                aSource = drawinglayer::primitive2d::Primitive2DContainer { xRef };
            }

            if(pTransform)
            {
                // create embedding group element with transformation. This applies the given
                // transformation to the graphical content, but *not* to mask and/or clip (as needed)
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        *pTransform,
                        std::move(aSource)));

                aSource = drawinglayer::primitive2d::Primitive2DContainer { xRef };
            }

            const SvgClipPathNode* pClip = accessClipPathXLink();
            while(pClip)
            {
                // #i124852# transform may be needed when SvgUnits::userSpaceOnUse
                pClip->apply(aSource, pTransform);
                pClip = pClip->getSvgStyleAttributes()->accessClipPathXLink();
            }

            if(!aSource.empty()) // test again, applied clipPath may have lead to empty geometry
            {
                const SvgMaskNode* pMask = accessMaskXLink();
                if(pMask)
                {
                    // #i124852# transform may be needed when SvgUnits::userSpaceOnUse
                    pMask->apply(aSource, pTransform);
                }
            }

            // This is part of the SVG import of self-written SVGs from
            // Draw/Impress containing multiple Slides/Pages. To be able
            // to later 'break' these to multiple Pages if wanted, embed
            // each Page-Content in an identifiable Primitive Grouping
            // Object.
            // This is the case when the current Node is a GroupNode, has
            // class="Page" set, has a parent that also is a GroupNode
            // at which class="Slide" is set.
            // Multiple Slides/Pages are possible for Draw and Impress.
            if(SVGToken::G == mrOwner.getType() && mrOwner.getClass())
            {
                const OUString aOwnerClass(*mrOwner.getClass());

                if("Page" == aOwnerClass)
                {
                    const SvgNode* pParent(mrOwner.getParent());

                    if(nullptr != pParent && SVGToken::G == pParent->getType() && pParent->getClass())
                    {
                        const OUString aParentClass(*pParent->getClass());

                        if("Slide" == aParentClass)
                        {
                            // embed to grouping primitive to identify the
                            // Slide/Page information
                            const drawinglayer::primitive2d::Primitive2DReference xRef(
                                new drawinglayer::primitive2d::PageHierarchyPrimitive2D(
                                    std::move(aSource)));

                            aSource = drawinglayer::primitive2d::Primitive2DContainer { xRef };
                        }
                    }
                }
            }

            if(!aSource.empty()) // test again, applied mask may have lead to empty geometry
            {
                // append to current target
                rTarget.append(aSource);
            }
        }

        SvgStyleAttributes::SvgStyleAttributes(SvgNode& rOwner)
        :   mrOwner(rOwner),
            mpCssStyleParent(nullptr),
            maStopColor(basegfx::BColor(0.0, 0.0, 0.0), true),
            maStrokeLinecap(StrokeLinecap::notset),
            maStrokeLinejoin(StrokeLinejoin::notset),
            maFontSize(),
            maFontStretch(FontStretch::notset),
            maFontStyle(FontStyle::notset),
            maFontWeight(FontWeight::notset),
            maTextAlign(TextAlign::notset),
            maTextDecoration(TextDecoration::notset),
            maTextAnchor(TextAnchor::notset),
            maVisibility(Visibility::notset),
            mpClipPathXLink(nullptr),
            mpMaskXLink(nullptr),
            mpMarkerStartXLink(nullptr),
            mpMarkerMidXLink(nullptr),
            mpMarkerEndXLink(nullptr),
            maFillRule(FillRule::notset),
            maClipRule(FillRule::nonzero),
            maBaselineShift(BaselineShift::Baseline),
            maBaselineShiftNumber(0),
            maResolvingParent(30, 0),
            mbIsClipPathContent(SVGToken::ClipPathNode == mrOwner.getType()),
            mbStrokeDasharraySet(false)
        {
            const SvgStyleAttributes* pParentStyle = getParentStyle();
            if(!mbIsClipPathContent)
            {
                if(pParentStyle)
                {
                    mbIsClipPathContent = pParentStyle->mbIsClipPathContent;
                }
            }
        }

        SvgStyleAttributes::~SvgStyleAttributes()
        {
        }

        void SvgStyleAttributes::parseStyleAttribute(
            SVGToken aSVGToken,
            const OUString& aContent,
            bool bCaseIndependent)
        {
            switch(aSVGToken)
            {
                case SVGToken::Fill:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        setFill(aSvgPaint);
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(std::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    else if(!aURL.isEmpty())
                    {
                       maNodeFillURL = aURL;
                    }
                    break;
                }
                case SVGToken::FillOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maFillOpacity = SvgNumber(std::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet());
                    }
                    break;
                }
                case SVGToken::FillRule:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrNonzero))
                        {
                            maFillRule = FillRule::nonzero;
                        }
                        else if(aContent.match(commonStrings::aStrEvenOdd))
                        {
                            maFillRule = FillRule::evenodd;
                        }
                    }
                    break;
                }
                case SVGToken::Stroke:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        maStroke = aSvgPaint;
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(std::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    else if(!aURL.isEmpty())
                    {
                        maNodeStrokeURL = aURL;
                    }
                    break;
                }
                case SVGToken::StrokeDasharray:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumberVector aVector;

                        if(aContent.startsWith("none"))
                        {
                            // #121221# The special value 'none' needs to be handled
                            // in the sense that *when* it is set, the parent shall not
                            // be used. Before this was only dependent on the array being
                            // empty
                            mbStrokeDasharraySet = true;
                        }
                        else if(readSvgNumberVector(aContent, aVector))
                        {
                            maStrokeDasharray = aVector;
                        }
                    }
                    break;
                }
                case SVGToken::StrokeDashoffset:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maStrokeDashOffset = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::StrokeLinecap:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("butt"))
                        {
                            setStrokeLinecap(StrokeLinecap::butt);
                        }
                        else if(aContent.startsWith("round"))
                        {
                            setStrokeLinecap(StrokeLinecap::round);
                        }
                        else if(aContent.startsWith("square"))
                        {
                            setStrokeLinecap(StrokeLinecap::square);
                        }
                    }
                    break;
                }
                case SVGToken::StrokeLinejoin:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("miter"))
                        {
                            setStrokeLinejoin(StrokeLinejoin::miter);
                        }
                        else if(aContent.startsWith("round"))
                        {
                            setStrokeLinejoin(StrokeLinejoin::round);
                        }
                        else if(aContent.startsWith("bevel"))
                        {
                            setStrokeLinejoin(StrokeLinejoin::bevel);
                        }
                    }
                    break;
                }
                case SVGToken::StrokeMiterlimit:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(basegfx::fTools::moreOrEqual(aNum.getNumber(), 1.0))
                        { //readSingleNumber sets SvgUnit::px as default, if unit is missing. Correct it here.
                            maStrokeMiterLimit = SvgNumber(aNum.getNumber(), SvgUnit::none);
                        }
                    }
                    break;
                }
                case SVGToken::StrokeOpacity:
                {

                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maStrokeOpacity = SvgNumber(std::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet());
                    }
                    break;
                }
                case SVGToken::StrokeWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maStrokeWidth = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::StopColor:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        maStopColor = aSvgPaint;
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(std::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    break;
                }
                case SVGToken::StopOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maStopOpacity = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::Font:
                {
                    break;
                }
                case SVGToken::FontFamily:
                {
                    SvgStringVector aSvgStringVector;

                    if(readSvgStringVector(aContent, aSvgStringVector))
                    {
                        maFontFamily = aSvgStringVector;
                    }
                    break;
                }
                case SVGToken::FontSize:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("xx-small"))
                        {
                            setFontSize(FontSize::xx_small);
                        }
                        else if(aContent.startsWith("x-small"))
                        {
                            setFontSize(FontSize::x_small);
                        }
                        else if(aContent.startsWith("small"))
                        {
                            setFontSize(FontSize::small);
                        }
                        else if(aContent.startsWith("smaller"))
                        {
                            setFontSize(FontSize::smaller);
                        }
                        else if(aContent.startsWith("medium"))
                        {
                            setFontSize(FontSize::medium);
                        }
                        else if(aContent.startsWith("larger"))
                        {
                            setFontSize(FontSize::larger);
                        }
                        else if(aContent.startsWith("large"))
                        {
                            setFontSize(FontSize::large);
                        }
                        else if(aContent.startsWith("x-large"))
                        {
                            setFontSize(FontSize::x_large);
                        }
                        else if(aContent.startsWith("xx-large"))
                        {
                            setFontSize(FontSize::xx_large);
                        }
                        else if(aContent.startsWith("initial"))
                        {
                            setFontSize(FontSize::initial);
                        }
                        else
                        {
                            SvgNumber aNum;

                            if(readSingleNumber(aContent, aNum))
                            {
                                maFontSizeNumber = aNum;
                            }
                        }
                    }
                    break;
                }
                case SVGToken::FontSizeAdjust:
                {
                    break;
                }
                case SVGToken::FontStretch:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("normal"))
                        {
                            setFontStretch(FontStretch::normal);
                        }
                        else if(aContent.startsWith("wider"))
                        {
                            setFontStretch(FontStretch::wider);
                        }
                        else if(aContent.startsWith("narrower"))
                        {
                            setFontStretch(FontStretch::narrower);
                        }
                        else if(aContent.startsWith("ultra-condensed"))
                        {
                            setFontStretch(FontStretch::ultra_condensed);
                        }
                        else if(aContent.startsWith("extra-condensed"))
                        {
                            setFontStretch(FontStretch::extra_condensed);
                        }
                        else if(aContent.startsWith("condensed"))
                        {
                            setFontStretch(FontStretch::condensed);
                        }
                        else if(aContent.startsWith("semi-condensed"))
                        {
                            setFontStretch(FontStretch::semi_condensed);
                        }
                        else if(aContent.startsWith("semi-expanded"))
                        {
                            setFontStretch(FontStretch::semi_expanded);
                        }
                        else if(aContent.startsWith("expanded"))
                        {
                            setFontStretch(FontStretch::expanded);
                        }
                        else if(aContent.startsWith("extra-expanded"))
                        {
                            setFontStretch(FontStretch::extra_expanded);
                        }
                        else if(aContent.startsWith("ultra-expanded"))
                        {
                            setFontStretch(FontStretch::ultra_expanded);
                        }
                    }
                    break;
                }
                case SVGToken::FontStyle:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("normal"))
                        {
                            setFontStyle(FontStyle::normal);
                        }
                        else if(aContent.startsWith("italic"))
                        {
                            setFontStyle(FontStyle::italic);
                        }
                        else if(aContent.startsWith("oblique"))
                        {
                            setFontStyle(FontStyle::oblique);
                        }
                    }
                    break;
                }
                case SVGToken::FontVariant:
                {
                    break;
                }
                case SVGToken::FontWeight:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("100"))
                        {
                            setFontWeight(FontWeight::N100);
                        }
                        else if(aContent.startsWith("200"))
                        {
                            setFontWeight(FontWeight::N200);
                        }
                        else if(aContent.startsWith("300"))
                        {
                            setFontWeight(FontWeight::N300);
                        }
                        else if(aContent.startsWith("400") || aContent.startsWith("normal"))
                        {
                            setFontWeight(FontWeight::N400);
                        }
                        else if(aContent.startsWith("500"))
                        {
                            setFontWeight(FontWeight::N500);
                        }
                        else if(aContent.startsWith("600"))
                        {
                            setFontWeight(FontWeight::N600);
                        }
                        else if(aContent.startsWith("700") || aContent.startsWith("bold"))
                        {
                            setFontWeight(FontWeight::N700);
                        }
                        else if(aContent.startsWith("800"))
                        {
                            setFontWeight(FontWeight::N800);
                        }
                        else if(aContent.startsWith("900"))
                        {
                            setFontWeight(FontWeight::N900);
                        }
                        else if(aContent.startsWith("bolder"))
                        {
                            setFontWeight(FontWeight::bolder);
                        }
                        else if(aContent.startsWith("lighter"))
                        {
                            setFontWeight(FontWeight::lighter);
                        }
                    }
                    break;
                }
                case SVGToken::Direction:
                {
                    break;
                }
                case SVGToken::LetterSpacing:
                {
                    break;
                }
                case SVGToken::TextDecoration:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("none"))
                        {
                            setTextDecoration(TextDecoration::none);
                        }
                        else if(aContent.startsWith("underline"))
                        {
                            setTextDecoration(TextDecoration::underline);
                        }
                        else if(aContent.startsWith("overline"))
                        {
                            setTextDecoration(TextDecoration::overline);
                        }
                        else if(aContent.startsWith("line-through"))
                        {
                            setTextDecoration(TextDecoration::line_through);
                        }
                        else if(aContent.startsWith("blink"))
                        {
                            setTextDecoration(TextDecoration::blink);
                        }
                    }
                    break;
                }
                case SVGToken::UnicodeBidi:
                {
                    break;
                }
                case SVGToken::WordSpacing:
                {
                    break;
                }
                case SVGToken::TextAnchor:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("start"))
                        {
                            setTextAnchor(TextAnchor::start);
                        }
                        else if(aContent.startsWith("middle"))
                        {
                            setTextAnchor(TextAnchor::middle);
                        }
                        else if(aContent.startsWith("end"))
                        {
                            setTextAnchor(TextAnchor::end);
                        }
                    }
                    break;
                }
                case SVGToken::TextAlign:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("left"))
                        {
                            setTextAlign(TextAlign::left);
                        }
                        else if(aContent.startsWith("right"))
                        {
                            setTextAlign(TextAlign::right);
                        }
                        else if(aContent.startsWith("center"))
                        {
                            setTextAlign(TextAlign::center);
                        }
                        else if(aContent.startsWith("justify"))
                        {
                            setTextAlign(TextAlign::justify);
                        }
                    }
                    break;
                }
                case SVGToken::Color:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        maColor = aSvgPaint;
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(std::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    break;
                }
                case SVGToken::Opacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setOpacity(SvgNumber(std::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet()));
                    }
                    break;
                }
                case SVGToken::Visibility:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("visible"))
                        {
                            setVisibility(Visibility::visible);
                        }
                        else if(aContent.startsWith("hidden"))
                        {
                            setVisibility(Visibility::hidden);
                        }
                        else if(aContent.startsWith("collapse"))
                        {
                            setVisibility(Visibility::collapse);
                        }
                        else if(aContent.startsWith("inherit"))
                        {
                            setVisibility(Visibility::inherit);
                        }
                    }
                    break;
                }
                case SVGToken::Title:
                {
                    maTitle = aContent;
                    break;
                }
                case SVGToken::Desc:
                {
                    maDesc = aContent;
                    break;
                }
                case SVGToken::ClipPathProperty:
                {
                    readLocalUrl(aContent, maClipPathXLink);
                    break;
                }
                case SVGToken::Mask:
                {
                    readLocalUrl(aContent, maMaskXLink);
                    break;
                }
                case SVGToken::ClipRule:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrNonzero))
                        {
                            maClipRule = FillRule::nonzero;
                        }
                        else if(aContent.match(commonStrings::aStrEvenOdd))
                        {
                            maClipRule = FillRule::evenodd;
                        }
                    }
                    break;
                }
                case SVGToken::Marker:
                {
                    if(bCaseIndependent)
                    {
                        readLocalUrl(aContent, maMarkerEndXLink);
                        maMarkerStartXLink = maMarkerMidXLink = maMarkerEndXLink;
                    }
                    break;
                }
                case SVGToken::MarkerStart:
                {
                    readLocalUrl(aContent, maMarkerStartXLink);
                    break;
                }
                case SVGToken::MarkerMid:
                {
                    readLocalUrl(aContent, maMarkerMidXLink);
                    break;
                }
                case SVGToken::MarkerEnd:
                {
                    readLocalUrl(aContent, maMarkerEndXLink);
                    break;
                }
                case SVGToken::Display:
                {
                    // There may be display:none statements inside of style defines, e.g. the following line:
                    // style="display:none"
                    // taken from a svg example; this needs to be parsed and set at the owning node. Do not call
                    // mrOwner.parseAttribute(...) here, this would lead to a recursion
                    if(!aContent.isEmpty())
                    {
                        mrOwner.setDisplay(getDisplayFromContent(aContent));
                    }
                    break;
                }
                case SVGToken::BaselineShift:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumber aNum;

                        if(aContent.startsWith("sub"))
                        {
                            setBaselineShift(BaselineShift::Sub);
                        }
                        else if(aContent.startsWith("super"))
                        {
                            setBaselineShift(BaselineShift::Super);
                        }
                        else if(readSingleNumber(aContent, aNum))
                        {
                            maBaselineShiftNumber = aNum;

                            if(SvgUnit::percent == aNum.getUnit())
                            {
                                setBaselineShift(BaselineShift::Percentage);
                            }
                            else
                            {
                                setBaselineShift(BaselineShift::Length);
                            }
                        }
                        else
                        {
                            // no BaselineShift or inherit (which is automatically)
                            setBaselineShift(BaselineShift::Baseline);
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        // #i125258# ask if fill is a direct hard attribute (no hierarchy)
        bool SvgStyleAttributes::isFillSet() const
        {
            if(mbIsClipPathContent)
            {
                return false;
            }
            else if(maFill.isSet())
            {
                return true;
            }

            return false;
        }

        const basegfx::BColor* SvgStyleAttributes::getCurrentColor() const
        {
            static basegfx::BColor aBlack(0.0, 0.0, 0.0);
            const basegfx::BColor *aColor = getColor();
            if( aColor )
                return aColor;
            else
                return &aBlack;
        }

        const basegfx::BColor* SvgStyleAttributes::getFill() const
        {
            if(maFill.isSet())
            {
                if(maFill.isCurrent())
                {
                    return getCurrentColor();
                }
                else if(maFill.isOn())
                {
                    return &maFill.getBColor();
                }
                else if(mbIsClipPathContent)
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                    if (pSvgStyleAttributes && maResolvingParent[0] < nStyleDepthLimit)
                    {
                        ++maResolvingParent[0];
                        const basegfx::BColor* pFill = pSvgStyleAttributes->getFill();
                        --maResolvingParent[0];

                        return pFill;
                    }
                }
            }
            else if (maNodeFillURL.isEmpty())
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[0] < nStyleDepthLimit)
                {
                    ++maResolvingParent[0];
                    const basegfx::BColor* pFill = pSvgStyleAttributes->getFill();
                    --maResolvingParent[0];

                    if(mbIsClipPathContent)
                    {
                        if (pFill)
                        {
                            return pFill;
                        }
                        else
                        {
                            static basegfx::BColor aBlack(0.0, 0.0, 0.0);
                            return &aBlack;
                        }
                    }
                    else
                    {
                        return pFill;
                    }
                }
            }

            return nullptr;
        }

        const basegfx::BColor* SvgStyleAttributes::getStroke() const
        {
            if(maStroke.isSet())
            {
                if(maStroke.isCurrent())
                {
                    return getCurrentColor();
                }
                else if(maStroke.isOn())
                {
                    return &maStroke.getBColor();
                }
            }
            else if (maNodeStrokeURL.isEmpty())
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[1] < nStyleDepthLimit)
                {
                    ++maResolvingParent[1];
                    auto ret = pSvgStyleAttributes->getStroke();
                    --maResolvingParent[1];
                    return ret;
                }
            }

            return nullptr;
        }

        const basegfx::BColor& SvgStyleAttributes::getStopColor() const
        {
            if(maStopColor.isCurrent())
            {
                return *getCurrentColor();
            }
            else
            {
                return maStopColor.getBColor();
            }
        }

        const SvgGradientNode* SvgStyleAttributes::getSvgGradientNodeFill() const
        {
            if (!maFill.isSet())
            {
                if (!maNodeFillURL.isEmpty())
                {
                    const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(maNodeFillURL);

                    if(pNode)
                    {
                        if(SVGToken::LinearGradient == pNode->getType() || SVGToken::RadialGradient == pNode->getType())
                        {
                            return static_cast< const SvgGradientNode* >(pNode);
                        }
                    }
                }
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[2] < nStyleDepthLimit)
                {
                    ++maResolvingParent[2];
                    auto ret = pSvgStyleAttributes->getSvgGradientNodeFill();
                    --maResolvingParent[2];
                    return ret;
                }
            }

            return nullptr;
        }

        const SvgGradientNode* SvgStyleAttributes::getSvgGradientNodeStroke() const
        {
            if (!maStroke.isSet())
            {
                if(!maNodeStrokeURL.isEmpty())
                {
                    const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(maNodeStrokeURL);

                    if(pNode)
                    {
                        if(SVGToken::LinearGradient == pNode->getType() || SVGToken::RadialGradient  == pNode->getType())
                        {
                            return static_cast< const SvgGradientNode* >(pNode);
                        }
                    }
                }

                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[3] < nStyleDepthLimit)
                {
                    ++maResolvingParent[3];
                    auto ret = pSvgStyleAttributes->getSvgGradientNodeStroke();
                    --maResolvingParent[3];
                    return ret;
                }
            }

            return nullptr;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeFill() const
        {
            if (!maFill.isSet())
            {
                if (!maNodeFillURL.isEmpty())
                {
                    const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(maNodeFillURL);

                    if(pNode)
                    {
                        if(SVGToken::Pattern == pNode->getType())
                        {
                            return static_cast< const SvgPatternNode* >(pNode);
                        }
                    }
                }

                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[4] < nStyleDepthLimit)
                {
                    ++maResolvingParent[4];
                    auto ret = pSvgStyleAttributes->getSvgPatternNodeFill();
                    --maResolvingParent[4];
                    return ret;
                }
            }

            return nullptr;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeStroke() const
        {
            if (!maStroke.isSet())
            {
                if(!maNodeStrokeURL.isEmpty())
                {
                    const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(maNodeStrokeURL);

                    if(pNode)
                    {
                        if(SVGToken::Pattern == pNode->getType())
                        {
                            return static_cast< const SvgPatternNode* >(pNode);
                        }
                    }
                }

                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[5] < nStyleDepthLimit)
                {
                    ++maResolvingParent[5];
                    auto ret = pSvgStyleAttributes->getSvgPatternNodeStroke();
                    --maResolvingParent[5];
                    return ret;
                }
            }

            return nullptr;
        }

        SvgNumber SvgStyleAttributes::getStrokeWidth() const
        {
            if(maStrokeWidth.isSet())
            {
                return maStrokeWidth;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[6] < nStyleDepthLimit)
            {
                ++maResolvingParent[6];
                auto ret = pSvgStyleAttributes->getStrokeWidth();
                --maResolvingParent[6];
                return ret;
            }

            if(mbIsClipPathContent)
            {
                return SvgNumber(0.0);
            }

            // default is 1
            return SvgNumber(1.0);
        }

        SvgNumber SvgStyleAttributes::getStopOpacity() const
        {
            if(maStopOpacity.isSet())
            {
                return maStopOpacity;
            }

            // default is 1
            return SvgNumber(1.0);
        }

        SvgNumber SvgStyleAttributes::getFillOpacity() const
        {
            if(maFillOpacity.isSet())
            {
                return maFillOpacity;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[7] < nStyleDepthLimit)
            {
                ++maResolvingParent[7];
                auto ret = pSvgStyleAttributes->getFillOpacity();
                --maResolvingParent[7];
                return ret;
            }

            // default is 1
            return SvgNumber(1.0);
        }

        SvgNumber SvgStyleAttributes::getOpacity() const
        {
            if(maOpacity.isSet())
            {
                return maOpacity;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[8] < nStyleDepthLimit)
            {
                ++maResolvingParent[8];
                auto ret = pSvgStyleAttributes->getOpacity();
                --maResolvingParent[8];
                return ret;
            }

            // default is 1
            return SvgNumber(1.0);
        }

        Visibility SvgStyleAttributes::getVisibility() const
        {
            if(Visibility::notset == maVisibility || Visibility::inherit == maVisibility)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[9] < nStyleDepthLimit)
                {
                    ++maResolvingParent[9];
                    auto ret = pSvgStyleAttributes->getVisibility();
                    --maResolvingParent[9];
                    return ret;
                }
                //default is Visible
                return Visibility::visible;
            }

            // Visibility correction/exception for self-exported SVGs:
            // When Impress exports single or multi-page SVGs, it puts the
            // single slides into <g visibility="hidden">. Not sure why
            // this happens, but this leads (correctly) to empty imported
            // Graphics.
            // Thus, if Visibility::hidden is active and owner is a SVGToken::G
            // and it's parent is also a SVGToken::G and it has a Class 'SlideGroup'
            // set, check if we are an Impress export.
            // We are an Impress export if an SVG-Node titled 'ooo:meta_slides'
            // exists.
            // All together gives:
            if(Visibility::hidden == maVisibility
                && SVGToken::G == mrOwner.getType()
                && nullptr != mrOwner.getDocument().findSvgNodeById("ooo:meta_slides"))
            {
                const SvgNode* pParent(mrOwner.getParent());

                if(nullptr != pParent && SVGToken::G == pParent->getType() && pParent->getClass())
                {
                    const OUString aClass(*pParent->getClass());

                    if("SlideGroup" == aClass)
                    {
                        // if we detect this exception,
                        // override Visibility::hidden -> Visibility::visible
                        return Visibility::visible;
                    }
                }
            }

            return maVisibility;
        }

        FillRule SvgStyleAttributes::getFillRule() const
        {
            if(FillRule::notset != maFillRule)
            {
                return maFillRule;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[10] < nStyleDepthLimit)
            {
                ++maResolvingParent[10];
                auto ret = pSvgStyleAttributes->getFillRule();
                --maResolvingParent[10];
                return ret;
            }

            // default is NonZero
            return FillRule::nonzero;
        }

        const SvgNumberVector& SvgStyleAttributes::getStrokeDasharray() const
        {
            if(!maStrokeDasharray.empty())
            {
                return maStrokeDasharray;
            }
            else if(mbStrokeDasharraySet)
            {
                // #121221# is set to empty *by purpose*, do not visit parent styles
                return maStrokeDasharray;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[11] < nStyleDepthLimit)
            {
                ++maResolvingParent[11];
                const SvgNumberVector& ret = pSvgStyleAttributes->getStrokeDasharray();
                --maResolvingParent[11];
                return ret;
            }

            // default empty
            return maStrokeDasharray;
        }

        SvgNumber SvgStyleAttributes::getStrokeDashOffset() const
        {
            if(maStrokeDashOffset.isSet())
            {
                return maStrokeDashOffset;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[12] < nStyleDepthLimit)
            {
                ++maResolvingParent[12];
                auto ret = pSvgStyleAttributes->getStrokeDashOffset();
                --maResolvingParent[12];
                return ret;
            }

            // default is 0
            return SvgNumber(0.0);
        }

        StrokeLinecap SvgStyleAttributes::getStrokeLinecap() const
        {
            if(maStrokeLinecap != StrokeLinecap::notset)
            {
                return maStrokeLinecap;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[13] < nStyleDepthLimit)
            {
                ++maResolvingParent[13];
                auto ret = pSvgStyleAttributes->getStrokeLinecap();
                --maResolvingParent[13];
                return ret;
            }

            // default is StrokeLinecap::butt
            return StrokeLinecap::butt;
        }

        StrokeLinejoin SvgStyleAttributes::getStrokeLinejoin() const
        {
            if(maStrokeLinejoin != StrokeLinejoin::notset)
            {
                return maStrokeLinejoin;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[14] < nStyleDepthLimit)
            {
                ++maResolvingParent[14];
                auto ret = pSvgStyleAttributes->getStrokeLinejoin();
                --maResolvingParent[14];
                return ret;
            }

            // default is StrokeLinejoin::butt
            return StrokeLinejoin::miter;
        }

        SvgNumber SvgStyleAttributes::getStrokeMiterLimit() const
        {
            if(maStrokeMiterLimit.isSet())
            {
                return maStrokeMiterLimit;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[15] < nStyleDepthLimit)
            {
                ++maResolvingParent[15];
                auto ret = pSvgStyleAttributes->getStrokeMiterLimit();
                --maResolvingParent[15];
                return ret;
            }

            // default is 4
            return SvgNumber(4.0, SvgUnit::none);
        }

        SvgNumber SvgStyleAttributes::getStrokeOpacity() const
        {
            if(maStrokeOpacity.isSet())
            {
                return maStrokeOpacity;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[16] < nStyleDepthLimit)
            {
                ++maResolvingParent[16];
                auto ret = pSvgStyleAttributes->getStrokeOpacity();
                --maResolvingParent[16];
                return ret;
            }

            // default is 1
            return SvgNumber(1.0);
        }

        const SvgStringVector& SvgStyleAttributes::getFontFamily() const
        {
            if(!maFontFamily.empty() && !maFontFamily[0].startsWith("inherit"))
            {
                return maFontFamily;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[17] < nStyleDepthLimit)
            {
                ++maResolvingParent[17];
                const SvgStringVector& ret = pSvgStyleAttributes->getFontFamily();
                --maResolvingParent[17];
                return ret;
            }

            // default is empty
            return maFontFamily;
        }

        SvgNumber SvgStyleAttributes::getFontSizeNumber() const
        {
            // default size is 'medium', i.e. 12 pt, or 16px
            constexpr double aDefaultSize = o3tl::convert(12.0, o3tl::Length::pt, o3tl::Length::px);

            if(maFontSizeNumber.isSet())
            {
                if(!maFontSizeNumber.isPositive())
                    return aDefaultSize;

                // #122524# Handle SvgUnit::percent relative to parent FontSize (see SVG1.1
                // spec 10.10 Font selection properties \91font-size\92, lastline (click 'normative
                // definition of the property')
                if(SvgUnit::percent == maFontSizeNumber.getUnit())
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();

                        return SvgNumber(
                            aParentNumber.getNumber() * maFontSizeNumber.getNumber() * 0.01,
                            aParentNumber.getUnit(),
                            true);
                    }
                    // if there's no parent style, set the font size based on the default size
                    // 100% = 16px
                    return SvgNumber(
                        maFontSizeNumber.getNumber() * aDefaultSize / 100.0, SvgUnit::px, true);
                }
                else if((SvgUnit::em == maFontSizeNumber.getUnit()) || (SvgUnit::ex == maFontSizeNumber.getUnit()))
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();

                        return SvgNumber(
                            aParentNumber.getNumber() * maFontSizeNumber.getNumber(),
                            aParentNumber.getUnit(),
                            true);
                    }
                }

                return maFontSizeNumber;
            }

            //In CSS2, the suggested scaling factor between adjacent indexes is 1.2
            switch(maFontSize)
            {
                case FontSize::notset:
                    break;
                case FontSize::xx_small:
                {
                    return SvgNumber(aDefaultSize / 1.728);
                }
                case FontSize::x_small:
                {
                    return SvgNumber(aDefaultSize / 1.44);
                }
                case FontSize::small:
                {
                    return SvgNumber(aDefaultSize / 1.2);
                }
                case FontSize::smaller:
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();
                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();
                        return SvgNumber(aParentNumber.getNumber() / 1.2, aParentNumber.getUnit());
                    }
                    [[fallthrough]];
                }
                case FontSize::medium:
                case FontSize::initial:
                {
                    return SvgNumber(aDefaultSize);
                }
                case FontSize::large:
                {
                    return SvgNumber(aDefaultSize * 1.2);
                }
                case FontSize::larger:
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();
                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();
                        return SvgNumber(aParentNumber.getNumber() * 1.2, aParentNumber.getUnit());
                    }
                    [[fallthrough]];
                }
                case FontSize::x_large:
                {
                    return SvgNumber(aDefaultSize * 1.44);
                }
                case FontSize::xx_large:
                {
                    return SvgNumber(aDefaultSize * 1.728);
                }
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFontSizeNumber();
            }

            return SvgNumber(aDefaultSize);
        }

        FontStretch SvgStyleAttributes::getFontStretch() const
        {
            if(maFontStretch != FontStretch::notset)
            {
                if(FontStretch::wider != maFontStretch && FontStretch::narrower != maFontStretch)
                {
                    return maFontStretch;
                }
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[18] < nStyleDepthLimit)
            {
                ++maResolvingParent[18];
                FontStretch aInherited = pSvgStyleAttributes->getFontStretch();
                --maResolvingParent[18];

                if(FontStretch::wider == maFontStretch)
                {
                    aInherited = getWider(aInherited);
                }
                else if(FontStretch::narrower == maFontStretch)
                {
                    aInherited = getNarrower(aInherited);
                }

                return aInherited;
            }

            // default is FontStretch::normal
            return FontStretch::normal;
        }

        FontStyle SvgStyleAttributes::getFontStyle() const
        {
            if(maFontStyle != FontStyle::notset)
            {
                return maFontStyle;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[19] < nStyleDepthLimit)
            {
                ++maResolvingParent[19];
                auto ret = pSvgStyleAttributes->getFontStyle();
                --maResolvingParent[19];
                return ret;
            }

            // default is FontStyle::normal
            return FontStyle::normal;
        }

        FontWeight SvgStyleAttributes::getFontWeight() const
        {
            if(maFontWeight != FontWeight::notset)
            {
                if(FontWeight::bolder != maFontWeight && FontWeight::lighter != maFontWeight)
                {
                    return maFontWeight;
                }
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[20] < nStyleDepthLimit)
            {
                ++maResolvingParent[20];
                FontWeight aInherited = pSvgStyleAttributes->getFontWeight();
                --maResolvingParent[20];

                if(FontWeight::bolder == maFontWeight)
                {
                    aInherited = getBolder(aInherited);
                }
                else if(FontWeight::lighter == maFontWeight)
                {
                    aInherited = getLighter(aInherited);
                }

                return aInherited;
            }

            // default is FontWeight::N400 (FontWeight::normal)
            return FontWeight::N400;
        }

        TextAlign SvgStyleAttributes::getTextAlign() const
        {
            if(maTextAlign != TextAlign::notset)
            {
                return maTextAlign;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[21] < nStyleDepthLimit)
            {
                ++maResolvingParent[21];
                auto ret = pSvgStyleAttributes->getTextAlign();
                --maResolvingParent[21];
                return ret;
            }

            // default is TextAlign::left
            return TextAlign::left;
        }

        const SvgStyleAttributes* SvgStyleAttributes::getTextDecorationDefiningSvgStyleAttributes() const
        {
            if(maTextDecoration != TextDecoration::notset)
            {
                return this;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[22] < nStyleDepthLimit)
            {
                ++maResolvingParent[22];
                auto ret = pSvgStyleAttributes->getTextDecorationDefiningSvgStyleAttributes();
                --maResolvingParent[22];
                return ret;
            }

            // default is 0
            return nullptr;
        }

        TextDecoration SvgStyleAttributes::getTextDecoration() const
        {
            const SvgStyleAttributes* pDefining = getTextDecorationDefiningSvgStyleAttributes();

            if(pDefining)
            {
                return pDefining->maTextDecoration;
            }
            else
            {
                // default is TextDecoration::none
                return TextDecoration::none;
            }
        }

        TextAnchor SvgStyleAttributes::getTextAnchor() const
        {
            if(maTextAnchor != TextAnchor::notset)
            {
                return maTextAnchor;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[23] < nStyleDepthLimit)
            {
                ++maResolvingParent[23];
                auto ret = pSvgStyleAttributes->getTextAnchor();
                --maResolvingParent[23];
                return ret;
            }

            // default is TextAnchor::start
            return TextAnchor::start;
        }

        const basegfx::BColor* SvgStyleAttributes::getColor() const
        {
            if(maColor.isSet())
            {
                if(maColor.isCurrent())
                {
                    OSL_ENSURE(false, "Svg error: current color uses current color (!)");
                    return nullptr;
                }
                else if(maColor.isOn())
                {
                    return &maColor.getBColor();
                }
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[24] < nStyleDepthLimit)
                {
                    ++maResolvingParent[24];
                    auto ret = pSvgStyleAttributes->getColor();
                    --maResolvingParent[24];
                    return ret;
                }
            }

            return nullptr;
        }

        OUString const & SvgStyleAttributes::getClipPathXLink() const
        {
            return maClipPathXLink;
        }

        const SvgClipPathNode* SvgStyleAttributes::accessClipPathXLink() const
        {
            if(!mpClipPathXLink)
            {
                const OUString aClipPath(getClipPathXLink());

                if(!aClipPath.isEmpty())
                {
                    const_cast< SvgStyleAttributes* >(this)->mpClipPathXLink = dynamic_cast< const SvgClipPathNode* >(mrOwner.getDocument().findSvgNodeById(aClipPath));
                }
            }

            return mpClipPathXLink;
        }

        OUString SvgStyleAttributes::getMaskXLink() const
        {
            if(!maMaskXLink.isEmpty())
            {
                return maMaskXLink;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && !pSvgStyleAttributes->maMaskXLink.isEmpty() && maResolvingParent[25] < nStyleDepthLimit)
            {
                ++maResolvingParent[25];
                auto ret = pSvgStyleAttributes->getMaskXLink();
                --maResolvingParent[25];
                return ret;
            }

            return OUString();
        }

        const SvgMaskNode* SvgStyleAttributes::accessMaskXLink() const
        {
            if(!mpMaskXLink)
            {
                const OUString aMask(getMaskXLink());

                if(!aMask.isEmpty())
                {
                    const_cast< SvgStyleAttributes* >(this)->mpMaskXLink = dynamic_cast< const SvgMaskNode* >(mrOwner.getDocument().findSvgNodeById(aMask));
                }
            }

            return mpMaskXLink;
        }

        OUString SvgStyleAttributes::getMarkerStartXLink() const
        {
            if(!maMarkerStartXLink.isEmpty())
            {
                return maMarkerStartXLink;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[26] < nStyleDepthLimit)
            {
                ++maResolvingParent[26];
                auto ret = pSvgStyleAttributes->getMarkerStartXLink();
                --maResolvingParent[26];
                return ret;
            }

            return OUString();
        }

        const SvgMarkerNode* SvgStyleAttributes::accessMarkerStartXLink() const
        {
            if(!mpMarkerStartXLink)
            {
                const OUString aMarker(getMarkerStartXLink());

                if(!aMarker.isEmpty())
                {
                    const_cast< SvgStyleAttributes* >(this)->mpMarkerStartXLink = dynamic_cast< const SvgMarkerNode* >(mrOwner.getDocument().findSvgNodeById(getMarkerStartXLink()));
                }
            }

            return mpMarkerStartXLink;
        }

        OUString SvgStyleAttributes::getMarkerMidXLink() const
        {
            if(!maMarkerMidXLink.isEmpty())
            {
                return maMarkerMidXLink;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[27] < nStyleDepthLimit)
            {
                ++maResolvingParent[27];
                auto ret = pSvgStyleAttributes->getMarkerMidXLink();
                --maResolvingParent[27];
                return ret;
            }

            return OUString();
        }

        const SvgMarkerNode* SvgStyleAttributes::accessMarkerMidXLink() const
        {
            if(!mpMarkerMidXLink)
            {
                const OUString aMarker(getMarkerMidXLink());

                if(!aMarker.isEmpty())
                {
                    const_cast< SvgStyleAttributes* >(this)->mpMarkerMidXLink = dynamic_cast< const SvgMarkerNode* >(mrOwner.getDocument().findSvgNodeById(getMarkerMidXLink()));
                }
            }

            return mpMarkerMidXLink;
        }

        OUString SvgStyleAttributes::getMarkerEndXLink() const
        {
            if(!maMarkerEndXLink.isEmpty())
            {
                return maMarkerEndXLink;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if (pSvgStyleAttributes && maResolvingParent[28] < nStyleDepthLimit)
            {
                ++maResolvingParent[28];
                auto ret = pSvgStyleAttributes->getMarkerEndXLink();
                --maResolvingParent[28];
                return ret;
            }

            return OUString();
        }

        const SvgMarkerNode* SvgStyleAttributes::accessMarkerEndXLink() const
        {
            if(!mpMarkerEndXLink)
            {
                const OUString aMarker(getMarkerEndXLink());

                if(!aMarker.isEmpty())
                {
                    const_cast< SvgStyleAttributes* >(this)->mpMarkerEndXLink = dynamic_cast< const SvgMarkerNode* >(mrOwner.getDocument().findSvgNodeById(getMarkerEndXLink()));
                }
            }

            return mpMarkerEndXLink;
        }

        SvgNumber SvgStyleAttributes::getBaselineShiftNumber() const
        {
            // #122524# Handle SvgUnit::percent relative to parent BaselineShift
            if(SvgUnit::percent == maBaselineShiftNumber.getUnit())
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if (pSvgStyleAttributes && maResolvingParent[29] < nStyleDepthLimit)
                {
                    ++maResolvingParent[29];
                    const SvgNumber aParentNumber = pSvgStyleAttributes->getBaselineShiftNumber();
                    --maResolvingParent[29];

                    return SvgNumber(
                        aParentNumber.getNumber() * maBaselineShiftNumber.getNumber() * 0.01,
                        aParentNumber.getUnit(),
                        true);
                }
            }

            return maBaselineShiftNumber;
        }
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

