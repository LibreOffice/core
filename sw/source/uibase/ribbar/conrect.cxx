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

#include <sfx2/bindings.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtaditm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <drawbase.hxx>
#include <conrect.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>

/**
 * set line starts and ends for the object to be created
 */

static const int DRGPIX = 2; // Drag MinMove in Pixel

::basegfx::B2DPolyPolygon getPolygon( sal_uInt16 nResId, SdrModel* pDoc )
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList = pDoc->GetLineEndList();

    if( pLineEndList.is() )
    {
        OUString aArrowName( SVX_RES(nResId) );
        long nCount = pLineEndList->Count();
        long nIndex;
        for( nIndex = 0L; nIndex < nCount; nIndex++ )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if( pEntry->GetName() == aArrowName )
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

ConstRectangle::ConstRectangle( SwWrtShell* pWrtShell, SwEditWin* pEditWin,
                                SwView* pSwView )
    : SwDrawBase( pWrtShell, pEditWin, pSwView )
    , bMarquee(false)
    , bCapVertical(false)
    , mbVertical(false)
{
}

bool ConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn;

    if ( rMEvt.IsLeft() && (bReturn = SwDrawBase::MouseButtonDown(rMEvt)))
    {
        m_pView->NoRotate();
        if (m_pView->IsDrawSelMode())
        {
            m_pView->FlipDrawSelMode();
            m_pSh->GetDrawView()->SetFrameDragSingles(m_pView->IsDrawSelMode());
        }

        Point aPnt( m_pWin->PixelToLogic( rMEvt.GetPosPixel() ) );
        m_pWin->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( m_pWin->PixelToLogic( Size( DRGPIX,0 )).Width() );

        if (m_pWin->GetSdrDrawMode() == OBJ_CAPTION)
        {
            Size aCaptionSize(846, 846);    // (4x2)cm
            bReturn = m_pView->GetDrawView()->BegCreateCaptionObj(aPnt, aCaptionSize,
                                                nullptr, nDrgLog);
        }
        else
        {
            m_pView->GetDrawView()->BegCreateObj(aPnt, nullptr, nDrgLog);
        }

        SdrObject* pObj = m_pView->GetDrawView()->GetCreateObj();
        if (pObj)
        {
            SfxItemSet aAttr(pObj->GetModel()->GetItemPool());
            SetLineEnds(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);
        }

    }
    return bReturn;
}

bool ConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bRet = SwDrawBase::MouseButtonUp(rMEvt);
    if( bRet )
    {
        SdrView *pSdrView = m_pSh->GetDrawView();
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        SdrObject* pObj = rMarkList.GetMark(0) ? rMarkList.GetMark(0)->GetMarkedSdrObj()
                                               : nullptr;
        switch( m_pWin->GetSdrDrawMode() )
        {
        case OBJ_TEXT:
            if( bMarquee )
            {
                m_pSh->ChgAnchor(FLY_AS_CHAR);

                if( pObj )
                {
                    // Set the attributes needed for scrolling
                    SfxItemSet aItemSet( pSdrView->GetModel()->GetItemPool(),
                                        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                    aItemSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SCROLL ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SdrTextAniCountItem( 0 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                            (sal_Int16)m_pWin->PixelToLogic(Size(2,1)).Width()) );

                    pObj->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }
            else if(mbVertical && pObj && dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr)
            {
                SdrTextObj* pText = static_cast<SdrTextObj*>(pObj);
                SfxItemSet aSet(pSdrView->GetModel()->GetItemPool());

                pText->SetVerticalWriting(true);

                aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }
            if( pObj )
            {
                SdrPageView* pPV = pSdrView->GetSdrPageView();
                m_pView->BeginTextEdit( pObj, pPV, m_pWin, true );
            }
            m_pView->LeaveDrawCreate();  // Switch to selection mode
            m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            break;

        case OBJ_CAPTION:
        {
            SdrCaptionObj* pCaptObj = dynamic_cast<SdrCaptionObj*>(pObj);
            if( bCapVertical && pCaptObj )
            {
                pCaptObj->ForceOutlinerParaObject();
                OutlinerParaObject* pOPO = pCaptObj->GetOutlinerParaObject();
                if( pOPO && !pOPO->IsVertical() )
                    pOPO->SetVertical( true );
            }
        }
        break;

        default:; //prevent warning
        }
    }
    return bRet;
}

