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

#include <memory>
#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <svl/itemiter.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <svl/itempool.hxx>
#include <itemdel.hxx>

#include <comphelper/processfactory.hxx>

#include <svl/hint.hxx>

#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <rtl/strbuf.hxx>


using namespace ::com::sun::star;

struct SfxRequest_Impl: public SfxListener

/*  [Description]

    Implementation structure of the <SfxRequest> class.
*/

{
    SfxRequest*     pAnti;       // Owner because of dying pool
    OUString        aTarget;     // if possible from target object set by App
    SfxItemPool*    pPool;       // ItemSet build with this pool
    std::unique_ptr<SfxPoolItem> pRetVal; // Return value belongs to itself
    SfxShell*       pShell;      // run from this shell
    const SfxSlot*  pSlot;       // executed Slot
    sal_uInt16      nModifier;   // which Modifier was pressed?
    bool            bDone;       // at all executed
    bool            bIgnored;    // Cancelled by the User
    bool            bCancelled;  // no longer notify
    SfxCallMode     nCallMode;   // Synch/Asynch/API/Record
    bool            bAllowRecording;
    std::unique_ptr<SfxAllItemSet>
                    pInternalArgs;
    SfxViewFrame*   pViewFrame;

    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;
    css::uno::Reference< uno::XComponentContext > xContext;

    explicit SfxRequest_Impl( SfxRequest *pOwner )
        : pAnti( pOwner)
        , pPool(nullptr)
        , pShell(nullptr)
        , pSlot(nullptr)
        , nModifier(0)
        , bDone(false)
        , bIgnored(false)
        , bCancelled(false)
        , nCallMode( SfxCallMode::SYNCHRON )
        , bAllowRecording( false )
        , pViewFrame(nullptr)
        , xContext(comphelper::getProcessComponentContext())
    {
    }

    void                SetPool( SfxItemPool *pNewPool );
    virtual void        Notify( SfxBroadcaster &rBC, const SfxHint &rHint ) override;
    void                Record( const uno::Sequence < beans::PropertyValue >& rArgs );
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
        pPool = pNewPool;
        if ( pNewPool )
            StartListening( pNewPool->BC() );
    }
}


SfxRequest::~SfxRequest()
{
    // Leave out Done() marked requests with 'rem'
    if ( pImpl->xRecorder.is() && !pImpl->bDone && !pImpl->bIgnored )
        pImpl->Record( uno::Sequence < beans::PropertyValue >() );

    // Clear object
    pArgs.reset();
    if ( pImpl->pRetVal )
        DeleteItemOnIdle(std::move(pImpl->pRetVal));
}


SfxRequest::SfxRequest
(
    const SfxRequest& rOrig
)
:   SfxHint( rOrig ),
    nSlot(rOrig.nSlot),
    pArgs(rOrig.pArgs? new SfxAllItemSet(*rOrig.pArgs): nullptr),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bAllowRecording = rOrig.pImpl->bAllowRecording;
    pImpl->bDone = false;
    pImpl->bIgnored = false;
    pImpl->pShell = nullptr;
    pImpl->pSlot = nullptr;
    pImpl->nCallMode = rOrig.pImpl->nCallMode;
    pImpl->aTarget = rOrig.pImpl->aTarget;
    pImpl->nModifier = rOrig.pImpl->nModifier;

    // deep copy needed !
    pImpl->pInternalArgs.reset( rOrig.pImpl->pInternalArgs ? new SfxAllItemSet(*rOrig.pImpl->pInternalArgs) : nullptr);

    if ( pArgs )
        pImpl->SetPool( pArgs->GetPool() );
    else
        pImpl->SetPool( rOrig.pImpl->pPool );

    // setup macro recording if it was in the original SfxRequest
    if (!rOrig.pImpl->pViewFrame || !rOrig.pImpl->xRecorder.is())
        return;

    nSlot = rOrig.nSlot;
    pImpl->pViewFrame = rOrig.pImpl->pViewFrame;
    if (pImpl->pViewFrame->GetDispatcher()->GetShellAndSlot_Impl(nSlot, &pImpl->pShell, &pImpl->pSlot, true, true))
    {
        pImpl->SetPool( &pImpl->pShell->GetPool() );
        pImpl->xRecorder = SfxRequest::GetMacroRecorder(pImpl->pViewFrame);
        pImpl->aTarget = pImpl->pShell->GetName();
    }
    else
    {
        SAL_WARN("sfx", "Recording unsupported slot: " << pImpl->pPool->GetSlotId(nSlot));
    }
}


