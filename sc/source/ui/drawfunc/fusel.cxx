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

#include <com/sun/star/embed/EmbedStates.hpp>

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/imapobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>

#include "fusel.hxx"
#include "sc.hrc"
#include "fudraw.hxx"
#include "futext.hxx"
#include "drawview.hxx"
#include "tabvwsh.hxx"
#include "drawpage.hxx"
#include "globstr.hrc"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "scmod.hxx"
#include "client.hxx"
#include "charthelper.hxx"
#include "docuno.hxx"
#include "docsh.hxx"





#define SC_MAXDRAGMOVE  3

#define SC_MINDRAGMOVE 2



using namespace com::sun::star;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSelection::FuSelection(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq ) :
    FuDraw(pViewSh, pWin, pViewP, pDoc, rReq),
    bVCAction(false)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuSelection::~FuSelection()
{
}

sal_uInt8 FuSelection::Command(const CommandEvent& rCEvt)
{
    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());
    const bool bSelectionOnly = rMEvt.IsRight();
    if ( pView->IsAction() )
    {
        if ( bSelectionOnly )
            pView->BckAction();
        return true;
    }

    bVCAction = false;
    bIsInDragMode = false;      

    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        if ( pHdl!=NULL || pView->IsMarkedHit(aMDPos) )
        {
            
            
            
            
            bool bDrag = false;
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pMarkedObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( ScDrawLayer::IsNoteCaption( pMarkedObj ) )
                {
                    
                    if(pHdl && (pHdl->GetKind() != HDL_POLY && pHdl->GetKind() != HDL_CIRC))
                        bDrag = true;
                    
                    else if(!pHdl)
                        bDrag = true;
                }
                else
                    bDrag = true;   
            }
            else
                bDrag = true;       

            if ( bDrag )
            {
                aDragTimer.Start();
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                bReturn = true;
            }
        }
        else
        {
            SdrObject* pObj;
            SdrPageView* pPV;
            sal_Bool bAlt = rMEvt.IsMod2();
            if ( !bAlt && pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO) )
            {
                pView->BegMacroObj(aMDPos, pObj, pPV, pWindow);
                bReturn = true;
            }
            else
            {
                OUString sURL, sTarget;
                if ( !bAlt && pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER))
                {
                   
                   
                   
                   
                   
                   

                   
                   
                   
                   
                   
                   
                   

                   if ( pObj->IsGroupObject() )
                   {
                       ScMacroInfo* pTmpInfo = ScDrawLayer::GetMacroInfo( pObj );
                       if ( !pTmpInfo || pTmpInfo->GetMacro().isEmpty() )
                       {
                           SdrObject* pHit = NULL;
                           if ( pView->PickObj(aMDPos, pView->getHitTolLog(), pHit, pPV, SDRSEARCH_DEEP ) )
                               pObj = pHit;
                       }
                   }

                   ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, true );
                   
                   if ( !pInfo->GetHlink().isEmpty() )
                   {
                       OSL_TRACE("** Got URL");
                       sURL = pInfo->GetHlink();
                   }
                   else if ( !pInfo->GetMacro().isEmpty() )
                   {
                       SfxObjectShell* pObjSh = SfxObjectShell::Current();
                       if ( pObjSh && SfxApplication::IsXScriptURL( pInfo->GetMacro() ) )
                       {
                           uno::Reference< beans::XPropertySet > xProps( pObj->getUnoShape(), uno::UNO_QUERY );
                           uno::Any aCaller;
                           if ( xProps.is() )
                           {
                               try
                               {
                                   aCaller = xProps->getPropertyValue("Name");
                               }
                               catch( uno::Exception& ) {}
                           }
                           uno::Any aRet;
                           uno::Sequence< sal_Int16 > aOutArgsIndex;
                           uno::Sequence< uno::Any > aOutArgs;
                           uno::Sequence< uno::Any >* pInArgs =
                               new uno::Sequence< uno::Any >(0);
                           pObjSh->CallXScript( pInfo->GetMacro(),
                               *pInArgs, aRet, aOutArgsIndex, aOutArgs, true, &aCaller );
                           pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                           return true;        
                       }
                   }
                }

                

                SdrViewEvent aVEvt;
                if ( !bAlt &&
                    pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt ) != SDRHIT_NONE &&
                    aVEvt.pObj != NULL )
                {
                    if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) )       
                    {
                        const IMapObject* pIMapObj =
                                ScDrawLayer::GetHitIMapObject( aVEvt.pObj, aMDPos, *pWindow );
                        if ( pIMapObj && !pIMapObj->GetURL().isEmpty() )
                        {
                            sURL = pIMapObj->GetURL();
                            sTarget = pIMapObj->GetTarget();
                        }
                    }
                    if ( aVEvt.eEvent == SDREVENT_EXECUTEURL && aVEvt.pURLField )   
                    {
                        sURL = aVEvt.pURLField->GetURL();
                        sTarget = aVEvt.pURLField->GetTargetFrame();
                    }
                }

                
                if ( !sURL.isEmpty() )
                {
                    ScGlobal::OpenURL( sURL, sTarget );
                    pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                    return true;        
                }

                
                
                SfxInPlaceClient* pClient = pViewShell->GetIPClient();
                bool bWasOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

                

                
                bool bCaptionClicked = IsNoteCaptionClicked( aMDPos );
                if ( !rMEvt.IsShift() || bCaptionClicked || IsNoteCaptionMarked() )
                    pView->UnmarkAll();

                /*  Unlock internal layer, if a note caption is clicked. The
                    layer will be relocked in ScDrawView::MarkListHasChanged(). */
                if( bCaptionClicked )
                    pView->UnlockInternalLayer();

                
                if ( pView->MarkObj( aMDPos, -2, false, rMEvt.IsMod1() ) )
                {
                    
                    
                    
                    if (pView->IsMarkedHit(aMDPos))
                    {
                        
                        
                        
                        
                        if ( !bWasOleActive )
                            aDragTimer.Start();

                        pHdl=pView->PickHandle(aMDPos);
                        pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                        bReturn = true;
                    }
                    else                                    
                        if (pViewShell->IsDrawSelMode())
                            bReturn = true;
                }
                else
                {
                    

                    if (pViewShell->IsDrawSelMode())
                    {
                        
                        
                        
                        pView->BegMarkObj(aMDPos);
                        bReturn = true;
                    }
                }
            }
        }

    }

    if (!bIsInDragMode)
    {
        if (!bVCAction)                 
            pWindow->CaptureMouse();
        ForcePointer(&rMEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( std::abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             std::abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->IsAction() )
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
        bReturn = true;
    }

    
    
    if( bVCAction )
    {
        bReturn = true;
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuDraw::MouseButtonUp(rMEvt);
    bool bOle = pViewShell->GetViewFrame()->GetFrame().IsInPlace();

    SdrObject* pObj = NULL;
    SdrPageView* pPV = NULL;
    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    sal_uInt16 nDrgLog = sal_uInt16 ( pWindow->PixelToLogic(Size(SC_MINDRAGMOVE,0)).Width() );
    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    bool bCopy = false;
    ScViewData* pViewData = ( pViewShell ? pViewShell->GetViewData() : NULL );
    ScDocument* pDocument = ( pViewData ? pViewData->GetDocument() : NULL );
    SdrPageView* pPageView = ( pView ? pView->GetSdrPageView() : NULL );
    SdrPage* pPage = ( pPageView ? pPageView->GetPage() : NULL );
    ::std::vector< OUString > aExcludedChartNames;
    ScRangeListVector aProtectedChartRangesVector;

    if ( rMEvt.IsLeft() )
    {
        if ( pView->IsDragObj() )
        {
            /******************************************************************
            * Objekt wurde verschoben
            ******************************************************************/
            if ( rMEvt.IsMod1() )
            {
                if ( pPage )
                {
                    ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
                }
                if ( pView && pDocument )
                {
                    const SdrMarkList& rSdrMarkList = pView->GetMarkedObjectList();
                    sal_uLong nMarkCount = rSdrMarkList.GetMarkCount();
                    for ( sal_uLong i = 0; i < nMarkCount; ++i )
                    {
                        SdrMark* pMark = rSdrMarkList.GetMark( i );
                        pObj = ( pMark ? pMark->GetMarkedSdrObj() : NULL );
                        if ( pObj )
                        {
                            ScChartHelper::AddRangesIfProtectedChart( aProtectedChartRangesVector, pDocument, pObj );
                        }
                    }
                }
                bCopy = true;
            }

            if (!rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                std::abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
            {
                /*************************************************************
                * If a user wants to click on an object in front of a marked
                * one, he releases the mouse button immediately
                **************************************************************/
                if (pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_BEFOREMARK))
                {
                    pView->UnmarkAllObj();
                    pView->MarkObj(pObj,pPV,false,false);
                    return true;
                }
            }
            pView->EndDragObj( rMEvt.IsMod1() );
            pView->ForceMarkedToAnotherPage();

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMarkCount() == 1)
            {
                  SdrMark* pMark = rMarkList.GetMark(0);
                  pObj = pMark->GetMarkedSdrObj();
                  FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                  FuText* pText = static_cast<FuText*>(pPoor);
                pText->StopDragMode(pObj );
            }
            bReturn = true;
        }
        else if (pView->IsAction() )
        {
            
            pView->UnlockInternalLayer();
            pView->EndAction();
            if ( pView->AreObjectsMarked() )
            {
                bReturn = true;

                /*  if multi-selection contains a note caption object, remove
                    all other objects from selection. */
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                sal_uLong nCount = rMarkList.GetMarkCount();
                if( nCount > 1 )
                {
                    bool bFound = false;
                    for( sal_uLong nIdx = 0; !bFound && (nIdx < nCount); ++nIdx )
                    {
                        pObj = rMarkList.GetMark( nIdx )->GetMarkedSdrObj();
                        bFound = ScDrawLayer::IsNoteCaption( pObj );
                        if( bFound )
                        {
                            pView->UnMarkAll();
                            pView->MarkObj( pObj, pView->GetSdrPageView() );
                        }
                    }
                }
            }
        }
    }

    /**************************************************************************
    * Ggf. OLE-Objekt beruecksichtigen
    **************************************************************************/
    SfxInPlaceClient* pIPClient = pViewShell->GetIPClient();

    if (pIPClient)
    {
        ScModule* pScMod = SC_MOD();
        bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

        if ( pIPClient->IsObjectInPlaceActive() && !bUnoRefDialog )
            pIPClient->DeactivateObject();
    }

    sal_uInt16 nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( pView->AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                pObj = pMark->GetMarkedSdrObj();

                
                

                SdrViewEvent aVEvt;
                SdrHitKind eHit = pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt );
                if ( eHit != SDRHIT_NONE && aVEvt.pObj == pObj )
                {
                    sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

                    //
                    
                    //

                    if (nSdrObjKind == OBJ_OLE2)
                    {
                        if (!bOle)
                        {
                            if (((SdrOle2Obj*) pObj)->GetObjRef().is())
                            {
                                pViewShell->ActivateObject( (SdrOle2Obj*) pObj, 0 );
                            }
                        }
                    }

                    //
                    
                    
                    
                    //
                    else if ( pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) && !pObj->ISA(SdrMediaObj) )
                    {
                        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                        sal_Bool bVertical = ( pOPO && pOPO->IsVertical() );
                        sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                        pViewShell->GetViewData()->GetDispatcher().
                            Execute(nTextSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

                        
                        FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                        if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    
                        {
                            FuText* pText = (FuText*)pPoor;
                            Point aMousePixel = rMEvt.GetPosPixel();
                            pText->SetInEditMode( pObj, &aMousePixel );
                        }
                        bReturn = true;
                    }
                }
            }
        }
        else if ( TestDetective( pView->GetSdrPageView(), aPnt ) )
            bReturn = true;
    }

    
    
    if( bVCAction )
    {
        bVCAction = false;
        bReturn = true;
    }

    ForcePointer(&rMEvt);

    pWindow->ReleaseMouse();

    
    
    if ( !bReturn && rMEvt.IsLeft() )
        if (pViewShell->IsDrawSelMode())
            pViewShell->GetViewData()->GetDispatcher().
                Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

    if ( bCopy && pViewData && pDocument && pPage )
    {
        ScDocShell* pDocShell = pViewData->GetDocShell();
        ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : NULL );
        if ( pModelObj )
        {
            SCTAB nTab = pViewData->GetTabNo();
            ScChartHelper::CreateProtectedChartListenersAndNotify( pDocument, pPage, pModelObj, nTab,
                aProtectedChartRangesVector, aExcludedChartNames );
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

bool FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    return FuDraw::KeyInput(rKEvt);
}


/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuSelection::Activate()
{
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuSelection::Deactivate()
{
    /**************************************************************************
    * Hide Cursor
    **************************************************************************/
    FuDraw::Deactivate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
