/*************************************************************************
 *
 *  $RCSfile: bindings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-20 17:10:51 $
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

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#include <unotools/processfactory.hxx>
#include <svtools/itemdel.hxx>

#pragma hdrstop

#include "ipfrm.hxx"
#include "ipenv.hxx"
#include "interno.hxx"
#include "bindings.hxx"
#include "msg.hxx"
#include "statcach.hxx"
#include "ctrlitem.hxx"
#include "app.hxx"
#include "appdata.hxx"
#include "dispatch.hxx"
#include "request.hxx"
#include "objface.hxx"
#include "sfxtypes.hxx"
#include "workwin.hxx"
#include "macrconf.hxx"
#include "unoctitm.hxx"
#include "sfx.hrc"
#include "sfxuno.hxx"
#include "topfrm.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

DBG_NAME(SfxBindingsMsgPos);
DBG_NAME(SfxBindingsUpdateServers);
DBG_NAME(SfxBindingsCreateSet);
DBG_NAME(SfxBindingsUpdateCtrl1);
DBG_NAME(SfxBindingsUpdateCtrl2);
DBG_NAME(SfxBindingsNextJob_Impl0);
DBG_NAME(SfxBindingsNextJob_Impl);
DBG_NAME(SfxBindingsUpdate_Impl);
DBG_NAME(SfxBindingsInvalidateAll);

//====================================================================

#define TIMEOUT_FIRST        20
#define TIMEOUT_UPDATING      0
#define TIMEOUT_IDLE       2500

static sal_uInt32 nCache1 = 0;
static sal_uInt32 nCache2 = 0;

//====================================================================

DECL_PTRARRAY(SfxStateCacheArr_Impl, SfxStateCache*, 32, 16);

//====================================================================

class SfxBindings_Impl

/*  [Beschreibung]

    Diese Implementations-Struktur der Klasse SfxBindings dient
    der Entkopplung von "Anderungen vom exportierten Interface sowie
    der Verringerung von extern sichtbaren Symbolen.

    Eine Instanz exisitiert pro SfxBindings-Instanz f"ur deren Laufzeit.
*/

{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv;
    SfxUnoControllerArr_Impl*
                            pUnoCtrlArr;
    SfxWorkWindow*          pWorkWin;
    SfxBindings*            pSubBindings;
    SfxBindings*            pSuperBindings;
    SfxStateCacheArr_Impl*  pCaches;        // je ein cache fuer jede gebundene
    sal_uInt16                  nCachedFunc1;   // index der zuletzt gerufenen
    sal_uInt16                  nCachedFunc2;   // index der vorletzt gerufenen
    sal_uInt16                  nMsgPos;        // Message-Position, ab der zu aktualisieren ist
    SfxPopupAction          ePopupAction;   // in DeleteFloatinWindow() abgefragt
    sal_Bool                    bMsgDirty;      // wurde ein MessageServer invalidiert?
    sal_Bool                    bAllMsgDirty;   // wurden die MessageServer invalidiert?
    sal_Bool                    bAllDirty;      // nach InvalidateAll
    sal_Bool                    bCtrlReleased;  // waehrend EnterRegistrations
    AutoTimer               aTimer;         // fuer volatile Slots
    sal_Bool                    bInUpdate;      // fuer Assertions
    sal_Bool                    bInNextJob;     // fuer Assertions
    sal_Bool                    bFirstRound;    // Erste Runde im Update
    sal_uInt16                  nFirstShell;    // Shell, die in erster Runde bevorzugt wird
    sal_uInt16                  nOwnRegLevel;   // z"ahlt die echten Locks, ohne die der SuperBindings
};

//--------------------------------------------------------------------

struct SfxFoundCache_Impl

/*  [Beschreibung]

    In Instanzen dieser Struktur werden in <SfxBindings::CreateSet_Impl()>
    weitere Informationen zu den gemeinsam von einem <Slot-Server> zu
    erfragenden Status gesammelt, deren Ids dort in die Ranges eines
    <SfxItemSet>s aufgenommen werden.

    Diese Informationen werden w"ahrend der Suche nach den zusammen
    upzudatenden Ids sowieso als Zwischenergebnis ermittelt und nachher
    wieder ben"otigt, daher macht es Sinn, sie f"ur diesen kurzen Zeitraum
    gleich aufzubewahren.
*/

{
    sal_uInt16          nSlotId;    // die Slot-Id
    sal_uInt16          nWhichId;   // falls verf"ugbar die Which-Id, sonst nSlotId
    const SfxSlot*  pSlot;      // Pointer auf den <Master-Slot>
    SfxStateCache*  pCache;     // Pointer auf den StatusCache, ggf. 0

    SfxFoundCache_Impl():
        nSlotId(0),
        nWhichId(0),
        pSlot(0),
        pCache(0)
    {}

    SfxFoundCache_Impl(SfxFoundCache_Impl&r):
        nSlotId(r.nSlotId),
        nWhichId(r.nWhichId),
        pSlot(r.pSlot),
        pCache(r.pCache)
    {}

    SfxFoundCache_Impl(sal_uInt16 nS, sal_uInt16 nW, const SfxSlot *pS, SfxStateCache *pC ):
        nSlotId(nS),
        nWhichId(nW),
        pSlot(pS),
        pCache(pC)
    {}

    int operator<( const SfxFoundCache_Impl &r ) const
    { return nWhichId < r.nWhichId; } const

    int operator==( const SfxFoundCache_Impl &r )
    { return nWhichId== r.nWhichId; }
};

//--------------------------------------------------------------------------

SV_DECL_PTRARR_SORT_DEL(SfxFoundCacheArr_Impl, SfxFoundCache_Impl*, 16, 16 );
SV_IMPL_OP_PTRARR_SORT(SfxFoundCacheArr_Impl, SfxFoundCache_Impl*);

//==========================================================================

SfxBindings::SfxBindings()

/*  [Beschreibung]

    Konstruktor der Klasse SfxBindings. Genau eine Instanz wird automatisch
    von der <SfxApplication> vor <SfxApplication::Init()> angelegt. Wird
    eine Instanz ben"otigt, z.B. zum Invalidieren von Slots, sollte diese
    "uber den zugeh"origen <SfxViewFrame> besorgt werden. Bestimmte
    SfxViewFrame Subklassen (z.B. <SfxInPlaceFrame>) legen ihre eigene
    Instanz der SfxBindings an.

    <SfxControllerItem> Instanzen k"onnen erst angelegt werden, wenn
    die zugeh"orige SfxBindings Instanz existiert.
*/

:   pImp(new SfxBindings_Impl),
    pDispatcher(0),
    nRegLevel(1)    // geht erst auf 0, wenn Dispatcher gesetzt
{
    pImp->nMsgPos = 0;
    pImp->bAllMsgDirty = sal_True;
    pImp->bMsgDirty = sal_True;
    pImp->bAllDirty = sal_True;
    pImp->ePopupAction = SFX_POPUP_DELETE;
    pImp->nCachedFunc1 = 0;
    pImp->nCachedFunc2 = 0;
    pImp->bCtrlReleased = sal_False;
    pImp->bFirstRound = sal_False;
    pImp->bInNextJob = sal_False;
    pImp->bInUpdate = sal_False;
    pImp->pSubBindings = NULL;
    pImp->pSuperBindings = NULL;
    pImp->pWorkWin = NULL;
    pImp->pUnoCtrlArr = NULL;
    pImp->nOwnRegLevel = nRegLevel;

    // all caches are valid (no pending invalidate-job)
    // create the list of caches
    pImp->pCaches = new SfxStateCacheArr_Impl;
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxBindings, NextJob_Impl) );
}

//====================================================================

SfxBindings::~SfxBindings()

/*  [Beschreibung]

    Destruktor der Klasse SfxBindings. Die eine, f"ur jede <SfxApplication>
    existierende Instanz wird von der <SfxApplication> nach Ausf"urhung
    von <SfxApplication::Exit()> automatisch zerst"ort.

    Noch existierende <SfxControllerItem> Instanzen, die bei dieser
    SfxBindings Instanz angemeldet sind, werden im Destruktor
    automatisch zerst"ort. Dies sind i.d.R. Floating-Toolboxen, Value-Sets
    etc. Arrays von SfxControllerItems d"urfen zu diesem Zeitpunkt nicht
    mehr exisitieren.
*/

{
    // Die SubBindings sollen ja nicht gelocked werden !
    pImp->pSubBindings = NULL;

    ENTERREGISTRATIONS();

    pImp->aTimer.Stop();
    DeleteControllers_Impl();

    // Caches selbst l"oschen
    sal_uInt16 nCount = pImp->pCaches->Count();
    for ( sal_uInt16 nCache = 0; nCache < nCount; ++nCache )
        delete pImp->pCaches->GetObject(nCache);

    delete pImp->pCaches;
    delete pImp;
}

//--------------------------------------------------------------------

void SfxBindings::DeleteControllers_Impl()

/*  [Beschreibung]

    Interne Methode zum l"oschen noch existierender <SfxControllerItem>
    Instanzen, die bei dieser SfxBindings Instanz angemeldet sind.

    Dies sind i.d.R. <SfxPopupWindow>s. Nich sich selbst geh"orende
    SfxControllerItems d"urfen bei Aufruf nur noch existieren, wenn sie
    einem der restlichen SfxPopupWindows geh"oren.


    [Anmerkung]

    Wird beim Beenden der Applikation gerufen, bevor das Applikations-
    Fenster gel"oscht wird.
*/

{
    // in der ersten Runde den SfxPopupWindows l"oschen
    sal_uInt16 nCount = pImp->pCaches->Count();
    sal_uInt16 nCache;
    for ( nCache = 0; nCache < nCount; ++nCache )
    {
        // merken wo man ist
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache);
        sal_uInt16 nSlotId = pCache->GetId();

        // SfxPopupWindow l"oschen lassen
        pCache->DeleteFloatingWindows();

        // da der Cache verkleinert worden sein kann, wiederaufsetzen
        sal_uInt16 nNewCount = pImp->pCaches->Count();
        if ( nNewCount < nCount )
        {
            nCache = GetSlotPos(nSlotId);
            if ( nCache >= nNewCount ||
                 nSlotId != pImp->pCaches->GetObject(nCache)->GetId() )
                --nCache;
            nCount = nNewCount;
        }
    }

    // alle Caches l"oschen
    for ( nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
    {
        // Cache via ::com::sun::star::sdbcx::Index besorgen
        SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

        // alle Controller in dem Cache unbinden
        SfxControllerItem *pNext;
        for ( SfxControllerItem *pCtrl = pCache->GetItemLink();
              pCtrl; pCtrl = pNext )
        {
            pNext = pCtrl->GetItemLink();
            pCtrl->UnBind();
        }

        // Cache l"oschen
        if( nCache-1 < pImp->pCaches->Count() )
            delete (*pImp->pCaches)[nCache-1];
        pImp->pCaches->Remove(nCache-1, 1);
    }

    if( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nCount = pImp->pUnoCtrlArr->Count();
        for ( sal_uInt16 n=nCount; n>0; n-- )
        {
            SfxUnoControllerItem *pCtrl = (*pImp->pUnoCtrlArr)[n-1];
            pCtrl->ReleaseBindings();
        }

        DBG_ASSERT( !pImp->pUnoCtrlArr->Count(), "UnoControllerItems nicht entfernt!" );
        DELETEZ( pImp->pUnoCtrlArr );
    }
}

