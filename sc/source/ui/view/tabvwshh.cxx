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

#include <config_features.h>

#include <basic/sberrors.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>

#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <client.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <sc.hrc>
#include <scmod.hxx>
#include <drwlayer.hxx>
#include <retypepassdlg.hxx>
#include <tabprotection.hxx>
#include <IAnyRefDialog.hxx>

using namespace com::sun::star;

void ScTabViewShell::ExecuteObject( const SfxRequest& rReq )
{
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

        // Always activate/deactivate object in the visible View

    ScTabViewShell* pVisibleSh = this;
    if ( nSlotId == SID_OLE_SELECT || nSlotId == SID_OLE_ACTIVATE || nSlotId == SID_OLE_DEACTIVATE )
    {
        OSL_FAIL("old slot SID_OLE...");
    }

    switch (nSlotId)
    {
        case SID_OLE_SELECT:
        case SID_OLE_ACTIVATE:
            {
                // In both cases, first select in the visible View

                OUString aName;
                SdrView* pDrView = GetSdrView();
                if (pDrView)
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                    if (rMarkList.GetMarkCount() == 1)
                        aName = ScDrawLayer::GetVisibleName( rMarkList.GetMark(0)->GetMarkedSdrObj() );
                }
                pVisibleSh->SelectObject( aName );

                // activate

                if ( nSlotId == SID_OLE_ACTIVATE )
                    pVisibleSh->DoVerb( 0 );
            }
            break;
        case SID_OLE_DEACTIVATE:
            pVisibleSh->DeactivateOle();
            break;

        case SID_OBJECT_LEFT:
        case SID_OBJECT_TOP:
        case SID_OBJECT_WIDTH:
        case SID_OBJECT_HEIGHT:
            {
                bool bDone = false;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlotId, true, &pItem ) == SfxItemState::SET )
                {
                    long nNewVal = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                    if ( nNewVal < 0 )
                        nNewVal = 0;

                    //! convert from something into 1/100mm ??????

                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            tools::Rectangle aRect = pObj->GetLogicRect();

                            if ( nSlotId == SID_OBJECT_LEFT )
                                pDrView->MoveMarkedObj( Size( nNewVal - aRect.Left(), 0 ) );
                            else if ( nSlotId == SID_OBJECT_TOP )
                                pDrView->MoveMarkedObj( Size( 0, nNewVal - aRect.Top() ) );
                            else if ( nSlotId == SID_OBJECT_WIDTH )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( nNewVal, aRect.GetWidth() ),
                                                Fraction( 1, 1 ) );
                            else // if ( nSlotId == SID_OBJECT_HEIGHT )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( 1, 1 ),
                                                Fraction( nNewVal, aRect.GetHeight() ) );
                            bDone = true;
                        }
                    }
                }
#if HAVE_FEATURE_SCRIPTING
                if (!bDone)
                    SbxBase::SetError( ERRCODE_BASIC_BAD_PARAMETER );  // basic error
#endif
            }
            break;

    }
}

static uno::Reference < embed::XEmbeddedObject > lcl_GetSelectedObj( const SdrView* pDrView )       //! member of ScDrawView?
{
    uno::Reference < embed::XEmbeddedObject > xRet;
    if (pDrView)
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if (pObj->GetObjIdentifier() == OBJ_OLE2)
            {
                SdrOle2Obj* pOle2Obj = static_cast<SdrOle2Obj*>(pObj);
                xRet = pOle2Obj->GetObjRef();
            }
        }
    }

    return xRet;
}

void ScTabViewShell::GetObjectState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_ACTIVE_OBJ_NAME:
                {
                    OUString aName;
                    uno::Reference < embed::XEmbeddedObject > xOLE = lcl_GetSelectedObj( GetSdrView() );
                    if (xOLE.is())
                    {
                        aName = GetViewData().GetSfxDocShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xOLE );
                    }
                    rSet.Put( SfxStringItem( nWhich, aName ) );
                }
                break;
            case SID_OBJECT_LEFT:
            case SID_OBJECT_TOP:
            case SID_OBJECT_WIDTH:
            case SID_OBJECT_HEIGHT:
                {
                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                            tools::Rectangle aRect = pObj->GetLogicRect();

                            long nVal;
                            if ( nWhich == SID_OBJECT_LEFT )
                                nVal = aRect.Left();
                            else if ( nWhich == SID_OBJECT_TOP )
                                nVal = aRect.Top();
                            else if ( nWhich == SID_OBJECT_WIDTH )
                                nVal = aRect.GetWidth();
                            else // if ( nWhich == SID_OBJECT_HEIGHT )
                                nVal = aRect.GetHeight();

                            //! convert from 1/100mm to something else ??????

                            rSet.Put( SfxInt32Item( nWhich, nVal ) );
                        }
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::AddAccessibilityObject( SfxListener& rObject )
{
    if (!pAccessibilityBroadcaster)
        pAccessibilityBroadcaster.reset( new SfxBroadcaster );

    rObject.StartListening( *pAccessibilityBroadcaster );
    ScDocument* pDoc = GetViewData().GetDocument();
    if (pDoc)
        pDoc->AddUnoObject(rObject);
}