void ConstRectangle::SetLineEnds(SfxItemSet& rAttr, SdrObject* pObj)
{
    SdrModel *mpDoc = pObj->GetModel();

    if ( m_nSlotId == SID_LINE_ARROW_START      ||
         m_nSlotId == SID_LINE_ARROW_END        ||
         m_nSlotId == SID_LINE_ARROWS           ||
         m_nSlotId == SID_LINE_ARROW_CIRCLE     ||
         m_nSlotId == SID_LINE_CIRCLE_ARROW     ||
         m_nSlotId == SID_LINE_ARROW_SQUARE     ||
         m_nSlotId == SID_LINE_SQUARE_ARROW )
    {

        // set attributes of line start and ends

        // arrowhead
        ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, mpDoc ) );
        if( !aArrow.count() )
        {
            ::basegfx::B2DPolygon aNewArrow;
            aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
            aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
            aNewArrow.setClosed(true);
            aArrow.append(aNewArrow);
        }

        // Circles
        ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, mpDoc ) );
        if( !aCircle.count() )
        {
            ::basegfx::B2DPolygon aNewCircle;
            aNewCircle = ::basegfx::tools::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
            aNewCircle.setClosed(true);
            aCircle.append(aNewCircle);
        }

        // Square
        ::basegfx::B2DPolyPolygon aSquare( getPolygon( RID_SVXSTR_SQUARE, mpDoc ) );
        if( !aSquare.count() )
        {
            ::basegfx::B2DPolygon aNewSquare;
            aNewSquare.append(::basegfx::B2DPoint(0.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 10.0));
            aNewSquare.append(::basegfx::B2DPoint(0.0, 10.0));
            aNewSquare.setClosed(true);
            aSquare.append(aNewSquare);
        }

        SfxItemSet aSet( mpDoc->GetItemPool() );
        long nWidth = 100; // (1/100th mm)

        // determine line width and calculate with it the line end width
        if( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
        {
            long nValue = static_cast<const XLineWidthItem&>( aSet.Get( XATTR_LINEWIDTH ) ).GetValue();
            if( nValue > 0 )
                nWidth = nValue * 3;
        }

        switch (m_nSlotId)
        {
            case SID_LINE_ARROWS:
            {
                // connector with arrow ends
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_START:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_ARROW_SQUARE:
            {
                // connector with arrow start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_END:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_SQUARE_ARROW:
            {
                // connector with arrow end
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;
        }

        // and again, for the still missing ends
        switch (m_nSlotId)
        {
            case SID_LINE_ARROW_CIRCLE:
            {
                // circle end
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_CIRCLE_ARROW:
            {
                // circle start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_SQUARE:
            {
                // square end
                rAttr.Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_SQUARE_ARROW:
            {
                // square start
                rAttr.Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;
        }
    }
}

void ConstRectangle::Activate(const sal_uInt16 nSlotId)
{
    bMarquee = bCapVertical = false;
    mbVertical = false;

    switch (nSlotId)
    {
    case SID_LINE_ARROW_END:
    case SID_LINE_ARROW_CIRCLE:
    case SID_LINE_ARROW_SQUARE:
    case SID_LINE_ARROW_START:
    case SID_LINE_CIRCLE_ARROW:
    case SID_LINE_SQUARE_ARROW:
    case SID_LINE_ARROWS:
    case SID_DRAW_LINE:
        m_pWin->SetSdrDrawMode(OBJ_LINE);
        break;

    case SID_DRAW_RECT:
        m_pWin->SetSdrDrawMode(OBJ_RECT);
        break;

    case SID_DRAW_ELLIPSE:
        m_pWin->SetSdrDrawMode(OBJ_CIRC);
        break;

    case SID_DRAW_TEXT_MARQUEE:
        bMarquee = true;
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_TEXT_VERTICAL:
        mbVertical = true;
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_TEXT:
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_CAPTION_VERTICAL:
        bCapVertical = true;
        SAL_FALLTHROUGH;
    case SID_DRAW_CAPTION:
        m_pWin->SetSdrDrawMode(OBJ_CAPTION);
        break;

    default:
        m_pWin->SetSdrDrawMode(OBJ_NONE);
        break;
    }

    SwDrawBase::Activate(nSlotId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
