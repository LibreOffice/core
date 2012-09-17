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
#include <svtools/itemdel.hxx>
#include <svtools/asynclink.hxx>
#include <basic/sbx.hxx>

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

//====================================================================

DBG_NAME(SfxShell)

//====================================================================

TYPEINIT0(SfxShell);

//====================================================================
class SfxVerbSlotArr_Impl : public std::vector<SfxSlot*>
{
public:
    ~SfxVerbSlotArr_Impl()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }
};

using namespace com::sun::star;

//=========================================================================
// SfxShell_Impl
//=========================================================================
struct SfxShell_Impl: public SfxBroadcaster
{
    String                   aObjectName;   // Name of Sbx-Objects
    SfxItemPtrMap            aItems;        // Data exchange on Item level
    SfxViewShell*            pViewSh;       // SfxViewShell if Shell is
                                            // ViewFrame/ViewShell/SubShell list
    SfxViewFrame*            pFrame;        // Frame, if  <UI-active>
    SfxRepeatTarget*         pRepeatTarget; // SbxObjectRef xParent;
    sal_Bool                        bInAppBASIC;
    sal_Bool                        bActive;
    sal_uIntPtr                     nDisableFlags;
    sal_uIntPtr                       nHelpId;
    svtools::AsynchronLink*     pExecuter;
    svtools::AsynchronLink*     pUpdater;
    SfxVerbSlotArr_Impl         aSlotArr;

    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > aVerbList;
    SfxShell_Impl()  : pExecuter( 0 ), pUpdater( 0 ) {}
    ~SfxShell_Impl() { delete pExecuter; delete pUpdater;}
};

//=========================================================================
// SfxShell
//=========================================================================

void SfxShell::EmptyExecStub(SfxShell *, SfxRequest &)
{
}

void SfxShell::EmptyStateStub(SfxShell *, SfxItemSet &)
{
}

SfxShell::SfxShell()

/*  [Description]

    The constructor of the SfxShell class initializes only simple types,
    the corresponding SbxObject is only created on-demand. Therefore,
    the application of a SfxShell instance is very cheap.
*/

:   pImp(0),
    pPool(0),
    pUndoMgr(0)
{
    DBG_CTOR(SfxShell, 0);
    pImp = new SfxShell_Impl;
    pImp->pViewSh = 0;
    pImp->pFrame = 0;
    pImp->pRepeatTarget = 0;
    pImp->bInAppBASIC = sal_False;
    pImp->nHelpId = 0L;
    pImp->bActive = sal_False;
    pImp->nDisableFlags = 0;
}

//-------------------------------------------------------------------------

SfxShell::SfxShell( SfxViewShell *pViewSh )

/*  [Description]

    The constructor of the SfxShell class initializes only simple types,
    the corresponding SbxObject is only created on-demand. Therefore,
    the application of a SfxShell instance is very cheap.
*/

:   pImp(0),
    pPool(0),
    pUndoMgr(0)
{
    DBG_CTOR(SfxShell, 0);
    pImp = new SfxShell_Impl;
    pImp->pViewSh = pViewSh;
    pImp->pFrame = 0;
    pImp->pRepeatTarget = 0;
    pImp->bInAppBASIC = sal_False;
    pImp->nHelpId = 0L;
    pImp->bActive = sal_False;
}

//--------------------------------------------------------------------

SfxShell::~SfxShell()

/*  [Description]

    The connection to a possible corresponding SbxObject is dissolved.
    The SbxObject may continoue to exist, but can not any longer perform
    any functions and can not provide any properties.
*/

{
    DBG_DTOR(SfxShell, 0);
    delete pImp;
}

//--------------------------------------------------------------------

void SfxShell::SetName( const String &rName )

/*  [Description]

    Sets the name of the Shell object. With this name, the SfxShell instance
    of BASIC can be expressed.
*/

{
    pImp->aObjectName = rName;
}

//--------------------------------------------------------------------

const String& SfxShell::GetName() const

