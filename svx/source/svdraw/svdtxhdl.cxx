/*************************************************************************
 *
 *  $RCSfile: svdtxhdl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2001-03-13 13:57:50 $
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

ImpTextPortionHandler::ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj):
    rOutliner(rOutln),
    rTextObj(rTxtObj),
    aPoly(0)
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
    if (bFitToSize) {
        Fraction nX(aAnchorRect.Right()-aAnchorRect.Left(),aTextRect.Right()-aTextRect.Left());
        Fraction nY(aAnchorRect.Bottom()-aAnchorRect.Top(),aTextRect.Bottom()-aTextRect.Top());
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
        pPara=rTextObj.ImpGetEditOutlinerParaObject();
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

        for (nParagraph = 0; nParagraph < nCnt; nParagraph++)
        {
            aPoly = XOutCreatePolygon(aXPP[USHORT(nParagraph)], rXOut.GetOutDev());
            nTextWidth = 0;

            rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FormTextWidthHdl));
            rOutliner.StripPortions();
            rOutliner.SetDrawPortionHdl(LINK(this,ImpTextPortionHandler,FormTextDrawHdl));
            rOutliner.StripPortions();
            rOutliner.SetDrawPortionHdl(Link());

            const Rectangle& rFTBR=rXOut.GetFormTextBoundRect();
            aFormTextBoundRect.Union(rFTBR);
        }
        rXOut.GetOutDev()->SetFont(aFont);
        rOutliner.Clear();
    }
    if (rTextObj.IsTextEditActive()) {
        delete pPara;
    }
}

IMPL_LINK(ImpTextPortionHandler,ConvertHdl,DrawPortionInfo*,pInfo)
{
    // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
    BOOL bIsVertical(rOutliner.IsVertical());
    Point aPos(aFormTextBoundRect.TopLeft() + pInfo->rStartPos);
    if(bIsVertical)
        aPos = aFormTextBoundRect.TopRight() + pInfo->rStartPos;
    Color aColor(pInfo->rFont.GetColor());
    xub_StrLen nCnt = pInfo->rText.Len();
    Point aStartPos(aPos);
    SfxItemSet aAttrSet((SfxItemPool&)(*rTextObj.GetItemPool()));

    long nHochTief=pInfo->rFont.GetEscapement();
    if (nHochTief==0) {
        aVDev.SetFont(pInfo->rFont); // Normalstellung
    } else { // Fuer Hoch-Tiefstellung den Font verkleinern
        long nPercent=pInfo->rFont.GetPropr();
        if (nPercent!=100) {
            Font aFont(pInfo->rFont);
            Size aSize(aFont.GetSize());
            aSize.Height()=(aSize.Height() * nPercent +50) / 100;
            aSize.Width()=(aSize.Width() * nPercent +50) / 100;
            aFont.SetSize(aSize);
            aVDev.SetFont(aFont);
        }
        FASTBOOL bNeg=nHochTief<0;
        if (bNeg) nHochTief=-nHochTief;
        nHochTief=(nHochTief * pInfo->rFont.GetSize().Height() +50) /100;
        if (bNeg) nHochTief=-nHochTief;
    }
    FontMetric aFontMetric(aVDev.GetFontMetric());
    if(bIsVertical)
        aPos.X() += nHochTief;
    else
        aPos.Y()-=aFontMetric.GetAscent()+nHochTief;

    if (pInfo->rFont.IsOutline()) {
        aAttrSet.Put(XLineColorItem(String(),aColor));
        aAttrSet.Put(XLineStyleItem(XLINE_SOLID));
        aAttrSet.Put(XLineWidthItem(0));
        aAttrSet.Put(XFillStyleItem(XFILL_NONE));
    } else {
        aAttrSet.Put(XFillColorItem(String(),aColor));
        aAttrSet.Put(XLineStyleItem(XLINE_NONE));
        aAttrSet.Put(XFillStyleItem(XFILL_SOLID));
    }

    for(xub_StrLen i = 0; i<nCnt; i++)
    {
        // UNICODE: BYTE aCharByte = (BYTE)((pInfo->rText)[i]);
        // since XOutGetCharOutline() is not yet changed, make a short
        // term solution here
        sal_Unicode aUnicode = (pInfo->rText).GetChar(i);
        XPolyPolygon aXPP(XOutGetCharOutline(aUnicode, aVDev));

// offset in Y(!) for testing make rough correction here
if(bIsVertical)
{
    Rectangle aPolyRect(aXPP.GetBoundRect());
    aXPP.Move(0, -aPolyRect.Top());
}

        if(aXPP.Count())
        {
            aXPP.Move(aPos.X(), aPos.Y());
// offset in Y(!)           aPolyRect = aXPP.GetBoundRect();

            // aFormTextBoundRect enthaelt den Ausgabebereich des Textobjekts
            // #35825# Rotieren erst nach Resize (wg. FitToSize)
            //RotateXPoly(aXPP,aFormTextBoundRect.TopLeft(),rTextObj.aGeo.nSin,rTextObj.aGeo.nCos);

            SdrObject* pObj = rTextObj.ImpConvertMakeObj(aXPP, TRUE, !bToPoly, TRUE);

            pObj->SetItemSet(aAttrSet);
            pGroup->GetSubList()->InsertObject(pObj);
        }

        if(bIsVertical)
            aPos.Y() = aStartPos.Y() + pInfo->pDXArray[i]; // - nSlant;
        else
            aPos.X() = aStartPos.X() + pInfo->pDXArray[i]; // - nSlant;
    }

    long nLineLen = (bIsVertical) ? aPos.Y() - aStartPos.Y() : aPos.X() - aStartPos.X();
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
        long y=-(long)(aFontMetric.GetAscent()-aFontMetric.GetLeading()+1)/3;
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

IMPL_LINK(ImpTextPortionHandler,FormTextWidthHdl,DrawPortionInfo*,pInfo)
{
    if(pInfo->nPara == nParagraph && pInfo->rText.Len())
    {
        // negative Gesamtlaenge fuer ersten Aufruf von DrawFormText
        nTextWidth -= pInfo->pDXArray[pInfo->rText.Len() - 1];
    }
    return 0;
}

IMPL_LINK(ImpTextPortionHandler,FormTextDrawHdl,DrawPortionInfo*,pInfo)
{
    if (pInfo->nPara==nParagraph) {
        nTextWidth=pXOut->DrawFormText(pInfo->rText,aPoly,pInfo->rFont,
                                       nTextWidth,bToLastPoint,bDraw,pInfo->pDXArray);
    }
    return 0;
}
