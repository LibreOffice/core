/*==================================================================
// class SfxRequest
//
// (C) 1996 - 2000 StarDivision GmbH, Hamburg, Germany
// $Author: rt $ $Date: 2003-09-19 07:58:55 $ $Revision: 1.13 $
// $Logfile:   T:/sfx2/source/control/request.cxv  $ $Workfile:   REQUEST.CXX  $
//------------------------------------------------------------------*/

#ifndef _COM_SUN_STAR_FRAME_DISPATCHSTATEMENT_HPP_
#include <com/sun/star/frame/DispatchStatement.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDERSUPPLIER_HPP_
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _ARGS_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVTOOLS_ITEMDEL_HXX
#include <svtools/itemdel.hxx>
#endif

#include <comphelper/processfactory.hxx>

#pragma hdrstop

#include "request.hxx"
#include "dispatch.hxx"
#include "msg.hxx"
#include "viewfrm.hxx"
#include "macro.hxx"
#include "objface.hxx"
#include "appuno.hxx"

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
    USHORT              nModifier;   // welche Modifier waren gedrueckt?
    BOOL                bDone;       // "uberhaupt ausgef"uhrt
    BOOL                bIgnored;    // vom User abgebrochen
    BOOL                bCancelled;  // nicht mehr zustellen
    BOOL                bUseTarget;  // aTarget wurde von Applikation gesetzt
    USHORT              nCallMode;   // Synch/Asynch/API/Record
    BOOL                bAllowRecording;
    SfxAllItemSet*      pInternalArgs;
    SfxViewFrame*       pViewFrame;

    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;

                        SfxRequest_Impl( SfxRequest *pOwner )
                        : pAnti( pOwner), bCancelled(FALSE),
                          nCallMode( SFX_CALLMODE_SYNCHRON ), nModifier(0),
                          pPool(0), pInternalArgs( 0 ), bAllowRecording( FALSE ), pViewFrame(0)
                        {}
    ~SfxRequest_Impl() { delete pInternalArgs; }


    void                SetPool( SfxItemPool *pNewPool );
    virtual void        Notify( SfxBroadcaster &rBC, const SfxHint &rHint );
    void                Record( const uno::Sequence < beans::PropertyValue >& rArgs );
};


//====================================================================

void SfxRequest_Impl::Notify( SfxBroadcaster &rBC, const SfxHint &rHint )
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
:   nSlot(rOrig.nSlot),
    pArgs(rOrig.pArgs? new SfxAllItemSet(*rOrig.pArgs): 0),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

    pImp->bAllowRecording = rOrig.pImp->bAllowRecording;
    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = rOrig.pImp->nCallMode;
    pImp->bUseTarget = rOrig.pImp->bUseTarget;
    pImp->aTarget = rOrig.pImp->aTarget;
    pImp->nModifier = rOrig.pImp->nModifier;

    if ( pArgs )
        pImp->SetPool( pArgs->GetPool() );
    else
        pImp->SetPool( rOrig.pImp->pPool );
}
//--------------------------------------------------------------------


SfxRequest::SfxRequest
(
    SfxViewFrame*   pViewFrame,
    USHORT          nSlotId

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

    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->SetPool( &pViewFrame->GetPool() );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = SFX_CALLMODE_SYNCHRON;
    pImp->bUseTarget = FALSE;
    pImp->pViewFrame = pViewFrame;
    if( pImp->pViewFrame->GetDispatcher()->GetShellAndSlot_Impl( nSlotId, &pImp->pShell, &pImp->pSlot, TRUE, TRUE ) )
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
    USHORT          nSlotId,    // auszuf"uhrende <Slot-Id>
    SfxCallMode     nMode,      // Synch/API/...
    SfxItemPool&    rPool       // ggf. f"ur das SfxItemSet f"ur Parameter
)

// creates a SfxRequest without arguments

:   nSlot(nSlotId),
    pArgs(0),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->SetPool( &rPool );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = FALSE;
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

    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->SetPool( &rPool );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = FALSE;
    TransformParameters( nSlot, rArgs, *pArgs, pSlot );
}

//-----------------------------------------------------------------------

SfxRequest::SfxRequest
(
    USHORT                  nSlotId,
    USHORT                  nMode,
    const SfxAllItemSet&    rSfxArgs
)

// creates a SfxRequest with arguments

:   nSlot(nSlotId),
    pArgs(new SfxAllItemSet(rSfxArgs)),
    pImp( new SfxRequest_Impl(this) )
{
    DBG_MEMTEST();

    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->SetPool( rSfxArgs.GetPool() );
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = nMode;
    pImp->bUseTarget = FALSE;
}
//--------------------------------------------------------------------

USHORT SfxRequest::GetCallMode() const
{
    return pImp->nCallMode;
}

//--------------------------------------------------------------------

BOOL SfxRequest::IsSynchronCall() const
{
    return SFX_CALLMODE_SYNCHRON == ( SFX_CALLMODE_SYNCHRON & pImp->nCallMode );
}

//--------------------------------------------------------------------

void SfxRequest::SetSynchronCall( BOOL bSynchron )
{
    if ( bSynchron )
        pImp->nCallMode |= SFX_CALLMODE_SYNCHRON;
    else
        pImp->nCallMode &= ~(USHORT) SFX_CALLMODE_SYNCHRON;
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
                xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")),
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

void SfxRequest::RemoveItem( USHORT nID )
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
    USHORT          nSlotId,    // Slot-Id oder Which-Id des Parameters
    FASTBOOL        bDeep,      // FALSE: nicht in Parent-ItemSets suchen
    TypeId          aType       // != 0:  RTTI Pruefung mit Assertion
)   const
{
    return GetItem( pArgs, nSlotId, bDeep, aType );
}


