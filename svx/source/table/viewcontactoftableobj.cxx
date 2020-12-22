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


#include "viewcontactoftableobj.hxx"
#include <svx/svdotable.hxx>
#include <com/sun/star/table/XTable.hpp>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <editeng/borderline.hxx>
#include <sdr/attribute/sdrfilltextattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/framelink.hxx>
#include <svx/framelinkarray.hxx>
#include <svx/sdooitm.hxx>
#include <vcl/canvastools.hxx>

#include <cell.hxx>
#include "tablelayouter.hxx"


using editeng::SvxBorderLine;
using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        namespace {

        class SdrCellPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrFillTextAttribute             maSdrFTAttribute;

        protected:
            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrCellPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrFillTextAttribute& rSdrFTAttribute)
            :   BufferedDecompositionPrimitive2D(),
                maTransform(rTransform),
                maSdrFTAttribute(rSdrFTAttribute)
            {
            }

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrFillTextAttribute& getSdrFTAttribute() const { return maSdrFTAttribute; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        }

        void SdrCellPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            // prepare unit polygon
            const basegfx::B2DPolyPolygon aUnitPolyPolygon(basegfx::utils::createUnitPolygon());

            // add fill
            if(!getSdrFTAttribute().getFill().isDefault())
            {
                basegfx::B2DPolyPolygon aTransformed(aUnitPolyPolygon);

                aTransformed.transform(getTransform());
                rContainer.push_back(
                    createPolyPolygonFillPrimitive(
                        aTransformed,
                        getSdrFTAttribute().getFill(),
                        getSdrFTAttribute().getFillFloatTransGradient()));
            }
            else
            {
                // if no fill create one for HitTest and BoundRect fallback
                rContainer.push_back(
                    createHiddenGeometryPrimitives2D(
                        true,
                        aUnitPolyPolygon,
                        getTransform()));
            }

            // add text
            if(!getSdrFTAttribute().getText().isDefault())
            {
                rContainer.push_back(
                    createTextPrimitive(
                        aUnitPolyPolygon,
                        getTransform(),
                        getSdrFTAttribute().getText(),
                        attribute::SdrLineAttribute(),
                        true,
                        false));
            }
        }

        bool SdrCellPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCellPrimitive2D& rCompare = static_cast<const SdrCellPrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getSdrFTAttribute() == rCompare.getSdrFTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrCellPrimitive2D, PRIMITIVE2D_ID_SDRCELLPRIMITIVE2D)

} // end of namespace

