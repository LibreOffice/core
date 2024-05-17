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

#include <LayerTabBar.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <helpids.h>
#include <app.hrc>
#include <strings.hrc>

#include <DrawViewShell.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <DrawDocShell.hxx>
#include <drawview.hxx>
#include <undolayer.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <svx/svditer.hxx>
#include <sdpage.hxx>
#include <svx/sdrpaintwindow.hxx>

namespace sd {

/**
 * default constructor
 */
LayerTabBar::LayerTabBar(DrawViewShell* pViewSh, vcl::Window* pParent)
    : TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL ) ),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh),
    m_aBringLayerObjectsToAttentionDelayTimer("LayerTabBar m_aBringLayerObjectsToAttentionDelayTimer")
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
    SetHelpId( HID_SD_TABBAR_LAYERS );

    m_aBringLayerObjectsToAttentionDelayTimer.SetInvokeHandler(
                LINK(this, LayerTabBar, BringLayerObjectsToAttentionDelayTimerHdl));
    m_aBringLayerObjectsToAttentionDelayTimer.SetTimeout(500);
}

LayerTabBar::~LayerTabBar()
{
    disposeOnce();
}

void LayerTabBar::dispose()
{
    DropTargetHelper::dispose();
    TabBar::dispose();
}

OUString LayerTabBar::convertToLocalizedName(const OUString& rName)
{
    if ( rName == sUNO_LayerName_background )
        return SdResId( STR_LAYER_BCKGRND );

    if ( rName == sUNO_LayerName_background_objects )
        return SdResId( STR_LAYER_BCKGRNDOBJ );

    if ( rName == sUNO_LayerName_layout )
        return SdResId( STR_LAYER_LAYOUT );

    if ( rName == sUNO_LayerName_controls )
        return SdResId( STR_LAYER_CONTROLS );

    if ( rName == sUNO_LayerName_measurelines )
        return SdResId( STR_LAYER_MEASURELINES );

    return rName;
}

// Use a method name, that is specific to LayerTabBar to make code better readable
OUString LayerTabBar::GetLayerName(sal_uInt16 nPageId) const
{
    return GetAuxiliaryText(nPageId);
}

void LayerTabBar::SetLayerName( sal_uInt16 nPageId, const OUString& rText )
{
    SetAuxiliaryText(nPageId, rText);
}

// Here "Page" is a tab in the LayerTabBar.
void LayerTabBar::InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                TabBarPageBits nBits, sal_uInt16 nPos)
{
    OUString sLocalizedName(convertToLocalizedName(rText));
    TabBar::InsertPage(nPageId, sLocalizedName, nBits, nPos );
    SetLayerName(nPageId, rText);
}

void LayerTabBar::SetPageText( sal_uInt16 nPageId, const OUString& rText )
{
    OUString sLocalizedName(convertToLocalizedName(rText));
    SetLayerName(nPageId, rText);
    TabBar::SetPageText(nPageId, sLocalizedName);
}

bool LayerTabBar::IsLocalizedNameOfStandardLayer(std::u16string_view rName)
{
    return (   rName == SdResId(STR_LAYER_LAYOUT)
            || rName == SdResId(STR_LAYER_CONTROLS)
            || rName == SdResId(STR_LAYER_MEASURELINES)
            || rName == SdResId(STR_LAYER_BCKGRND)
            || rName == SdResId(STR_LAYER_BCKGRNDOBJ) );
}

bool LayerTabBar::IsRealNameOfStandardLayer(std::u16string_view rName)
{
    return (   rName == sUNO_LayerName_layout
            || rName == sUNO_LayerName_controls
            || rName == sUNO_LayerName_measurelines
            || rName == sUNO_LayerName_background
            || rName == sUNO_LayerName_background_objects );
}

void LayerTabBar::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHLAYER, SfxCallMode::SYNCHRON);
}

