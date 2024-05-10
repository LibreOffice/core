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
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editstat.hxx>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtfchim.hxx>
#include <textchain.hxx>


bool SdrTextObj::HasTextEdit() const
{
    // linked text objects may be changed (no automatic reload)
    return true;
}

bool SdrTextObj::BegTextEdit(SdrOutliner& rOutl)
{
    if (mpEditingOutliner!=nullptr) return false; // Textedit might already run in another View!
    mpEditingOutliner=&rOutl;

    mbInEditMode = true;

    OutlinerMode nOutlinerMode = OutlinerMode::OutlineObject;
    if ( !IsOutlText() )
        nOutlinerMode = OutlinerMode::TextObject;
    rOutl.Init( nOutlinerMode );
    rOutl.SetRefDevice(getSdrModelFromSdrObject().GetRefDevice());

    bool bFitToSize(IsFitToSize());
    bool bContourFrame=IsContourTextFrame();
    ImpSetTextEditParams();

    if (!bContourFrame) {
        EEControlBits nStat=rOutl.GetControlWord();
        nStat|=EEControlBits::AUTOPAGESIZE;
        if (bFitToSize || IsAutoFit())
            nStat|=EEControlBits::STRETCHING;
        else
            nStat&=~EEControlBits::STRETCHING;
        rOutl.SetControlWord(nStat);
    }

    // disable AUTOPAGESIZE if IsChainable (might be required for overflow check)
    if ( IsChainable() ) {
        EEControlBits nStat1=rOutl.GetControlWord();
        nStat1 &=~EEControlBits::AUTOPAGESIZE;
        rOutl.SetControlWord(nStat1);
    }


    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if(pOutlinerParaObject!=nullptr)
    {
        rOutl.SetText(*GetOutlinerParaObject());
        rOutl.SetFixedCellHeight(GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
    }

    // if necessary, set frame attributes for the first (new) paragraph of the
    // outliner
    if( !HasTextImpl( &rOutl ) )
    {
        // Outliner has no text so we must set some
        // empty text so the outliner initialise itself
        rOutl.SetText( u""_ustr, rOutl.GetParagraph( 0 ) );

        if(GetStyleSheet())
            rOutl.SetStyleSheet( 0, GetStyleSheet());

        // When setting the "hard" attributes for first paragraph, the Parent
        // pOutlAttr (i. e. the template) has to be removed temporarily. Else,
        // at SetParaAttribs(), all attributes contained in the parent become
        // attributed hard to the paragraph.
        const SfxItemSet& rSet = GetObjectItemSet();
        SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END> aFilteredSet(*rSet.GetPool());
        aFilteredSet.Put(rSet);
        rOutl.SetParaAttribs(0, aFilteredSet);
    }
    if (bFitToSize)
    {
        tools::Rectangle aAnchorRect;
        tools::Rectangle aTextRect;
        TakeTextRect(rOutl, aTextRect, false,
            &aAnchorRect);
        Fraction aFitXCorrection(1,1);
        ImpSetCharStretching(rOutl,aTextRect.GetSize(),aAnchorRect.GetSize(),aFitXCorrection);
    }
    else if (IsAutoFit())
    {
        setupAutoFitText(rOutl);
    }

    if(pOutlinerParaObject)
    {
        if (maGeo.m_nRotationAngle || IsFontwork())
        {
            // only repaint here, no real objectchange
            BroadcastObjectChange();
        }
    }

    rOutl.UpdateFields();
    rOutl.ClearModifyFlag();

    return true;
}

void SdrTextObj::TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const
{
    bool bFitToSize(IsFitToSize());
    Size aPaperMin,aPaperMax;
    tools::Rectangle aViewInit;
    TakeTextAnchorRect(aViewInit);
    if (maGeo.m_nRotationAngle) {
        Point aCenter(aViewInit.Center());
        aCenter-=aViewInit.TopLeft();
        Point aCenter0(aCenter);
        RotatePoint(aCenter,Point(),maGeo.mfSinRotationAngle,maGeo.mfCosRotationAngle);
        aCenter-=aCenter0;
        aViewInit.Move(aCenter.X(),aCenter.Y());
    }
    Size aAnkSiz(aViewInit.GetSize());
    aAnkSiz.AdjustWidth( -1 ); aAnkSiz.AdjustHeight( -1 ); // because GetSize() adds 1
    Size aMaxSiz(1000000,1000000);
    Size aTmpSiz(getSdrModelFromSdrObject().GetMaxObjSize());
    if (aTmpSiz.Width()!=0) aMaxSiz.setWidth(aTmpSiz.Width() );
    if (aTmpSiz.Height()!=0) aMaxSiz.setHeight(aTmpSiz.Height() );

    // Done earlier since used in else tree below
    SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
    SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

    if(IsTextFrame())
    {
        tools::Long nMinWdt=GetMinTextFrameWidth();
        tools::Long nMinHgt=GetMinTextFrameHeight();
        tools::Long nMaxWdt=GetMaxTextFrameWidth();
        tools::Long nMaxHgt=GetMaxTextFrameHeight();
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

            bool bInEditMode = IsInEditMode();

            if (!bInEditMode && (eAniKind==SdrTextAniKind::Scroll || eAniKind==SdrTextAniKind::Alternate || eAniKind==SdrTextAniKind::Slide))
            {
                // ticker text uses an unlimited paper size
                if (eAniDirection==SdrTextAniDirection::Left || eAniDirection==SdrTextAniDirection::Right) nMaxWdt=1000000;
                if (eAniDirection==SdrTextAniDirection::Up || eAniDirection==SdrTextAniDirection::Down) nMaxHgt=1000000;
            }

            bool bChainedFrame = IsChainable();
            // Might be required for overflow check working: do limit height to frame if box is chainable.
            if (!bChainedFrame) {
                // #i119885# Do not limit/force height to geometrical frame (vice versa for vertical writing)
                if(IsVerticalWriting())
                {
                    nMaxWdt = 1000000;
                }
                else
                {
                    nMaxHgt = 1000000;
                }
            }

            aPaperMax.setWidth(nMaxWdt );
            aPaperMax.setHeight(nMaxHgt );
        }
        else
        {
            aPaperMax=aMaxSiz;
        }
        aPaperMin.setWidth(nMinWdt );
        aPaperMin.setHeight(nMinHgt );
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

    if (pViewMin!=nullptr) {
        *pViewMin=aViewInit;

        tools::Long nXFree=aAnkSiz.Width()-aPaperMin.Width();
        if (eHAdj==SDRTEXTHORZADJUST_LEFT) pViewMin->AdjustRight( -nXFree );
        else if (eHAdj==SDRTEXTHORZADJUST_RIGHT) pViewMin->AdjustLeft(nXFree );
        else { pViewMin->AdjustLeft(nXFree/2 ); pViewMin->SetRight(pViewMin->Left()+aPaperMin.Width() ); }

        tools::Long nYFree=aAnkSiz.Height()-aPaperMin.Height();
        if (eVAdj==SDRTEXTVERTADJUST_TOP) pViewMin->AdjustBottom( -nYFree );
        else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM) pViewMin->AdjustTop(nYFree );
        else { pViewMin->AdjustTop(nYFree/2 ); pViewMin->SetBottom(pViewMin->Top()+aPaperMin.Height() ); }
    }

    // PaperSize should grow automatically in most cases
    if(IsVerticalWriting())
        aPaperMin.setWidth( 0 );
    else
        aPaperMin.setHeight( 0 );

    if(eHAdj!=SDRTEXTHORZADJUST_BLOCK || bFitToSize) {
        aPaperMin.setWidth(0 );
    }

    // For complete vertical adjustment support, set paper min height to 0, here.
    if(SDRTEXTVERTADJUST_BLOCK != eVAdj || bFitToSize)
    {
        aPaperMin.setHeight( 0 );
    }

    if (pPaperMin!=nullptr) *pPaperMin=aPaperMin;
    if (pPaperMax!=nullptr) *pPaperMax=aPaperMax;
    if (pViewInit!=nullptr) *pViewInit=aViewInit;
}