namespace sdr::contact
{
        static svx::frame::Style impGetLineStyle(
            const sdr::table::TableLayouter& rLayouter,
            sal_Int32 nX,
            sal_Int32 nY,
            bool bHorizontal,
            sal_Int32 nColCount,
            sal_Int32 nRowCount,
            bool bIsRTL)
        {
            if(nX >= 0 && nX <= nColCount && nY >= 0 && nY <= nRowCount)
            {
                const SvxBorderLine* pLine = rLayouter.getBorderLine(nX, nY, bHorizontal);

                if(pLine)
                {
                    // copy line content
                    SvxBorderLine aLine(*pLine);

                    // check for mirroring. This shall always be done when it is
                    // not a top- or rightmost line
                    bool bMirror(aLine.isDouble());

                    if(bMirror)
                    {
                        if(bHorizontal)
                        {
                            // mirror all bottom lines
                            bMirror = (0 != nY);
                        }
                        else
                        {
                            // mirror all left lines
                            bMirror = (bIsRTL ? 0 != nX : nX != nColCount);
                        }
                    }

                    if(bMirror)
                    {
                        aLine.SetMirrorWidths( );
                    }

                    const double fTwipsToMM(127.0 / 72.0);
                    return svx::frame::Style(&aLine, fTwipsToMM);
                }
            }

            // no success, copy empty line
            return svx::frame::Style();
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfTableObj::createViewIndependentPrimitive2DSequence() const
        {
            const sdr::table::SdrTableObj& rTableObj = static_cast<const sdr::table::SdrTableObj&>(GetSdrObject());
            const uno::Reference< css::table::XTable > xTable = rTableObj.getTable();

            if(xTable.is())
            {
                // create primitive representation for table. Cell info goes
                // directly to aRetval, Border info to aBorderSequence and added
                // later to get the correct overlapping
                drawinglayer::primitive2d::Primitive2DContainer aRetval;
                drawinglayer::primitive2d::Primitive2DContainer aRetvalForShadow;
                const sal_Int32 nRowCount(xTable->getRowCount());
                const sal_Int32 nColCount(xTable->getColumnCount());
                const sal_Int32 nAllCount(nRowCount * nColCount);

                if(nAllCount)
                {
                    const sdr::table::TableLayouter& rTableLayouter(rTableObj.getTableLayouter());
                    const bool bIsRTL(css::text::WritingMode_RL_TB == rTableObj.GetWritingMode());
                    sdr::table::CellPos aCellPos;
                    sdr::table::CellRef xCurrentCell;
                    basegfx::B2IRectangle aCellArea;

                    // create range using the model data directly. This is in SdrTextObj::aRect which i will access using
                    // GetGeoRect() to not trigger any calculations. It's the unrotated geometry.
                    const basegfx::B2DRange aObjectRange = vcl::unotools::b2DRectangleFromRectangle(rTableObj.GetGeoRect());

                    // To create the CellBorderPrimitives, use the tolling from svx::frame::Array
                    // which is capable of creating the needed visualization. Fill it during the
                    // anyways needed run over the table.
                    svx::frame::Array aArray;

                    // initialize CellBorderArray for primitive creation
                    aArray.Initialize(nColCount, nRowCount);

                    // create single primitives per cell
                    for(aCellPos.mnRow = 0; aCellPos.mnRow < nRowCount; aCellPos.mnRow++)
                    {
                        // add RowHeight to CellBorderArray for primitive creation
                        aArray.SetRowHeight(aCellPos.mnRow, rTableLayouter.getRowHeight(aCellPos.mnRow));

                        for(aCellPos.mnCol = 0; aCellPos.mnCol < nColCount; aCellPos.mnCol++)
                        {
                            // add ColWidth to CellBorderArray for primitive creation, only
                            // needs to be done in the 1st run
                            if(0 == aCellPos.mnRow)
                            {
                                aArray.SetColWidth(aCellPos.mnCol, rTableLayouter.getColumnWidth(aCellPos.mnCol));
                            }

                            // access the cell
                            xCurrentCell.set(dynamic_cast< sdr::table::Cell* >(xTable->getCellByPosition(aCellPos.mnCol, aCellPos.mnRow).get()));

                            if(xCurrentCell.is())
                            {
                                // copy styles for current cell to CellBorderArray for primitive creation
                                aArray.SetCellStyleLeft(aCellPos.mnCol, aCellPos.mnRow, impGetLineStyle(rTableLayouter, aCellPos.mnCol, aCellPos.mnRow, false, nColCount, nRowCount, bIsRTL));
                                aArray.SetCellStyleRight(aCellPos.mnCol, aCellPos.mnRow, impGetLineStyle(rTableLayouter, aCellPos.mnCol + 1, aCellPos.mnRow, false, nColCount, nRowCount, bIsRTL));
                                aArray.SetCellStyleTop(aCellPos.mnCol, aCellPos.mnRow, impGetLineStyle(rTableLayouter, aCellPos.mnCol, aCellPos.mnRow, true, nColCount, nRowCount, bIsRTL));
                                aArray.SetCellStyleBottom(aCellPos.mnCol, aCellPos.mnRow, impGetLineStyle(rTableLayouter, aCellPos.mnCol, aCellPos.mnRow + 1, true, nColCount, nRowCount, bIsRTL));

                                // ignore merged cells (all except the top-left of a merged cell)
                                if(!xCurrentCell->isMerged())
                                {
                                    // check if we are the top-left of a merged cell
                                    const sal_Int32 nXSpan(xCurrentCell->getColumnSpan());
                                    const sal_Int32 nYSpan(xCurrentCell->getRowSpan());

                                    if(nXSpan > 1 || nYSpan > 1)
                                    {
                                        // if merged, set so at CellBorderArray for primitive creation
                                        aArray.SetMergedRange(aCellPos.mnCol, aCellPos.mnRow, aCellPos.mnCol + nXSpan - 1, aCellPos.mnRow + nYSpan - 1);
                                    }
                                }
                            }

                            if(xCurrentCell.is() && !xCurrentCell->isMerged())
                            {
                                if(rTableLayouter.getCellArea(xCurrentCell, aCellPos, aCellArea))
                                {
                                    // create cell transformation matrix
                                    basegfx::B2DHomMatrix aCellMatrix;
                                    aCellMatrix.set(0, 0, static_cast<double>(aCellArea.getWidth()));
                                    aCellMatrix.set(1, 1, static_cast<double>(aCellArea.getHeight()));
                                    aCellMatrix.set(0, 2, static_cast<double>(aCellArea.getMinX()) + aObjectRange.getMinX());
                                    aCellMatrix.set(1, 2, static_cast<double>(aCellArea.getMinY()) + aObjectRange.getMinY());

                                    // handle cell fillings and text
                                    const SfxItemSet& rCellItemSet = xCurrentCell->GetItemSet();
                                    const sal_uInt32 nTextIndex(nColCount * aCellPos.mnRow + aCellPos.mnCol);
                                    const SdrText* pSdrText = rTableObj.getText(nTextIndex);
                                    drawinglayer::attribute::SdrFillTextAttribute aAttribute;

                                    if(pSdrText)
                                    {
                                        // #i101508# take cell's local text frame distances into account
                                        const sal_Int32 nLeft(xCurrentCell->GetTextLeftDistance());
                                        const sal_Int32 nRight(xCurrentCell->GetTextRightDistance());
                                        const sal_Int32 nUpper(xCurrentCell->GetTextUpperDistance());
                                        const sal_Int32 nLower(xCurrentCell->GetTextLowerDistance());

                                        aAttribute = drawinglayer::primitive2d::createNewSdrFillTextAttribute(
                                            rCellItemSet,
                                            pSdrText,
                                            &nLeft,
                                            &nUpper,
                                            &nRight,
                                            &nLower);
                                    }
                                    else
                                    {
                                        aAttribute = drawinglayer::primitive2d::createNewSdrFillTextAttribute(
                                            rCellItemSet,
                                            pSdrText);
                                    }

                                    // always create cell primitives for BoundRect and HitTest
                                    {
                                        const drawinglayer::primitive2d::Primitive2DReference xCellReference(
                                            new drawinglayer::primitive2d::SdrCellPrimitive2D(
                                                aCellMatrix, aAttribute));
                                        aRetval.append(xCellReference);
                                    }

                                    // Create cell primitive without text.
                                    aAttribute
                                        = drawinglayer::primitive2d::createNewSdrFillTextAttribute(
                                            rCellItemSet, nullptr);
                                    const drawinglayer::primitive2d::Primitive2DReference
                                        xCellReference(
                                            new drawinglayer::primitive2d::SdrCellPrimitive2D(
                                                aCellMatrix, aAttribute));
                                    aRetvalForShadow.append(xCellReference);
                                }
                            }
                        }
                    }

                    // now create all CellBorderPrimitives
                    const drawinglayer::primitive2d::Primitive2DContainer aCellBorderPrimitives(aArray.CreateB2DPrimitiveArray());

                    if(!aCellBorderPrimitives.empty())
                    {
                        // this is already scaled (due to Table in non-uniform coordinates), so
                        // first transform removing scale
                        basegfx::B2DHomMatrix aTransform(
                            basegfx::utils::createScaleB2DHomMatrix(
                                1.0 / aObjectRange.getWidth(),
                                1.0 / aObjectRange.getHeight()));

                        // If RTL, mirror the whole unified table in X and move right.
                        // This is much easier than taking this into account for the whole
                        // index calculations
                        if(bIsRTL)
                        {
                            aTransform.scale(-1.0, 1.0);
                            aTransform.translate(1.0, 0.0);
                        }

                        // create object matrix
                        const GeoStat& rGeoStat(rTableObj.GetGeoStat());
                        const double fShearX(-rGeoStat.mfTanShearAngle);
                        const double fRotate(rGeoStat.nRotationAngle ? (36000 - rGeoStat.nRotationAngle) * F_PI18000 : 0.0);
                        const basegfx::B2DHomMatrix aObjectMatrix(basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                            aObjectRange.getWidth(), aObjectRange.getHeight(), fShearX, fRotate,
                            aObjectRange.getMinX(), aObjectRange.getMinY()));

                        // add object matrix to transform. By doing so theoretically
                        // CellBorders could be also rotated/sheared for the first time ever.
                        // To completely make that work, the primitives already created in
                        // aRetval would also have to be based on ObjectMatrix, not only on
                        // ObjectRange as it currently is.
                        aTransform *= aObjectMatrix;

                        // create a transform primitive with this and embed CellBorders
                        // and append to retval
                        aRetval.append(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                aTransform,
                                aCellBorderPrimitives));

                        // Borders are always the same for shadow as well.
                        aRetvalForShadow.append(new drawinglayer::primitive2d::TransformPrimitive2D(
                            aTransform, aCellBorderPrimitives));
                    }
                }

                if(!aRetval.empty())
                {
                    // check and create evtl. shadow for created content
                    const SfxItemSet& rObjectItemSet = rTableObj.GetMergedItemSet();
                    const drawinglayer::attribute::SdrShadowAttribute aNewShadowAttribute(
                        drawinglayer::primitive2d::createNewSdrShadowAttribute(rObjectItemSet));

                    if(!aNewShadowAttribute.isDefault())
                    {
                        bool bDirectShadow
                            = rObjectItemSet.Get(SDRATTR_SHADOW, /*bSrchInParent=*/false)
                                  .GetValue();
                        if (bDirectShadow)
                        {
                            // Shadow as direct formatting: no shadow for text, to be compatible
                            // with PowerPoint.
                            basegfx::B2DHomMatrix aMatrix;
                            aRetval = drawinglayer::primitive2d::createEmbeddedShadowPrimitive(
                                aRetval, aNewShadowAttribute, aMatrix, &aRetvalForShadow);
                        }
                        else
                        {
                            // Shadow as style: shadow for text, to be backwards-compatible.
                            aRetval = drawinglayer::primitive2d::createEmbeddedShadowPrimitive(
                                aRetval, aNewShadowAttribute);
                        }
                    }
                }

                return aRetval;
            }
            else
            {
                // take unrotated snap rect (direct model data) for position and size
                const basegfx::B2DRange aObjectRange = vcl::unotools::b2DRectangleFromRectangle(rTableObj.GetGeoRect());

                // create object matrix
                const GeoStat& rGeoStat(rTableObj.GetGeoStat());
                const double fShearX(-rGeoStat.mfTanShearAngle);
                const double fRotate(rGeoStat.nRotationAngle ? (36000 - rGeoStat.nRotationAngle) * F_PI18000 : 0.0);
                const basegfx::B2DHomMatrix aObjectMatrix(basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                    aObjectRange.getWidth(), aObjectRange.getHeight(), fShearX, fRotate,
                    aObjectRange.getMinX(), aObjectRange.getMinY()));

                // created an invisible outline for the cases where no visible content exists
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        aObjectMatrix));

                return drawinglayer::primitive2d::Primitive2DContainer { xReference };
            }
        }

        ViewContactOfTableObj::ViewContactOfTableObj(sdr::table::SdrTableObj& rTableObj)
        :   ViewContactOfSdrObj(rTableObj)
        {
        }

        ViewContactOfTableObj::~ViewContactOfTableObj()
        {
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
