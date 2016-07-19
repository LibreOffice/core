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

#include <svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
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
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <svgpatternnode.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

namespace svgio
{
    namespace svgreader
    {
        basegfx::B2DLineJoin StrokeLinejoinToB2DLineJoin(StrokeLinejoin aStrokeLinejoin)
        {
            if(StrokeLinejoin_round == aStrokeLinejoin)
            {
                return basegfx::B2DLineJoin::Round;
            }
            else if(StrokeLinejoin_bevel == aStrokeLinejoin)
            {
                return basegfx::B2DLineJoin::Bevel;
            }

            return basegfx::B2DLineJoin::Miter;
        }

        css::drawing::LineCap StrokeLinecapToDrawingLineCap(StrokeLinecap aStrokeLinecap)
        {
            switch(aStrokeLinecap)
            {
                default: /* StrokeLinecap_notset, StrokeLinecap_butt */
                {
                    return css::drawing::LineCap_BUTT;
                }
                case StrokeLinecap_round:
                {
                    return css::drawing::LineCap_ROUND;
                }
                case StrokeLinecap_square:
                {
                    return css::drawing::LineCap_SQUARE;
                }
            }
        }

        FontStretch getWider(FontStretch aSource)
        {
            switch(aSource)
            {
                case FontStretch_ultra_condensed: aSource = FontStretch_extra_condensed; break;
                case FontStretch_extra_condensed: aSource = FontStretch_condensed; break;
                case FontStretch_condensed: aSource = FontStretch_semi_condensed; break;
                case FontStretch_semi_condensed: aSource = FontStretch_normal; break;
                case FontStretch_normal: aSource = FontStretch_semi_expanded; break;
                case FontStretch_semi_expanded: aSource = FontStretch_expanded; break;
                case FontStretch_expanded: aSource = FontStretch_extra_expanded; break;
                case FontStretch_extra_expanded: aSource = FontStretch_ultra_expanded; break;
                default: break;
            }

            return aSource;
        }

        FontStretch getNarrower(FontStretch aSource)
        {
            switch(aSource)
            {
                case FontStretch_extra_condensed: aSource = FontStretch_ultra_condensed; break;
                case FontStretch_condensed: aSource = FontStretch_extra_condensed; break;
                case FontStretch_semi_condensed: aSource = FontStretch_condensed; break;
                case FontStretch_normal: aSource = FontStretch_semi_condensed; break;
                case FontStretch_semi_expanded: aSource = FontStretch_normal; break;
                case FontStretch_expanded: aSource = FontStretch_semi_expanded; break;
                case FontStretch_extra_expanded: aSource = FontStretch_expanded; break;
                case FontStretch_ultra_expanded: aSource = FontStretch_extra_expanded; break;
                default: break;
            }

            return aSource;
        }

        FontWeight getBolder(FontWeight aSource)
        {
            switch(aSource)
            {
                case FontWeight_100: aSource = FontWeight_200; break;
                case FontWeight_200: aSource = FontWeight_300; break;
                case FontWeight_300: aSource = FontWeight_400; break;
                case FontWeight_400: aSource = FontWeight_500; break;
                case FontWeight_500: aSource = FontWeight_600; break;
                case FontWeight_600: aSource = FontWeight_700; break;
                case FontWeight_700: aSource = FontWeight_800; break;
                case FontWeight_800: aSource = FontWeight_900; break;
                default: break;
            }

            return aSource;
        }

        FontWeight getLighter(FontWeight aSource)
        {
            switch(aSource)
            {
                case FontWeight_200: aSource = FontWeight_100; break;
                case FontWeight_300: aSource = FontWeight_200; break;
                case FontWeight_400: aSource = FontWeight_300; break;
                case FontWeight_500: aSource = FontWeight_400; break;
                case FontWeight_600: aSource = FontWeight_500; break;
                case FontWeight_700: aSource = FontWeight_600; break;
                case FontWeight_800: aSource = FontWeight_700; break;
                case FontWeight_900: aSource = FontWeight_800; break;
                default: break;
            }

            return aSource;
        }

