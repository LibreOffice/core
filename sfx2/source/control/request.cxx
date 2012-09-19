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

#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <svl/itemiter.hxx>

#include <svl/itempool.hxx>
#include <svtools/itemdel.hxx>

#include <comphelper/processfactory.hxx>

#include <svl/smplhint.hxx>

#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/appuno.hxx>
#include <rtl/strbuf.hxx>

//===================================================================

using namespace ::com::sun::star;

struct SfxRequest_Impl: public SfxListener

/*  [Description]

    Implementation structur of the <SfxRequest> class.
*/

{
    SfxRequest*     pAnti;       // Owner because of dying pool
    String          aTarget;     // if possible from target object set by App
    SfxItemPool*    pPool;       // ItemSet build with this pool
    SfxPoolItem*    pRetVal;     // Return value belongs to itself
    SfxShell*       pShell;      // run from this shell
    const SfxSlot*  pSlot;       // executed Slot
    sal_uInt16          nModifier;   // which Modifier was pressed?
    sal_Bool            bDone;       // at all executed
    sal_Bool            bIgnored;    // Cancelled by the User
    sal_Bool            bCancelled;  // no longer notify
    sal_Bool            bUseTarget;  // aTarget was set by Application
    sal_uInt16              nCallMode;   // Synch/Asynch/API/Record
    sal_Bool                bAllowRecording;
    SfxAllItemSet*      pInternalArgs;
    SfxViewFrame*       pViewFrame;

    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;

                        SfxRequest_Impl( SfxRequest *pOwner )
                        : pAnti( pOwner)
                        , pPool(0)
                        , nModifier(0)
                        , bCancelled(sal_False)
                        , nCallMode( SFX_CALLMODE_SYNCHRON )
                        , bAllowRecording( sal_False )
                        , pInternalArgs( 0 )
                        , pViewFrame(0)
                        {}
    ~SfxRequest_Impl() { delete pInternalArgs; }


    void                SetPool( SfxItemPool *pNewPool );
    virtual void        Notify( SfxBroadcaster &rBC, const SfxHint &rHint );
    void                Record( const uno::Sequence < beans::PropertyValue >& rArgs );
};


//====================================================================

void SfxRequest_Impl::Notify( SfxBroadcaster&, const SfxHint &rHint )
{
    SfxSimpleHint *pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        pAnti->Cancel();
}

//====================================================================

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

//====================================================================


SfxRequest::~SfxRequest()
{
    // Leave out Done() marked requests with 'rem'
    if ( pImp->xRecorder.is() && !pImp->bDone && !pImp->bIgnored )
        pImp->Record( uno::Sequence < beans::PropertyValue >() );

    // Clear object
    delete pArgs;
    if ( pImp->pRetVal )
        DeleteItemOnIdle(pImp->pRetVal);
    delete pImp;
}
//--------------------------------------------------------------------


SfxRequest::SfxRequest
(
    const SfxRequest& rOrig
)
:   SfxHint( rOrig ),
    nSlot(rOrig.nSlot),
    pArgs(rOrig.pArgs? new SfxAllItemSet(*rOrig.pArgs): 0),
    pImp( new SfxRequest_Impl(this) )
{
    pImp->bAllowRecording = rOrig.pImp->bAllowRecording;
    pImp->bDone = sal_False;
    pImp->bIgnored = sal_False;
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = rOrig.pImp->nCallMode;
    pImp->bUseTarget = rOrig.pImp->bUseTarget;
    pImp->aTarget = rOrig.pImp->aTarget;
    pImp->nModifier = rOrig.pImp->nModifier;

    // deep copy needed !
    pImp->pInternalArgs = (rOrig.pImp->pInternalArgs ? new SfxAllItemSet(*rOrig.pImp->pInternalArgs) : 0);

    if ( pArgs )
        pImp->SetPool( pArgs->GetPool() );
    else
        pImp->SetPool( rOrig.pImp->pPool );
}
//--------------------------------------------------------------------


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
    pArgs(0),
    pImp( new SfxRequest_Impl(this) )
{
    pImp->bDone = sal_False;
    pImp->bIgnored = sal_False;
    pImp->SetPool( &pViewFrame->GetPool() );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = SFX_CALLMODE_SYNCHRON;
    pImp->bUseTarget = sal_False;
    pImp->pViewFrame = pViewFrame;
    if( pImp->pViewFrame->GetDispatcher()->GetShellAndSlot_Impl( nSlotId, &pImp->pShell, &pImp->pSlot, sal_True, sal_True ) )
    {
        pImp->SetPool( &pImp->pShell->GetPool() );
        pImp->xRecorder = SfxRequest::GetMacroRecorder( pViewFrame );
        pImp->aTarget = pImp->pShell->GetName();
    }
#ifdef DBG_UTIL
    else
    {
        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
            "Recording unsupported slot: "));
        aStr.append(static_cast<sal_Int32>(pImp->pPool->GetSlotId(nSlotId)));
        OSL_FAIL(aStr.getStr());
    }
