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


#include <svx/svdotext.hxx>
#include <svx/svdmodel.hxx> // for GetMaxObjSize
#include <svx/svdoutl.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtfchim.hxx>


bool SdrTextObj::HasTextEdit() const
{
    // linked text objects may be changed (no automatic reload)
    return true;
}

sal_Bool SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
{
    if (pEdtOutl!=NULL) return sal_False; // Textedit might already run in another View!
    pEdtOutl=&rOutl;

    mbInEditMode = sal_True;

    sal_uInt16 nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
    if ( !IsOutlText() )
        nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
    rOutl.Init( nOutlinerMode );
    rOutl.SetRefDevice( pModel->GetRefDevice() );

    bool bFitToSize(IsFitToSize());
    bool bContourFrame=IsContourTextFrame();
    ImpSetTextEditParams();

    if (!bContourFrame) {
        sal_uIntPtr nStat=rOutl.GetControlWord();
        nStat|=EE_CNTRL_AUTOPAGESIZE;
        if (bFitToSize || IsAutoFit())
            nStat|=EE_CNTRL_STRETCHING;
        else
            nStat&=~EE_CNTRL_STRETCHING;
        rOutl.SetControlWord(nStat);
    }

    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject!=NULL)
    {
        rOutl.SetText(*GetOutlinerParaObject());
        rOutl.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
    }

    // if necessary, set frame attributes for the first (new) paragraph of the
    // outliner
    if( !HasTextImpl( &rOutl ) )
    {
        // Outliner has no text so we must set some
        // empty text so the outliner initialise itself
        rOutl.SetText( "", rOutl.GetParagraph( 0 ) );

        if(GetStyleSheet())
            rOutl.SetStyleSheet( 0, GetStyleSheet());

        // When setting the "hard" attributes for first paragraph, the Parent
        // pOutlAttr (i. e. the template) has to be removed temporarily. Else,
        // at SetParaAttribs(), all attributes contained in the parent become
        // attributed hard to the paragraph.
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
            &aAnchorRect);
        Fraction aFitXKorreg(1,1);
        ImpSetCharStretching(rOutl,aTextRect.GetSize(),aAnchorRect.GetSize(),aFitXKorreg);
    }
    else if (IsAutoFit())
    {
        ImpAutoFitText(rOutl);
    }

    if(pOutlinerParaObject)
    {
        if(aGeo.nDrehWink || IsFontwork())
        {
            // only repaint here, no real objectchange
            BroadcastObjectChange();
        }
    }

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return sal_True;
}

void SdrTextObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const
{
    bool bFitToSize(IsFitToSize());
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
    aAnkSiz.Width()--; aAnkSiz.Height()--; // because GetSize() adds 1
    Size aMaxSiz(1000000,1000000);
    if (pModel!=NULL) {
        Size aTmpSiz(pModel->GetMaxObjSize());
        if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
        if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
    }

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

            if (!IsAutoGrowWidth() )
            {
                nMinWdt = aAnkSiz.Width();
                nMaxWdt = nMinWdt;
            }

            if (!IsAutoGrowHeight())
            {
                nMinHgt = aAnkSiz.Height();
                nMaxHgt = nMinHgt;
            }

            SdrTextAniKind      eAniKind=GetTextAniKind();
            SdrTextAniDirection eAniDirection=GetTextAniDirection();

            sal_Bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
            {
                // ticker text uses an unlimited paper size
                if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nMaxWdt=1000000;
                if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nMaxHgt=1000000;
            }

            // #i119885# Do not limit/force height to geometrical frame (vice versa for vertical writing)
            if(IsVerticalWriting())
            {
                nMaxWdt = 1000000;
            }
            else
            {
                nMaxHgt = 1000000;
            }

            aPaperMax.Width()=nMaxWdt;
            aPaperMax.Height()=nMaxHgt;
        }
        else
        {
            aPaperMax=aMaxSiz;
        }
        aPaperMin.Width()=nMinWdt;
        aPaperMin.Height()=nMinHgt;
    }
    else
    {
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

    // PaperSize should grow automatically in most cases
    if(IsVerticalWriting())
        aPaperMin.Width() = 0;
    else
        aPaperMin.Height() = 0;

    if(eHAdj!=SDRTEXTHORZADJUST_BLOCK || bFitToSize) {
        aPaperMin.Width()=0;
    }

    // For complete vertical adjustment support, set paper min height to 0, here.
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

        // to make the gray field background vanish again
        rOutl.UpdateFields();

        sal_Int32 nParaAnz = rOutl.GetParagraphCount();
        pNewText = rOutl.CreateParaObject( 0, nParaAnz );

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
        bool bUpdMerk=pEdtOutl->GetUpdateMode();
        if (bUpdMerk) pEdtOutl->SetUpdateMode(sal_False);
        Size aPaperMin;
        Size aPaperMax;
        Rectangle aEditArea;
        TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,NULL);
        bool bContourFrame=IsContourTextFrame();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