void ScTabViewShell::RemoveAccessibilityObject( SfxListener& rObject )
{
    SolarMutexGuard g;

    if (pAccessibilityBroadcaster)
    {
        rObject.EndListening( *pAccessibilityBroadcaster );
        ScDocument* pDoc = GetViewData().GetDocument();
        if (pDoc)
            pDoc->RemoveUnoObject(rObject);
    }
    else
    {
        OSL_FAIL("no accessibility broadcaster?");
    }
}

void ScTabViewShell::BroadcastAccessibility( const SfxHint &rHint )
{
    if (pAccessibilityBroadcaster)
        pAccessibilityBroadcaster->Broadcast( rHint );
}

bool ScTabViewShell::HasAccessibilityObjects()
{
    return pAccessibilityBroadcaster != nullptr;
}

bool ScTabViewShell::ExecuteRetypePassDlg(ScPasswordHash eDesiredHash)
{
    ScDocument* pDoc = GetViewData().GetDocument();

    VclPtrInstance< ScRetypePassDlg > pDlg(GetDialogParent());
    pDlg->SetDataFromDocument(*pDoc);
    pDlg->SetDesiredHash(eDesiredHash);
    if (pDlg->Execute() != RET_OK)
        return false;

    pDlg->WriteNewDataToDocument(*pDoc);
    return true;
}

/**
 * Reference dialogs
 */
void ScTabViewShell::SetRefDialog( sal_uInt16 nId, bool bVis )
{
    if(m_nCurRefDlgId==0 || (nId==m_nCurRefDlgId && !bVis))
    {
        SfxViewFrame* pViewFrm = GetViewFrame();

        // bindings update causes problems with update of stylist if
        // current style family has changed
        //if ( pViewFrm )
        //  pViewFrm->GetBindings().Update();       // to avoid trouble in LockDispatcher

        m_nCurRefDlgId = bVis ? nId : 0 ;             // before SetChildWindow

        if ( pViewFrm )
        {
            pViewFrm->SetChildWindow( nId, bVis );
        }

        SfxApplication* pSfxApp = SfxGetpApp();
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
    }
}

static SfxChildWindow* lcl_GetChildWinFromCurrentView( sal_uInt16 nId )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();

    // #i46999# current view frame can be null (for example, when closing help)
    return pViewFrm ? pViewFrm->GetChildWindow( nId ) : nullptr;
}

bool ScTabViewShell::IsModalMode(SfxObjectShell* pDocSh) const
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsModal = false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsModal = pChildWnd->IsVisible() && pRefDlg &&
                !( pRefDlg->IsRefInputMode() && pRefDlg->IsDocAllowed(pDocSh) );
        }
    }
    else if (pDocSh)
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if ( pHdl )
            bIsModal = pHdl->IsModalMode(pDocSh);
    }

    return bIsModal;
}

static SfxChildWindow* lcl_GetChildWinFromAnyView( sal_uInt16 nId )
{
    // First, try the current view
    SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( nId );
    if ( pChildWnd )
        return pChildWnd;           // found in the current view

    //  if not found there, get the child window from any open view
    //  it can be open only in one view because nCurRefDlgId is global

    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst();
    while ( pViewFrm )
    {
        pChildWnd = pViewFrm->GetChildWindow( nId );
        if ( pChildWnd )
            return pChildWnd;       // found in any view

        pViewFrm = SfxViewFrame::GetNext( *pViewFrm );
    }

    return nullptr;                    // none found
}

bool ScTabViewShell::IsTableLocked() const
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bLocked = false;

    // Up until now just for ScAnyRefDlg
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg(dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow()));
            assert(pRefDlg);
            if(pRefDlg)
            {
                bLocked = pRefDlg->IsTableLocked();
            }
        }
        else
            bLocked = true;     // for other views, see IsModalMode
    }

    return bLocked;
}

bool ScTabViewShell::IsRefDialogOpen() const
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsOpen = false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
            bIsOpen = pChildWnd->IsVisible();
    }

    return bIsOpen;
}

