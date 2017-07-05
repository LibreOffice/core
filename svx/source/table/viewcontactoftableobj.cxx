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
            const basegfx::B2DPolyPolygon aUnitPolyPolygon(basegfx::tools::createUnitPolygon());

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


namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrBorderlinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            SvxBorderLine                               maLeftLine;
            SvxBorderLine                               maBottomLine;
            SvxBorderLine                               maRightLine;
            SvxBorderLine                               maTopLine;

            // Neighbor cells' borders
            SvxBorderLine                               maLeftFromTLine;
            SvxBorderLine                               maLeftFromBLine;
            SvxBorderLine                               maRightFromTLine;
            SvxBorderLine                               maRightFromBLine;
            SvxBorderLine                               maTopFromLLine;
            SvxBorderLine                               maTopFromRLine;
            SvxBorderLine                               maBottomFromLLine;
            SvxBorderLine                               maBottomFromRLine;

            bool                                        mbLeftIsOutside : 1;
            bool                                        mbBottomIsOutside : 1;
            bool                                        mbRightIsOutside : 1;
            bool                                        mbTopIsOutside : 1;

        protected:
            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrBorderlinePrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const SvxBorderLine& rLeftLine,
                const SvxBorderLine& rBottomLine,
                const SvxBorderLine& rRightLine,
                const SvxBorderLine& rTopLine,
                const SvxBorderLine& rLeftFromTLine,
                const SvxBorderLine& rLeftFromBLine,
                const SvxBorderLine& rRightFromTLine,
                const SvxBorderLine& rRightFromBLine,
                const SvxBorderLine& rTopFromLLine,
                const SvxBorderLine& rTopFromRLine,
                const SvxBorderLine& rBottomFromLLine,
                const SvxBorderLine& rBottomFromRLine,
                bool bLeftIsOutside,
                bool bBottomIsOutside,
                bool bRightIsOutside,
                bool bTopIsOutside)
            :   BufferedDecompositionPrimitive2D(),
                maTransform(rTransform),
                maLeftLine(rLeftLine),
                maBottomLine(rBottomLine),
                maRightLine(rRightLine),
                maTopLine(rTopLine),
                maLeftFromTLine(rLeftFromTLine),
                maLeftFromBLine(rLeftFromBLine),
                maRightFromTLine(rRightFromTLine),
                maRightFromBLine(rRightFromBLine),
                maTopFromLLine(rTopFromLLine),
                maTopFromRLine(rTopFromRLine),
                maBottomFromLLine(rBottomFromLLine),
                maBottomFromRLine(rBottomFromRLine),
                mbLeftIsOutside(bLeftIsOutside),
                mbBottomIsOutside(bBottomIsOutside),
                mbRightIsOutside(bRightIsOutside),
                mbTopIsOutside(bTopIsOutside)
            {
            }

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const SvxBorderLine& getLeftLine() const { return maLeftLine; }
            const SvxBorderLine& getBottomLine() const { return maBottomLine; }
            const SvxBorderLine& getRightLine() const { return maRightLine; }
            const SvxBorderLine& getTopLine() const { return maTopLine; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            DeclPrimitive2DIDBlock()
        };

        sal_uInt16 getBorderLineOutWidth(const SvxBorderLine& rLineA)
        {
            return (1 == rLineA.GetOutWidth() ? 0 : rLineA.GetOutWidth());
        }

        sal_uInt16 getBorderLineDistance(const SvxBorderLine& rLineA)
        {
            return (1 == rLineA.GetDistance() ? 0 : rLineA.GetDistance());
        }

        sal_uInt16 getBorderLineInWidth(const SvxBorderLine& rLineA)
        {
            return (1 == rLineA.GetInWidth() ? 0 : rLineA.GetInWidth());
        }

        sal_uInt16 getBorderLineWidth(const SvxBorderLine& rLineA)
        {
            return getBorderLineOutWidth(rLineA) + getBorderLineDistance(rLineA) + getBorderLineInWidth(rLineA);
        }

        double getExtend(const SvxBorderLine& rLineSide, const SvxBorderLine& rLineOpposite)
        {
            double nExtend = 0.0;
            if(!rLineSide.isEmpty())
            {
                // reduce to inner edge of associated matching line
                nExtend = -(getBorderLineWidth(rLineSide) / 2.0);
            }
            else
            {
                nExtend = (getBorderLineWidth(rLineOpposite) / 2.0);
            }

            return nExtend;
        }

        double getChangedValue(sal_uInt16 nValue, bool bChangeToMM)
        {
            if(1 == nValue)
                return 1.0;

            if(bChangeToMM)
                return nValue * (127.0 / 72.0);

            return (double)nValue;
        }

        void SdrBorderlinePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            const double fTwipsToMM(127.0 / 72.0);

            if(!getLeftLine().isEmpty())
            {
                // create left line from top to bottom
                basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 0.0));
                basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(0.0, 1.0));

                // Move the left border to the left.
                double fOffset = getChangedValue(getLeftLine().GetDistance(), true/*InTwips*/);
                aStart += basegfx::B2DPoint(-fOffset,-fOffset);
                aEnd += basegfx::B2DPoint(-fOffset,fOffset);

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getTopLine(), maTopFromLLine));
                    const double fExtendIE(getExtend(getBottomLine(), maBottomFromLLine));
                    const double fExtendOS(getExtend(maTopFromLLine, getTopLine()));
                    const double fExtendOE(getExtend(maBottomFromLLine, getBottomLine()));

                    rContainer.push_back(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getLeftLine().GetOutWidth(), true/*InTwips*/),
                        getChangedValue(getLeftLine().GetDistance(), true/*InTwips*/),
                        getChangedValue(getLeftLine().GetInWidth(), true/*InTwips*/),
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        getLeftLine().GetColorOut().getBColor(),
                        getLeftLine().GetColorIn().getBColor(),
                        getLeftLine().GetColorGap().getBColor(),
                        getLeftLine().HasGapColor(),
                        getLeftLine().GetBorderLineStyle()));
                }
            }

            if(!getBottomLine().isEmpty() && mbBottomIsOutside)
            {
                // create bottom line from left to right
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 1.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 1.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getLeftLine(), maLeftFromBLine ));
                    const double fExtendIE(getExtend(getRightLine(), maRightFromBLine));
                    const double fExtendOS(getExtend(maLeftFromBLine, getLeftLine()));
                    const double fExtendOE(getExtend(maRightFromBLine, getRightLine()));

                    rContainer.push_back(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getBottomLine().GetOutWidth(), true/*InTwips*/),
                        getChangedValue(getBottomLine().GetDistance(), true/*InTwips*/),
                        getChangedValue(getBottomLine().GetInWidth(), true/*InTwips*/),
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        getBottomLine().GetColorOut(false).getBColor(),
                        getBottomLine().GetColorIn(false).getBColor(),
                        getBottomLine().GetColorGap().getBColor(),
                        getBottomLine().HasGapColor(),
                        getBottomLine().GetBorderLineStyle()));
                }
            }

            if(!getRightLine().isEmpty())
            {
                // create right line from top to bottom
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(1.0, 0.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 1.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getTopLine(), maTopFromRLine));
                    const double fExtendIE(getExtend(getBottomLine(), maBottomFromRLine));
                    const double fExtendOS(getExtend(maTopFromRLine, getTopLine()));
                    const double fExtendOE(getExtend(maBottomFromRLine, getBottomLine()));

                    rContainer.push_back(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getRightLine().GetOutWidth(), true/*InTwips*/),
                        getChangedValue(getRightLine().GetDistance(), true/*InTwips*/),
                        getChangedValue(getRightLine().GetInWidth(), true/*InTwips*/),
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        getRightLine().GetColorOut().getBColor(),
                        getRightLine().GetColorIn().getBColor(),
                        getRightLine().GetColorGap().getBColor(),
                        getRightLine().HasGapColor(),
                        getRightLine().GetBorderLineStyle()));
                }
            }

            if(!getTopLine().isEmpty())
            {
                // create top line from left to right
                basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 0.0));
                basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 0.0));

                // Move the top border up a bit.
                double fOffset = getChangedValue(getTopLine().GetDistance(), true/*InTwips*/);
                aStart += basegfx::B2DPoint(-fOffset,-fOffset);
                aEnd += basegfx::B2DPoint(fOffset,-fOffset);

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getLeftLine(), maLeftFromTLine));
                    const double fExtendIE(getExtend(getRightLine(), maRightFromTLine));
                    const double fExtendOS(getExtend(maLeftFromTLine, getLeftLine()));
                    const double fExtendOE(getExtend(maRightFromTLine, getRightLine()));

                    rContainer.push_back(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getTopLine().GetOutWidth(), true/*InTwips*/),
                        getChangedValue(getTopLine().GetDistance(), true/*InTwips*/),
                        getChangedValue(getTopLine().GetInWidth(), true/*InTwips*/),
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        getTopLine().GetColorOut(false).getBColor(),
                        getTopLine().GetColorIn(false).getBColor(),
                        getTopLine().GetColorGap().getBColor(),
                        getTopLine().HasGapColor(),
                        getTopLine().GetBorderLineStyle()));
                }
            }
        }

        bool SdrBorderlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrBorderlinePrimitive2D& rCompare = static_cast<const SdrBorderlinePrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getLeftLine() == rCompare.getLeftLine()
                    && getBottomLine() == rCompare.getBottomLine()
                    && getRightLine() == rCompare.getRightLine()
                    && getTopLine() == rCompare.getTopLine()
                    && maLeftFromTLine == rCompare.maLeftFromTLine
                    && maLeftFromBLine == rCompare.maLeftFromBLine
                    && maRightFromTLine == rCompare.maRightFromTLine
                    && maRightFromBLine == rCompare.maRightFromBLine
                    && maTopFromLLine == rCompare.maTopFromLLine
                    && maTopFromRLine == rCompare.maTopFromRLine
                    && maBottomFromLLine == rCompare.maBottomFromLLine
                    && maBottomFromRLine == rCompare.maBottomFromRLine
                    && mbLeftIsOutside == rCompare.mbLeftIsOutside
                    && mbBottomIsOutside == rCompare.mbBottomIsOutside
                    && mbRightIsOutside == rCompare.mbRightIsOutside
                    && mbTopIsOutside == rCompare.mbTopIsOutside);
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrBorderlinePrimitive2D, PRIMITIVE2D_ID_SDRBORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace sdr
{
    namespace contact
    {
        void impGetLine(SvxBorderLine& aLine, const sdr::table::TableLayouter& rLayouter, sal_Int32 nX, sal_Int32 nY, bool bHorizontal, sal_Int32 nColCount, sal_Int32 nRowCount, bool bIsRTL)
        {
            if(nX >= 0 && nX <= nColCount && nY >= 0 && nY <= nRowCount)
            {
                const SvxBorderLine* pLine = rLayouter.getBorderLine(nX, nY, bHorizontal);

                if(pLine)
                {
                    // copy line content
                    aLine = *pLine;

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

                    return;
                }
            }

            // no success, copy empty line
            const SvxBorderLine aEmptyLine;
            aLine = aEmptyLine;
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfTableObj::createViewIndependentPrimitive2DSequence() const
        {
            const sdr::table::SdrTableObj& rTableObj = static_cast<const sdr::table::SdrTableObj&>(GetSdrObject());
            const uno::Reference< css::table::XTable > xTable = rTableObj.getTable();

            if(xTable.is())
            {
                // create primitive representation for table
                drawinglayer::primitive2d::Primitive2DContainer xRetval;
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
                    drawinglayer::primitive2d::Primitive2DContainer xCellSequence(nAllCount);
                    drawinglayer::primitive2d::Primitive2DContainer xBorderSequence(nAllCount);
                    sal_uInt32 nCellInsert(0);
                    sal_uInt32 nBorderInsert(0);

                    // variables for border lines
                    SvxBorderLine aLeftLine;
                    SvxBorderLine aBottomLine;
                    SvxBorderLine aRightLine;
                    SvxBorderLine aTopLine;

                    SvxBorderLine aLeftFromTLine;
                    SvxBorderLine aLeftFromBLine;
                    SvxBorderLine aRightFromTLine;
                    SvxBorderLine aRightFromBLine;
                    SvxBorderLine aTopFromLLine;
                    SvxBorderLine aTopFromRLine;
                    SvxBorderLine aBottomFromLLine;
                    SvxBorderLine aBottomFromRLine;

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
                                        xCellSequence[nCellInsert++] = xCellReference;
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
                                    impGetLine(aLeftLine, rTableLayouter, nX, nY, false, nColCount, nRowCount, bIsRTL);
                                    //To resolve the bug fdo#59117
                                    //In RTL table as BottomLine & TopLine are drawn from Left Side to Right, nX should be nX-1
                                    impGetLine(aBottomLine, rTableLayouter, bIsRTL?nX-1:nX, nYBottom, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightLine, rTableLayouter, nXRight, nY, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopLine, rTableLayouter, bIsRTL?nX-1:nX, nY, true, nColCount, nRowCount, bIsRTL);

                                    // get the neighbor cells' borders
                                    impGetLine(aLeftFromTLine, rTableLayouter, nX, nY - 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aLeftFromBLine, rTableLayouter, nX, nYBottom + 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightFromTLine, rTableLayouter, nXRight, nY - 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightFromBLine, rTableLayouter, nXRight, nYBottom + 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopFromLLine, rTableLayouter, nX - 1, nY, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopFromRLine, rTableLayouter, nXRight + 1, nY, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aBottomFromLLine, rTableLayouter, nX - 1, nYBottom, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aBottomFromRLine, rTableLayouter, nXRight + 1, nYBottom, true, nColCount, nRowCount, bIsRTL);

                                    // create the primtive containing all data for one cell with borders
                                    xBorderSequence[nBorderInsert++] = drawinglayer::primitive2d::Primitive2DReference(
                                        new drawinglayer::primitive2d::SdrBorderlinePrimitive2D(
                                            aCellMatrix,
                                            aLeftLine,
                                            aBottomLine,
                                            aRightLine,
                                            aTopLine,
                                            aLeftFromTLine,
                                            aLeftFromBLine,
                                            aRightFromTLine,
                                            aRightFromBLine,
                                            aTopFromLLine,
                                            aTopFromRLine,
                                            aBottomFromLLine,
                                            aBottomFromRLine,
                                            bIsRTL ? nX == nColCount : 0 == nX,
                                            nRowCount == nYBottom,
                                            bIsRTL ? 0 == nXRight : nXRight == nColCount,
                                            0 == nY));
                                }
                            }
                        }
                    }

                    // no empty references; reallocate sequences by used count
                    xCellSequence.resize(nCellInsert);
                    xBorderSequence.resize(nBorderInsert);

                    // append to target. We want fillings and text first
                    xRetval = xCellSequence;
                    xRetval.append(xBorderSequence);
                }

                if(!xRetval.empty())
                {
                    // check and create evtl. shadow for created content
                    const SfxItemSet& rObjectItemSet = rTableObj.GetMergedItemSet();
                    const drawinglayer::attribute::SdrShadowAttribute aNewShadowAttribute(
                        drawinglayer::primitive2d::createNewSdrShadowAttribute(rObjectItemSet));

                    if(!aNewShadowAttribute.isDefault())
                    {
                        xRetval = drawinglayer::primitive2d::createEmbeddedShadowPrimitive(xRetval, aNewShadowAttribute);
                    }
                }

                return xRetval;
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
                const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
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
