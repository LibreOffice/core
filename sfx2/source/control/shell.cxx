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

#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/VerbAttributes.hpp>
#include <basic/sbstar.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <svl/itempool.hxx>
#include <svl/undo.hxx>
#include <itemdel.hxx>
#include <svtools/asynclink.hxx>
#include <basic/sbx.hxx>
#include <unotools/configmgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfxtypes.hxx>
#include <sfx2/request.hxx>
#include <statcach.hxx>
#include <sfx2/msgpool.hxx>
#include <sidebar/ContextChangeBroadcaster.hxx>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <memory>
#include <vector>
#include <map>

using namespace com::sun::star;

struct SfxShell_Impl: public SfxBroadcaster
{
    OUString                    aObjectName;   // Name of Sbx-Objects
    // Maps the Which() field to a pointer to a SfxPoolItem
    std::map<sal_uInt16, std::unique_ptr<SfxPoolItem>>
                                m_Items;       // Data exchange on Item level
    SfxViewShell*               pViewSh;       // SfxViewShell if Shell is
                                               // ViewFrame/ViewShell/SubShell list
    SfxViewFrame*               pFrame;        // Frame, if  <UI-active>
    SfxRepeatTarget*            pRepeatTarget; // SbxObjectRef xParent;
    bool                        bActive;
    SfxDisableFlags             nDisableFlags;
    std::unique_ptr<svtools::AsynchronLink> pExecuter;
    std::unique_ptr<svtools::AsynchronLink> pUpdater;
    std::vector<std::unique_ptr<SfxSlot> >  aSlotArr;

    css::uno::Sequence < css::embed::VerbDescriptor > aVerbList;
    ::sfx2::sidebar::ContextChangeBroadcaster maContextChangeBroadcaster;

    SfxShell_Impl()
        : pViewSh(nullptr)
        , pFrame(nullptr)
        , pRepeatTarget(nullptr)
        , bActive(false)
        , nDisableFlags(SfxDisableFlags::NONE)
        , pExecuter(nullptr)
        , pUpdater(nullptr)
    {
    }

    virtual ~SfxShell_Impl() override { pExecuter.reset(); pUpdater.reset();}
};


void SfxShell::EmptyExecStub(SfxShell *, SfxRequest &)
{
}

void SfxShell::EmptyStateStub(SfxShell *, SfxItemSet &)
{
}

SfxShell::SfxShell()
:   pImpl(new SfxShell_Impl),
    pPool(nullptr),
    pUndoMgr(nullptr)
{
}

SfxShell::SfxShell( SfxViewShell *pViewSh )
:   pImpl(new SfxShell_Impl),
    pPool(nullptr),
    pUndoMgr(nullptr)
{
    pImpl->pViewSh = pViewSh;
}

SfxShell::~SfxShell()
{
}

void SfxShell::SetName( const OUString &rName )
{
    pImpl->aObjectName = rName;
}

const OUString& SfxShell::GetName() const
{
    return pImpl->aObjectName;
}

SfxDispatcher* SfxShell::GetDispatcher() const
{
    return pImpl->pFrame ? pImpl->pFrame->GetDispatcher() : nullptr;
}

SfxViewShell* SfxShell::GetViewShell() const
{
    return pImpl->pViewSh;
}

SfxViewFrame* SfxShell::GetFrame() const
{
    if ( pImpl->pFrame )
        return pImpl->pFrame;
    if ( pImpl->pViewSh )
        return pImpl->pViewSh->GetViewFrame();
    return nullptr;
}

const SfxPoolItem* SfxShell::GetItem
(
    sal_uInt16  nSlotId         // Slot-Id of the querying <SfxPoolItem>s
)   const
{
    auto const it = pImpl->m_Items.find( nSlotId );
    if (it != pImpl->m_Items.end())
        return it->second.get();
    return nullptr;
}