//--------------------------------------------------------------------

SfxPopupAction SfxBindings::GetPopupAction_Impl() const
{
    return pImp->ePopupAction;
}


//--------------------------------------------------------------------

void SfxBindings::HidePopups( FASTBOOL bHide )

/*  [Beschreibung]

    Dieser Methode versteckt und zeigt die <SfxPopupWindows>, die aus
    <SfxToolboxControl>s dieser SfxBindings-Instanz abgerissen wurden bzw.
    floating <SfxChildWindow>-Instanzen dieser SfxBindings-Instanz.


    [Anmerkung]

    Es k"onnten noch weitere Floating-Windows exisitieren, die durch
    diese Methode nicht erfa\st werden.
*/

{
    // SfxPopupWindows hiden
    HidePopupCtrls_Impl( bHide );
    SfxBindings *pSub = pImp->pSubBindings;
    while ( pSub )
    {
        pImp->pSubBindings->HidePopupCtrls_Impl( bHide );
        pSub = pSub->pImp->pSubBindings;
    }

    // SfxChildWindows hiden
    DBG_ASSERT( pDispatcher, "HidePopups not allowed without dispatcher" );
    if ( pImp->pWorkWin )
        pImp->pWorkWin->HidePopups_Impl( bHide, sal_True );
}

void SfxBindings::HidePopupCtrls_Impl( FASTBOOL bHide )
{
    if ( bHide )
    {
        // SfxPopupWindows hiden
        pImp->ePopupAction = SFX_POPUP_HIDE;
    }
    else
    {
        // SfxPopupWindows showen
        pImp->ePopupAction = SFX_POPUP_SHOW;
    }

    for ( sal_uInt16 nCache = 0; nCache < pImp->pCaches->Count(); ++nCache )
        pImp->pCaches->GetObject(nCache)->DeleteFloatingWindows();
    pImp->ePopupAction = SFX_POPUP_DELETE;
}

//--------------------------------------------------------------------

void SfxBindings::Update_Impl
(
    SfxStateCache*  pCache      // der upzudatende SfxStatusCache
)

/*  [Beschreibung]

    Interne Methode zum Updaten eines Caches und den von derselben
    Status-Methode in derselben Shell bedienten und dirty Slots.
*/

{
    if( pCache->GetDispatch().is() && pCache->GetItemLink() )
    {
        pCache->SetCachedState();
        return;
    }

    if ( !pDispatcher )
        return;
    DBG_PROFSTART(SfxBindingsUpdate_Impl);

    // alle mit derselben Statusmethode zusammensammeln, die dirty sind
    SfxDispatcher &rDispat = *pDispatcher;
    const SfxSlot *pRealSlot = 0;
    const SfxSlotServer* pMsgServer = 0;
    SfxFoundCacheArr_Impl aFound;
    SfxItemSet *pSet = CreateSet_Impl( pCache, pRealSlot, &pMsgServer, aFound );
    sal_Bool bUpdated = sal_False;
    if ( pSet )
    {
        // Status erfragen
        if ( rDispat._FillState( *pMsgServer, *pSet, pRealSlot ) )
        {
            // Status posten
            const SfxInterface *pInterface =
                rDispat.GetShell(pMsgServer->GetShellLevel())->GetInterface();
            for ( sal_uInt16 nPos = 0; nPos < aFound.Count(); ++nPos )
            {
                const SfxFoundCache_Impl *pFound = aFound[nPos];
                sal_uInt16 nWhich = pFound->nWhichId;
                const SfxPoolItem *pItem = 0;
                SfxItemState eState = pSet->GetItemState(nWhich, sal_True, &pItem);
                if ( eState == SFX_ITEM_DEFAULT && SfxItemPool::IsWhich(nWhich) )
                    pItem = &pSet->Get(nWhich);
                UpdateControllers_Impl( pInterface, aFound[nPos], pItem, eState );
            }
            bUpdated = sal_True;
        }

        delete pSet;
    }

    if ( !bUpdated && pCache )
    {
        // Wenn pCache == NULL und kein SlotServer ( z.B. weil Dispatcher gelockt! ),
        // darf nat"urlich kein Update versucht werden
        SfxFoundCache_Impl aFound( pCache->GetId(), 0,
                                   pRealSlot, pCache );
        UpdateControllers_Impl( 0, &aFound, 0, SFX_ITEM_DISABLED);
    }

    DBG_PROFSTOP(SfxBindingsUpdate_Impl);
}

//--------------------------------------------------------------------

void SfxBindings::Update
(
    sal_uInt16      nId     // die gebundene und upzudatende Slot-Id
)

/*  [Beschreibung]

    Diese Methode sorgt f"ur synchrones Updaten der auf die Slot-Id nId
    gebundenen <SfxContollerItem> Instanzen, die an dieser SfxBindings
    Instanz angemeldet sind. Vergleichbar zu Window::Update()
    (StarView) erfolgt ein Update nur, wenn entweder ein auf diese
    Slot-Id gebundenes SfxContollerItem dirty ist, oder die Slot-Id
    selbst dirty ist. Dies kann durch einen vorhergehendes Aufruf von
    <SfxBindings::Invalidate(sal_uInt16)> erzwungen werden.


    [Anmerkung]

    Es ist g"unstiger, zun"achst alle zu invalidierenden Slot-Ids per
    <SfxBindings::Invalidate(sal_uInt16)> zu invalidieren und dann
    Update() aufzurufen, als einzelne abwechselnde Invalidate/Update,
    da von derselben Status-Methode bediente Status-Anfragen von
    den SfxBindings automatisch zusammengefa"st werden.


    [Querverweise]

    <SfxShell::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::InvalidateAll(sal_Bool)>
    <SfxBindings::Update()>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

//!!TLX: Fuehrte zu Vorlagenkatalogstillstand
//  if ( nRegLevel )
//      return;

    if ( pDispatcher )
        pDispatcher->Flush();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update( nId );

    SfxStateCache* pCache = GetStateCache( nId );
    if ( pCache )
    {
        pImp->bInUpdate = sal_True;
        if ( pImp->bMsgDirty )
        {
            UpdateSlotServer_Impl();
            pCache = GetStateCache( nId );
        }

        if (pCache)
        {
            if( pCache->GetDispatch().is() && pCache->GetItemLink() )
            {
                pCache->SetCachedState();
            }
            else
            {
                // Status erfragen
                const SfxSlotServer* pMsgServer = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
                if ( !pCache->IsControllerDirty() &&
                    ( !pMsgServer ||
                    !pMsgServer->GetSlot()->IsMode(SFX_SLOT_VOLATILE) ) )
                {
                    pImp->bInUpdate = sal_False;
                    return;
                }
                if (!pMsgServer)
                {
                    pCache->SetState(SFX_ITEM_DISABLED, 0);
                    pImp->bInUpdate = sal_False;
                    return;
                }

                Update_Impl(pCache);
            }

            pImp->bAllDirty = sal_False;
        }

        pImp->bInUpdate = sal_False;
    }
}

//--------------------------------------------------------------------

void SfxBindings::Update()

/*  [Beschreibung]

    Diese Methode sorgt f"ur synchrones Updaten aller <SfxContollerItem>
    Instanzen, die an dieser SfxBindings Instanz angemeldet sind. Vergleichbar
    zu Window::Update() (StarView) erfolgt ein Update nur, wenn entweder ein
    SfxContollerItem dirty ist, in einem Status-Cache der Zeiger auf den
    <Slot-Server> dirty ist. Ersteres kann durch einen Aufruf von
    <SfxBindings::Invalidate(sal_uInt16)> erzwungen werden, letzters durch
    <SfxBindings::InvalidateAll(sal_Bool)>.


    [Querverweise]

    <SfxShell::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::InvalidateAll(sal_Bool)>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Update();

    if ( pDispatcher )
    {
        if ( nRegLevel )
            return;

        pImp->bInUpdate = sal_True;
        pDispatcher->Flush();
        pDispatcher->Update_Impl();
        while ( !NextJob_Impl(0) )
            ; // loop
        pImp->bInUpdate = sal_False;
    }
}

//--------------------------------------------------------------------

void SfxBindings::SetState
(
    const SfxItemSet&   rSet    // zu setzende Status-Werte
)

/*  [Beschreibung]

    Diese Methode erlaubt das direkte Setzen neuer Status-Werte, ohne
    den Umweg "uber <SfxBindings::Invalidate()> und das dann im Update
    erfolgende Rufen der Status-Methoden an den <SfxShell>s.


    [Querverweise]

    <SfxBindings::SetState(const SfxPoolItem&)>
*/

