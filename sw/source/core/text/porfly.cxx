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

#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <pam.hxx>
#include "portab.hxx"
#include <flyfrm.hxx>
#include <rootfrm.hxx>
#include <frmfmt.hxx>
#include <viewsh.hxx>
#include <textboxhelper.hxx>

#include <vcl/outdev.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sal/log.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <flyfrms.hxx>
#include <txatbase.hxx>
#include "porfly.hxx"
#include "porlay.hxx"
#include "inftxt.hxx"

#include <sortedobjs.hxx>

/**
 * class SwFlyPortion => we expect a frame-locale SwRect!
 */

void SwFlyPortion::Paint( const SwTextPaintInfo& ) const
{
}

bool SwFlyPortion::Format( SwTextFormatInfo &rInf )
{
    OSL_ENSURE( GetFix() >= rInf.X(), "SwFlyPortion::Format" );

    // tabs must be expanded
    if( rInf.GetLastTab() )
        rInf.GetLastTab()->FormatEOL( rInf );

    rInf.GetLast()->FormatEOL( rInf );
    PrtWidth( static_cast<sal_uInt16>(GetFix() - rInf.X() + PrtWidth()) );
    if( !Width() )
    {
        OSL_ENSURE( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
        Width(1);
    }

    // resetting
    rInf.SetFly( nullptr );
    rInf.Width( rInf.RealWidth() );
    rInf.GetParaPortion()->SetFly();

    // trailing blank:
    if( rInf.GetIdx() < TextFrameIndex(rInf.GetText().getLength())
        && TextFrameIndex(1) < rInf.GetIdx()
        && !rInf.GetRest()
        && ' ' == rInf.GetChar( rInf.GetIdx() )
        && ' ' != rInf.GetChar(rInf.GetIdx() - TextFrameIndex(1))
        && ( !rInf.GetLast() || !rInf.GetLast()->IsBreakPortion() ) )
    {
        SetBlankWidth( rInf.GetTextSize(OUString(' ')).Width() );
        SetLen(TextFrameIndex(1));
    }

    const sal_uInt16 nNewWidth = static_cast<sal_uInt16>(rInf.X() + PrtWidth());
    if( rInf.Width() <= nNewWidth )
    {
        Truncate();
        if( nNewWidth > rInf.Width() )
        {
            PrtWidth( nNewWidth - rInf.Width() );
            SetFixWidth( PrtWidth() );
        }
        return true;
    }
    return false;
}

bool SwFlyCntPortion::Format( SwTextFormatInfo &rInf )
{
    bool bFull = rInf.Width() < rInf.X() + PrtWidth();

    if( bFull )
    {
        // If the line is full, and the character-bound frame is at
        // the beginning of a line
        // If it is not possible to side step into a Fly
        // "Begin of line" criteria ( ! rInf.X() ) has to be extended.
        // KerningPortions at beginning of line, e.g., for grid layout
        // must be considered.
        const SwLinePortion* pLastPor = rInf.GetLast();
        const sal_uInt16 nLeft = ( pLastPor &&
                                    ( pLastPor->IsKernPortion() ||
                                      pLastPor->IsErgoSumPortion() ) ) ?
                               pLastPor->Width() :
                               0;

        if( nLeft == rInf.X() && ! rInf.GetFly() )
        {
            Width( rInf.Width() );
            bFull = false; // so that notes can still be placed in this line
        }
        else
        {
            if( !rInf.GetFly() )
                rInf.SetNewLine( true );
            Width(0);
            SetAscent(0);
            SetLen(TextFrameIndex(0));
            if( rInf.GetLast() )
                rInf.GetLast()->FormatEOL( rInf );

            return bFull;
        }
    }

    rInf.GetParaPortion()->SetFly();
    return bFull;
}

//TODO: improve documentation
/** move character-bound objects inside the given area
 *
 * This allows moving those objects from Master to Follow, or vice versa.
 *
 * @param pNew
 * @param nStart
 * @param nEnd
 */
void SwTextFrame::MoveFlyInCnt(SwTextFrame *pNew,
        TextFrameIndex const nStart, TextFrameIndex const nEnd)
{
    SwSortedObjs *pObjs = nullptr;
    if ( nullptr != (pObjs = GetDrawObjs()) )
    {
        for ( size_t i = 0; GetDrawObjs() && i < pObjs->size(); ++i )
        {
            // Consider changed type of <SwSortedList> entries
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            const SwFormatAnchor& rAnch = pAnchoredObj->GetFrameFormat().GetAnchor();
            if (rAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
            {
                const SwPosition* pPos = rAnch.GetContentAnchor();
                TextFrameIndex const nIndex(MapModelToViewPos(*pPos));
                if (nStart <= nIndex && nIndex < nEnd)
                {
                    if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                    {
                        RemoveFly( static_cast<SwFlyFrame*>(pAnchoredObj) );
                        pNew->AppendFly( static_cast<SwFlyFrame*>(pAnchoredObj) );
                    }
                    else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchoredObj ) !=  nullptr )
                    {
                        RemoveDrawObj( *pAnchoredObj );
                        pNew->AppendDrawObj( *pAnchoredObj );
                    }
                    --i;
                }
            }
        }
    }
}

