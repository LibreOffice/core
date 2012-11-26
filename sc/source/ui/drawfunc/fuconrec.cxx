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
#include "precompiled_sc.hxx"

#include "fuconrec.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"
#include <editeng/outlobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrobjectfactory.hxx>

#include "scresid.hxx"

//------------------------------------------------------------------------


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstRectangle::FuConstRectangle(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstRectangle::~FuConstRectangle()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

        pWindow->CaptureMouse();

        if ( OBJ_CAPTION == pView->getSdrObjectCreationInfo().getIdent() )
        {
            bReturn = pView->BegCreateCaptionObj( aLogicPos, basegfx::B2DVector(2268.0, 1134.0)); // 4x2cm

            // wie stellt man den Font ein, mit dem geschrieben wird
        }
        else
        {
            bReturn = pView->BegCreateObj(aLogicPos);
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = sal_False;

    if ( pView->GetCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj(SDRCREATE_FORCEEND);

        if (aSfxRequest.GetSlot() == SID_DRAW_CAPTION_VERTICAL)
        {
            //  set vertical flag for caption object
            SdrObject* pSelected = pView->getSelectedIfSingle();

            if (pSelected)
            {
                //  create OutlinerParaObject now so it can be set to vertical
                SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pSelected);

                if ( pSdrTextObj )
                    pSdrTextObj->ForceOutlinerParaObject();

                OutlinerParaObject* pOPO = pSelected->GetOutlinerParaObject();

                if( pOPO && !pOPO->IsVertical() )
                    pOPO->SetVertical( sal_True );
            }
        }

        bReturn = sal_True;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool __EXPORT FuConstRectangle::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstRectangle::Activate()
{
    SdrObjectCreationInfo aSdrObjectCreationInfo(OBJ_RECT);

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_LINE:
            aNewPointer = Pointer( POINTER_DRAW_LINE );
            aSdrObjectCreationInfo.setIdent(OBJ_POLY);
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_Line);
            break;

        case SID_DRAW_RECT:
            aNewPointer = Pointer( POINTER_DRAW_RECT );
            break;

        case SID_DRAW_ELLIPSE:
            aSdrObjectCreationInfo.setIdent(OBJ_CIRC);
            aNewPointer = Pointer( POINTER_DRAW_ELLIPSE );
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            aSdrObjectCreationInfo.setIdent(OBJ_CAPTION);
            aNewPointer = Pointer( POINTER_DRAW_CAPTION );
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
            break;
    }

    pView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer(aNewPointer);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstRectangle::Deactivate()
{
    FuConstruct::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
}

// #98185# Create default drawing objects via keyboard
SdrObject* FuConstRectangle::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    // case SID_DRAW_LINE:
    // case SID_DRAW_RECT:
    // case SID_DRAW_ELLIPSE:
    // case SID_DRAW_CAPTION:
    // case SID_DRAW_CAPTION_VERTICAL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->getSdrModelFromSdrView(),
        pView->getSdrObjectCreationInfo());

    if(pObj)
    {
        switch(nID)
        {
            case SID_DRAW_LINE:
            {
                SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                if(pSdrPathObj)
                {
                    const double fYMiddle((rRange.getMinY() + rRange.getMaxY()) * 0.5);
                    basegfx::B2DPolygon aPoly;
                    aPoly.append(basegfx::B2DPoint(rRange.getMinX(), fYMiddle));
                    aPoly.append(basegfx::B2DPoint(rRange.getMaxX(), fYMiddle));
                    pSdrPathObj->setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aPoly));
                }
                else
                {
                    DBG_ERROR("Object is NO line object");
                }

                break;
            }
            case SID_DRAW_CAPTION:
            case SID_DRAW_CAPTION_VERTICAL:
            {
                SdrCaptionObj* pSdrCaptionObj = dynamic_cast< SdrCaptionObj* >(pObj);

                if(pSdrCaptionObj)
                {
                    sal_Bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    pSdrCaptionObj->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pSdrCaptionObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pSdrCaptionObj->SetMergedItemSet(aSet);
                    }

                    //  #105815# don't set default text, start edit mode instead
                    //  (Edit mode is started in ScTabViewShell::ExecDraw, because
                    //  it must be handled by FuText)
                    // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
                    // pSdrCaptionObj->SetText(aText);

                    sdr::legacy::SetLogicRange(*pSdrCaptionObj, rRange);
                    pSdrCaptionObj->SetTailPos(rRange.getMinimum() - (rRange.getRange() * 0.5));
                }
                else
                {
                    DBG_ERROR("Object is NO caption object");
                }

                break;
            }

            default:
            {
                sdr::legacy::SetLogicRange(*pObj, rRange);

                break;
            }
        }

        SfxItemSet aAttr(pDrDoc->GetItemPool());
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

