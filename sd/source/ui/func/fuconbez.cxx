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

#include <com/sun/star/presentation/EffectNodeType.hpp>

#include <fuconbez.hxx>
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
#include <svx/xlnclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>

#include <app.hrc>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <ToolBarManager.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <CustomAnimationEffect.hxx>

using namespace ::com::sun::star::uno;

namespace sd {

/*//Extra attributes coming from parameters
    sal_uInt16  mnTransparence;  // Default: 0
    OUString    msColor;         // Default: ""
    sal_uInt16  mnWidth;         // Default: 0
    OUString    msShapeName;     // Default: ""*/
FuConstructBezierPolygon::FuConstructBezierPolygon (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
      nEditMode(SID_BEZIER_MOVE),
      mnTransparence(0),
      mnWidth(0)
{
}

namespace{

/// Checks to see if the request has a parameter of IsSticky:bool=true
/// It means that the selected command/button will stay selected after use
bool isSticky(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs ();
    if (pArgs)
    {
        const SfxBoolItem* pIsSticky = rReq.GetArg<SfxBoolItem>(FN_PARAM_4);
        if (pIsSticky && pIsSticky->GetValue())
            return true;
    }

    return false;
}

}

rtl::Reference<FuPoor> FuConstructBezierPolygon::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructBezierPolygon* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructBezierPolygon( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent || isSticky(rReq));
    return xFunc;
}

void FuConstructBezierPolygon::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
        return;

    const SfxPoolItem*  pPoolItem = nullptr;
    if( SfxItemState::SET == pArgs->GetItemState( SID_ADD_MOTION_PATH, true, &pPoolItem ) )
        maTargets = static_cast<const SfxUnoAnyItem*>( pPoolItem )->GetValue();

    if (nSlotId == SID_DRAW_FREELINE_NOFILL)
    {
        const SfxUInt16Item* pTransparence  = rReq.GetArg<SfxUInt16Item>(FN_PARAM_1);
        const SfxStringItem* pColor         = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        const SfxUInt16Item* pWidth         = rReq.GetArg<SfxUInt16Item>(FN_PARAM_3);
        const SfxStringItem* pShapeName     = rReq.GetArg<SfxStringItem>(SID_SHAPE_NAME);

        if (pTransparence && pTransparence->GetValue() > 0)
        {
            mnTransparence = pTransparence->GetValue();
        }
        if (pColor && !pColor->GetValue().isEmpty())
        {
            msColor = pColor->GetValue();
        }
        if (pWidth && pWidth->GetValue() > 0)
        {
            mnWidth = pWidth->GetValue();
        }
        if (pShapeName && !pShapeName->GetValue().isEmpty())
        {
            msShapeName = pShapeName->GetValue();
        }
    }
}

bool FuConstructBezierPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

    if (eHit == SdrHitKind::Handle || rMEvt.IsMod1())
    {
        mpView->SetEditMode(SdrViewEditMode::Edit);
    }
    else
    {
        mpView->SetEditMode(SdrViewEditMode::Create);
    }

    if (aVEvt.eEvent == SdrEventKind::BeginTextEdit)
    {
        // here, we do not allow text input
        aVEvt.eEvent = SdrEventKind::BeginDragObj;
        mpView->EnableExtendedMouseEventDispatcher(false);
    }
    else
    {
        mpView->EnableExtendedMouseEventDispatcher(true);
    }

    if (eHit == SdrHitKind::MarkedObject && nEditMode == SID_BEZIER_INSERT)
    {
        // insert glue point
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
            SetAttributes(aAttr, pObj);
            pObj->SetMergedItemSet(aAttr);
        }
    }

    return bReturn;
}

