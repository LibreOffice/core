/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx> // fuer GetMaxObjSize
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtfchim.hxx>

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
    return sal_True;
}

sal_Bool SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
{
    if (pEdtOutl!=NULL) return sal_False; // Textedit laeuft evtl. schon an einer anderen View!
    pEdtOutl=&rOutl;

    // #101684#
    mbInEditMode = sal_True;

    sal_uInt16 nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
    if ( !IsOutlText() )
        nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
    rOutl.Init( nOutlinerMode );
    rOutl.SetRefDevice( pModel->GetRefDevice() );

    SdrFitToSizeType eFit=GetFitToSize();
    FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
    FASTBOOL bContourFrame=IsContourTextFrame();
    ImpSetTextEditParams();

    if (!bContourFrame) {
        sal_uIntPtr nStat=rOutl.GetControlWord();
        nStat|=EE_CNTRL_AUTOPAGESIZE;
        if (bFitToSize) nStat|=EE_CNTRL_STRETCHING; else nStat&=~EE_CNTRL_STRETCHING;
        rOutl.SetControlWord(nStat);
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject!=NULL)
    {
        rOutl.SetText(*GetOutlinerParaObject());
        rOutl.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    }

    // ggf. Rahmenattribute am 1. (neuen) Absatz des Outliners setzen
    if( !HasTextImpl( &rOutl ) )
    {
        // Outliner has no text so we must set some
        // empty text so the outliner initialise itself
        rOutl.SetText( String(), rOutl.GetParagraph( 0 ) );

        if(GetStyleSheet())
            rOutl.SetStyleSheet( 0, GetStyleSheet());

        // Beim setzen der harten Attribute an den ersten Absatz muss
        // der Parent pOutlAttr (=die Vorlage) temporaer entfernt
        // werden, da sonst bei SetParaAttribs() auch alle in diesem
        // Parent enthaltenen Items hart am Absatz attributiert werden.
        // -> BugID 22467
        const SfxItemSet& rSet = GetObjectItemSet();
        SfxItemSet aFilteredSet(*rSet.GetPool(), EE_ITEMS_START, EE_ITEMS_END);
        aFilteredSet.Put(rSet);
        rOutl.SetParaAttribs(0, aFilteredSet);
    }
    if (bFitToSize)
    {
        Rectangle aAnchorRect;
        Rectangle aTextRect;
        TakeTextRect(rOutl, aTextRect, sal_False,
            &aAnchorRect/* #97097# give sal_True here, not sal_False */);
        Fraction aFitXKorreg(1,1);
        ImpSetCharStretching(rOutl,aTextRect,aAnchorRect,aFitXKorreg);
    }

    if(pOutlinerParaObject)
    {
        // #78476# also repaint when animated text is put to edit mode
        // to not make appear the text double
        // #111096# should now repaint automatically.
        // sal_Bool bIsAnimated(pPlusData && pPlusData->pAnimator);

        if(aGeo.nDrehWink || IsFontwork() /*|| bIsAnimated*/)
        {
            // only repaint here, no real objectchange

//          ActionChanged();
            BroadcastObjectChange();
        }
    }

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return sal_True;
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

    // #106879#
    // Done earlier since used in else tree below
    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    if(IsTextFrame())
    {
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

            // #101684#
            sal_Bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
            {
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
    }
    else
    {
        // #106879#
        // aPaperMin needs to be set to object's size if full width is activated
        // for hor or ver writing respectively
        if((SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
            || (SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting()))
        {
            aPaperMin = aAnkSiz;
        }

        aPaperMax=aMaxSiz;
    }

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
    // #89459#
    if(IsVerticalWriting())
        aPaperMin.Width() = 0;
    else
        aPaperMin.Height() = 0; // #33102#

    if(eHAdj!=SDRTEXTHORZADJUST_BLOCK || bFitToSize) {
        aPaperMin.Width()=0;
    }

    // #103516# For complete ver adjust support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj || bFitToSize)
    {
        aPaperMin.Height() = 0;
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

        if(HasTextImpl( &rOutl ) )
        {
            // Damit der grauen Feldhintergrund wieder verschwindet
            rOutl.UpdateFields();

            sal_uInt16 nParaAnz = static_cast< sal_uInt16 >( rOutl.GetParagraphCount() );
            pNewText = rOutl.CreateParaObject( 0, nParaAnz );
        }

        // need to end edit mode early since SetOutlinerParaObject already
        // uses GetCurrentBoundRect() which needs to take the text into account
        // to work correct
        mbInEditMode = sal_False;
        SetOutlinerParaObject(pNewText);
    }

    pEdtOutl = NULL;
    rOutl.Clear();
    sal_uInt32 nStat = rOutl.GetControlWord();
    nStat &= ~EE_CNTRL_AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    // #101684#
    mbInEditMode = sal_False;
}

sal_uInt16 SdrTextObj::GetOutlinerViewAnchorMode() const
{
    SdrTextHorzAdjust eH=GetTextHorizontalAdjust();
    SdrTextVertAdjust eV=GetTextVerticalAdjust();
    EVAnchorMode eRet=ANCHOR_TOP_LEFT;
    if (IsContourTextFrame()) return (sal_uInt16)eRet;
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
    return (sal_uInt16)eRet;
}

void SdrTextObj::ImpSetTextEditParams() const
{
    if (pEdtOutl!=NULL) {
        FASTBOOL bUpdMerk=pEdtOutl->GetUpdateMode();
        if (bUpdMerk) pEdtOutl->SetUpdateMode(sal_False);
        Size aPaperMin;
        Size aPaperMax;
        Rectangle aEditArea;
        TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,NULL);
        //SdrFitToSizeType eFit=GetFitToSize();
        //FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
        FASTBOOL bContourFrame=IsContourTextFrame();
        //EVAnchorMode eAM=(EVAnchorMode)GetOutlinerViewAnchorMode();
        //sal_uIntPtr nViewAnz=pEdtOutl->GetViewCount();
        pEdtOutl->SetMinAutoPaperSize(aPaperMin);
        pEdtOutl->SetMaxAutoPaperSize(aPaperMax);
        pEdtOutl->SetPaperSize(Size());
        if (bContourFrame) {
            Rectangle aAnchorRect;
            TakeTextAnchorRect(aAnchorRect);
            ImpSetContourPolygon(*pEdtOutl,aAnchorRect, sal_True);
        }
        if (bUpdMerk) pEdtOutl->SetUpdateMode(sal_True);
    }
}