void SdrTextObj::EndTextEdit(SdrOutliner& rOutl)
{
    if(rOutl.IsModified())
    {

        // to make the gray field background vanish again
        rOutl.UpdateFields();

        std::optional<OutlinerParaObject> pNewText = rOutl.CreateParaObject( 0, rOutl.GetParagraphCount() );

        // need to end edit mode early since SetOutlinerParaObject already
        // uses GetCurrentBoundRect() which needs to take the text into account
        // to work correct
        mbInEditMode = false;

        // We don't want broadcasting if we are merely trying to move to next box (this prevents infinite loops)
        if (IsChainable() && GetTextChain()->GetSwitchingToNextBox(this)) {
            GetTextChain()->SetSwitchingToNextBox(this, false);
            if( getActiveText() )
            {
                getActiveText()->SetOutlinerParaObject( std::move(pNewText) );
            }
        } else { // If we are not doing in-chaining switching just set the ParaObject
            SetOutlinerParaObject(std::move(pNewText));
        }
    }

    /* Chaining-related code */
    rOutl.ClearOverflowingParaNum();

    mpEditingOutliner = nullptr;
    rOutl.Clear();
    EEControlBits nStat = rOutl.GetControlWord();
    nStat &= ~EEControlBits::AUTOPAGESIZE;
    rOutl.SetControlWord(nStat);

    mbInEditMode = false;
}

