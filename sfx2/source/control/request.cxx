/*************************************************************************
 *
 *  $RCSfile: request.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-04 17:35:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif

#ifndef _SVTOOLS_ITEMDEL_HXX
#include <svtools/itemdel.hxx>
#endif

#include <svtools/itempool.hxx>

#pragma hdrstop

#include "request.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "msg.hxx"
#include "objsh.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "app.hxx"
#include "objface.hxx"

//===================================================================

struct SfxRequest_Impl: public SfxListener

/*  [Beschreibung]

    Implementations-Struktur der Klasse <SfxRequest>.
*/

{
    SfxRequest*         pAnti;       // Owner wegen sterbendem Pool
    SfxItemPool*        pPool;       // ItemSet mit diesem Pool bauen
    SfxPoolItem*        pRetVal;     // R"uckgabewert geh"ort sich selbst
    const SfxShell*     pShell;      // ausgef"uhrt an dieser Shell
    const SfxSlot*      pSlot;       // ausgef"uhrter Slot
    USHORT              nModifier;   // welche Modifier waren gedrueckt?
    BOOL                bDone;       // "uberhaupt ausgef"uhrt
    BOOL                bIgnored;    // vom User abgebrochen
    BOOL                bCancelled;  // nicht mehr zustellen
    USHORT              nCallMode;   // Synch/Asynch/API/Record
    SfxAllItemSet*      pInternalArgs;

                        SfxRequest_Impl( SfxRequest *pOwner )
                        : pAnti( pOwner), bCancelled(FALSE),
                          nCallMode( SFX_CALLMODE_SYNCHRON ), nModifier(0),
                          pPool(0), pInternalArgs( 0 )
                        {}
    ~SfxRequest_Impl() { delete pInternalArgs; }


    void                SetPool( SfxItemPool *pNewPool );
    virtual void        Notify( SfxBroadcaster &rBC, const SfxHint &rHint );
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

    pImp->bDone = FALSE;
    pImp->bIgnored = FALSE;
    pImp->pRetVal = 0;
    pImp->pShell = 0;
    pImp->pSlot = 0;
    pImp->nCallMode = rOrig.pImp->nCallMode;
    pImp->nModifier = rOrig.pImp->nModifier;

    if ( pArgs )
        pImp->SetPool( pArgs->GetPool() );
    else
        pImp->SetPool( rOrig.pImp->pPool );
}

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
    pImp->bDone = TRUE;

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
    pImp->bDone = TRUE;
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

BOOL SfxRequest::IsAPI() const

/*  [Beschreibung]

    Liefert TRUE, wenn dieser SfxRequest von einer API (z.B. BASIC)
    erzeugt wurde, sonst FALSE.
*/

{
    return SFX_CALLMODE_API == ( SFX_CALLMODE_API & pImp->nCallMode );
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


