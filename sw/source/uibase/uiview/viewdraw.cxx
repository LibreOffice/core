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

#include <hintids.hxx>
#include <svl/aeitem.hxx>
#include <svl/itempool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include <svx/fmview.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <sfx2/viewfrm.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <textboxhelper.hxx>
#include <editeng/langitem.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/unolingu.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/fontworkgallery.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editstat.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <svx/fmglob.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdoutl.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <cmdid.h>
#include <drawsh.hxx>
#include <drwbassh.hxx>
#include <beziersh.hxx>
#include <conrect.hxx>
#include <conpoly.hxx>
#include <conarc.hxx>
#include <conform.hxx>
#include <concustomshape.hxx>
#include <dselect.hxx>
#include <edtwin.hxx>

#include <dcontact.hxx>

#include <svx/svdpagv.hxx>
#include <svx/extrusionbar.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace ::com::sun::star;

// Execute Drawing-Ids

void SwView::ExecDraw(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    const SfxStringItem* pStringItem = nullptr;
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    bool bDeselect = false;

    sal_uInt16 nSlotId = rReq.GetSlot();
    if(pArgs && SfxItemState::SET == pArgs->GetItemState(GetPool().GetWhich(nSlotId), false, &pItem))
        pStringItem = dynamic_cast< const SfxStringItem*>(pItem);

    if (nSlotId == SID_OBJECT_SELECT && m_nFormSfxId == nSlotId)
    {
        bDeselect = true;
    }
    else if (nSlotId == SID_FM_CREATE_CONTROL)
    {
        const SfxUInt16Item* pIdentifierItem = rReq.GetArg<SfxUInt16Item>(SID_FM_CONTROL_IDENTIFIER);
        if( pIdentifierItem )
        {
            sal_uInt16 nNewId = pIdentifierItem->GetValue();
            if (nNewId == m_nFormSfxId)
            {
                bDeselect = true;
                GetViewFrame()->GetDispatcher()->Execute(SID_FM_LEAVE_CREATE);  // Button should popping out
            }
        }
    }
    else if (nSlotId == SID_FM_CREATE_FIELDCONTROL)
    {
        FmFormView* pFormView = dynamic_cast<FmFormView*>(pSdrView);
        if (pFormView)
        {
            const SfxUnoAnyItem* pDescriptorItem = rReq.GetArg<SfxUnoAnyItem>(SID_FM_DATACCESS_DESCRIPTOR);
            OSL_ENSURE( pDescriptorItem, "SwView::ExecDraw(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );
            if( pDescriptorItem )
            {
                svx::ODataAccessDescriptor aDescriptor( pDescriptorItem->GetValue() );
                SdrObject* pObj = pFormView->CreateFieldControl( aDescriptor );

                if ( pObj )
                {
                    Size aDocSize(m_pWrtShell->GetDocSize());
                    const SwRect& rVisArea = m_pWrtShell->VisArea();
                    Point aStartPos = rVisArea.Center();
                    if(rVisArea.Width() > aDocSize.Width())
                        aStartPos.setX( aDocSize.Width() / 2 + rVisArea.Left() );
                    if(rVisArea.Height() > aDocSize.Height())
                        aStartPos.setY( aDocSize.Height() / 2 + rVisArea.Top() );

                    //determine the size of the object
                    if(pObj->IsGroupObject())
                    {
                        const tools::Rectangle& rBoundRect = static_cast<SdrObjGroup*>(pObj)->GetCurrentBoundRect();
                        aStartPos.AdjustX( -(rBoundRect.GetWidth()/2) );
                        aStartPos.AdjustY( -(rBoundRect.GetHeight()/2) );
                    }

                    // TODO: unmark all other
                    m_pWrtShell->EnterStdMode();
                    m_pWrtShell->SwFEShell::InsertDrawObj( *pObj, aStartPos );
                }
            }
        }
    }
    else if ( nSlotId == SID_FONTWORK_GALLERY_FLOATER )
    {
        vcl::Window*  pWin = &( m_pWrtShell->GetView().GetViewFrame()->GetWindow() );

        if ( pWin )
            pWin->EnterWait();

        if( !m_pWrtShell->HasDrawView() )
            m_pWrtShell->MakeDrawView();

        pSdrView = m_pWrtShell->GetDrawView();
        if ( pSdrView )
        {
            SdrObject* pObj = nullptr;
            ScopedVclPtrInstance< svx::FontWorkGalleryDialog > aDlg( pSdrView, pWin );
            aDlg->SetSdrObjectRef( &pObj, pSdrView->GetModel() );
            aDlg->Execute();
            if ( pObj )
            {
                Size            aDocSize( m_pWrtShell->GetDocSize() );
                const SwRect&   rVisArea = m_pWrtShell->VisArea();
                Point           aPos( rVisArea.Center() );
                Size            aSize;
                Size            aPrefSize( pObj->GetSnapRect().GetSize() );

                if( rVisArea.Width() > aDocSize.Width())
                    aPos.setX( aDocSize.Width() / 2 + rVisArea.Left() );

                if(rVisArea.Height() > aDocSize.Height())
                    aPos.setY( aDocSize.Height() / 2 + rVisArea.Top() );

                if( aPrefSize.Width() && aPrefSize.Height() )
                {
                    if( pWin )
                        aSize = pWin->PixelToLogic(aPrefSize, MapMode(MapUnit::MapTwip));
                    else
                        aSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::MapTwip));
                }
                else
                    aSize = Size( 2835, 2835 );

                m_pWrtShell->EnterStdMode();
                m_pWrtShell->SwFEShell::InsertDrawObj( *pObj, aPos );
                rReq.Ignore ();
            }
        }
        if( pWin )
            pWin->LeaveWait();
    }
    else if ( m_nFormSfxId != USHRT_MAX )
        GetViewFrame()->GetDispatcher()->Execute( SID_FM_LEAVE_CREATE );

    if( nSlotId == SID_DRAW_CS_ID )
    {
        //deselect if same custom shape is selected again
        SwDrawBase* pFuncPtr = GetDrawFuncPtr();
        if( pFuncPtr && pFuncPtr->GetSlotId() == SID_DRAW_CS_ID )
        {
            ConstCustomShape* pConstCustomShape = static_cast<ConstCustomShape*>(pFuncPtr);
            OUString aNew = ConstCustomShape::GetShapeTypeFromRequest( rReq );
            OUString aOld = pConstCustomShape->GetShapeType();
            if( aNew == aOld )
            {
                bDeselect = true;
            }
        }
    }

    //deselect if same shape is selected again (but different custom shapes do have same slot id)
    if ( bDeselect || (nSlotId == m_nDrawSfxId &&
            (!pStringItem || (pStringItem->GetValue() == m_sDrawCustom))
                && (nSlotId != SID_DRAW_CS_ID) ) )
    {
        if (GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(nullptr);
        }

        if (m_pWrtShell->IsObjSelected() && !m_pWrtShell->IsSelFrameMode())
            m_pWrtShell->EnterSelFrameMode();
        LeaveDrawCreate();

        AttrChangedNotify(m_pWrtShell.get());
        return;
    }

    LeaveDrawCreate();

    if (m_pWrtShell->IsFrameSelected())
        m_pWrtShell->EnterStdMode();  // because bug #45639

    std::unique_ptr<SwDrawBase> pFuncPtr;

    switch (nSlotId)
    {
        case SID_OBJECT_SELECT:
        case SID_DRAW_SELECT:
            pFuncPtr.reset( new DrawSelection(m_pWrtShell.get(), m_pEditWin, this) );
            m_nDrawSfxId = m_nFormSfxId = SID_OBJECT_SELECT;
            m_sDrawCustom.clear();
            break;

        case SID_LINE_ARROW_END:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_ARROW_START:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        case SID_LINE_ARROWS:
        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            pFuncPtr.reset( new ConstRectangle(m_pWrtShell.get(), m_pEditWin, this) );
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom.clear();
            break;

        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_FREELINE:
            pFuncPtr.reset( new ConstPolygon(m_pWrtShell.get(), m_pEditWin, this) );
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom.clear();
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pFuncPtr.reset( new ConstArc(m_pWrtShell.get(), m_pEditWin, this) );
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom.clear();
            break;

        case SID_FM_CREATE_CONTROL:
        {
            const SfxUInt16Item* pIdentifierItem = rReq.GetArg<SfxUInt16Item>(SID_FM_CONTROL_IDENTIFIER);
            if( pIdentifierItem )
                nSlotId = pIdentifierItem->GetValue();
            pFuncPtr.reset( new ConstFormControl(m_pWrtShell.get(), m_pEditWin, this) );
            m_nFormSfxId = nSlotId;
        }
        break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            pFuncPtr.reset( new ConstCustomShape(m_pWrtShell.get(), m_pEditWin, this, rReq ) );
            m_nDrawSfxId = nSlotId;
            if ( nSlotId != SID_DRAW_CS_ID )
            {
                if ( pStringItem )
                {
                    m_sDrawCustom = pStringItem->GetValue();
                    SfxBindings& rBind = GetViewFrame()->GetBindings();
                    rBind.Invalidate( nSlotId );
                    rBind.Update( nSlotId );
                }
            }
        }
        break;

        default:
            break;
    }

    GetViewFrame()->GetBindings().Invalidate(SID_ATTRIBUTES_AREA);

    bool bEndTextEdit = true;
    if (pFuncPtr)
    {
        if (GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
        }

        auto pTempFuncPtr = pFuncPtr.get();
        SetDrawFuncPtr(std::move(pFuncPtr));
        AttrChangedNotify(m_pWrtShell.get());

        pTempFuncPtr->Activate(nSlotId);
        NoRotate();
        if(rReq.GetModifier() == KEY_MOD1)
        {
            if(SID_OBJECT_SELECT == m_nDrawSfxId )
            {
                m_pWrtShell->GotoObj(true);
            }
            else
            {
                pTempFuncPtr->CreateDefaultObject();
                pTempFuncPtr->Deactivate();
                SetDrawFuncPtr(nullptr);
                LeaveDrawCreate();
                m_pWrtShell->EnterStdMode();
                SdrView *pTmpSdrView = m_pWrtShell->GetDrawView();
                const SdrMarkList& rMarkList = pTmpSdrView->GetMarkedObjectList();
                if(rMarkList.GetMarkCount() == 1 &&
                        (SID_DRAW_TEXT == nSlotId || SID_DRAW_TEXT_VERTICAL == nSlotId ||
                            SID_DRAW_TEXT_MARQUEE == nSlotId ))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                    BeginTextEdit(pObj);
                    bEndTextEdit = false;
                }
            }
        }
    }
    else
    {
        if (m_pWrtShell->IsObjSelected() && !m_pWrtShell->IsSelFrameMode())
            m_pWrtShell->EnterSelFrameMode();
    }

    if(bEndTextEdit && pSdrView && pSdrView->IsTextEdit())
        pSdrView->SdrEndTextEdit( true );

    AttrChangedNotify(m_pWrtShell.get());
}

