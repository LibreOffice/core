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

// INCLUDE ---------------------------------------------------------------

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

// -----------------------------------------------------------------------

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

using namespace com::sun::star;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSelection::FuSelection(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq ) :
    FuDraw(pViewSh, pWin, pViewP, pDoc, rReq),
    bVCAction(sal_False)
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
    //  special code for non-VCL OS2/UNX removed

    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());
    const bool bSelectionOnly = rMEvt.IsRight();
    if ( pView->IsAction() )
    {
        if ( bSelectionOnly )
            pView->BckAction();
        return sal_True;
    }

    bVCAction = sal_False;
    bIsInDragMode = sal_False;      //  irgendwo muss es ja zurueckgesetzt werden (#50033#)

    sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    aMDPos = pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aMDPos);
        SdrObject* pObj;

        if ( pHdl!=NULL || pView->IsMarkedObjHit(aMDPos) )
        {
            // Determine if this is the tail of a SdrCaptionObj i.e.
            // we need to disable the drag option on the tail of a note
            // object. Also, disable the ability to use the circular
            // drag of a note object.
            bool bDrag = false;
            const SdrObject* pSelected = pView->getSelectedIfSingle();

            if( pSelected )
            {
                if( ScDrawLayer::IsNoteCaption( *pSelected ) )
                {
                    // move using the valid caption handles for note text box.
                    if(pHdl && (pHdl->GetKind() != HDL_POLY && pHdl->GetKind() != HDL_CIRC))
                        bDrag = true;

                    // move the complete note box.
                    else if(!pHdl)
                        bDrag = true;
                }
                else
                    bDrag = true;   // different object
            }
            else
                bDrag = true;       // several objects

            if ( bDrag )
            {
                aDragTimer.Start();
                pView->BegDragObj(aMDPos, pHdl);
                bReturn = sal_True;
            }
        }
        else
        {
            sal_Bool bAlt = rMEvt.IsMod2();
            if ( !bAlt && pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, SDRSEARCH_PICKMACRO) )
            {
                pView->BegMacroObj(aMDPos, 2.0, pObj, pWindow);
                bReturn = sal_True;
            }
            else
            {
                String sURL, sTarget;
                if ( !bAlt && pView->PickObj(aMDPos, pView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER))
                {
                   // Support for imported Excel docs
                   // Excel is of course not consistent and allows
                   // a hyperlink to be assigned for an object group
                   // and even though the hyperlink is exported in the Escher layer
                   // its never used, when dealing with a group object the link
                   // associated with the clicked object is used only

                   // additionally you can also select a macro in Excel for a grouped
                   // objects and this results in the macro being set for the elements
                   // in the group and no macro is exported for the group

                   // if a macro and hlink are defined favour the hlink

                   // If a group object has no hyperlink use the hyperlink of the
                   // object clicked

                   if ( pObj->getChildrenOfSdrObject() )
                   {
                       SdrObject* pHit = NULL;
                       if ( pView->PickObj(aMDPos, pView->getHitTolLog(), pHit, SDRSEARCH_DEEP ) )
                           pObj = pHit;
                   }

                   ScMacroInfo* pInfo = ScDrawLayer::GetMacroInfo( pObj, sal_True );
#ifdef ISSUE66550_HLINK_FOR_SHAPES
                   // For interoperability favour links over macros if both are defined
                   if ( pInfo->GetHlink().getLength() > 0 )
                   {
                       OSL_TRACE("** Got URL");
                       sURL = pInfo->GetHlink();
                   }
                   else if ( pInfo->GetMacro().getLength() > 0 )
#else
                   if ( pInfo->GetMacro().getLength() > 0 )
#endif
                   {
                       SfxObjectShell* pObjSh = SfxObjectShell::Current();
                       if ( pObjSh && SfxApplication::IsXScriptURL( pInfo->GetMacro() ) )
                       {
                           uno::Any aRet;
                           uno::Sequence< sal_Int16 > aOutArgsIndex;
                           uno::Sequence< uno::Any > aOutArgs;
                           uno::Sequence< uno::Any >* pInArgs =
                               new uno::Sequence< uno::Any >(0);
                           pObjSh->CallXScript( pInfo->GetMacro(),
                               *pInArgs, aRet, aOutArgsIndex, aOutArgs);
                           pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                           return sal_True;        // kein CaptureMouse etc.
                       }
                   }
                }

                //  URL / ImageMap

                SdrViewEvent aVEvt;
                if ( !bAlt &&
                    pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt ) != SDRHIT_NONE &&
                    aVEvt.mpObj != NULL )
                {
                    if ( ScDrawLayer::GetIMapInfo( aVEvt.mpObj ) )      // ImageMap
                    {
                        const IMapObject* pIMapObj = ScDrawLayer::GetHitIMapObject( *aVEvt.mpObj, aMDPos, *pWindow );
                        if ( pIMapObj && pIMapObj->GetURL().Len() )
                        {
                            sURL = pIMapObj->GetURL();
                            sTarget = pIMapObj->GetTarget();
                        }
                    }
                    if ( aVEvt.meEvent == SDREVENT_EXECUTEURL && aVEvt.maURLField.Len() )   // URL
                    {
                        sURL = aVEvt.maURLField;
                        sTarget = aVEvt.maTargetFrame;
                    }
                }

                // open hyperlink, if found at object or in object's text
                if ( sURL.Len() > 0 )
                {
                    ScGlobal::OpenURL( sURL, sTarget );
                    pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                    return sal_True;        // kein CaptureMouse etc.
                }

                //  Is another object being edited in this view?
                SfxInPlaceClient* pClient = pViewShell->GetIPClient();
                sal_Bool bWasOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

                //  Markieren

                // do not allow multiselection with note caption
                bool bCaptionClicked = IsNoteCaptionClicked( aMDPos );
                if ( !rMEvt.IsShift() || bCaptionClicked || IsNoteCaptionMarked() )
                    pView->UnmarkAll();

                /*  Unlock internal layer, if a note caption is clicked */
                if( bCaptionClicked )
                    pView->UnlockInternalLayer();

                // try to select the clicked object
                if ( pView->MarkObj( aMDPos, 2.0, false, rMEvt.IsMod1() ) )
                {
                    //*********************************************************
                    //Objekt verschieben
                    //********************************************************
                    if (pView->IsMarkedObjHit(aMDPos))
                    {
                        //  #95834# Don't start drag timer if inplace editing of an OLE object
                        //  was just ended with this mouse click - the view will be moved
                        //  (different tool bars) and the object that was clicked on would
                        //  be moved unintentionally.
                        if ( !bWasOleActive )
                            aDragTimer.Start();

                        pHdl=pView->PickHandle(aMDPos);
                        pView->BegDragObj(aMDPos, pHdl);
                        bReturn = sal_True;
                    }
                    else                                    // Objekt am Rand getroffen
                        if (pViewShell->IsDrawSelMode())
                            bReturn = sal_True;
                }
                else
                {
                    //      nichts getroffen

                    if (pViewShell->IsDrawSelMode())
                    {
                        //*********************************************************
                        //Objekt selektieren
                        //********************************************************
                        pView->BegMarkObj(aMDPos);
                        bReturn = sal_True;
                    }
                }
            }
        }

    }

    if (!bIsInDragMode)
    {
        if (!bVCAction)                 // VC rufen selber CaptureMouse
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

sal_Bool __EXPORT FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        const basegfx::B2DPoint aOldPixel(pWindow->GetViewTransformation() * aMDPos);
        const basegfx::B2DPoint aNewPixel(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

        if ( fabs( aOldPixel.getX() - aNewPixel.getX() ) > SC_MAXDRAGMOVE ||
             fabs( aOldPixel.getY() - aNewPixel.getY() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->IsAction() )
    {
        const Point aPix(rMEvt.GetPosPixel());
        const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(aPix.X(), aPix.Y()));

        ForceScroll(aPix);
        pView->MovAction(aLogicPos);
        bReturn = sal_True;
    }

    // Ein VCControl ist aktiv
    // Event an den Manager weiterleiten
    if( bVCAction )
    {
        //  GetSbxForm gibts nicht mehr - Basic-Controls sind tot
        //SdrPageView* pPgView = pView->GetPageViewByIndex(0);
        //ScDrawPage*  pPage     = (ScDrawPage*)pPgView->GetPage();
        //VCSbxForm* pForm = (VCSbxForm*)(SbxObject*)(pPage->GetSbxForm());
        //((VCManager*)(pForm->GetVCContainer()))->
        //    MouseMove( pWindow, rMEvt );
        bReturn = sal_True;
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuDraw::MouseButtonUp(rMEvt);
//  sal_Bool bOle    = pViewShell->GetViewData()->IsOle();
    sal_Bool bOle = pViewShell->GetViewFrame()->GetFrame().IsInPlace();

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    const basegfx::B2DPoint aPnt(pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

    bool bCopy = false;
    ScViewData* pViewData = ( pViewShell ? pViewShell->GetViewData() : NULL );
    ScDocument* pDocument = ( pViewData ? pViewData->GetDocument() : NULL );
    SdrPageView* pPageView = ( pView ? pView->GetSdrPageView() : NULL );
    SdrPage* pPage = ( pPageView ? &pPageView->getSdrPageFromSdrPageView() : NULL );
    ::std::vector< ::rtl::OUString > aExcludedChartNames;
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
                    const SdrObjectVector aSelection(pView->getSelectedSdrObjectVectorFromSdrMarkView());

                    for ( sal_uInt32 i(0); i < aSelection.size(); ++i )
                    {
                        ScChartHelper::AddRangesIfProtectedChart( aProtectedChartRangesVector, pDocument, aSelection[i] );
                    }
                }
                bCopy = true;
            }

            pView->EndDragObj( rMEvt.IsMod1() );
            const SdrObject* pSelected = pView->getSelectedIfSingle();

            if (pSelected)
            {
                  FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                  FuText* pText = static_cast<FuText*>(pPoor);
                pText->StopDragMode(*pSelected);
            }
            bReturn = sal_True;
        }
        else if (pView->IsAction() )
        {
            // unlock internal layer to include note captions
            pView->UnlockInternalLayer();
            pView->EndAction();

            if(pView->areSdrObjectsSelected())
            {
                const SdrObjectVector aSelection(pView->getSelectedSdrObjectVectorFromSdrMarkView());
                bReturn = sal_True;

                /*  if multi-selection contains a note caption object, remove
                    all other objects from selection. */
                if( aSelection.size() > 1 )
                {
                    bool bFound = false;

                    for( sal_uInt32 nIdx = 0; !bFound && (nIdx < aSelection.size()); ++nIdx )
                    {
                        SdrObject* pObj = aSelection[nIdx];
                        bFound = ScDrawLayer::IsNoteCaption( *pObj );

                        if( bFound )
                        {
                            pView->UnmarkAllObj();
                            pView->MarkObj( *pObj );
                        }
                    }
                }
            }
        }
    }