/*  [Description]

    Returns the name of the Shell object. With this name, the SfxShell instance
    of BASIC can be expressed.
*/

{
    return pImp->aObjectName;
}

//--------------------------------------------------------------------

SvGlobalName SfxShell::GetGlobalName() const

/*  [Description]

    Provides the Global Unique Identifier of the Shell object. With this name
    can the SfxShell instance for example be expressed via OLE Automation, or
    be found in the Registration Database.
*/

{
    return SvGlobalName();
}

//--------------------------------------------------------------------

SfxDispatcher* SfxShell::GetDispatcher() const

/*  [Description]

    This method returns a pointer to the <SfxDispatcher>, when the SfxShell
    is currently <UI-active> or a NULL-pointer if it is not UI-active.

    The returned pointer is only valid in the immediate context of the method
    call.
*/

{
    return pImp->pFrame ? pImp->pFrame->GetDispatcher() : 0;
}

//--------------------------------------------------------------------

SfxViewShell* SfxShell::GetViewShell() const

/*  [Description]

    Returns the SfxViewShell in which they are located in the subshells.
    Otherwise, and if not specified by the App developer, this method
    returns NULL.
*/

{
    return pImp->pViewSh;
}

//--------------------------------------------------------------------

SfxViewFrame* SfxShell::GetFrame() const

/*  [Description]

    This method returns a pointer to the <SfxViewFrame> to which this SfxShell
    instance is associated or in which they currently is <UI-active>.
    A NULL pointer is returned if this SfxShell instance is not UI-active at
    the moment and also no SfxViewFrame is permanently assigned.

    The returned pointer is only valid in the immediate context of the method
    call.

    [Note]

    Only instances of a subclass of SfxApplication and SfxObjectShell
    should here provide a NULL-pointer. Otherwise, there is an error in the
    application program (wrong constructor was called from SfxShell).

    [Cross-reference]

    <SfxViewShell::GetViewFrame()const>
*/

{
    if ( pImp->pFrame )
        return pImp->pFrame;
    if ( pImp->pViewSh )
        return pImp->pViewSh->GetViewFrame();
    return 0;
}

//--------------------------------------------------------------------

const SfxPoolItem* SfxShell::GetItem
(
    sal_uInt16  nSlotId         // Slot-Id of the querying <SfxPoolItem>s
)   const

/*  [Description]

    With this method any objects of <SfxPoolItemu> subclasses can be accessed.
    This exchange method is needed if, for example special <SfxToolBoxControl>
    subclasses need access to certain data such as the <SfxObjectShell>.

    The returned instance belongs to the particular SfxShell and may be
    used only in the immediate context of the method call.

    [Cross-reference]

    <SfxShell::PutItem(const SfxPoolItem&)>
    <SfxShell::RemoveItem(sal_uInt16)>
*/

{
    SfxItemPtrMap::iterator it = pImp->aItems.find( nSlotId );
    if( it != pImp->aItems.end() )
        return it->second;
    return 0;
}

//--------------------------------------------------------------------

void SfxShell::PutItem
(
    const SfxPoolItem&  rItem  /* Instance, of which a copy is created,
                                  which is stored in the SfxShell in a list. */
)

/*  [Description]

    With this method, any objects of subclasses of <SfxPoolItem> can be made
    available. This exchange technology is needed if, for example, special
    <SfxToolBoxControl> Subclasses need access to certain data such as the
    <SfxObjectShell>

    If a SfxPoolItem exists with the same slot ID, it is deleted automatically.

    [Cross-reference]

    <SfxShell::RemoveItem(sal_uInt16)>
    <SfxShell::GetItem(sal_uInt16)>
*/