EEAnchorMode SdrTextObj::GetOutlinerViewAnchorMode() const
{
    SdrTextHorzAdjust eH=GetTextHorizontalAdjust();
    SdrTextVertAdjust eV=GetTextVerticalAdjust();
    EEAnchorMode eRet=EEAnchorMode::TopLeft;
    if (IsContourTextFrame()) return eRet;
    if (eH==SDRTEXTHORZADJUST_LEFT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=EEAnchorMode::TopLeft;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=EEAnchorMode::BottomLeft;
        } else {
            eRet=EEAnchorMode::VCenterLeft;
        }
    } else if (eH==SDRTEXTHORZADJUST_RIGHT) {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=EEAnchorMode::TopRight;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=EEAnchorMode::BottomRight;
        } else {
            eRet=EEAnchorMode::VCenterRight;
        }
    } else {
        if (eV==SDRTEXTVERTADJUST_TOP) {
            eRet=EEAnchorMode::TopHCenter;
        } else if (eV==SDRTEXTVERTADJUST_BOTTOM) {
            eRet=EEAnchorMode::BottomHCenter;
        } else {
            eRet=EEAnchorMode::VCenterHCenter;
        }
    }
    return eRet;
}

void SdrTextObj::ImpSetTextEditParams() const
{
    if (mpEditingOutliner==nullptr)
        return;

    bool bUpdBuf=mpEditingOutliner->SetUpdateLayout(false);
    Size aPaperMin;
    Size aPaperMax;
    tools::Rectangle aEditArea;
    TakeTextEditArea(&aPaperMin,&aPaperMax,&aEditArea,nullptr);
    bool bContourFrame=IsContourTextFrame();
    mpEditingOutliner->SetMinAutoPaperSize(aPaperMin);
    mpEditingOutliner->SetMaxAutoPaperSize(aPaperMax);
    mpEditingOutliner->SetPaperSize(Size());
    mpEditingOutliner->SetTextColumns(GetTextColumnsNumber(), GetTextColumnsSpacing());
    if (bContourFrame) {
        tools::Rectangle aAnchorRect;
        TakeTextAnchorRect(aAnchorRect);
        ImpSetContourPolygon(*mpEditingOutliner,aAnchorRect, true);
    }
    if (bUpdBuf) mpEditingOutliner->SetUpdateLayout(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
