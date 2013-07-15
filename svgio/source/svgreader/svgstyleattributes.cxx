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

#include <svgio/svgreader/svgstyleattributes.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgdocument.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <svgio/svgreader/svggradientnode.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svgio/svgreader/svgclippathnode.hxx>
#include <svgio/svgreader/svgmasknode.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svgio/svgreader/svgmarkernode.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <svgio/svgreader/svgpatternnode.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        basegfx::B2DLineJoin StrokeLinejoinToB2DLineJoin(StrokeLinejoin aStrokeLinejoin)
        {
            if(StrokeLinejoin_round == aStrokeLinejoin)
            {
                return basegfx::B2DLINEJOIN_ROUND;
            }
            else if(StrokeLinejoin_bevel == aStrokeLinejoin)
            {
                return basegfx::B2DLINEJOIN_BEVEL;
            }

            return basegfx::B2DLINEJOIN_MITER;
        }

        com::sun::star::drawing::LineCap StrokeLinecapToDrawingLineCap(StrokeLinecap aStrokeLinecap)
        {
            switch(aStrokeLinecap)
            {
                default: /* StrokeLinecap_notset, StrokeLinecap_butt */
                {
                    return com::sun::star::drawing::LineCap_BUTT;
                }
                case StrokeLinecap_round:
                {
                    return com::sun::star::drawing::LineCap_ROUND;
                }
                case StrokeLinecap_square:
                {
                    return com::sun::star::drawing::LineCap_SQUARE;
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

        void SvgStyleAttributes::readStyle(const OUString& rCandidate)
        {
            const sal_Int32 nLen(rCandidate.getLength());
            sal_Int32 nPos(0);

            while(nPos < nLen)
            {
                const sal_Int32 nInitPos(nPos);
                skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);
                OUStringBuffer aTokenName;
                copyString(rCandidate, nPos, aTokenName, nLen);

                if(!aTokenName.isEmpty())
                {
                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(':'), nPos, nLen);
                    OUStringBuffer aTokenValue;
                    copyToLimiter(rCandidate, sal_Unicode(';'), nPos, aTokenValue, nLen);
                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(';'), nPos, nLen);
                    const OUString aOUTokenName(aTokenName.makeStringAndClear());
                    const OUString aOUTokenValue(aTokenValue.makeStringAndClear());

                    parseStyleAttribute(aOUTokenName, StrToSVGToken(aOUTokenName), aOUTokenValue);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }
        }

        const SvgStyleAttributes* SvgStyleAttributes::getParentStyle() const
        {
            if(getCssStyleParent())
            {
                return getCssStyleParent();
            }

            if(mrOwner.getParent())
            {
                return mrOwner.getParent()->getSvgStyleAttributes();
            }

            return 0;
        }

        void SvgStyleAttributes::add_text(
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            drawinglayer::primitive2d::Primitive2DSequence& rSource) const
        {
            if(rSource.hasElements())
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

                    // proccess
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
                // normally not really match optically due to divrese system text
                // renderers
                if(aMergedArea.count() && (pFillGradient || pFillPattern || bStrokeUsed))
                {
                    // create text fill content based on geometry
                    add_fill(aMergedArea, rTarget, aMergedArea.getB2DRange());
                }
                else if(pFill)
                {
                    // add the already prepared primitives for single color fill
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, rSource);
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
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
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

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                        rTarget,
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
                        fRadius = rFillGradient.getR().solve(mrOwner, length);

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

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                        rTarget,
                        new drawinglayer::primitive2d::SvgRadialGradientPrimitive2D(
                            aGradientTransform,
                            rPath,
                            aSvgGradientEntryVector,
                            aStart,
                            fRadius,
                            userSpaceOnUse != rFillGradient.getGradientUnits(),
                            rFillGradient.getSpreadMethod(),
                            bFocal ? &aFocal : 0));
                }
            }
        }

        void SvgStyleAttributes::add_fillPatternTransform(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
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
                drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                aInv.invert();
                aPath.transform(aInv);
                add_fillPattern(aPath, aNewTarget, rFillPattern, aPath.getB2DRange());

                if(aNewTarget.hasElements())
                {
                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                        rTarget,
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
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const SvgPatternNode& rFillPattern,
            const basegfx::B2DRange& rGeoRange) const
        {
            // fill polyPolygon with given pattern
            const drawinglayer::primitive2d::Primitive2DSequence& rPrimitives = rFillPattern.getPatternPrimitives();

            if(rPrimitives.hasElements())
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
                                aMapPrimitivesToUnitRange = rRatio.createLinearMapping(aUnitRange, *pViewBox);
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
                        drawinglayer::primitive2d::Primitive2DSequence aPrimitives(rPrimitives);

                        if(!aMapPrimitivesToUnitRange.isIdentity())
                        {
                            const drawinglayer::primitive2d::Primitive2DReference xRef(
                                new drawinglayer::primitive2d::TransformPrimitive2D(
                                    aMapPrimitivesToUnitRange,
                                    aPrimitives));

                            aPrimitives = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                        }

                        // embed in PatternFillPrimitive2D
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                            rTarget,
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
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const basegfx::B2DRange& rGeoRange) const
        {
            const basegfx::BColor* pFill = getFill();
            const SvgGradientNode* pFillGradient = getSvgGradientNodeFill();
            const SvgPatternNode* pFillPattern = getSvgPatternNodeFill();

            if(pFill || pFillGradient || pFillPattern)
            {
                const double fFillOpacity(getFillOpacity().solve(mrOwner, length));

                if(basegfx::fTools::more(fFillOpacity, 0.0))
                {
                    drawinglayer::primitive2d::Primitive2DSequence aNewFill;

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
                        aNewFill.realloc(1);
                        aNewFill[0] = new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                            rPath,
                            *pFill);
                    }

                    if(aNewFill.hasElements())
                    {
                        if(basegfx::fTools::less(fFillOpacity, 1.0))
                        {
                            // embed in UnifiedTransparencePrimitive2D
                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                                rTarget,
                                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                    aNewFill,
                                    1.0 - fFillOpacity));
                        }
                        else
                        {
                            // append
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewFill);
                        }
                    }
                }
            }
        }

        void SvgStyleAttributes::add_stroke(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const basegfx::B2DRange& rGeoRange) const
        {
            const basegfx::BColor* pStroke = getStroke();
            const SvgGradientNode* pStrokeGradient = getSvgGradientNodeStroke();
            const SvgPatternNode* pStrokePattern = getSvgPatternNodeStroke();

            if(pStroke || pStrokeGradient || pStrokePattern)
            {
                drawinglayer::primitive2d::Primitive2DSequence aNewStroke;
                const double fStrokeOpacity(getStrokeOpacity().solve(mrOwner, length));

                if(basegfx::fTools::more(fStrokeOpacity, 0.0))
                {
                    // get stroke width; SVG does not use 0.0 == hairline, so 0.0 is no line at all
                    const double fStrokeWidth(getStrokeWidth().isSet() ? getStrokeWidth().solve(mrOwner, length) : 1.0);

                    if(basegfx::fTools::more(fStrokeWidth, 0.0))
                    {
                        // get LineJoin, LineCap and stroke array
                        const basegfx::B2DLineJoin aB2DLineJoin(StrokeLinejoinToB2DLineJoin(getStrokeLinejoin()));
                        const com::sun::star::drawing::LineCap aLineCap(StrokeLinecapToDrawingLineCap(getStrokeLinecap()));
                        ::std::vector< double > aDashArray;

                        if(!getStrokeDasharray().empty())
                        {
                            aDashArray = solveSvgNumberVector(getStrokeDasharray(), mrOwner, length);
                        }

                        // todo: Handle getStrokeDashOffset()

                        // prepare line attribute
                        drawinglayer::primitive2d::Primitive2DReference aNewLinePrimitive;
                        const drawinglayer::attribute::LineAttribute aLineAttribute(
                            pStroke ? *pStroke : basegfx::BColor(0.0, 0.0, 0.0),
                            fStrokeWidth,
                            aB2DLineJoin,
                            aLineCap);

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

                        if(pStrokeGradient || pStrokePattern)
                        {
                            // put primitive into Primitive2DReference and Primitive2DSequence
                            const drawinglayer::primitive2d::Primitive2DSequence aSeq(&aNewLinePrimitive, 1);

                            // use neutral ViewInformation and create LineGeometryExtractor2D
                            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                            drawinglayer::processor2d::LineGeometryExtractor2D aExtractor(aViewInformation2D);

                            // proccess
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
                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aNewStroke, aNewLinePrimitive);
                        }

                        if(aNewStroke.hasElements())
                        {
                            if(basegfx::fTools::less(fStrokeOpacity, 1.0))
                            {
                                // embed in UnifiedTransparencePrimitive2D
                                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                                    rTarget,
                                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                        aNewStroke,
                                        1.0 - fStrokeOpacity));
                            }
                            else
                            {
                                // append
                                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewStroke);
                            }
                        }
                    }
                }
            }
        }

        double get_markerRotation(
            const SvgMarkerNode& rMarker,
            const basegfx::B2DPolygon& rPolygon,
            const sal_uInt32 nIndex)
        {
            double fAngle(0.0);
            const sal_uInt32 nPointCount(rPolygon.count());

            if(nPointCount)
            {
                if(rMarker.getOrientAuto())
                {
                    const bool bPrev(rPolygon.isClosed() || nIndex > 0);
                    basegfx::B2DCubicBezier aSegment;
                    basegfx::B2DVector aPrev;
                    basegfx::B2DVector aNext;

                    if(bPrev)
                    {
                        rPolygon.getBezierSegment((nIndex - 1) % nPointCount, aSegment);
                        aPrev = aSegment.getTangent(1.0);
                    }

                    const bool bNext(rPolygon.isClosed() || nIndex + 1 < nPointCount);

                    if(bNext)
                    {
                        rPolygon.getBezierSegment(nIndex % nPointCount, aSegment);
                        aNext = aSegment.getTangent(0.0);
                    }

                    if(bPrev && bNext)
                    {
                        fAngle = atan2(aPrev.getY() + aNext.getY(), aPrev.getX() + aNext.getX());
                    }
                    else if(bPrev)
                    {
                        fAngle = atan2(aPrev.getY(), aPrev.getX());
                    }
                    else if(bNext)
                    {
                        fAngle = atan2(aNext.getY(), aNext.getX());
                    }
                }
                else
                {
                    fAngle = rMarker.getAngle();
                }
            }

            return fAngle;
        }

        bool SvgStyleAttributes::prepare_singleMarker(
            drawinglayer::primitive2d::Primitive2DSequence& rMarkerPrimitives,
            basegfx::B2DHomMatrix& rMarkerTransform,
            basegfx::B2DRange& rClipRange,
            const SvgMarkerNode& rMarker) const
        {
            // reset return values
            rMarkerTransform.identity();
            rClipRange.reset();

            // get marker primitive representation
            rMarkerPrimitives = rMarker.getMarkerPrimitives();

            if(rMarkerPrimitives.hasElements())
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
                    const double fStrokeWidth(getStrokeWidth().isSet() ? getStrokeWidth().solve(mrOwner, length) : 1.0);

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
                                rMarkerTransform = rRatio.createLinearMapping(aTargetRange, aPrimitiveRange);
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

        void SvgStyleAttributes::add_singleMarker(
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const drawinglayer::primitive2d::Primitive2DSequence& rMarkerPrimitives,
            const basegfx::B2DHomMatrix& rMarkerTransform,
            const basegfx::B2DRange& rClipRange,
            const SvgMarkerNode& rMarker,
            const basegfx::B2DPolygon& rCandidate,
            const sal_uInt32 nIndex) const
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(nPointCount)
            {
                // get and apply rotation
                basegfx::B2DHomMatrix aCombinedTransform(rMarkerTransform);
                aCombinedTransform.rotate(get_markerRotation(rMarker, rCandidate, nIndex));

                // get and apply target position
                const basegfx::B2DPoint aPoint(rCandidate.getB2DPoint(nIndex % nPointCount));
                aCombinedTransform.translate(aPoint.getX(), aPoint.getY());

                // prepare marker
                drawinglayer::primitive2d::Primitive2DReference xMarker(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aCombinedTransform,
                        rMarkerPrimitives));

                if(!rClipRange.isEmpty())
                {
                    // marker needs to be clipped, it's bigger as the mapping
                    basegfx::B2DPolyPolygon aClipPolygon(basegfx::tools::createPolygonFromRect(rClipRange));

                    aClipPolygon.transform(aCombinedTransform);
                    xMarker = new drawinglayer::primitive2d::MaskPrimitive2D(
                        aClipPolygon,
                        drawinglayer::primitive2d::Primitive2DSequence(&xMarker, 1));
                }

                // add marker
                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xMarker);
            }
        }

        void SvgStyleAttributes::add_markers(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DSequence& rTarget) const
        {
            // try to access linked markers
            const SvgMarkerNode* pStart = accessMarkerStartXLink();
            const SvgMarkerNode* pMid = accessMarkerMidXLink();
            const SvgMarkerNode* pEnd = accessMarkerEndXLink();

            if(pStart || pMid || pEnd)
            {
                const sal_uInt32 nCount(rPath.count());

                for (sal_uInt32 a(0); a < nCount; a++)
                {
                    const basegfx::B2DPolygon aCandidate(rPath.getB2DPolygon(a));
                    const sal_uInt32 nPointCount(aCandidate.count());

                    if(nPointCount)
                    {
                        const sal_uInt32 nMarkerCount(aCandidate.isClosed() ? nPointCount + 1 : nPointCount);
                        drawinglayer::primitive2d::Primitive2DSequence aMarkerPrimitives;
                        basegfx::B2DHomMatrix aMarkerTransform;
                        basegfx::B2DRange aClipRange;
                        const SvgMarkerNode* pPrepared = 0;

                        if(pStart)
                        {
                            if(prepare_singleMarker(aMarkerPrimitives, aMarkerTransform, aClipRange, *pStart))
                            {
                                pPrepared = pStart;
                                add_singleMarker(rTarget, aMarkerPrimitives, aMarkerTransform, aClipRange, *pPrepared, aCandidate, 0);
                            }
                        }

                        if(pMid && nMarkerCount > 2)
                        {
                            if(pMid == pPrepared || prepare_singleMarker(aMarkerPrimitives, aMarkerTransform, aClipRange, *pMid))
                            {
                                pPrepared = pMid;

                                for(sal_uInt32 b(1); b < nMarkerCount - 1; b++)
                                {
                                    add_singleMarker(rTarget, aMarkerPrimitives, aMarkerTransform, aClipRange, *pPrepared, aCandidate, b);
                                }
                            }
                        }

                        if(pEnd)
                        {
                            if(pEnd == pPrepared || prepare_singleMarker(aMarkerPrimitives, aMarkerTransform, aClipRange, *pEnd))
                            {
                                pPrepared = pEnd;
                                add_singleMarker(rTarget, aMarkerPrimitives, aMarkerTransform, aClipRange, *pPrepared, aCandidate, nMarkerCount - 1);
                            }
                        }
                    }
                }
            }
        }

        void SvgStyleAttributes::add_path(
            const basegfx::B2DPolyPolygon& rPath,
            drawinglayer::primitive2d::Primitive2DSequence& rTarget) const
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

            const double fOpacity(getOpacity().getNumber());

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
                    // nonzero is wanted, solve geometrically (see description on basegfx)
                    aPath = basegfx::tools::createNonzeroConform(aPath);
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
                add_markers(rPath, rTarget);
            }
        }

        void SvgStyleAttributes::add_postProcess(
            drawinglayer::primitive2d::Primitive2DSequence& rTarget,
            const drawinglayer::primitive2d::Primitive2DSequence& rSource,
            const basegfx::B2DHomMatrix* pTransform) const
        {
            if(rSource.hasElements())
            {
                const double fOpacity(getOpacity().getNumber());

                if(basegfx::fTools::equalZero(fOpacity))
                {
                    return;
                }

                drawinglayer::primitive2d::Primitive2DSequence aSource(rSource);

                if(basegfx::fTools::less(fOpacity, 1.0))
                {
                    // embed in UnifiedTransparencePrimitive2D
                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                            aSource,
                            1.0 - fOpacity));

                    aSource = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                }

                if(pTransform)
                {
                    // create embedding group element with transformation. This applies the given
                    // transformation to the graphical content, but *not* to mask and/or clip (as needed)
                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                        new drawinglayer::primitive2d::TransformPrimitive2D(
                            *pTransform,
                            aSource));

                    aSource = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                }

                if(!getClipPathXLink().isEmpty())
                {
                    // try to access linked ClipPath
                    const SvgClipPathNode* mpClip = dynamic_cast< const SvgClipPathNode* >(mrOwner.getDocument().findSvgNodeById(getClipPathXLink()));

                    if(mpClip)
                    {
                        mpClip->apply(aSource);
                    }
                }

                if(aSource.hasElements()) // test again, applied clipPath may have lead to empty geometry
                {
                    if(!getMaskXLink().isEmpty())
                    {
                        // try to access linked Mask
                        const SvgMaskNode* mpMask = dynamic_cast< const SvgMaskNode* >(mrOwner.getDocument().findSvgNodeById(getMaskXLink()));

                        if(mpMask)
                        {
                            mpMask->apply(aSource);
                        }
                    }

                    if(aSource.hasElements()) // test again, applied mask may have lead to empty geometry
                    {
                        // append to current target
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSource);
                    }
                }
            }
        }

        SvgStyleAttributes::SvgStyleAttributes(SvgNode& rOwner)
        :   mrOwner(rOwner),
            mpCssStyleParent(0),
            maFill(),
            maStroke(),
            maStopColor(basegfx::BColor(0.0, 0.0, 0.0), true),
            maStrokeWidth(),
            maStopOpacity(),
            mpSvgGradientNodeFill(0),
            mpSvgGradientNodeStroke(0),
            mpSvgPatternNodeFill(0),
            mpSvgPatternNodeStroke(0),
            maFillOpacity(),
            maStrokeDasharray(),
            maStrokeDashOffset(),
            maStrokeLinecap(StrokeLinecap_notset),
            maStrokeLinejoin(StrokeLinejoin_notset),
            maStrokeMiterLimit(),
            maStrokeOpacity(),
            maFontFamily(),
            maFontSize(),
            maFontStretch(FontStretch_notset),
            maFontStyle(FontStyle_notset),
            maFontVariant(FontVariant_notset),
            maFontWeight(FontWeight_notset),
            maTextAlign(TextAlign_notset),
            maTextDecoration(TextDecoration_notset),
            maTextAnchor(TextAnchor_notset),
            maColor(),
            maOpacity(1.0),
            maTitle(),
            maDesc(),
            maClipPathXLink(),
            maMaskXLink(),
            maMarkerStartXLink(),
            mpMarkerStartXLink(0),
            maMarkerMidXLink(),
            mpMarkerMidXLink(0),
            maMarkerEndXLink(),
            mpMarkerEndXLink(0),
            maFillRule(FillRule_notset),
            maClipRule(FillRule_nonzero),
            maBaselineShift(BaselineShift_Baseline),
            maBaselineShiftNumber(0),
            mbIsClipPathContent(SVGTokenClipPathNode == mrOwner.getType()),
            mbStrokeDasharraySet(false)
        {
            if(!mbIsClipPathContent)
            {
                const SvgStyleAttributes* pParentStyle = getParentStyle();

                if(pParentStyle)
                {
                    mbIsClipPathContent = pParentStyle->mbIsClipPathContent;
                }
            }
        }

        SvgStyleAttributes::~SvgStyleAttributes()
        {
        }

        void SvgStyleAttributes::parseStyleAttribute(const OUString& /*rTokenName*/, SVGToken aSVGToken, const OUString& aContent)
        {
            switch(aSVGToken)
            {
                case SVGTokenFill:
                {
                    SvgPaint aSvgPaint;
                    OUString aURL;

                    if(readSvgPaint(aContent, aSvgPaint, aURL))
                    {
                        setFill(aSvgPaint);
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
                        if(aNum.isPositive())
                        {
                            setFillOpacity(aNum);
                        }
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

                    if(readSvgPaint(aContent, aSvgPaint, aURL))
                    {
                        setStroke(aSvgPaint);
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
                        static rtl::OUString aStrNone(rtl::OUString::createFromAscii("none"));
                        SvgNumberVector aVector;

                        if(aContent.match(aStrNone))
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
                        static OUString aStrButt(OUString::createFromAscii("butt"));
                        static OUString aStrRound(OUString::createFromAscii("round"));
                        static OUString aStrSquare(OUString::createFromAscii("square"));

                        if(aContent.match(aStrButt))
                        {
                            setStrokeLinecap(StrokeLinecap_butt);
                        }
                        else if(aContent.match(aStrRound))
                        {
                            setStrokeLinecap(StrokeLinecap_round);
                        }
                        else if(aContent.match(aStrSquare))
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
                        static OUString aStrMiter(OUString::createFromAscii("miter"));
                        static OUString aStrRound(OUString::createFromAscii("round"));
                        static OUString aStrBevel(OUString::createFromAscii("bevel"));

                        if(aContent.match(aStrMiter))
                        {
                            setStrokeLinejoin(StrokeLinejoin_miter);
                        }
                        else if(aContent.match(aStrRound))
                        {
                            setStrokeLinejoin(StrokeLinejoin_round);
                        }
                        else if(aContent.match(aStrBevel))
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
                        if(aNum.isPositive())
                        {
                            setStrokeMiterLimit(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenStrokeOpacity:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setStrokeOpacity(aNum);
                        }
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

                    if(readSvgPaint(aContent, aSvgPaint, aURL))
                    {
                        setStopColor(aSvgPaint);
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
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setFontSize(aNum);
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
                        static OUString aStrNormal(OUString::createFromAscii("normal"));
                        static OUString aStrWider(OUString::createFromAscii("wider"));
                        static OUString aStrNarrower(OUString::createFromAscii("narrower"));
                        static OUString aStrUltra_condensed(OUString::createFromAscii("ultra-condensed"));
                        static OUString aStrExtra_condensed(OUString::createFromAscii("extra-condensed"));
                        static OUString aStrCondensed(OUString::createFromAscii("condensed"));
                        static OUString aStrSemi_condensed(OUString::createFromAscii("semi-condensed"));
                        static OUString aStrSemi_expanded(OUString::createFromAscii("semi-expanded"));
                        static OUString aStrExpanded(OUString::createFromAscii("expanded"));
                        static OUString aStrExtra_expanded(OUString::createFromAscii("extra-expanded"));
                        static OUString aStrUltra_expanded(OUString::createFromAscii("ultra-expanded"));

                        if(aContent.match(aStrNormal))
                        {
                            setFontStretch(FontStretch_normal);
                        }
                        else if(aContent.match(aStrWider))
                        {
                            setFontStretch(FontStretch_wider);
                        }
                        else if(aContent.match(aStrNarrower))
                        {
                            setFontStretch(FontStretch_narrower);
                        }
                        else if(aContent.match(aStrUltra_condensed))
                        {
                            setFontStretch(FontStretch_ultra_condensed);
                        }
                        else if(aContent.match(aStrExtra_condensed))
                        {
                            setFontStretch(FontStretch_extra_condensed);
                        }
                        else if(aContent.match(aStrCondensed))
                        {
                            setFontStretch(FontStretch_condensed);
                        }
                        else if(aContent.match(aStrSemi_condensed))
                        {
                            setFontStretch(FontStretch_semi_condensed);
                        }
                        else if(aContent.match(aStrSemi_expanded))
                        {
                            setFontStretch(FontStretch_semi_expanded);
                        }
                        else if(aContent.match(aStrExpanded))
                        {
                            setFontStretch(FontStretch_expanded);
                        }
                        else if(aContent.match(aStrExtra_expanded))
                        {
                            setFontStretch(FontStretch_extra_expanded);
                        }
                        else if(aContent.match(aStrUltra_expanded))
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
                        static OUString aStrNormal(OUString::createFromAscii("normal"));
                        static OUString aStrItalic(OUString::createFromAscii("italic"));
                        static OUString aStrOblique(OUString::createFromAscii("oblique"));

                        if(aContent.match(aStrNormal))
                        {
                            setFontStyle(FontStyle_normal);
                        }
                        else if(aContent.match(aStrItalic))
                        {
                            setFontStyle(FontStyle_italic);
                        }
                        else if(aContent.match(aStrOblique))
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
                        static OUString aStrNormal(OUString::createFromAscii("normal"));
                        static OUString aStrSmallCaps(OUString::createFromAscii("small-caps"));

                        if(aContent.match(aStrNormal))
                        {
                            setFontVariant(FontVariant_normal);
                        }
                        else if(aContent.match(aStrSmallCaps))
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
                        static OUString aStrNormal(OUString::createFromAscii("normal"));
                        static OUString aStrBold(OUString::createFromAscii("bold"));
                        static OUString aStrBolder(OUString::createFromAscii("bolder"));
                        static OUString aStrLighter(OUString::createFromAscii("lighter"));
                        static OUString aStr100(OUString::createFromAscii("100"));
                        static OUString aStr200(OUString::createFromAscii("200"));
                        static OUString aStr300(OUString::createFromAscii("300"));
                        static OUString aStr400(OUString::createFromAscii("400"));
                        static OUString aStr500(OUString::createFromAscii("500"));
                        static OUString aStr600(OUString::createFromAscii("600"));
                        static OUString aStr700(OUString::createFromAscii("700"));
                        static OUString aStr800(OUString::createFromAscii("800"));
                        static OUString aStr900(OUString::createFromAscii("900"));

                        if(aContent.match(aStr100))
                        {
                            setFontWeight(FontWeight_100);
                        }
                        else if(aContent.match(aStr200))
                        {
                            setFontWeight(FontWeight_200);
                        }
                        else if(aContent.match(aStr300))
                        {
                            setFontWeight(FontWeight_300);
                        }
                        else if(aContent.match(aStr400) || aContent.match(aStrNormal))
                        {
                            setFontWeight(FontWeight_400);
                        }
                        else if(aContent.match(aStr500))
                        {
                            setFontWeight(FontWeight_500);
                        }
                        else if(aContent.match(aStr600))
                        {
                            setFontWeight(FontWeight_600);
                        }
                        else if(aContent.match(aStr700) || aContent.match(aStrBold))
                        {
                            setFontWeight(FontWeight_700);
                        }
                        else if(aContent.match(aStr800))
                        {
                            setFontWeight(FontWeight_800);
                        }
                        else if(aContent.match(aStr900))
                        {
                            setFontWeight(FontWeight_900);
                        }
                        else if(aContent.match(aStrBolder))
                        {
                            setFontWeight(FontWeight_bolder);
                        }
                        else if(aContent.match(aStrLighter))
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
                        static OUString aStrNone(OUString::createFromAscii("none"));
                        static OUString aStrUnderline(OUString::createFromAscii("underline"));
                        static OUString aStrOverline(OUString::createFromAscii("overline"));
                        static OUString aStrLineThrough(OUString::createFromAscii("line-through"));
                        static OUString aStrBlink(OUString::createFromAscii("blink"));

                        if(aContent.match(aStrNone))
                        {
                            setTextDecoration(TextDecoration_none);
                        }
                        else if(aContent.match(aStrUnderline))
                        {
                            setTextDecoration(TextDecoration_underline);
                        }
                        else if(aContent.match(aStrOverline))
                        {
                            setTextDecoration(TextDecoration_overline);
                        }
                        else if(aContent.match(aStrLineThrough))
                        {
                            setTextDecoration(TextDecoration_line_through);
                        }
                        else if(aContent.match(aStrBlink))
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
                        static OUString aStrStart(OUString::createFromAscii("start"));
                        static OUString aStrMiddle(OUString::createFromAscii("middle"));
                        static OUString aStrEnd(OUString::createFromAscii("end"));

                        if(aContent.match(aStrStart))
                        {
                            setTextAnchor(TextAnchor_start);
                        }
                        else if(aContent.match(aStrMiddle))
                        {
                            setTextAnchor(TextAnchor_middle);
                        }
                        else if(aContent.match(aStrEnd))
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
                        static OUString aStrLeft(OUString::createFromAscii("left"));
                        static OUString aStrRight(OUString::createFromAscii("right"));
                        static OUString aStrCenter(OUString::createFromAscii("center"));
                        static OUString aStrJustify(OUString::createFromAscii("justify"));

                        if(aContent.match(aStrLeft))
                        {
                            setTextAlign(TextAlign_left);
                        }
                        else if(aContent.match(aStrRight))
                        {
                            setTextAlign(TextAlign_right);
                        }
                        else if(aContent.match(aStrCenter))
                        {
                            setTextAlign(TextAlign_center);
                        }
                        else if(aContent.match(aStrJustify))
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

                    if(readSvgPaint(aContent, aSvgPaint, aURL))
                    {
                        setColor(aSvgPaint);
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
                    readLocalUrl(aContent, maMarkerEndXLink);
                    maMarkerStartXLink = maMarkerMidXLink = maMarkerEndXLink;
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
                        static rtl::OUString aStrSub(rtl::OUString::createFromAscii("sub"));
                        static rtl::OUString aStrSuper(rtl::OUString::createFromAscii("super"));
                        SvgNumber aNum;

                        if(aContent.match(aStrSub))
                        {
                            setBaselineShift(BaselineShift_Sub);
                        }
                        else if(aContent.match(aStrSuper))
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
                            setBaselineShift(BaselineShift_Baseline);
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

        const basegfx::BColor* SvgStyleAttributes::getFill() const
        {
            if(mbIsClipPathContent)
            {
                static basegfx::BColor aBlack(0.0, 0.0, 0.0);

                return &aBlack;
            }
            else if(maFill.isSet())
            {
                if(maFill.isCurrent())
                {
                    return getColor();
                }
                else if(maFill.isOn())
                {
                    return &maFill.getBColor();
                }
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getFill();
                }
            }

            return 0;
        }

        const basegfx::BColor* SvgStyleAttributes::getStroke() const
        {
            if(mbIsClipPathContent)
            {
                return 0;
            }
            else if(maStroke.isSet())
            {
                if(maStroke.isCurrent())
                {
                    return getColor();
                }
                else if(maStroke.isOn())
                {
                    return &maStroke.getBColor();
                }
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getStroke();
                }
            }

            return 0;
        }

        const basegfx::BColor& SvgStyleAttributes::getStopColor() const
        {
            if(maStopColor.isCurrent())
            {
                return *getColor();
            }
            else
            {
                return maStopColor.getBColor();
            }
        }

        const SvgGradientNode* SvgStyleAttributes::getSvgGradientNodeFill() const
        {
            if(mbIsClipPathContent)
            {
                return 0;
            }
            else if(mpSvgGradientNodeFill)
            {
                return mpSvgGradientNodeFill;
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgGradientNodeFill();
                }
            }

            return 0;
        }

        const SvgGradientNode* SvgStyleAttributes::getSvgGradientNodeStroke() const
        {
            if(mbIsClipPathContent)
            {
                return 0;
            }
            else if(mpSvgGradientNodeStroke)
            {
                return mpSvgGradientNodeStroke;
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgGradientNodeStroke();
                }
            }

            return 0;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeFill() const
        {
            if(mbIsClipPathContent)
            {
                return 0;
            }
            else if(mpSvgPatternNodeFill)
            {
                return mpSvgPatternNodeFill;
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgPatternNodeFill();
                }
            }

            return 0;
        }

        const SvgPatternNode* SvgStyleAttributes::getSvgPatternNodeStroke() const
        {
            if(mbIsClipPathContent)
            {
                return 0;
            }
            else if(mpSvgPatternNodeStroke)
            {
                return mpSvgPatternNodeStroke;
            }
            else
            {
                const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                if(pSvgStyleAttributes)
                {
                    return pSvgStyleAttributes->getSvgPatternNodeStroke();
                }
            }

            return 0;
        }

        SvgNumber SvgStyleAttributes::getStrokeWidth() const
        {
            if(mbIsClipPathContent)
            {
                return SvgNumber(0.0);
            }
            else if(maStrokeWidth.isSet())
            {
                return maStrokeWidth;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getStrokeWidth();
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
            if(mbIsClipPathContent)
            {
                return SvgNumber(1.0);
            }
            else if(maFillOpacity.isSet())
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
            return SvgNumber(4.0);
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
            if(!maFontFamily.empty())
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

        SvgNumber SvgStyleAttributes::getFontSize() const
        {
            if(maFontSize.isSet())
            {
                // #122524# Handle Unit_percent realtive to parent FontSize (see SVG1.1
                // spec 10.10 Font selection properties ‘font-size’, lastline (klick 'normative
                // definition of the property')
                if(Unit_percent == maFontSize.getUnit())
                {
                    const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

                    if(pSvgStyleAttributes)
                    {
                        const SvgNumber aParentNumber = pSvgStyleAttributes->getFontSize();

                        return SvgNumber(
                            aParentNumber.getNumber() * maFontSize.getNumber() * 0.01,
                            aParentNumber.getUnit(),
                            true);
                    }
                }

                return maFontSize;
            }

            const SvgStyleAttributes* pSvgStyleAttributes = getParentStyle();

            if(pSvgStyleAttributes)
            {
                return pSvgStyleAttributes->getFontSize();
            }

            // default is 'medium'
            return SvgNumber(12.0);
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
            return 0;
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
                    return 0;
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

            return 0;
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