#endif
}

//--------------------------------------------------------------------


SfxRequest::SfxRequest
(
    sal_uInt16        nSlotId,  // executed <Slot-Id>
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&  rPool     // necessary for the SfxItemSet for parameters
)

// creates a SfxRequest without arguments

:   nSlot(nSlotId),
    pArgs(0),
    pImp( new SfxRequest_Impl(this) )
{
    pImp->bDone = sal_False;
    pImp->bIgnored = sal_False;
    pImp->SetPool( &rPool );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = sal_False;
}

SfxRequest::SfxRequest
(
    const SfxSlot* pSlot,       // executed <Slot-Id>
    const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rArgs,
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&   rPool        // necessary for the SfxItemSet for parameters
)
:   nSlot(pSlot->GetSlotId()),
    pArgs(new SfxAllItemSet(rPool)),
    pImp( new SfxRequest_Impl(this) )
{
    pImp->bDone = sal_False;
    pImp->bIgnored = sal_False;
    pImp->SetPool( &rPool );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = sal_False;
    TransformParameters( nSlot, rArgs, *pArgs, pSlot );
}

//-----------------------------------------------------------------------

SfxRequest::SfxRequest
(
    sal_uInt16                  nSlotId,
    sal_uInt16                  nMode,
    const SfxAllItemSet&    rSfxArgs
)

// creates a SfxRequest with arguments

:   nSlot(nSlotId),
    pArgs(new SfxAllItemSet(rSfxArgs)),
    pImp( new SfxRequest_Impl(this) )
{
    pImp->bDone = sal_False;
    pImp->bIgnored = sal_False;
    pImp->SetPool( rSfxArgs.GetPool() );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = sal_False;
}
//--------------------------------------------------------------------

sal_uInt16 SfxRequest::GetCallMode() const
{
    return pImp->nCallMode;
}

//--------------------------------------------------------------------

sal_Bool SfxRequest::IsSynchronCall() const
{
    return SFX_CALLMODE_SYNCHRON == ( SFX_CALLMODE_SYNCHRON & pImp->nCallMode );
}

//--------------------------------------------------------------------

void SfxRequest::SetSynchronCall( sal_Bool bSynchron )
{
    if ( bSynchron )
        pImp->nCallMode |= SFX_CALLMODE_SYNCHRON;
    else
        pImp->nCallMode &= ~(sal_uInt16) SFX_CALLMODE_SYNCHRON;
}

void SfxRequest::SetInternalArgs_Impl( const SfxAllItemSet& rArgs )
{
    delete pImp->pInternalArgs;
    pImp->pInternalArgs = new SfxAllItemSet( rArgs );
}

const SfxItemSet* SfxRequest::GetInternalArgs_Impl() const
{
    return pImp->pInternalArgs;
}

//--------------------------------------------------------------------


void SfxRequest_Impl::Record
(
    const uno::Sequence < beans::PropertyValue >& rArgs  // current Parameter
)

/*  [Description]

    Internal helper method to create a repeatable description of the just
    executed SfxRequest.
*/