// End drawing

void SwView::ExitDraw()
{
    NoRotate();

    if(m_pShell)
    {
        // the shell may be invalid at close/reload/SwitchToViewShell
        SfxDispatcher* pDispatch = GetViewFrame()->GetDispatcher();
        sal_uInt16 nIdx = 0;
        SfxShell* pTest = nullptr;
        do
        {
            pTest = pDispatch->GetShell(nIdx++);
        }
        while( pTest && pTest != this && pTest != m_pShell);
        if(pTest == m_pShell &&
            // don't call LeaveSelFrameMode() etc. for the below,
            // because objects may still be selected:
            dynamic_cast< const SwDrawBaseShell *>( m_pShell ) ==  nullptr &&
            dynamic_cast< const SwBezierShell *>( m_pShell ) ==  nullptr &&
            dynamic_cast< const svx::ExtrusionBar *>( m_pShell ) ==  nullptr &&
            dynamic_cast< const svx::FontworkBar *>( m_pShell ) ==  nullptr)
        {
            SdrView *pSdrView = m_pWrtShell->GetDrawView();

            if (pSdrView && pSdrView->IsGroupEntered())
            {
                pSdrView->LeaveOneGroup();
                pSdrView->UnmarkAll();
                GetViewFrame()->GetBindings().Invalidate(SID_ENTER_GROUP);
            }

            if (GetDrawFuncPtr())
            {
                if (m_pWrtShell->IsSelFrameMode())
                    m_pWrtShell->LeaveSelFrameMode();
                GetDrawFuncPtr()->Deactivate();

                SetDrawFuncPtr(nullptr);
                LeaveDrawCreate();

                GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
            GetEditWin().SetPointer(Pointer(PointerStyle::Text));
        }
    }
}

// Disable rotate mode

void SwView::NoRotate()
{
    if (IsDrawRotate())
    {
        m_pWrtShell->SetDragMode(SdrDragMode::Move);
        FlipDrawRotate();

        const SfxBoolItem aTmp( SID_OBJECT_ROTATE, false );
        GetViewFrame()->GetBindings().SetState( aTmp );
    }
}

// Enable DrawTextEditMode

static bool lcl_isTextBox(SdrObject const * pObject)
{
    if (SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pObject->GetUserCall()))
    {
        if (SwFrameFormat* pFormat = pDrawContact->GetFormat())
            return SwTextBoxHelper::isTextBox(pFormat, RES_DRAWFRMFMT);
    }
    return false;
}

