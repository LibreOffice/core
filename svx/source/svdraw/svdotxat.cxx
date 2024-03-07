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

#include <comphelper/string.hxx>
#include <o3tl/sorted_vector.hxx>
#include <o3tl/string_view.hxx>
#include <svl/style.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/editdata.hxx>
#include <svx/sdtfchim.hxx>


#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editobj.hxx>

namespace {
// The style family which is appended to the style names is padded to this many characters.
const short PADDING_LENGTH_FOR_STYLE_FAMILY = 5;
// this character will be used to pad the style families when they are appended to the style names
const char PADDING_CHARACTER_FOR_STYLE_FAMILY = ' ';
}

bool SdrTextObj::AdjustTextFrameWidthAndHeight( tools::Rectangle& rR, bool bHgt, bool bWdt ) const
{
    if (!mbTextFrame)
        // Not a text frame.  Bail out.
        return false;

    if (rR.IsEmpty())
        // Empty rectangle.
        return false;

    bool bFitToSize = IsFitToSize();
    if (bFitToSize)
        return false;

    bool bWdtGrow = bWdt && IsAutoGrowWidth();
    bool bHgtGrow = bHgt && IsAutoGrowHeight();
    if (!bWdtGrow && !bHgtGrow)
        // Not supposed to auto-adjust width or height.
        return false;

    SdrTextAniKind eAniKind = GetTextAniKind();
    SdrTextAniDirection eAniDir = GetTextAniDirection();

    bool bScroll = eAniKind == SdrTextAniKind::Scroll || eAniKind == SdrTextAniKind::Alternate || eAniKind == SdrTextAniKind::Slide;
    bool bHScroll = bScroll && (eAniDir == SdrTextAniDirection::Left || eAniDir == SdrTextAniDirection::Right);
    bool bVScroll = bScroll && (eAniDir == SdrTextAniDirection::Up || eAniDir == SdrTextAniDirection::Down);

    tools::Rectangle aOldRect = rR;
    tools::Long nHgt = 0, nMinHgt = 0, nMaxHgt = 0;
    tools::Long nWdt = 0, nMinWdt = 0, nMaxWdt = 0;

    Size aNewSize = rR.GetSize();
    aNewSize.AdjustWidth( -1 ); aNewSize.AdjustHeight( -1 );

    Size aMaxSiz(100000, 100000);
    Size aTmpSiz(getSdrModelFromSdrObject().GetMaxObjSize());

    if (aTmpSiz.Width())
        aMaxSiz.setWidth( aTmpSiz.Width() );
    if (aTmpSiz.Height())
        aMaxSiz.setHeight( aTmpSiz.Height() );

    if (bWdtGrow)
    {
        nMinWdt = GetMinTextFrameWidth();
        nMaxWdt = GetMaxTextFrameWidth();
        if (nMaxWdt == 0 || nMaxWdt > aMaxSiz.Width())
            nMaxWdt = aMaxSiz.Width();
        if (nMinWdt <= 0)
            nMinWdt = 1;

        aNewSize.setWidth( nMaxWdt );
    }

    if (bHgtGrow)
    {
        nMinHgt = GetMinTextFrameHeight();
        nMaxHgt = GetMaxTextFrameHeight();
        if (nMaxHgt == 0 || nMaxHgt > aMaxSiz.Height())
            nMaxHgt = aMaxSiz.Height();
        if (nMinHgt <= 0)
            nMinHgt = 1;

        aNewSize.setHeight( nMaxHgt );
    }

    tools::Long nHDist = GetTextLeftDistance() + GetTextRightDistance();
    tools::Long nVDist = GetTextUpperDistance() + GetTextLowerDistance();
    aNewSize.AdjustWidth( -nHDist );
    aNewSize.AdjustHeight( -nVDist );

    if (aNewSize.Width() < 2)
        aNewSize.setWidth( 2 );
    if (aNewSize.Height() < 2)
        aNewSize.setHeight( 2 );

    if (!IsInEditMode())
    {
        if (bHScroll)
            aNewSize.setWidth( 0x0FFFFFFF ); // don't break ticker text
        if (bVScroll)
            aNewSize.setHeight( 0x0FFFFFFF );
    }

    if (mpEditingOutliner)
    {
        mpEditingOutliner->SetMaxAutoPaperSize(aNewSize);
        if (bWdtGrow)
        {
            Size aSiz2(mpEditingOutliner->CalcTextSize());
            nWdt = aSiz2.Width() + 1; // a little tolerance
            if (bHgtGrow)
                nHgt = aSiz2.Height() + 1; // a little tolerance
        }
        else
        {
            nHgt = mpEditingOutliner->GetTextHeight() + 1; // a little tolerance
        }
    }
    else
    {
        Outliner& rOutliner = ImpGetDrawOutliner();
        rOutliner.SetPaperSize(aNewSize);
        rOutliner.SetUpdateLayout(true);
        // TODO: add the optimization with bPortionInfoChecked etc. here
        OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
        if (pOutlinerParaObject)
        {
            rOutliner.SetText(*pOutlinerParaObject);
            rOutliner.SetFixedCellHeight(GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
        }

        if (bWdtGrow)
        {
            Size aSiz2(rOutliner.CalcTextSize());
            nWdt = aSiz2.Width() + 1; // a little tolerance
            if (bHgtGrow)
                nHgt = aSiz2.Height() + 1; // a little tolerance
        }
        else
        {
            nHgt = rOutliner.GetTextHeight() + 1; // a little tolerance
        }
        rOutliner.Clear();
    }

    if (nWdt < nMinWdt)
        nWdt = nMinWdt;
    if (nWdt > nMaxWdt)
        nWdt = nMaxWdt;
    nWdt += nHDist;
    if (nWdt < 1)
        nWdt = 1; // nHDist may be negative
    if (nHgt < nMinHgt)
        nHgt = nMinHgt;
    if (nHgt > nMaxHgt)
        nHgt = nMaxHgt;
    nHgt += nVDist;
    if (nHgt < 1)
        nHgt = 1; // nVDist may be negative
    tools::Long nWdtGrow = nWdt - (rR.Right() - rR.Left());
    tools::Long nHgtGrow = nHgt - (rR.Bottom() - rR.Top());

    if (nWdtGrow == 0)
        bWdtGrow = false;
    if (nHgtGrow == 0)
        bHgtGrow = false;

    if (!bWdtGrow && !bHgtGrow)
        return false;

    if (bWdtGrow)
    {
        SdrTextHorzAdjust eHAdj = GetTextHorizontalAdjust();

        if (eHAdj == SDRTEXTHORZADJUST_LEFT)
            rR.AdjustRight(nWdtGrow );
        else if (eHAdj == SDRTEXTHORZADJUST_RIGHT)
            rR.AdjustLeft( -nWdtGrow );
        else
        {
            tools::Long nWdtGrow2 = nWdtGrow / 2;
            rR.AdjustLeft( -nWdtGrow2 );
            rR.SetRight( rR.Left() + nWdt );
        }
    }

    if (bHgtGrow)
    {
        SdrTextVertAdjust eVAdj = GetTextVerticalAdjust();

        if (eVAdj == SDRTEXTVERTADJUST_TOP)
            rR.AdjustBottom(nHgtGrow );
        else if (eVAdj == SDRTEXTVERTADJUST_BOTTOM)
            rR.AdjustTop( -nHgtGrow );
        else
        {
            tools::Long nHgtGrow2 = nHgtGrow / 2;
            rR.AdjustTop( -nHgtGrow2 );
            rR.SetBottom( rR.Top() + nHgt );
        }
    }

    if (maGeo.m_nRotationAngle)
    {
        // Object is rotated.
        Point aD1(rR.TopLeft());
        aD1 -= aOldRect.TopLeft();
        Point aD2(aD1);
        RotatePoint(aD2, Point(), maGeo.mfSinRotationAngle, maGeo.mfCosRotationAngle);
        aD2 -= aD1;
        rR.Move(aD2.X(), aD2.Y());
    }

    return true;
}

bool SdrTextObj::NbcAdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
    tools::Rectangle aRectangle(getRectangle());
    bool bRet = AdjustTextFrameWidthAndHeight(aRectangle, bHgt, bWdt);
    setRectangle(aRectangle);
    if (bRet)
    {
        SetBoundAndSnapRectsDirty();
        if (auto pRectObj = dynamic_cast<SdrRectObj *>(this)) { // this is a hack
            pRectObj->SetXPolyDirty();
        }
        if (auto pCaptionObj = dynamic_cast<SdrCaptionObj *>(this)) { // this is a hack
            pCaptionObj->ImpRecalcTail();
        }
    }
    return bRet;
}