TextFrameIndex SwTextFrame::CalcFlyPos( SwFrameFormat const * pSearch )
{
    sw::MergedAttrIter iter(*this);
    for (SwTextAttr const* pHt = iter.NextAttr(); pHt; pHt = iter.NextAttr())
    {
        if( RES_TXTATR_FLYCNT == pHt->Which() )
        {
            SwFrameFormat* pFrameFormat = pHt->GetFlyCnt().GetFrameFormat();
            if( pFrameFormat == pSearch )
            {
                return TextFrameIndex(pHt->GetStart());
            }
        }
    }
    OSL_ENSURE(false, "CalcFlyPos: Not Found!");
    return TextFrameIndex(COMPLETE_STRING);
}

void sw::FlyContentPortion::Paint(const SwTextPaintInfo& rInf) const
{
    // Baseline output
    // Re-paint everything at a CompletePaint call
    SwRect aRepaintRect(rInf.GetPaintRect());

    if(rInf.GetTextFrame()->IsRightToLeft())
        rInf.GetTextFrame()->SwitchLTRtoRTL(aRepaintRect);

    if(rInf.GetTextFrame()->IsVertical())
        rInf.GetTextFrame()->SwitchHorizontalToVertical(aRepaintRect);

    if((m_pFly->IsCompletePaint() ||
            m_pFly->getFrameArea().IsOver(aRepaintRect)) &&
            SwFlyFrame::IsPaint(m_pFly->GetVirtDrawObj(), m_pFly->getRootFrame()->GetCurrShell()))
    {
        SwRect aRect(m_pFly->getFrameArea());
        if(!m_pFly->IsCompletePaint())
            aRect.Intersection_(aRepaintRect);

        // GetFlyFrame() may change the layout mode at the output device.
        {
            SwLayoutModeModifier aLayoutModeModifier(*rInf.GetOut());
            m_pFly->PaintSwFrame(const_cast<vcl::RenderContext&>(*rInf.GetOut()), aRect);
        }
        const_cast<SwTextPaintInfo&>(rInf).GetRefDev()->SetLayoutMode(rInf.GetOut()->GetLayoutMode());

        // As the OutputDevice might be anything, the font must be re-selected.
        // Being in const method should not be a problem.
        const_cast<SwTextPaintInfo&>(rInf).SelectFont();

        assert(rInf.GetVsh());
        SAL_WARN_IF(rInf.GetVsh()->GetOut() != rInf.GetOut(), "sw.core", "SwFlyCntPortion::Paint: Outdev has changed");
        if(rInf.GetVsh())
            const_cast<SwTextPaintInfo&>(rInf).SetOut(rInf.GetVsh()->GetOut());
    }
}

void sw::DrawFlyCntPortion::Paint(const SwTextPaintInfo&) const
{
    if(!m_pContact->GetAnchorFrame())
    {
        // No direct positioning of the drawing object is needed
        m_pContact->ConnectToLayout();
    }
}

/**
 * Use the dimensions of pFly->OutRect()
 */
SwFlyCntPortion::SwFlyCntPortion()
    : m_bMax(false)
    , m_eAlign(sw::LineAlign::NONE)
{
    nLineLength = TextFrameIndex(1);
    SetWhichPor(PortionType::FlyCnt);
}

sw::FlyContentPortion::FlyContentPortion(SwFlyInContentFrame* pFly)
    : m_pFly(pFly)
{
    SAL_WARN_IF(!pFly, "sw.core", "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInContentFrame!");
}

sw::DrawFlyCntPortion::DrawFlyCntPortion(SwFrameFormat const & rFormat)
    : m_pContact(nullptr)
{
    rFormat.CallSwClientNotify(sw::CreatePortionHint(&m_pContact));
    assert(m_pContact);
}

sw::FlyContentPortion* sw::FlyContentPortion::Create(const SwTextFrame& rFrame, SwFlyInContentFrame* pFly, const Point& rBase, long nLnAscent, long nLnDescent, long nFlyAsc, long nFlyDesc, AsCharFlags nFlags)
{
    auto pNew(new sw::FlyContentPortion(pFly));
    pNew->SetBase(rFrame, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags | AsCharFlags::UlSpace | AsCharFlags::Init);
    return pNew;
}

sw::DrawFlyCntPortion* sw::DrawFlyCntPortion::Create(const SwTextFrame& rFrame, SwFrameFormat const & rFormat, const Point& rBase, long nLnAscent, long nLnDescent, long nFlyAsc, long nFlyDesc, AsCharFlags nFlags)
{
    auto pNew(new DrawFlyCntPortion(rFormat));
    pNew->SetBase(rFrame, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags | AsCharFlags::UlSpace | AsCharFlags::Init);
    return pNew;
}