bool SwView::EnterDrawTextMode(const Point& aDocPos)
{
    SwWrtShell *pSh = &GetWrtShell();
    SdrView *pSdrView = pSh->GetDrawView();
    OSL_ENSURE( pSdrView, "EnterDrawTextMode without DrawView?" );

    bool bReturn = false;

    sal_uInt16 nOld = pSdrView->GetHitTolerancePixel();
    pSdrView->SetHitTolerancePixel( 2 );

    SdrObject* pObj = nullptr;
    SdrPageView* pPV = nullptr;
    if (pSdrView->IsMarkedHit(aDocPos) && !pSdrView->PickHandle(aDocPos) && IsTextTool())
        pObj = pSdrView->PickObj(aDocPos, pSdrView->getHitTolLog(), pPV, SdrSearchOptions::PICKTEXTEDIT);

    if (pObj &&
        // To allow SwDrawVirtObj text objects to be activated, allow their type, too.
        ( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr ||
          ( dynamic_cast< const SwDrawVirtObj *>( pObj ) !=  nullptr &&
            dynamic_cast< const SdrTextObj *>(&static_cast<SwDrawVirtObj*>(pObj)->GetReferencedObj() ) != nullptr ) ) &&

        m_pWrtShell->IsSelObjProtected(FlyProtectFlags::Content) == FlyProtectFlags::NONE)
    {
        // Refuse to edit editeng text of the shape if it has textbox attached.
        if (!lcl_isTextBox(pObj))
            bReturn = BeginTextEdit( pObj, pPV, m_pEditWin );
    }

    pSdrView->SetHitTolerancePixel( nOld );

    return bReturn;
}

