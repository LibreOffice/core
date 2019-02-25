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

#include <svx/svddef.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdotext.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>

#include <futext.hxx>
#include <drwlayer.hxx>
#include <sc.hrc>
#include <tabvwsh.hxx>
#include <drawview.hxx>

// maximum of mouse movement which allows to start Drag&Drop
//! fusel,fuconstr,futext - combined!
#define SC_MAXDRAGMOVE  3

static void lcl_InvalidateAttribs( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
    rBindings.Invalidate( SID_ATTR_CHAR_BACK_COLOR );
    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_HYPERLINK_GETLINK );
    rBindings.Invalidate( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
    rBindings.Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    rBindings.Invalidate( SID_TABLE_VERT_NONE );
    rBindings.Invalidate( SID_TABLE_VERT_CENTER );
    rBindings.Invalidate( SID_TABLE_VERT_BOTTOM );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );
}

static void lcl_UpdateHyphenator( Outliner& rOutliner, const SdrObject* pObj )
{
    // use hyphenator only if hyphenation attribute is set
    if ( pObj && pObj->GetMergedItem(EE_PARA_HYPHENATE).GetValue() ) {
            css::uno::Reference<css::linguistic2::XHyphenator> xHyphenator( LinguMgr::GetHyphenator() );
            rOutliner.SetHyphenator( xHyphenator );
    }
}

FuText::FuText(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, const SfxRequest& rReq)
    : FuConstruct(rViewSh, pWin, pViewP, pDoc, rReq)
{
}

FuText::~FuText()
{
//  StopEditMode();                 // in Deactivate !
}

bool FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());
    bool bStraightEnter = true;

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return true;                 // event handled from SdrView

    if ( pView->IsTextEdit() )
    {
        if ( IsEditingANote() )
        {
            if( !IsSizingOrMovingNote(rMEvt) )
            {
                StopEditMode();            // Clicked outside, ending edit
                bStraightEnter = false;
            }
        }
        else
        {
            StopEditMode();            // Clicked outside, ending edit
            pView->UnmarkAll();
            bStraightEnter = false;
        }
        pView->SetCreateMode();
    }

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->PickHandle(aMDPos);
        const size_t nHdlNum = pView->GetHdlNum(pHdl);
        if (pHdl != nullptr)
        {
            if (pView->HasMarkablePoints() && pView->IsPointMarkable(*pHdl))
            {
                bool bPointMarked=pView->IsPointMarked(*pHdl);

                if ( rMEvt.IsShift() )
                {
                    if (!bPointMarked)
                    {
                        pView->MarkPoint(*pHdl);
                    }
                    else
                    {
                        pView->UnmarkPoint(*pHdl);
                    }
                }
                else
                {
                    if (!bPointMarked)
                    {
                        pView->UnmarkAllPoints();
                        pView->MarkPoint(*pHdl);
                    }
                }
                pHdl=pView->GetHdl(nHdlNum);
            }
        }

        SdrPageView* pPV = nullptr;

        if ( pHdl != nullptr || pView->IsMarkedHit(aMDPos) )
        {
            SdrObject* pObj = (pHdl == nullptr) ?
                pView->PickObj(aMDPos, pView->getHitTolLog(), pPV, SdrSearchOptions::PICKTEXTEDIT) :
                nullptr;
            if (pObj)
            {
                std::unique_ptr<SdrOutliner> pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content
                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!?? the default values are not correct when result is without outliner ???!?
                auto pOTemp = pO.get();
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, true, pO.release()) )
                {
                    // subscribe EditEngine-UndoManager
                    rViewShell.SetDrawTextUndo( &pOTemp->GetUndoManager() );

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                    if ( pOLV->MouseButtonDown(rMEvt) )
                        return true; // Event to the Outliner
                }
            }
            else
            {
                // disable tail & circular move for caption objects.
                bool bDrag = false;
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if( rMarkList.GetMarkCount() == 1 )
                {
                    SdrObject* pMarkedObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                    if( ScDrawLayer::IsNoteCaption( pMarkedObj ) )
                    {
                        if(pHdl->GetKind() != SdrHdlKind::Poly && pHdl->GetKind() != SdrHdlKind::Circle)
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
                    pView->BegDragObj(aMDPos, nullptr, pHdl);
                }
            }
        }
        else
        {
            if (pView->IsEditMode())
            {
                bool bPointMode=pView->HasMarkablePoints();

                if (!rMEvt.IsShift())
                {
                    if (bPointMode)
                    {
                        pView->UnmarkAllPoints();
                    }
                    else
                    {
                        pView->UnmarkAll();
                    }

                    pView->SetDragMode(SdrDragMode::Move);
                    SfxBindings& rBindings = rViewShell.GetViewFrame()->GetBindings();
                    rBindings.Invalidate( SID_OBJECT_ROTATE );
                    rBindings.Invalidate( SID_OBJECT_MIRROR );
                }

                if ( pView->MarkObj(aMDPos, -2, false, rMEvt.IsMod1()) )
                {
                    aDragTimer.Start();

                    pHdl=pView->PickHandle(aMDPos);

                    if (pHdl!=nullptr)
                    {
                        pView->MarkPoint(*pHdl);
                        pHdl=pView->GetHdl(nHdlNum);
                    }

                    pView->BegDragObj(aMDPos, nullptr, pHdl);
                }
                else
                {
                    if (bPointMode)
                    {
                        pView->BegMarkPoints(aMDPos);
                    }
                    else
                    {
                        pView->BegMarkObj(aMDPos);
                    }
                }
            }
            else if (aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
            {
                //  Edit notes -> create no new text objects
                //  and leave text mode
                rViewShell.GetViewData().GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
            }
            else
            {
                if (bStraightEnter)//Hack for that silly idea that creating text fields is inside the text routine
                {
                    // create object
                    pView->BegCreateObj(aMDPos);
                }
                else if (SdrObject* pObj = pView->PickObj(aMDPos, pView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::BEFOREMARK))
                {
                    pView->UnmarkAllObj();
                    ScViewData& rViewData = rViewShell.GetViewData();
                    rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
                    pView->MarkObj(pObj,pPV);

                    pHdl=pView->PickHandle(aMDPos);
                    pView->BegDragObj(aMDPos, nullptr, pHdl);
                    return true;
                }
            }
        }
    }

    if (!bIsInDragMode)
    {
        pWindow->CaptureMouse();
//      ForcePointer(&rMEvt);
        lcl_InvalidateAttribs( rViewShell.GetViewFrame()->GetBindings() );
    }

    rViewShell.SetActivePointer(pView->GetPreferredPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));
    if (!bStraightEnter)
    {
            pView->UnmarkAll();
            ScViewData& rViewData = rViewShell.GetViewData();
            rViewData.GetDispatcher().Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
    }

    return true;
}