{
    DBG_ASSERT( !rItem.ISA(SfxSetItem), "SetItems aren't allowed here" );
    DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                "items with Which-Ids aren't allowed here" );

    // MSC made a mess here of WNT/W95, beware of changes
    SfxPoolItem *pItem = rItem.Clone();
    SfxPoolItemHint aItemHint( pItem );
    const sal_uInt16 nWhich = rItem.Which();

    SfxItemPtrMap::iterator it = pImp->aItems.find( nWhich );
    if( it != pImp->aItems.end() )
    {
        SfxPoolItem *pLoopItem = it->second;
        // Replace Item
        delete pLoopItem;
        it->second = pItem;

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
                pCache->SetState( SFX_ITEM_AVAILABLE, pItem->Clone(), sal_True );
                pCache->SetCachedState( sal_True );
            }
        }
        return;
    }
    else
    {
        Broadcast( aItemHint );
        pImp->aItems[ pItem->Which() ] = pItem;
    }
}

//--------------------------------------------------------------------

SfxInterface* SfxShell::GetInterface() const

/*  [Description]

    With this virtual method, which is automatically overloaded by each subclass
    with its own slots through the macro <SFX_DECL_INTERFACE>, one can access
    each of the <SfxInterface> instance beloning to the subclass.

    The class SfxShell itself has no own SfxInterface (no slots), therefore a
    NULL-pointer is returned.
*/

{
    return GetStaticInterface();
}

//--------------------------------------------------------------------

::svl::IUndoManager* SfxShell::GetUndoManager()

/*  [Description]

    Each Subclass of SfxShell can hava a <SfxUndoManager>. This can be set in
    the derived class with <SfxShell:SetUndoManager()>.

    The class SfxShell itself does not have a SfxUndoManager, a NULL-pointer
    is therefore returned.
*/

{
    return pUndoMgr;
}

//--------------------------------------------------------------------

void SfxShell::SetUndoManager( ::svl::IUndoManager *pNewUndoMgr )

/*  [Description]

    Sets a <SfxUndoManager> for this <SfxShell> Instance. For the undo
    is only the undo-manager used for SfxShell at the top of the stack of each
    <SfxDispatcher>.

    On the given <SfxUndoManager> is automatically the current
    Max-Undo-Action-Count setting set form the options.

    'pNewUndoMgr' must exist until the Destuctor of SfxShell instance is called
    or until the next 'SetUndoManager()'.
*/

{
    OSL_ENSURE( ( pUndoMgr == NULL ) || ( pNewUndoMgr == NULL ) || ( pUndoMgr == pNewUndoMgr ),
        "SfxShell::SetUndoManager: exchanging one non-NULL manager with another non-NULL manager? Suspicious!" );
    // there's at least one client of our UndoManager - the DocumentUndoManager at the SfxBaseModel - which
    // caches the UndoManager, and registers itself as listener. If exchanging non-NULL UndoManagers is really
    // a supported scenario (/me thinks it is not), then we would need to notify all such clients instances.

    pUndoMgr = pNewUndoMgr;
    if ( pUndoMgr )
        pUndoMgr->SetMaxUndoActionCount(
            officecfg::Office::Common::Undo::Steps::get());
}

//--------------------------------------------------------------------

SfxRepeatTarget* SfxShell::GetRepeatTarget() const

/*  [Description]

    Returns a pointer to the <SfxRepeatTarget> instance that is used in
    SID_REPEAT as repeat target when it is addressed from the <SfxUndoManager>
    supplied by this SfxShell. The return value can be NULL.

    [Note]

    A derivation of <SfxShell> or one of its subclasses of <SfxRepeatTarget>
    is not recommended, as compiler errors are provoked.
    (due to Call-to-Pointer-to-Member-Function to the subclass).
*/

{
    return pImp->pRepeatTarget;
}

//--------------------------------------------------------------------

void SfxShell::SetRepeatTarget( SfxRepeatTarget *pTarget )

/*  [Description]

    Sets the <SfxRepeatTarget> instance that is used in SID_REPEAT as
    RepeatTarget, when the current supplied by this <SfxUndoManager> is
    addressed. By 'pTarget==0' the SID_REPEAT is disabled for this SfxShell.
    The instance '*pTarget' must live as long as it is registered.

    [Note]

    A derivation of <SfxShell> or one of its subclasses of <SfxRepeatTarget>
    is not recommended, as compiler errors are provoked.
    (due to Call-to-Pointer-to-Member-Function to the subclass).
*/