{
    String aCommand = rtl::OUString(".uno:");
    aCommand.AppendAscii( pSlot->GetUnoName() );
    ::rtl::OUString aCmd( aCommand );
    if(xRecorder.is())
    {
        uno::Reference< container::XIndexReplace > xReplace( xRecorder, uno::UNO_QUERY );
        if ( xReplace.is() && aCmd.compareToAscii(".uno:InsertText") == COMPARE_EQUAL )
        {
            sal_Int32 nCount = xReplace->getCount();
            if ( nCount )
            {
                frame::DispatchStatement aStatement;
                uno::Any aElement = xReplace->getByIndex(nCount-1);
                if ( (aElement >>= aStatement) && aStatement.aCommand == aCmd )
                {
                    ::rtl::OUString aStr;
                    ::rtl::OUString aNew;
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

        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xFactory(
                ::comphelper::getProcessServiceFactory(),
                com::sun::star::uno::UNO_QUERY);

        uno::Reference< util::XURLTransformer > xTransform( util::URLTransformer::create( ::comphelper::getComponentContext(xFactory) ) );

        com::sun::star::util::URL aURL;
        aURL.Complete = aCmd;
        xTransform->parseStrict(aURL);

        if (bDone)
            xRecorder->recordDispatch(aURL,rArgs);
        else
            xRecorder->recordDispatchAsComment(aURL,rArgs);
    }
}

//--------------------------------------------------------------------

void SfxRequest::Record_Impl
(
    SfxShell&       rSh,    // the <SfxShell>, which has excecuted the Request
    const SfxSlot&  rSlot,  // the <SfxSlot>, which has executed the Request
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder,
    SfxViewFrame* pViewFrame
)

/*  [Description]

    This internal method marks the specified SfxMakro SfxRequest as recorded in
    SfxMakro. Pointer to the parameters in Done() is used again, thus has to
    still be alive.
*/

{
    pImp->pShell = &rSh;
    pImp->pSlot = &rSlot;
    pImp->xRecorder = xRecorder;
    pImp->aTarget = rSh.GetName();
    pImp->pViewFrame = pViewFrame;
}

//--------------------------------------------------------------------

void SfxRequest::SetArgs( const SfxAllItemSet& rArgs )
{
    delete pArgs;
    pArgs = new SfxAllItemSet(rArgs);
    pImp->SetPool( pArgs->GetPool() );
}

//--------------------------------------------------------------------

void SfxRequest::AppendItem(const SfxPoolItem &rItem)
{
    if(!pArgs)
        pArgs = new SfxAllItemSet(*pImp->pPool);
    pArgs->Put(rItem, rItem.Which());
}

//--------------------------------------------------------------------

void SfxRequest::RemoveItem( sal_uInt16 nID )
{
    if (pArgs)
    {
        pArgs->ClearItem(nID);
        if ( !pArgs->Count() )
            DELETEZ(pArgs);
    }
}

//--------------------------------------------------------------------

const SfxPoolItem* SfxRequest::GetArg
(
    sal_uInt16  nSlotId,  // Slot-Id or Which-Id of the parameters
    bool    bDeep,    // sal_False: do not seach in the Parent-ItemSets
    TypeId  aType     // != 0:  RTTI check with Assertion
)   const
{
    return GetItem( pArgs, nSlotId, bDeep, aType );
}


//--------------------------------------------------------------------
const SfxPoolItem* SfxRequest::GetItem
(
    const SfxItemSet* pArgs,
    sal_uInt16            nSlotId,  // Slot-Id or Which-Id of the parameters
    bool              bDeep,    // sal_False: do not seach in the Parent-ItemSets
    TypeId            aType     // != 0:  RTTI check with Assertion
)

/*  [Description]

    With this method the access to individual parameters in the SfxRequest is
    simplified. In particular the type-examination (by Assertion) is performed,
    whereby the application source code will be much clearer. In the product-
    version is a 0 returned, if the found item is not of the specified class.

    [Example]

    void MyShell::Execute( SfxRequest &rReq )
    {
        switch ( rReq.GetSlot() )
        {
            case SID_MY:
            {
                ...
                // An Example on not using the macros
                const SfxInt32Item *pPosItem = (const SfxUInt32Item*)
                    rReq.GetArg( SID_POS, sal_False, TYPE(SfxInt32Item) );
                sal_uInt16 nPos = pPosItem ? pPosItem->GetValue() : 0;

                // An Example on using the macros
                SFX_REQUEST_ARG(rReq, pSizeItem, SfxInt32Item, SID_SIZE, sal_False);
                sal_uInt16 nSize = pSizeItem ? pPosItem->GetValue() : 0;

                ...
            }

            ...
        }
    }
*/

{
    if ( pArgs )
    {
        // Which may be converted to ID
        sal_uInt16 nWhich = pArgs->GetPool()->GetWhich(nSlotId);

        // Is the item set or available at bDeep == sal_True?
        const SfxPoolItem *pItem = 0;
        if ( ( bDeep ? SFX_ITEM_AVAILABLE : SFX_ITEM_SET )
             <= pArgs->GetItemState( nWhich, bDeep, &pItem ) )
        {
            // Compare type
            if ( !pItem || pItem->IsA(aType) )
                return pItem;

            // Item of wrong type => Programming error
            OSL_FAIL(  "invalid argument type" );
        }
    }

    // No Parameter, not found or wrong type
    return 0;
}

//--------------------------------------------------------------------

void SfxRequest::SetReturnValue(const SfxPoolItem &rItem)
{
    DBG_ASSERT(!pImp->pRetVal, "Set Return value multiple times?");
    if(pImp->pRetVal)
        delete pImp->pRetVal;
    pImp->pRetVal = rItem.Clone();
}

//--------------------------------------------------------------------

const SfxPoolItem* SfxRequest::GetReturnValue() const
{
    return pImp->pRetVal;
}

//--------------------------------------------------------------------

void SfxRequest::Done
(
    const SfxItemSet&   rSet,   /* parameters passed on by the application,
                                   that for example were asked for by the user
                                   in a dialogue, 0 if no parameters have been
                                   set */

    bool                bKeep   /*  true (default)
                                   'rSet' is saved and GetArgs() queriable.

                                    false
                                   'rSet' is not copied (faster) */
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

    'Done ()' is not called, for example when a dialoge started by the function
    was canceled by the user or if the execution could not be performed due to
    a wrong context (without use of separate <SfxShell>s). 'Done ()' will be
    launched, when executing the function led to a regular error
    (for example, file could not be opened).
*/

{
    Done_Impl( &rSet );

    // Keep items if possible, so they can be queried by StarDraw.
    if ( bKeep )
    {
        if ( !pArgs )
        {
            pArgs = new SfxAllItemSet( rSet );
            pImp->SetPool( pArgs->GetPool() );
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
}

//--------------------------------------------------------------------


void SfxRequest::Done( sal_Bool bRelease )
//  [<SfxRequest::Done(SfxItemSet&)>]
{
    Done_Impl( pArgs );
    if( bRelease )
        DELETEZ( pArgs );
}

//--------------------------------------------------------------------

void SfxRequest::ForgetAllArgs()
{
    DELETEZ( pArgs );
    DELETEZ( pImp->pInternalArgs );
}

//--------------------------------------------------------------------

sal_Bool SfxRequest::IsCancelled() const
{
    return pImp->bCancelled;
}

//--------------------------------------------------------------------

void SfxRequest::Cancel()

/*  [Description]

    Marks this request as no longer executable. For example, if called when
    the target (more precisely, its pool) dies.
*/

{
    pImp->bCancelled = sal_True;
    pImp->SetPool( 0 );
    DELETEZ( pArgs );
}

//--------------------------------------------------------------------


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
    pImp->bIgnored = sal_True;
}

//--------------------------------------------------------------------

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
    pImp->bDone = sal_True;

    // not Recording
    if ( !pImp->xRecorder.is() )
        return;

    // was running a different slot than requested (Delegation)
    if ( nSlot != pImp->pSlot->GetSlotId() )
    {
        // Search Slot again
        pImp->pSlot = pImp->pShell->GetInterface()->GetSlot(nSlot);
        DBG_ASSERT( pImp->pSlot, "delegated SlotId not found" );
        if ( !pImp->pSlot ) // playing it safe
            return;
    }

    // recordable?
    // new Recording uses UnoName!
    if ( !pImp->pSlot->pUnoName )
    {
        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
            "Recording not exported slot: "));
        aStr.append(static_cast<sal_Int32>(pImp->pSlot->GetSlotId()));
        OSL_FAIL(aStr.getStr());
    }

    if ( !pImp->pSlot->pUnoName ) // playing it safe
        return;

    // often required values
    SfxItemPool &rPool = pImp->pShell->GetPool();

    // Property-Slot?
    if ( !pImp->pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // get the property as SfxPoolItem
        const SfxPoolItem *pItem;
        sal_uInt16 nWhich = rPool.GetWhich(pImp->pSlot->GetSlotId());
        SfxItemState eState = pSet ? pSet->GetItemState( nWhich, sal_False, &pItem ) : SFX_ITEM_UNKNOWN;
#ifdef DBG_UTIL
        if ( SFX_ITEM_SET != eState )
        {
            rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                "Recording property not available: "));
            aStr.append(static_cast<sal_Int32>(pImp->pSlot->GetSlotId()));
            OSL_FAIL(aStr.getStr());
        }
#endif
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( eState == SFX_ITEM_SET )
            TransformItems( pImp->pSlot->GetSlotId(), *pSet, aSeq, pImp->pSlot );
        pImp->Record( aSeq );
    }

    // record everything in a single statement?
    else if ( pImp->pSlot->IsMode(SFX_SLOT_RECORDPERSET) )
    {
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( pSet )
            TransformItems( pImp->pSlot->GetSlotId(), *pSet, aSeq, pImp->pSlot );
        pImp->Record( aSeq );
    }

    // record each item as a single statement
    else if ( pImp->pSlot->IsMode(SFX_SLOT_RECORDPERITEM) )
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
                    SfxSlot *pSlot = (SfxSlot*) pImp->pSlot;
                    pSlot->nFlags &= ~((sal_uIntPtr)SFX_SLOT_RECORDPERITEM);
                    pSlot->nFlags &=  SFX_SLOT_RECORDPERSET;
                }

                // Record a Sub-Request
                SfxRequest aReq( pImp->pViewFrame, nSlotId );
                if ( aReq.pImp->pSlot )
                    aReq.AppendItem( *pItem );
                aReq.Done();
            }
        }
        else
        {
          //HACK(think about this again)
            pImp->Record( uno::Sequence < beans::PropertyValue >() );
        }
    }
}

