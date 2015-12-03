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
#include "itemdel.hxx"
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
#include "sfxtypes.hxx"
#include <sfx2/request.hxx>
#include <sfx2/mnumgr.hxx>
#include "statcach.hxx"
#include <sfx2/msgpool.hxx>
#include <sidebar/ContextChangeBroadcaster.hxx>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <memory>
#include <vector>
#include <map>

// Maps the Which() field to a pointer to a SfxPoolItem
typedef std::map<sal_uInt16, std::unique_ptr<SfxPoolItem>> SfxItemPtrMap;


using namespace com::sun::star;

struct SfxShell_Impl: public SfxBroadcaster
{
    OUString                    aObjectName;   // Name of Sbx-Objects
    SfxItemPtrMap               m_Items;       // Data exchange on Item level
    SfxViewShell*               pViewSh;       // SfxViewShell if Shell is
                                            // ViewFrame/ViewShell/SubShell list
    SfxViewFrame*               pFrame;        // Frame, if  <UI-active>
    SfxRepeatTarget*            pRepeatTarget; // SbxObjectRef xParent;
    bool                        bActive;
    sal_uIntPtr                 nDisableFlags;
    sal_uIntPtr                 nHelpId;
    svtools::AsynchronLink*     pExecuter;
    svtools::AsynchronLink*     pUpdater;
    std::vector<std::unique_ptr<SfxSlot> >  aSlotArr;

    css::uno::Sequence < css::embed::VerbDescriptor > aVerbList;
    ::sfx2::sidebar::ContextChangeBroadcaster maContextChangeBroadcaster;

    SfxShell_Impl()
        : pViewSh(nullptr)
        , pFrame(nullptr)
        , pRepeatTarget(nullptr)
        , bActive(false)
        , nDisableFlags(0)
        , nHelpId(0)
        , pExecuter(nullptr)
        , pUpdater(nullptr)
    {
    }

    virtual ~SfxShell_Impl() { delete pExecuter; delete pUpdater;}
};


void SfxShell::EmptyExecStub(SfxShell *, SfxRequest &)
{
}

void SfxShell::EmptyStateStub(SfxShell *, SfxItemSet &)
{
}

SfxShell::SfxShell()
:   pImp(nullptr),
    pPool(nullptr),
    pUndoMgr(nullptr)
{
    pImp = new SfxShell_Impl;
}

SfxShell::SfxShell( SfxViewShell *pViewSh )
:   pImp(nullptr),
    pPool(nullptr),
    pUndoMgr(nullptr)
{
    pImp = new SfxShell_Impl;
    pImp->pViewSh = pViewSh;
}

SfxShell::~SfxShell()
{
    delete pImp;
}

void SfxShell::SetName( const OUString &rName )
{
    pImp->aObjectName = rName;
}

const OUString& SfxShell::GetName() const
{
    return pImp->aObjectName;
}

SfxDispatcher* SfxShell::GetDispatcher() const
{
    return pImp->pFrame ? pImp->pFrame->GetDispatcher() : nullptr;
}

SfxViewShell* SfxShell::GetViewShell() const
{
    return pImp->pViewSh;
}

SfxViewFrame* SfxShell::GetFrame() const
{
    if ( pImp->pFrame )
        return pImp->pFrame;
    if ( pImp->pViewSh )
        return pImp->pViewSh->GetViewFrame();
    return nullptr;
}

const SfxPoolItem* SfxShell::GetItem
(
    sal_uInt16  nSlotId         // Slot-Id of the querying <SfxPoolItem>s
)   const
{
    auto const it = pImp->m_Items.find( nSlotId );
    if (it != pImp->m_Items.end())
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

    auto const it = pImp->m_Items.find(nWhich);
    if (it != pImp->m_Items.end())
    {
        // Replace Item
        pImp->m_Items.erase( it );
        pImp->m_Items.insert(std::make_pair(nWhich, std::unique_ptr<SfxPoolItem>(pItem)));

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
        pImp->m_Items.insert(std::make_pair(nWhich, std::unique_ptr<SfxPoolItem>(pItem)));
    }
}