{
    pImp->pRepeatTarget = pTarget;
}

//--------------------------------------------------------------------

void SfxShell::Invalidate
(
    sal_uInt16          nId     /* Invalidated Slot-Id or Which-Id.
                               If these are 0 (default), then all
                               by this Shell currently handled Slot-Ids are
                               invalidated. */
)

/*  [Description]

    With this method can the slots of the subclasses be invalidated through the
    slot Id or alternatively through the Which ID. Slot IDs, which are
    inherited by the subclass are also invalidert.

    [Cross-reference]

    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::InvalidateAll(sal_Bool)>
*/

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
        rBindings.InvalidateShell( *this, sal_False );
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

        DBG_WARNING( "W3: invalidating slot-id unknown in shell" );
    }
}

//--------------------------------------------------------------------

void SfxShell::DoActivate_Impl( SfxViewFrame *pFrame, sal_Bool bMDI )

/*  [Description]

    This method controls the activation of SfxShell instance. First, by calling
    the virtual method <SfxShell::Activate(sal_Bool)> which gives the subclass the
    opportunity to respond to the event.

    When bMDI == TRUE, the associated SbxObject is being 'armed', so that
    unqualified methods of the object (without the name of the object)
    from BASIC are found.
*/

{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
    SAL_INFO(
        "sfx2.vb",
        "SfxShell::DoActivate() " << this << "  " << GetInterface()->GetName()
            << " bMDI " << (bMDI ? "MDI" : ""));

    if ( bMDI )
    {
        // Remember Frame, in which it was activated
        pImp->pFrame = pFrame;
        pImp->bActive = sal_True;
    }

    // Notify Subclass
    Activate(bMDI);
}

//--------------------------------------------------------------------

void SfxShell::DoDeactivate_Impl( SfxViewFrame *pFrame, sal_Bool bMDI )

/*  [Description]

    This method controls the deactivation of the SfxShell instance. When
    bMDI == TRUE the SbxObject is first set to a status that only qualified
    BASIC methods can be called.

    Then the subclass gets the opportunity in every case to respond to the
    event by calling the virtual method <SfxShell::Deactivate(sal_Bool)>.
*/

{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
    SAL_INFO(
        "sfx2.vb",
        "SfxShell::DoDeactivate()" << this << "  " << GetInterface()->GetName()
            << " bMDI " << (bMDI ? "MDI" : ""));

    // Only when it comes from a Frame
    // (not when for instance by poping BASIC-IDE from AppDisp)
    if ( bMDI && pImp->pFrame == pFrame )
    {
        // deliver
        pImp->pFrame = 0;
        pImp->bActive = sal_False;
    }

    // Notify Subclass
    Deactivate(bMDI);
}

//--------------------------------------------------------------------

sal_Bool SfxShell::IsActive() const
{
    return pImp->bActive;
}

//--------------------------------------------------------------------

void SfxShell::Activate
(
    sal_Bool    /*bMDI*/        /*  TRUE
                            the <SfxDispatcher>, on which the SfxShell is
                            located, is activated or the SfxShell instance
                            was pushed on an active SfxDispatcher.
                            (compare with SystemWindow::IsMDIActivate())

                            FALSE
                            the <SfxViewFrame>, on which SfxDispatcher
                            the SfxShell instance is located, was
                            activated. (for example by a closing dialoge) */
)

/*  [Description]

    Virtual method that is called when enabling the SfxShell instance,
    in order to give the Subclasses the opportunity to respond to the
    to the enabling.

    The base implementation is empty and does not need to be called.

    [Cross-reference]

    StarView SystemWindow::Activate(sal_Bool)
*/

{
}

//--------------------------------------------------------------------

