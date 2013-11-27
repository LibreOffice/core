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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include "DrawViewShell.hxx"
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/fmglob.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <svx/globl3d.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopath.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/editview.hxx>
#include <vcl/cursor.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fusnapln.hxx"
#include "app.hxx"
#include "Ruler.hxx"
#include "sdresid.hxx"
#include "GraphicViewShell.hxx"
#include "sdpage.hxx"
#include "slideshow.hxx"
#include "anminfo.hxx"
#include "sdpopup.hxx"
#include "drawview.hxx"
#include <svx/bmpmask.hxx>
#include "LayerTabBar.hxx"
#include <svx/svdlegacy.hxx>
#include <svx/svditer.hxx>

//IAccessibility2 Implementation 2009-----
#include <navigatr.hxx>
//-----IAccessibility2 Implementation 2009
namespace sd {

#define PIPETTE_RANGE 0

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;

/*************************************************************************
|*
|* aktuelle Seite loeschen
|*
\************************************************************************/

void DrawViewShell::DeleteActualPage()
{
    sal_uInt16          nPage = maTabControl.GetCurPageId() - 1;

    mpDrawView->SdrEndTextEdit();

    try
    {
        Reference<XDrawPagesSupplier> xDrawPagesSupplier( GetDoc()->getUnoModel(), UNO_QUERY_THROW );
        Reference<XDrawPages> xPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
        Reference< XDrawPage > xPage( xPages->getByIndex( nPage ), UNO_QUERY_THROW );
        xPages->remove( xPage );
    }
    catch( Exception& )
    {
        DBG_ERROR("SelectionManager::DeleteSelectedMasterPages(), exception caught!");
    }
}

/*************************************************************************
|*
|* aktuelle Ebene loeschen
|*
\************************************************************************/

void DrawViewShell::DeleteActualLayer()
{
    SdrLayerAdmin& rAdmin = GetDoc()->GetModelLayerAdmin();
    const String&  rName  = GetLayerTabControl()->GetPageText(GetLayerTabControl()->GetCurPageId());
    String         aString(SdResId(STR_ASK_DELETE_LAYER));

    // Platzhalter ersetzen
    sal_uInt16 nPos = aString.Search(sal_Unicode('$'));
    aString.Erase(nPos, 1);
    aString.Insert(rName, nPos);

    if (QueryBox(GetActiveWindow(), WB_YES_NO, aString).Execute() == RET_YES)
    {
        const SdrLayer* pLayer = rAdmin.GetLayer(rName, false);
        mpDrawView->DeleteLayer( pLayer->GetName() );

        // damit TabBar und Window neu gezeichnet werden;
        // sollte spaeter wie beim Aendern der Layerfolge durch einen
        // Hint von Joe angestossen werden
        // ( View::Notify() --> ViewShell::ResetActualLayer() )

        mbIsLayerModeActive = false;    // damit ChangeEditMode() ueberhaupt was tut
        ChangeEditMode(GetEditMode(), true);
    }
}


/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

bool DrawViewShell::KeyInput (const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    bool bRet = false;

    if ( !IsInputLocked() || ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE ) )
    {
        // #97016# IV
        if(KEY_RETURN == rKEvt.GetKeyCode().GetCode()
            && rKEvt.GetKeyCode().IsMod1()
            && GetView()->IsTextEdit())
        {
            // this should be used for cursor travelling.
            SdPage* pActualPage = GetActualPage();
            SdrTextObj* pCandidate = 0L;
            SdrObject* pSelected = GetView()->getSelectedIfSingle();

            if(pActualPage && pSelected)
            {
                // end text edit now
                GetView()->SdrEndTextEdit();

                // look for a new candidate, a successor of pSelected
                SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);
                bool bDidVisitOldObject(false);

                while(aIter.IsMore() && !pCandidate)
                {
                    SdrTextObj* pObj = dynamic_cast< SdrTextObj* >(aIter.Next());

                    if(pObj)
                    {
                        sal_uInt32 nInv(pObj->GetObjInventor());
                        sal_uInt16 nKnd(pObj->GetObjIdentifier());

                        if(SdrInventor == nInv &&
                            (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd)
                            && bDidVisitOldObject)
                        {
                            pCandidate = pObj;
                        }

                        if(pObj == pSelected)
                        {
                            bDidVisitOldObject = true;
                        }
                    }
                }
            }

            if(pCandidate)
            {
                // set the new candidate to text edit mode
                GetView()->UnmarkAllObj();
                GetView()->MarkObj(*pCandidate);

                GetViewFrame()->GetDispatcher()->Execute(
                    SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON);
            }
            else
            {
                // insert a new page with the same page layout
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_INSERTPAGE_QUICK, SFX_CALLMODE_ASYNCHRON);
            }
        }
        else
        {
            bRet = ViewShell::KeyInput(rKEvt, pWin);
//IAccessibility2 Implementation 2009-----
            //Solution: If object is marked , the corresponding entry is set true ,
            //else the corresponding entry is set false .
            if(KEY_TAB == rKEvt.GetKeyCode().GetCode())
            {
               FreshNavigatrTree();
            }
//-----IAccessibility2 Implementation 2009
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* Vom Lineal ausgehenden Drag (Hilflinien, Ursprung) beginnen
|*
\************************************************************************/

void DrawViewShell::StartRulerDrag (
    const Ruler& rRuler,
    const MouseEvent& rMEvt)
{
    GetActiveWindow()->CaptureMouse();

    const basegfx::B2DPoint aWPos(GetActiveWindow()->GetInverseViewTransformation() *
        basegfx::B2DPoint(GetActiveWindow()->GetPointerPosPixel().X(), GetActiveWindow()->GetPointerPosPixel().Y()));

    if ( rRuler.GetExtraRect().IsInside(rMEvt.GetPosPixel()) )
    {
        mpDrawView->BegSetPageOrg(aWPos);
        mbIsRulerDrag = true;
    }
    else
    {
        // #i34536# if no guide-lines are visible yet, that show them
        if( ! mpDrawView->IsHlplVisible())
            mpDrawView->SetHlplVisible( true );

        SdrHelpLineKind eKind;

        if ( rMEvt.IsMod1() )
            eKind = SDRHELPLINE_POINT;
        else if ( rRuler.IsHorizontal() )
            eKind = SDRHELPLINE_HORIZONTAL;
        else
            eKind = SDRHELPLINE_VERTICAL;

        mpDrawView->BegDragHelpLine(aWPos, eKind);
        mbIsRulerDrag = true;
    }
}
//IAccessibility2 Implementation 2009-----
//Solution: If object is marked , the corresponding entry is set true ,
//else the corresponding entry is set false .
void DrawViewShell::FreshNavigatrEntry()
{
        sal_uInt16 nId = SID_NAVIGATOR;
        SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
        if( pWindow )
        {
            SdNavigatorWin* pNavWin = (SdNavigatorWin*)( pWindow->GetContextWindow( SD_MOD() ) );
            if( pNavWin )
                pNavWin->FreshEntry();
        }
}

void DrawViewShell::FreshNavigatrTree()
{
        sal_uInt16 nId = SID_NAVIGATOR;
        SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
        if( pWindow )
        {
            SdNavigatorWin* pNavWin = (SdNavigatorWin*)( pWindow->GetContextWindow( SD_MOD() ) );
            if( pNavWin )
                pNavWin->FreshTree( GetDoc() );
        }
}
//-----IAccessibility2 Implementation 2009
/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void DrawViewShell::MouseButtonDown(const MouseEvent& rMEvt,
    ::sd::Window* pWin)
{
    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    // See #126086# and #128122#
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    if ( !IsInputLocked() )
    {
        ViewShell::MouseButtonDown(rMEvt, pWin);

//IAccessibility2 Implementation 2009-----
        //Solution: If object is marked , the corresponding entry is set true ,
        //else the corresponding entry is set false .
              FreshNavigatrTree();
//-----IAccessibility2 Implementation 2009
        if ( mbPipette )
            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->PipetteClicked();
    }
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/


void DrawViewShell::MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        if ( mpDrawView->IsAction() )
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            {
                bool bInsideOtherWindow = false;

                if (mpContentWindow.get() != NULL)
                {
                    aOutputArea = Rectangle(Point(0,0),
                        mpContentWindow->GetOutputSizePixel());

                    Point aPos = mpContentWindow->GetPointerPosPixel();
                    if ( aOutputArea.IsInside(aPos) )
                        bInsideOtherWindow = true;
                }

                if (! GetActiveWindow()->HasFocus ())
                {
                    GetActiveWindow()->ReleaseMouse ();
                    mpDrawView->BrkAction ();
                    return;
                }
                else if ( bInsideOtherWindow )
                {
                    GetActiveWindow()->ReleaseMouse();
                    pWin->CaptureMouse ();
                }
            }
            else if ( pWin != GetActiveWindow() )
                 pWin->CaptureMouse();
        }

        // #109585#
        // Since the next MouseMove may execute a IsSolidDraggingNow() in
        // SdrCreateView::MovCreateObj and there the ApplicationBackgroundColor
        // is needed it is necessary to set it here.
        if(mpDrawView!=NULL && GetDoc()!=NULL)
        {
            svtools::ColorConfig aColorConfig;
            Color aFillColor;

            if(DOCUMENT_TYPE_IMPRESS == GetDoc()->GetDocumentType())
            {
                aFillColor = Color( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor );
            }
            else
            {
                aFillColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
            }

            mpDrawView->SetApplicationBackgroundColor(aFillColor);
        }

        ViewShell::MouseMove(rMEvt, pWin);

        if( !mbMousePosFreezed )
        {
            maMousePos = basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        }

        if ( mbIsRulerDrag )
        {
            const basegfx::B2DPoint aLogicPos(GetActiveWindow()->GetInverseViewTransformation() * maMousePos);
            mpDrawView->MovAction(aLogicPos);
        }

        basegfx::B2DRange aRange;

        if ( mpDrawView->IsAction() )
        {
            aRange = GetActiveWindow()->GetViewTransformation() * mpDrawView->TakeActionRange();
        }
        else
        {
            aRange = basegfx::B2DRange(maMousePos);
        }

        ShowMousePosInfo(aRange, pWin);

        if ( mbPipette && GetViewFrame()->HasChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() ) )
        {
            const long      nStartX = basegfx::fround(maMousePos.getX()) - PIPETTE_RANGE;
            const long      nEndX = basegfx::fround(maMousePos.getX()) + PIPETTE_RANGE;
            const long      nStartY = basegfx::fround(maMousePos.getY()) - PIPETTE_RANGE;
            const long      nEndY = basegfx::fround(maMousePos.getY()) + PIPETTE_RANGE;
            long            nRed = 0;
            long            nGreen = 0;
            long            nBlue = 0;
            const double    fDiv = ( ( PIPETTE_RANGE << 1 ) + 1 ) * ( ( PIPETTE_RANGE << 1 ) + 1 );

            for ( long nY = nStartY; nY <= nEndY; nY++ )
            {
                for( long nX = nStartX; nX <= nEndX; nX++ )
                {
                    const Color aCol( pWin->GetPixel( pWin->PixelToLogic( Point( nX, nY ) ) ) );

                    nRed += aCol.GetRed();
                    nGreen += aCol.GetGreen();
                    nBlue += aCol.GetBlue();
                }
            }

            ( (SvxBmpMask*) GetViewFrame()->GetChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->
                SetColor( Color( (sal_uInt8) ( nRed / fDiv + .5 ),
                                 (sal_uInt8) ( nGreen / fDiv + .5 ),
                                 (sal_uInt8) ( nBlue / fDiv + .5 ) ) );
        }
    }
}