bool SdrTextObj::AdjustTextFrameWidthAndHeight()
{
    tools::Rectangle aNewRect(getRectangle());
    bool bRet = AdjustTextFrameWidthAndHeight(aNewRect);
    if (bRet) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        setRectangle(aNewRect);
        SetBoundAndSnapRectsDirty();
        if (auto pRectObj = dynamic_cast<SdrRectObj *>(this)) { // this is a hack
            pRectObj->SetXPolyDirty();
        }
        bool bScPostIt = false;
        if (auto pCaptionObj = dynamic_cast<SdrCaptionObj *>(this)) { // this is a hack
            pCaptionObj->ImpRecalcTail();
            // tdf#114956, tdf#138549 use GetSpecialTextBoxShadow to recognize
            // that this SdrCaption is for a ScPostit
            bScPostIt = pCaptionObj->GetSpecialTextBoxShadow();
        }

        // to not slow down EditView visualization on Overlay (see
        // TextEditOverlayObject) it is necessary to suppress the
        // Invalidates for the deep repaint when the size of the
        // TextFrame changed (AdjustTextFrameWidthAndHeight returned
        // true). The ObjectChanges are valid, invalidate will be
        // done on EndTextEdit anyways
        const bool bSuppressChangeWhenEditOnOverlay(
            IsInEditMode() &&
            GetTextEditOutliner() &&
            GetTextEditOutliner()->hasEditViewCallbacks());

        if (!bSuppressChangeWhenEditOnOverlay || bScPostIt)
        {
            SetChanged();
            BroadcastObjectChange();
        }

        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
    return bRet;
}