{
    // wenn gelockt, dann nur invalidieren
    if ( nRegLevel )
    {
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
            Invalidate( pItem->Which() );
    }
    else
    {
        // Status d"urfen nur angenommen werden, wenn alle Slot-Pointer gesetzt sind
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        // "uber das ItemSet iterieren, falls Slot gebunden, updaten
        //! Bug: WhichIter verwenden und ggf. VoidItems hochschicken
        SfxItemIter aIter(rSet);
        for ( const SfxPoolItem *pItem = aIter.FirstItem();
              pItem;
              pItem = aIter.NextItem() )
        {
            SfxStateCache* pCache =
                    GetStateCache( rSet.GetPool()->GetSlotId(pItem->Which()) );
            if ( pCache )
            {
                // Status updaten
                if ( !pCache->IsControllerDirty() )
                    pCache->Invalidate(sal_False);
                pCache->SetState( SFX_ITEM_AVAILABLE, pItem );

                //! nicht implementiert: Updates von EnumSlots via MasterSlots
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxBindings::SetState
(
    const SfxPoolItem&  rItem   // zu setzender Status-Wert
)

/*  [Beschreibung]

    Diese Methode erlaubt das direkte Setzen eines neuen Status-Wertes,
    ohne den Umweg "uber <SfxBindings::Invalidate()> und das dann im Update
    erfolgende Rufen der Status-Methoden an den <SfxShell>s.

    Mit dieser Methode k"onnen nur <SfxPoolItem>s mit Slot, nicht
    aber mit Which-Id gesetzt werden, da kein <SfxItemPool> bekannt ist,
    "uber den gemappt werden k"onnte.


    [Querverweise]

    <SfxBindings::SetState(const SfxItemSet&)>
*/

{
    if ( nRegLevel )
    {
        Invalidate( rItem.Which() );
    }
    else
    {
        // Status d"urfen nur angenommen werden, wenn alle Slot-Pointer gesetzt sind
        if ( pImp->bMsgDirty )
            UpdateSlotServer_Impl();

        // falls der Slot gebunden ist, updaten
        DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                    "cannot set items with which-id" );
        SfxStateCache* pCache = GetStateCache( rItem.Which() );
        if ( pCache )
        {
            // Status updaten
            if ( !pCache->IsControllerDirty() )
                pCache->Invalidate(sal_False);
            pCache->SetState( SFX_ITEM_AVAILABLE, &rItem );

            //! nicht implementiert: Updates von EnumSlots via MasterSlots
        }
    }
}


//--------------------------------------------------------------------

SfxStateCache* SfxBindings::GetAnyStateCache_Impl( sal_uInt16 nId )
{
    SfxStateCache* pCache = GetStateCache( nId );
    if ( !pCache && pImp->pSubBindings )
        return pImp->pSubBindings->GetAnyStateCache_Impl( nId );
    return pCache;
}

SfxStateCache* SfxBindings::GetStateCache
(
    sal_uInt16      nId         /*  Slot-Id, deren SfxStatusCache gefunden
                                werden soll */,
    sal_uInt16*     pPos        /*  0 bzw. Position, ab der die Bindings
                                bin"ar durchsucht werden sollen. Liefert
                                die Position zur"uck, an der nId gefunden
                                wurde, bzw. an der es einfef"ugt werden
                                w"urde. */
)

/*  [Beschreibung]

    Diese Methode sucht die zu einer Slot-Id geh"orige <SfxStatusCache>
    Instanz. Falls die Slot-Id in keinem Controller gebunden ist, wird
    ein 0-Pointer zur"uckgegeben.

    Falls pPos != 0, wird erst ab der Position mit der Suche angefangen.
    Dieses ist eine Optimierung, f"ur den Fall, da"s die kleineren
    Ids bereits abgearbeitet wurden.

    In *pPos wird der ::com::sun::star::sdbcx::Index innerhalb der SfxBindings zur"uckgegeben,
    unter dem dieser Cache z.Zt. abgelegt ist. Dieser ::com::sun::star::sdbcx::Index ist bis zum
    n"achsten Aufruf von <SfxBindings::EnterRegistrations()> g"ultig.
    W"ahrend der Umkonfiguration (<SfxBindings::IsInRegistrations()> == sal_True)
    kann ist der ::com::sun::star::sdbcx::Index und der R"uckgabewert nur sehr kurzfristig
    g"ultig.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    // is the specified function bound?
    const sal_uInt16 nStart = ( pPos ? *pPos : 0 );
    const sal_uInt16 nPos = GetSlotPos( nId, nStart );

    if ( nPos < pImp->pCaches->Count() &&
         (*pImp->pCaches)[nPos]->GetId() == nId )
    {
        if ( pPos )
            *pPos = nPos;
        return (*pImp->pCaches)[nPos];
    }
    return 0;
}

//--------------------------------------------------------------------

void SfxBindings::InvalidateAll
(
    sal_Bool    bWithMsg        /*  sal_True
                                Slot-Server als ung"ultig markieren

                                sal_False
                                Slot-Server bleiben g"ultig */
)

/*  [Beschreibung]

    Invalidiert alle <SfxControllerItem> Instanzen, die an dieser
    SfxBindings Instanz angemeldet sind, und bei bWithMsg == sal_True
    ebenfalls die <Slot-Server>-Caches.

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.


    [Querverweise]

    <SfxShell::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16*)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    DBG_MEMTEST();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateAll( bWithMsg );

    // ist schon alles dirty gesetzt oder downing => nicht zu tun
    if ( !pDispatcher ||
         ( pImp->bAllDirty && ( !bWithMsg || pImp->bAllMsgDirty ) ) ||
         SFX_APP()->IsDowning() )
    {
        DBG_PROFSTOP(SfxBindingsInvalidateAll);
        return;
    }

    pImp->bAllMsgDirty = pImp->bAllMsgDirty || bWithMsg;
    pImp->bMsgDirty = pImp->bMsgDirty || pImp->bAllMsgDirty || bWithMsg;
    pImp->bAllDirty = sal_True;

    for ( sal_uInt16 n = 0; n < pImp->pCaches->Count(); ++n )
        pImp->pCaches->GetObject(n)->Invalidate(bWithMsg);
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
//      pImp->bFirstRound = sal_True;
//      pImp->nFirstShell = 0;
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    const sal_uInt16* pIds      /*  numerisch sortiertes 0-terminiertes Array
                                von Slot-Ids (einzel, nicht als Paare!) */
)

/*  [Beschreibung]

    Invalidiert die <SfxControllerItem> Instanzen der Slot-Ids in 'pIds',
    die an dieser SfxBindings Instanz angemeldet sind.

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.


    [Querverweise]

    <SfxShell::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::InvalidateAll(sal_uInt16)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    DBG_MEMTEST();

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( pIds );

    // ist schon alles dirty gesetzt oder downing => nicht zu tun
    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    // in immer kleiner werdenden Berichen bin"ar suchen
    for ( sal_uInt16 n = GetSlotPos(*pIds);
          *pIds && n < pImp->pCaches->Count();
          n = GetSlotPos(*pIds, n) )
    {
        // falls SID "uberhaupt gebunden ist, den Cache invalidieren
        SfxStateCache *pCache = pImp->pCaches->GetObject(n);
        if ( pCache->GetId() == *pIds )
            pCache->Invalidate(sal_False);

        // n"achste SID
        if ( !*++pIds )
            break;
        DBG_ASSERT( *pIds > *(pIds-1), "pIds unsorted" );
    }

    // falls nicht gelockt, Update-Timer starten
    pImp->nMsgPos = 0;
    if ( !nRegLevel )
    {
        pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
        pImp->aTimer.Start();
//      pImp->bFirstRound = sal_True;
//      pImp->nFirstShell = 0;
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::InvalidateShell
(
    const SfxShell&     rSh             /*  Die <SfxShell>, deren Slot-Ids
                                            invalidiert werden sollen. */,

    sal_Bool                bDeep           /*  sal_True
                                            auch die, von der SfxShell
                                            ererbten Slot-Ids werden invalidert

                                            sal_False
                                            die ererbten und nicht "uberladenen
                                            Slot-Ids werden invalidiert */
                                            //! MI: z. Zt. immer bDeep
)

/*  [Beschreibung]

    Invalidiert alle <SfxControllerItem> Instanzen, die zur Zeit von
    der angegebenen SfxShell Instanz bedient werden  und an dieser
    SfxBindings Instanz angemeldet sind

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.


    [Querverweise]

    <SfxShell::Invalidate(sal_uInt16)>
    <SfxBindings::Invalidate(sal_uInt16)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateShell( rSh, bDeep );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    DBG_PROFSTART(SfxBindingsInvalidateAll);
    DBG_MEMTEST();

    // Jetzt schon flushen, wird in GetShellLevel(rSh) sowieso gemacht; wichtig,
    // damit pImp->bAll(Msg)Dirty korrekt gesetzt ist
    pDispatcher->Flush();

    if ( !pDispatcher ||
         ( pImp->bAllDirty && pImp->bAllMsgDirty ) ||
         SFX_APP()->IsDowning() )
    {
        // Wenn sowieso demn"achst alle Server geholt werden
        return;
    }

    // Level finden
    sal_uInt16 nLevel = pDispatcher->GetShellLevel(rSh);
    if ( nLevel != USHRT_MAX )
    {
        for ( sal_uInt16 n = 0; n < pImp->pCaches->Count(); ++n )
        {
            SfxStateCache *pCache = pImp->pCaches->GetObject(n);
            const SfxSlotServer *pMsgServer =
                pCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pMsgServer && pMsgServer->GetShellLevel() == nLevel )
                pCache->Invalidate(sal_False);
        }
        pImp->nMsgPos = 0;
        if ( !nRegLevel )
        {
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
            pImp->bFirstRound = sal_True;
            pImp->nFirstShell = nLevel;
        }
    }

    DBG_PROFSTOP(SfxBindingsInvalidateAll);
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    sal_uInt16 nId              // zu invalidierende Slot-Id
)

/*  [Beschreibung]

    Invalidiert alle <SfxControllerItem> Instanzen, die auf die Slot-Id
    nId gebunden sind und an dieser SfxBindings Instanz angemeldet sind.

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.


    [Querverweise]
    <SfxBindings::Invalidate(sal_uInt16*)>
    <SfxBindings::InvalidateAll(sal_Bool)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        pCache->Invalidate(sal_False);
        pImp->nMsgPos = Min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}

//--------------------------------------------------------------------

void SfxBindings::Invalidate
(
    sal_uInt16  nId,                // zu invalidierende Slot-Id
    sal_Bool    bWithItem,          // StateCache clearen ?
    sal_Bool    bWithMsg            // SlotServer neu holen ?
)

/*  [Beschreibung]

    Invalidiert alle <SfxControllerItem> Instanzen, die auf die Slot-Id
    nId gebunden sind und an dieser SfxBindings Instanz angemeldet sind,
    und bei bWithMsg == sal_True ebenfalls den <Slot-Server>-Cache.

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.

    [Querverweise]
    <SfxBindings::Invalidate(sal_uInt16*)>
    <SfxBindings::InvalidateAll(sal_Bool)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId, bWithItem, bWithMsg );

    if ( SFX_APP()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        if ( bWithItem )
            pCache->ClearCache();
        pCache->Invalidate(bWithMsg);

        if ( !pDispatcher || pImp->bAllDirty )
            return;

        pImp->nMsgPos = Min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
}

void SfxBindings::Invalidate
(
    sal_uInt16  nId,                // zu invalidierende Slot-Id
    sal_Bool    bWithMsg            // SlotServer neu holen ?
)

/*  [Beschreibung]

    Invalidiert alle <SfxControllerItem> Instanzen, die auf die Slot-Id
    nId gebunden sind und an dieser SfxBindings Instanz angemeldet sind,
    und bei bWithMsg == sal_True ebenfalls den <Slot-Server>-Cache.

    Es wird daraufhin ein Timer gestartet, bei dessen Ablauf das Updaten
    beginnt. Somit k"onnen mehrere Invalidierungen vorgenommen werden,
    bevor "uberhaupt etwas passiert.

    [Querverweise]
    <SfxBindings::Invalidate(sal_uInt16*)>
    <SfxBindings::InvalidateAll(sal_Bool)>
    <SfxBindings::Update()>
    <SfxBindings::Update(sal_uInt16)>
*/