void SfxShell::PutItem
(
    const SfxPoolItem&  rItem  /* Instance, of which a copy is created,
                                  which is stored in the SfxShell in a list. */
)
{
    DBG_ASSERT( dynamic_cast< const SfxSetItem* >( &rItem) ==  nullptr, "SetItems aren't allowed here" );
    DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                "items with Which-Ids aren't allowed here" );

    // MSC made a mess here of WNT/W95, beware of changes
    SfxPoolItem *pItem = rItem.Clone();
    SfxPoolItemHint aItemHint( pItem );
    sal_uInt16 nWhich = rItem.Which();

    auto const it = pImpl->m_Items.find(nWhich);
    if (it != pImpl->m_Items.end())
    {
        // Replace Item
        pImpl->m_Items.erase( it );
        pImpl->m_Items.insert(std::make_pair(nWhich, std::unique_ptr<SfxPoolItem>(pItem)));

        // if active, notify Bindings
        SfxDispatcher *pDispat = GetDispatcher();
        if ( pDispat )
        {
            SfxBindings* pBindings = pDispat->GetBindings();
            pBindings->Broadcast( aItemHint );
            sal_uInt16 nSlotId = nWhich; //pItem->GetSlotId();
            SfxStateCache* pCache = pBindings->GetStateCache( nSlotId );
            if ( pCache )
            {
                pCache->SetState( SfxItemState::DEFAULT, pItem, true );
                pCache->SetCachedState( true );
            }
        }
        return;
    }
    else
    {
        Broadcast( aItemHint );
        pImpl->m_Items.insert(std::make_pair(nWhich, std::unique_ptr<SfxPoolItem>(pItem)));
    }
}

SfxInterface* SfxShell::GetInterface() const
{
    return GetStaticInterface();
}

SfxUndoManager* SfxShell::GetUndoManager()
{
    return pUndoMgr;
}

void SfxShell::SetUndoManager( SfxUndoManager *pNewUndoMgr )
{
    OSL_ENSURE( ( pUndoMgr == nullptr ) || ( pNewUndoMgr == nullptr ) || ( pUndoMgr == pNewUndoMgr ),
        "SfxShell::SetUndoManager: exchanging one non-NULL manager with another non-NULL manager? Suspicious!" );
    // there's at least one client of our UndoManager - the DocumentUndoManager at the SfxBaseModel - which
    // caches the UndoManager, and registers itself as listener. If exchanging non-NULL UndoManagers is really
    // a supported scenario (/me thinks it is not), then we would need to notify all such clients instances.

    pUndoMgr = pNewUndoMgr;
    if (pUndoMgr && !utl::ConfigManager::IsFuzzing())
    {
        pUndoMgr->SetMaxUndoActionCount(
            officecfg::Office::Common::Undo::Steps::get());
    }
}

SfxRepeatTarget* SfxShell::GetRepeatTarget() const
{
    return pImpl->pRepeatTarget;
}

void SfxShell::SetRepeatTarget( SfxRepeatTarget *pTarget )
{
    pImpl->pRepeatTarget = pTarget;
}

void SfxShell::Invalidate
(
    sal_uInt16          nId     /* Invalidated Slot-Id or Which-Id.
                               If these are 0 (default), then all
                               by this Shell currently handled Slot-Ids are
                               invalidated. */
)
{
    if ( !GetViewShell() )
    {
        OSL_FAIL( "wrong Invalidate method called!" );
        return;
    }

    Invalidate_Impl( GetViewShell()->GetViewFrame()->GetBindings(), nId );
}

void SfxShell::Invalidate_Impl( SfxBindings& rBindings, sal_uInt16 nId )
{
    if ( nId == 0 )
    {
        rBindings.InvalidateShell( *this );
    }
    else
    {
        const SfxInterface *pIF = GetInterface();
        do
        {
            const SfxSlot *pSlot = pIF->GetSlot(nId);
            if ( pSlot )
            {
                // Invalidate the Slot itself
                rBindings.Invalidate( pSlot->GetSlotId() );
                return;
            }

            pIF = pIF->GetGenoType();
        }

        while ( pIF );

        SAL_INFO( "sfx.control", "W3: invalidating slot-id unknown in shell" );
    }
}

void SfxShell::HandleOpenXmlFilterSettings(SfxRequest & rReq)
{
    try
    {
        uno::Reference < ui::dialogs::XExecutableDialog > xDialog = ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
        xDialog->execute();
    }
    catch (const uno::Exception&)
    {
    }
    rReq.Ignore ();
}

void SfxShell::DoActivate_Impl( SfxViewFrame *pFrame, bool bMDI )
{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
    SAL_INFO(
        "sfx.control",
        "SfxShell::DoActivate() " << this << "  " << GetInterface()->GetClassName()
            << " bMDI " << (bMDI ? "MDI" : ""));

    if ( bMDI )
    {
        // Remember Frame, in which it was activated
        pImpl->pFrame = pFrame;
        pImpl->bActive = true;
    }

    // Notify Subclass
    Activate(bMDI);
}