bool FuText::MouseMove(const MouseEvent& rMEvt)
{
    rViewShell.SetActivePointer(pView->GetPreferredPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( std::abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             std::abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt(pWindow->PixelToLogic(aPix));

    if ( pView->MouseMove(rMEvt, pWindow) )
        return true; // event handled from SdrView

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

    return false;
}

bool FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( rViewShell.GetViewFrame()->GetBindings() );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return true; // Event evaluated by SdrView

    if ( pView->IsDragObj() )
    {
        pView->EndDragObj( rMEvt.IsShift() );
        pView->ForceMarkedToAnotherPage();
    }
    else if ( pView->IsCreateObj() )
    {
        if (rMEvt.IsLeft())
        {
            pView->EndCreateObj(SdrCreateCmd::ForceEnd);
            if (aSfxRequest.GetSlot() == SID_DRAW_TEXT_MARQUEE)
            {
                // create marquee-object?
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if (rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                    // set needed attributes for scrolling
                    SfxItemSet aItemSet( pDrDoc->GetItemPool(),
                                            svl::Items<SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST>{});

                    aItemSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                    aItemSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                    aItemSet.Put( SdrTextAniKindItem( SdrTextAniKind::Slide ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SdrTextAniDirection::Left ) );
                    aItemSet.Put( SdrTextAniCountItem( 1 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                                    static_cast<sal_Int16>(pWindow->PixelToLogic(Size(2,1)).Width())) );
                    pObj->SetMergedItemSetAndBroadcast(aItemSet);
                }
            }

            // init object different when vertical writing
            sal_uInt16 nSlotID(aSfxRequest.GetSlot());
            bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotID);
            if(bVertical)
            {
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                if(rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    if(auto pText = dynamic_cast<SdrTextObj*>( pObj))
                    {
                        SfxItemSet aSet(pDrDoc->GetItemPool());

                        pText->SetVerticalWriting(true);

                        aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                        aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                        pText->SetMergedItemSet(aSet);
                    }
                }
            }

            SetInEditMode();

                // leave mode when sole click (-> fuconstr)
            if ( !pView->AreObjectsMarked() )
            {
                pView->MarkObj(aPnt, -2, false, rMEvt.IsMod1());

                SfxDispatcher& rDisp = rViewShell.GetViewData().GetDispatcher();
                if ( pView->AreObjectsMarked() )
                    rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
                else
                    rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
            }
        }
    }
    else if ( pView->IsAction() )
    {
        pView->EndAction();
    }
    else if( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->AreObjectsMarked() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, -2, false, rMEvt.IsMod1());

            SfxDispatcher& rDisp = rViewShell.GetViewData().GetDispatcher();
            if ( pView->AreObjectsMarked() )
                rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
        }
    }

    return false;
}

// switch mouse-pointer
void FuText::ForcePointer(const MouseEvent* /* pMEvt */)
{
    rViewShell.SetActivePointer( aNewPointer );
}

// modify keyboard events
// if a KeyEvent is being processed, then the return value is sal_True, else FALSE.
bool FuText::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    if ( pView->KeyInput(rKEvt, pWindow) )
    {
        bReturn = true;
        lcl_InvalidateAttribs( rViewShell.GetViewFrame()->GetBindings() );
    }
    else
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return bReturn;
}