void SdrTextObj::ImpSetTextStyleSheetListeners()
{
    SfxStyleSheetBasePool* pStylePool(getSdrModelFromSdrObject().GetStyleSheetPool());
    if (pStylePool==nullptr)
        return;

    std::vector<OUString> aStyleNames;
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if (pOutlinerParaObject!=nullptr)
    {
        // First, we collect all stylesheets contained in the ParaObject in
        // the container aStyles. The Family is always appended to the name
        // of the stylesheet.
        const EditTextObject& rTextObj=pOutlinerParaObject->GetTextObject();
        OUString aStyleName;
        SfxStyleFamily eStyleFam;
        sal_Int32 nParaCnt=rTextObj.GetParagraphCount();


        for(sal_Int32 nParaNum(0); nParaNum < nParaCnt; nParaNum++)
        {
            rTextObj.GetStyleSheet(nParaNum, aStyleName, eStyleFam);

            if (!aStyleName.isEmpty())
            {
                AppendFamilyToStyleName(aStyleName, eStyleFam);

                bool bFnd(false);
                sal_uInt32 nNum(aStyleNames.size());

                while(!bFnd && nNum > 0)
                {
                    // we don't want duplicate stylesheets
                    nNum--;
                    bFnd = aStyleName == aStyleNames[nNum];
                }

                if(!bFnd)
                {
                    aStyleNames.push_back(aStyleName);
                }
            }
        }
    }

    // now convert the strings in the vector from names to StyleSheet*
    o3tl::sorted_vector<SfxStyleSheet*> aStyleSheets;
    while (!aStyleNames.empty()) {
        OUString aName = aStyleNames.back();
        aStyleNames.pop_back();

        SfxStyleFamily eFam = ReadFamilyFromStyleName(aName);
        SfxStyleSheetBase* pStyleBase = pStylePool->Find(aName,eFam);
        SfxStyleSheet* pStyle = dynamic_cast<SfxStyleSheet*>( pStyleBase );
        if (pStyle!=nullptr && pStyle!=GetStyleSheet()) {
            aStyleSheets.insert(pStyle);
        }
    }
    // now remove all superfluous stylesheets
    sal_uInt16 nNum=GetBroadcasterCount();
    while (nNum>0) {
        nNum--;
        SfxBroadcaster* pBroadcast=GetBroadcasterJOE(nNum);
        if (pBroadcast->IsSfxStyleSheet())
        {
            SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>( pBroadcast );
            if (pStyle!=GetStyleSheet()) { // special case for stylesheet of the object
                if (aStyleSheets.find(pStyle)==aStyleSheets.end()) {
                    EndListening(*pStyle);
                }
            }
        }
    }
    // and finally, merge all stylesheets that are contained in aStyles with previous broadcasters
    for(SfxStyleSheet* pStyle : aStyleSheets) {
        // let StartListening see for itself if there's already a listener registered
        StartListening(*pStyle, DuplicateHandling::Prevent);
    }
}