void SfxShell::Deactivate
(
    sal_Bool    /*bMDI*/        /*  TRUE
                            the <SfxDispatcher>, on which the SfxShell is
                            located, is inactivated or the SfxShell instance
                            was popped on an active SfxDispatcher.
                            (compare with SystemWindow::IsMDIActivate())

                            FALSE
                            the <SfxViewFrame>, on which SfxDispatcher
                            the SfxShell instance is located, was
                            deactivated. (for example by a dialoge) */

)

/*  [Description]

    Virtual method that is called when disabling the SfxShell instance,
    to give the Subclasses the opportunity to respond to the disabling.

    The base implementation is empty and does not need to be called.

    [Cross-reference]

    StarView SystemWindow::Dectivate(sal_Bool)
*/

{
}

void SfxShell::ParentActivate
(
)

/*  [Description]

    A parent of the <SfxDispatcher> on which the SfxShell is located, has
    become active, or the SfxShell instance was pushed on a <SfxDispatcher>,
    which parent is active.

    The base implementation is empty and does not need to be called.

    [Cross-reference]

    SfxShell::Activate()
*/
{
}

//--------------------------------------------------------------------

void SfxShell::ParentDeactivate
(
)

/*  [Description]

    The active parent of the <SfxDispatcher> on which the SfxShell is located,
    has been disabled.

    The base implementation is empty and does not need to be called.

    [Cross-reference]

    SfxShell::Deactivate()
*/
{
}

//--------------------------------------------------------------------

ResMgr* SfxShell::GetResMgr() const

/*  [Description]

    This method provides the ResMgr of the <Resource-DLL> that are used by
    the SfxShell instance. If this is a NULL-pointer, then the current
    resource manager is to be used.
*/

{
    return GetInterface()->GetResMgr();
}

//--------------------------------------------------------------------

bool SfxShell::CanExecuteSlot_Impl( const SfxSlot &rSlot )

/*  [Description]

    This method determines by calling the status function whether 'rSlot'
    can be executed currently.
*/
{
    // Get Slot status
    SfxItemPool &rPool = GetPool();
    const sal_uInt16 nId = rSlot.GetWhich( rPool );
    SfxItemSet aSet(rPool, nId, nId);
    SfxStateFunc pFunc = rSlot.GetStateFnc();
    CallState( pFunc, aSet );
    return aSet.GetItemState(nId) != SFX_ITEM_DISABLED;
}

//--------------------------------------------------------------------

long ShellCall_Impl( void* pObj, void* pArg )
{
    ((SfxShell* )pObj)->ExecuteSlot( *(SfxRequest*)pArg, (SfxInterface*)0L );
    return 0;
}

/*  [Description]

    Asynchronous ExecuteSlot for the RELOAD
 */

//--------------------------------------------------------------------
const SfxPoolItem* SfxShell::ExecuteSlot( SfxRequest& rReq, sal_Bool bAsync )
{
    if( !bAsync )
        return ExecuteSlot( rReq, (SfxInterface*)0L );
    else
    {
        if( !pImp->pExecuter )
            pImp->pExecuter = new svtools::AsynchronLink(
                Link( this, ShellCall_Impl ) );
        pImp->pExecuter->Call( new SfxRequest( rReq ) );
        return 0;
    }
}

const SfxPoolItem* SfxShell::ExecuteSlot
(
    SfxRequest          &rReq,  // the relayed <SfxRequest>
    const SfxInterface* pIF     // default = 0 means get virtually
)

/*  [Description]

    This method allows you to forward a <SfxRequest> to the specified
    base <SfxShell>.

    [Example]

    In a derived class of SfxViewShell the SID_PRINTDOCDIRECT will be
    intercepted. Under certain circumstances a query should appear before
    you print, and the request will be aborted if necessary.

    Also in the IDL of this subclass of the above slot is entered. The status
    method will contain in outline:

    void SubViewShell::Exec( SfxRequest &rReq )
    {
        if ( rReq.GetSlot() == SID_PRINTDOCDIRECT )
        {
            'dialog'
            if ( 'condition' )
                ExecuteSlot( rReq, SfxViewShell::GetInterface() );
        }
    }

    It usually takes no rReq.Done() to be called as that is already completed
    in implementation of the SfxViewShell, for instance it has been canceled.

    [Cross-reference]

    <SfxShell::GetSlotState(sal_uInt16,const SfxInterface*,SfxItemSet*)>
*/