/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void DrawViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    if ( !IsInputLocked() )
    {
        bool bIsSetPageOrg = mpDrawView->IsSetPageOrg();

        if (mbIsRulerDrag)
        {
            Rectangle aOutputArea(Point(0,0), GetActiveWindow()->GetOutputSizePixel());

            if (aOutputArea.IsInside(rMEvt.GetPosPixel()))
            {
                mpDrawView->EndAction();

                if (bIsSetPageOrg)
                    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else if (rMEvt.IsLeft() && bIsSetPageOrg)
            {
                mpDrawView->BrkAction();

                if(mpDrawView->GetSdrPageView())
                {
                    SdPage& rPage = (SdPage&) mpDrawView->GetSdrPageView()->getSdrPageFromSdrPageView();
                    const basegfx::B2DPoint aOrg(rPage.GetLeftPageBorder(), rPage.GetTopPageBorder());
                    mpDrawView->GetSdrPageView()->SetPageOrigin(aOrg);
                }

                GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);
            }
            else
            {
                mpDrawView->BrkAction();
            }

            GetActiveWindow()->ReleaseMouse();
            mbIsRulerDrag = false;
        }
        else
            ViewShell::MouseButtonUp(rMEvt, pWin);
//IAccessibility2 Implementation 2009-----
        //Solution: If object is marked , the corresponding entry is set true ,
        //else the corresponding entry is set false .
        FreshNavigatrTree();
//-----IAccessibility2 Implementation 2009
    }
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void DrawViewShell::Command(const CommandEvent& rCEvt, ::sd::Window* pWin)
{
    // The command event is send to the window after a possible context
    // menu from an inplace client is closed. Now we have the chance to
    // deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    // For more information, see #126086# and #128122#
    SfxInPlaceClient* pIPClient = GetViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        // Deactivate OLE object
        mpDrawView->UnmarkAll();
        SelectionHasChanged();
        return;
    }

    if ( !IsInputLocked() )
    {
        if( GetView() &&GetView()->getSmartTags().Command(rCEvt) )
            return;

        const bool bNativeShow (SlideShow::IsRunning(GetViewShellBase()));

        if( rCEvt.GetCommand() == COMMAND_PASTESELECTION && !bNativeShow )
        {
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSelection( GetActiveWindow() ) );

            if( aDataHelper.GetTransferable().is() )
            {
                basegfx::B2DPoint aPos(0.0, 0.0);
                sal_Int8    nDnDAction = DND_ACTION_COPY;

                if( GetActiveWindow() )
                {
                    aPos = GetActiveWindow()->GetInverseViewTransformation() * basegfx::B2DPoint(rCEvt.GetMousePosPixel().X(), rCEvt.GetMousePosPixel().Y());
                }

                if( !mpDrawView->InsertData( aDataHelper, aPos, nDnDAction, false ) )
                {
                    INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                    if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, 0);
                    }
                }
            }
        }
        else if( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && !bNativeShow &&
                 pWin != NULL && !mpDrawView->IsAction() && !SD_MOD()->GetWaterCan() )
        {
            sal_uInt16 nSdResId = 0;          // ResourceID fuer Popup-Menue
            bool bGraphicShell = dynamic_cast< GraphicViewShell* >(this);

            // Ist ein Fangobjekt unter dem Mauszeiger?
            basegfx::B2DPoint aMPos(pWin->GetInverseViewTransformation() * maMousePos);
            sal_uInt32 nHelpLine;
            // fuer Klebepunkt
            SdrObject*  pObj = NULL;
            sal_uInt32 nPickId = 0;
            // fuer Feldbefehl
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
            const SvxFieldItem* pFldItem = NULL;
            if( pOLV )
                pFldItem = pOLV->GetFieldAtSelection();
                //pFldItem = pOLV->GetFieldUnderMousePointer();

            // Hilfslinie
            if ( mpDrawView->PickHelpLine( aMPos, mpView->getHitTolLog(), nHelpLine) )
            {
                if(mpDrawView->GetSdrPageView())
                {
                    nSdResId = RID_DRAW_SNAPOBJECT_POPUP;
                    ShowSnapLineContextMenu(*mpDrawView->GetSdrPageView(), nHelpLine, rCEvt.GetMousePosPixel());
                    return;
                }
            }
            // Klebepunkt unter dem Mauszeiger markiert?
            else if( mpDrawView->PickGluePoint( aMPos, pObj, nPickId ) && mpDrawView->IsGluePointMarked( *pObj, nPickId ) )
            {
                nSdResId = RID_DRAW_GLUEPOINT_POPUP;
            }
            // Feldbefehl ?
            else if( pFldItem && (
                dynamic_cast< const SvxDateField* >(pFldItem->GetField()) ||
                dynamic_cast< const SvxExtTimeField* >(pFldItem->GetField()) ||
                dynamic_cast< const SvxExtFileField* >(pFldItem->GetField()) ||
                dynamic_cast< const SvxAuthorField* >(pFldItem->GetField()) ) )
            {
                LanguageType eLanguage( LANGUAGE_SYSTEM );

                // #101743# Format popup with outliner language, if possible
                if( pOLV->GetOutliner() )
                {
                    ESelection aSelection( pOLV->GetSelection() );
                    eLanguage = pOLV->GetOutliner()->GetLanguage( aSelection.nStartPara, aSelection.nStartPos );
                }

                SdFieldPopup aFieldPopup( pFldItem->GetField(), eLanguage );

                if ( rCEvt.IsMouseEvent() )
                {
                    aMPos = basegfx::B2DPoint(rCEvt.GetMousePosPixel().X(), rCEvt.GetMousePosPixel().Y());
                }
                else
                {
                    aMPos = basegfx::B2DPoint( 20.0, 20.0 );
                }

                aFieldPopup.Execute( pWin, Point(basegfx::fround(aMPos.getX()), basegfx::fround(aMPos.getY())) );

                SvxFieldData* pField = aFieldPopup.GetField();
                if( pField )
                {
                    SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );
                    //pOLV->DeleteSelected(); <-- fehlt leider !
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOLV->GetSelection();
                    bool bSel = true;
                    if( aSel.nStartPos == aSel.nEndPos )
                    {
                        bSel = false;
                        aSel.nEndPos++;
                    }
                    pOLV->SetSelection( aSel );

                    pOLV->InsertField( aFieldItem );

                    // Selektion wird wieder in den Ursprungszustand gebracht
                    if( !bSel )
                        aSel.nEndPos--;
                    pOLV->SetSelection( aSel );

                    delete pField;
                }
            }
            else
            {
                // ist etwas selektiert?
                pObj = mpDrawView->getSelectedIfSingle();

                if(pObj)
                {
                    if( HasCurrentFunction(SID_BEZIER_EDIT) && (dynamic_cast< SdrPathObj * >( pObj ) != 0 ) )
                    {
                        nSdResId = RID_BEZIER_POPUP;
                    }
                    else
                    {
                        if( mpDrawView->GetTextEditObject() )
                        {
                            OutlinerView* pOutlinerView = mpDrawView->GetTextEditOutlinerView();
                            Point aPos(rCEvt.GetMousePosPixel());

                            if ( pOutlinerView )
                            {
                                if( (  rCEvt.IsMouseEvent() && pOutlinerView->IsWrongSpelledWordAtPos(aPos) ) ||
                                    ( !rCEvt.IsMouseEvent() && pOutlinerView->IsCursorAtWrongSpelledWord() ) )
                                {
                                    // #91457# Popup for Online-Spelling now handled by DrawDocShell
                                    // Link aLink = LINK(GetDoc(), SdDrawDocument, OnlineSpellCallback);
                                    Link aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

                                    if( !rCEvt.IsMouseEvent() )
                                    {
                                        aPos = GetActiveWindow()->LogicToPixel( pOutlinerView->GetEditView().GetCursor()->GetPos() );
                                    }
                                    // While showing the spell context menu
                                    // we lock the input so that another
                                    // context menu can not be opened during
                                    // that time (crash #i43235#).  In order
                                    // to not lock the UI completely we
                                    // first release the mouse.
                                    GetActiveWindow()->ReleaseMouse();
                                    LockInput();
                                    pOutlinerView->ExecuteSpellPopup(aPos, &aLink);
                                    UnlockInput();
                                }
                                else
                                {
                                    if( (pObj->GetObjInventor() == SdrInventor) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
                                    {
                                        nSdResId = RID_DRAW_TABLEOBJ_INSIDE_POPUP;
                                    }
                                    else
                                    {
                                        nSdResId = RID_DRAW_TEXTOBJ_INSIDE_POPUP;
                                    }
                                }
                            }
                        }
                        else
                        {
                            sal_uInt32 nInv = pObj->GetObjInventor();
                            sal_uInt16 nId = pObj->GetObjIdentifier();

                            if (nInv == SdrInventor)
                            {
                                switch ( nId )
                                {
                                    case OBJ_CAPTION:
                                    case OBJ_TITLETEXT:
                                    case OBJ_OUTLINETEXT:
                                    case OBJ_TEXT:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_TEXTOBJ_POPUP :
                                                                    RID_DRAW_TEXTOBJ_POPUP;
                                        break;

                                    case OBJ_POLY:
                                    {
                                        SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                                        if(pSdrPathObj)
                                        {
                                            switch(pSdrPathObj->getSdrPathObjType())
                                            {
                                                case PathType_Line:
                                                {
                                                    nSdResId = bGraphicShell ? RID_GRAPHIC_LINEOBJ_POPUP :
                                                                                RID_DRAW_LINEOBJ_POPUP;
                                                    break;
                                                }
                                                case PathType_OpenPolygon:
                                                case PathType_OpenBezier:
                                                {
                                                    nSdResId = bGraphicShell ? RID_GRAPHIC_POLYLINEOBJ_POPUP :
                                                                                RID_DRAW_POLYLINEOBJ_POPUP;
                                                    break;
                                                }
                                                case PathType_ClosedPolygon:
                                                case PathType_ClosedBezier:
                                                {
                                                    nSdResId = bGraphicShell ? RID_GRAPHIC_GEOMOBJ_POPUP :
                                                                                RID_DRAW_GEOMOBJ_POPUP;
                                                    break;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            OSL_ENSURE(false, "OOps, SdrObjKind and dynamic_cast do not fit (!)");
                                        }
                                        break;
                                    }

                                    case OBJ_EDGE:      // Connector
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_EDGEOBJ_POPUP :
                                                                    RID_DRAW_EDGEOBJ_POPUP;
                                        break;

                                    case OBJ_MEASURE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_MEASUREOBJ_POPUP :
                                                                    RID_DRAW_MEASUREOBJ_POPUP;
                                        break;

                                    case OBJ_RECT:
                                    case OBJ_CIRC:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GEOMOBJ_POPUP :
                                                                    RID_DRAW_GEOMOBJ_POPUP;
                                        break;

                                    case OBJ_CUSTOMSHAPE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_CUSTOMSHAPE_POPUP :
                                                                    RID_DRAW_CUSTOMSHAPE_POPUP;
                                        break;

                                    case OBJ_GRUP:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GROUPOBJ_POPUP :
                                                                    RID_DRAW_GROUPOBJ_POPUP;
                                        break;

                                    case OBJ_GRAF:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_GRAPHIC_POPUP :
                                                                    RID_DRAW_GRAPHIC_POPUP;
                                        break;

                                    case OBJ_OLE2:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_OLE2_POPUP :
                                                                    RID_DRAW_OLE2_POPUP;
                                        break;
                                    case OBJ_MEDIA:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_MEDIA_POPUP :
                                                                    RID_DRAW_MEDIA_POPUP;
                                        break;
                                    case OBJ_TABLE:
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_TABLE_POPUP :
                                                                    RID_DRAW_TABLE_POPUP;
                                        break;
                                }
                            }
                            else if( nInv == E3dInventor )
                            {
                                if( nId == E3D_SCENE_ID )
                                {
                                    if( !mpDrawView->IsGroupEntered() )
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_3DSCENE_POPUP :
                                                                RID_DRAW_3DSCENE_POPUP;
                                    else
                                        nSdResId = bGraphicShell ? RID_GRAPHIC_3DSCENE2_POPUP :
                                                                RID_DRAW_3DSCENE2_POPUP;
                                }
                                else
                                    nSdResId = bGraphicShell ? RID_GRAPHIC_3DOBJ_POPUP :
                                                                RID_DRAW_3DOBJ_POPUP;
                            }
                            else if( nInv == FmFormInventor )
                            {
                                nSdResId = RID_FORM_CONTROL_POPUP;
                            }
                        }
                    }
                }
                else if (mpDrawView->getSelectedSdrObjectCount() > 1 )
                {
                    // Mehrfachselektion
                    nSdResId = bGraphicShell ? RID_GRAPHIC_MULTISELECTION_POPUP :
                                                RID_DRAW_MULTISELECTION_POPUP;
                }
                else
                {
                    // nichts selektiert
                    nSdResId = bGraphicShell ? RID_GRAPHIC_NOSEL_POPUP :
                                                RID_DRAW_NOSEL_POPUP;
                }
            }
            // Popup-Menue anzeigen
            if (nSdResId)
            {
                GetActiveWindow()->ReleaseMouse();

                if(rCEvt.IsMouseEvent())
                {
                    GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId));
                }
                else
                {
                    //#106326# don't open contextmenu at mouse position if not opened via mouse

                    //middle of the window if nothing is marked
                    basegfx::B2DPoint aMenuPos(
                        GetActiveWindow()->GetSizePixel().Width()/2,
                        GetActiveWindow()->GetSizePixel().Height()/2);

                    //middle of the bounding rect if something is marked
                    if( mpDrawView->areSdrObjectsSelected() )
                    {
                        const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
                        const basegfx::B2DRange aMarkRange(sdr::legacy::GetAllObjBoundRange(aSelection));

                        aMenuPos = GetActiveWindow()->GetViewTransformation() * aMarkRange.getCenter();

                        //move the point into the visible window area
                        if( aMenuPos.getX() < 0 )
                        {
                            aMenuPos.setX(0.0);
                        }

                        if( aMenuPos.getY() < 0 )
                        {
                            aMenuPos.setY(0.0);
                        }

                        if( aMenuPos.getX() > GetActiveWindow()->GetSizePixel().Width() )
                        {
                            aMenuPos.setX(GetActiveWindow()->GetSizePixel().Width());
                        }

                        if( aMenuPos.getY() > GetActiveWindow()->GetSizePixel().Height() )
                        {
                            aMenuPos.setY(GetActiveWindow()->GetSizePixel().Height());
                        }
                    }

                    //open context menu at that point
                    const Point aOldMenuPos(basegfx::fround(aMenuPos.getX()), basegfx::fround(aMenuPos.getY()));
                    GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(nSdResId),GetActiveWindow(),&aOldMenuPos);
                }

                mbMousePosFreezed = false;
            }
        }
        else
        {
            ViewShell::Command(rCEvt, pWin);
        }
    }
}

