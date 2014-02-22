/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <svx/sdr/attribute/sdrfilltextattribute.hxx>
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
            
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrCellPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrFillTextAttribute& rSdrFTAttribute)
            :   BufferedDecompositionPrimitive2D(),
                maTransform(rTransform),
                maSdrFTAttribute(rSdrFTAttribute)
            {
            }

            
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrFillTextAttribute& getSdrFTAttribute() const { return maSdrFTAttribute; }

            
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            
            DeclPrimitive2DIDBlock()
        };

        Primitive2DSequence SdrCellPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            
            Primitive2DSequence aRetval;
            const basegfx::B2DPolyPolygon aUnitPolyPolygon(basegfx::tools::createUnitPolygon());

            
            if(!getSdrFTAttribute().getFill().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createPolyPolygonFillPrimitive(
                        aUnitPolyPolygon,
                        getTransform(),
                        getSdrFTAttribute().getFill(),
                        getSdrFTAttribute().getFillFloatTransGradient()));
            }
            else
            {
                
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createHiddenGeometryPrimitives2D(
                        true,
                        aUnitPolyPolygon,
                        getTransform()));
            }

            
            if(!getSdrFTAttribute().getText().isDefault())
            {
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        aUnitPolyPolygon,
                        getTransform(),
                        getSdrFTAttribute().getText(),
                        attribute::SdrLineAttribute(),
                        true,
                        false,
                        false));
            }

            return aRetval;
        }

        bool SdrCellPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCellPrimitive2D& rCompare = (SdrCellPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getSdrFTAttribute() == rCompare.getSdrFTAttribute());
            }

            return false;
        }

        
        ImplPrimitive2DIDBlock(SdrCellPrimitive2D, PRIMITIVE2D_ID_SDRCELLPRIMITIVE2D)

    } 
} 



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
            bool                                        mbInTwips : 1;

        protected:
            
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

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
                bool bTopIsOutside,
                bool bInTwips)
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
                mbTopIsOutside(bTopIsOutside),
                mbInTwips(bInTwips)
            {
            }

            
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const SvxBorderLine& getLeftLine() const { return maLeftLine; }
            const SvxBorderLine& getBottomLine() const { return maBottomLine; }
            const SvxBorderLine& getRightLine() const { return maRightLine; }
            const SvxBorderLine& getTopLine() const { return maTopLine; }
            bool getLeftIsOutside() const { return mbLeftIsOutside; }
            bool getBottomIsOutside() const { return mbBottomIsOutside; }
            bool getRightIsOutside() const { return mbRightIsOutside; }
            bool getTopIsOutside() const { return mbTopIsOutside; }
            bool getInTwips() const { return mbInTwips; }

            
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            
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
                
                nExtend = -((getBorderLineWidth(rLineSide) / 2.0));
            }
            else
            {
                nExtend = ((getBorderLineWidth(rLineOpposite) / 2.0));
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

        Primitive2DSequence SdrBorderlinePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence xRetval(4);
            sal_uInt32 nInsert(0);
            const double fTwipsToMM(getInTwips() ? (127.0 / 72.0) : 1.0);

            if(!getLeftLine().isEmpty())
            {
                
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 0.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(0.0, 1.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getTopLine(), maTopFromLLine));
                    const double fExtendIE(getExtend(getBottomLine(), maBottomFromLLine));
                    const double fExtendOS(getExtend(maTopFromLLine, getTopLine()));
                    const double fExtendOE(getExtend(maBottomFromLLine, getBottomLine()));

                    xRetval[nInsert++] = Primitive2DReference(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getLeftLine().GetOutWidth(), getInTwips()),
                        getChangedValue(getLeftLine().GetDistance(), getInTwips()),
                        getChangedValue(getLeftLine().GetInWidth(), getInTwips()),
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        getLeftLine().GetColorOut(true).getBColor(),
                        getLeftLine().GetColorIn(true).getBColor(),
                        getLeftLine().GetColorGap().getBColor(),
                        getLeftLine().HasGapColor(),
                        getLeftLine().GetBorderLineStyle()));
                }
            }

            if(!getBottomLine().isEmpty() && getBottomIsOutside())
            {
                
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 1.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 1.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getLeftLine(), maLeftFromBLine ));
                    const double fExtendIE(getExtend(getRightLine(), maRightFromBLine));
                    const double fExtendOS(getExtend(maLeftFromBLine, getLeftLine()));
                    const double fExtendOE(getExtend(maRightFromBLine, getRightLine()));

                    xRetval[nInsert++] = Primitive2DReference(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getBottomLine().GetOutWidth(), getInTwips()),
                        getChangedValue(getBottomLine().GetDistance(), getInTwips()),
                        getChangedValue(getBottomLine().GetInWidth(), getInTwips()),
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
                
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(1.0, 0.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 1.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getTopLine(), maTopFromRLine));
                    const double fExtendIE(getExtend(getBottomLine(), maBottomFromRLine));
                    const double fExtendOS(getExtend(maTopFromRLine, getTopLine()));
                    const double fExtendOE(getExtend(maBottomFromRLine, getBottomLine()));

                    xRetval[nInsert++] = Primitive2DReference(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getRightLine().GetOutWidth(), getInTwips()),
                        getChangedValue(getRightLine().GetDistance(), getInTwips()),
                        getChangedValue(getRightLine().GetInWidth(), getInTwips()),
                        fExtendOS * fTwipsToMM,
                        fExtendOE * fTwipsToMM,
                        fExtendIS * fTwipsToMM,
                        fExtendIE * fTwipsToMM,
                        getRightLine().GetColorOut(true).getBColor(),
                        getRightLine().GetColorIn(true).getBColor(),
                        getRightLine().GetColorGap().getBColor(),
                        getRightLine().HasGapColor(),
                        getRightLine().GetBorderLineStyle()));
                }
            }

            if(!getTopLine().isEmpty())
            {
                
                const basegfx::B2DPoint aStart(getTransform() * basegfx::B2DPoint(0.0, 0.0));
                const basegfx::B2DPoint aEnd(getTransform() * basegfx::B2DPoint(1.0, 0.0));

                if(!aStart.equal(aEnd))
                {
                    const double fExtendIS(getExtend(getLeftLine(), maLeftFromTLine));
                    const double fExtendIE(getExtend(getRightLine(), maRightFromTLine));
                    const double fExtendOS(getExtend(maLeftFromTLine, getLeftLine()));
                    const double fExtendOE(getExtend(maRightFromTLine, getRightLine()));

                    xRetval[nInsert++] = Primitive2DReference(new BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        getChangedValue(getTopLine().GetOutWidth(), getInTwips()),
                        getChangedValue(getTopLine().GetDistance(), getInTwips()),
                        getChangedValue(getTopLine().GetInWidth(), getInTwips()),
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

            xRetval.realloc(nInsert);
            return xRetval;
        }

        bool SdrBorderlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrBorderlinePrimitive2D& rCompare = (SdrBorderlinePrimitive2D&)rPrimitive;

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
                    && getLeftIsOutside() == rCompare.getLeftIsOutside()
                    && getBottomIsOutside() == rCompare.getBottomIsOutside()
                    && getRightIsOutside() == rCompare.getRightIsOutside()
                    && getTopIsOutside() == rCompare.getTopIsOutside()
                    && getInTwips() == rCompare.getInTwips());
            }

            return false;
        }

        
        ImplPrimitive2DIDBlock(SdrBorderlinePrimitive2D, PRIMITIVE2D_ID_SDRBORDERLINEPRIMITIVE2D)

    } 
} 



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
                    
                    aLine = *pLine;

                    
                    
                    bool bMirror(aLine.isDouble());

                    if(bMirror)
                    {
                        if(bHorizontal)
                        {
                            
                            bMirror = (0 != nY);
                        }
                        else
                        {
                            
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

            
            const SvxBorderLine aEmptyLine;
            aLine = aEmptyLine;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfTableObj::createViewIndependentPrimitive2DSequence() const
        {
            const sdr::table::SdrTableObj& rTableObj = GetTableObj();
            const uno::Reference< com::sun::star::table::XTable > xTable = rTableObj.getTable();

            if(xTable.is())
            {
                
                drawinglayer::primitive2d::Primitive2DSequence xRetval;
                const sal_Int32 nRowCount(xTable->getRowCount());
                const sal_Int32 nColCount(xTable->getColumnCount());
                const sal_Int32 nAllCount(nRowCount * nColCount);

                if(nAllCount)
                {
                    const sdr::table::TableLayouter& rTableLayouter = rTableObj.getTableLayouter();
                    const bool bIsRTL(com::sun::star::text::WritingMode_RL_TB == rTableObj.GetWritingMode());
                    sdr::table::CellPos aCellPos;
                    sdr::table::CellRef xCurrentCell;
                    basegfx::B2IRectangle aCellArea;

                    
                    
                    const Rectangle& rObjectRectangle(rTableObj.GetGeoRect());
                    const basegfx::B2DRange aObjectRange(rObjectRectangle.Left(), rObjectRectangle.Top(), rObjectRectangle.Right(), rObjectRectangle.Bottom());

                    
                    
                    drawinglayer::primitive2d::Primitive2DSequence xCellSequence(nAllCount);
                    drawinglayer::primitive2d::Primitive2DSequence xBorderSequence(nAllCount);
                    sal_uInt32 nCellInsert(0);
                    sal_uInt32 nBorderInsert(0);

                    
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

                    
                    for(aCellPos.mnRow = 0; aCellPos.mnRow < nRowCount; aCellPos.mnRow++)
                    {
                        for(aCellPos.mnCol = 0; aCellPos.mnCol < nColCount; aCellPos.mnCol++)
                        {
                            xCurrentCell.set(dynamic_cast< sdr::table::Cell* >(xTable->getCellByPosition(aCellPos.mnCol, aCellPos.mnRow).get()));

                            if(xCurrentCell.is() && !xCurrentCell->isMerged())
                            {
                                if(rTableLayouter.getCellArea(aCellPos, aCellArea))
                                {
                                    
                                    basegfx::B2DHomMatrix aCellMatrix;
                                    aCellMatrix.set(0, 0, (double)aCellArea.getWidth());
                                    aCellMatrix.set(1, 1, (double)aCellArea.getHeight());
                                    aCellMatrix.set(0, 2, (double)aCellArea.getMinX() + aObjectRange.getMinX());
                                    aCellMatrix.set(1, 2, (double)aCellArea.getMinY() + aObjectRange.getMinY());

                                    
                                    const SfxItemSet& rCellItemSet = xCurrentCell->GetItemSet();
                                    const sal_uInt32 nTextIndex(nColCount * aCellPos.mnRow + aCellPos.mnCol);
                                    const SdrText* pSdrText = rTableObj.getText(nTextIndex);
                                    drawinglayer::attribute::SdrFillTextAttribute aAttribute;

                                    if(pSdrText)
                                    {
                                        
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

                                    
                                    {
                                        const drawinglayer::primitive2d::Primitive2DReference xCellReference(
                                            new drawinglayer::primitive2d::SdrCellPrimitive2D(
                                                aCellMatrix, aAttribute));
                                        xCellSequence[nCellInsert++] = xCellReference;
                                    }

                                    
                                    const sal_Int32 nX(bIsRTL ? nColCount - aCellPos.mnCol : aCellPos.mnCol);
                                    const sal_Int32 nY(aCellPos.mnRow);

                                    
                                    const sal_Int32 nXSpan(xCurrentCell->getColumnSpan());
                                    const sal_Int32 nYSpan(xCurrentCell->getRowSpan());
                                    const sal_Int32 nXRight(bIsRTL ? nX - nXSpan : nX + nXSpan);
                                    const sal_Int32 nYBottom(nY + nYSpan);

                                    
                                    impGetLine(aLeftLine, rTableLayouter, nX, nY, false, nColCount, nRowCount, bIsRTL);
                                    
                                    
                                    impGetLine(aBottomLine, rTableLayouter, bIsRTL?nX-1:nX, nYBottom, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightLine, rTableLayouter, nXRight, nY, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopLine, rTableLayouter, bIsRTL?nX-1:nX, nY, true, nColCount, nRowCount, bIsRTL);

                                    
                                    impGetLine(aLeftFromTLine, rTableLayouter, nX, nY - 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aLeftFromBLine, rTableLayouter, nX, nYBottom + 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightFromTLine, rTableLayouter, nXRight, nY - 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aRightFromBLine, rTableLayouter, nXRight, nYBottom + 1, false, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopFromLLine, rTableLayouter, nX - 1, nY, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aTopFromRLine, rTableLayouter, nXRight + 1, nY, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aBottomFromLLine, rTableLayouter, nX - 1, nYBottom, true, nColCount, nRowCount, bIsRTL);
                                    impGetLine(aBottomFromRLine, rTableLayouter, nXRight + 1, nYBottom, true, nColCount, nRowCount, bIsRTL);

                                    
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
                                            0 == nY,
                                            true));
                                }
                            }
                        }
                    }

                    
                    xCellSequence.realloc(nCellInsert);
                    xBorderSequence.realloc(nBorderInsert);

                    
                    xRetval = xCellSequence;
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, xBorderSequence);
                }

                if(xRetval.hasElements())
                {
                    
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
                
                const Rectangle& rRectangle = rTableObj.GetGeoRect();
                const basegfx::B2DRange aObjectRange(
                    rRectangle.Left(), rRectangle.Top(),
                    rRectangle.Right(), rRectangle.Bottom());

                
                const GeoStat& rGeoStat(rTableObj.GetGeoStat());
                const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
                const double fRotate(rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0);
                const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                    aObjectRange.getWidth(), aObjectRange.getHeight(), fShearX, fRotate,
                    aObjectRange.getMinX(), aObjectRange.getMinY()));

                
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false,
                        aObjectMatrix));

                return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }
        }

        ViewContactOfTableObj::ViewContactOfTableObj(::sdr::table::SdrTableObj& rTableObj)
        :   ViewContactOfSdrObj(rTableObj)
        {
        }

        ViewContactOfTableObj::~ViewContactOfTableObj()
        {
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