{
    DBG_ERROR( "Methode veraltet!" );
/*
    DBG_MEMTEST();
    DBG_ASSERT( !pImp->bInUpdate, "SfxBindings::Invalidate while in update" );
    DBG_ASSERT( sal_False, "alte Methode aus Kompatibilitaetsgruenden gerettet (MH)");

    if ( pImp->pSubBindings )
        pImp->pSubBindings->Invalidate( nId, bWithMsg );

    if ( !pDispatcher || pImp->bAllDirty || SFX_APP()->IsDowning() )
        return;

    SfxStateCache* pCache = GetStateCache(nId);
    if ( pCache )
    {
        pCache->Invalidate(bWithMsg);
        pImp->nMsgPos = Min(GetSlotPos(nId), pImp->nMsgPos);
        if ( !nRegLevel )
        {
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
        }
    }
*/
}

//--------------------------------------------------------------------

sal_Bool SfxBindings::IsBound( sal_uInt16 nSlotId, sal_uInt16 nStartSearchAt )

/*  [Beschreibung]

    Stellt fest, ob die angegebene Slot-Id in einem <SfxControllerItem>
    gebunden ist, der an dieser SfxBindings Instanz angemeldet ist.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Die angegeben Slot-Id ist gebunden.

                        sal_False
                        Die angegeben Slot-Id ist nicht gebunden.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    return GetStateCache(nSlotId, &nStartSearchAt ) != 0;
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::GetSlotPos( sal_uInt16 nId, sal_uInt16 nStartSearchAt )

/*  [Beschreibung]

    Ermittelt den ::com::sun::star::sdbcx::Index der angegebenen Slot-Id in den SfxBindings.
    Falls die Slot-Id nicht gebunden ist, wird der ::com::sun::star::sdbcx::Index zur"uckgegeben,
    an dem sie eingef"ugt w"urde.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    DBG_PROFSTART(SfxBindingsMsgPos);

    // answer immediately if a function-seek comes repeated
    if ( pImp->nCachedFunc1 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc1]->GetId() == nId )
    {
        ++nCache1;
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }
    if ( pImp->nCachedFunc2 < pImp->pCaches->Count() &&
         (*pImp->pCaches)[pImp->nCachedFunc2]->GetId() == nId )
    {
        ++nCache2;

        // swap the caches
        sal_uInt16 nTemp = pImp->nCachedFunc1;
        pImp->nCachedFunc1 = pImp->nCachedFunc2;
        pImp->nCachedFunc2 = nTemp;
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return pImp->nCachedFunc1;
    }

    // binary search, if not found, seek to target-position
    if ( pImp->pCaches->Count() <= nStartSearchAt )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return 0;
    }
    if ( pImp->pCaches->Count() == (nStartSearchAt+1) )
    {
        DBG_PROFSTOP(SfxBindingsMsgPos);
        return (*pImp->pCaches)[nStartSearchAt]->GetId() >= nId ? 0 : 1;
    }
    size_t nLow = nStartSearchAt, nMid, nHigh = 0;
    sal_Bool bFound = sal_False;
    nHigh = pImp->pCaches->Count() - 1;
    while ( !bFound && nLow <= nHigh )
    {
        nMid = (nLow + nHigh) >> 1;
        DBG_ASSERT( nMid < pImp->pCaches->Count(), "bsearch ist buggy" );
        int nDiff = (int) nId - (int) ( ((*pImp->pCaches)[nMid])->GetId() );
        if ( nDiff < 0)
        {   if ( nMid == 0 )
                break;
            nHigh = nMid - 1;
        }
        else if ( nDiff > 0 )
        {   nLow = nMid + 1;
            if ( nLow == 0 )
                break;
        }
        else
            bFound = sal_True;
    }
    sal_uInt16 nPos = bFound ? nMid : nLow;
    DBG_ASSERT( nPos <= pImp->pCaches->Count(), "" );
    DBG_ASSERT( nPos == pImp->pCaches->Count() ||
                nId <= (*pImp->pCaches)[nPos]->GetId(), "" );
    DBG_ASSERT( nPos == nStartSearchAt ||
                nId > (*pImp->pCaches)[nPos-1]->GetId(), "" );
    DBG_ASSERT( ( (nPos+1) >= pImp->pCaches->Count() ) ||
                nId < (*pImp->pCaches)[nPos+1]->GetId(), "" );
    pImp->nCachedFunc2 = pImp->nCachedFunc1;
    pImp->nCachedFunc1 = nPos;
    DBG_PROFSTOP(SfxBindingsMsgPos);
    return nPos;
}
//--------------------------------------------------------------------

void SfxBindings::Register( SfxControllerItem& rItem )

/*  [Beschreibung]

    Mit dieser Methode melden sich Instanzen der Klasse <SfxControllerItem>
    an der betreffenden SfxBindings Instanz an, wenn sie eine Slot-Id binden.


    [Querverweise]
    <SfxBindings::Release(SfxControllerItem&)>
    <SfxBindings::EnterRegistrations()>
    <SfxBindings::LeaveRegistrations()>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( nRegLevel > 0, "registration without EnterRegistrations" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Register while status-updating" );
    //! EnterRegistrations();

    // if not exist, insert a new cache
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    if ( nPos >= pImp->pCaches->Count() ||
         (*pImp->pCaches)[nPos]->GetId() != nId )
    {
        SfxStateCache* pCache = new SfxStateCache(nId);
        pImp->pCaches->Insert( nPos, pCache );
        DBG_ASSERT( nPos == 0 ||
                    (*pImp->pCaches)[nPos]->GetId() >
                        (*pImp->pCaches)[nPos-1]->GetId(), "" );
        DBG_ASSERT( (nPos == pImp->pCaches->Count()-1) ||
                    (*pImp->pCaches)[nPos]->GetId() <
                        (*pImp->pCaches)[nPos+1]->GetId(), "" );
        pImp->bMsgDirty = sal_True;
    }

    // enqueue the new binding
    SfxControllerItem *pOldItem = (*pImp->pCaches)[nPos]->ChangeItemLink(&rItem);
    rItem.ChangeItemLink(pOldItem);

    //! LeaveRegistrations();
}
//--------------------------------------------------------------------

void SfxBindings::Release( SfxControllerItem& rItem )

/*  [Beschreibung]

    Mit dieser Methode melden sich Instanzen der Klasse <SfxControllerItem>
    an den SfxBindings ab.


    [Querverweise]
    <SfxBindings::Register(SfxControllerItem&)>
    <SfxBindings::Release(SfxControllerItem&)>
    <SfxBindings::EnterRegistrations()>
    <SfxBindings::LeaveRegistrations()>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );
    //! DBG_ASSERT( nRegLevel > 0, "release without EnterRegistrations" );
    DBG_ASSERT( !pImp->bInNextJob, "SfxBindings::Release while status-updating" );
    ENTERREGISTRATIONS();

    // find the bound function
    sal_uInt16 nId = rItem.GetId();
    sal_uInt16 nPos = GetSlotPos(nId);
    SfxStateCache* pCache = (*pImp->pCaches)[nPos];
    if ( pCache->GetId() == nId )
    {
        // is this the first binding in the list?
        SfxControllerItem* pItem = pCache->GetItemLink();
        if ( pItem == &rItem )
            pCache->ChangeItemLink( rItem.GetItemLink() );
        else
        {
            // search the binding in the list
            while ( pItem && pItem->GetItemLink() != &rItem )
                pItem = pItem->GetItemLink();

            // unlink it if it was found
            if ( pItem )
                pItem->ChangeItemLink( rItem.GetItemLink() );
        }
        // was this the last controller?
        if ( pCache->GetItemLink() == 0 )
        {
#ifdef slow
            // remove the BoundFunc
            delete (*pImp->pCaches)[nPos];
            pImp->pCaches->Remove(nPos, 1);
#endif
            if ( SfxMacroConfig::IsMacroSlot( nId ) )
            {
                delete (*pImp->pCaches)[nPos];
                pImp->pCaches->Remove(nPos, 1);
            }
            else
                pImp->bCtrlReleased = sal_True;
        }
    }

    LEAVEREGISTRATIONS();
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxBindings::ExecuteSynchron( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi,
            const SfxPoolItem **ppInternalArgs )
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return NULL;

    return Execute_Impl( nId, ppItems, nModi, SFX_CALLMODE_SYNCHRON, ppInternalArgs );
}

sal_Bool SfxBindings::Execute( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs )

/*  [Beschreibung]

    F"uhrt den Slot mit der Slot-Id nId "uber den <Slot-Server> Cache
    aus. Dies ist nur bei in dieser SfxBindings INstanz gebundenen
    Slot-Ids m"oglich.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Das Execute wurde ausgef"uhrt.

                        sal_False
                        Das Execute konnte nicht ausgef"uhrt werden,
                        weil der Slot entweder nicht zur Verf"ugung steht
                        (in keiner aktiven <SfxShell> vorhanden oder
                        disabled) ist oder der Anwender die Ausf"uhrung
                        abgebrochen hat (Cancel in einem Dialog).


    [Querverweise]
    <SfxDispatcher>
*/
{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    if( !nId || !pDispatcher )
        return sal_False;

    const SfxPoolItem* pRet = Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs );
    return ( pRet != 0 );
}