SfxRequest::SfxRequest
(
    SfxViewFrame*   pViewFrame,
    sal_uInt16          nSlotId

)

/*  [Description]

    With this constructor events can subsequently be recorded that are not run
    across SfxDispatcher (eg from KeyInput() or mouse events). For this, a
    SfxRequest instance is created by this constructor and then proceed
    exactly as with a SfxRequest that in a <Slot-Execute-Method> is given as a
    parameter.
*/

:   nSlot(nSlotId),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->bIgnored = false;
    pImpl->SetPool( &pViewFrame->GetPool() );
    pImpl->pShell = nullptr;
    pImpl->pSlot = nullptr;
    pImpl->nCallMode = SfxCallMode::SYNCHRON;
    pImpl->pViewFrame = pViewFrame;
    if( pImpl->pViewFrame->GetDispatcher()->GetShellAndSlot_Impl( nSlotId, &pImpl->pShell, &pImpl->pSlot, true, true ) )
    {
        pImpl->SetPool( &pImpl->pShell->GetPool() );
        pImpl->xRecorder = SfxRequest::GetMacroRecorder( pViewFrame );
        pImpl->aTarget = pImpl->pShell->GetName();
    }
    else
    {
        SAL_WARN( "sfx", "Recording unsupported slot: " << pImpl->pPool->GetSlotId(nSlotId) );
    }
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
    pImpl->bIgnored = false;
    pImpl->SetPool( &rPool );
    pImpl->pShell = nullptr;
    pImpl->pSlot = nullptr;
    pImpl->nCallMode = nMode;
}