bool SwView::EnterShapeDrawTextMode(SdrObject* pObject)
{
    SdrView* pSdrView = GetWrtShell().GetDrawView();
    SdrPageView* pPageView = pSdrView->GetSdrPageView();
    return BeginTextEdit(pObject, pPageView, m_pEditWin);
}

// Enable DrawTextEditMode

bool SwView::BeginTextEdit(SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin,
        bool bIsNewObj, bool bSetSelectionToStart)
{
    SwWrtShell *pSh = &GetWrtShell();
    SdrView *pSdrView = pSh->GetDrawView();
    std::unique_ptr<SdrOutliner> pOutliner = ::SdrMakeOutliner(OutlinerMode::TextObject, *pSdrView->GetModel());
    uno::Reference< linguistic2::XSpellChecker1 >  xSpell( ::GetSpellChecker() );
    if (pOutliner)
    {
        pOutliner->SetRefDevice(pSh->getIDocumentDeviceAccess().getReferenceDevice(false));
        pOutliner->SetSpeller(xSpell);
        uno::Reference<linguistic2::XHyphenator> xHyphenator( ::GetHyphenator() );
        pOutliner->SetHyphenator( xHyphenator );
        pSh->SetCalcFieldValueHdl(pOutliner.get());

        EEControlBits nCntrl = pOutliner->GetControlWord();
        nCntrl |= EEControlBits::ALLOWBIGOBJS;

        const SwViewOption *pOpt = pSh->GetViewOptions();

        if (SwViewOption::IsFieldShadings())
            nCntrl |= EEControlBits::MARKFIELDS;
        else
            nCntrl &= ~EEControlBits::MARKFIELDS;

        if (pOpt->IsOnlineSpell())
            nCntrl |= EEControlBits::ONLINESPELLING;
        else
            nCntrl &= ~EEControlBits::ONLINESPELLING;

        pOutliner->SetControlWord(nCntrl);
        const SfxPoolItem& rItem = pSh->GetDoc()->GetDefault(RES_CHRATR_LANGUAGE);
        pOutliner->SetDefaultLanguage(static_cast<const SvxLanguageItem&>(rItem).GetLanguage());

        if( bIsNewObj )
            pOutliner->SetVertical( SID_DRAW_TEXT_VERTICAL == m_nDrawSfxId ||
                                    SID_DRAW_CAPTION_VERTICAL == m_nDrawSfxId );

        // set default horizontal text direction at outliner
        EEHorizontalTextDirection aDefHoriTextDir =
            pSh->IsShapeDefaultHoriTextDirR2L() ? EEHorizontalTextDirection::R2L : EEHorizontalTextDirection::L2R;
        pOutliner->SetDefaultHorizontalTextDirection( aDefHoriTextDir );
    }

    // To allow editing the referenced object from a SwDrawVirtObj here
    // the original needs to be fetched eventually. This ATM activates the
    // text edit mode for the original object.
    SdrObject* pToBeActivated = pObj;

    // Always the original object is edited. To allow the TextEdit to happen
    // where the VirtObj is positioned, on demand a occurring offset is set at
    // the TextEdit object. That offset is used for creating and managing the
    // OutlinerView.
    Point aNewTextEditOffset(0, 0);

    if (SwDrawVirtObj* pVirtObj = dynamic_cast<SwDrawVirtObj *>(pObj))
    {
        pToBeActivated = &const_cast<SdrObject&>(pVirtObj->GetReferencedObj());
        aNewTextEditOffset = pVirtObj->GetOffset();
    }

    // set in each case, thus it will be correct for all objects
    static_cast<SdrTextObj*>(pToBeActivated)->SetTextEditOffset(aNewTextEditOffset);

    bool bRet(pSdrView->SdrBeginTextEdit( pToBeActivated, pPV, pWin, true, pOutliner.release(), nullptr, false, false, false ));

    // #i7672#
    // Since SdrBeginTextEdit actually creates the OutlinerView and thus also
    // sets the background color, an own background color needs to be set
    // after TextEditing was started. This is now done here.
    if(bRet)
    {
        OutlinerView* pView = pSdrView->GetTextEditOutlinerView();

        if(pView)
        {
            Color aBackground(pSh->GetShapeBackgrd());
            pView->SetBackgroundColor(aBackground);
        }

        // editing should start at the end of text, spell checking at the beginning ...
        ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
        if (bSetSelectionToStart)
            aNewSelection = ESelection();
        if (pView)
        {
            pView->SetSelection(aNewSelection);

            if (comphelper::LibreOfficeKit::isActive())
            {
                OString sRect = pView->GetOutputArea().toString();
                SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_VIEW_LOCK, "rectangle", sRect);
            }
        }
    }

    return bRet;
}