const SfxPoolItem* SfxBindings::Execute_Impl( sal_uInt16 nId, const SfxPoolItem** ppItems, sal_uInt16 nModi, SfxCallMode nCallMode,
                        const SfxPoolItem **ppInternalArgs )
{
    SfxStateCache *pCache = GetStateCache( nId );
    if ( !pCache )
    {
        SfxBindings *pBind = pImp->pSubBindings;
        while ( pBind )
        {
            if ( pBind->GetStateCache( nId ) )
                return pBind->Execute_Impl( nId, ppItems, nModi, nCallMode, ppInternalArgs );
            pBind = pBind->pImp->pSubBindings;
        };
    }

    // synchronisieren
    SfxDispatcher &rDispatcher = *pDispatcher;
    rDispatcher.Flush();
    SfxViewFrame *pFrame = rDispatcher.GetFrame();

    // vom cache den Server (Slot+ShellLevel) und die Shell etc. abholen
    sal_Bool bDeleteCache = sal_False;
    if ( !pCache )
    {
        pCache = new SfxStateCache( nId );
        pCache->GetSlotServer( rDispatcher, pImp->xProv );
        bDeleteCache = sal_True;
    }

    if ( pCache && pCache->GetDispatch().is() )
    {
        pCache->Dispatch( nCallMode == SFX_CALLMODE_SYNCHRON );
        if ( bDeleteCache )
            DELETEZ( pCache );
        SfxPoolItem *pVoid = new SfxVoidItem( nId );
        DeleteItemOnIdle( pVoid );
        return pVoid;
    }

    // Zur Sicherheit!
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    // Neuerdings k"onnen wir auch nicht gecachete Slots executen
    // ( wg. Acceleratoren, deren Controller aus Performance-Gr"unden nicht mehr
    // gebunden werden ).
    SfxShell *pShell=0;
    const SfxSlot *pSlot=0;

    const SfxSlotServer* pServer = pCache ? pCache->GetSlotServer( rDispatcher, pImp->xProv ) : 0;
    if ( !pServer )
    {
        SfxSlotServer aServer;
        if ( !rDispatcher._FindServer( nId, aServer, sal_False ) )
        {
            if ( bDeleteCache )
                delete pCache;
            return NULL;
        }

        pShell = rDispatcher.GetShell( aServer.GetShellLevel() );
        pSlot = aServer.GetSlot();
    }
    else
    {
        pShell = rDispatcher.GetShell( pServer->GetShellLevel() );
        pSlot = pServer->GetSlot();
    }

    SfxItemPool &rPool = pShell->GetPool();
    SfxRequest aReq( nId, nCallMode, rPool );
    aReq.SetModifier( nModi );
    if( ppItems )
        while( *ppItems )
            aReq.AppendItem( **ppItems++ );
    if ( ppInternalArgs )
    {
        SfxAllItemSet aSet( rPool );
        for ( const SfxPoolItem **pArg = ppInternalArgs; *pArg; ++pArg )
            aSet.Put( **pArg );
        aReq.SetInternalArgs_Impl( aSet );
    }

    if ( SFX_KIND_ENUM == pSlot->GetKind() )
    {
        // bei Enum-Slots muss der Master mit dem Wert des Enums executet werden
        const SfxSlot *pRealSlot = pShell->GetInterface()->GetRealSlot(pSlot);
        const sal_uInt16 nSlotId = pRealSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        aReq.AppendItem( SfxAllEnumItem( rPool.GetWhich(nSlotId),
                                         pSlot->GetValue() ) );
        rDispatcher._Execute( *pShell, *pRealSlot, aReq, nCallMode | SFX_CALLMODE_RECORD );
    }
    else if ( SFX_KIND_ATTR == pSlot->GetKind() )
    {
        // bei Attr-Slots muá der Which-Wert gemapped werden
        const sal_uInt16 nSlotId = pSlot->GetSlotId();
        aReq.SetSlot( nSlotId );
        if ( pSlot->IsMode(SFX_SLOT_TOGGLE) )
        {
            // an togglebare-Attribs (Bools) wird der Wert angeheangt
            sal_uInt16 nWhich = pSlot->GetWhich(rPool);
            SfxItemSet aSet(rPool, nWhich, nWhich, 0);
            SfxStateFunc aFunc  = pSlot->GetStateFnc();
            pShell->CallState( aFunc, aSet );
            const SfxPoolItem *pOldItem;
            SfxItemState eState = aSet.GetItemState(nWhich, sal_True, &pOldItem);
            if ( SFX_ITEM_SET == eState ||
                 ( SFX_ITEM_AVAILABLE == eState &&
                   SfxItemPool::IsWhich(nWhich) &&
                   ( pOldItem = &aSet.Get(nWhich) ) ) )
            {
                if ( pOldItem->ISA(SfxBoolItem) )
                {
                    // wir koennen Bools toggeln
                    sal_Bool bOldValue = ((const SfxBoolItem *)pOldItem)->GetValue();
                    SfxBoolItem *pNewItem = (SfxBoolItem*) (pOldItem->Clone());
                    pNewItem->SetValue( !bOldValue );
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else if ( pOldItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pOldItem)->HasBoolValue())
                {
                    // und Enums mit Bool-Interface
                    SfxEnumItemInterface *pNewItem =
                        (SfxEnumItemInterface*) (pOldItem->Clone());
                    pNewItem->SetBoolValue(!((SfxEnumItemInterface *)pOldItem)->GetBoolValue());
                    aReq.AppendItem( *pNewItem );
                    delete pNewItem;
                }
                else
                    DBG_ERROR( "Toggle only for Enums and Bools allowed" );
            }
            else if ( SFX_ITEM_DONTCARE == eState )
            {
                // ein Status-Item per Factory erzeugen
                SfxPoolItem *pNewItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pNewItem, "Toggle an Slot ohne ItemFactory" );
                pNewItem->SetWhich( nWhich );

                if ( pNewItem->ISA(SfxBoolItem) )
                {
                    // wir koennen Bools toggeln
                    ((SfxBoolItem*)pNewItem)->SetValue( sal_True );
                    aReq.AppendItem( *pNewItem );
                }
                else if ( pNewItem->ISA(SfxEnumItemInterface) &&
                        ((SfxEnumItemInterface *)pNewItem)->HasBoolValue())
                {
                    // und Enums mit Bool-Interface
                    ((SfxEnumItemInterface*)pNewItem)->SetBoolValue(sal_True);
                    aReq.AppendItem( *pNewItem );
                }
                else
                    DBG_ERROR( "Toggle only for Enums and Bools allowed" );
                delete pNewItem;
            }
            else
                DBG_ERROR( "suspicious Toggle-Slot" );
        }

        rDispatcher._Execute( *pShell, *pSlot, aReq, nCallMode | SFX_CALLMODE_RECORD );
    }
    else
        rDispatcher._Execute( *pShell, *pSlot, aReq, nCallMode | SFX_CALLMODE_RECORD );

    const SfxPoolItem* pRet = aReq.GetReturnValue();
    if ( !pRet )
    {
        SfxPoolItem *pVoid = new SfxVoidItem( nId );
        DeleteItemOnIdle( pVoid );
        pRet = pVoid;
    }

    if ( bDeleteCache )
        delete pCache;

    return pRet;
}

//--------------------------------------------------------------------

void SfxBindings::UpdateSlotServer_Impl()

/*  [Beschreibung]

    Interne Methode zum Updaten der Pointer auf die SlotServer
    nach <SfxBindings::InvalidateAll(sal_Bool)>.
*/

{
    DBG_PROFSTART(SfxBindingsUpdateServers);
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // synchronisieren
    pDispatcher->Flush();
//  pDispatcher->Update_Impl();

    const sal_uInt16 nCount = pImp->pCaches->Count();
    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        SfxStateCache *pCache = pImp->pCaches->GetObject(i);
        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    }
    pImp->bMsgDirty = pImp->bAllMsgDirty = sal_False;

    Broadcast( SfxSimpleHint(SFX_HINT_DOCCHANGED) );

    DBG_PROFSTOP(SfxBindingsUpdateServers);
}

//--------------------------------------------------------------------

#ifdef WNT
int __cdecl CmpUS_Impl(const void *p1, const void *p2)
#else
int CmpUS_Impl(const void *p1, const void *p2)
#endif

/*  [Beschreibung]

    Interne Vergleichsfunktion fuer qsort.
*/

{
    return *(sal_uInt16 *)p1 - *(sal_uInt16 *)p2;
}

//--------------------------------------------------------------------

SfxItemSet* SfxBindings::CreateSet_Impl
(
    SfxStateCache*&             pCache,     // in: Status-Cache von nId
    const SfxSlot*&             pRealSlot,  // out: RealSlot zu nId
    const SfxSlotServer**    pMsgServer, // out: Slot-Server zu nId
    SfxFoundCacheArr_Impl&      rFound      // out: Liste der Caches der Siblings
)

