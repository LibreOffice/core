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


#include <com/sun/star/presentation/EffectNodeType.hpp>

#include "fuconbez.hxx"
#include <svl/aeitem.hxx>
#include <svx/svdopath.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>


#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>

#include "app.hrc"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "ToolBarManager.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include "CustomAnimationEffect.hxx"

using namespace ::com::sun::star::uno;

namespace sd {

TYPEINIT1( FuConstructBezierPolygon, FuConstruct );


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstructBezierPolygon::FuConstructBezierPolygon (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
      nEditMode(SID_BEZIER_MOVE)
{
}

FunctionReference FuConstructBezierPolygon::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructBezierPolygon* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructBezierPolygon( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructBezierPolygon::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    const SfxItemSet* pArgs = rReq.GetArgs();
    if( pArgs )
    {
        const SfxPoolItem*  pPoolItem = NULL;
        if( SFX_ITEM_SET == pArgs->GetItemState( SID_ADD_MOTION_PATH, sal_True, &pPoolItem ) )
            maTargets = ( ( const SfxUnoAnyItem* ) pPoolItem )->GetValue();
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuConstructBezierPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_HANDLE || rMEvt.IsMod1())
    {
        mpView->SetEditMode(SDREDITMODE_EDIT);
    }
    else
    {
        mpView->SetEditMode(SDREDITMODE_CREATE);
    }

    if (aVEvt.eEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.eEvent = SDREVENT_BEGDRAGOBJ;
        mpView->EnableExtendedMouseEventDispatcher(sal_False);
    }
    else
    {
        mpView->EnableExtendedMouseEventDispatcher(sal_True);
    }

    if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
    {
        /******************************************************************
        * Klebepunkt einfuegen
        ******************************************************************/
        mpView->BegInsObjPoint(aMDPos, rMEvt.IsMod1());
    }
    else
    {
        mpView->MouseButtonDown(rMEvt, mpWindow);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstructBezierPolygon::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuConstruct::MouseMove(rMEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstructBezierPolygon::MouseButtonUp(const MouseEvent& rMEvt )
{
    sal_Bool bReturn = sal_False;
    sal_Bool bCreated = sal_False;

    SdrViewEvent aVEvt;
    mpView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);

    sal_uLong nCount = mpView->GetSdrPageView()->GetObjList()->GetObjCount();

    if (mpView->IsInsObjPoint())
    {
        mpView->EndInsObjPoint(SDRCREATE_FORCEEND);
    }
    else
    {
        mpView->MouseButtonUp(rMEvt, mpWindow);
    }

    if (aVEvt.eEvent == SDREVENT_ENDCREATE)
    {
        bReturn = sal_True;

        if (nCount == (mpView->GetSdrPageView()->GetObjList()->GetObjCount() - 1))
        {
            bCreated = sal_True;
        }

        // Trick, um FuDraw::DoubleClick nicht auszuloesen
        bMBDown = sal_False;

    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    bool bDeleted = false;
    if( bCreated && maTargets.hasValue() )
    {
        SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( mpView->GetSdrPageView()->GetObjList()->GetObj( nCount ) );
        SdPage* pPage = dynamic_cast< SdPage* >( pPathObj ? pPathObj->GetPage() : 0 );
        if( pPage )
        {
            boost::shared_ptr< sd::MainSequence > pMainSequence( pPage->getMainSequence() );
            if( pMainSequence.get() )
            {
                Sequence< Any > aTargets;
                maTargets >>= aTargets;

                sal_Int32 nTCount = aTargets.getLength();
                if( nTCount > 1 )
                {
                    const Any* pTarget = aTargets.getConstArray();
                    double fDuration = 0.0;
                    *pTarget++ >>= fDuration;
                    bool bFirst = true;
                    while( --nTCount )
                    {
                        CustomAnimationEffectPtr pCreated( pMainSequence->append( *pPathObj, *pTarget++, fDuration ) );
                        if( bFirst )
                            bFirst = false;
                        else
                            pCreated->setNodeType( ::com::sun::star::presentation::EffectNodeType::WITH_PREVIOUS );
                    }
                }
            }
        }
        mpView->DeleteMarked();
        bDeleted = true;
    }

    if ((!bPermanent && bCreated) || bDeleted)
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuConstructBezierPolygon::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Activate()
{
    mpView->EnableExtendedMouseEventDispatcher(sal_True);

    SdrObjKind eKind;

    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            eKind = OBJ_PLIN;
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            eKind = OBJ_POLY;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            eKind = OBJ_PATHLINE;
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            eKind = OBJ_PATHFILL;
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            eKind = OBJ_FREELINE;
        }
        break;

        case SID_DRAW_FREELINE:
        {
            eKind = OBJ_FREEFILL;
        }
        break;

        default:
        {
            eKind = OBJ_PATHLINE;
        }
        break;
    }

    mpView->SetCurrentObj((sal_uInt16)eKind);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Deactivate()
{
    mpView->EnableExtendedMouseEventDispatcher(sal_False);

    FuConstruct::Deactivate();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuConstructBezierPolygon::SelectionHasChanged()
{
    FuDraw::SelectionHasChanged();

    mpViewShell->GetViewShellBase().GetToolBarManager()->SelectionHasChanged(
        *mpViewShell,
        *mpView);
}



/*************************************************************************
|*
|* Aktuellen Bezier-Editmodus setzen
|*
\************************************************************************/

void FuConstructBezierPolygon::SetEditMode(sal_uInt16 nMode)
{
    nEditMode = nMode;
    ForcePointer();

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

SdrObject* FuConstructBezierPolygon::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_POLYGON:
    // case SID_DRAW_POLYGON_NOFILL:
    // case SID_DRAW_XPOLYGON:
    // case SID_DRAW_XPOLYGON_NOFILL:
    // case SID_DRAW_FREELINE:
    // case SID_DRAW_FREELINE_NOFILL:
    // case SID_DRAW_BEZIER_FILL:          // BASIC
    // case SID_DRAW_BEZIER_NOFILL:        // BASIC

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        0L, mpDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrPathObj))
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_FILL:
                {
                    const sal_Int32 nWdt(rRectangle.GetWidth() / 2);
                    const sal_Int32 nHgt(rRectangle.GetHeight() / 2);
                    const basegfx::B2DPolygon aInnerPoly(basegfx::tools::createPolygonFromEllipse(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()), nWdt, nHgt));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_BEZIER_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));

                    const basegfx::B2DPoint aCenterBottom(rRectangle.Center().X(), rRectangle.Bottom());
                    aInnerPoly.appendBezierSegment(
                        aCenterBottom,
                        aCenterBottom,
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()));

                    const basegfx::B2DPoint aCenterTop(rRectangle.Center().X(), rRectangle.Top());
                    aInnerPoly.appendBezierSegment(
                        aCenterTop,
                        aCenterTop,
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_FREELINE:
                case SID_DRAW_FREELINE_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top()),
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Top()),
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Bottom()),
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()),
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));

                    if(SID_DRAW_FREELINE == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_XPOLYGON:
                case SID_DRAW_XPOLYGON_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Top()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Center().Y()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()));

                    if(SID_DRAW_XPOLYGON_NOFILL == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Bottom()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_POLYGON:
                case SID_DRAW_POLYGON_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;
                    const sal_Int32 nWdt(rRectangle.GetWidth());
                    const sal_Int32 nHgt(rRectangle.GetHeight());

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 30) / 100, rRectangle.Top() + (nHgt * 70) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top() + (nHgt * 15) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 65) / 100, rRectangle.Top()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + nWdt, rRectangle.Top() + (nHgt * 30) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 80) / 100, rRectangle.Top() + (nHgt * 50) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 80) / 100, rRectangle.Top() + (nHgt * 75) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Bottom(), rRectangle.Right()));

                    if(SID_DRAW_POLYGON_NOFILL == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Bottom()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                    break;
                }
            }

            ((SdrPathObj*)pObj)->SetPathPoly(aPoly);
        }
        else
        {
            OSL_FAIL("Object is NO path object");
        }

        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