void FuText::Activate()
{
    pView->SetDragMode(SdrDragMode::Move);
    SfxBindings& rBindings = rViewShell.GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OBJECT_ROTATE );
    rBindings.Invalidate( SID_OBJECT_MIRROR );

// instant set the edit mode
//  SetInEditMode();

//  if (!pTextObj)
    {
        // no text object in EditMode, therefore set CreateMode

        pView->SetCurrentObj(OBJ_TEXT);

        pView->SetCreateMode();
    }

    aNewPointer = PointerStyle::Text;

    aOldPointer = pWindow->GetPointer();
    rViewShell.SetActivePointer( aNewPointer );

    FuConstruct::Activate();
}

void FuText::Deactivate()
{
    FuConstruct::Deactivate();
    rViewShell.SetActivePointer( aOldPointer );
    StopEditMode();
}

// switch object to Edit-Mode
void FuText::SetInEditMode(SdrObject* pObj, const Point* pMousePixel,
                            bool bCursorToEnd, const KeyEvent* pInitialKey)
{
    /*  It is possible to pass a special (unselected) object in pObj, e.g. the
        caption object of a cell note. If pObj is 0, then the selected object
        is used. The layer will be relocked in FuText::StopEditMode(). */
    if ( pObj && (pObj->GetLayer() == SC_LAYER_INTERN) )
        pView->UnlockInternalLayer();

    if ( !pObj && pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetMarkedSdrObj();
        }
    }

    if ( pObj )
    {
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
        {
            SdrPageView* pPV = pView->GetSdrPageView();

            if ( pObj->HasTextEdit() )
            {
                std::unique_ptr<SdrOutliner> pO = MakeOutliner();
                lcl_UpdateHyphenator( *pO, pObj );

                //  vertical flag:
                //  deduced from slot ids only if text object has no content

                sal_uInt16 nSlotID = aSfxRequest.GetSlot();
                bool bVertical = ( nSlotID == SID_DRAW_TEXT_VERTICAL );
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if ( pOPO )
                    bVertical = pOPO->IsVertical();     // content wins
                pO->SetVertical( bVertical );

                //!??  without returned Outliner the defaults are not correct ???!?
                auto pOTemp = pO.get();
                if ( pView->SdrBeginTextEdit(pObj, pPV, pWindow, true, pO.release()) )
                {
                    //  Toggle out of paste mode if we are in it, otherwise
                    //  pressing return in this object will instead go to the
                    //  sheet and be considered an overwrite-cell instruction
                    rViewShell.GetViewData().SetPasteMode(ScPasteFlags::NONE);
                    rViewShell.UpdateCopySourceOverlay();

                    //  EditEngine-UndoManager anmelden
                    rViewShell.SetDrawTextUndo( &pOTemp->GetUndoManager() );

                    pView->SetEditMode();

                    //  set text cursor to click position or to end,
                    //  pass initial key event to outliner view
                    if ( pMousePixel || bCursorToEnd || pInitialKey )
                    {
                        OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                        if (pOLV)
                        {
                            if ( pMousePixel )
                            {
                                MouseEvent aEditEvt( *pMousePixel, 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0 );
                                pOLV->MouseButtonDown(aEditEvt);
                                pOLV->MouseButtonUp(aEditEvt);
                            }
                            else if ( bCursorToEnd )
                            {
                                ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                                pOLV->SetSelection(aNewSelection);
                            }

                            if ( pInitialKey )
                                pOLV->PostKeyEvent( *pInitialKey );
                        }
                    }
                }
            }
        }
    }
}

// Create default drawing objects via keyboard
SdrObjectUniquePtr FuText::CreateDefaultObject(const sal_uInt16 nID, const tools::Rectangle& rRectangle)
{
    // case SID_DRAW_TEXT:
    // case SID_DRAW_TEXT_VERTICAL:
    // case SID_DRAW_TEXT_MARQUEE:
    // case SID_DRAW_NOTEEDIT:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        *pDrDoc,
        pView->GetCurrentObjInventor(),
        pView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        if(auto pText = dynamic_cast<SdrTextObj*>( pObj.get() ))
        {
            pText->SetLogicRect(rRectangle);

            // don't set default text, start edit mode instead
            // String aText(ScResId(STR_CAPTION_DEFAULT_TEXT));
            // pText->SetText(aText);

            bool bVertical = (SID_DRAW_TEXT_VERTICAL == nID);
            bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nID);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool());

                aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));

                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrDoc->GetItemPool(), svl::Items<SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST>{});

                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                aSet.Put( SdrTextAniKindItem( SdrTextAniKind::Slide ) );
                aSet.Put( SdrTextAniDirectionItem( SdrTextAniDirection::Left ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( static_cast<sal_Int16>(pWindow->PixelToLogic(Size(2,1)).Width())) );

                pObj->SetMergedItemSetAndBroadcast(aSet);
            }

            SetInEditMode( pObj.get() );      // start edit mode
        }
        else
        {
            OSL_FAIL("Object is NO text object");
        }
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