bool FuConstructBezierPolygon::MouseButtonUp(const MouseEvent& rMEvt )
{
    bool bReturn = false;
    bool bCreated = false;

    SdrViewEvent aVEvt;
    mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONUP, aVEvt);

    const size_t nCount = mpView->GetSdrPageView()->GetObjList()->GetObjCount();

    if (mpView->IsInsObjPoint())
    {
        mpView->EndInsObjPoint(SdrCreateCmd::ForceEnd);
    }
    else
    {
        mpView->MouseButtonUp(rMEvt, mpWindow);
    }

    if (aVEvt.eEvent == SdrEventKind::EndCreate)
    {
        bReturn = true;

        if (nCount+1 == mpView->GetSdrPageView()->GetObjList()->GetObjCount())
        {
            bCreated = true;
        }

        // trick to suppress FuDraw::DoubleClick
        bMBDown = false;

    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    bool bDeleted = false;
    if( bCreated && maTargets.hasValue() )
    {
        SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( mpView->GetSdrPageView()->GetObjList()->GetObj( nCount ) );
        SdPage* pPage = dynamic_cast< SdPage* >( pPathObj ? pPathObj->getSdrPageFromSdrObject() : nullptr );
        if( pPage )
        {
            std::shared_ptr< sd::MainSequence > pMainSequence( pPage->getMainSequence() );
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
                            pCreated->setNodeType( css::presentation::EffectNodeType::WITH_PREVIOUS );
                    }
                }
            }
        }
        mpView->DeleteMarked();
        bDeleted = true;
    }

    if ((!bPermanent && bCreated) || bDeleted)
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
    }

    return bReturn;
}

void FuConstructBezierPolygon::Activate()
{
    mpView->EnableExtendedMouseEventDispatcher(true);

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

    mpView->SetCurrentObj(static_cast<sal_uInt16>(eKind));

    FuConstruct::Activate();
}

void FuConstructBezierPolygon::Deactivate()
{
    mpView->EnableExtendedMouseEventDispatcher(false);

    FuConstruct::Deactivate();
}

void FuConstructBezierPolygon::SelectionHasChanged()
{
    FuDraw::SelectionHasChanged();

    mpViewShell->GetViewShellBase().GetToolBarManager()->SelectionHasChanged(
        *mpViewShell,
        *mpView);
}

namespace {
/// Returns the color based on the color names listed in core/include/tools/color.hxx
/// Feel free to extend with more color names from color.hxx
Color strToColor(const OUString& sColor)
{
    Color aColor = COL_AUTO;

    if (sColor == "COL_GRAY")
        aColor = COL_GRAY;
    else if (sColor == "COL_GRAY3")
        aColor = COL_GRAY3;
    else if (sColor == "COL_GRAY7")
        aColor = COL_GRAY7;

    return aColor;
}
}

void FuConstructBezierPolygon::SetAttributes(SfxItemSet& rAttr, SdrObject *pObj)
{
    if (nSlotId == SID_DRAW_FREELINE_NOFILL)
    {
        if (mnTransparence > 0 && mnTransparence <= 100)
            rAttr.Put(XLineTransparenceItem(mnTransparence));
        if (!msColor.isEmpty())
            rAttr.Put(XLineColorItem(OUString(), strToColor(msColor)));
        if (mnWidth > 0)
            rAttr.Put(XLineWidthItem(mnWidth));
        if (!msShapeName.isEmpty())
            pObj->SetName(msShapeName);
    }
}

/**
 * Set current bezier edit mode
 */
void FuConstructBezierPolygon::SetEditMode(sal_uInt16 nMode)
{
    nEditMode = nMode;
    ForcePointer();

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

SdrObjectUniquePtr FuConstructBezierPolygon::CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle)
{
    // case SID_DRAW_POLYGON:
    // case SID_DRAW_POLYGON_NOFILL:
    // case SID_DRAW_XPOLYGON:
    // case SID_DRAW_XPOLYGON_NOFILL:
    // case SID_DRAW_FREELINE:
    // case SID_DRAW_FREELINE_NOFILL:
    // case SID_DRAW_BEZIER_FILL:          // BASIC
    // case SID_DRAW_BEZIER_NOFILL:        // BASIC

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->GetCurrentObjInventor(),
        mpView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        if( auto pPathObj = dynamic_cast< SdrPathObj *>( pObj.get() ) )
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_FILL:
                {
                    const sal_Int32 nWdt(rRectangle.GetWidth() / 2);
                    const sal_Int32 nHgt(rRectangle.GetHeight() / 2);
                    const basegfx::B2DPolygon aInnerPoly(basegfx::utils::createPolygonFromEllipse(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()), nWdt, nHgt));

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

            pPathObj->SetPathPoly(aPoly);
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
