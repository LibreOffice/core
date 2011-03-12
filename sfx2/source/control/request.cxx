/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <com/sun/star/frame/DispatchStatement.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
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
#include "macro.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/appuno.hxx>

//===================================================================

using namespace ::com::sun::star;

struct SfxRequest_Impl: public SfxListener

/*  [Beschreibung]

    Implementations-Struktur der Klasse <SfxRequest>.
*/

{
    SfxRequest*         pAnti;       // Owner wegen sterbendem Pool
    String              aTarget;     // ggf. von App gesetztes Zielobjekt
    SfxItemPool*        pPool;       // ItemSet mit diesem Pool bauen
    SfxPoolItem*        pRetVal;     // R"uckgabewert geh"ort sich selbst
    SfxShell*           pShell;      // ausgef"uhrt an dieser Shell
    const SfxSlot*      pSlot;       // ausgef"uhrter Slot
    sal_uInt16              nModifier;   // welche Modifier waren gedrueckt?
    sal_Bool                bDone;       // "uberhaupt ausgef"uhrt
    sal_Bool                bIgnored;    // vom User abgebrochen
    sal_Bool                bCancelled;  // nicht mehr zustellen
    sal_Bool                bUseTarget;  // aTarget wurde von Applikation gesetzt
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
    DBG_MEMTEST();

    // nicht mit Done() marktierte Requests mit 'rem' rausschreiben
    if ( pImp->xRecorder.is() && !pImp->bDone && !pImp->bIgnored )
        pImp->Record( uno::Sequence < beans::PropertyValue >() );

    // Objekt abr"aumen
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
    DBG_MEMTEST();

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

/*  [Beschreibung]

    Mit diesem Konstruktor k"onnen Events, die nicht "uber den SfxDispatcher
    gelaufen sind (z.B aus KeyInput() oder Mouse-Events) nachtr"aglich
    recorded werden. Dazu wird eine SfxRequest-Instanz mit diesem Konstruktor
    erzeugt und dann genauso verfahren, wie mit einem SfxRequest, der in
    eine <Slot-Execute-Methode> als Parameter gegeben wird.
*/

:   nSlot(nSlotId),
    pArgs(0),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

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
        ByteString aStr( "Recording unsupported slot: ");
        aStr += ByteString::CreateFromInt32( pImp->pPool->GetSlotId(nSlotId) );
        DBG_ERROR( aStr.GetBuffer() );
    }
#endif
}

//--------------------------------------------------------------------


SfxRequest::SfxRequest
(
    sal_uInt16          nSlotId,    // auszuf"uhrende <Slot-Id>
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&    rPool       // ggf. f"ur das SfxItemSet f"ur Parameter
)

// creates a SfxRequest without arguments

:   nSlot(nSlotId),
    pArgs(0),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

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
    const SfxSlot* pSlot,   // auszuf"uhrende <Slot-Id>
    const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rArgs,
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&    rPool       // ggf. f"ur das SfxItemSet f"ur Parameter
)
:   nSlot(pSlot->GetSlotId()),
    pArgs(new SfxAllItemSet(rPool)),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

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
    DBG_MEMTEST();

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
    const uno::Sequence < beans::PropertyValue >& rArgs    // aktuelle Parameter
)

/*  [Beschreibung]

    Interne Hilfsmethode zum erzeugen einer <SfxMacroStatement>-Instanz,
    welche den bereits ausgef"uhrten SfxRequest wiederholbar beschreibt.

    Die erzeugte Instanz, auf die ein Pointer zur"uckgeliefert wird
    geht in das Eigentum des Aufrufers "uber.
*/

{
    String aCommand = String::CreateFromAscii(".uno:");
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

        com::sun::star::uno::Reference< com::sun::star::util::XURLTransformer > xTransform(
                xFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
                com::sun::star::uno::UNO_QUERY);

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
    SfxShell& rSh,    // die <SfxShell>, die den Request ausgef"uhrt hat
    const SfxSlot&  rSlot,  // der <SfxSlot>, der den Request ausgef"uhrt hat
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder,  // der Recorder, mit dem aufgezeichnet wird
    SfxViewFrame* pViewFrame
)

/*  [Beschreibung]

    Diese interne Methode markiert den SfxRequest als in dem angegebenen
    SfxMakro aufzuzeichnen.

    Pointer auf die Parameter werden in Done() wieder verwendet, m"usseb
    dann also noch leben.
*/

