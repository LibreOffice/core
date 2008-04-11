/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdtxhdl.cxx,v $
 * $Revision: 1.27 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svdtxhdl.hxx"
#include <limits.h>
#include "svditext.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotext.hxx>
#include "svdxout.hxx"
#include <svx/svdoutl.hxx>
#include <svx/outliner.hxx>
#include <svx/outlobj.hxx>
#include <svx/svxfont.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xflclit.hxx>
#include <vcl/metric.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif
#include "unolingu.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpTextPortionHandler::ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj):
    rOutliner(rOutln),
    rTextObj(rTxtObj),
    mpRecordPortions(0L)
{
    pModel=rTextObj.GetModel();
    MapMode aMap=aVDev.GetMapMode();

    aMap.SetMapUnit(pModel->GetScaleUnit());
    aMap.SetScaleX(pModel->GetScaleFraction());
    aMap.SetScaleY(pModel->GetScaleFraction());
    aVDev.SetMapMode(aMap);
}

void ImpTextPortionHandler::ConvertToPathObj(SdrObjGroup& rGroup, FASTBOOL bPoly)
{
    bToPoly=bPoly;
    pGroup=&rGroup;

    Rectangle aAnchorRect;
    Rectangle aTextRect;
    SdrFitToSizeType eFit=rTextObj.GetFitToSize();
    FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    // Bei TakeTextRect wird u.a. auch der Text in
    // den Outliner gesteckt
    rTextObj.TakeTextRect(rOutliner,aTextRect,FALSE,&aAnchorRect);
    aFormTextBoundRect=aTextRect; // Missbrauch von FormTextBoundRect
    if (bFitToSize) aFormTextBoundRect=aAnchorRect;
    rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,ConvertHdl));
    rOutliner.StripPortions();
    rOutliner.SetDrawPortionHdl(Link());
    rOutliner.Clear();

    if(bFitToSize)
    {
        Fraction nX(aAnchorRect.Right()-aAnchorRect.Left(),aTextRect.Right()-aTextRect.Left());
        Fraction nY(aAnchorRect.Bottom()-aAnchorRect.Top(),aTextRect.Bottom()-aTextRect.Top());

        // #95395# scale from top-right when vertical text
        if(rOutliner.IsVertical())
            pGroup->NbcResize(aAnchorRect.TopRight(),nX,nY);
        else
            pGroup->NbcResize(aAnchorRect.TopLeft(),nX,nY);
    }
    if (rTextObj.aGeo.nDrehWink!=0) { // #35825# Rotieren erst nach Resize (wg. FitToSize)
        pGroup->NbcRotate(aFormTextBoundRect.TopLeft(),rTextObj.aGeo.nDrehWink,rTextObj.aGeo.nSin,rTextObj.aGeo.nCos);
    }
}

void ImpTextPortionHandler::DrawTextToPath(XOutputDevice& rXOut, FASTBOOL bDrawEffect)
{
    aFormTextBoundRect=Rectangle();
    //const Rectangle& rBR = rTextObj.GetSnapRect();

    bDraw = bDrawEffect;

    OutlinerParaObject* pPara=rTextObj.GetOutlinerParaObject();
    if (rTextObj.IsTextEditActive()) {
        pPara=rTextObj.GetEditOutlinerParaObject();
    }
    if (pPara!=NULL) {
        basegfx::B2DPolyPolygon aContourPolyPolygon(rTextObj.TakeXorPoly(FALSE));
        pXOut=&rXOut;
        Font aFont(rXOut.GetOutDev()->GetFont());
        rOutliner.Clear();
        rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
        rOutliner.SetText(*pPara);
        sal_uInt32 nCnt(Min(aContourPolyPolygon.count(),
            static_cast<sal_uInt32>(rOutliner.GetParagraphCount())));

        if ( nCnt == 1 )    bToLastPoint = TRUE;
        else                bToLastPoint = FALSE;

        // #101498# completely different methodology needed here for making this BIDI-able
        // iterate over paragraphs and Polygons, thus each paragraph will be put to
        // one Polygon
        const sal_uInt32 nSavedLayoutMode(rXOut.GetOutDev()->GetLayoutMode());
        sal_uInt32 nLayoutMode(nSavedLayoutMode);

        if(TRUE)
        {
            nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG);
        }
        else
        {
            nLayoutMode &= ~(TEXT_LAYOUT_BIDI_RTL);
            nLayoutMode |= ~(TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG);
        }

        rXOut.GetOutDev()->SetLayoutMode(nLayoutMode);

        for(nParagraph = 0; nParagraph < nCnt; nParagraph++)
        {
            basegfx::B2DPolygon aContourPolygon(aContourPolyPolygon.getB2DPolygon(nParagraph));

            if(aContourPolygon.areControlPointsUsed())
            {
                aContourPolygon = basegfx::tools::adaptiveSubdivideByAngle(aContourPolygon);
            }

            rOutliner.SetDrawPortionHdl(LINK(this, ImpTextPortionHandler, FormTextRecordPortionHdl));
            rOutliner.StripPortions();

            DrawFormTextRecordPortions(Polygon(aContourPolygon));
            ClearFormTextRecordPortions();

            const Rectangle& rFTBR = rXOut.GetFormTextBoundRect();
            aFormTextBoundRect.Union(rFTBR);
        }

        rXOut.GetOutDev()->SetLayoutMode(nSavedLayoutMode);
        rXOut.GetOutDev()->SetFont(aFont);
        rOutliner.Clear();
    }
    if (rTextObj.IsTextEditActive()) {
        delete pPara;
    }
}

// #101498# Record and sort all portions
IMPL_LINK(ImpTextPortionHandler, FormTextRecordPortionHdl, DrawPortionInfo*, pInfo)
{
    if(pInfo->nPara == nParagraph)
    {
        SortedAddFormTextRecordPortion(pInfo);
    }

    return 0;
}

// #101498# Helper class to remember text portions in sorted manner
class ImpRecordPortion
{
public:
    Point                       maPosition;
    String                      maText;
    xub_StrLen                  mnTextStart;
    xub_StrLen                  mnTextLength;
    sal_uInt16                  mnPara;
    xub_StrLen                  mnIndex;

    // #102819# Here SvxFont needs to be used instead of Font
    SvxFont                     maFont;

    sal_Int32*                  mpDXArray;
    sal_uInt8                   mnBiDiLevel;

    ImpRecordPortion(DrawPortionInfo* pInfo);
    ~ImpRecordPortion();

    sal_uInt8 GetBiDiLevel() const { return mnBiDiLevel; }
};

ImpRecordPortion::ImpRecordPortion(DrawPortionInfo* pInfo)
:   maPosition(pInfo->rStartPos),
    maText(pInfo->rText),
    mnTextStart((xub_StrLen)pInfo->nTextStart),
    mnTextLength((xub_StrLen)pInfo->nTextLen),
    mnPara(pInfo->nPara),
    mnIndex(pInfo->nIndex),
    maFont(pInfo->rFont),
    mpDXArray(NULL),
    mnBiDiLevel(pInfo->GetBiDiLevel())
{
    if(pInfo->pDXArray)
    {
        mpDXArray = new sal_Int32[pInfo->nTextLen];

        for(sal_uInt32 a(0L); a < pInfo->nTextLen; a++)
        {
            mpDXArray[a] = pInfo->pDXArray[a];
        }
    }
}

ImpRecordPortion::~ImpRecordPortion()
{
    if(mpDXArray)
    {
        delete[] mpDXArray;
    }
}

// #101498# List classes for recording portions
DECLARE_LIST(ImpRecordPortionList, ImpRecordPortion*)
DECLARE_LIST(ImpRecordPortionListList, ImpRecordPortionList*)

// #101498# Draw recorded formtext along Poly
void ImpTextPortionHandler::DrawFormTextRecordPortions(Polygon aPoly)
{
    sal_Int32 nTextWidth = -((sal_Int32)GetFormTextPortionsLength(pXOut->GetOutDev()));

    ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
    if(pListList)
    {
        for(sal_uInt32 a(0L); a < pListList->Count(); a++)
        {
            ImpRecordPortionList* pList = pListList->GetObject(a);

            for(sal_uInt32 b(0L); b < pList->Count(); b++)
            {
                ImpRecordPortion* pPortion = pList->GetObject(b);

                DrawPortionInfo aNewInfo(
                    pPortion->maPosition,
                    pPortion->maText,
                    pPortion->mnTextStart,
                    pPortion->mnTextLength,
                    pPortion->maFont,
                    pPortion->mnPara,
                    pPortion->mnIndex,
                    pPortion->mpDXArray,
                    pPortion->mnBiDiLevel);

                nTextWidth = pXOut->DrawFormText(&aNewInfo, aPoly, nTextWidth, bToLastPoint, bDraw);
            }
        }
    }
}