void LayerTabBar::MouseMove(const MouseEvent &rMEvt)
{
    sal_uInt16 nPageId = 0;
    if (!rMEvt.IsLeaveWindow())
        nPageId = GetPageId(rMEvt.GetPosPixel());
    BringLayerObjectsToAttention(nPageId);
    return;
}

void LayerTabBar::BringLayerObjectsToAttention(const sal_uInt16 nPageId)
{
    if (nPageId == m_nBringLayerObjectsToAttentionLastPageId)
        return;

    m_aBringLayerObjectsToAttentionDelayTimer.Stop();

    m_nBringLayerObjectsToAttentionLastPageId = nPageId;

    std::vector<basegfx::B2DRange> aRanges;

    if (nPageId != 0)
    {
        OUString aLayerName(GetLayerName(nPageId));
        if (pDrViewSh->GetView()->GetSdrPageView()->IsLayerVisible(aLayerName))
        {
            SdrLayerAdmin& rLayerAdmin = pDrViewSh->GetDoc()->GetLayerAdmin();
            SdrObjListIter aIter(pDrViewSh->GetActualPage(), SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                SdrObject* pObj = aIter.Next();
                assert(pObj != nullptr);
                if (!pObj)
                    continue;
                const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID(pObj->GetLayer());
                if (!pSdrLayer)
                    continue;
                if (aLayerName == pSdrLayer->GetName())
                {
                    ::tools::Rectangle aRect(pObj->GetLogicRect());
                    if (!aRect.IsEmpty())
                        aRanges.emplace_back(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
                    // skip over objects in groups
                    if (pObj->IsGroupObject())
                    {
                        SdrObjListIter aSubListIter(pObj->GetSubList(), SdrIterMode::DeepWithGroups);
                        while (aSubListIter.IsMore())
                        {
                            aIter.Next();
                            aSubListIter.Next();
                        }
                    }
                }
            }
        }
    }

    if (m_xOverlayObject && m_xOverlayObject->getOverlayManager())
        m_xOverlayObject->getOverlayManager()->remove(*m_xOverlayObject);
    if (aRanges.empty())
        m_xOverlayObject.reset();
    else
    {
        m_xOverlayObject.reset(new sdr::overlay::OverlaySelection(
                                   sdr::overlay::OverlayType::Invert,
                                   Color(), std::move(aRanges), true/*unused for Invert type*/));
        m_aBringLayerObjectsToAttentionDelayTimer.Start();
    }
}

IMPL_LINK_NOARG(LayerTabBar, BringLayerObjectsToAttentionDelayTimerHdl, Timer *, void)
{
    m_aBringLayerObjectsToAttentionDelayTimer.Stop();
    if (m_xOverlayObject)
    {
        if (SdrView* pView = pDrViewSh->GetDrawView())
        {
            if (SdrPaintWindow* pPaintWindow = pView->GetPaintWindow(0))
            {
                const rtl::Reference<sdr::overlay::OverlayManager>& xOverlayManager =
                        pPaintWindow->GetOverlayManager();
                xOverlayManager->add(*m_xOverlayObject);
            }
        }
    }
}

void LayerTabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bSetPageID=false;

    if (rMEvt.IsLeft())
    {
        Point aPosPixel = rMEvt.GetPosPixel();
        sal_uInt16 aTabId = GetPageId( PixelToLogic(aPosPixel) );
        if (aTabId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_INSERTLAYER, SfxCallMode::SYNCHRON);

            bSetPageID=true;
        }
        else if (rMEvt.IsMod2())
        {
            // direct editing of tab text
            // make sure the clicked tab is the current tab otherwise Edit() acts on the wrong tab
            if ( aTabId != GetCurPageId())
            {
                MouseEvent aSyntheticEvent (rMEvt.GetPosPixel(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0);
                TabBar::MouseButtonDown(aSyntheticEvent);
            }
        }
        else if (rMEvt.IsMod1() || rMEvt.IsShift())
        {
            // keyboard Shortcuts to change layer attributes

            OUString aName(GetLayerName(aTabId));
            SdrPageView* pPV = pDrViewSh->GetView()->GetSdrPageView();

            // Save old state

            bool bOldPrintable = pPV->IsLayerPrintable(aName);
            bool bOldVisible = pPV->IsLayerVisible(aName);
            bool bOldLocked = pPV->IsLayerLocked(aName);

            bool bNewPrintable = bOldPrintable;
            bool bNewVisible = bOldVisible;
            bool bNewLocked = bOldLocked;

            if (rMEvt.IsMod1() && rMEvt.IsShift())
            {
                // Shift+Ctrl: Toggle between layer printable / not printable
                bNewPrintable = !bOldPrintable;
                pPV->SetLayerPrintable(aName, bNewPrintable);
            }
            else if (rMEvt.IsShift())
            {
                // Shift: Toggle between layer visible / hidden
                // see also SID_TOGGLELAYERVISIBILITY / tdf#113439
                bNewVisible = !bOldVisible;
                pPV->SetLayerVisible(aName, bNewVisible);
            }
            else // if (rMEvt.IsMod1())
            {
                // Ctrl: Toggle between layer locked / unlocked
                bNewLocked = !bOldLocked;
                pPV->SetLayerLocked(aName, bNewLocked);
            }

            pDrViewSh->ResetActualLayer();

            // Add Undo action

            ::sd::View* pView = pDrViewSh->GetView();
            DrawView* pDrView = dynamic_cast<DrawView*>(pView);

            SdDrawDocument& rDoc = pView->GetDoc();
            SdrLayer* pLayer = rDoc.GetLayerAdmin().GetLayer(aName);

            if (pLayer)
            {
                assert (pDrView && "Change layer attribute undo action is only working with a SdDrawView");
                if(pDrView)
                {
                    SfxUndoManager* pManager = rDoc.GetDocSh()->GetUndoManager();
                    std::unique_ptr<SdLayerModifyUndoAction> pAction(new SdLayerModifyUndoAction(
                        &rDoc,
                        pLayer,
                        aName,
                        pLayer->GetTitle(),
                        pLayer->GetDescription(),
                        bOldVisible,
                        bOldLocked,
                        bOldPrintable,
                        aName,
                        pLayer->GetTitle(),
                        pLayer->GetDescription(),
                        bNewVisible,
                        bNewLocked,
                        bNewPrintable
                        ));
                    pManager->AddUndoAction(std::move(pAction));
                }
            }

            // Mark document changed

            pView->GetDoc().SetChanged();
        }
    }

    // If you insert a new layer you must not call TabBar::MouseButtonDown(rMEvt);
    // because you want to activate the new layer
    if( !bSetPageID )
        TabBar::MouseButtonDown(rMEvt);
}

