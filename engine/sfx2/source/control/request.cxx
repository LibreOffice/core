/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cpo/uno/Sequence.hxx>
#include <svl/itemiter.hxx>
#include <tools/debug.hxx>

#include <svl/itempool.hxx>

#include <svl/hint.hxx>

#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxuno.hxx>


using namespace ::com::sun::star;
using namespace ::cpo;

struct SfxRequest_Impl: public SfxListener

/*  [Description]

    Implementation structure of the <SfxRequest> class.
*/

{
    SfxRequest*     pAnti;       // Owner because of dying pool
    SfxItemPool*    pPool;       // ItemSet build with this pool
    SfxPoolItemHolder aRetVal; // Return value belongs to itself
    sal_uInt16      nModifier;   // which Modifier was pressed?
    bool            bDone;       // at all executed
    bool            bCancelled;  // no longer notify
    SfxCallMode     nCallMode;   // Synch/Asynch/API/Record
    std::unique_ptr<SfxAllItemSet>
                    pInternalArgs;
    int m_nKitViewId = -1;

    explicit SfxRequest_Impl( SfxRequest *pOwner )
        : pAnti( pOwner)
        , pPool(nullptr)
        , nModifier(0)
        , bDone(false)
        , bCancelled(false)
        , nCallMode( SfxCallMode::SYNCHRON )
    {
    }

    void                SetPool( SfxItemPool *pNewPool );
    virtual void        Notify( SfxBroadcaster &rBC, const SfxHint &rHint ) override;
};


void SfxRequest_Impl::Notify( SfxBroadcaster&, const SfxHint &rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        pAnti->Cancel();
}


void SfxRequest_Impl::SetPool( SfxItemPool *pNewPool )
{
    if ( pNewPool != pPool )
    {
        if ( pPool )
            EndListening( pPool->BC() );

        // tdf#159719 reset SfxPoolItemHolder
        aRetVal = SfxPoolItemHolder();

        pPool = pNewPool;

        if ( pNewPool )
            StartListening( pNewPool->BC() );
    }
}


SfxRequest::~SfxRequest()
{
    // Clear object
    pArgs.reset();
}


SfxRequest::SfxRequest
(
    const SfxRequest& rOrig
)
:   nSlot(rOrig.nSlot),
    pArgs(rOrig.pArgs? new SfxAllItemSet(*rOrig.pArgs): nullptr),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->nCallMode = rOrig.pImpl->nCallMode;
    pImpl->nModifier = rOrig.pImpl->nModifier;

    // deep copy needed !
    pImpl->pInternalArgs.reset( rOrig.pImpl->pInternalArgs ? new SfxAllItemSet(*rOrig.pImpl->pInternalArgs) : nullptr);

    if ( pArgs )
        pImpl->SetPool( pArgs->GetPool() );
    else
        pImpl->SetPool( rOrig.pImpl->pPool );
}


SfxRequest::SfxRequest(SfxViewFrame& rViewFrame, sal_uInt16 nSlotId)
    : nSlot(nSlotId)
    , pImpl(new SfxRequest_Impl(this))
{
    pImpl->bDone = false;
    pImpl->SetPool( &rViewFrame.GetPool() );
    pImpl->nCallMode = SfxCallMode::SYNCHRON;
    SfxShell* pShell = nullptr;
    const SfxSlot* pSlot = nullptr;
    if( rViewFrame.GetDispatcher()->GetShellAndSlot_Impl( nSlotId, &pShell, &pSlot, true, true ) )
        pImpl->SetPool( &pShell->GetPool() );
}


SfxRequest::SfxRequest
(
    sal_uInt16        nSlotId,  // executed <Slot-Id>
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&  rPool     // necessary for the SfxItemSet for parameters
)

// creates a SfxRequest without arguments

:   nSlot(nSlotId),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->SetPool( &rPool );
    pImpl->nCallMode = nMode;
}

SfxRequest::SfxRequest
(
    const SfxSlot* pSlot,       // executed <Slot-Id>
    const cpo::uno::Sequence < css::beans::PropertyValue >& rArgs,
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&   rPool        // necessary for the SfxItemSet for parameters
)
:   nSlot(pSlot->GetSlotId()),
    pArgs(new SfxAllItemSet(rPool)),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->SetPool( &rPool );
    pImpl->nCallMode = nMode;
    TransformParameters( nSlot, rArgs, *pArgs, pSlot );
}


SfxRequest::SfxRequest
(
    sal_uInt16                  nSlotId,
    SfxCallMode                 nMode,
    const SfxAllItemSet&        rSfxArgs
)

// creates a SfxRequest with arguments

:   nSlot(nSlotId),
    pArgs(new SfxAllItemSet(rSfxArgs)),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->SetPool( rSfxArgs.GetPool() );
    pImpl->nCallMode = nMode;
}


SfxRequest::SfxRequest
(
    sal_uInt16                  nSlotId,
    SfxCallMode                 nMode,
    const SfxAllItemSet&        rSfxArgs,
    const SfxAllItemSet&        rSfxInternalArgs
)
: SfxRequest(nSlotId, nMode, rSfxArgs)
{
    SetInternalArgs_Impl(rSfxInternalArgs);
}