{
    if ( !pIF )
        pIF = GetInterface();

    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxSlot* pSlot = NULL;
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

//--------------------------------------------------------------------

const SfxPoolItem* SfxShell::GetSlotState
(
    sal_uInt16              nSlotId,    // Slot-Id to the Slots in question
    const SfxInterface* pIF,        // default = 0 means get virtually
    SfxItemSet*         pStateSet   // SfxItemSet of the Slot-State method
)

/*  [Description]

    This method returns the status of the slot with the specified slot ID
    on the specified interface.

    If the slot is disabled or in this SfxShell (and their parent shells) are
    not known, a Null-pointer is returned.

    If the slot does not have a Status, a SfxVoidItem is returned.

    The status is set directly in this Set when pStateSet != 0 , so that
    overloaded Slots of the <SfxShell> Subclasses and also in the Status
    method of the base implementation can be called.

    [Example]

    In a derived class of SfxViewShell the SID_PRINTDOCDIRECT will be
    intercepted. Under certain circumstances a query should appear before
    you print, and the request will be aborted if necessary.

    Also in the IDL of this subclass of the above slot is entered. The status
    method will contain in outline:

    void SubViewShell::PrintState( SfxItemSet &rState )
    {
        if ( rState.GetItemState( SID_PRINTDOCDIRECT ) != SFX_ITEM_UNKNOWN )
            GetSlotState( SID_PRINTDOCDIRECT, SfxViewShell::GetInterface(),
                    &rState );
        ...
    }

    [Cross-reference]

    <SfxShell::ExecuteSlot(SfxRequest&)>
*/

{
    // Get Slot on the given Interface
    if ( !pIF )
        pIF = GetInterface();
    SfxItemState eState;
    SfxItemPool &rPool = GetPool();

    const SfxSlot* pSlot = NULL;
    if ( nSlotId >= SID_VERB_START && nSlotId <= SID_VERB_END )
        pSlot = GetVerbSlot_Impl(nSlotId);
    if ( !pSlot )
        pSlot = pIF->GetSlot(nSlotId);
    if ( pSlot )
        // Map on Which-Id if possible
        nSlotId = pSlot->GetWhich( rPool );

    // Get Item and Item status
    const SfxPoolItem *pItem = NULL;
    SfxItemSet aSet( rPool, nSlotId, nSlotId ); // else pItem dies too soon
    if ( pSlot )
    {
        // Call Status method
        SfxStateFunc pFunc = pSlot->GetStateFnc();
        if ( pFunc )
            CallState( pFunc, aSet );
        eState = aSet.GetItemState( nSlotId, sal_True, &pItem );

        // get default Item if possible
        if ( eState == SFX_ITEM_DEFAULT )
        {
            if ( SfxItemPool::IsWhich(nSlotId) )
                pItem = &rPool.GetDefaultItem(nSlotId);
            else
                eState = SFX_ITEM_DONTCARE;
        }
    }
    else
        eState = SFX_ITEM_UNKNOWN;

    // Evaluate Item and item status and possibly maintain them in pStateSet
    SfxPoolItem *pRetItem = 0;
    if ( eState <= SFX_ITEM_DISABLED )
    {
        if ( pStateSet )
            pStateSet->DisableItem(nSlotId);
        return 0;
    }
    else if ( eState == SFX_ITEM_DONTCARE )
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

//--------------------------------------------------------------------

SFX_EXEC_STUB(SfxShell, VerbExec)
SFX_STATE_STUB(SfxShell, VerbState)

void SfxShell::SetVerbs(const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& aVerbs)
{
    SfxViewShell *pViewSh = PTR_CAST ( SfxViewShell, this);

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
            pBindings->Invalidate(nId, sal_False, sal_True);
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
        pNewSlot->nFlags = SFX_SLOT_ASYNCHRON | SFX_SLOT_CONTAINER;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxShell,VerbExec);
        pNewSlot->fnState = SFX_STUB_PTR(SfxShell,VerbState);
        pNewSlot->pType = 0; HACK(SFX_TYPE(SfxVoidItem))
        pNewSlot->pName = U2S(aVerbs[n].VerbName).getStr();
        pNewSlot->pLinkedSlot = 0;
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = 0;
        pNewSlot->pUnoName = 0;

        if (!pImp->aSlotArr.empty())
        {
            SfxSlot *pSlot = pImp->aSlotArr[0];
            pNewSlot->pNextSlot = pSlot->pNextSlot;
            pSlot->pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        pImp->aSlotArr.insert(pImp->aSlotArr.begin() + (sal_uInt16) n, pNewSlot);
    }

    pImp->aVerbList = aVerbs;

    if (pViewSh)
    {
        // The status of SID_OBJECT is collected in the controller directly on
        // the Shell, it is thus enough to encourage a new status update
        SfxBindings *pBindings = pViewSh->GetViewFrame()->GetDispatcher()->
                GetBindings();
        pBindings->Invalidate( SID_OBJECT, sal_True, sal_True );
    }
}

//--------------------------------------------------------------------

const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& SfxShell::GetVerbs() const
{
    return pImp->aVerbList;
}

//--------------------------------------------------------------------

void SfxShell::VerbExec(SfxRequest& rReq)
{
    sal_uInt16 nId = rReq.GetSlot();
    SfxViewShell *pViewShell = GetViewShell();
    if ( pViewShell )
    {
        sal_Bool bReadOnly = pViewShell->GetObjectShell()->IsReadOnly();
        com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > aList = pViewShell->GetVerbs();
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

//--------------------------------------------------------------------

void SfxShell::VerbState(SfxItemSet& )
{
}

//--------------------------------------------------------------------

const SfxSlot* SfxShell::GetVerbSlot_Impl(sal_uInt16 nId) const
{
    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > rList = pImp->aVerbList;

    DBG_ASSERT(nId >= SID_VERB_START && nId <= SID_VERB_END,"Wrong VerbId!");
    sal_uInt16 nIndex = nId - SID_VERB_START;
    DBG_ASSERT(nIndex < rList.getLength(),"Wrong VerbId!");

    if (nIndex < rList.getLength())
        return pImp->aSlotArr[nIndex];
    else
        return 0;
}

//--------------------------------------------------------------------

void SfxShell::SetHelpId(sal_uIntPtr nId)
{
    pImp->nHelpId = nId;
}

//--------------------------------------------------------------------

sal_uIntPtr SfxShell::GetHelpId() const
{
    return pImp->nHelpId;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxShell::GetObjectShell()
{
    if ( GetViewShell() )
        return GetViewShell()->GetViewFrame()->GetObjectShell();
    else
        return NULL;
}

//--------------------------------------------------------------------

sal_Bool SfxShell::HasUIFeature( sal_uInt32 )
{
    return sal_False;
}

long DispatcherUpdate_Impl( void*, void* pArg )
{
    ((SfxDispatcher*) pArg)->Update_Impl( sal_True );
    ((SfxDispatcher*) pArg)->GetBindings()->InvalidateAll(sal_False);
    return 0;
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
            pImp->pUpdater = new svtools::AsynchronLink( Link( this, DispatcherUpdate_Impl ) );

        // Multiple views allowed
        pImp->pUpdater->Call( pFrame->GetDispatcher(), sal_True );
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
    return NULL;
}

void SfxShell::ApplyItemSet( sal_uInt16, const SfxItemSet& )
{
}

void SfxShell::SetViewShell_Impl( SfxViewShell* pView )
{
    pImp->pViewSh = pView;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
