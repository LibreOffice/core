/*************************************************************************
 *
 *  $RCSfile: svdotxed.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#include "svdotext.hxx"
#include "svditext.hxx"
#include "svdmodel.hxx" // fuer GetMaxObjSize
#include "svdoutl.hxx"

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <editstat.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
//  TextEdit
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrTextObj::HasTextEdit() const
{
    // lt. Anweisung von MB duerfen gelinkte Textobjekte nun doch
    // geaendert werden (kein automatisches Reload)
    return TRUE;
}

FASTBOOL SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
{
    if (pEdtOutl!=NULL) return FALSE; // Textedit laeuft evtl. schon an einer anderen View!
    pEdtOutl=&rOutl;

    USHORT nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
    if ( !IsOutlText() )
        nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
    rOutl.Init( nOutlinerMode );
    rOutl.SetRefDevice( pModel->GetRefDevice() );

    SdrFitToSizeType eFit=GetFitToSize();
    FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    FASTBOOL bContourFrame=IsContourTextFrame();
    ImpSetTextEditParams();

    if (!bContourFrame) {
        ULONG nStat=rOutl.GetControlWord();
        nStat|=EE_CNTRL_AUTOPAGESIZE;
        if (bFitToSize) nStat|=EE_CNTRL_STRETCHING; else nStat&=~EE_CNTRL_STRETCHING;
        rOutl.SetControlWord(nStat);
    }

    if (pOutlinerParaObject!=NULL) {
        rOutl.SetText(*pOutlinerParaObject);
    }
    // ggf. Rahmenattribute am 1. (neuen) Absatz des Outliners setzen
    if (rOutl.GetParagraphCount()==1) { // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
        XubString aStr( rOutl.GetText( rOutl.GetParagraph( 0 ) ) );

        if(!aStr.Len())
        {
            // Aha, steht nix drin!
            // damit sich der Outliner initiallisiert
            rOutl.SetText( String(), rOutl.GetParagraph( 0 ) );

            if(pStyleSheet)
                rOutl.SetStyleSheet( 0, pStyleSheet );

            // Beim setzen der harten Attribute an den ersten Absatz muss
            // der Parent pOutlAttr (=die Vorlage) temporaer entfernt
            // werden, da sonst bei SetParaAttribs() auch alle in diesem
            // Parent enthaltenen Items hart am Absatz attributiert werden.
            // -> BugID 22467
            const SfxItemSet* pTmpSet=&pOutlAttr->GetItemSet();
            const SfxItemSet* pParentMerk=pTmpSet->GetParent();
            ((SfxItemSet*)pTmpSet)->SetParent(NULL);
            rOutl.SetParaAttribs(0,*pTmpSet);
            ((SfxItemSet*)pTmpSet)->SetParent(pParentMerk);
        }
    }
    if (bFitToSize) {
        Rectangle aAnchorRect;
        Rectangle aTextRect;
        TakeTextRect(rOutl, aTextRect, FALSE, &aAnchorRect, FALSE);
        Fraction aFitXKorreg(1,1);
        ImpSetCharStretching(rOutl,aTextRect,aAnchorRect,aFitXKorreg);
    }

    if (pOutlinerParaObject!=NULL && (aGeo.nDrehWink!=0 || IsFontwork())) {
        SendRepaintBroadcast();
    }
    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();
    return TRUE;
}

void SdrTextObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    SdrFitToSizeType eFit=GetFitToSize();
    FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    Size aPaperMin,aPaperMax;
    Rectangle aViewInit;
    TakeTextAnchorRect(aViewInit);
    if (aGeo.nDrehWink!=0) {
        Point aCenter(aViewInit.Center());
        aCenter-=aViewInit.TopLeft();
        Point aCenter0(aCenter);
        RotatePoint(aCenter,Point(),aGeo.nSin,aGeo.nCos);
        aCenter-=aCenter0;
        aViewInit.Move(aCenter.X(),aCenter.Y());
    }
    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.Width()--; aAnkSiz.Height()--; // weil GetSize() ein draufaddiert
    Size aMaxSiz(1000000,1000000);
    if (pModel!=NULL) {
        Size aTmpSiz(pModel->GetMaxObjSize());
        if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
        if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
    }
    if (bTextFrame) {
        long nMinWdt=GetMinTextFrameWidth();
        long nMinHgt=GetMinTextFrameHeight();
        long nMaxWdt=GetMaxTextFrameWidth();
        long nMaxHgt=GetMaxTextFrameHeight();
        if (nMinWdt<1) nMinWdt=1;
        if (nMinHgt<1) nMinHgt=1;
        if (!bFitToSize) {
            if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width())  nMaxWdt=aMaxSiz.Width();
            if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
            if (!IsAutoGrowWidth() ) { nMaxWdt=aAnkSiz.Width();  nMinWdt=nMaxWdt; }
            if (!IsAutoGrowHeight()) { nMaxHgt=aAnkSiz.Height(); nMinHgt=nMaxHgt; }
            SdrTextAniKind      eAniKind=GetTextAniKind();
            SdrTextAniDirection eAniDirection=GetTextAniDirection();
            if (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE) {
                // Grenzenlose Papiergroesse fuer Laufschrift
                if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nMaxWdt=1000000;
                if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nMaxHgt=1000000;
            }
            aPaperMax.Width()=nMaxWdt;
            aPaperMax.Height()=nMaxHgt;
        } else {
            aPaperMax=aMaxSiz;
        }
        aPaperMin.Width()=nMinWdt;
        aPaperMin.Height()=nMinHgt;
    } else {
        aPaperMax=aMaxSiz;
    }

    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
    if (pViewMin!=NULL) {
        *pViewMin=aViewInit;

        long nXFree=aAnkSiz.Width()-aPaperMin.Width();
        if (eHAdj==SDRTEXTHORZADJUST_LEFT) pViewMin->Right()-=nXFree;
        else if (eHAdj==SDRTEXTHORZADJUST_RIGHT) pViewMin->Left()+=nXFree;
        else { pViewMin->Left()+=nXFree/2; pViewMin->Right()=pViewMin->Left()+aPaperMin.Width(); }

        long nYFree=aAnkSiz.Height()-aPaperMin.Height();
        if (eVAdj==SDRTEXTVERTADJUST_TOP) pViewMin->Bottom()-=nYFree;
        else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM) pViewMin->Top()+=nYFree;
        else { pViewMin->Top()+=nYFree/2; pViewMin->Bottom()=pViewMin->Top()+aPaperMin.Height(); }
    }

    // Die PaperSize soll in den meisten Faellen von selbst wachsen
    aPaperMin.Height()=0; // #33102#
    if (eHAdj!=SDRTEXTHORZADJUST_BLOCK || bFitToSize) {
        aPaperMin.Width()=0;
    }
    if (pPaperMin!=NULL) *pPaperMin=aPaperMin;
    if (pPaperMax!=NULL) *pPaperMax=aPaperMax;
    if (pViewInit!=NULL) *pViewInit=aViewInit;
}

void SdrTextObj::EndTextEdit(SdrOutliner& rOutl)
{
    if(rOutl.IsModified())
    {
        OutlinerParaObject* pNewText = NULL;
        Paragraph* p1stPara = rOutl.GetParagraph( 0 );
        UINT32 nParaAnz = rOutl.GetParagraphCount();

        if(p1stPara)
        {
            if(nParaAnz == 1)
            {
                // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
                XubString aStr(rOutl.GetText(p1stPara));

                if(!aStr.Len())
                {
                    // Aha, steht nix drin!
                    nParaAnz = 0;
                }
            }

            // Damit der grauen Feldhintergrund wieder verschwindet
            rOutl.UpdateFields();

            if(nParaAnz != 0)
            {
                // Wirklich Textobjekt kreieren
                pNewText = rOutl.CreateParaObject( 0, nParaAnz );
            }
        }
        SetOutlinerParaObject(pNewText);
    }

    pEdtOutl = NULL;
    rOutl.Clear();
    UINT32 nStat = rOutl.GetControlWord();
    nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);
}

SdrObject* SdrTextObj::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer);
}

USHORT SdrTextObj::GetOutlinerViewAnchorMode() const
{
    SdrTextHorzAdjust eH=GetTextHorizontalAdjust();
    SdrTextVertAdjust eV=GetTextVerticalAdjust();
    EVAnchorMode eRet=ANCHOR_TOP_LEFT;
    if (IsContourTextFrame()) return (USHORT)eRet;
    if (eH==SDRTEXTHORZADJUST_LEFT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_LEFT;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_LEFT;
        } else {
            eRet=ANCHOR_VCENTER_LEFT;
        }
    } else if (eH==SDRTEXTHORZADJUST_RIGHT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_RIGHT;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_RIGHT;
        } else {
            eRet=ANCHOR_VCENTER_RIGHT;
        }
    } else {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=ANCHOR_TOP_HCENTER;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=ANCHOR_BOTTOM_HCENTER;
        } else {
            eRet=ANCHOR_VCENTER_HCENTER;
        }
    }
    return (USHORT)eRet;
}

void SdrTextObj::ImpSetTextEditParams() const
{
    if (pEdtOutl!=NULL) {
        FASTBOOL bUpdMerk=pEdtOutl->GetUpdateMode();
        if (bUpdMerk) pEdtOutl->SetUpdateMode(FALSE);
        Size aPaperMin;
        Size aPaperMax;
        Rectangle aEditArea;
        TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,NULL);
        SdrFitToSizeType eFit=GetFitToSize();
        FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
        FASTBOOL bContourFrame=IsContourTextFrame();
        EVAnchorMode eAM=(EVAnchorMode)GetOutlinerViewAnchorMode();
        ULONG nViewAnz=pEdtOutl->GetViewCount();
        pEdtOutl->SetMinAutoPaperSize(aPaperMin);
        pEdtOutl->SetMaxAutoPaperSize(aPaperMax);
        pEdtOutl->SetPaperSize(Size());
        if (bContourFrame) {
            Rectangle aAnchorRect;
            TakeTextAnchorRect(aAnchorRect);
            ImpSetContourPolygon(*pEdtOutl,aAnchorRect, TRUE);
        }
        if (bUpdMerk) pEdtOutl->SetUpdateMode(TRUE);
    }
}

