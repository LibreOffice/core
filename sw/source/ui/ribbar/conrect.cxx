/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conrect.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:24:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef sdtacitm_hxx //autogen
#include <svx/sdtacitm.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SDTAGITM_HXX //autogen
#include <svx/sdtagitm.hxx>
#endif
#ifndef SDTAKITM_HXX //autogen
#include <svx/sdtakitm.hxx>
#endif
#ifndef _SDTADITM_HXX //autogen
#include <svx/sdtaditm.hxx>
#endif
#ifndef _SDTAAITM_HXX //autogen
#include <svx/sdtaaitm.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDCAPT_HXX
#include <svx/svdocapt.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _CONRECT_HXX
#include <conrect.hxx>
#endif

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
                m_pSh->ChgAnchor(FLY_IN_CNTNT);

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
            if( bCapVertical && pObj )
            {
                SdrCaptionObj* pCaptObj = (SdrCaptionObj*)pObj;
                pCaptObj->ForceOutlinerParaObject();
                OutlinerParaObject* pOPO = pCaptObj->GetOutlinerParaObject();
                if( pOPO && !pOPO->IsVertical() )
                    pOPO->SetVertical( TRUE );
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