//--------------------------------------------------------------------

sal_Bool SfxRequest::IsDone() const

/*  [Description]

    With this method it can be queried whether the SfxRequest was actually
    executed or not. If a SfxRequest was not executed, then this is for example
    because it was canceled by the user or the context for this request was
    wrong, this was not implemented on a separate <SfxShell>.

    SfxRequest instances that return sal_False will not be recorded.

    [Cross-reference]

    <SfxRequest::Done(const SfxItemSet&)>
    <SfxRequest::Done()>
*/

{
    return pImp->bDone;
}

//--------------------------------------------------------------------

com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > SfxRequest::GetMacroRecorder( SfxViewFrame* pView )

/*  [Description]

    This recorder is an attempt for dispatch () to get calls from the Frame.
    This is then available through a property by a supplier but only when
    recording was turned on.

    (See also SfxViewFrame::MiscExec_Impl() and SID_RECORDING)
*/

{
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
        (pView ? pView : SfxViewFrame::Current())->GetFrame().GetFrameInterface(),
        com::sun::star::uno::UNO_QUERY);

    if(xSet.is())
    {
        com::sun::star::uno::Any aProp = xSet->getPropertyValue(rtl::OUString("DispatchRecorderSupplier"));
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
        aProp >>= xSupplier;
        if(xSupplier.is())
            xRecorder = xSupplier->getDispatchRecorder();
    }

    return xRecorder;
}