bool ScTabViewShell::IsFormulaMode() const
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    bool bIsFormula = false;

    // formula mode in online is not usable in collaborative mode,
    // this is a workaround for disabling formula mode in online
    // when there is more than a single view
    if (comphelper::LibreOfficeKit::isActive() && SfxViewShell::GetActiveShells() > 1)
            return false;

    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromCurrentView( m_nCurRefDlgId );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            bIsFormula = pChildWnd->IsVisible() && pRefDlg && pRefDlg->IsRefInputMode();
        }
    }
    else
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if ( pHdl )
            bIsFormula = pHdl->IsFormulaMode();
    }

    if (m_bIsInEditCommand)
        bIsFormula = true;

    return bIsFormula;
}

static void lcl_MarkedTabs( const ScMarkData& rMark, SCTAB& rStartTab, SCTAB& rEndTab )
{
    if (rMark.GetSelectCount() > 1)
    {
        rEndTab = rMark.GetLastSelected();
        rStartTab = rMark.GetFirstSelected();
    }
}

void ScTabViewShell::SetReference( const ScRange& rRef, ScDocument* pDoc,
                                    const ScMarkData* pMarkData )
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents

    // In RefDialogs we also trigger the ZoomIn, if the Ref's Start and End are different
    ScRange aNew = rRef;
    aNew.PutInOrder(); // Always in the right direction

    if( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            if ( m_nCurRefDlgId == SID_OPENDLG_CONSOLIDATE && pMarkData )
            {
                SCTAB nStartTab = aNew.aStart.Tab();
                SCTAB nEndTab   = aNew.aEnd.Tab();
                lcl_MarkedTabs( *pMarkData, nStartTab, nEndTab );
                aNew.aStart.SetTab(nStartTab);
                aNew.aEnd.SetTab(nEndTab);
            }

            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                // hide the (color) selection now instead of later from LoseFocus,
                // don't abort the ref input that causes this call (bDoneRefMode = sal_False)
                pRefDlg->HideReference( false );
                pRefDlg->SetReference( aNew, pDoc );
            }
        }
    }
    else
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if (pHdl)
            pHdl->SetReference( aNew, pDoc );
        else
        {
            OSL_FAIL("SetReference without receiver");
        }
    }
}

/**
 * Multiple selection
 */
void ScTabViewShell::AddRefEntry()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                pRefDlg->AddRefEntry();
            }
        }
    }
    else
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
        if (pHdl)
            pHdl->AddRefEntry();
    }
}

void ScTabViewShell::EndReference()
{
    //TODO: Move reference dialog handling to view
    //      Just keep function autopilot here for references to other documents

    // We also annul the ZoomIn again in RefDialogs

    //FIXME: ShowRefFrame at InputHdl, if the Function AutoPilot is open?
    if ( m_nCurRefDlgId )
    {
        SfxChildWindow* pChildWnd = lcl_GetChildWinFromAnyView( m_nCurRefDlgId );
        OSL_ENSURE( pChildWnd, "NoChildWin" );
        if ( pChildWnd )
        {
            IAnyRefDialog* pRefDlg = dynamic_cast<IAnyRefDialog*>(pChildWnd->GetWindow());
            assert(pRefDlg);
            if(pRefDlg)
            {
                pRefDlg->SetActive();
            }
        }
    }
}

void ScTabViewShell::RegisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    std::vector<VclPtr<vcl::Window> > & rlRefWindow = m_mapRefWindow[nSlotId];

    if( std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd ) == rlRefWindow.end() )
    {
        rlRefWindow.emplace_back(pWnd );
    }

}

void  ScTabViewShell::UnregisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd )
{
    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return;

    std::vector<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    auto i = std::find( rlRefWindow.begin(), rlRefWindow.end(), pWnd );

    if( i == rlRefWindow.end() )
        return;

    rlRefWindow.erase( i );

    if( rlRefWindow.empty() )
        m_mapRefWindow.erase( nSlotId );
}

vcl::Window *  ScTabViewShell::Find1RefWindow( sal_uInt16 nSlotId, vcl::Window *pWndAncestor )
{
    if (!pWndAncestor)
        return nullptr;

    auto iSlot = m_mapRefWindow.find( nSlotId );

    if( iSlot == m_mapRefWindow.end() )
        return nullptr;

    std::vector<VclPtr<vcl::Window> > & rlRefWindow = iSlot->second;

    while( vcl::Window *pParent = pWndAncestor->GetParent() ) pWndAncestor = pParent;

    for (auto const& refWindow : rlRefWindow)
        if ( pWndAncestor->IsWindowOrChild( refWindow, refWindow->IsSystemWindow() ) )
            return refWindow;

    return nullptr;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