SfxInterface* SfxShell::GetInterface() const
{
    return GetStaticInterface();
}

::svl::IUndoManager* SfxShell::GetUndoManager()
{
    return pUndoMgr;
}

void SfxShell::SetUndoManager( ::svl::IUndoManager *pNewUndoMgr )
{
    OSL_ENSURE( ( pUndoMgr == nullptr ) || ( pNewUndoMgr == nullptr ) || ( pUndoMgr == pNewUndoMgr ),
        "SfxShell::SetUndoManager: exchanging one non-NULL manager with another non-NULL manager? Suspicious!" );
    // there's at least one client of our UndoManager - the DocumentUndoManager at the SfxBaseModel - which
    // caches the UndoManager, and registers itself as listener. If exchanging non-NULL UndoManagers is really
    // a supported scenario (/me thinks it is not), then we would need to notify all such clients instances.

    pUndoMgr = pNewUndoMgr;
    if (pUndoMgr && !utl::ConfigManager::IsAvoidConfig())
    {
        pUndoMgr->SetMaxUndoActionCount(
            officecfg::Office::Common::Undo::Steps::get());
    }
}

SfxRepeatTarget* SfxShell::GetRepeatTarget() const
{
    return pImp->pRepeatTarget;
}

void SfxShell::SetRepeatTarget( SfxRepeatTarget *pTarget )
{
    pImp->pRepeatTarget = pTarget;
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
                // At Enum-Slots invalidate the Master-Slot
                if ( SFX_KIND_ENUM == pSlot->GetKind() )
                    pSlot = pSlot->GetLinkedSlot();

                // Invalidate the Slot itself and possible also all Slave-Slots
                rBindings.Invalidate( pSlot->GetSlotId() );
                for ( const SfxSlot *pSlave = pSlot->GetLinkedSlot();
                      pSlave && pIF->ContainsSlot_Impl( pSlave ) &&
                        pSlave->GetLinkedSlot() == pSlot;
                      ++pSlave )
                    rBindings.Invalidate( pSlave->GetSlotId() );

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
        pImp->pFrame = pFrame;
        pImp->bActive = true;
    }

    // Notify Subclass
    Activate(bMDI);
}

void SfxShell::DoDeactivate_Impl( SfxViewFrame *pFrame, bool bMDI )
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
    // (not when for instance by poping BASIC-IDE from AppDisp)
    if ( bMDI && pImp->pFrame == pFrame )
    {
        // deliver
        pImp->pFrame = nullptr;
        pImp->bActive = false;
    }

    // Notify Subclass
    Deactivate(bMDI);
}

bool SfxShell::IsActive() const
{
    return pImp->bActive;
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
                            activated. (for example by a closing dialoge) */
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
                            deactivated. (for example by a dialoge) */
)
{
    BroadcastContextForActivation(false);
}

bool SfxShell::CanExecuteSlot_Impl( const SfxSlot &rSlot )
{
    // Get Slot status
    SfxItemPool &rPool = GetPool();
    const sal_uInt16 nId = rSlot.GetWhich( rPool );
    SfxItemSet aSet(rPool, nId, nId);
    SfxStateFunc pFunc = rSlot.GetStateFnc();
    CallState( pFunc, aSet );
    return aSet.GetItemState(nId) != SfxItemState::DISABLED;
}

void ShellCall_Impl( void* pObj, void* pArg )
{
    static_cast<SfxShell*>(pObj)->ExecuteSlot( *static_cast<SfxRequest*>(pArg) );
}