/*************************************************************************
|*
|* Linealmarkierungen anzeigen
|*
\************************************************************************/

void DrawViewShell::ShowMousePosInfo(const basegfx::B2DRange& rRange,
    ::sd::Window* pWin)
{
    if (mbHasRulers && pWin )
    {
        RulerLine   pHLines[2];
        RulerLine   pVLines[2];
        long        nHOffs = 0L;
        long        nVOffs = 0L;
        sal_uInt16      nCnt;

        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->SetLines();

        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->SetLines();

        if (mpHorizontalRuler.get() != NULL)
        {
            nHOffs = mpHorizontalRuler->GetNullOffset() +
                     mpHorizontalRuler->GetPageOffset();
        }

        if (mpVerticalRuler.get() != NULL)
        {
            nVOffs = mpVerticalRuler->GetNullOffset() +
                     mpVerticalRuler->GetPageOffset();
        }

        nCnt = 1;
        pHLines[0].nPos = basegfx::fround(rRange.getMinX()) - nHOffs;
        pVLines[0].nPos = basegfx::fround(rRange.getMinY()) - nVOffs;
        pHLines[0].nStyle = 0;
        pVLines[0].nStyle = 0;

        if(!basegfx::fTools::equal(rRange.getMaxX(), rRange.getMinX()) || !basegfx::fTools::equal(rRange.getMaxY(), rRange.getMinY()))
        {
            pHLines[1].nPos = basegfx::fround(rRange.getMaxX()) - nHOffs;
            pVLines[1].nPos = basegfx::fround(rRange.getMaxY()) - nVOffs;
            pHLines[1].nStyle = 0;
            pVLines[1].nStyle = 0;
            nCnt++;
        }

        if (mpHorizontalRuler.get() != NULL)
            mpHorizontalRuler->SetLines(nCnt, pHLines);
        if (mpVerticalRuler.get() != NULL)
            mpVerticalRuler->SetLines(nCnt, pVLines);
    }

    // StatusBar Koordinatenanzeige
    OSL_ASSERT (GetViewShell()!=NULL);
    if ( !GetViewShell()->GetUIActiveClient() )
    {
        SfxItemSet aSet(GetPool(), SID_CONTEXT, SID_CONTEXT,
                                   SID_ATTR_POSITION, SID_ATTR_POSITION,
                                   SID_ATTR_SIZE, SID_ATTR_SIZE,
                                   0L);

//        GetStatusBarState(aSet);  nicht performant bei gedrueckter Modifiertaste!!

        aSet.Put( SfxStringItem( SID_CONTEXT, mpDrawView->GetStatusText() ) );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.SetState(aSet);
        rBindings.Invalidate(SID_CONTEXT);
        rBindings.Invalidate(SID_ATTR_POSITION);
        rBindings.Invalidate(SID_ATTR_SIZE);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::LockInput()
{
    mnLockCount++;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::UnlockInput()
{
    DBG_ASSERT( mnLockCount, "Input for this shell is not locked!" );
    if ( mnLockCount )
        mnLockCount--;
}




void DrawViewShell::ShowSnapLineContextMenu (
    SdrPageView& rPageView,
    const sal_uInt32 nSnapLineIndex,
    const Point& rMouseLocation)
{
    const SdrHelpLine& rHelpLine (rPageView.GetHelpLines()[nSnapLineIndex]);
    ::boost::scoped_ptr<PopupMenu> pMenu (new PopupMenu ());

    if (rHelpLine.GetKind() == SDRHELPLINE_POINT)
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            String(SdResId(STR_POPUP_EDIT_SNAPPOINT)));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            String(SdResId(STR_POPUP_DELETE_SNAPPOINT)));
    }
    else
    {
        pMenu->InsertItem(
            SID_SET_SNAPITEM,
            String(SdResId(STR_POPUP_EDIT_SNAPLINE)));
        pMenu->InsertSeparator();
        pMenu->InsertItem(
            SID_DELETE_SNAPITEM,
            String(SdResId(STR_POPUP_DELETE_SNAPLINE)));
    }

    pMenu->RemoveDisabledEntries(false, false);

    const sal_uInt16 nResult = pMenu->Execute(
        GetActiveWindow(),
        Rectangle(rMouseLocation, Size(10,10)),
        POPUPMENU_EXECUTE_DOWN);
    switch (nResult)
    {
        case SID_SET_SNAPITEM:
        {
            SfxUInt32Item aHelpLineItem (ID_VAL_INDEX, nSnapLineIndex);
            const SfxPoolItem* aArguments[] = {&aHelpLineItem, NULL};
            GetViewFrame()->GetDispatcher()->Execute(
                SID_SET_SNAPITEM,
                SFX_CALLMODE_SLOT,
                aArguments);
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            rPageView.DeleteHelpLine(nSnapLineIndex);
        }
        break;

        default:
            break;
    }
}




#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

} // end of namespace sd