/*
    if ( pView->IsObjEdit() )
    {
        sal_Bool bShowCursor = sal_True;
//!     pOutlinerView = pView->GetOutlinerView(pWindow, bShowCursor);
        bReturn = sal_True;
    }
*/
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
        SdrObject* pSelected = pView->getSelectedIfSingle();

        if ( pSelected )
        {
            //  #43984# aktivieren nur, wenn die Maus auch (noch) ueber dem
            //  selektierten Objekt steht
            SdrViewEvent aVEvt;
            SdrHitKind eHit = pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

            if ( eHit != SDRHIT_NONE && aVEvt.mpObj == pSelected )
            {
                //
                //  OLE: aktivieren
                //

                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pSelected) ;

                if (pSdrOle2Obj && !bOle && pSdrOle2Obj->GetObjRef().is())
                {
                    pViewShell->ActivateObject( pSdrOle2Obj, 0 );
                }

                //
                //  Edit text
                //  #49458# not in UNO controls
                //  #i32352# not in media objects
                //
                else if ( dynamic_cast< SdrTextObj* >(pSelected)
                    && !dynamic_cast< SdrUnoObj* >(pSelected)
                    && !dynamic_cast< SdrMediaObj* >(pSelected) )
                {
                    OutlinerParaObject* pOPO = pSelected->GetOutlinerParaObject();
                    sal_Bool bVertical = ( pOPO && pOPO->IsVertical() );
                    sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                    pViewShell->GetViewData()->GetDispatcher().
                        Execute(nTextSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

                    // jetzt den erzeugten FuText holen und in den EditModus setzen
                    FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                    if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // hat keine RTTI
                    {
                        FuText* pText = (FuText*)pPoor;
                        Point aMousePixel = rMEvt.GetPosPixel();
                        pText->SetInEditMode( pSelected, &aMousePixel );
                    }
                    bReturn = sal_True;
                }
            }
        }
        else if ( TestDetective( pView->GetSdrPageView(), aPnt ) )
        {
            bReturn = sal_True;
        }
    }

    // Ein VCControl ist aktiv
    // Event an den Manager weiterleiten
    if( bVCAction )
    {
        bVCAction = sal_False;
        bReturn = sal_True;
    }

    ForcePointer(&rMEvt);

    pWindow->ReleaseMouse();

    //  Command-Handler fuer Kontext-Menue kommt erst nach MouseButtonUp,
    //  darum hier die harte IsLeft-Abfrage
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

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return(bReturn);
}


/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuSelection::Activate()
{
/*
    SdrDragMode eMode;
    switch (aSfxRequest.GetSlot() )
    {
        case SID_OBJECT_SELECT:
            eMode = SDRDRAG_MOVE;
            break;
        case SID_OBJECT_ROTATE:
            eMode = SDRDRAG_ROTATE;
            break;
        case SID_OBJECT_MIRROR:
            eMode = SDRDRAG_MIRROR;
            break;
    }
    pView->SetDragMode(eMode);
*/
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
//    sal_Bool bShowCursor = sal_False;
//! pOutlinerView = pView->GetOutlinerView(pWindow, bShowCursor);

//  pView->SetDragMode(SDRDRAG_MOVE);
    FuDraw::Deactivate();
}


#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif





