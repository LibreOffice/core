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


#include "hintids.hxx"
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

#include "view.hxx"
#include "wrtsh.hxx"
#include "viewopt.hxx"
#include "cmdid.h"
#include "drawsh.hxx"
#include "drwbassh.hxx"
#include "beziersh.hxx"
#include "conrect.hxx"
#include "conpoly.hxx"
#include "conarc.hxx"
#include "conform.hxx"
#include "concustomshape.hxx"
#include "dselect.hxx"
#include "edtwin.hxx"

#include <dcontact.hxx>

#include <svx/svdpagv.hxx>
#include <svx/extrusionbar.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

// Execute Drawing-Ids

void SwView::ExecDraw(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    const SfxAllEnumItem* pEItem = 0;
    const SfxStringItem* pStringItem = 0;
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    sal_Bool bDeselect = sal_False;

    sal_uInt16 nSlotId = rReq.GetSlot();
    if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(GetPool().GetWhich(nSlotId), false, &pItem))
    {
        pEItem = dynamic_cast< const SfxAllEnumItem*>(pItem);
        pStringItem = dynamic_cast< const SfxStringItem*>(pItem);
    }

    if (SID_INSERT_DRAW == nSlotId && pEItem)
        switch ( pEItem->GetValue() )
        {
            case SVX_SNAP_DRAW_SELECT:              nSlotId = SID_OBJECT_SELECT;            break;
            case SVX_SNAP_DRAW_LINE:                nSlotId = SID_DRAW_LINE;                break;
            case SVX_SNAP_DRAW_RECT:                nSlotId = SID_DRAW_RECT;                break;
            case SVX_SNAP_DRAW_ELLIPSE:             nSlotId = SID_DRAW_ELLIPSE;             break;
            case SVX_SNAP_DRAW_POLYGON_NOFILL:      nSlotId = SID_DRAW_POLYGON_NOFILL;      break;
            case SVX_SNAP_DRAW_BEZIER_NOFILL:       nSlotId = SID_DRAW_BEZIER_NOFILL;       break;
            case SVX_SNAP_DRAW_FREELINE_NOFILL:     nSlotId = SID_DRAW_FREELINE_NOFILL;     break;
            case SVX_SNAP_DRAW_ARC:                 nSlotId = SID_DRAW_ARC;                 break;
            case SVX_SNAP_DRAW_PIE:                 nSlotId = SID_DRAW_PIE;                 break;
            case SVX_SNAP_DRAW_CIRCLECUT:           nSlotId = SID_DRAW_CIRCLECUT;           break;
            case SVX_SNAP_DRAW_TEXT:                nSlotId = SID_DRAW_TEXT;                break;
            case SVX_SNAP_DRAW_TEXT_VERTICAL:       nSlotId = SID_DRAW_TEXT_VERTICAL;       break;
            case SVX_SNAP_DRAW_TEXT_MARQUEE:        nSlotId = SID_DRAW_TEXT_MARQUEE;        break;
            case SVX_SNAP_DRAW_CAPTION:             nSlotId = SID_DRAW_CAPTION;             break;
            case SVX_SNAP_DRAW_CAPTION_VERTICAL:    nSlotId = SID_DRAW_CAPTION_VERTICAL;    break;
        }

    if (nSlotId == SID_OBJECT_SELECT && m_nFormSfxId == nSlotId)
    {
        bDeselect = sal_True;
    }
    else if (nSlotId == SID_FM_CREATE_CONTROL)
    {
        SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER, false );
        if( pIdentifierItem )
        {
            sal_uInt16 nNewId = pIdentifierItem->GetValue();
            if (nNewId == m_nFormSfxId)
            {
                bDeselect = sal_True;
                GetViewFrame()->GetDispatcher()->Execute(SID_FM_LEAVE_CREATE);  // Button should popping out
            }
        }
    }
    else if( nSlotId == SID_FM_CREATE_FIELDCONTROL)
    {
        FmFormView* pFormView = PTR_CAST( FmFormView, pSdrView );
        if ( pFormView )
        {
            SFX_REQUEST_ARG( rReq, pDescriptorItem, SfxUnoAnyItem, SID_FM_DATACCESS_DESCRIPTOR, false );
            OSL_ENSURE( pDescriptorItem, "SwView::ExecDraw(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );
            if( pDescriptorItem )
            {
                ::svx::ODataAccessDescriptor aDescriptor( pDescriptorItem->GetValue() );
                SdrObject* pObj = pFormView->CreateFieldControl( aDescriptor );

                if ( pObj )
                {
                    Size aDocSize(m_pWrtShell->GetDocSize());
                    const SwRect& rVisArea = m_pWrtShell->VisArea();
                    Point aStartPos = rVisArea.Center();
                    if(rVisArea.Width() > aDocSize.Width())
                        aStartPos.X() = aDocSize.Width() / 2 + rVisArea.Left();
                    if(rVisArea.Height() > aDocSize.Height())
                        aStartPos.Y() = aDocSize.Height() / 2 + rVisArea.Top();

                    //determine the size of the object
                    if(pObj->IsGroupObject())
                    {
                        const Rectangle& rBoundRect = ((SdrObjGroup*)pObj)->GetCurrentBoundRect();
                        aStartPos.X() -= rBoundRect.GetWidth()/2;
                        aStartPos.Y() -= rBoundRect.GetHeight()/2;
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
        Window*  pWin = &( m_pWrtShell->GetView().GetViewFrame()->GetWindow() );

        if ( pWin )
            pWin->EnterWait();

        if( !m_pWrtShell->HasDrawView() )
            m_pWrtShell->MakeDrawView();

        pSdrView = m_pWrtShell->GetDrawView();
        if ( pSdrView )
        {
            SdrObject* pObj = NULL;
            svx::FontWorkGalleryDialog aDlg( pSdrView, pWin, nSlotId );
            aDlg.SetSdrObjectRef( &pObj, pSdrView->GetModel() );
            aDlg.Execute();
            if ( pObj )
            {
                Size            aDocSize( m_pWrtShell->GetDocSize() );
                const SwRect&   rVisArea = m_pWrtShell->VisArea();
                Point           aPos( rVisArea.Center() );
                Size            aSize;
                Size            aPrefSize( pObj->GetSnapRect().GetSize() );

                if( rVisArea.Width() > aDocSize.Width())
                    aPos.X() = aDocSize.Width() / 2 + rVisArea.Left();

                if(rVisArea.Height() > aDocSize.Height())
                    aPos.Y() = aDocSize.Height() / 2 + rVisArea.Top();

                if( aPrefSize.Width() && aPrefSize.Height() )
                {
                    if( pWin )
                        aSize = pWin->PixelToLogic( aPrefSize, MAP_TWIP );
                    else
                        aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_TWIP );
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

    if( nSlotId == SID_DRAW_CS_ID )
    {
        //deselect if same custom shape is selected again
        SwDrawBase* pFuncPtr = GetDrawFuncPtr();
        if( pFuncPtr && pFuncPtr->GetSlotId() == SID_DRAW_CS_ID )
        {
            ConstCustomShape* pConstCustomShape = (ConstCustomShape*)(pFuncPtr);
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
            SetDrawFuncPtr(NULL);
        }

        if (m_pWrtShell->IsObjSelected() && !m_pWrtShell->IsSelFrmMode())
            m_pWrtShell->EnterSelFrmMode(NULL);
        LeaveDrawCreate();

        GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

        AttrChangedNotify(m_pWrtShell);
        return;
    }

    LeaveDrawCreate();

    if (m_pWrtShell->IsFrmSelected())
        m_pWrtShell->EnterStdMode();  // because bug #45639

    SwDrawBase* pFuncPtr = NULL;

    switch (nSlotId)
    {
        case SID_OBJECT_SELECT:
        case SID_DRAW_SELECT:
            pFuncPtr = new DrawSelection(m_pWrtShell, m_pEditWin, this);
            m_nDrawSfxId = m_nFormSfxId = SID_OBJECT_SELECT;
            m_sDrawCustom = "";
            break;

        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            pFuncPtr = new ConstRectangle(m_pWrtShell, m_pEditWin, this);
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom = "";
            break;

        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
            pFuncPtr = new ConstPolygon(m_pWrtShell, m_pEditWin, this);
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom = "";
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pFuncPtr = new ConstArc(m_pWrtShell, m_pEditWin, this);
            m_nDrawSfxId = nSlotId;
            m_sDrawCustom = "";
            break;

        case SID_FM_CREATE_CONTROL:
        {
            SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER, false );
            if( pIdentifierItem )
                nSlotId = pIdentifierItem->GetValue();
            pFuncPtr = new ConstFormControl(m_pWrtShell, m_pEditWin, this);
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
            pFuncPtr = new ConstCustomShape(m_pWrtShell, m_pEditWin, this, rReq );
            m_nDrawSfxId = nSlotId;
            if ( nSlotId != SID_DRAW_CS_ID )
            {
                if ( pStringItem )
                {
                    m_sDrawCustom = pStringItem->GetValue();
                    m_aCurrShapeEnumCommand[ nSlotId - SID_DRAWTBX_CS_BASIC ] = m_sDrawCustom;
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

    static sal_uInt16 const aInval[] =
    {
        // Slot IDs must be sorted when calling Invalidate!
        SID_ATTRIBUTES_AREA,
        SID_INSERT_DRAW,
        0
    };
    GetViewFrame()->GetBindings().Invalidate(aInval);

    sal_Bool bEndTextEdit = sal_True;
    if (pFuncPtr)
    {
        if (GetDrawFuncPtr())
        {
            GetDrawFuncPtr()->Deactivate();
            SetDrawFuncPtr(NULL);
        }

        SetDrawFuncPtr(pFuncPtr);
        AttrChangedNotify(m_pWrtShell);

        pFuncPtr->Activate(nSlotId);
        NoRotate();
        if(rReq.GetModifier() == KEY_MOD1)
        {
            if(SID_OBJECT_SELECT == m_nDrawSfxId )
            {
                m_pWrtShell->GotoObj(sal_True);
            }
            else
            {
                pFuncPtr->CreateDefaultObject();
                pFuncPtr->Deactivate();
                SetDrawFuncPtr(NULL);
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
                    bEndTextEdit = sal_False;
                }
            }
        }
    }
    else
    {
        if (m_pWrtShell->IsObjSelected() && !m_pWrtShell->IsSelFrmMode())
            m_pWrtShell->EnterSelFrmMode(NULL);
    }

    if(bEndTextEdit && pSdrView && pSdrView->IsTextEdit())
        pSdrView->SdrEndTextEdit( sal_True );

    AttrChangedNotify(m_pWrtShell);
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
        SfxShell* pTest = 0;
        do
        {
            pTest = pDispatch->GetShell(nIdx++);
        }
        while( pTest && pTest != this && pTest != m_pShell);
        if(pTest == m_pShell &&
            // don't call LeaveSelFrmMode() etc. for the below,
            // because objects may still be selected:
            !m_pShell->ISA(SwDrawBaseShell) &&
            !m_pShell->ISA(SwBezierShell) &&
            !m_pShell->ISA(svx::ExtrusionBar) &&
            !m_pShell->ISA(svx::FontworkBar))
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
                if (m_pWrtShell->IsSelFrmMode())
                    m_pWrtShell->LeaveSelFrmMode();
                GetDrawFuncPtr()->Deactivate();

                SetDrawFuncPtr(NULL);
                LeaveDrawCreate();

                GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
            GetEditWin().SetPointer(Pointer(POINTER_TEXT));
        }
    }
}

// Disable rotate mode

void SwView::NoRotate()
{
    if (IsDrawRotate())
    {
        m_pWrtShell->SetDragMode(SDRDRAG_MOVE);
        FlipDrawRotate();

        const SfxBoolItem aTmp( SID_OBJECT_ROTATE, false );
        GetViewFrame()->GetBindings().SetState( aTmp );
    }
}

// Enable DrawTextEditMode

sal_Bool SwView::EnterDrawTextMode(const Point& aDocPos)
{
    SdrObject* pObj;
    SdrPageView* pPV;
    SwWrtShell *pSh = &GetWrtShell();
    SdrView *pSdrView = pSh->GetDrawView();
    OSL_ENSURE( pSdrView, "EnterDrawTextMode without DrawView?" );

    sal_Bool bReturn = sal_False;

    sal_uInt16 nOld = pSdrView->GetHitTolerancePixel();
    pSdrView->SetHitTolerancePixel( 2 );

    if( pSdrView->IsMarkedHit( aDocPos ) &&
        !pSdrView->PickHandle( aDocPos ) && IsTextTool() &&
        pSdrView->PickObj( aDocPos, pSdrView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKTEXTEDIT ) &&

        // To allow SwDrawVirtObj text objects to be activated, allow their type, too.
        ( pObj->ISA( SdrTextObj ) ||
          ( pObj->ISA(SwDrawVirtObj) &&
            ((SwDrawVirtObj*)pObj)->GetReferencedObj().ISA(SdrTextObj) ) ) &&

        !m_pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT))
    {
        bReturn = BeginTextEdit( pObj, pPV, m_pEditWin, false );
    }

    pSdrView->SetHitTolerancePixel( nOld );

    return bReturn;
}

// Enable DrawTextEditMode

sal_Bool SwView::BeginTextEdit(SdrObject* pObj, SdrPageView* pPV, Window* pWin,
        bool bIsNewObj, bool bSetSelectionToStart)
{
    SwWrtShell *pSh = &GetWrtShell();
    SdrView *pSdrView = pSh->GetDrawView();
    SdrOutliner* pOutliner = ::SdrMakeOutliner(OUTLINERMODE_TEXTOBJECT, pSdrView->GetModel());
    uno::Reference< linguistic2::XSpellChecker1 >  xSpell( ::GetSpellChecker() );
    if (pOutliner)
    {
        pOutliner->SetRefDevice(pSh->getIDocumentDeviceAccess()->getReferenceDevice(false));
        pOutliner->SetSpeller(xSpell);
        uno::Reference<linguistic2::XHyphenator> xHyphenator( ::GetHyphenator() );
        pOutliner->SetHyphenator( xHyphenator );
        pSh->SetCalcFieldValueHdl(pOutliner);

        sal_uInt32 nCntrl = pOutliner->GetControlWord();
        nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
        nCntrl |= EE_CNTRL_URLSFXEXECUTE;

        const SwViewOption *pOpt = pSh->GetViewOptions();

        if (SwViewOption::IsFieldShadings())
            nCntrl |= EE_CNTRL_MARKFIELDS;
        else
            nCntrl &= ~EE_CNTRL_MARKFIELDS;

        if (pOpt->IsOnlineSpell())
            nCntrl |= EE_CNTRL_ONLINESPELLING;
        else
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;

        pOutliner->SetControlWord(nCntrl);
        const SfxPoolItem& rItem = pSh->GetDoc()->GetDefault(RES_CHRATR_LANGUAGE);
        pOutliner->SetDefaultLanguage(((const SvxLanguageItem&)rItem).GetLanguage());

        if( bIsNewObj )
            pOutliner->SetVertical( SID_DRAW_TEXT_VERTICAL == m_nDrawSfxId ||
                                    SID_DRAW_CAPTION_VERTICAL == m_nDrawSfxId );

        // set default horizontal text direction at outliner
        EEHorizontalTextDirection aDefHoriTextDir =
            pSh->IsShapeDefaultHoriTextDirR2L() ? EE_HTEXTDIR_R2L : EE_HTEXTDIR_L2R;
        pOutliner->SetDefaultHorizontalTextDirection( aDefHoriTextDir );
    }

    // To allow editing the referenced object from a SwDrawVirtObj here
    // the original needs to be fetched evenually. This ATM activates the
    // text edit mode for the original object.
    SdrObject* pToBeActivated = pObj;

    // Always the original object is edited. To allow the TextEdit to happen
    // where the VirtObj is positioned, on demand a occurring offset is set at
    // the TextEdit object. That offset is used for creating and managing the
    // OutlinerView.
    Point aNewTextEditOffset(0, 0);

    if(pObj->ISA(SwDrawVirtObj))
    {
        SwDrawVirtObj* pVirtObj = (SwDrawVirtObj*)pObj;
        pToBeActivated = &((SdrObject&)pVirtObj->GetReferencedObj());
        aNewTextEditOffset = pVirtObj->GetOffset();
    }

    // set in each case, thus it will be correct for all objects
    ((SdrTextObj*)pToBeActivated)->SetTextEditOffset(aNewTextEditOffset);

    sal_Bool bRet(pSdrView->SdrBeginTextEdit( pToBeActivated, pPV, pWin, sal_True, pOutliner, 0, sal_False, sal_False, sal_False ));

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
        pView->SetSelection(aNewSelection);
    }

    return bRet;
}

// Is a DrawTextObjekt selected?

bool SwView::IsTextTool() const
{
    sal_uInt16 nId;
    sal_uInt32 nInvent;
    SdrView *pSdrView = GetWrtShell().GetDrawView();
    OSL_ENSURE( pSdrView, "IsTextTool without DrawView?" );

    if (pSdrView->IsCreateMode())
        pSdrView->SetCreateMode(sal_False);

    pSdrView->TakeCurrentObj(nId,nInvent);
    return (nInvent==SdrInventor);
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
        return (GetDrawFuncPtr()->IsInsertForm());
    }

    return AreOnlyFormsSelected();
}

void SwView::SetDrawFuncPtr(SwDrawBase* pFuncPtr)
{
    delete m_pDrawActual;
    m_pDrawActual = pFuncPtr;
}

void SwView::SetSelDrawSlot()
{
    m_nDrawSfxId = SID_OBJECT_SELECT;
    m_sDrawCustom = "";
}

bool SwView::AreOnlyFormsSelected() const
{
    if ( GetWrtShell().IsFrmSelected() )
        return false;

    bool bForm = true;

    SdrView* pSdrView = GetWrtShell().GetDrawView();

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uInt32 nCount = rMarkList.GetMarkCount();

    if (nCount)
    {
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            // Except controls, are still normal draw objects selected?
            SdrObject *pSdrObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

            if (!HasOnlyObj(pSdrObj, FmFormInventor))
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

sal_Bool SwView::HasDrwObj(SdrObject *pSdrObj) const
{
    sal_Bool bRet = sal_False;

    if (pSdrObj->IsGroupObject())
    {
        SdrObjList* pList = pSdrObj->GetSubList();
        sal_uInt32 nCnt = pList->GetObjCount();

        for (sal_uInt32 i = 0; i < nCnt; i++)
            if ((bRet = HasDrwObj(pList->GetObj(i))) == sal_True)
                break;
    }
    else if (SdrInventor == pSdrObj->GetObjInventor() || pSdrObj->Is3DObj())
        return sal_True;

    return bRet;
}

sal_Bool SwView::HasOnlyObj(SdrObject *pSdrObj, sal_uInt32 eObjInventor) const
{
    sal_Bool bRet = sal_False;

    if (pSdrObj->IsGroupObject())
    {
        SdrObjList* pList = pSdrObj->GetSubList();
        sal_uInt32 nCnt = pList->GetObjCount();

        for (sal_uInt32 i = 0; i < nCnt; i++)
            if ((bRet = HasOnlyObj(pList->GetObj(i), eObjInventor)) == sal_False)
                break;
    }
    else if (eObjInventor == pSdrObj->GetObjInventor())
        return sal_True;

    return bRet;
}

//#i87414# mod
IMPL_LINK(SwView, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    if (pInfo->nCommand == SPELLCMD_STARTSPELLDLG)
        GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

sal_Bool SwView::ExecDrwTxtSpellPopup(const Point& rPt)
{
    sal_Bool bRet = sal_False;
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    Point aPos( GetEditWin().LogicToPixel( rPt ) );

    if (pOLV->IsWrongSpelledWordAtPos( aPos ))
    {
        bRet = sal_True;
        Link aLink = LINK(this, SwView, OnlineSpellCallback);
        pOLV->ExecuteSpellPopup( aPos,&aLink );
    }
    return bRet;
}

sal_Bool SwView::IsDrawTextHyphenate()
{
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    sal_Bool bHyphenate = sal_False;

    SfxItemSet aNewAttr( pSdrView->GetModel()->GetItemPool(),
                            EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
    if( pSdrView->GetAttributes( aNewAttr ) &&
        aNewAttr.GetItemState( EE_PARA_HYPHENATE ) >= SFX_ITEM_AVAILABLE )
        bHyphenate = ((const SfxBoolItem&)aNewAttr.Get( EE_PARA_HYPHENATE )).
                        GetValue();

    return bHyphenate;
}

void SwView::HyphenateDrawText()
{
    SdrView *pSdrView = m_pWrtShell->GetDrawView();
    sal_Bool bHyphenate = IsDrawTextHyphenate();

    SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, !bHyphenate ) );
    pSdrView->SetAttributes( aSet );
    GetViewFrame()->GetBindings().Invalidate(FN_HYPHENATE_OPT_DLG);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