{
    DBG_MEMTEST();
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
    sal_uInt16      nSlotId,    // Slot-Id oder Which-Id des Parameters
    bool            bDeep,      // false: nicht in Parent-ItemSets suchen
    TypeId          aType       // != 0:  RTTI Pruefung mit Assertion
)   const
{
    return GetItem( pArgs, nSlotId, bDeep, aType );
}


//--------------------------------------------------------------------
const SfxPoolItem* SfxRequest::GetItem
(
    const SfxItemSet* pArgs,
    sal_uInt16          nSlotId,    // Slot-Id oder Which-Id des Parameters
    bool            bDeep,      // false: nicht in Parent-ItemSets suchen
    TypeId          aType       // != 0:  RTTI Pruefung mit Assertion
)

/*  [Beschreibung]

    Mit dieser Methode wird der Zugriff auf einzelne Parameter im
    SfxRequest wesentlich vereinfacht. Insbesondere wird die Typpr"ufung
    (per Assertion) durchgef"uhrt, wodurch die Applikations-Sourcen
    wesentlich "ubersichtlicher werden. In der PRODUCT-Version wird
    eine 0 zur"uckgegeben, wenn das gefundene Item nicht von der
    angegebenen Klasse ist.


    [Beispiel]

    void MyShell::Execute( SfxRequest &rReq )
    {
        switch ( rReq.GetSlot() )
        {
            case SID_MY:
            {
                ...
                // ein Beispiel ohne Verwendung des Makros
                const SfxInt32Item *pPosItem = (const SfxUInt32Item*)
                    rReq.GetArg( SID_POS, sal_False, TYPE(SfxInt32Item) );
                sal_uInt16 nPos = pPosItem ? pPosItem->GetValue() : 0;

                // ein Beispiel mit Verwendung des Makros
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
        // ggf. in Which-Id umrechnen
        sal_uInt16 nWhich = pArgs->GetPool()->GetWhich(nSlotId);

        // ist das Item gesetzt oder bei bDeep==TRUE verf"ugbar?
        const SfxPoolItem *pItem = 0;
        if ( ( bDeep ? SFX_ITEM_AVAILABLE : SFX_ITEM_SET )
             <= pArgs->GetItemState( nWhich, bDeep, &pItem ) )
        {
            // stimmt der Typ "uberein?
            if ( !pItem || pItem->IsA(aType) )
                return pItem;

            // Item da aber falsch => Programmierfehler
            OSL_FAIL(  "invalid argument type" );
        }
    }

    // keine Parameter, nicht gefunden oder falschen Typ gefunden
    return 0;
}

//--------------------------------------------------------------------

void SfxRequest::SetReturnValue(const SfxPoolItem &rItem)
{
    DBG_ASSERT(!pImp->pRetVal, "Returnwert mehrfach setzen?");
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
    const SfxItemSet&   rSet,   /*  von der Applikation mitgeteilte Parameter,
                                    die z.B. in einem Dialog vom Benuter
                                    erfragt wurden, ggf. 0 falls keine
                                    Parameter gesetzt wurden */

    bool            bKeep   /*  true (default)
                                    'rSet' wird gepeichert und ist "uber
                                    GetArgs() abfragbar

                                    false
                                    'rSet' wird nicht kopiert (schneller) */
)

/*  [Beschreibung]

    Diese Methode mu\s in der <Execute-Methode> des <SfxSlot>s gerufen
    werden, der den SfxRequest ausgef"uhrt hat, wenn die Ausf"uhrung
    tats"achlich stattgefunden hat. Wird 'Done()' nicht gerufen, gilt
    der SfxRequest als abgebrochen.

    Etwaige Returnwerte werden nur durchgereicht, wenn 'Done()' gerufen
    wurde. Ebenso werden beim Aufzeichnen von Makros nur echte
    Statements erzeugt, wenn 'Done()' gerufen wurde; f"ur SfxRequests,
    die nicht derart gekennzeichnet wurden, wird anstelle dessen eine
    auf die abgebrochene Funktion hinweisende Bemerkung ('rem') eingf"ugt.


    [Anmerkung]

    'Done()' wird z.B. nicht gerufen, wenn ein durch die Funktion gestarteter
    Dialog vom Benutzer abgebrochen wurde oder das Ausf"uhren aufgrund
    eines falschen Kontextes (ohne Verwendung separater <SfxShell>s)
    nicht durchgef"uhrt werden konnte. 'Done()' mu\s sehr wohl gerufen
    werden, wenn das Ausf"uhren der Funktion zu einem regul"aren Fehler
    f"uhrte (z.B. Datei konnte nicht ge"offnet werden).
*/

{
    Done_Impl( &rSet );

    // ggf. Items merken, damit StarDraw sie abfragen kann
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

/*  [Beschreibung]

    Markiert diesen Request als nicht mehr auszufuehren. Wird z.B. gerufen,
    wenn das Ziel (genauer dessen Pool) stirbt.
*/

{
    pImp->bCancelled = sal_True;
    pImp->SetPool( 0 );
    DELETEZ( pArgs );
}

//--------------------------------------------------------------------


void SfxRequest::Ignore()

/*  [Beschreibung]

    Wird diese Methode anstelle von <SfxRequest::Done()> gerufen, dann
    wird dieser Request nicht recorded.


    [Bespiel]

    Das Selektieren von Tools im StarDraw soll nicht aufgezeichnet werden,
    dieselben Slots sollen aber zum erzeugen der von den Tools zu
    erzeugenden Objekte verwendet werde. Also kann nicht NoRecord
    angegeben werden, dennoch soll u.U. nicht aufgezeichnet werden.
*/

{
    // als tats"achlich ausgef"uhrt markieren
    pImp->bIgnored = sal_True;
}

//--------------------------------------------------------------------

void SfxRequest::Done_Impl
(
    const SfxItemSet*   pSet    /*  von der Applikation mitgeteilte Parameter,
                                    die z.B. in einem Dialog vom Benuter
                                    erfragt wurden, ggf. 0 falls keine
                                    Parameter gesetzt wurden */
)

/*  [Beschreibung]

    Interne Methode zum als 'done' markieren des SfxRequest und zum Auswerten
    der Parameter in 'pSet' falls aufgezeichnet wird.
*/

{
    // als tats"achlich ausgef"uhrt markieren
    pImp->bDone = sal_True;

    // nicht Recorden
    if ( !pImp->xRecorder.is() )
        return;

    // wurde ein anderer Slot ausgef"uhrt als angefordert (Delegation)
    if ( nSlot != pImp->pSlot->GetSlotId() )
    {
        // Slot neu suchen
        pImp->pSlot = pImp->pShell->GetInterface()->GetSlot(nSlot);
        DBG_ASSERT( pImp->pSlot, "delegated SlotId not found" );
        if ( !pImp->pSlot ) // Hosentr"ger und G"urtel
            return;
    }

    // record-f"ahig?
    // neues Recorden verwendet UnoName!
    if ( !pImp->pSlot->pUnoName )
    {
        ByteString aStr( "Recording not exported slot: ");
        aStr += ByteString::CreateFromInt32( pImp->pSlot->GetSlotId() );
        DBG_ERROR( aStr.GetBuffer() );
    }

    if ( !pImp->pSlot->pUnoName ) // Hosentr"ger und G"urtel
        return;

    // "ofters ben"otigte Werte
    SfxItemPool &rPool = pImp->pShell->GetPool();

    // Property-Slot?
    if ( !pImp->pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // des Property als SfxPoolItem besorgen
        const SfxPoolItem *pItem;
        sal_uInt16 nWhich = rPool.GetWhich(pImp->pSlot->GetSlotId());
        SfxItemState eState = pSet ? pSet->GetItemState( nWhich, sal_False, &pItem ) : SFX_ITEM_UNKNOWN;
#ifdef DBG_UTIL
        if ( SFX_ITEM_SET != eState )
        {
            ByteString aStr( "Recording property not available: ");
            aStr += ByteString::CreateFromInt32( pImp->pSlot->GetSlotId() );
            DBG_ERROR( aStr.GetBuffer() );
        }
#endif
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( eState == SFX_ITEM_SET )
            TransformItems( pImp->pSlot->GetSlotId(), *pSet, aSeq, pImp->pSlot );
        pImp->Record( aSeq );
    }

    // alles in ein einziges Statement aufzeichnen?
    else if ( pImp->pSlot->IsMode(SFX_SLOT_RECORDPERSET) )
    {
        uno::Sequence < beans::PropertyValue > aSeq;
        if ( pSet )
            TransformItems( pImp->pSlot->GetSlotId(), *pSet, aSeq, pImp->pSlot );
        pImp->Record( aSeq );
    }

    // jedes Item als einzelnes Statement recorden
    else if ( pImp->pSlot->IsMode(SFX_SLOT_RECORDPERITEM) )
    {
        if ( pSet )
        {
            // "uber die Items iterieren
            SfxItemIter aIter(*pSet);
            for ( const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem() )
            {
                // die Slot-Id f"ur das einzelne Item ermitteln
                sal_uInt16 nSlotId = rPool.GetSlotId( pItem->Which() );
                if ( nSlotId == nSlot )
                {
                    // mit Hosentr"ager und G"urtel reparieren des falschen Flags
                    OSL_FAIL( "recursion RecordPerItem - use RecordPerSet!" );
                    SfxSlot *pSlot = (SfxSlot*) pImp->pSlot;
                    pSlot->nFlags &= ~((sal_uIntPtr)SFX_SLOT_RECORDPERITEM);
                    pSlot->nFlags &=  SFX_SLOT_RECORDPERSET;
                }

                // einen Sub-Request recorden
                SfxRequest aReq( pImp->pViewFrame, nSlotId );
                if ( aReq.pImp->pSlot )
                    aReq.AppendItem( *pItem );
                aReq.Done();
            }
        }
        else
        {
            HACK(hierueber nochmal nachdenken)
            pImp->Record( uno::Sequence < beans::PropertyValue >() );
        }
    }
}

//--------------------------------------------------------------------

sal_Bool SfxRequest::IsDone() const

/*  [Beschreibung]

    Mit dieser Methode kann abgefragt werden, ob der SfxRequest tats"achlich
    ausgef"uhrt wurde oder nicht. Wurde ein SfxRequest nicht ausgef"uhrt,
    liegt dies z.B. daran, da\s der Benutzer abgebrochen hat oder
    der Kontext f"ur diesen Request falsch war, dieses aber nicht "uber
    eine separate <SfxShell> realisiert wurde.

    SfxRequest-Instanzen, die hier sal_False liefern, werden nicht recorded.


    [Querverweise]

    <SfxRequest::Done(const SfxItemSet&)>
    <SfxRequest::Done()>
*/

{
    return pImp->bDone;
}

//--------------------------------------------------------------------

SfxMacro* SfxRequest::GetRecordingMacro()

/*  [Beschreibung]

    Mit dieser Methode kann abgefragt werden, ob und in welchem <SfxMacro>
    die SfxRequests gerade aufgezeichnet werden.
*/

{
    return NULL;
}

//--------------------------------------------------------------------

com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > SfxRequest::GetMacroRecorder( SfxViewFrame* pView )

/*  [Beschreibung]

    Hier wird versucht einen Recorder fuer dispatch() Aufrufe vom Frame zu bekommen.
    Dieser ist dort per Property an einem Supplier verfuegbar - aber nur dann, wenn
    recording angeschaltet wurde.
    (Siehe auch SfxViewFrame::MiscExec_Impl() und SID_RECORDING)
*/

{
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;

    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
        (pView ? pView : SfxViewFrame::Current())->GetFrame().GetFrameInterface(),
        com::sun::star::uno::UNO_QUERY);

    if(xSet.is())
    {
        com::sun::star::uno::Any aProp = xSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DispatchRecorderSupplier")));
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

/*  [Beschreibung]

    Liefert sal_True, wenn dieser SfxRequest von einer API (z.B. BASIC)
    erzeugt wurde, sonst sal_False.
*/

{
    return SFX_CALLMODE_API == ( SFX_CALLMODE_API & pImp->nCallMode );
}

//--------------------------------------------------------------------


bool SfxRequest::IsRecording() const

/*  [Beschreibung]

    Liefert sal_True, wenn dieser SfxRequest recorded werden soll, d.h.
    1. zu Zeit ein Makro aufgezeichnet wird
    2. dieser Request "uberhaupt aufgezeichnet wird
    3. der Request nicht von reiner API (z.B. BASIC) ausgeht,
    sonst sal_False.
*/

{
    return ( AllowsRecording() && GetMacroRecorder().is() );
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

void SfxRequest::SetTarget( const String &rTarget )

/*  [Beschreibung]

    Mit dieser Methode kann das zu recordende Zielobjekt umgesetzt werden.


    [Beispiel]

    Die BASIC-Methode 'Open' wird zwar von der Shell 'Application' ausgef"uhrt,
    aber am Objekt 'Documents' (global) recorded:

        rReq.SetTarget( "Documents" );

    Dies f"uhrt dann zu:

        Documents.Open( ... )
*/

{
    pImp->aTarget = rTarget;
    pImp->bUseTarget = sal_True;
}

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
