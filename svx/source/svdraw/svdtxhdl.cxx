/*************************************************************************
 *
 *  $RCSfile: svdtxhdl.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:48:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdtxhdl.hxx"
#include <limits.h>
#include "svditext.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdogrp.hxx"
#include "svdotext.hxx"
#include "svdxout.hxx"
#include "svdoutl.hxx"

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

#ifndef _OUTLOBJ_HXX //autogen
#include <outlobj.hxx>
#endif

#ifndef _SVX_SVXFONT_HXX //autogen
#include "svxfont.hxx"
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include "xlnclit.hxx"
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include "xlnwtit.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include "xflclit.hxx"
#endif

#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// #101499#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HDL_
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#endif

#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpTextPortionHandler::ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj):
    rOutliner(rOutln),
    rTextObj(rTxtObj),
    // #101498# aPoly(0)
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

void ImpTextPortionHandler::DrawTextToPath(ExtOutputDevice& rXOut, FASTBOOL bDrawEffect)
{
    aFormTextBoundRect=Rectangle();
    const Rectangle& rBR = rTextObj.GetSnapRect();

    bDraw = bDrawEffect;

    OutlinerParaObject* pPara=rTextObj.GetOutlinerParaObject();
    if (rTextObj.IsTextEditActive()) {
        pPara=rTextObj.GetEditOutlinerParaObject();
    }
    if (pPara!=NULL) {
        XPolyPolygon aXPP;
        //rTextObj.TakeContour(aXPP);
        rTextObj.TakeXorPoly(aXPP,FALSE);
        pXOut=&rXOut;
        Font aFont(rXOut.GetOutDev()->GetFont());
        rOutliner.Clear();
        rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
        rOutliner.SetText(*pPara);
        USHORT nCnt = Min(aXPP.Count(), (USHORT) rOutliner.GetParagraphCount());

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
            Polygon aPoly = XOutCreatePolygon(aXPP[sal_uInt16(nParagraph)], rXOut.GetOutDev());

            rOutliner.SetDrawPortionHdl(LINK(this, ImpTextPortionHandler, FormTextRecordPortionHdl));
            rOutliner.StripPortions();

            DrawFormTextRecordPortions(aPoly);
            ClearFormTextRecordPortions();

            const Rectangle& rFTBR = rXOut.GetFormTextBoundRect();
            aFormTextBoundRect.Union(rFTBR);
        }

        rXOut.GetOutDev()->SetLayoutMode(nSavedLayoutMode);

        //for (nParagraph = 0; nParagraph < nCnt; nParagraph++)
        //{
        //  aPoly = XOutCreatePolygon(aXPP[USHORT(nParagraph)], rXOut.GetOutDev());
        //  nTextWidth = 0;
        //
        //  rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FormTextWidthHdl));
        //  rOutliner.StripPortions();
        //  rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FormTextDrawHdl));
        //  rOutliner.StripPortions();
        //  rOutliner.SetDrawPortionHdl(Link());
        //
        //  const Rectangle& rFTBR=rXOut.GetFormTextBoundRect();
        //  aFormTextBoundRect.Union(rFTBR);
        //}

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
DECLARE_LIST(ImpRecordPortionList, ImpRecordPortion*);
DECLARE_LIST(ImpRecordPortionListList, ImpRecordPortionList*);

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
    Point aPos(aFormTextBoundRect.TopLeft() + pInfo->rStartPos);
    Color aColor(pInfo->rFont.GetColor());

    if(bIsVertical)
        aPos = aFormTextBoundRect.TopRight() + pInfo->rStartPos;

    // #100318# new for XOutGetCharOutline
    // xub_StrLen nCnt = pInfo->nTextLen;

    Point aStartPos(aPos);
    SfxItemSet aAttrSet((SfxItemPool&)(*rTextObj.GetItemPool()));
    long nHochTief(pInfo->rFont.GetEscapement());
    FontMetric aFontMetric(aVDev.GetFontMetric());
    sal_Int32 nLineLen(0L);

    if(!nHochTief)
    {
        // Normalstellung
        aVDev.SetFont(pInfo->rFont);
    }
    else
    {
        // Fuer Hoch-Tiefstellung den Font verkleinern
        long nPercent(pInfo->rFont.GetPropr());

        if(nPercent != 100)
        {
            Font aFont(pInfo->rFont);
            Size aSize(aFont.GetSize());

            aSize.Height() = (aSize.Height() * nPercent +50) / 100;
            aSize.Width() = (aSize.Width() * nPercent +50) / 100;
            aFont.SetSize(aSize);
            aVDev.SetFont(aFont);
        }

        sal_Bool bNeg(nHochTief < 0);

        if(bNeg)
            nHochTief = -nHochTief;

        nHochTief = (nHochTief * pInfo->rFont.GetSize().Height() +50) /100;

        if(bNeg)
            nHochTief = -nHochTief;
    }

    if(bIsVertical)
        // #83068#
        aPos.X() += aFontMetric.GetAscent() + nHochTief;
    else
        aPos.Y() -= aFontMetric.GetAscent() + nHochTief;

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

                XPolyPolygon aXPP(aPolyPoly);

                // rotate 270 degree if vertical since result is unrotated
                if(bIsVertical)
                    aXPP.Rotate(Point(), 2700);

                // result is baseline oriented, thus move one line height, too
                if(bIsVertical)
                    aXPP.Move(-aFontMetric.GetAscent(), 0);
                else
                    aXPP.Move(0, aFontMetric.GetAscent());

                // move to output coordinates
                aXPP.Move(aPos.X(), aPos.Y());
                SdrObject* pObj = rTextObj.ImpConvertMakeObj(aXPP, TRUE, !bToPoly, TRUE);

                pObj->SetItemSet(aAttrSet);
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

        XPolyPolygon aXPP;
        if (eUndl!=UNDERLINE_DOTTED) {
            Point aPoint(0,0);
            XPolygon aXP(Rectangle(aPoint,bIsVertical ? Point(nDick,nLineLen) : Point(nLineLen,nDick)));
            if(bIsVertical)
                aXP.Move(nAscend-nDist,0);
            aXPP.Insert(aXP);
            if (bDouble) {
                if(bIsVertical)
                    aXP.Move(-(nDick+nDist),0);
                else
                    aXP.Move(0,nDick+nDist);
                aXPP.Insert(aXP);
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
                aXPP.Insert(aXP);
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

        if(bIsVertical)
            aXPP.Move(aStartPos.X()-(y-nHochTief),aStartPos.Y());
        else
            aXPP.Move(aStartPos.X(),aStartPos.Y()+y-nHochTief);
        // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
        // #35825# Rotieren erst nach Resize (wg. FitToSize)
        //RotateXPoly(aXPP,aFormTextBoundRect.TopLeft(),rTextObj.aGeo.nSin,rTextObj.aGeo.nCos);
        SdrObject* pObj=rTextObj.ImpConvertMakeObj(aXPP,TRUE,!bToPoly, TRUE);
        pObj->SetItemSet(aAttrSet);
        pGroup->GetSubList()->InsertObject(pObj);
    }
    if (eStrk!=STRIKEOUT_NONE) {
        FASTBOOL bDouble=eStrk==STRIKEOUT_DOUBLE;
        long nDescent=aFontMetric.GetDescent();
        long nAscend=aFontMetric.GetAscent();
        long nDick=nDescent / (bDouble ? 5 : 3);
        long nDist=(nDescent-nDick*2)/3; // Linienabstand bei doppelt

        XPolyPolygon aXPP;
        Point aPoint(0,0);
        XPolygon aXP(Rectangle(aPoint,bIsVertical ? Point(nDick,nLineLen) : Point(nLineLen,nDick)));
        aXPP.Insert(aXP);
        if (bDouble) {
            if(bIsVertical)
                aXP.Move(-(nDick+nDist),0);
            else
                aXP.Move(0,nDick+nDist);
            aXPP.Insert(aXP);
        }

        // y-Position der Striche zur Baseline bestimmen
        long y=-(long)(aFontMetric.GetAscent()-aFontMetric.GetIntLeading()+1)/3;
        if (!bDouble) y-=(nDick+1)/2;
        else y-=nDick+(nDist+1)/2;

        if(bIsVertical)
            aXPP.Move(aStartPos.X()-(y-nHochTief),aStartPos.Y());
        else
            aXPP.Move(aStartPos.X(),aStartPos.Y() +y-nHochTief);
        // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
        // #35825# Rotieren erst nach Resize (wg. FitToSize)
        //RotateXPoly(aXPP,aFormTextBoundRect.TopLeft(),rTextObj.aGeo.nSin,rTextObj.aGeo.nCos);
        SdrObject* pObj=rTextObj.ImpConvertMakeObj(aXPP,TRUE,!bToPoly, TRUE);
        pObj->SetItemSet(aAttrSet);
        pGroup->GetSubList()->InsertObject(pObj);
    }
    return 0;
}

void ImpTextPortionHandler::DrawFitText(ExtOutputDevice& rXOut, const Point& rPos, const Fraction& rXFact)
{
    pXOut=&rXOut;
    aPos=rPos;
    aXFact=rXFact;
    rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FitTextDrawHdl));
    rOutliner.StripPortions();
    rOutliner.SetDrawPortionHdl(Link());
}

IMPL_LINK(ImpTextPortionHandler,FitTextDrawHdl,DrawPortionInfo*,pInfo)
{
    return 0;
}

//IMPL_LINK(ImpTextPortionHandler, FormTextWidthHdl, DrawPortionInfo*, pInfo)
//{
//  // #101498# change calculation of nTextWidth
//  if(pInfo->nPara == nParagraph && pInfo->nTextLen)
//  {
//      // negative value is used because of the interface of
//      // XOutputDevice::ImpDrawFormText(...), please look there
//      // for more info.
//      nTextWidth -= pInfo->pDXArray[pInfo->nTextLen - 1];
//  }
//
//  return 0;
//}

//IMPL_LINK(ImpTextPortionHandler, FormTextDrawHdl, DrawPortionInfo*, pInfo)
//{
//  // #101498# Implementation of DrawFormText needs to be updated, too.
//  if(pInfo->nPara == nParagraph)
//  {
//      nTextWidth = pXOut->DrawFormText(pInfo, aPoly, nTextWidth, bToLastPoint, bDraw);
//          //pInfo->rText, aPoly, pInfo->rFont, nTextWidth,
//          //bToLastPoint, bDraw, pInfo->pDXArray);
//  }
//
//  return 0;
//}