// #101498# Insert new portion sorted
void ImpTextPortionHandler::SortedAddFormTextRecordPortion(DrawPortionInfo* pInfo)
{
    // get ListList and create on demand
    ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
    if(!pListList)
    {
        mpRecordPortions = (void*)(new ImpRecordPortionListList(2, 2));
        pListList = (ImpRecordPortionListList*)mpRecordPortions;
    }

    // create new portion
    ImpRecordPortion* pNewPortion = new ImpRecordPortion(pInfo);

    // look for the list where to insert new portion
    ImpRecordPortionList* pList = 0L;

    for(sal_uInt32 nListListIndex(0L); !pList && nListListIndex < pListList->Count(); nListListIndex++)
    {
        ImpRecordPortionList* pTmpList = pListList->GetObject(nListListIndex);

        if(pTmpList->GetObject(0)->maPosition.Y() == pNewPortion->maPosition.Y())
            pList = pTmpList;
    }

    if(!pList)
    {
        // no list for that Y-Coordinate yet, create a new one.
        pList = new ImpRecordPortionList(8, 8);
        pList->Insert(pNewPortion, LIST_APPEND);
        pListList->Insert(pList, LIST_APPEND);
    }
    else
    {
        // found a list for that for that Y-Coordinate, sort in
        sal_uInt32 nInsertInd(0L);

        while(nInsertInd < pList->Count()
            && pList->GetObject(nInsertInd)->maPosition.X() < pNewPortion->maPosition.X())
        {
            nInsertInd++;
        }

        if(nInsertInd == pList->Count())
            nInsertInd = LIST_APPEND;

        pList->Insert(pNewPortion, nInsertInd);
    }
}

// #101498# Calculate complete length of FormTextPortions
sal_uInt32 ImpTextPortionHandler::GetFormTextPortionsLength(OutputDevice* pOut)
{
    sal_uInt32 nRetval(0L);

    ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
    if(pListList)
    {
        for(sal_uInt32 a(0L); a < pListList->Count(); a++)
        {
            ImpRecordPortionList* pList = pListList->GetObject(a);

            for(sal_uInt32 b(0L); b < pList->Count(); b++)
            {
                ImpRecordPortion* pPortion = pList->GetObject(b);

                if(pPortion->mpDXArray)
                {
                    if(pPortion->maFont.IsVertical() && pOut)
                        nRetval += pOut->GetTextHeight() * pPortion->mnTextLength;
                    else
                        nRetval += pPortion->mpDXArray[pPortion->mnTextLength - 1];
                }
            }
        }
    }

    return nRetval;
}

// #101498# Cleanup recorded portions
void ImpTextPortionHandler::ClearFormTextRecordPortions()
{
    ImpRecordPortionListList* pListList = (ImpRecordPortionListList*)mpRecordPortions;
    if(pListList)
    {
        for(sal_uInt32 a(0L); a < pListList->Count(); a++)
        {
            ImpRecordPortionList* pList = pListList->GetObject(a);

            for(sal_uInt32 b(0L); b < pList->Count(); b++)
            {
                delete pList->GetObject(b);
            }

            delete pList;
        }

        delete pListList;
        mpRecordPortions = (void*)0L;
    }
}