SfxCallMode SfxRequest::GetCallMode() const
{
    return pImpl->nCallMode;
}


bool SfxRequest::IsSynchronCall() const
{
    return SfxCallMode::SYNCHRON == ( SfxCallMode::SYNCHRON & pImpl->nCallMode );
}


void SfxRequest::SetSynchronCall( bool bSynchron )
{
    if ( bSynchron )
        pImpl->nCallMode |= SfxCallMode::SYNCHRON;
    else
        pImpl->nCallMode &= ~SfxCallMode::SYNCHRON;
}

void SfxRequest::SetInternalArgs_Impl( const SfxAllItemSet& rArgs )
{
    pImpl->pInternalArgs.reset( new SfxAllItemSet( rArgs ) );
}

const SfxItemSet* SfxRequest::GetInternalArgs_Impl() const
{
    return pImpl->pInternalArgs.get();
}


void SfxRequest::SetArgs( const SfxAllItemSet& rArgs )
{
    pArgs.reset(new SfxAllItemSet(rArgs));
    pImpl->SetPool( pArgs->GetPool() );
}


void SfxRequest::AppendItem(const SfxPoolItem &rItem)
{
    if(!pArgs)
        pArgs.reset( new SfxAllItemSet(*pImpl->pPool) );
    pArgs->Put(rItem);
}


void SfxRequest::RemoveItem( sal_uInt16 nID )
{
    if (pArgs)
    {
        pArgs->ClearItem(nID);
        if ( !pArgs->Count() )
            pArgs.reset();
    }
}

void SfxRequest::SetReturnValue(const SfxPoolItem &rItem)
{
    DBG_ASSERT(nullptr != pImpl->pPool, "Missing SfxItemPool (!)");
    if (nullptr != pImpl->pPool)
        pImpl->aRetVal = SfxPoolItemHolder(*pImpl->pPool, &rItem);
}


const SfxPoolItemHolder& SfxRequest::GetReturnValue() const
{
    return pImpl->aRetVal;
}


void SfxRequest::Done
(
    const SfxItemSet&   rSet    /* parameters passed on by the application,
                                   that for example were asked for by the user
                                   in a dialogue, 0 if no parameters have been
                                   set */
)

/*  [Description]

    This method must be called in the <Execute-Method> of the <SfxSlot>s, which
    has performed the SfxRequest when the execution actually took place. If
    'Done()' is not called, then the SfxRequest is considered canceled.

    Any return values are passed only when 'Done()' was called.

    [Note]

    'Done ()' is not called, for example when a dialog started by the function
    was canceled by the user or if the execution could not be performed due to
    a wrong context (without use of separate <SfxShell>s). 'Done ()' will be
    launched, when executing the function led to a regular error
    (for example, file could not be opened).
*/

{
    pImpl->bDone = true;

    // Keep items if possible, so they can be queried by StarDraw.
    if ( !pArgs )
    {
        pArgs.reset( new SfxAllItemSet( rSet ) );
        pImpl->SetPool( pArgs->GetPool() );
    }
    else
    {
        for (SfxItemIter aIter(rSet); !aIter.IsAtEnd(); aIter.Next())
        {
            const SfxPoolItem* pItem = aIter.GetCurItem();
            if(!IsInvalidItem(pItem))
                pArgs->Put(*pItem);
        }
    }
}


void SfxRequest::Done( bool bRelease )
//  [<SfxRequest::Done(SfxItemSet&)>]
{
    pImpl->bDone = true;
    if( bRelease )
        pArgs.reset();
}


void SfxRequest::ForgetAllArgs()
{
    pArgs.reset();
    pImpl->pInternalArgs.reset();
}


bool SfxRequest::IsCancelled() const
{
    return pImpl->bCancelled;
}


void SfxRequest::Cancel()

/*  [Description]

    Marks this request as no longer executable. For example, if called when
    the target (more precisely, its pool) dies.
*/

{
    pImpl->bCancelled = true;
    pImpl->SetPool( nullptr );
    pArgs.reset();
}


bool SfxRequest::IsDone() const

/*  [Description]

    With this method it can be queried whether the SfxRequest was actually
    executed or not. If a SfxRequest was not executed, then this is for example
    because it was canceled by the user or the context for this request was
    wrong, this was not implemented on a separate <SfxShell>.

    [Cross-reference]

    <SfxRequest::Done(const SfxItemSet&)>
    <SfxRequest::Done()>
*/

{
    return pImpl->bDone;
}


bool SfxRequest::IsAPI() const

/*  [Description]

    Returns true if this SfxRequest was generated by an API (for example BASIC),
    otherwise false.
*/

{
    return SfxCallMode::API == ( SfxCallMode::API & pImpl->nCallMode );
}


void SfxRequest::SetModifier( sal_uInt16 nModi )
{
    pImpl->nModifier = nModi;
}


sal_uInt16 SfxRequest::GetModifier() const
{
    return pImpl->nModifier;
}


void SfxRequest::ReleaseArgs()
{
    pArgs.reset();
    pImpl->pInternalArgs.reset();
}

void SfxRequest::SetKitViewId(int nId)
{
    pImpl->m_nKitViewId = nId;
}

int SfxRequest::GetKitViewId() const
{
    return pImpl->m_nKitViewId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