/**  iterates over the paragraphs of a given SdrObject and removes all
     hard set character attributes with the which ids contained in the
     given vector
*/
void SdrTextObj::RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds )
{
    sal_Int32 nText = getTextCount();

    while( --nText >= 0 )
    {
        SdrText* pText = getText( nText );
        OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : nullptr;

        if(pOutlinerParaObject)
        {
            Outliner* pOutliner = nullptr;

            if( mpEditingOutliner || (pText == getActiveText()) )
                pOutliner = mpEditingOutliner;

            if(!pOutliner)
            {
                pOutliner = &ImpGetDrawOutliner();
                pOutliner->SetText(*pOutlinerParaObject);
            }

            ESelection aSelAll( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL );
            for( const auto& rWhichId : rCharWhichIds )
            {
                pOutliner->RemoveAttribs( aSelAll, false, rWhichId );
            }

            if(!mpEditingOutliner || (pText != getActiveText()) )
            {
                const sal_Int32 nParaCount = pOutliner->GetParagraphCount();
                std::optional<OutlinerParaObject> pTemp = pOutliner->CreateParaObject(0, nParaCount);
                pOutliner->Clear();
                NbcSetOutlinerParaObjectForText(std::move(pTemp), pText);
            }
        }
    }
}

bool SdrTextObj::HasText() const
{
    if (mpEditingOutliner)
        return HasTextImpl(mpEditingOutliner);

    OutlinerParaObject* pOPO = GetOutlinerParaObject();

    if( !pOPO )
        return false;

    const EditTextObject& rETO = pOPO->GetTextObject();
    sal_Int32 nParaCount = rETO.GetParagraphCount();
    if( nParaCount == 0 )
        return false;
    if( nParaCount > 1 )
        return true;
    return rETO.HasText( 0 );
}

void SdrTextObj::AppendFamilyToStyleName(OUString& styleName, SfxStyleFamily family)
{
    OUStringBuffer aFam = OUString::number(static_cast<sal_Int32>(family));
    comphelper::string::padToLength(aFam, PADDING_LENGTH_FOR_STYLE_FAMILY , PADDING_CHARACTER_FOR_STYLE_FAMILY);

    styleName += "|" + aFam;
}

SfxStyleFamily SdrTextObj::ReadFamilyFromStyleName(std::u16string_view styleName)
{
    std::u16string_view familyString = styleName.substr(styleName.size() - PADDING_LENGTH_FOR_STYLE_FAMILY);
    familyString = comphelper::string::stripEnd(familyString, PADDING_CHARACTER_FOR_STYLE_FAMILY);
    sal_uInt16 nFam = static_cast<sal_uInt16>(o3tl::toInt32(familyString));
    assert(nFam != 0);
    return static_cast<SfxStyleFamily>(nFam);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