        ::FontWeight getVclFontWeight(FontWeight aSource)
        {
            ::FontWeight nRetval(WEIGHT_NORMAL);

            switch(aSource)
            {
                case FontWeight_100: nRetval = WEIGHT_ULTRALIGHT; break;
                case FontWeight_200: nRetval = WEIGHT_LIGHT; break;
                case FontWeight_300: nRetval = WEIGHT_SEMILIGHT; break;
                case FontWeight_400: nRetval = WEIGHT_NORMAL; break;
                case FontWeight_500: nRetval = WEIGHT_MEDIUM; break;
                case FontWeight_600: nRetval = WEIGHT_SEMIBOLD; break;
                case FontWeight_700: nRetval = WEIGHT_BOLD; break;
                case FontWeight_800: nRetval = WEIGHT_ULTRABOLD; break;
                case FontWeight_900: nRetval = WEIGHT_BLACK; break;
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
                skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);
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
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(':'), nPos, nLen);
                copyToLimiter(rCandidate, sal_Unicode(';'), nPos, aTokenValue, nLen);
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(';'), nPos, nLen);

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
                        aNewOUTokenValue += aOUTokenValue.copy(0, nIndexTokenImportant);
                    }

                    if(aOUTokenValue.getLength() > nIndexTokenImportant + aTokenImportant.getLength())
                    {
                        // copy content after token
                        aNewOUTokenValue += aOUTokenValue.copy(nIndexTokenImportant + aTokenImportant.getLength());
                    }

                    // remove spaces
                    aOUTokenValue = aNewOUTokenValue.trim();
                }

                // valid token-value pair, parse it
                parseStyleAttribute(aOUTokenName, StrToSVGToken(aOUTokenName, true), aOUTokenValue, true);
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
            drawinglayer::primitive2d::Primitive2DContainer& rSource) const
        {
            if(!rSource.empty())
            {
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
                        aMergedArea = basegfx::tools::mergeToSinglePolyPolygon(aTextFillVector);
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
                    rTarget.append(rSource);
                }

                // add stroke
                if(aMergedArea.count() && bStrokeUsed)
                {
                    // create text stroke content
                    add_stroke(aMergedArea, rTarget, aMergedArea.getB2DRange());
                }
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

            if(!aSvgGradientEntryVector.empty())
            {
                basegfx::B2DHomMatrix aGeoToUnit;
                basegfx::B2DHomMatrix aGradientTransform;

                if(rFillGradient.getGradientTransform())
                {
                    aGradientTransform = *rFillGradient.getGradientTransform();
                }

                if(userSpaceOnUse == rFillGradient.getGradientUnits())
                {
                    aGeoToUnit.translate(-rGeoRange.getMinX(), -rGeoRange.getMinY());
                    aGeoToUnit.scale(1.0 / rGeoRange.getWidth(), 1.0 / rGeoRange.getHeight());
                }

                if(SVGTokenLinearGradient == rFillGradient.getType())
                {
                    basegfx::B2DPoint aStart(0.0, 0.0);
                    basegfx::B2DPoint aEnd(1.0, 0.0);

                    if(userSpaceOnUse == rFillGradient.getGradientUnits())
                    {
                        // all possible units
                        aStart.setX(rFillGradient.getX1().solve(mrOwner, xcoordinate));
                        aStart.setY(rFillGradient.getY1().solve(mrOwner, ycoordinate));
                        aEnd.setX(rFillGradient.getX2().solve(mrOwner, xcoordinate));
                        aEnd.setY(rFillGradient.getY2().solve(mrOwner, ycoordinate));
                    }
                    else
                    {
                        // fractions or percent relative to object bounds
                        const SvgNumber X1(rFillGradient.getX1());
                        const SvgNumber Y1(rFillGradient.getY1());
                        const SvgNumber X2(rFillGradient.getX2());
                        const SvgNumber Y2(rFillGradient.getY2());

                        aStart.setX(Unit_percent == X1.getUnit() ? X1.getNumber() * 0.01 : X1.getNumber());
                        aStart.setY(Unit_percent == Y1.getUnit() ? Y1.getNumber() * 0.01 : Y1.getNumber());
                        aEnd.setX(Unit_percent == X2.getUnit() ? X2.getNumber() * 0.01 : X2.getNumber());
                        aEnd.setY(Unit_percent == Y2.getUnit() ? Y2.getNumber() * 0.01 : Y2.getNumber());
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
                            aSvgGradientEntryVector,
                            aStart,
                            aEnd,
                            userSpaceOnUse != rFillGradient.getGradientUnits(),
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

                    if(userSpaceOnUse == rFillGradient.getGradientUnits())
                    {
                        // all possible units
                        aStart.setX(rFillGradient.getCx().solve(mrOwner, xcoordinate));
                        aStart.setY(rFillGradient.getCy().solve(mrOwner, ycoordinate));
                        fRadius = rFillGradient.getR().solve(mrOwner);

                        if(bFocal)
                        {
                            aFocal.setX(pFx ? pFx->solve(mrOwner, xcoordinate) : aStart.getX());
                            aFocal.setY(pFy ? pFy->solve(mrOwner, ycoordinate) : aStart.getY());
                        }
                    }
                    else
                    {
                        // fractions or percent relative to object bounds
                        const SvgNumber Cx(rFillGradient.getCx());
                        const SvgNumber Cy(rFillGradient.getCy());
                        const SvgNumber R(rFillGradient.getR());

                        aStart.setX(Unit_percent == Cx.getUnit() ? Cx.getNumber() * 0.01 : Cx.getNumber());
                        aStart.setY(Unit_percent == Cy.getUnit() ? Cy.getNumber() * 0.01 : Cy.getNumber());
                        fRadius = (Unit_percent == R.getUnit()) ? R.getNumber() * 0.01 : R.getNumber();

                        if(bFocal)
                        {
                            aFocal.setX(pFx ? (Unit_percent == pFx->getUnit() ? pFx->getNumber() * 0.01 : pFx->getNumber()) : aStart.getX());
                            aFocal.setY(pFy ? (Unit_percent == pFy->getUnit() ? pFy->getNumber() * 0.01 : pFy->getNumber()) : aStart.getY());
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
                            aSvgGradientEntryVector,
                            aStart,
                            fRadius,
                            userSpaceOnUse != rFillGradient.getGradientUnits(),
                            rFillGradient.getSpreadMethod(),
                            bFocal ? &aFocal : nullptr));
                }
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
                            aNewTarget));
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

            if(!rPrimitives.empty())
            {
                double fTargetWidth(rGeoRange.getWidth());
                double fTargetHeight(rGeoRange.getHeight());

                if(fTargetWidth > 0.0 && fTargetHeight > 0.0)
                {
                    // get relative values from pattern
                    double fX(0.0);
                    double fY(0.0);
                    double fW(0.0);
                    double fH(0.0);

                    rFillPattern.getValuesRelative(fX, fY, fW, fH, rGeoRange, mrOwner);

                    if(fW > 0.0 && fH > 0.0)
                    {
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
                            const SvgUnits aPatternContentUnits(rFillPattern.getPatternContentUnits() ? *rFillPattern.getPatternContentUnits() : userSpaceOnUse);

                            if(userSpaceOnUse == aPatternContentUnits)
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
                                    aPrimitives));

                            aPrimitives = drawinglayer::primitive2d::Primitive2DContainer { xRef };
                        }

                        // embed in PatternFillPrimitive2D
                        rTarget.push_back(
                            new drawinglayer::primitive2d::PatternFillPrimitive2D(
                                rPath,
                                aPrimitives,
                                aReferenceRange));
                    }
                }
            }
        }

        void SvgStyleAttributes::add_fill(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::B2DRange& rGeoRange) const
        {
            const basegfx::BColor* pFill = getFill();
            const SvgGradientNode* pFillGradient = getSvgGradientNodeFill();
            const SvgPatternNode* pFillPattern = getSvgPatternNodeFill();

            if(pFill || pFillGradient || pFillPattern)
            {
                const double fFillOpacity(getFillOpacity().solve(mrOwner));

                if(basegfx::fTools::more(fFillOpacity, 0.0))
                {
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

                    if(!aNewFill.empty())
                    {
                        if(basegfx::fTools::less(fFillOpacity, 1.0))
                        {
                            // embed in UnifiedTransparencePrimitive2D
                            rTarget.push_back(
                                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                    aNewFill,
                                    1.0 - fFillOpacity));
                        }
                        else
                        {
                            // append
                            rTarget.append(aNewFill);
                        }
                    }
                }
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

            if(pStroke || pStrokeGradient || pStrokePattern)
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewStroke;
                const double fStrokeOpacity(getStrokeOpacity().solve(mrOwner));

                if(basegfx::fTools::more(fStrokeOpacity, 0.0))
                {
                    // get stroke width; SVG does not use 0.0 == hairline, so 0.0 is no line at all
                    const double fStrokeWidth(getStrokeWidth().isSet() ? getStrokeWidth().solve(mrOwner) : 1.0);

                    if(basegfx::fTools::more(fStrokeWidth, 0.0))
                    {
                        drawinglayer::primitive2d::Primitive2DReference aNewLinePrimitive;

                        // if we have a line with two identical points it is not really a line,
                        // but used by SVG sometimes to paint a single dot.In that case, create
                        // the geometry for a single dot
                        if(1 == rPath.count())
                        {
                            const basegfx::B2DPolygon aSingle(rPath.getB2DPolygon(0));

                            if(2 == aSingle.count() && aSingle.getB2DPoint(0).equal(aSingle.getB2DPoint(1)))
                            {
                                aNewLinePrimitive = new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                    basegfx::B2DPolyPolygon(
                                        basegfx::tools::createPolygonFromCircle(
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
                            // The default needs to be set explicitely, because svg default <> Draw default
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
                                const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(aDashArray);

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
                                    basegfx::tools::mergeToSinglePolyPolygon(
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

                        if(!aNewStroke.empty())
                        {
                            if(basegfx::fTools::less(fStrokeOpacity, 1.0))
                            {
                                // embed in UnifiedTransparencePrimitive2D
                                rTarget.push_back(
                                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                        aNewStroke,
                                        1.0 - fStrokeOpacity));
                            }
                            else
                            {
                                // append
                                rTarget.append(aNewStroke);
                            }
                        }
                    }
                }
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
                    double fTargetWidth(rMarker.getMarkerWidth().isSet() ? rMarker.getMarkerWidth().solve(mrOwner, xcoordinate) : 3.0);
                    double fTargetHeight(rMarker.getMarkerHeight().isSet() ? rMarker.getMarkerHeight().solve(mrOwner, xcoordinate) : 3.0);
                    const bool bStrokeWidth(SvgMarkerNode::strokeWidth == rMarker.getMarkerUnits());
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
                            rMarker.getRefX().isSet() ? rMarker.getRefX().solve(mrOwner, xcoordinate) : 0.0,
                            rMarker.getRefY().isSet() ? rMarker.getRefY().solve(mrOwner, ycoordinate) : 0.0);

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
            const basegfx::tools::PointIndexSet* pHelpPointIndices) const
        {
            // try to access linked markers
            const SvgMarkerNode* pStart = accessMarkerStartXLink();
            const SvgMarkerNode* pMid = accessMarkerMidXLink();
            const SvgMarkerNode* pEnd = accessMarkerEndXLink();

            if(pStart || pMid || pEnd)
            {
                const sal_uInt32 nSubPathCount(rPath.count());

                if(nSubPathCount)
                {
                    // remember prepared marker; pStart, pMid and pEnd may all be equal when
                    // only 'marker' was used instead of 'marker-start', 'marker-mid' or 'marker-end',
                    // see 'case SVGTokenMarker' in this file; thus in this case only one common
                    // marker in primitive form will be prepared
                    const SvgMarkerNode* pPrepared = nullptr;

                    // values for the prepared marker, results of prepare_singleMarker
                    drawinglayer::primitive2d::Primitive2DContainer aPreparedMarkerPrimitives;
                    basegfx::B2DHomMatrix aPreparedMarkerTransform;
                    basegfx::B2DRange aPreparedMarkerClipRange;

                    for (sal_uInt32 a(0); a < nSubPathCount; a++)
                    {
                        // iterate over sub-paths
                        const basegfx::B2DPolygon aSubPolygonPath(rPath.getB2DPolygon(a));
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
                                    const basegfx::tools::PointIndexSet::const_iterator aFound(
                                        pHelpPointIndices->find(basegfx::tools::PointIndex(a, b)));

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
                                    // and entering tangent for end marker. To achieve this (if wanted) it is possibe
                                    // to make the fetch of aEntering/aLeaving dependent on bIsFirstMarker/bIsLastMarker.
                                    // This is not done here, see comment 14 in task #1232379#
                                    // or http://www.w3.org/TR/SVG/painting.html#OrientAttribute
                                    basegfx::B2DVector aEntering(
                                        basegfx::tools::getTangentEnteringPoint(
                                            aSubPolygonPath,
                                            nPointIndex));
                                    basegfx::B2DVector aLeaving(
                                        basegfx::tools::getTangentLeavingPoint(
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
                                        aPreparedMarkerPrimitives));

                                if(!aPreparedMarkerClipRange.isEmpty())
                                {
                                    // marker needs to be clipped, it's bigger as the mapping
                                    basegfx::B2DPolyPolygon aClipPolygon(basegfx::tools::createPolygonFromRect(aPreparedMarkerClipRange));

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
            }
        }

        void SvgStyleAttributes::add_path(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::tools::PointIndexSet* pHelpPointIndices) const
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
                const bool bNeedToCheckClipRule(SVGTokenPath == mrOwner.getType() || SVGTokenPolygon == mrOwner.getType());
                const bool bClipPathIsNonzero(!bIsLine && bNeedToCheckClipRule && mbIsClipPathContent && FillRule_nonzero == maClipRule);
                const bool bFillRuleIsNonzero(!bIsLine && bNeedToCheckClipRule && !mbIsClipPathContent && FillRule_nonzero == getFillRule());

                if(bClipPathIsNonzero || bFillRuleIsNonzero)
                {
                    if(getFill() || getSvgGradientNodeFill() || getSvgPatternNodeFill()) {
                        // nonzero is wanted, solve geometrically (see description on basegfx)
                        // basegfx::tools::createNonzeroConform() is expensive for huge paths
                        // and is only needed if path will be filled later on
                        aPath = basegfx::tools::createNonzeroConform(aPath);
                    }
                }

                add_fill(aPath, rTarget, aGeoRange);
            }

            // create stroke
            add_stroke(rPath, rTarget, aGeoRange);

            // Svg supports markers for path, polygon, polyline and line
            if(SVGTokenPath == mrOwner.getType() ||         // path
                SVGTokenPolygon == mrOwner.getType() ||     // polygon, polyline
                SVGTokenLine == mrOwner.getType())          // line
            {
                // try to add markers
                add_markers(rPath, rTarget, pHelpPointIndices);
            }
        }

        void SvgStyleAttributes::add_postProcess(
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const drawinglayer::primitive2d::Primitive2DContainer& rSource,
            const basegfx::B2DHomMatrix* pTransform) const
        {
            if(!rSource.empty())
            {
                const double fOpacity(getOpacity().solve(mrOwner));

                if(basegfx::fTools::equalZero(fOpacity))
                {
                    return;
                }

                drawinglayer::primitive2d::Primitive2DContainer aSource(rSource);

                if(basegfx::fTools::less(fOpacity, 1.0))
                {
                    // embed in UnifiedTransparencePrimitive2D
                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                            aSource,
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
                            aSource));

                    aSource = drawinglayer::primitive2d::Primitive2DContainer { xRef };
                }

                const SvgClipPathNode* pClip = accessClipPathXLink();
                while(pClip)
                {
                    // #i124852# transform may be needed when userSpaceOnUse
                    pClip->apply(aSource, pTransform);
                    pClip = pClip->getSvgStyleAttributes()->accessClipPathXLink();
                }

                if(!aSource.empty()) // test again, applied clipPath may have lead to empty geometry
                {
                    const SvgMaskNode* pMask = accessMaskXLink();
                    if(pMask)
                    {
                        // #i124852# transform may be needed when userSpaceOnUse
                        pMask->apply(aSource, pTransform);
                    }

                    if(!aSource.empty()) // test again, applied mask may have lead to empty geometry
                    {
                        // append to current target
                        rTarget.append(aSource);
                    }
                }
            }
        }

        SvgStyleAttributes::SvgStyleAttributes(SvgNode& rOwner)
        :   mrOwner(rOwner),
            mpCssStyleParent(nullptr),
            maFill(),
            maStroke(),
            maStopColor(basegfx::BColor(0.0, 0.0, 0.0), true),
            maStrokeWidth(),
            maStopOpacity(),
            mpSvgGradientNodeFill(nullptr),
            mpSvgGradientNodeStroke(nullptr),
            mpSvgPatternNodeFill(nullptr),
            mpSvgPatternNodeStroke(nullptr),
            maFillOpacity(),
            maStrokeDasharray(),
            maStrokeDashOffset(),
            maStrokeLinecap(StrokeLinecap_notset),
            maStrokeLinejoin(StrokeLinejoin_notset),
            maStrokeMiterLimit(),
            maStrokeOpacity(),
            maFontFamily(),
            maFontSize(),
            maFontSizeNumber(),
            maFontStretch(FontStretch_notset),
            maFontStyle(FontStyle_notset),
            maFontVariant(FontVariant_notset),
            maFontWeight(FontWeight_notset),
            maTextAlign(TextAlign_notset),
            maTextDecoration(TextDecoration_notset),
            maTextAnchor(TextAnchor_notset),
            maColor(),
            maOpacity(),
            maVisibility(Visibility_notset),
            maTitle(),
            maDesc(),
            maClipPathXLink(),
            mpClipPathXLink(nullptr),
            maMaskXLink(),
            mpMaskXLink(nullptr),
            maMarkerStartXLink(),
            mpMarkerStartXLink(nullptr),
            maMarkerMidXLink(),
            mpMarkerMidXLink(nullptr),
            maMarkerEndXLink(),
            mpMarkerEndXLink(nullptr),
            maFillRule(FillRule_notset),
            maClipRule(FillRule_nonzero),
            maBaselineShift(BaselineShift_Baseline),
            maBaselineShiftNumber(0),
            mbIsClipPathContent(SVGTokenClipPathNode == mrOwner.getType()),
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
            const OUString& /* rTokenName */,
            SVGToken aSVGToken,
            const OUString& aContent,
            bool bCaseIndependent)
        {
            switch(aSVGToken)
            {
                case SVGTokenFill:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        setFill(aSvgPaint);
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(basegfx::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    else if(!aURL.isEmpty())
                    {
                        const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(aURL);

                        if(pNode)
                        {
                            if(SVGTokenLinearGradient == pNode->getType() || SVGTokenRadialGradient == pNode->getType())
                            {
                                setSvgGradientNodeFill(static_cast< const SvgGradientNode* >(pNode));
                            }
                            else if(SVGTokenPattern == pNode->getType())
                            {
                                setSvgPatternNodeFill(static_cast< const SvgPatternNode* >(pNode));
                            }
                        }
                    }
                    break;
                }
                case SVGTokenFillOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setFillOpacity(SvgNumber(basegfx::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet()));
                    }
                    break;
                }
                case SVGTokenFillRule:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrNonzero))
                        {
                            maFillRule = FillRule_nonzero;
                        }
                        else if(aContent.match(commonStrings::aStrEvenOdd))
                        {
                            maFillRule = FillRule_evenodd;
                        }
                    }
                    break;
                }
                case SVGTokenStroke:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        setStroke(aSvgPaint);
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(basegfx::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    else if(!aURL.isEmpty())
                    {
                        const SvgNode* pNode = mrOwner.getDocument().findSvgNodeById(aURL);

                        if(pNode)
                        {
                            if(SVGTokenLinearGradient == pNode->getType() || SVGTokenRadialGradient  == pNode->getType())
                            {
                                setSvgGradientNodeStroke(static_cast< const SvgGradientNode* >(pNode));
                            }
                            else if(SVGTokenPattern == pNode->getType())
                            {
                                setSvgPatternNodeStroke(static_cast< const SvgPatternNode* >(pNode));
                            }
                        }
                    }
                    break;
                }
                case SVGTokenStrokeDasharray:
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
                            setStrokeDasharraySet(true);
                        }
                        else if(readSvgNumberVector(aContent, aVector))
                        {
                            setStrokeDasharray(aVector);
                        }
                    }
                    break;
                }
                case SVGTokenStrokeDashoffset:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setStrokeDashOffset(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenStrokeLinecap:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("butt"))
                        {
                            setStrokeLinecap(StrokeLinecap_butt);
                        }
                        else if(aContent.startsWith("round"))
                        {
                            setStrokeLinecap(StrokeLinecap_round);
                        }
                        else if(aContent.startsWith("square"))
                        {
                            setStrokeLinecap(StrokeLinecap_square);
                        }
                    }
                    break;
                }
                case SVGTokenStrokeLinejoin:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("miter"))
                        {
                            setStrokeLinejoin(StrokeLinejoin_miter);
                        }
                        else if(aContent.startsWith("round"))
                        {
                            setStrokeLinejoin(StrokeLinejoin_round);
                        }
                        else if(aContent.startsWith("bevel"))
                        {
                            setStrokeLinejoin(StrokeLinejoin_bevel);
                        }
                    }
                    break;
                }
                case SVGTokenStrokeMiterlimit:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(basegfx::fTools::moreOrEqual(aNum.getNumber(), 1.0))
                        { //readSingleNumber sets Unit_px as default, if unit is missing. Correct it here.
                            setStrokeMiterLimit(SvgNumber(aNum.getNumber(), Unit_none));
                        }
                    }
                    break;
                }
                case SVGTokenStrokeOpacity:
                {

                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setStrokeOpacity(SvgNumber(basegfx::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet()));
                    }
                    break;
                }
                case SVGTokenStrokeWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setStrokeWidth(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenStopColor:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        setStopColor(aSvgPaint);
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(basegfx::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    break;
                }
                case SVGTokenStopOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setStopOpacity(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenFont:
                {
                    break;
                }
                case SVGTokenFontFamily:
                {
                    SvgStringVector aSvgStringVector;

                    if(readSvgStringVector(aContent, aSvgStringVector))
                    {
                        setFontFamily(aSvgStringVector);
                    }
                    break;
                }
                case SVGTokenFontSize:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("xx-small"))
                        {
                            setFontSize(FontSize_xx_small);
                        }
                        else if(aContent.startsWith("x-small"))
                        {
                            setFontSize(FontSize_x_small);
                        }
                        else if(aContent.startsWith("small"))
                        {
                            setFontSize(FontSize_small);
                        }
                        else if(aContent.startsWith("smaller"))
                        {
                            setFontSize(FontSize_smaller);
                        }
                        else if(aContent.startsWith("medium"))
                        {
                            setFontSize(FontSize_medium);
                        }
                        else if(aContent.startsWith("larger"))
                        {
                            setFontSize(FontSize_larger);
                        }
                        else if(aContent.startsWith("large"))
                        {
                            setFontSize(FontSize_large);
                        }
                        else if(aContent.startsWith("x-large"))
                        {
                            setFontSize(FontSize_x_large);
                        }
                        else if(aContent.startsWith("xx-large"))
                        {
                            setFontSize(FontSize_xx_large);
                        }
                        else if(aContent.startsWith("initial"))
                        {
                            setFontSize(FontSize_initial);
                        }
                        else
                        {
                            SvgNumber aNum;

                            if(readSingleNumber(aContent, aNum))
                            {
                                setFontSizeNumber(aNum);
                            }
                        }
                    }
                    break;
                }
                case SVGTokenFontSizeAdjust:
                {
                    break;
                }
                case SVGTokenFontStretch:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("normal"))
                        {
                            setFontStretch(FontStretch_normal);
                        }
                        else if(aContent.startsWith("wider"))
                        {
                            setFontStretch(FontStretch_wider);
                        }
                        else if(aContent.startsWith("narrower"))
                        {
                            setFontStretch(FontStretch_narrower);
                        }
                        else if(aContent.startsWith("ultra-condensed"))
                        {
                            setFontStretch(FontStretch_ultra_condensed);
                        }
                        else if(aContent.startsWith("extra-condensed"))
                        {
                            setFontStretch(FontStretch_extra_condensed);
                        }
                        else if(aContent.startsWith("condensed"))
                        {
                            setFontStretch(FontStretch_condensed);
                        }
                        else if(aContent.startsWith("semi-condensed"))
                        {
                            setFontStretch(FontStretch_semi_condensed);
                        }
                        else if(aContent.startsWith("semi-expanded"))
                        {
                            setFontStretch(FontStretch_semi_expanded);
                        }
                        else if(aContent.startsWith("expanded"))
                        {
                            setFontStretch(FontStretch_expanded);
                        }
                        else if(aContent.startsWith("extra-expanded"))
                        {
                            setFontStretch(FontStretch_extra_expanded);
                        }
                        else if(aContent.startsWith("ultra-expanded"))
                        {
                            setFontStretch(FontStretch_ultra_expanded);
                        }
                    }
                    break;
                }
                case SVGTokenFontStyle:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("normal"))
                        {
                            setFontStyle(FontStyle_normal);
                        }
                        else if(aContent.startsWith("italic"))
                        {
                            setFontStyle(FontStyle_italic);
                        }
                        else if(aContent.startsWith("oblique"))
                        {
                            setFontStyle(FontStyle_oblique);
                        }
                    }
                    break;
                }
                case SVGTokenFontVariant:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("normal"))
                        {
                            setFontVariant(FontVariant_normal);
                        }
                        else if(aContent.startsWith("small-caps"))
                        {
                            setFontVariant(FontVariant_small_caps);
                        }
                    }
                    break;
                }
                case SVGTokenFontWeight:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("100"))
                        {
                            setFontWeight(FontWeight_100);
                        }
                        else if(aContent.startsWith("200"))
                        {
                            setFontWeight(FontWeight_200);
                        }
                        else if(aContent.startsWith("300"))
                        {
                            setFontWeight(FontWeight_300);
                        }
                        else if(aContent.startsWith("400") || aContent.startsWith("normal"))
                        {
                            setFontWeight(FontWeight_400);
                        }
                        else if(aContent.startsWith("500"))
                        {
                            setFontWeight(FontWeight_500);
                        }
                        else if(aContent.startsWith("600"))
                        {
                            setFontWeight(FontWeight_600);
                        }
                        else if(aContent.startsWith("700") || aContent.startsWith("bold"))
                        {
                            setFontWeight(FontWeight_700);
                        }
                        else if(aContent.startsWith("800"))
                        {
                            setFontWeight(FontWeight_800);
                        }
                        else if(aContent.startsWith("900"))
                        {
                            setFontWeight(FontWeight_900);
                        }
                        else if(aContent.startsWith("bolder"))
                        {
                            setFontWeight(FontWeight_bolder);
                        }
                        else if(aContent.startsWith("lighter"))
                        {
                            setFontWeight(FontWeight_lighter);
                        }
                    }
                    break;
                }
                case SVGTokenDirection:
                {
                    break;
                }
                case SVGTokenLetterSpacing:
                {
                    break;
                }
                case SVGTokenTextDecoration:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("none"))
                        {
                            setTextDecoration(TextDecoration_none);
                        }
                        else if(aContent.startsWith("underline"))
                        {
                            setTextDecoration(TextDecoration_underline);
                        }
                        else if(aContent.startsWith("overline"))
                        {
                            setTextDecoration(TextDecoration_overline);
                        }
                        else if(aContent.startsWith("line-through"))
                        {
                            setTextDecoration(TextDecoration_line_through);
                        }
                        else if(aContent.startsWith("blink"))
                        {
                            setTextDecoration(TextDecoration_blink);
                        }
                    }
                    break;
                }
                case SVGTokenUnicodeBidi:
                {
                    break;
                }
                case SVGTokenWordSpacing:
                {
                    break;
                }
                case SVGTokenTextAnchor:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("start"))
                        {
                            setTextAnchor(TextAnchor_start);
                        }
                        else if(aContent.startsWith("middle"))
                        {
                            setTextAnchor(TextAnchor_middle);
                        }
                        else if(aContent.startsWith("end"))
                        {
                            setTextAnchor(TextAnchor_end);
                        }
                    }
                    break;
                }
                case SVGTokenTextAlign:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("left"))
                        {
                            setTextAlign(TextAlign_left);
                        }
                        else if(aContent.startsWith("right"))
                        {
                            setTextAlign(TextAlign_right);
                        }
                        else if(aContent.startsWith("center"))
                        {
                            setTextAlign(TextAlign_center);
                        }
                        else if(aContent.startsWith("justify"))
                        {
                            setTextAlign(TextAlign_justify);
                        }
                    }
                    break;
                }
                case SVGTokenColor:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;
                    SvgNumber aOpacity;

                    if(readSvgPaint(aContent, aSvgPaint, aURL, bCaseIndependent, aOpacity))
                    {
                        setColor(aSvgPaint);
                        if(aOpacity.isSet())
                        {
                            setOpacity(SvgNumber(basegfx::clamp(aOpacity.getNumber(), 0.0, 1.0)));
                        }
                    }
                    break;
                }
                case SVGTokenOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setOpacity(SvgNumber(basegfx::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(), aNum.isSet()));
                    }
                    break;
                }
                case SVGTokenVisibility:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("visible"))
                        {
                            setVisibility(Visibility_visible);
                        }
                        else if(aContent.startsWith("hidden"))
                        {
                            setVisibility(Visibility_hidden);
                        }
                        else if(aContent.startsWith("collapse"))
                        {
                            setVisibility(Visibility_collapse);
                        }
                        else if(aContent.startsWith("inherit"))
                        {
                            setVisibility(Visibility_inherit);
                        }
                    }
                    break;
                }
                case SVGTokenTitle:
                {
                    setTitle(aContent);
                    break;
                }
                case SVGTokenDesc:
                {
                    setDesc(aContent);
                    break;
                }
                case SVGTokenClipPathProperty:
                {
                    readLocalUrl(aContent, maClipPathXLink);
                    break;
                }
                case SVGTokenMask:
                {
                    readLocalUrl(aContent, maMaskXLink);
                    break;
                }
                case SVGTokenClipRule:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrNonzero))
                        {
                            maClipRule = FillRule_nonzero;
                        }
                        else if(aContent.match(commonStrings::aStrEvenOdd))
                        {
                            maClipRule = FillRule_evenodd;
                        }
                    }
                    break;
                }
                case SVGTokenMarker:
                {
                    if(bCaseIndependent)
                    {
                        readLocalUrl(aContent, maMarkerEndXLink);
                        maMarkerStartXLink = maMarkerMidXLink = maMarkerEndXLink;
                    }
                    break;
                }
                case SVGTokenMarkerStart:
                {
                    readLocalUrl(aContent, maMarkerStartXLink);
                    break;
                }
                case SVGTokenMarkerMid:
                {
                    readLocalUrl(aContent, maMarkerMidXLink);
                    break;
                }
                case SVGTokenMarkerEnd:
                {
                    readLocalUrl(aContent, maMarkerEndXLink);
                    break;
                }
                case SVGTokenDisplay:
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
                case SVGTokenBaselineShift:
                {
                    if(!aContent.isEmpty())
                    {
                        SvgNumber aNum;

                        if(aContent.startsWith("sub"))
                        {
                            setBaselineShift(BaselineShift_Sub);
                        }
                        else if(aContent.startsWith("super"))
                        {
                            setBaselineShift(BaselineShift_Super);
                        }
                        else if(readSingleNumber(aContent, aNum))
                        {
                            setBaselineShiftNumber(aNum);

                            if(Unit_percent == aNum.getUnit())
                            {
                                setBaselineShift(BaselineShift_Percentage);
                            }
                            else
                            {
                                setBaselineShift(BaselineShift_Length);
                            }
                        }
                        else
                        {
                            // no BaselineShift or inherit (which is automatically)
                            setBaselineShift();
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
            }
            else if (!mpSvgGradientNodeFill && !mpSvgPatternNodeFill)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getFill();
                }
            }
            else if(mbIsClipPathContent)
            {
                static basegfx::BColor aBlack(0.0, 0.0, 0.0);
                return &aBlack;
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
            else if (!mpSvgGradientNodeStroke && !mpSvgPatternNodeStroke)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getStroke();
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
            if(mpSvgGradientNodeFill)
            {
                return mpSvgGradientNodeFill;
            }
            else if (!maFill.isSet() && !mpSvgPatternNodeFill)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgGradientNodeFill();
                }
            }

            return nullptr;
        }

        const SvgGradientNode* SvgStyleAttributes::getSvgGradientNodeStroke() const
        {
            if(mpSvgGradientNodeStroke)
            {
                return mpSvgGradientNodeStroke;
            }
            else if (!maStroke.isSet() && !mpSvgPatternNodeStroke)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgGradientNodeStroke();
                }
            }

            return nullptr;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeFill() const
        {
            if(mpSvgPatternNodeFill)
            {
                return mpSvgPatternNodeFill;
            }
            else if (!maFill.isSet() && !mpSvgGradientNodeFill)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgPatternNodeFill();
                }
            }

            return nullptr;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeStroke() const
        {
            if(mpSvgPatternNodeStroke)
            {
                return mpSvgPatternNodeStroke;
            }
            else if (!maStroke.isSet() && !mpSvgGradientNodeStroke)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgPatternNodeStroke();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeWidth();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFillOpacity();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getOpacity();
            }

            // default is 1
            return SvgNumber(1.0);
        }

        Visibility SvgStyleAttributes::getVisibility() const
        {
            if(Visibility_notset == maVisibility || Visibility_inherit == maVisibility)
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getVisibility();
                }
                //default is Visible
                return Visibility_visible;
            }

            return maVisibility;
        }

        FillRule SvgStyleAttributes::getFillRule() const
        {
            if(FillRule_notset != maFillRule)
            {
                return maFillRule;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFillRule();
            }

            // default is NonZero
            return FillRule_nonzero;
        }

        const SvgNumberVector& SvgStyleAttributes::getStrokeDasharray() const
        {
            if(!maStrokeDasharray.empty())
            {
                return maStrokeDasharray;
            }
            else if(getStrokeDasharraySet())
            {
                // #121221# is set to empty *by purpose*, do not visit parent styles
                return maStrokeDasharray;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeDasharray();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeDashOffset();
            }

            // default is 0
            return SvgNumber(0.0);
        }

        StrokeLinecap SvgStyleAttributes::getStrokeLinecap() const
        {
            if(maStrokeLinecap != StrokeLinecap_notset)
            {
                return maStrokeLinecap;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeLinecap();
            }

            // default is StrokeLinecap_butt
            return StrokeLinecap_butt;
        }

        StrokeLinejoin SvgStyleAttributes::getStrokeLinejoin() const
        {
            if(maStrokeLinejoin != StrokeLinejoin_notset)
            {
                return maStrokeLinejoin;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeLinejoin();
            }

            // default is StrokeLinejoin_butt
            return StrokeLinejoin_miter;
        }

        SvgNumber SvgStyleAttributes::getStrokeMiterLimit() const
        {
            if(maStrokeMiterLimit.isSet())
            {
                return maStrokeMiterLimit;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeMiterLimit();
            }

            // default is 4
            return SvgNumber(4.0, Unit_none);
        }

        SvgNumber SvgStyleAttributes::getStrokeOpacity() const
        {
            if(maStrokeOpacity.isSet())
            {
                return maStrokeOpacity;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeOpacity();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFontFamily();
            }

            // default is empty
            return maFontFamily;
        }

        SvgNumber SvgStyleAttributes::getFontSizeNumber() const
        {
            // default size is 'medium' or 16px, which is equal to the default PPI used in svgio ( 96.0 )
            // converted to pixels
            const double aDefaultSize = F_SVG_PIXEL_PER_INCH / 6.0;

            if(maFontSizeNumber.isSet())
            {
                if(!maFontSizeNumber.isPositive())
                    return aDefaultSize;

                // #122524# Handle Unit_percent realtive to parent FontSize (see SVG1.1
                // spec 10.10 Font selection properties \91font-size\92, lastline (click 'normative
                // definition of the property')
                if(Unit_percent == maFontSizeNumber.getUnit())
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
                        maFontSizeNumber.getNumber() * aDefaultSize / 100.0, Unit_px, true);
                }
                else if((Unit_em == maFontSizeNumber.getUnit()) || (Unit_ex == maFontSizeNumber.getUnit()))
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
                case FontSize_notset:
                    break;
                case FontSize_xx_small:
                {
                    return SvgNumber(aDefaultSize / 1.728);
                }
                case FontSize_x_small:
                {
                    return SvgNumber(aDefaultSize / 1.44);
                }
                case FontSize_small:
                {
                    return SvgNumber(aDefaultSize / 1.2);
                }
                case FontSize_smaller:
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();
                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();
                        return SvgNumber(aParentNumber.getNumber() / 1.2, aParentNumber.getUnit());
                    }
                    SAL_FALLTHROUGH;
                }
                case FontSize_medium:
                case FontSize_initial:
                {
                    return SvgNumber(aDefaultSize);
                }
                case FontSize_large:
                {
                    return SvgNumber(aDefaultSize * 1.2);
                }
                case FontSize_larger:
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();
                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSizeNumber();
                        return SvgNumber(aParentNumber.getNumber() * 1.2, aParentNumber.getUnit());
                    }
                    SAL_FALLTHROUGH;
                }
                case FontSize_x_large:
                {
                    return SvgNumber(aDefaultSize * 1.44);
                }
                case FontSize_xx_large:
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
            if(maFontStretch != FontStretch_notset)
            {
                if(FontStretch_wider != maFontStretch && FontStretch_narrower != maFontStretch)
                {
                    return maFontStretch;
                }
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                FontStretch aInherited = pSvgStyleAttributes->getFontStretch();

                if(FontStretch_wider == maFontStretch)
                {
                    aInherited = getWider(aInherited);
                }
                else if(FontStretch_narrower == maFontStretch)
                {
                    aInherited = getNarrower(aInherited);
                }

                return aInherited;
            }

            // default is FontStretch_normal
            return FontStretch_normal;
        }

        FontStyle SvgStyleAttributes::getFontStyle() const
        {
            if(maFontStyle != FontStyle_notset)
            {
                return maFontStyle;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFontStyle();
            }

            // default is FontStyle_normal
            return FontStyle_normal;
        }

        FontWeight SvgStyleAttributes::getFontWeight() const
        {
            if(maFontWeight != FontWeight_notset)
            {
                if(FontWeight_bolder != maFontWeight && FontWeight_lighter != maFontWeight)
                {
                    return maFontWeight;
                }
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                FontWeight aInherited = pSvgStyleAttributes->getFontWeight();

                if(FontWeight_bolder == maFontWeight)
                {
                    aInherited = getBolder(aInherited);
                }
                else if(FontWeight_lighter == maFontWeight)
                {
                    aInherited = getLighter(aInherited);
                }

                return aInherited;
            }

            // default is FontWeight_400 (FontWeight_normal)
            return FontWeight_400;
        }

        TextAlign SvgStyleAttributes::getTextAlign() const
        {
            if(maTextAlign != TextAlign_notset)
            {
                return maTextAlign;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getTextAlign();
            }

            // default is TextAlign_left
            return TextAlign_left;
        }

        const SvgStyleAttributes* SvgStyleAttributes::getTextDecorationDefiningSvgStyleAttributes() const
        {
            if(maTextDecoration != TextDecoration_notset)
            {
                return this;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getTextDecorationDefiningSvgStyleAttributes();
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
                // default is TextDecoration_none
                return TextDecoration_none;
            }
        }

        TextAnchor SvgStyleAttributes::getTextAnchor() const
        {
            if(maTextAnchor != TextAnchor_notset)
            {
                return maTextAnchor;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getTextAnchor();
            }

            // default is TextAnchor_start
            return TextAnchor_start;
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

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getColor();
                }
            }

            return nullptr;
        }

        OUString SvgStyleAttributes::getClipPathXLink() const
        {
            if(mbIsClipPathContent)
            {
                return maClipPathXLink;
            }

            if(!maClipPathXLink.isEmpty())
            {
                return maClipPathXLink;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes && !pSvgStyleAttributes->maClipPathXLink.isEmpty())
            {
                return pSvgStyleAttributes->getClipPathXLink();
            }

            return OUString();
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

            if(pSvgStyleAttributes && !pSvgStyleAttributes->maMaskXLink.isEmpty())
            {
                return pSvgStyleAttributes->getMaskXLink();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getMarkerStartXLink();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getMarkerMidXLink();
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

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getMarkerEndXLink();
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
            // #122524# Handle Unit_percent realtive to parent BaselineShift
            if(Unit_percent == maBaselineShiftNumber.getUnit())
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    const SvgNumber aParentNumber = pSvgStyleAttributes->getBaselineShiftNumber();

                    return SvgNumber(
                        aParentNumber.getNumber() * maBaselineShiftNumber.getNumber() * 0.01,
                        aParentNumber.getUnit(),
                        true);
                }
            }

            return maBaselineShiftNumber;
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

