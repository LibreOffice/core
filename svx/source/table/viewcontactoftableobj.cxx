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
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <editeng/borderline.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <sdr/attribute/sdrfilltextattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/framelink.hxx>

#include "cell.hxx"
#include "tablelayouter.hxx"


using editeng::SvxBorderLine;
using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive2d
    {
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
            DeclPrimitive2DIDBlock()
        };

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
                        false,
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

    } // end of namespace primitive2d
} // end of namespace drawinglayer

namespace sdr
{
    namespace contact
    {
        svx::frame::Style impGetLineStyle(
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

        void createForVector(bool bHor, drawinglayer::primitive2d::Primitive2DContainer& rContainer, const basegfx::B2DPoint& rOrigin, const basegfx::B2DVector& rX,
            const svx::frame::Style& rLine,
            const svx::frame::Style& rLeftA, const svx::frame::Style& rLeftB, const svx::frame::Style& rLeftC,
            const svx::frame::Style& rRightA, const svx::frame::Style& rRightB, const svx::frame::Style& rRightC)
        {
            /// top-left and bottom-right Style Tables
            const basegfx::B2DVector aY(basegfx::getNormalizedPerpendicular(rX));

            /// Fill top-left Style Table
            svx::frame::StyleVectorTable aStart;

            aStart.add(rLeftA, rX, -aY, bHor); // bHor ? true : false));
            aStart.add(rLeftB, rX, -rX, true); // bHor ? true : true));
            aStart.add(rLeftC, rX, aY, !bHor); // bHor ? false : true));
            aStart.sort();

            /// Fill bottom-right Style Table
            svx::frame::StyleVectorTable aEnd;
            const basegfx::B2DVector aAxis(-rX);

            aEnd.add(rRightA, aAxis, -aY, bHor); // bHor ? true : false));
            aEnd.add(rRightB, aAxis, rX, false); // bHor ? false : false));
            aEnd.add(rRightC, aAxis, aY, !bHor); // bHor ? false : true));
            aEnd.sort();

            CreateBorderPrimitives(
                rContainer,
                rOrigin,
                rX,
                rLine,
                aStart,
                aEnd,
                nullptr
            );
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
                const sal_Int32 nRowCount(xTable->getRowCount());
                const sal_Int32 nColCount(xTable->getColumnCount());
                const sal_Int32 nAllCount(nRowCount * nColCount);

                if(nAllCount)
                {
                    const sdr::table::TableLayouter& rTableLayouter = rTableObj.getTableLayouter();
                    const bool bIsRTL(css::text::WritingMode_RL_TB == rTableObj.GetWritingMode());
                    sdr::table::CellPos aCellPos;
                    sdr::table::CellRef xCurrentCell;
                    basegfx::B2IRectangle aCellArea;

                    // create range using the model data directly. This is in SdrTextObj::aRect which i will access using
                    // GetGeoRect() to not trigger any calculations. It's the unrotated geometry.
                    const tools::Rectangle& rObjectRectangle(rTableObj.GetGeoRect());
                    const basegfx::B2DRange aObjectRange(rObjectRectangle.Left(), rObjectRectangle.Top(), rObjectRectangle.Right(), rObjectRectangle.Bottom());

                    // for each cell we need potentially a cell primitive and a border primitive
                    // (e.g. single cell). Prepare sequences and input counters
                    drawinglayer::primitive2d::Primitive2DContainer aBorderSequence;

                    // create single primitives per cell
                    for(aCellPos.mnRow = 0; aCellPos.mnRow < nRowCount; aCellPos.mnRow++)
                    {
                        for(aCellPos.mnCol = 0; aCellPos.mnCol < nColCount; aCellPos.mnCol++)
                        {
                            xCurrentCell.set(dynamic_cast< sdr::table::Cell* >(xTable->getCellByPosition(aCellPos.mnCol, aCellPos.mnRow).get()));

                            if(xCurrentCell.is() && !xCurrentCell->isMerged())
                            {
                                if(rTableLayouter.getCellArea(xCurrentCell, aCellPos, aCellArea))
                                {
                                    // create cell transformation matrix
                                    basegfx::B2DHomMatrix aCellMatrix;
                                    aCellMatrix.set(0, 0, (double)aCellArea.getWidth());
                                    aCellMatrix.set(1, 1, (double)aCellArea.getHeight());
                                    aCellMatrix.set(0, 2, (double)aCellArea.getMinX() + aObjectRange.getMinX());
                                    aCellMatrix.set(1, 2, (double)aCellArea.getMinY() + aObjectRange.getMinY());

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

                                    // handle cell borders
                                    const sal_Int32 nX(bIsRTL ? nColCount - aCellPos.mnCol : aCellPos.mnCol);
                                    const sal_Int32 nY(aCellPos.mnRow);

                                    // get access values for X,Y at the cell's end
                                    const sal_Int32 nXSpan(xCurrentCell->getColumnSpan());
                                    const sal_Int32 nYSpan(xCurrentCell->getRowSpan());
                                    const sal_Int32 nXRight(bIsRTL ? nX - nXSpan : nX + nXSpan);
                                    const sal_Int32 nYBottom(nY + nYSpan);

                                    // get basic lines
                                    const svx::frame::Style aLeftLine(impGetLineStyle(rTableLayouter, nX, nY, false, nColCount, nRowCount, bIsRTL));
                                    //To resolve the bug fdo#59117
                                    //In RTL table as BottomLine & TopLine are drawn from Left Side to Right, nX should be nX-1
                                    const svx::frame::Style aBottomLine(impGetLineStyle(rTableLayouter, bIsRTL?nX-1:nX, nYBottom, true, nColCount, nRowCount, bIsRTL));
                                    const svx::frame::Style aRightLine(impGetLineStyle(rTableLayouter, nXRight, nY, false, nColCount, nRowCount, bIsRTL));
                                    const svx::frame::Style aTopLine(impGetLineStyle(rTableLayouter, bIsRTL?nX-1:nX, nY, true, nColCount, nRowCount, bIsRTL));

                                    if(aLeftLine.IsUsed() || aBottomLine.IsUsed() || aRightLine.IsUsed() || aTopLine.IsUsed())
                                    {
                                        // get the neighbor cells' borders
                                        const svx::frame::Style aLeftFromTLine(impGetLineStyle(rTableLayouter, nX, nY - 1, false, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aLeftFromBLine(impGetLineStyle(rTableLayouter, nX, nYBottom + 1, false, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aRightFromTLine(impGetLineStyle(rTableLayouter, nXRight, nY - 1, false, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aRightFromBLine(impGetLineStyle(rTableLayouter, nXRight, nYBottom + 1, false, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aTopFromLLine(impGetLineStyle(rTableLayouter, nX - 1, nY, true, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aTopFromRLine(impGetLineStyle(rTableLayouter, nXRight + 1, nY, true, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aBottomFromLLine(impGetLineStyle(rTableLayouter, nX - 1, nYBottom, true, nColCount, nRowCount, bIsRTL));
                                        const svx::frame::Style aBottomFromRLine(impGetLineStyle(rTableLayouter, nXRight + 1, nYBottom, true, nColCount, nRowCount, bIsRTL));

                                        // get cell coordinate system
                                        const basegfx::B2DPoint aOrigin(aCellMatrix * basegfx::B2DPoint(0.0, 0.0));
                                        const basegfx::B2DVector aX(aCellMatrix * basegfx::B2DVector(1.0, 0.0));
                                        const basegfx::B2DVector aY(aCellMatrix * basegfx::B2DVector(0.0, 1.0));

                                        if(aLeftLine.IsUsed())
                                        {
                                            createForVector(false, aBorderSequence, aOrigin, aY, aLeftLine,
                                                aTopLine, aLeftFromTLine, aTopFromLLine,
                                                aBottomLine, aLeftFromBLine, aBottomFromLLine);
                                        }

                                        if(aBottomLine.IsUsed())
                                        {
                                            createForVector(true, aBorderSequence, aOrigin + aY, aX, aBottomLine,
                                                aLeftLine, aBottomFromLLine, aLeftFromBLine,
                                                aRightLine, aBottomFromRLine, aRightFromBLine);
                                        }

                                        if(aRightLine.IsUsed())
                                        {
                                            createForVector(false, aBorderSequence, aOrigin + aX, aY, aRightLine,
                                                aTopFromRLine, aRightFromTLine, aTopLine,
                                                aBottomFromRLine, aRightFromBLine, aBottomLine);
                                        }

                                        if(aTopLine.IsUsed())
                                        {
                                            createForVector(true, aBorderSequence, aOrigin, aX, aTopLine,
                                                aLeftFromTLine, aTopFromLLine, aLeftLine,
                                                aRightFromTLine, aTopFromRLine, aRightLine);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // append Border info to target. We want fillings and text first
                    aRetval.append(aBorderSequence);
                }

                if(!aRetval.empty())
                {
                    // check and create evtl. shadow for created content
                    const SfxItemSet& rObjectItemSet = rTableObj.GetMergedItemSet();
                    const drawinglayer::attribute::SdrShadowAttribute aNewShadowAttribute(
                        drawinglayer::primitive2d::createNewSdrShadowAttribute(rObjectItemSet));

                    if(!aNewShadowAttribute.isDefault())
                    {
                        aRetval = drawinglayer::primitive2d::createEmbeddedShadowPrimitive(aRetval, aNewShadowAttribute);
                    }
                }

                return aRetval;
            }
            else
            {
                // take unrotated snap rect (direct model data) for position and size
                const tools::Rectangle& rRectangle = rTableObj.GetGeoRect();
                const basegfx::B2DRange aObjectRange(
                    rRectangle.Left(), rRectangle.Top(),
                    rRectangle.Right(), rRectangle.Bottom());

                // create object matrix
                const GeoStat& rGeoStat(rTableObj.GetGeoStat());
                const double fShearX(rGeoStat.nShearAngle ? tan((36000 - rGeoStat.nShearAngle) * F_PI18000) : 0.0);
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
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