void SfxShell::DoDeactivate_Impl( SfxViewFrame const *pFrame, bool bMDI )
{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
    SAL_INFO(
        "sfx.control",
        "SfxShell::DoDeactivate()" << this << "  " << GetInterface()->GetClassName()
            << " bMDI " << (bMDI ? "MDI" : ""));

    // Only when it comes from a Frame
    // (not when for instance by popping BASIC-IDE from AppDisp)
    if ( bMDI && pImpl->pFrame == pFrame )
    {
        // deliver
        pImpl->pFrame = nullptr;
        pImpl->bActive = false;
    }

    // Notify Subclass
    Deactivate(bMDI);
}

bool SfxShell::IsActive() const
{
    return pImpl->bActive;
}

void SfxShell::Activate
(
    bool    /*bMDI*/        /*  TRUE
                            the <SfxDispatcher>, on which the SfxShell is
                            located, is activated or the SfxShell instance
                            was pushed on an active SfxDispatcher.
                            (compare with SystemWindow::IsMDIActivate())

                            FALSE
                            the <SfxViewFrame>, on which SfxDispatcher
                            the SfxShell instance is located, was
                            activated. (for example by a closing dialog) */
)
{
    BroadcastContextForActivation(true);
}

void SfxShell::Deactivate
(
    bool    /*bMDI*/        /*  TRUE
                            the <SfxDispatcher>, on which the SfxShell is
                            located, is inactivated or the SfxShell instance
                            was popped on an active SfxDispatcher.
                            (compare with SystemWindow::IsMDIActivate())

                            FALSE
                            the <SfxViewFrame>, on which SfxDispatcher
                            the SfxShell instance is located, was
                            deactivated. (for example by a dialog) */
)
{
    BroadcastContextForActivation(false);
}

bool SfxShell::CanExecuteSlot_Impl( const SfxSlot &rSlot )
{
    // Get Slot status
    SfxItemPool &rPool = GetPool();
    const sal_uInt16 nId = rSlot.GetWhich( rPool );
    SfxItemSet aSet(rPool, {{nId, nId}});
    SfxStateFunc pFunc = rSlot.GetStateFnc();
    CallState( pFunc, aSet );
    return aSet.GetItemState(nId) != SfxItemState::DISABLED;
}

bool SfxShell::IsConditionalFastCall( const SfxRequest &rReq )
{
    sal_uInt16 nId = rReq.GetSlot();
    bool bRet = false;

    if (nId == SID_UNDO || nId == SID_REDO)
    {
        const SfxItemSet* pArgs = rReq.GetArgs();
        if (pArgs && pArgs->HasItem(SID_REPAIRPACKAGE))
            bRet = true;
    }
    return bRet;
}


void ShellCall_Impl( void* pObj, void* pArg )
{
    static_cast<SfxShell*>(pObj)->ExecuteSlot( *static_cast<SfxRequest*>(pArg) );
}

void SfxShell::ExecuteSlot( SfxRequest& rReq, bool bAsync )
{
    if( !bAsync )
        ExecuteSlot( rReq );
    else
    {
        if( !pImpl->pExecuter )
            pImpl->pExecuter.reset( new svtools::AsynchronLink(
                Link<void*,void>( this, ShellCall_Impl ) ) );
        pImpl->pExecuter->Call( new SfxRequest( rReq ) );
    }
}

const SfxPoolItem* SfxShell::ExecuteSlot
(
    SfxRequest          &rReq,  // the relayed <SfxRequest>
    const SfxInterface* pIF     // default = 0 means get virtually
)
{
    if ( !pIF )
        pIF = GetInterface();

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxSlot* pSlot = nullptr;
    if ( nSlot >= SID_VERB_START && nSlot <= SID_VERB_END )
        pSlot = GetVerbSlot_Impl(nSlot);
    if ( !pSlot )
        pSlot = pIF->GetSlot(nSlot);
    DBG_ASSERT( pSlot, "slot not supported" );

    SfxExecFunc pFunc = pSlot->GetExecFnc();
    if ( pFunc )
        CallExec( pFunc, rReq );

    return rReq.GetReturnValue();
}