//--------------------------------------------------------------------
const SfxPoolItem* SfxRequest::GetItem
(
    const SfxItemSet* pArgs,
    USHORT          nSlotId,    // Slot-Id oder Which-Id des Parameters
    FASTBOOL        bDeep,      // FALSE: nicht in Parent-ItemSets suchen
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
                    rReq.GetArg( SID_POS, FALSE, TYPE(SfxInt32Item) );
                USHORT nPos = pPosItem ? pPosItem->GetValue() : 0;

                // ein Beispiel mit Verwendung des Makros
                SFX_REQUEST_ARG(rReq, pSizeItem, SfxInt32Item, SID_SIZE, FALSE);
                USHORT nSize = pSizeItem ? pPosItem->GetValue() : 0;

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
        USHORT nWhich = pArgs->GetPool()->GetWhich(nSlotId);

        // ist das Item gesetzt oder bei bDeep==TRUE verf"ugbar?
        const SfxPoolItem *pItem = 0;
        if ( ( bDeep ? SFX_ITEM_AVAILABLE : SFX_ITEM_SET )
             <= pArgs->GetItemState( nWhich, bDeep, &pItem ) )
        {
            // stimmt der Typ "uberein?
            if ( !pItem || pItem->IsA(aType) )
                return pItem;

            // Item da aber falsch => Programmierfehler
            DBG_ERROR(  "invalid argument type" );
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

    FASTBOOL            bKeep   /*  TRUE (default)
                                    'rSet' wird gepeichert und ist "uber
                                    GetArgs() abfragbar

                                    FALSE
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


void SfxRequest::Done( BOOL bRelease )
//  [<SfxRequest::Done(SfxItemSet&)>]
{
    Done_Impl( pArgs );
    if( bRelease )
        DELETEZ( pArgs );
}

//--------------------------------------------------------------------

BOOL SfxRequest::IsCancelled() const
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
    pImp->bCancelled = TRUE;
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
    pImp->bIgnored = TRUE;
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
    pImp->bDone = TRUE;

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
        USHORT nWhich = rPool.GetWhich(pImp->pSlot->GetSlotId());
        SfxItemState eState = pSet ? pSet->GetItemState( nWhich, FALSE, &pItem ) : SFX_ITEM_UNKNOWN;
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
                USHORT nWhich = rPool.GetWhich( pItem->Which() );
                USHORT nSlotId = rPool.GetSlotId( pItem->Which() );
                if ( nSlotId == nSlot )
                {
                    // mit Hosentr"ager und G"urtel reparieren des falschen Flags
                    DBG_ERROR( "recursion RecordPerItem - use RecordPerSet!" );
                    SfxSlot *pSlot = (SfxSlot*) pImp->pSlot;
                    pSlot->nFlags &= ~((ULONG)SFX_SLOT_RECORDPERITEM);
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

BOOL SfxRequest::IsDone() const

/*  [Beschreibung]

    Mit dieser Methode kann abgefragt werden, ob der SfxRequest tats"achlich
    ausgef"uhrt wurde oder nicht. Wurde ein SfxRequest nicht ausgef"uhrt,
    liegt dies z.B. daran, da\s der Benutzer abgebrochen hat oder
    der Kontext f"ur diesen Request falsch war, dieses aber nicht "uber
    eine separate <SfxShell> realisiert wurde.

    SfxRequest-Instanzen, die hier FALSE liefern, werden nicht recorded.


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
        (pView ? pView : SfxViewFrame::Current())->GetFrame()->GetFrameInterface(),
        com::sun::star::uno::UNO_QUERY);

    if(xSet.is())
    {
        com::sun::star::uno::Any aProp = xSet->getPropertyValue(rtl::OUString::createFromAscii("DispatchRecorderSupplier"));
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
        aProp >>= xSupplier;
        if(xSupplier.is())
            xRecorder = xSupplier->getDispatchRecorder();
    }

    return xRecorder;
}

BOOL SfxRequest::HasMacroRecorder( SfxViewFrame* pView )
{
    return GetMacroRecorder( pView ).is();
}


//--------------------------------------------------------------------

BOOL SfxRequest::IsAPI() const

/*  [Beschreibung]

    Liefert TRUE, wenn dieser SfxRequest von einer API (z.B. BASIC)
    erzeugt wurde, sonst FALSE.
*/

{
    return SFX_CALLMODE_API == ( SFX_CALLMODE_API & pImp->nCallMode );
}

//--------------------------------------------------------------------


FASTBOOL SfxRequest::IsRecording() const

/*  [Beschreibung]

    Liefert TRUE, wenn dieser SfxRequest recorded werden soll, d.h.
    1. zu Zeit ein Makro aufgezeichnet wird
    2. dieser Request "uberhaupt aufgezeichnet wird
    3. der Request nicht von reiner API (z.B. BASIC) ausgeht,
    sonst FALSE.
*/

{
    return ( AllowsRecording() && GetMacroRecorder().is() );
}

//--------------------------------------------------------------------
void SfxRequest::SetModifier( USHORT nModi )
{
    pImp->nModifier = nModi;
}

//--------------------------------------------------------------------
USHORT SfxRequest::GetModifier() const
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
    pImp->bUseTarget = TRUE;
}

void SfxRequest::AllowRecording( BOOL bSet )
{
    pImp->bAllowRecording = bSet;
}

BOOL SfxRequest::AllowsRecording() const
{
    BOOL bAllow = pImp->bAllowRecording;
    if( !bAllow )
        bAllow = ( SFX_CALLMODE_API != ( SFX_CALLMODE_API & pImp->nCallMode ) ) &&
                 ( SFX_CALLMODE_RECORD == ( SFX_CALLMODE_RECORD & pImp->nCallMode ) );
    return bAllow;
}