/*  [Beschreibung]

    Diese interne Methode sucht zu pCache die Slot-Ids, die von derselben
    Status-Methode bedient werden und ebenfalls gebunden und dirty sind.
    Es wird ein SfxItemSet zusammengestellt, das die Slot-Ids (oder falls
    vorhanden die mit dem Pool der Shell gemappten Which-Ids) enth"alt.
    Die Caches dieser Slots werden in pFoundCaches zur"uckgeliefert.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    DBG_ASSERT( !pImp->bMsgDirty, "CreateSet_Impl mit dirty MessageServer" );

    const SfxSlotServer* pMsgSvr = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
    if(!pMsgSvr || !pDispatcher)
        return 0;

    DBG_PROFSTART(SfxBindingsCreateSet);
    pRealSlot = 0;
    *pMsgServer = pMsgSvr;

    sal_uInt16 nShellLevel = pMsgSvr->GetShellLevel();
    SfxShell *pShell = pDispatcher->GetShell( nShellLevel );
    if ( !pShell ) // seltener GPF beim Browsen durch Update aus Inet-Notify
        return 0;

    SfxItemPool &rPool = pShell->GetPool();

    // hole die Status-Methode, von der pCache bedient wird
    SfxStateFunc pFnc = 0;
    const SfxInterface *pInterface = pShell->GetInterface();
    if ( SFX_KIND_ENUM == pMsgSvr->GetSlot()->GetKind() )
    {
        pRealSlot = pInterface->GetRealSlot(pMsgSvr->GetSlot());
        pCache = GetStateCache( pRealSlot->GetSlotId() );
//      DBG_ASSERT( pCache, "Kein Slotcache fuer den Masterslot gefunden!" );
    }
    else
        pRealSlot = pMsgSvr->GetSlot();

    //
    // Achtung: pCache darf auch NULL sein !!!
    //

    pFnc = pRealSlot->GetStateFnc();

    // der RealSlot ist immer drin
    const SfxFoundCache_Impl *pFound = new SfxFoundCache_Impl(
        pRealSlot->GetSlotId(), pRealSlot->GetWhich(rPool), pRealSlot, pCache );
    rFound.Insert( pFound );

    if ( !SfxMacroConfig::IsMacroSlot( pRealSlot->GetSlotId() ) )
    {
        pInterface->GetRealInterfaceForSlot( pRealSlot );
        DBG_ASSERT (pInterface,"Slot in angegebener Shell nicht gefunden!");
    }

    // Durchsuche die Bindings nach den von derselben Funktion bedienten Slots.
    // Daf"ur kommen nur Slots in Frage, die es im gefundenen Interface gibt.

    // Die Position des Statecaches im StateCache-Array
    sal_uInt16 nCachePos = pImp->nMsgPos;
    const SfxSlot *pSibling = pRealSlot->GetNextSlot();

    // Die Slots eines Interfaces sind im Kreis verkettet
    while ( pSibling > pRealSlot )
    {
        SfxStateFunc pSiblingFnc=0;
        SfxStateCache *pSiblingCache =
                GetStateCache( pSibling->GetSlotId(), &nCachePos );

        // Ist der Slot "uberhaupt gecached ?
        if ( pSiblingCache )
        {
            const SfxSlotServer *pServ = pSiblingCache->GetSlotServer(*pDispatcher, pImp->xProv);
            if ( pServ && pServ->GetShellLevel() == nShellLevel )
                pSiblingFnc = pServ->GetSlot()->GetStateFnc();
        }

        // Mu\s der Slot "uberhaupt upgedatet werden ?
        FASTBOOL bInsert = pSiblingCache && pSiblingCache->IsControllerDirty();

        // Bugfix #26161#: Es reicht nicht, nach der selben Shell zu fragen !!
        FASTBOOL bSameMethod = pSiblingCache && pFnc == pSiblingFnc;

        // Wenn der Slot ein nicht-dirty MasterSlot ist, dann ist vielleicht
        // einer seiner Slaves dirty ? Dann wird der Masterslot doch eingef"ugt.
        if ( !bInsert && bSameMethod && pSibling->GetLinkedSlot() )
        {
            // auch Slave-Slots auf Binding pru"fen
            const SfxSlot* pFirstSlave = pSibling->GetLinkedSlot();
            for ( const SfxSlot *pSlaveSlot = pFirstSlave;
                  !bInsert;
                  pSlaveSlot = pSlaveSlot->GetNextSlot())
            {
                // Die Slaves zeigen auf ihren Master
                DBG_ASSERT(pSlaveSlot->GetLinkedSlot() == pSibling,
                    "Falsche Master/Slave-Beziehung!");

                sal_uInt16 nCurMsgPos = pImp->nMsgPos;
                const SfxStateCache *pSlaveCache =
                    GetStateCache( pSlaveSlot->GetSlotId(), &nCurMsgPos );

                // Ist der Slave-Slot gecached und dirty ?
                bInsert = pSlaveCache && pSlaveCache->IsControllerDirty();

                // Slaves sind untereinander im Kreis verkettet
                if (pSlaveSlot->GetNextSlot() == pFirstSlave)
                    break;
            }
        }

        if ( bInsert && bSameMethod )
        {
            const SfxFoundCache_Impl *pFound = new SfxFoundCache_Impl(
                pSibling->GetSlotId(), pSibling->GetWhich(rPool),
                pSibling, pSiblingCache );

            rFound.Insert( pFound );
        }

        pSibling = pSibling->GetNextSlot();
    }

    // aus den Ranges ein Set erzeugen
    sal_uInt16 *pRanges = new sal_uInt16[rFound.Count() * 2 + 1];
    int j = 0;
    int i = 0;
    while ( i < rFound.Count() )
    {
        pRanges[j++] = rFound[i]->nWhichId;
            // aufeinanderfolgende Zahlen
        for ( ; i < rFound.Count()-1; ++i )
            if ( rFound[i]->nWhichId+1 != rFound[i+1]->nWhichId )
                break;
        pRanges[j++] = rFound[i++]->nWhichId;
    }
    pRanges[j] = 0; // terminierende NULL
    SfxItemSet *pSet = new SfxItemSet(rPool, pRanges);
    delete pRanges;
    DBG_PROFSTOP(SfxBindingsCreateSet);
    return pSet;
}

//--------------------------------------------------------------------

void SfxBindings::UpdateControllers_Impl
(
    const SfxInterface*         pIF,    // das diese Id momentan bedienende Interface
    const SfxFoundCache_Impl*   pFound, // Cache, Slot, Which etc.
    const SfxPoolItem*          pItem,  // item to send to controller
    SfxItemState                eState  // state of item
)

/*  [Beschreibung]

    Dieses ist eine Hilfsmethode f"ur NextJob_Impl mit der die SfxController,
    welche auf nSlotId gebunden sind, upgedated werden. Dabei wird der
    Wert aus dem SfxPoolItem unter dem Which-Wert nWhich aus dem Set rSet
    genommen.

    Falls zu rSlot Enum-Werte in der Slotmap eingetragen sind, und diese
    gebunden sind, werden sie ebenfalls upgedated.
*/

{
    DBG_ASSERT( !pFound->pSlot || SFX_KIND_ENUM != pFound->pSlot->GetKind(),
                "direct update of enum slot isn't allowed" );
    DBG_PROFSTART(SfxBindingsUpdateCtrl1);

    SfxStateCache* pCache = pFound->pCache;
    const SfxSlot* pSlot = pFound->pSlot;
    DBG_ASSERT( !pCache || !pSlot || pCache->GetId() == pSlot->GetSlotId(), "SID mismatch" );

    // insofern gebunden, die Controller f"uer den Slot selbst updaten
    if ( pCache && pCache->IsControllerDirty() )
    {
        if ( SFX_ITEM_DONTCARE == eState )
        {
            // uneindeuting
            pCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
        }
        else if ( SFX_ITEM_DEFAULT == eState &&
                    pFound->nWhichId > SFX_WHICH_MAX )
        {
            // kein Status oder Default aber ohne Pool
            SfxVoidItem aVoid(0);
            pCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );
        }
        else if ( SFX_ITEM_DISABLED == eState )
            pCache->SetState(SFX_ITEM_DISABLED, 0);
        else
            pCache->SetState(SFX_ITEM_AVAILABLE, pItem);
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl1);

    // insofern vorhanden und gebunden, die Controller f"uer Slave-Slots
    // (Enum-Werte) des Slots updaten
    DBG_PROFSTART(SfxBindingsUpdateCtrl2);
    DBG_ASSERT( !pSlot || 0 == pSlot->GetLinkedSlot() || !pItem ||
                pItem->ISA(SfxEnumItemInterface),
                "master slot with non-enum-type found" );
    const SfxSlot *pFirstSlave = pSlot ? pSlot->GetLinkedSlot() : 0;
    if ( pIF && pFirstSlave)
    {
        // Items auf EnumItem casten
        const SfxEnumItemInterface *pEnumItem =
                PTR_CAST(SfxEnumItemInterface,pItem);
        if ( eState == SFX_ITEM_AVAILABLE && !pEnumItem )
            eState = SFX_ITEM_DONTCARE;
        else
            eState = SfxControllerItem::GetItemState( pEnumItem );

        // "uber alle Slaves-Slots iterieren
        for ( const SfxSlot *pSlave = pFirstSlave; pSlave;
              pSlave = pSlave->GetNextSlot() )
        {
            DBG_ASSERT(pSlave, "Falsche SlaveSlot-Verkettung!");
            DBG_ASSERT(SFX_KIND_ENUM == pSlave->GetKind(),"non enum slaves aren't allowed");
            DBG_ASSERT(pSlave->GetMasterSlotId() == pSlot->GetSlotId(),"falscher MasterSlot!");

            // ist die Funktion gebunden?
            SfxStateCache *pEnumCache = GetStateCache( pSlave->GetSlotId() );
            if ( pEnumCache )
            {
                pEnumCache->Invalidate(sal_False);

                HACK(CONTROL/SELECT Kram)
                if ( eState == SFX_ITEM_DONTCARE && pFound->nWhichId == 10144 )
                {
                    SfxVoidItem aVoid(0);
                    pEnumCache->SetState( SFX_ITEM_UNKNOWN, &aVoid );

                    if (pSlave->GetNextSlot() == pFirstSlave)
                        break;

                    continue;
                }

                if ( SFX_ITEM_DISABLED == eState
#ifdef OV_hat_das_SvxChooseControlItem_angepasst
                          || USHRT_MAX == pEnumItem->GetPosByValue(pEnumCache->GetValue())
#endif
                    )
                {
                    // disabled
                    pEnumCache->SetState(SFX_ITEM_DISABLED, 0);
                }
                else if ( SFX_ITEM_AVAILABLE == eState )
                {
                    // enum-Wert ermitteln
                    sal_uInt16 nValue = pEnumItem->GetEnumValue();
                    SfxBoolItem aBool( pFound->nWhichId, pSlave->GetValue() == nValue );
                    pEnumCache->SetState(SFX_ITEM_AVAILABLE, &aBool);
                }
                else
                {
                    // uneindeuting
                    pEnumCache->SetState( SFX_ITEM_DONTCARE, (SfxPoolItem *)-1 );
                }
            }

            if (pSlave->GetNextSlot() == pFirstSlave)
                break;
        }
    }

    DBG_PROFSTOP(SfxBindingsUpdateCtrl2);
}


//--------------------------------------------------------------------

IMPL_LINK( SfxBindings, NextJob_Impl, Timer *, pTimer )

/*  [Beschreibung]

    Die SfxController werden "uber einen Timer updated. Dieses ist der
    dazugeh"orige interne TimeOut-Handler.
 */

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    DBG_PROFSTART(SfxBindingsNextJob_Impl0);
    SfxApplication *pSfxApp = SFX_APP();

    if( pDispatcher )
        pDispatcher->Update_Impl();

    // modifying the SfxObjectInterface-stack without SfxBindings => nothing to do
    SfxViewFrame* pFrame = pDispatcher->GetFrame();
    if ( pFrame && pFrame->GetObjectShell()->IsInModalMode() || pSfxApp->IsDowning() || !pImp->pCaches->Count() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }
    if ( !pDispatcher || !pDispatcher->IsFlushed() )
    {
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_True;
    }

    // gfs. alle Server aktualisieren / geschieht in eigener Zeitscheibe
    if ( pImp->bMsgDirty )
    {
        UpdateSlotServer_Impl();
        DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
        return sal_False;
    }

    DBG_PROFSTOP(SfxBindingsNextJob_Impl0);
    DBG_PROFSTART(SfxBindingsNextJob_Impl);
    pImp->bAllDirty = sal_False;
    pImp->aTimer.SetTimeout(TIMEOUT_UPDATING);

    // at least 10 loops and further if more jobs are available but no input
    FASTBOOL bPreEmptive = pTimer && !pSfxApp->Get_Impl()->nInReschedule;
    sal_uInt16 nLoops = 10;
    pImp->bInNextJob = sal_True;
    const sal_uInt16 nCount = pImp->pCaches->Count();
    while ( pImp->nMsgPos < nCount )
    {
        // iterate through the bound functions
        sal_Bool bJobDone = sal_False;
        while ( !bJobDone )
        {
            SfxStateCache* pCache = (*pImp->pCaches)[pImp->nMsgPos];
            DBG_ASSERT( pCache, "invalid SfxStateCache-position in job queue" );
            sal_Bool bWasDirty = pCache->IsControllerDirty();
            if ( bWasDirty )
            {
/*
                sal_Bool bSkip = sal_False;
                if ( pImp->bFirstRound )
                {
                    // Falls beim Update eine Shell vorgezogen werden soll,
                    // kommt in einer ersten Update-Runde nur diese dran
                    const SfxSlotServer *pMsgServer =
                        pCache->GetSlotServer(*pDispatcher, pImp->xProv);
                    if ( pMsgServer &&
                        pMsgServer->GetShellLevel() != pImp->nFirstShell )
                            bSkip = sal_True;
                }

                if ( !bSkip )
                {
*/
                    Update_Impl( pCache );
                    DBG_ASSERT( nCount == pImp->pCaches->Count(),
                            "Reschedule in StateChanged => buff" );
//              }
            }

            // skip to next function binding
            ++pImp->nMsgPos;

            // keep job if it is not completed, but any input is available
            bJobDone = pImp->nMsgPos >= nCount;
            if ( bJobDone && pImp->bFirstRound )
            {
                // Update der bevorzugten Shell ist gelaufen, nun d"urfen
                // auch die anderen
                bJobDone = sal_False;
                pImp->bFirstRound = sal_False;
                pImp->nMsgPos = 0;
            }

            if ( bWasDirty && !bJobDone && bPreEmptive && (--nLoops == 0) )
            {
                DBG_PROFSTOP(SfxBindingsNextJob_Impl);
                pImp->bInNextJob = sal_False;
                return sal_False;
            }
        }
    }

    // volatiles wieder von vorne starten
    pImp->nMsgPos = 0;
    pImp->aTimer.SetTimeout(TIMEOUT_IDLE);
    for ( sal_uInt16 n = 0; n < nCount; ++n )
    {
        SfxStateCache* pCache = (*pImp->pCaches)[n];
        const SfxSlotServer *pSlotServer = pCache->GetSlotServer(*pDispatcher, pImp->xProv);
        if ( pSlotServer &&
             pSlotServer->GetSlot()->IsMode(SFX_SLOT_VOLATILE) )
            pCache->Invalidate(sal_False);
    }

    // Update-Runde ist beendet
    pImp->bInNextJob = sal_False;
    Broadcast(SfxSimpleHint(SFX_HINT_UPDATEDONE));
    DBG_PROFSTOP(SfxBindingsNextJob_Impl);
    return sal_True;
}

