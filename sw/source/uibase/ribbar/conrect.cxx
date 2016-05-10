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

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt))
                                    && m_pWin->GetSdrDrawMode() == OBJ_CAPTION)
    {
        m_pView->NoRotate();
        if (m_pView->IsDrawSelMode())
        {
            m_pView->FlipDrawSelMode();
            m_pSh->GetDrawView()->SetFrameDragSingles(m_pView->IsDrawSelMode());
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

void ConstRectangle::Activate(const sal_uInt16 nSlotId)
{
    bMarquee = bCapVertical = false;
    mbVertical = false;

    switch (nSlotId)
    {
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