sal_Bool SfxRequest::HasMacroRecorder( SfxViewFrame* pView )
{
    return GetMacroRecorder( pView ).is();
}


//--------------------------------------------------------------------

sal_Bool SfxRequest::IsAPI() const

/*  [Description]

    Returns sal_True if this SfxRequest was generated by an API (for example BASIC),
    otherwise sal_False.
*/

{
    return SFX_CALLMODE_API == ( SFX_CALLMODE_API & pImp->nCallMode );
}

//--------------------------------------------------------------------
void SfxRequest::SetModifier( sal_uInt16 nModi )
{
    pImp->nModifier = nModi;
}

//--------------------------------------------------------------------
sal_uInt16 SfxRequest::GetModifier() const
{
    return pImp->nModifier;
}

//--------------------------------------------------------------------

void SfxRequest::AllowRecording( sal_Bool bSet )
{
    pImp->bAllowRecording = bSet;
}

sal_Bool SfxRequest::AllowsRecording() const
{
    sal_Bool bAllow = pImp->bAllowRecording;
    if( !bAllow )
        bAllow = ( SFX_CALLMODE_API != ( SFX_CALLMODE_API & pImp->nCallMode ) ) &&
                 ( SFX_CALLMODE_RECORD == ( SFX_CALLMODE_RECORD & pImp->nCallMode ) );
    return bAllow;
}

void SfxRequest::ReleaseArgs()
{
    DELETEZ( pArgs );
    DELETEZ( pImp->pInternalArgs );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