const SfxPoolItem* SfxShell::GetSlotState
(
    sal_uInt16              nSlotId,    // Slot-Id to the Slots in question
    const SfxInterface* pIF,        // default = 0 means get virtually
    SfxItemSet*         pStateSet   // SfxItemSet of the Slot-State method
)
{
    // Get Slot on the given Interface
    if ( !pIF )
        pIF = GetInterface();
    SfxItemState eState = SfxItemState::UNKNOWN;
    SfxItemPool &rPool = GetPool();

    const SfxSlot* pSlot = nullptr;
    if ( nSlotId >= SID_VERB_START && nSlotId <= SID_VERB_END )
        pSlot = GetVerbSlot_Impl(nSlotId);
    if ( !pSlot )
        pSlot = pIF->GetSlot(nSlotId);
    if ( pSlot )
        // Map on Which-Id if possible
        nSlotId = pSlot->GetWhich( rPool );

    // Get Item and Item status
    const SfxPoolItem *pItem = nullptr;
    SfxItemSet aSet( rPool, {{nSlotId, nSlotId}} ); // else pItem dies too soon
    if ( pSlot )
    {
        // Call Status method
        SfxStateFunc pFunc = pSlot->GetStateFnc();
        if ( pFunc )
            CallState( pFunc, aSet );
        eState = aSet.GetItemState( nSlotId, true, &pItem );

        // get default Item if possible
        if ( eState == SfxItemState::DEFAULT )
        {
            if ( SfxItemPool::IsWhich(nSlotId) )
                pItem = &rPool.GetDefaultItem(nSlotId);
            else
                eState = SfxItemState::DONTCARE;
        }
    }
    else
        eState = SfxItemState::UNKNOWN;

    // Evaluate Item and item status and possibly maintain them in pStateSet
    SfxPoolItem *pRetItem = nullptr;
    if ( eState <= SfxItemState::DISABLED )
    {
        if ( pStateSet )
            pStateSet->DisableItem(nSlotId);
        return nullptr;
    }
    else if ( eState == SfxItemState::DONTCARE )
    {
        if ( pStateSet )
            pStateSet->ClearItem(nSlotId);
        pRetItem = new SfxVoidItem(0);
    }
    else
    {
        if ( pStateSet && pStateSet->Put( *pItem ) )
            return &pStateSet->Get( pItem->Which() );
        pRetItem = pItem->Clone();
    }
    DeleteItemOnIdle(pRetItem);

    return pRetItem;
}

SFX_EXEC_STUB(SfxShell, VerbExec)
void SfxStubSfxShellVerbState(SfxShell *, SfxItemSet& rSet)
{
    SfxShell::VerbState( rSet );
}

void SfxShell::SetVerbs(const css::uno::Sequence < css::embed::VerbDescriptor >& aVerbs)
{
    SfxViewShell *pViewSh = dynamic_cast<SfxViewShell*>( this );

    DBG_ASSERT(pViewSh, "Only call SetVerbs at the ViewShell!");
    if ( !pViewSh )
        return;

    // First make all Statecaches dirty, so that no-one no longer tries to use
    // the Slots
    {
        SfxBindings *pBindings =
            pViewSh->GetViewFrame()->GetDispatcher()->GetBindings();
        sal_uInt16 nCount = pImpl->aSlotArr.size();
        for (sal_uInt16 n1=0; n1<nCount ; n1++)
        {
            sal_uInt16 nId = SID_VERB_START + n1;
            pBindings->Invalidate(nId, false, true);
        }
    }

    sal_uInt16 nr=0;
    for (sal_Int32 n=0; n<aVerbs.getLength(); n++)
    {
        sal_uInt16 nSlotId = SID_VERB_START + nr++;
        DBG_ASSERT(nSlotId <= SID_VERB_END, "Too many Verbs!");
        if (nSlotId > SID_VERB_END)
            break;

        SfxSlot *pNewSlot = new SfxSlot;
        pNewSlot->nSlotId = nSlotId;
        pNewSlot->nGroupId = SfxGroupId::NONE;

        // Verb slots must be executed asynchronously, so that they can be
        // destroyed while executing.
        pNewSlot->nFlags = SfxSlotMode::ASYNCHRON | SfxSlotMode::CONTAINER;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxShell,VerbExec);
        pNewSlot->fnState = SFX_STUB_PTR(SfxShell,VerbState);
        pNewSlot->pType = nullptr; // HACK(SFX_TYPE(SfxVoidItem)) ???
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = nullptr;
        pNewSlot->pUnoName = nullptr;

        if (!pImpl->aSlotArr.empty())
        {
            SfxSlot& rSlot = *pImpl->aSlotArr[0].get();
            pNewSlot->pNextSlot = rSlot.pNextSlot;
            rSlot.pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        pImpl->aSlotArr.insert(pImpl->aSlotArr.begin() + static_cast<sal_uInt16>(n), std::unique_ptr<SfxSlot>(pNewSlot));
    }

    pImpl->aVerbList = aVerbs;

    if (pViewSh)
    {
        // The status of SID_OBJECT is collected in the controller directly on
        // the Shell, it is thus enough to encourage a new status update
        SfxBindings *pBindings = pViewSh->GetViewFrame()->GetDispatcher()->
                GetBindings();
        pBindings->Invalidate( SID_OBJECT, true, true );
    }
}