IMPL_LINK(ImpTextPortionHandler,ConvertHdl,DrawPortionInfo*,pInfo)
{
    // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
    BOOL bIsVertical(rOutliner.IsVertical());
    Point aPos2(aFormTextBoundRect.TopLeft() + pInfo->rStartPos);
    Color aColor(pInfo->rFont.GetColor());

    if(bIsVertical)
        aPos2 = aFormTextBoundRect.TopRight() + pInfo->rStartPos;

    Point aStartPos(aPos2);
    SfxItemSet aAttrSet((SfxItemPool&)(*rTextObj.GetObjectItemPool()));
    long nHochTief(pInfo->rFont.GetEscapement());
    FontMetric aFontMetric(aVDev.GetFontMetric());
    sal_Int32 nLineLen(0L);

    Font aFont( pInfo->rFont );

    if( nHochTief)
    {
        // shrink the font for sub-/superscripting
        long nPercent(pInfo->rFont.GetPropr());

        if(nPercent != 100)
        {
            Size aSize(aFont.GetSize());
            aSize.Height() = (aSize.Height() * nPercent +50) / 100;
            aSize.Width() = (aSize.Width() * nPercent +50) / 100;
            aFont.SetSize(aSize);
        }

        sal_Bool bNeg(nHochTief < 0);

        if(bNeg)
            nHochTief = -nHochTief;

        nHochTief = (nHochTief * pInfo->rFont.GetSize().Height() +50) /100;

        if(bNeg)
            nHochTief = -nHochTief;
    }

aFont.SetOrientation( 0 );
aVDev.SetFont( aFont );

    if(bIsVertical)
        // #83068#
        aPos2.X() += aFontMetric.GetAscent() + nHochTief;
    else
        aPos2.Y() -= aFontMetric.GetAscent() + nHochTief;

    if (pInfo->rFont.IsOutline())
    {
        aAttrSet.Put(XLineColorItem(String(),aColor));
        aAttrSet.Put(XLineStyleItem(XLINE_SOLID));
        aAttrSet.Put(XLineWidthItem(0));
        aAttrSet.Put(XFillStyleItem(XFILL_NONE));
    }
    else
    {
        aAttrSet.Put(XFillColorItem(String(),aColor));
        aAttrSet.Put(XLineStyleItem(XLINE_NONE));
        aAttrSet.Put(XFillStyleItem(XFILL_SOLID));
    }

    // #100318# convert in a single step
    // #101499# Use GetTextOutlines and a PolyPolyVector now
    PolyPolyVector aPolyPolyVector;
    if(aVDev.GetTextOutlines(aPolyPolyVector, pInfo->rText, pInfo->nTextStart, pInfo->nTextStart, pInfo->nTextLen)
        && aPolyPolyVector.size())
    {
        for(sal_uInt32 a(0); a < aPolyPolyVector.size(); a++)
        {
            PolyPolygon aPolyPoly(aPolyPolyVector[a]);

            if(aPolyPoly.Count() > 0 && aPolyPoly[0].GetSize() > 0)
            {
                basegfx::B2DPolyPolygon aPolyPolygon(aPolyPoly.getB2DPolyPolygon());
                basegfx::B2DHomMatrix aMatrix;

                // rotate 270 degree if vertical since result is unrotated
                if(pInfo->rFont.GetOrientation())
                {
                    double fAngle(F_PI * (pInfo->rFont.GetOrientation() % 3600) / 1800.0);
                    aMatrix.rotate(fAngle);
                }

                // result is baseline oriented, thus move one line height, too
                if(bIsVertical)
                {
                    aMatrix.translate(-aFontMetric.GetAscent(), 0.0);
                }
                else
                {
                    aMatrix.translate(0.0, aFontMetric.GetAscent());
                }

                // move to output coordinates
                aMatrix.translate(aPos2.X(), aPos2.Y());

                // transform
                aPolyPolygon.transform(aMatrix);

                // create object
                SdrObject* pObj = rTextObj.ImpConvertMakeObj(aPolyPolygon, sal_True, !bToPoly, sal_True);
                pObj->SetMergedItemSet(aAttrSet);
                pGroup->GetSubList()->InsertObject(pObj);
            }
        }

        nLineLen = pInfo->pDXArray[pInfo->nTextLen - 1];
    }

    FontUnderline eUndl=pInfo->rFont.GetUnderline();
    FontStrikeout eStrk=pInfo->rFont.GetStrikeout();
    if (eUndl!=UNDERLINE_NONE) {
        FASTBOOL bDouble=eUndl==UNDERLINE_DOUBLE;
        long nDescent=aFontMetric.GetDescent();
        long nAscend=aFontMetric.GetAscent();
        long nDick=nDescent / (bDouble ? 5 : 3);
        long nDist=(nDescent-nDick*2)/3; // Linienabstand bei doppelt
        basegfx::B2DPolyPolygon aPolyPolygon;

        if (eUndl!=UNDERLINE_DOTTED) {
            Point aPoint(0,0);
            XPolygon aXP(Rectangle(aPoint,bIsVertical ? Point(nDick,nLineLen) : Point(nLineLen,nDick)));
            if(bIsVertical)
                aXP.Move(nAscend-nDist,0);
            aPolyPolygon.append(aXP.getB2DPolygon());
            if (bDouble) {
                if(bIsVertical)
                    aXP.Move(-(nDick+nDist),0);
                else
                    aXP.Move(0,nDick+nDist);
                aPolyPolygon.append(aXP.getB2DPolygon());
            }
        } else {
            Point aPoint(0,0);
            XPolygon aXP(Rectangle(aPoint,Point(nDick,nDick)));
            long n=0;
            while (n<=nLineLen) {
                if (n+nDick>nLineLen) { // ler letzte Dot ggf. etwas schmaler
                    aXP=XPolygon(Rectangle(
                        bIsVertical ? Point(0,n) : Point(n,0),
                        bIsVertical ? Point(nDick,nLineLen) : Point(nLineLen,nDick)));
                }
                aPolyPolygon.append(aXP.getB2DPolygon());
                if(bIsVertical)
                    aXP.Move(0,2*nDick);
                else
                    aXP.Move(2*nDick,0);
                n+=2*nDick;
            }
        }

        long y=nDescent-nDick; // y-Position der Striche zur Baseline bestimmen
        if (bDouble) y-=nDick+nDist;
        y=(y+1)/2;

        basegfx::B2DHomMatrix aMatrix;

        if(bIsVertical)
        {
            aMatrix.translate(aStartPos.X() - (y - nHochTief), aStartPos.Y());
        }
        else
        {
            aMatrix.translate(aStartPos.X(), aStartPos.Y() + y - nHochTief);
        }

        aPolyPolygon.transform(aMatrix);

        // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
        SdrObject* pObj=rTextObj.ImpConvertMakeObj(aPolyPolygon, sal_True, !bToPoly, sal_True);
        pObj->SetMergedItemSet(aAttrSet);
        pGroup->GetSubList()->InsertObject(pObj);
    }
    if (eStrk!=STRIKEOUT_NONE) {
        FASTBOOL bDouble=eStrk==STRIKEOUT_DOUBLE;
        long nDescent=aFontMetric.GetDescent();
        //long nAscend=aFontMetric.GetAscent();
        long nDick=nDescent / (bDouble ? 5 : 3);
        long nDist=(nDescent-nDick*2)/3; // Linienabstand bei doppelt
        basegfx::B2DPolyPolygon aPolyPolygon;

        const Point aPoint(0,0);
        const Rectangle aRect(aPoint,bIsVertical ? Point(nDick,nLineLen) : Point(nLineLen,nDick));
        const basegfx::B2DRange aRectRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
        basegfx::B2DPolygon aPolyFromRect(basegfx::tools::createPolygonFromRect(aRectRange));
        aPolyPolygon.append(aPolyFromRect);

        if (bDouble)
        {
            basegfx::B2DHomMatrix aMatrix;

            if(bIsVertical)
                aMatrix.translate(-(nDick+nDist), 0.0);
            else
                aMatrix.translate(0.0, nDick+nDist);

            aPolyFromRect.transform(aMatrix);
            aPolyPolygon.append(aPolyFromRect);
        }

        // y-Position der Striche zur Baseline bestimmen
        long y=-(long)(aFontMetric.GetAscent()-aFontMetric.GetIntLeading()+1)/3;
        if (!bDouble) y-=(nDick+1)/2;
        else y-=nDick+(nDist+1)/2;

        basegfx::B2DHomMatrix aMatrix;

        if(bIsVertical)
        {
            aMatrix.translate(aStartPos.X() - (y - nHochTief), aStartPos.Y());
        }
        else
        {
            aMatrix.translate(aStartPos.X(), aStartPos.Y() + y - nHochTief);
        }

        aPolyPolygon.transform(aMatrix);

        // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
        SdrObject* pObj=rTextObj.ImpConvertMakeObj(aPolyPolygon, sal_True, !bToPoly, sal_True);
        pObj->SetMergedItemSet(aAttrSet);
        pGroup->GetSubList()->InsertObject(pObj);
    }

    return 0;
}

void ImpTextPortionHandler::DrawFitText(XOutputDevice& rXOut, const Point& rPos, const Fraction& rXFact)
{
    pXOut=&rXOut;
    aPos=rPos;
    aXFact=rXFact;
    rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FitTextDrawHdl));
    rOutliner.StripPortions();
    rOutliner.SetDrawPortionHdl(Link());
}

IMPL_LINK(ImpTextPortionHandler,FitTextDrawHdl,DrawPortionInfo*,EMPTYARG)
{
    return 0;
}