void LayerTabBar::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYLAYER, SfxCallMode::SYNCHRON );
    }
}

/**
 * AcceptDrop-Event
 */

sal_Int8 LayerTabBar::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving )
        EndSwitchPage();

    if( !pDrViewSh->GetDocSh()->IsReadOnly() )
    {
        Point         aPos( PixelToLogic( rEvt.maPosPixel ) );
        OUString      sLayerName( GetLayerName(GetPageId(aPos)) );
        SdrLayerID    nLayerId = pDrViewSh->GetView()->GetDoc().GetLayerAdmin().GetLayerID(sLayerName);

        nRet = pDrViewSh->AcceptDrop( rEvt, *this, nullptr, SDRPAGE_NOTFOUND, nLayerId );

        SwitchPage( aPos );
    }

    return nRet;
}

/**
 * ExecuteDrop-Event
 */
sal_Int8 LayerTabBar::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    Point         aPos( PixelToLogic(rEvt.maPosPixel) );
    OUString      sLayerName( GetLayerName(GetPageId(aPos)) );
    SdrLayerID    nLayerId = pDrViewSh->GetView()->GetDoc().GetLayerAdmin().GetLayerID(sLayerName);

    sal_Int8        nRet = pDrViewSh->ExecuteDrop( rEvt, *this, nullptr, SDRPAGE_NOTFOUND, nLayerId );

    EndSwitchPage();

    return nRet;

}