const css::uno::Sequence < css::embed::VerbDescriptor >& SfxShell::GetVerbs() const
{
    return pImpl->aVerbList;
}

void SfxShell::VerbExec(SfxRequest& rReq)
{
    sal_uInt16 nId = rReq.GetSlot();
    SfxViewShell *pViewShell = GetViewShell();
    if ( pViewShell )
    {
        bool bReadOnly = pViewShell->GetObjectShell()->IsReadOnly();
        css::uno::Sequence < css::embed::VerbDescriptor > aList = pViewShell->GetVerbs();
        for (sal_Int32 n=0, nVerb=0; n<aList.getLength(); n++)
        {
            // check for ReadOnly verbs
            if ( bReadOnly && !(aList[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES) )
                continue;

            // check for verbs that shouldn't appear in the menu
            if ( !(aList[n].VerbAttributes & embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU) )
                continue;

            if (nId == SID_VERB_START + nVerb++)
            {
                pViewShell->DoVerb(aList[n].VerbID);
                rReq.Done();
                return;
            }
        }
    }
}

void SfxShell::VerbState(SfxItemSet& )
{
}

const SfxSlot* SfxShell::GetVerbSlot_Impl(sal_uInt16 nId) const
{
    css::uno::Sequence < css::embed::VerbDescriptor > rList = pImpl->aVerbList;

    DBG_ASSERT(nId >= SID_VERB_START && nId <= SID_VERB_END,"Wrong VerbId!");
    sal_uInt16 nIndex = nId - SID_VERB_START;
    DBG_ASSERT(nIndex < rList.getLength(),"Wrong VerbId!");

    if (nIndex < rList.getLength())
        return pImpl->aSlotArr[nIndex].get();
    else
        return nullptr;
}

SfxObjectShell* SfxShell::GetObjectShell()
{
    if ( GetViewShell() )
        return GetViewShell()->GetViewFrame()->GetObjectShell();
    else
        return nullptr;
}

bool SfxShell::HasUIFeature(SfxShellFeature) const
{
    return false;
}

void DispatcherUpdate_Impl( void*, void* pArg )
{
    static_cast<SfxDispatcher*>(pArg)->Update_Impl( true );
    static_cast<SfxDispatcher*>(pArg)->GetBindings()->InvalidateAll(false);
}

void SfxShell::UIFeatureChanged()
{
    SfxViewFrame *pFrame = GetFrame();
    if ( pFrame && pFrame->IsVisible() )
    {
        // Also force an update, if dispatcher is already updated otherwise
        // something my get stuck in the bunkered tools. Asynchronous call to
        // prevent recursion.
        if ( !pImpl->pUpdater )
            pImpl->pUpdater.reset( new svtools::AsynchronLink( Link<void*,void>( this, DispatcherUpdate_Impl ) ) );

        // Multiple views allowed
        pImpl->pUpdater->Call( pFrame->GetDispatcher(), true );
    }
}

void SfxShell::SetDisableFlags( SfxDisableFlags nFlags )
{
    pImpl->nDisableFlags = nFlags;
}

SfxDisableFlags SfxShell::GetDisableFlags() const
{
    return pImpl->nDisableFlags;
}

std::unique_ptr<SfxItemSet> SfxShell::CreateItemSet( sal_uInt16 )
{
    return nullptr;
}

void SfxShell::ApplyItemSet( sal_uInt16, const SfxItemSet& )
{
}

void SfxShell::SetContextName (const ::rtl::OUString& rsContextName)
{
    pImpl->maContextChangeBroadcaster.Initialize(rsContextName);
}

void SfxShell::SetViewShell_Impl( SfxViewShell* pView )
{
    pImpl->pViewSh = pView;
}

void SfxShell::BroadcastContextForActivation (const bool bIsActivated)
{
    SfxViewFrame* pViewFrame = GetFrame();
    if (pViewFrame != nullptr)
    {
        if (bIsActivated)
            pImpl->maContextChangeBroadcaster.Activate(pViewFrame->GetFrame().GetFrameInterface());
        else
            pImpl->maContextChangeBroadcaster.Deactivate(pViewFrame->GetFrame().GetFrameInterface());
   }
}

bool SfxShell::SetContextBroadcasterEnabled (const bool bIsEnabled)
{
    return pImpl->maContextChangeBroadcaster.SetBroadcasterEnabled(bIsEnabled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