SfxRequest::SfxRequest
(
    const SfxSlot* pSlot,       // executed <Slot-Id>
    const css::uno::Sequence < css::beans::PropertyValue >& rArgs,
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&   rPool        // necessary for the SfxItemSet for parameters
)
:   nSlot(pSlot->GetSlotId()),
    pArgs(new SfxAllItemSet(rPool)),
    pImpl( new SfxRequest_Impl(this) )
{
    pImpl->bDone = false;
    pImpl->bIgnored = false;
    pImpl->SetPool( &rPool );
    pImpl->pShell = nullptr;
    pImpl->pSlot = nullptr;
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
    pImpl->bIgnored = false;
    pImpl->SetPool( rSfxArgs.GetPool() );
    pImpl->pShell = nullptr;
    pImpl->pSlot = nullptr;
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


void SfxRequest_Impl::Record
(
    const uno::Sequence < beans::PropertyValue >& rArgs  // current Parameter
)

/*  [Description]

    Internal helper method to create a repeatable description of the just
    executed SfxRequest.
*/

{
    OUString aCommand(".uno:");
    aCommand += OUString( pSlot->GetUnoName(), strlen( pSlot->GetUnoName() ), RTL_TEXTENCODING_UTF8 );
    OUString aCmd( aCommand );
    if(!xRecorder.is())
        return;

    uno::Reference< container::XIndexReplace > xReplace( xRecorder, uno::UNO_QUERY );
    if ( xReplace.is() && aCmd == ".uno:InsertText" )
    {
        sal_Int32 nCount = xReplace->getCount();
        if ( nCount )
        {
            frame::DispatchStatement aStatement;
            uno::Any aElement = xReplace->getByIndex(nCount-1);
            if ( (aElement >>= aStatement) && aStatement.aCommand == aCmd )
            {
                OUString aStr;
                OUString aNew;
                aStatement.aArgs[0].Value >>= aStr;
                rArgs[0].Value >>= aNew;
                aStr += aNew;
                aStatement.aArgs[0].Value <<= aStr;
                aElement <<= aStatement;
                xReplace->replaceByIndex( nCount-1, aElement );
                return;
            }
        }
    }

    uno::Reference< util::XURLTransformer > xTransform = util::URLTransformer::create( xContext );

    css::util::URL aURL;
    aURL.Complete = aCmd;
    xTransform->parseStrict(aURL);

    if (bDone)
        xRecorder->recordDispatch(aURL,rArgs);
    else
        xRecorder->recordDispatchAsComment(aURL,rArgs);
}


void SfxRequest::Record_Impl
(
    SfxShell&       rSh,    // the <SfxShell>, which has executed the Request
    const SfxSlot&  rSlot,  // the <SfxSlot>, which has executed the Request
    const css::uno::Reference< css::frame::XDispatchRecorder >& xRecorder,
    SfxViewFrame* pViewFrame
)

/*  [Description]

    This internal method marks the specified SfxMakro SfxRequest as recorded in
    SfxMakro. Pointer to the parameters in Done() is used again, thus has to
    still be alive.
*/

{
    pImpl->pShell = &rSh;
    pImpl->pSlot = &rSlot;
    pImpl->xRecorder = xRecorder;
    pImpl->aTarget = rSh.GetName();
    pImpl->pViewFrame = pViewFrame;
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
    pArgs->Put(rItem, rItem.Which());
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
    DBG_ASSERT(!pImpl->pRetVal, "Set Return value multiple times?");
    pImpl->pRetVal.reset(rItem.Clone());
}


const SfxPoolItem* SfxRequest::GetReturnValue() const
{
    return pImpl->pRetVal.get();
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

    Any return values are passed only when 'Done()' was called. Similar, when
    recording a macro only true statements are generated if 'Done()' was
    called; for SfxRequests that were not identified as such will instead
    be commented out by inserting ('rem').

    [Note]

    'Done ()' is not called, for example when a dialog started by the function
    was canceled by the user or if the execution could not be performed due to
    a wrong context (without use of separate <SfxShell>s). 'Done ()' will be
    launched, when executing the function led to a regular error
    (for example, file could not be opened).
*/

{
    Done_Impl( &rSet );

    // Keep items if possible, so they can be queried by StarDraw.
    if ( !pArgs )
    {
        pArgs.reset( new SfxAllItemSet( rSet ) );
        pImpl->SetPool( pArgs->GetPool() );
    }
    else
    {
        SfxItemIter aIter(rSet);
        const SfxPoolItem* pItem = aIter.FirstItem();
        while(pItem)
        {
            if(!IsInvalidItem(pItem))
                pArgs->Put(*pItem,pItem->Which());
            pItem = aIter.NextItem();
        }
    }
}


void SfxRequest::Done( bool bRelease )
//  [<SfxRequest::Done(SfxItemSet&)>]
{
    Done_Impl( pArgs.get() );
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


void SfxRequest::Ignore()

/*  [Description]

    If this method is called instead of <SfxRequest::Done()>, then this
    request is not recorded.

    [Example]

    The selecting of tools in StarDraw should not be recorded, but the same
    slots are to be used from the generation of the tools to the generated
    objects. Thus can NoRecords not be specified, i.e. should not be recorded.
*/

{
    // Mark as actually executed
    pImpl->bIgnored = true;
}


void SfxRequest::Done_Impl
(
    const SfxItemSet*   pSet    /* parameters passed on by the application,
                                   that for example were asked for by the user
                                   in a dialogue, 0 if no parameters have been
                                   set */

)

/*  [Description]

    Internal method to mark SfxRequest with 'done' and to evaluate the
    parameters in 'pSet' in case it is recorded.
*/

{
    // Mark as actually executed
    pImpl->bDone = true;

    // not Recording
    if ( !pImpl->xRecorder.is() )
        return;

    // was running a different slot than requested (Delegation)
    if ( nSlot != pImpl->pSlot->GetSlotId() )
    {
        // Search Slot again
        pImpl->pSlot = pImpl->pShell->GetInterface()->GetSlot(nSlot);
        DBG_ASSERT( pImpl->pSlot, "delegated SlotId not found" );
        if ( !pImpl->pSlot ) // playing it safe
            return;
    }

    // recordable?
    // new Recording uses UnoName!
    SAL_WARN_IF( !pImpl->pSlot->pUnoName, "sfx", "Recording not exported slot: "
                    << pImpl->pSlot->GetSlotId() );

    if ( !pImpl->pSlot->pUnoName ) // playing it safe
        return;

    // often required values
    SfxItemPool &rPool = pImpl->pShell->GetPool();

    // Property-Slot?
    if ( !pImpl->pSlot->IsMode(SfxSlotMode::METHOD) )
    {
        // get the property as SfxPoolItem
        const SfxPoolItem *pItem;
        sal_uInt16 nWhich = rPool.GetWhich(pImpl->pSlot->GetSlotId());
        SfxItemState eState = pSet ? pSet->GetItemState( nWhich, false, &pItem ) : SfxItemState::UNKNOWN;
        SAL_WARN_IF( SfxItemState::SET != eState, "sfx", "Recording property not available: "
                     << pImpl->pSlot->GetSlotId() );
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( eState == SfxItemState::SET )
            TransformItems( pImpl->pSlot->GetSlotId(), *pSet, aSeq, pImpl->pSlot );
        pImpl->Record( aSeq );
    }

    // record everything in a single statement?
    else if ( pImpl->pSlot->IsMode(SfxSlotMode::RECORDPERSET) )
    {
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( pSet )
            TransformItems( pImpl->pSlot->GetSlotId(), *pSet, aSeq, pImpl->pSlot );
        pImpl->Record( aSeq );
    }

    // record each item as a single statement
    else if ( pImpl->pSlot->IsMode(SfxSlotMode::RECORDPERITEM) )
    {
        if ( pSet )
        {
            // iterate over Items
            SfxItemIter aIter(*pSet);
            for ( const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem() )
            {
                // to determine the slot ID for the individual item
                sal_uInt16 nSlotId = rPool.GetSlotId( pItem->Which() );
                if ( nSlotId == nSlot )
                {
                    // play it safe; repair the wrong flags
                    OSL_FAIL( "recursion RecordPerItem - use RecordPerSet!" );
                    SfxSlot *pSlot = const_cast<SfxSlot*>(pImpl->pSlot);
                    pSlot->nFlags &= ~SfxSlotMode::RECORDPERITEM;
                    pSlot->nFlags &=  SfxSlotMode::RECORDPERSET;
                }

                // Record a Sub-Request
                SfxRequest aReq( pImpl->pViewFrame, nSlotId );
                if ( aReq.pImpl->pSlot )
                    aReq.AppendItem( *pItem );
                aReq.Done();
            }
        }
        else
        {
          //HACK(think about this again)
            pImpl->Record( uno::Sequence < beans::PropertyValue >() );
        }
    }
}


bool SfxRequest::IsDone() const

/*  [Description]

    With this method it can be queried whether the SfxRequest was actually
    executed or not. If a SfxRequest was not executed, then this is for example
    because it was canceled by the user or the context for this request was
    wrong, this was not implemented on a separate <SfxShell>.

    SfxRequest instances that return false will not be recorded.

    [Cross-reference]

    <SfxRequest::Done(const SfxItemSet&)>
    <SfxRequest::Done()>
*/

{
    return pImpl->bDone;
}


css::uno::Reference< css::frame::XDispatchRecorder > SfxRequest::GetMacroRecorder( SfxViewFrame const * pView )

/*  [Description]

    This recorder is an attempt for dispatch () to get calls from the Frame.
    This is then available through a property by a supplier but only when
    recording was turned on.

    (See also SfxViewFrame::MiscExec_Impl() and SID_RECORDING)
*/

{
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder;

    css::uno::Reference< css::beans::XPropertySet > xSet(
        (pView ? pView : SfxViewFrame::Current())->GetFrame().GetFrameInterface(),
        css::uno::UNO_QUERY);

    if(xSet.is())
    {
        css::uno::Any aProp = xSet->getPropertyValue("DispatchRecorderSupplier");
        css::uno::Reference< css::frame::XDispatchRecorderSupplier > xSupplier;
        aProp >>= xSupplier;
        if(xSupplier.is())
            xRecorder = xSupplier->getDispatchRecorder();
    }

    return xRecorder;
}

bool SfxRequest::HasMacroRecorder( SfxViewFrame const * pView )
{
    return GetMacroRecorder( pView ).is();
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


void SfxRequest::AllowRecording( bool bSet )
{
    pImpl->bAllowRecording = bSet;
}

bool SfxRequest::AllowsRecording() const
{
    bool bAllow = pImpl->bAllowRecording;
    if( !bAllow )
        bAllow = ( SfxCallMode::API != ( SfxCallMode::API & pImpl->nCallMode ) ) &&
                 ( SfxCallMode::RECORD == ( SfxCallMode::RECORD & pImpl->nCallMode ) );
    return bAllow;
}

void SfxRequest::ReleaseArgs()
{
    pArgs.reset();
    pImpl->pInternalArgs.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