sw::DrawFlyCntPortion::~DrawFlyCntPortion() {};
sw::FlyContentPortion::~FlyContentPortion() {};

SdrObject* sw::FlyContentPortion::GetSdrObj(const SwTextFrame&)
{
    return m_pFly->GetVirtDrawObj();
}

SdrObject* sw::DrawFlyCntPortion::GetSdrObj(const SwTextFrame& rFrame)
{
    SdrObject* pSdrObj;
    // Determine drawing object ('master' or 'virtual') by frame
    pSdrObj = m_pContact->GetDrawObjectByAnchorFrame(rFrame);
    if(!pSdrObj)
    {
        SAL_WARN("sw.core", "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrame( rFrame )>");
        pSdrObj = m_pContact->GetMaster();
    }

    // Call <SwAnchoredDrawObject::MakeObjPos()> to assure that flag at
    // the <DrawFrameFormat> and at the <SwAnchoredDrawObject> instance are
    // correctly set
    if(pSdrObj)
        m_pContact->GetAnchoredObj(pSdrObj)->MakeObjPos();
    return pSdrObj;
}

/**
 * After setting the RefPoints, the ascent needs to be recalculated
 * because it is dependent on RelPos
 *
 * @param rBase CAUTION: needs to be an absolute value!
 */
void SwFlyCntPortion::SetBase( const SwTextFrame& rFrame, const Point &rBase,
                               long nLnAscent, long nLnDescent,
                               long nFlyAsc, long nFlyDesc,
                               AsCharFlags nFlags )
{
    // Use new class to position object
    // Determine drawing object
    SdrObject* pSdrObj = GetSdrObj(rFrame);
    if (!pSdrObj)
        return;

    // position object
    objectpositioning::SwAsCharAnchoredObjectPosition aObjPositioning(
                                    *pSdrObj,
                                    rBase, nFlags,
                                    nLnAscent, nLnDescent, nFlyAsc, nFlyDesc );

    // Scope of local variable <aObjPosInProgress>
    {
        SwObjPositioningInProgress aObjPosInProgress( *pSdrObj );
        aObjPositioning.CalcPosition();
    }

    SwFrameFormat* pShape = FindFrameFormat(pSdrObj);
    const SwFormatAnchor& rAnchor(pShape->GetAnchor());
    if (rAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
    {
        // This is an inline draw shape, see if it has a textbox.
        SwFrameFormat* pTextBox = SwTextBoxHelper::getOtherTextBoxFormat(pShape, RES_DRAWFRMFMT);
        if (pTextBox)
        {
            // It has, so look up its text rectangle, and adjust the position
            // of the textbox accordingly.
            // Both rectangles are absolute, SwFormatHori/VertOrient's position
            // is relative to the print area of the anchor text frame.
            tools::Rectangle aTextRectangle = SwTextBoxHelper::getTextRectangle(pShape);

            SwFormatHoriOrient aHori(pTextBox->GetHoriOrient());
            aHori.SetHoriOrient(css::text::HoriOrientation::NONE);
            sal_Int32 nLeft = aTextRectangle.getX() - rFrame.getFrameArea().Left()
                              - rFrame.getFramePrintArea().Left();
            aHori.SetPos(nLeft);

            SwFormatVertOrient aVert(pTextBox->GetVertOrient());
            aVert.SetVertOrient(css::text::VertOrientation::NONE);
            sal_Int32 const nTop = aTextRectangle.getY() - rFrame.getFrameArea().Top()
                                   - rFrame.getFramePrintArea().Top();
            aVert.SetPos(nTop);

            pTextBox->LockModify();
            pTextBox->SetFormatAttr(aHori);
            pTextBox->SetFormatAttr(aVert);
            pTextBox->UnlockModify();
        }
    }

    SetAlign( aObjPositioning.GetLineAlignment() );

    m_aRef = aObjPositioning.GetAnchorPos();
    if( nFlags & AsCharFlags::Rotate )
        SvXSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    else
        SvLSize( aObjPositioning.GetObjBoundRectInclSpacing().SSize() );
    if( Height() )
    {
        // GetRelPosY returns the relative position to baseline (if 0, the
        // upper border of the FlyCnt if on the baseline of a line)
        SwTwips nRelPos = aObjPositioning.GetRelPosY();
        if ( nRelPos < 0 )
        {
            nAscent = static_cast<sal_uInt16>(-nRelPos);
            if( nAscent > Height() )
                Height( nAscent );
        }
        else
        {
            nAscent = 0;
            Height( Height() + static_cast<sal_uInt16>(nRelPos) );
        }
    }
    else
    {
        Height( 1 );
        nAscent = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
