/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
#include <svx/svdlegacy.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include "app.hrc"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "ToolBarManager.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "CustomAnimationEffect.hxx"

using namespace ::com::sun::star::uno;

namespace sd {

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
        if( SFX_ITEM_SET == pArgs->GetItemState( SID_ADD_MOTION_PATH, true, &pPoolItem ) )
            maTargets = ( ( const SfxUnoAnyItem* ) pPoolItem )->GetValue();
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstructBezierPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_HANDLE || rMEvt.IsMod1())
    {
        mpView->SetViewEditMode(SDREDITMODE_EDIT);
    }
    else
    {
        mpView->SetViewEditMode(SDREDITMODE_CREATE);
    }

    if (aVEvt.meEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.meEvent = SDREVENT_BEGDRAGOBJ;
        mpView->EnableExtendedMouseEventDispatcher(false);
    }
    else
    {
        mpView->EnableExtendedMouseEventDispatcher(true);
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
            SfxItemSet aAttr(mpDoc->GetItemPool());
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

bool FuConstructBezierPolygon::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseMove(rMEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstructBezierPolygon::MouseButtonUp(const MouseEvent& rMEvt )
{
    bool bReturn = false;
    bool bCreated = false;

    SdrViewEvent aVEvt;
    mpView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);
    SdrPageView* pPV = mpView->GetSdrPageView();

    sal_uInt32 nCount = pPV ? pPV->GetCurrentObjectList()->GetObjCount() : 0;

    if (mpView->IsInsObjPoint())
    {
        mpView->EndInsObjPoint(SDRCREATE_FORCEEND);
    }
    else
    {
        mpView->MouseButtonUp(rMEvt, mpWindow);
    }

    if (aVEvt.meEvent == SDREVENT_ENDCREATE)
    {
        bReturn = true;

        if (pPV && nCount == (pPV->GetCurrentObjectList()->GetObjCount() - 1))
        {
            bCreated = true;
        }

        // Trick, um FuDraw::DoubleClick nicht auszuloesen
        bMBDown = false;
    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    bool bDeleted = false;
    if( bCreated && maTargets.hasValue() && pPV )
    {
        SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( pPV->GetCurrentObjectList()->GetObj( nCount ) );
        SdPage* pPage = dynamic_cast< SdPage* >( pPathObj ? pPathObj->getSdrPageFromSdrObject() : 0 );
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuConstructBezierPolygon::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Activate()
{
    mpView->EnableExtendedMouseEventDispatcher(true);
    bool bCreateFreehandMode(false);
    SdrPathObjType aSdrPathObjType(PathType_OpenPolygon);

    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            aSdrPathObjType = PathType_OpenPolygon;
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            aSdrPathObjType = PathType_ClosedPolygon;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            aSdrPathObjType = PathType_OpenBezier;
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            aSdrPathObjType = PathType_ClosedBezier;
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            aSdrPathObjType = PathType_OpenBezier;
            bCreateFreehandMode = true;
        }
        break;

        case SID_DRAW_FREELINE:
        {
            aSdrPathObjType = PathType_ClosedBezier;
            bCreateFreehandMode = true;
        }
        break;

        default:
        {
            aSdrPathObjType = PathType_OpenBezier;
        }
        break;
    }

    SdrObjectCreationInfo aSdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_POLY));

    aSdrObjectCreationInfo.setSdrPathObjType(aSdrPathObjType);
    aSdrObjectCreationInfo.setFreehandMode(bCreateFreehandMode);
    mpView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstructBezierPolygon::Deactivate()
{
    mpView->EnableExtendedMouseEventDispatcher(false);

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

// #97016#
SdrObject* FuConstructBezierPolygon::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
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
        mpView->getSdrModelFromSdrView(),
        mpView->getSdrObjectCreationInfo());

    if(pObj)
    {
        SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

        if(pSdrPathObj)
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_FILL:
                {
                    const double fWdt(rRange.getWidth() * 0.5);
                    const double fHgt(rRange.getHeight() * 0.5);
                    const basegfx::B2DPolygon aInnerPoly(basegfx::tools::createPolygonFromEllipse(rRange.getCenter(), fWdt, fHgt));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_BEZIER_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()));

                    const basegfx::B2DPoint aCenterBottom(rRange.getCenterX(), rRange.getMaxY());
                    aInnerPoly.appendBezierSegment(
                        aCenterBottom,
                        aCenterBottom,
                        rRange.getCenter());

                    const basegfx::B2DPoint aCenterTop(rRange.getCenterX(), rRange.getMinY());
                    aInnerPoly.appendBezierSegment(
                        aCenterTop,
                        aCenterTop,
                        basegfx::B2DPoint(rRange.getMaxX(), rRange.getMinY()));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_FREELINE:
                case SID_DRAW_FREELINE_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()));

                    aInnerPoly.appendBezierSegment(
                        rRange.getMinimum(),
                        basegfx::B2DPoint(rRange.getCenterX(), rRange.getMinY()),
                        rRange.getCenter());

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(rRange.getCenterX(), rRange.getMaxY()),
                        rRange.getMaximum(),
                        basegfx::B2DPoint(rRange.getMaxX(), rRange.getMinY()));

                    if(SID_DRAW_FREELINE == nID)
                    {
                        aInnerPoly.append(rRange.getMaximum());
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

                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()));
                    aInnerPoly.append(rRange.getMinimum());
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getCenterX(), rRange.getMinY()));
                    aInnerPoly.append(rRange.getCenter());
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMaxX(), rRange.getCenterY()));
                    aInnerPoly.append(rRange.getMaximum());

                    if(SID_DRAW_XPOLYGON_NOFILL == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRange.getCenterX(), rRange.getMaxY()));
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
                    const double fWdt(rRange.getWidth());
                    const double fHgt(rRange.getHeight());

                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMaxY()));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX() + fWdt * 0.3, rRange.getMinY() + fHgt * 0.7));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX(), rRange.getMinY() + fHgt * 0.15));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX() + fWdt * 0.65, rRange.getMinY()));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX() + fWdt, rRange.getMinY() + fHgt * 0.3));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX() + fWdt * 0.8, rRange.getMinY() + fHgt * 0.5));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMinX() + fWdt * 0.8, rRange.getMinY() + fHgt * 075));
                    aInnerPoly.append(basegfx::B2DPoint(rRange.getMaxY(), rRange.getMaxX()));

                    if(SID_DRAW_POLYGON_NOFILL == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRange.getCenterX(), rRange.getMaxY()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                    break;
                }
            }

            pSdrPathObj->setB2DPolyPolygonInObjectCoordinates(aPoly);
        }
        else
        {
            DBG_ERROR("Object is NO path object");
        }

        sdr::legacy::SetLogicRange(*pObj, rRange);
    }

    return pObj;
}

} // end of namespace sd

// eof
