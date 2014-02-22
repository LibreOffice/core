/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "fuconrec.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"

#include <editeng/outlobj.hxx>

#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include "scresid.hxx"




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

bool FuConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPos( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        
        
        
        
        Point aGridOff = CurrentGridSyncOffsetAndPos( aPos );
        pWindow->CaptureMouse();

        if ( pView->GetCurrentObjIdentifier() == OBJ_CAPTION )
        {
            Size aCaptionSize ( 2268, 1134 ); 

            bReturn = pView->BegCreateCaptionObj( aPos, aCaptionSize );

            
        }
        else
            bReturn = pView->BegCreateObj(aPos);
        if ( bReturn )
            pView->GetCreateObj()->SetGridOffset( aGridOff );
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstRectangle::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);

        if (aSfxRequest.GetSlot() == SID_DRAW_CAPTION_VERTICAL)
        {
            

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMark(0))
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                
                if ( pObj->ISA(SdrTextObj) )
                    ((SdrTextObj*)pObj)->ForceOutlinerParaObject();
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if( pOPO && !pOPO->IsVertical() )
                    pOPO->SetVertical( true );
            }
        }

        bReturn = true;
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

bool FuConstRectangle::KeyInput(const KeyEvent& rKEvt)
{
    return FuConstruct::KeyInput(rKEvt);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstRectangle::Activate()
{
    SdrObjKind aObjKind;

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_LINE:
            aNewPointer = Pointer( POINTER_DRAW_LINE );
            aObjKind = OBJ_LINE;
            break;

        case SID_DRAW_RECT:
            aNewPointer = Pointer( POINTER_DRAW_RECT );
            aObjKind = OBJ_RECT;
            break;

        case SID_DRAW_ELLIPSE:
            aNewPointer = Pointer( POINTER_DRAW_ELLIPSE );
            aObjKind = OBJ_CIRC;
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            aNewPointer = Pointer( POINTER_DRAW_CAPTION );
            aObjKind = OBJ_CAPTION;
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
            aObjKind = OBJ_RECT;
            break;
    }

    pView->SetCurrentObj(sal::static_int_cast<sal_uInt16>(aObjKind));

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

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


SdrObject* FuConstRectangle::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        Rectangle aRect(rRectangle);
        Point aStart = aRect.TopLeft();
        Point aEnd = aRect.BottomRight();

        switch(nID)
        {
            case SID_DRAW_LINE:
            {
                if(pObj->ISA(SdrPathObj))
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    basegfx::B2DPolygon aPoly;
                    aPoly.append(basegfx::B2DPoint(aStart.X(), nYMiddle));
                    aPoly.append(basegfx::B2DPoint(aEnd.X(), nYMiddle));
                    ((SdrPathObj*)pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aPoly));
                }
                else
                {
                    OSL_FAIL("Object is NO line object");
                }

                break;
            }
            case SID_DRAW_CAPTION:
            case SID_DRAW_CAPTION_VERTICAL:
            {
                if(pObj->ISA(SdrCaptionObj))
                {
                    sal_Bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    ((SdrTextObj*)pObj)->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    
                    
                    
                    
                    

                    ((SdrCaptionObj*)pObj)->SetLogicRect(aRect);
                    ((SdrCaptionObj*)pObj)->SetTailPos(
                        aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
                }
                else
                {
                    OSL_FAIL("Object is NO caption object");
                }

                break;
            }

            default:
            {
                pObj->SetLogicRect(aRect);

                break;
            }
        }

        SfxItemSet aAttr(pDrDoc->GetItemPool());
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
