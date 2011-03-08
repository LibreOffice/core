/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"


#include <sfx2/bindings.hxx>
#include <svx/htmlmode.hxx>
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


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

ConstRectangle::ConstRectangle( SwWrtShell* pWrtShell, SwEditWin* pEditWin,
                                SwView* pSwView )
    : SwDrawBase( pWrtShell, pEditWin, pSwView ),
    bMarquee(FALSE),
    // #93382#
    mbVertical(sal_False)
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL ConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == TRUE
                                    && m_pWin->GetSdrDrawMode() == OBJ_CAPTION)
    {
        m_pView->NoRotate();
        if (m_pView->IsDrawSelMode())
        {
            m_pView->FlipDrawSelMode();
            m_pSh->GetDrawView()->SetFrameDragSingles(m_pView->IsDrawSelMode());
        }
    }
    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL ConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));

    BOOL bRet = SwDrawBase::MouseButtonUp(rMEvt);
    if( bRet )
    {
        SdrView *pSdrView = m_pSh->GetDrawView();
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        SdrObject* pObj = rMarkList.GetMark(0) ? rMarkList.GetMark(0)->GetMarkedSdrObj()
                                               : 0;
        switch( m_pWin->GetSdrDrawMode() )
        {
        case OBJ_TEXT:
            if( bMarquee )
            {
                m_pSh->ChgAnchor(FLY_AS_CHAR);

                if( pObj )
                {
                    // die fuer das Scrollen benoetigten Attribute setzen
                    SfxItemSet aItemSet( pSdrView->GetModel()->GetItemPool(),
                                        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                    aItemSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SCROLL ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SdrTextAniCountItem( 0 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                            (INT16)m_pWin->PixelToLogic(Size(2,1)).Width()) );

                    pObj->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }
            else if(mbVertical && pObj && pObj->ISA(SdrTextObj))
            {
                // #93382#
                SdrTextObj* pText = (SdrTextObj*)pObj;
                SfxItemSet aSet(pSdrView->GetModel()->GetItemPool());

                pText->SetVerticalWriting(TRUE);

                aSet.Put(SdrTextAutoGrowWidthItem(TRUE));
                aSet.Put(SdrTextAutoGrowHeightItem(FALSE));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }
            if( pObj )
            {
                SdrPageView* pPV = pSdrView->GetSdrPageView();
                m_pView->BeginTextEdit( pObj, pPV, m_pWin, sal_True );
            }
            m_pView->LeaveDrawCreate();  // In Selektionsmode wechseln
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
                    pOPO->SetVertical( TRUE );
            }
        }
        break;
        default:; //prevent warning
        }
    }
    return bRet;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void ConstRectangle::Activate(const USHORT nSlotId)
{
    bMarquee = bCapVertical = FALSE;
    mbVertical = sal_False;

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
        bMarquee = TRUE;
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_TEXT_VERTICAL:
        // #93382#
        mbVertical = sal_True;
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_TEXT:
        m_pWin->SetSdrDrawMode(OBJ_TEXT);
        break;

    case SID_DRAW_CAPTION_VERTICAL:
        bCapVertical = TRUE;
        // no break
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