const SfxPoolItem* SfxShell::ExecuteSlot( SfxRequest& rReq, bool bAsync )
{
    if( !bAsync )
        return ExecuteSlot( rReq );
    else
    {
        if( !pImp->pExecuter )
            pImp->pExecuter = new svtools::AsynchronLink(
                Link<void*,void>( this, ShellCall_Impl ) );
        pImp->pExecuter->Call( new SfxRequest( rReq ) );
        return nullptr;
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
    SfxItemSet aSet( rPool, nSlotId, nSlotId ); // else pItem dies too soon
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
        sal_uInt16 nCount = pImp->aSlotArr.size();
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
        DBG_ASSERT(nSlotId <= SID_VERB_END, "To many Verbs!");
        if (nSlotId > SID_VERB_END)
            break;

        SfxSlot *pNewSlot = new SfxSlot;
        pNewSlot->nSlotId = nSlotId;
        pNewSlot->nGroupId = 0;

        // Verb slots must be executed asynchronously, so that they can be
        // destroyed while executing.
        pNewSlot->nFlags = SfxSlotMode::ASYNCHRON | SfxSlotMode::CONTAINER;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxShell,VerbExec);
        pNewSlot->fnState = SFX_STUB_PTR(SfxShell,VerbState);
        pNewSlot->pType = nullptr; // HACK(SFX_TYPE(SfxVoidItem)) ???
        pNewSlot->pLinkedSlot = nullptr;
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = nullptr;
        pNewSlot->pUnoName = nullptr;

        if (!pImp->aSlotArr.empty())
        {
            SfxSlot& rSlot = *pImp->aSlotArr[0].get();
            pNewSlot->pNextSlot = rSlot.pNextSlot;
            rSlot.pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        pImp->aSlotArr.insert(pImp->aSlotArr.begin() + (sal_uInt16) n, std::unique_ptr<SfxSlot>(pNewSlot));
    }

    pImp->aVerbList = aVerbs;

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
    return pImp->aVerbList;
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
    css::uno::Sequence < css::embed::VerbDescriptor > rList = pImp->aVerbList;

    DBG_ASSERT(nId >= SID_VERB_START && nId <= SID_VERB_END,"Wrong VerbId!");
    sal_uInt16 nIndex = nId - SID_VERB_START;
    DBG_ASSERT(nIndex < rList.getLength(),"Wrong VerbId!");

    if (nIndex < rList.getLength())
        return pImp->aSlotArr[nIndex].get();
    else
        return nullptr;
}

void SfxShell::SetHelpId(sal_uIntPtr nId)
{
    pImp->nHelpId = nId;
}

sal_uIntPtr SfxShell::GetHelpId() const
{
    return pImp->nHelpId;
}

SfxObjectShell* SfxShell::GetObjectShell()
{
    if ( GetViewShell() )
        return GetViewShell()->GetViewFrame()->GetObjectShell();
    else
        return nullptr;
}

bool SfxShell::HasUIFeature( sal_uInt32 )
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
        if ( !pImp->pUpdater )
            pImp->pUpdater = new svtools::AsynchronLink( Link<void*,void>( this, DispatcherUpdate_Impl ) );

        // Multiple views allowed
        pImp->pUpdater->Call( pFrame->GetDispatcher(), true );
    }
}

void SfxShell::SetDisableFlags( sal_uIntPtr nFlags )
{
    pImp->nDisableFlags = nFlags;
}

sal_uIntPtr SfxShell::GetDisableFlags() const
{
    return pImp->nDisableFlags;
}

SfxItemSet* SfxShell::CreateItemSet( sal_uInt16 )
{
    return nullptr;
}

void SfxShell::ApplyItemSet( sal_uInt16, const SfxItemSet& )
{
}

void SfxShell::SetContextName (const ::rtl::OUString& rsContextName)
{
    pImp->maContextChangeBroadcaster.Initialize(rsContextName);
}

void SfxShell::SetViewShell_Impl( SfxViewShell* pView )
{
    pImp->pViewSh = pView;
}

void SfxShell::BroadcastContextForActivation (const bool bIsActivated)
{
    SfxViewFrame* pViewFrame = GetFrame();
    if (pViewFrame != nullptr)
    {
        if (bIsActivated)
            pImp->maContextChangeBroadcaster.Activate(pViewFrame->GetFrame().GetFrameInterface());
        else
            pImp->maContextChangeBroadcaster.Deactivate(pViewFrame->GetFrame().GetFrameInterface());
   }
}

bool SfxShell::SetContextBroadcasterEnabled (const bool bIsEnabled)
{
    return pImp->maContextChangeBroadcaster.SetBroadcasterEnabled(bIsEnabled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