//--------------------------------------------------------------------

sal_uInt16 SfxBindings::EnterRegistrations(char *pFile, int nLine)

/*  [Beschreibung]

    Die An- oder Abmeldung von <SfxControllerItem> Instanzen mu"s in
    EnterRegistrations() und LeaveRegistrations() geklammert werden.
    W"ahrend dieser Zeit erfolgen keine Udates der <SfxContollerItem>
    Instanzen (weder der alten noch der neu angemeldeten).

    [Parameter]

    pFile, nLine            Dateiname und Zeilennummer der rufenden
                            Methode (nur Debug)

    [R"uckgabewert]

    sal_uInt16                  Level der Registrierung. Dieser kann in
                            <SfxBindings::LeaveRegistrations(sal_uInt16)> als
                            Parameter angegeben werden, um die Paarigkeit
                            der EnterRegistrations() und LeaveRegistrations()
                            zu pr"ufen.


    [Querverweise]
    <SfxBindings::IsInRegistrations()>
    <SfxBindings::Register(SfxControllerItem&)>
    <SfxBindings::Release(SfxControllerItem&)>
    <SfxBindings::LeaveRegistrations()>
*/

{
    DBG_MEMTEST();
#ifdef DBG_UTIL
    ByteString aMsg;
    aMsg.Fill( Min(nRegLevel, sal_uInt16(8) ) );
    aMsg += "this = ";
    aMsg += ByteString::CreateFromInt32((long)this);
    aMsg += " Level = ";
    aMsg += ByteString::CreateFromInt32(nRegLevel);
    aMsg += " SfxBindings::EnterRegistrations ";
    if(pFile) {
        aMsg += "File: ";
        aMsg += pFile;
        aMsg += " Line: ";
        aMsg += ByteString::CreateFromInt32(nLine);
    }
    DbgTrace( aMsg.GetBuffer() );
#endif

    // Wenn Bindings gelockt werden, auch SubBindings locken
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->ENTERREGISTRATIONS();

        // Dieses EnterRegistrations ist f"ur die SubBindings kein "echtes"
        pImp->pSubBindings->pImp->nOwnRegLevel--;

        // Bindings synchronisieren
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel + 1;
    }

    pImp->nOwnRegLevel++;

    // check if this is the outer most level
    if ( ++nRegLevel == 1 )
    {
        // stop background-processing
        pImp->aTimer.Stop();

        // flush the cache
        pImp->nCachedFunc1 = 0;
        pImp->nCachedFunc2 = 0;

        // merken, ob ganze Caches verschwunden sind
        pImp->bCtrlReleased = sal_False;
    }

    return nRegLevel;
}
//--------------------------------------------------------------------

void SfxBindings::LeaveRegistrations( sal_uInt16 nLevel, char *pFile, int nLine )

/*  [Beschreibung]

    Die An- oder Abmeldung von <SfxControllerItem> Instanzen mu"s in
    EnterRegistrations() und LeaveRegistrations() geklammert werden.
    W"ahrend dieser Zeit erfolgen keine Udates der <SfxContollerItem>
    Instanzen (weder der alten noch der neu angemeldeten).


    [Parameter]

    sal_uInt16 nLevel           == USRT_MAX
                            keine Paarigkeits-Pr"ufung f"ur diese Klammerung


    pFile, nLine            Dateiname und Zeilennummer der rufenden
                            Methode (nur Debug)

                            < USHRT_MAX
                            R"uckgabewert des zugeh"origen EnterRegistrations()
                            zum pr"ufen der Paarigkeit.


    [Querverweise]
    <SfxBindings::IsInRegistrations()>
    <SfxBindings::Register(SfxControllerItem&)>
    <SfxBindings::Release(SfxControllerItem&)>
    <SfxBindings::EnterRegistrations()>
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( nRegLevel, "Leave without Enter" );
    DBG_ASSERT( nLevel == USHRT_MAX || nLevel == nRegLevel, "wrong Leave" );

    // Nur wenn die SubBindings noch von den SuperBindings gelockt sind, diesen Lock entfernen
    // ( d.h. wenn es mehr Locks als "echte" Locks dort gibt )
    if ( pImp->pSubBindings && pImp->pSubBindings->nRegLevel > pImp->pSubBindings->pImp->nOwnRegLevel )
    {
        // Bindings synchronisieren
        pImp->pSubBindings->nRegLevel = nRegLevel + pImp->pSubBindings->pImp->nOwnRegLevel;

        // Dieses LeaveRegistrations ist f"ur die SubBindings kein "echtes"
        pImp->pSubBindings->pImp->nOwnRegLevel++;
        pImp->pSubBindings->LEAVEREGISTRATIONS();
    }

    pImp->nOwnRegLevel--;

    // check if this is the outer most level
    if ( --nRegLevel == 0 && !SFX_APP()->IsDowning() )
    {
#ifndef slow
        SfxViewFrame* pFrame = pDispatcher->GetFrame();

        // ggf unbenutzte Caches entfernen bzw. PlugInInfo aufbereiten
        if ( pImp->bCtrlReleased )
        {
            for ( sal_uInt16 nCache = pImp->pCaches->Count(); nCache > 0; --nCache )
            {
                // Cache via ::com::sun::star::sdbcx::Index besorgen
                SfxStateCache *pCache = pImp->pCaches->GetObject(nCache-1);

                // kein Controller mehr interessiert
                if ( pCache->GetItemLink() == 0 )
                {
                    // Cache entfernen
                    delete (*pImp->pCaches)[nCache-1];
                    pImp->pCaches->Remove(nCache-1, 1);
                }
                else
                {
                    // neue Controller mit den alten Items benachrichtigen
                    //!pCache->SetCachedState();
                }
            }
        }
#endif
        // restart background-processing
        pImp->nMsgPos = 0;
        if ( !pFrame || !pFrame->GetObjectShell() )
            return;
        if ( pImp->pCaches && pImp->pCaches->Count() )
        {
            pImp->aTimer.SetTimeout(TIMEOUT_FIRST);
            pImp->aTimer.Start();
//          pImp->bFirstRound = sal_True;
        }
    }

#ifdef DBG_UTIL
    ByteString aMsg;
    aMsg.Fill( Min(nRegLevel, sal_uInt16(8)) );
    aMsg += "this = ";
    aMsg += ByteString::CreateFromInt32((long)this);
    aMsg += " Level = ";
    aMsg += ByteString::CreateFromInt32(nRegLevel);
    aMsg += " SfxBindings::LeaveRegistrations ";
    if(pFile) {
        aMsg += "File: ";
        aMsg += pFile;
        aMsg += " Line: ";
        aMsg += ByteString::CreateFromInt32(nLine);
    }
    DbgTrace( aMsg.GetBuffer() );
#endif
}

//--------------------------------------------------------------------

const SfxSlot* SfxBindings::GetSlot(sal_uInt16 nSlotId)

/*  [Beschreibung]

    Diese Methode liefert einen Pointer auf den zur Zeit gecacheten
    SfxSlot f"ur die angegebene Slot-Id.


    [R"uckgabewert]

    const <SfxSlot>*        0
                            Falls die Slot-Id nicht gebunden ist oder
                            ein solcher Slot momentan in keiner aktiven
                            <SfxShell> vorhanden ist.

                            != 0
                            Falls die Slot-Id gebunden ist und ein solcher
                            Slot momentan in einer aktiven <SfxShell>
                            vorhanden ist.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->pCaches != 0, "SfxBindings not initialized" );

    // syncronisieren
    pDispatcher->Flush();
    if ( pImp->bMsgDirty )
        UpdateSlotServer_Impl();

    // get the cache for the specified function; return if not bound
    SfxStateCache* pCache = GetStateCache(nSlotId);
    return pCache && pCache->GetSlotServer(*pDispatcher, pImp->xProv)?
            pCache->GetSlotServer(*pDispatcher, pImp->xProv)->GetSlot(): 0;
}

//--------------------------------------------------------------------

void SfxBindings::SetDispatcher( SfxDispatcher *pDisp )

/*  [Beschreibung]

    Setzt den zur Zeit von dieser SfxBindings Instanz zu verwendenden
    Dispatcher um.

    Falls sich der Dispatcher dadurch "andert, wird intern
    <SFxBindings::InvalidateAll(sal_Bool)> mit sal_True gerufen, also jegliche
    gecachete Information der Bindings weggeworfen.
*/