// Is a DrawTextObject selected?

bool SwView::IsTextTool() const
{
    sal_uInt16  nId;
    SdrInventor nInvent;
    SdrView *pSdrView = GetWrtShell().GetDrawView();
    OSL_ENSURE( pSdrView, "IsTextTool without DrawView?" );

    if (pSdrView->IsCreateMode())
        pSdrView->SetCreateMode(false);

    pSdrView->TakeCurrentObj(nId,nInvent);
    return nInvent == SdrInventor::Default;
}

SdrView* SwView::GetDrawView() const
{
    return GetWrtShell().GetDrawView();
}

bool SwView::IsBezierEditMode()
{
    return (!IsDrawSelMode() && GetWrtShell().GetDrawView()->HasMarkablePoints());
}

bool SwView::IsFormMode() const
{
    if (GetDrawFuncPtr() && GetDrawFuncPtr()->IsCreateObj())
    {
        return GetDrawFuncPtr()->IsInsertForm();
    }

    return AreOnlyFormsSelected();
}

void SwView::SetDrawFuncPtr(std::unique_ptr<SwDrawBase> pFuncPtr)
{
    m_pDrawActual = std::move(pFuncPtr);
}

void SwView::SetSelDrawSlot()
{
    m_nDrawSfxId = SID_OBJECT_SELECT;
    m_sDrawCustom.clear();
}