void  LayerTabBar::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        BringLayerObjectsToAttention();
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup(u"layertab"_ustr);
    }
}

bool LayerTabBar::StartRenaming()
{
    bool bOK = true;
    OUString aLayerName = GetLayerName( GetEditPageId() );

    if ( IsRealNameOfStandardLayer(aLayerName))
    {
        // It is not allowed to change these names
        bOK = false;
    }
    else
    {
        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
        {
            pView->SdrEndTextEdit();
        }
    }

    return bOK;
}

TabBarAllowRenamingReturnCode LayerTabBar::AllowRenaming()
{
    bool bOK = true;

    // Check if names already exists
    ::sd::View* pView = pDrViewSh->GetView();
    SdDrawDocument& rDoc = pView->GetDoc();
    OUString aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
    OUString aNewName( GetEditText() );

    if (aNewName.isEmpty() ||
        (rLayerAdmin.GetLayer( aNewName ) && aLayerName != aNewName) )
    {
        // Name already exists.
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pDrViewSh->GetViewFrame()->GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
        xWarn->run();
        bOK = false;
    }

    if (bOK)
    {
        if ( IsLocalizedNameOfStandardLayer(aNewName) || IsRealNameOfStandardLayer(aNewName) )
        {
            // Standard layer names may not be changed.
            bOK = false;
        }
    }

    return bOK ? TABBAR_RENAMING_YES : TABBAR_RENAMING_NO;
}

void LayerTabBar::EndRenaming()
{
    if( IsEditModeCanceled() )
        return;

    ::sd::View* pView = pDrViewSh->GetView();
    DrawView* pDrView = dynamic_cast<DrawView*>( pView  );

    SdDrawDocument& rDoc = pView->GetDoc();
    OUString aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = rDoc.GetLayerAdmin();
    SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName);

    if (!pLayer)
        return;

    OUString aNewName( GetEditText() );
    assert (pDrView && "Rename layer undo action is only working with a SdDrawView");
    if( pDrView )
    {
        SfxUndoManager* pManager = rDoc.GetDocSh()->GetUndoManager();
        std::unique_ptr<SdLayerModifyUndoAction> pAction(new SdLayerModifyUndoAction(
            &rDoc,
            pLayer,
            aLayerName,
            pLayer->GetTitle(),
            pLayer->GetDescription(),
            pDrView->IsLayerVisible(aLayerName),
            pDrView->IsLayerLocked(aLayerName),
            pDrView->IsLayerPrintable(aLayerName),
            aNewName,
            pLayer->GetTitle(),
            pLayer->GetDescription(),
            pDrView->IsLayerVisible(aLayerName),
            pDrView->IsLayerLocked(aLayerName),
            pDrView->IsLayerPrintable(aLayerName)
            ));
        pManager->AddUndoAction( std::move(pAction) );
    }

    // First notify View since SetName() calls ResetActualLayer() and
    // the View then already has to know the Layer
    pView->SetActiveLayer(aNewName);
    pLayer->SetName(aNewName);
    rDoc.SetChanged();
}

void LayerTabBar::ActivatePage()
{
    if (pDrViewSh!=nullptr)
    {

        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHLAYER, SfxCallMode::ASYNCHRON);
    }
}

void LayerTabBar::SendActivatePageEvent()
{
    CallEventListeners (VclEventId::TabbarPageActivated,
        reinterpret_cast<void*>(GetCurPageId()));
}

void LayerTabBar::SendDeactivatePageEvent()
{
    CallEventListeners (VclEventId::TabbarPageDeactivated,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