{
    SfxDispatcher *pOldDispat = pDispatcher;
    if ( pDisp != pDispatcher )
    {
        if ( pOldDispat )
        {
            SfxBindings* pBind = pOldDispat->GetBindings();
            while ( pBind )
            {
                if ( pBind->pImp->pSubBindings == this && pBind->pDispatcher != pDisp )
                    pBind->SetSubBindings_Impl( NULL );
                pBind = pBind->pImp->pSubBindings;
            }
        }

        pDispatcher = pDisp;

        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider > xProv;
        if ( pDisp )
            xProv = ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider >
                                        ( pDisp->GetFrame()->GetFrame()->GetFrameInterface(), UNO_QUERY );

        SetDispatchProvider_Impl( xProv );
        InvalidateAll( sal_True );
        InvalidateUnoControllers_Impl();

        if ( pDispatcher && !pOldDispat )
        {
            if ( pImp->pSubBindings && pImp->pSubBindings->pDispatcher != pOldDispat )
            {
                DBG_ERROR( "SubBindings vor Aktivieren schon gesetzt!" );
                pImp->pSubBindings->ENTERREGISTRATIONS();
            }
            LEAVEREGISTRATIONS();
        }
        else if( !pDispatcher )
        {
            ENTERREGISTRATIONS();
            if ( pImp->pSubBindings && pImp->pSubBindings->pDispatcher != pOldDispat )
            {
                DBG_ERROR( "SubBindings im Deaktivieren immer noch gesetzt!" );
                pImp->pSubBindings->LEAVEREGISTRATIONS();
            }
        }

        Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        if ( pDisp )
        {
            SfxBindings* pBind = pDisp->GetBindings();
            while ( pBind && pBind != this )
            {
                if ( !pBind->pImp->pSubBindings )
                {
                    pBind->SetSubBindings_Impl( this );
                    break;
                }

                pBind = pBind->pImp->pSubBindings;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxBindings::ClearCache_Impl( sal_uInt16 nSlotId )

//  interne Methode zum forwarden dieses Methodenaufrufs

{
    GetStateCache(nSlotId)->ClearCache();
}

//--------------------------------------------------------------------

// interne Methode zum Ansto\sen des Statusupdates

void SfxBindings::StartUpdate_Impl( sal_Bool bComplete )
{
    if ( pImp->pSubBindings )
        pImp->pSubBindings->StartUpdate_Impl( bComplete );

    if ( !bComplete )
        // Update darf unterbrochen werden
        NextJob_Impl(&pImp->aTimer);
    else
        // alle Slots am St"uck updaten
        NextJob_Impl(0);
}

//--------------------------------------------------------------------
// Obsolete code, remove later
struct SfxSlotInterceptor_Impl
{
    SfxBindings*    _pBindings;
    BOOL            _bIsActive;
};

SfxSlotInterceptor::SfxSlotInterceptor( SfxBindings *pBindings, sal_Bool bActivate )
    : _pImp( new SfxSlotInterceptor_Impl )
{
    _pImp->_pBindings = pBindings;
    _pImp->_bIsActive = bActivate;
}

SfxSlotInterceptor::~SfxSlotInterceptor()
{
    delete _pImp;
}

void SfxSlotInterceptor::SetBindings( SfxBindings* pBindings )
{
    _pImp->_pBindings = pBindings;
}

SfxBindings* SfxSlotInterceptor::GetBindings() const
{
    return _pImp->_pBindings;
}

void SfxSlotInterceptor::Activate( sal_Bool bActivate )
{
    _pImp->_bIsActive = bActivate;
}

sal_Bool SfxSlotInterceptor::IsActive() const
{
    return _pImp->_bIsActive;
}

sal_Bool SfxSlotInterceptor::Execute( sal_uInt16 nSID, SfxPoolItem **ppArgs )
{
    return sal_False;
}

SfxItemState SfxSlotInterceptor::QueryState( sal_uInt16 nSID, SfxPoolItem*& rpState )
{
    return SFX_ITEM_UNKNOWN;
}

//-------------------------------------------------------------------------

SfxItemState SfxBindings::QueryState( sal_uInt16 nSlot, SfxPoolItem* &rpState )
/*  [Beschreibung]

    Wird gerufen, um den Status f"ur 'nSlot' zu erfragen. Wenn der return
    value SFX_ITEM_SET ist, wird ein SfxPoolItem zur"uckgegeben, indem der
    rpState entsprechend gesetzt wird. Es findet dabei ein Eigent"umer"ubergang
    statt, d.h. die aufrufende Methode mu\s das Item l"oschen.

    Anmerkung: diese Methode ist sehr teuer und sollte nur gerufen werden,
    wenn kein Controller f"ur das Erfragen des Status angelegt werden kann oder
    der Status unbedingt sofort geliefert werden mu\s.
*/

{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp;
    SfxStateCache *pCache = GetStateCache( nSlot );
    if ( pCache )
        xDisp = pCache->GetDispatch();
    if ( xDisp.is() || !pCache )
    {
//(mba)/compview
        const SfxSlot* pSlot = 0;//SfxComponentViewShell::GetUnoSlot( nSlot );
        ::com::sun::star::util::URL aURL;
        if ( pSlot )
        {
            String aCmd( DEFINE_CONST_UNICODE(".uno:"));
            aCmd += String::CreateFromAscii(pSlot->GetUnoName());
            aURL.Complete = aCmd;
        }
        else
        {
            String aCmd( DEFINE_CONST_UNICODE("slot:") );
            aCmd += String::CreateFromInt32(nSlot);
            aURL.Complete = aCmd;
        }

        Reference < XURLTransformer > xTrans( ::utl::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );
        if ( !xDisp.is() )
            xDisp = pImp->xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );

        if ( xDisp.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel( xDisp, ::com::sun::star::uno::UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = (SfxOfficeDispatch*)(nImplementation);
            }

            if ( !pDisp || pDisp->IsInterceptDispatch() )
            {
                SfxItemState eState = SFX_ITEM_SET;
                SfxPoolItem *pItem=NULL;
                BindDispatch_Impl *pBind = new BindDispatch_Impl( xDisp, aURL, NULL );
                pBind->acquire();
                xDisp->addStatusListener( pBind, aURL );
                if ( !pBind->GetStatus().IsEnabled )
                    eState = SFX_ITEM_DISABLED;
                else
                {
                    ::com::sun::star::uno::Any aAny = pBind->GetStatus().State;
                    ::com::sun::star::uno::Type pType = aAny.getValueType();

                    if ( pType == ::getBooleanCppuType() )
                    {
                        sal_Bool bTemp ;
                        aAny >>= bTemp ;
                        pItem = new SfxBoolItem( nSlot, bTemp );
                    }
                    else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                    {
                        sal_uInt16 nTemp ;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt16Item( nSlot, nTemp );
                    }
                    else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                    {
                        sal_uInt32 nTemp ;
                        aAny >>= nTemp ;
                        pItem = new SfxUInt32Item( nSlot, nTemp );
                    }
                    else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                    {
                        ::rtl::OUString sTemp ;
                        aAny >>= sTemp ;
                        pItem = new SfxStringItem( nSlot, sTemp );
                    }
                    else
                        pItem = new SfxVoidItem( nSlot );
                }

                pBind->Release();
                rpState = pItem;
                return eState;
            }
        }
    }

    // Dann am Dispatcher testen; da die von dort zur"uckgegebenen Items immer
    // DELETE_ON_IDLE sind, mu\s eine Kopie davon gezogen werden, um einen
    // Eigent"umer"ubergang zu erm"oglichen
    const SfxPoolItem *pItem = NULL;
    SfxItemState eState = pDispatcher->QueryState( nSlot, pItem );
    if ( eState == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem, "SFX_ITEM_SET aber kein Item!" );
        if ( pItem )
            rpState = pItem->Clone();
    }
    else if ( eState == SFX_ITEM_AVAILABLE && pItem )
    {
        rpState = pItem->Clone();
    }

    return eState;
}

void SfxBindings::SetSubBindings_Impl( SfxBindings *pSub )
{
    if ( pImp->pSubBindings )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > () );
        pImp->pSubBindings->pImp->pSuperBindings = NULL;
    }

    pImp->pSubBindings = pSub;

    if ( pSub )
    {
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
        pSub->pImp->pSuperBindings = this;
    }
}

SfxBindings* SfxBindings::GetSubBindings_Impl( sal_Bool bTop ) const
{
    SfxBindings *pRet = pImp->pSubBindings;
    if ( bTop )
    {
        while ( pRet->pImp->pSubBindings )
            pRet = pRet->pImp->pSubBindings;
    }

    return pRet;
}

void SfxBindings::SetWorkWindow_Impl( SfxWorkWindow* pWork )
{
    pImp->pWorkWin = pWork;
}

SfxWorkWindow* SfxBindings::GetWorkWindow_Impl() const
{
    return pImp->pWorkWin;
}

void SfxBindings::RegisterUnoController_Impl( SfxUnoControllerItem* pControl )
{
    if ( !pImp->pUnoCtrlArr )
        pImp->pUnoCtrlArr = new SfxUnoControllerArr_Impl;
    pImp->pUnoCtrlArr->Insert( pControl, pImp->pUnoCtrlArr->Count() );
}

void SfxBindings::ReleaseUnoController_Impl( SfxUnoControllerItem* pControl )
{
    if ( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nPos = pImp->pUnoCtrlArr->GetPos( pControl );
        if ( nPos != 0xFFFF )
        {
            pImp->pUnoCtrlArr->Remove( nPos );
            return;
        }
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->ReleaseUnoController_Impl( pControl );
}

void SfxBindings::InvalidateUnoControllers_Impl()
{
    if ( pImp->pUnoCtrlArr )
    {
        sal_uInt16 nCount = pImp->pUnoCtrlArr->Count();
        for ( sal_uInt16 n=nCount; n>0; n-- )
        {
            SfxUnoControllerItem *pCtrl = (*pImp->pUnoCtrlArr)[n-1];
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  xRef( (::cppu::OWeakObject*)pCtrl, ::com::sun::star::uno::UNO_QUERY );
            pCtrl->ReleaseDispatch();
            pCtrl->GetNewDispatch();
        }
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->InvalidateUnoControllers_Impl();
}

sal_Bool SfxBindings::IsInUpdate() const
{
    sal_Bool bInUpdate = pImp->bInUpdate;
    if ( !bInUpdate && pImp->pSubBindings )
        bInUpdate = pImp->pSubBindings->IsInUpdate();
    return bInUpdate;
}

void SfxBindings::SetActiveFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & rFrame )
{
    SetDispatchProvider_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > ( rFrame, ::com::sun::star::uno::UNO_QUERY ) );
}

void SfxBindings::SetDispatchProvider_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & rProv )
{
    sal_Bool bInvalidate = ( rProv != pImp->xProv );
    if ( bInvalidate )
    {
        pImp->xProv = rProv;
        InvalidateAll( sal_True );
        InvalidateUnoControllers_Impl();
    }

    if ( pImp->pSubBindings )
        pImp->pSubBindings->SetDispatchProvider_Impl( pImp->xProv );
}

const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > & SfxBindings::GetDispatchProvider_Impl() const
{
    return pImp->xProv;
}

SystemWindow* SfxBindings::GetSystemWindow() const
{
    SfxViewFrame *pFrame = pDispatcher->GetFrame();
    while ( pFrame->GetParentViewFrame_Impl() )
        pFrame = pFrame->GetParentViewFrame_Impl();
    SfxTopViewFrame* pTop = PTR_CAST( SfxTopViewFrame, pFrame->GetTopViewFrame() );
    return pTop->GetTopFrame_Impl()->GetTopWindow_Impl();
}