bool SwView::AreOnlyFormsSelected() const
{
    if ( GetWrtShell().IsFrameSelected() )
        return false;

    bool bForm = true;

    SdrView* pSdrView = GetWrtShell().GetDrawView();

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    if (nCount)
    {
        for (size_t i = 0; i < nCount; ++i)
        {
            // Except controls, are still normal draw objects selected?
            SdrObject *pSdrObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

            if (!HasOnlyObj(pSdrObj, SdrInventor::FmForm))
            {
                bForm = false;
                break;
            }
        }
    }
    else
        bForm = false;

    return bForm;
}

bool SwView::HasOnlyObj(SdrObject const *pSdrObj, SdrInventor eObjInventor) const
{
    bool bRet = false;

    if (pSdrObj->IsGroupObject())
    {
        SdrObjList* pList = pSdrObj->GetSubList();
        const size_t nCnt = pList->GetObjCount();

        for (size_t i = 0; i < nCnt; ++i)
        {
            bRet = HasOnlyObj(pList->GetObj(i), eObjInventor);
            if (!bRet)
                break;
        }
    }
    else if (eObjInventor == pSdrObj->GetObjInventor())
        return true;

    return bRet;
}

//#i87414# mod
IMPL_LINK(SwView, OnlineSpellCallback, SpellCallbackInfo&, rInfo, void)
{
    if (rInfo.nCommand == SpellCallbackCommand::STARTSPELLDLG)
        GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON);
    else if (rInfo.nCommand == SpellCallbackCommand::AUTOCORRECT_OPTIONS)
        GetViewFrame()->GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
}

bool SwView::ExecDrwTextSpellPopup(const Point& rPt)
{
    bool bRet = false;
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    Point aPos( GetEditWin().LogicToPixel( rPt ) );

    if (pOLV->IsWrongSpelledWordAtPos( aPos ))
    {
        bRet = true;
        Link<SpellCallbackInfo&,void> aLink = LINK(this, SwView, OnlineSpellCallback);
        pOLV->ExecuteSpellPopup( aPos,&aLink );
    }
    return bRet;
}

bool SwView::IsDrawTextHyphenate()
{
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    bool bHyphenate = false;

    SfxItemSet aNewAttr( pSdrView->GetModel()->GetItemPool(),
                            svl::Items<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE>{} );
    if( pSdrView->GetAttributes( aNewAttr ) &&
        aNewAttr.GetItemState( EE_PARA_HYPHENATE ) >= SfxItemState::DEFAULT )
        bHyphenate = aNewAttr.Get( EE_PARA_HYPHENATE ).GetValue();

    return bHyphenate;
}

void SwView::HyphenateDrawText()
{
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    bool bHyphenate = IsDrawTextHyphenate();

    SfxItemSet aSet( GetPool(), svl::Items<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE>{} );
    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, !bHyphenate ) );
    pSdrView->SetAttributes( aSet );
    GetViewFrame()->GetBindings().Invalidate(FN_HYPHENATE_OPT_DLG);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
