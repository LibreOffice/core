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
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/embed/VerbAttributes.hpp>
#include <basic/sbstar.hxx>
#include <svl/itempool.hxx>
#include <svl/undo.hxx>
#include <svtools/itemdel.hxx>
#include <svtools/asynclink.hxx>
#include <basic/sbx.hxx>

#include <unotools/undoopt.hxx>

#include <sfx2/app.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfxbasic.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/request.hxx>
#include <sfx2/mnumgr.hxx>
#include "statcach.hxx"
#include <sfx2/macrconf.hxx>
#include <sfx2/msgpool.hxx>

//====================================================================

DBG_NAME(SfxShell)

//====================================================================

TYPEINIT0(SfxShell);

//====================================================================
typedef SfxSlot* SfxSlotPtr;
SV_DECL_PTRARR_DEL( SfxVerbSlotArr_Impl, SfxSlotPtr, 4, 4)
SV_IMPL_PTRARR( SfxVerbSlotArr_Impl, SfxSlotPtr);

using namespace com::sun::star;

//=========================================================================
// SfxShell_Impl
//=========================================================================
struct SfxShell_Impl: public SfxBroadcaster
{
    String                      aObjectName;// Name des Sbx-Objects
    SfxItemArray_Impl           aItems;     // Datenaustausch auf Item-Basis
    SfxViewShell*               pViewSh;    // SfxViewShell falls Shell ViewFrame/ViewShell/SubShell ist
    SfxViewFrame*               pFrame;     // Frame, falls <UI-aktiv>
    SfxRepeatTarget*            pRepeatTarget;
    BOOL                        bInAppBASIC;
    BOOL                        bActive;
    ULONG                       nDisableFlags;
    ULONG                       nHelpId;
    svtools::AsynchronLink*     pExecuter;
    svtools::AsynchronLink*     pUpdater;
    SfxVerbSlotArr_Impl         aSlotArr;
    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > aVerbList;
    SfxShell_Impl()  : pExecuter( 0 ), pUpdater( 0 ) {}
    ~SfxShell_Impl() { delete pExecuter; delete pUpdater;}
};

//====================================================================
#ifdef DBG_UTIL

String SfxShellIdent_Impl( const SfxShell *pSh )

/*  [Beschreibung]

    Interne Hilfesfunktion. Liefert einen die SfxShell 'pSh' beschreibenden
    String zur"uck. Z.B.: SfxApplication[StarWriter]
*/

{
    String aIdent( pSh->ISA(SfxApplication) ? DEFINE_CONST_UNICODE("SfxApplication") :
                   pSh->ISA(SfxViewFrame) ? DEFINE_CONST_UNICODE("SfxViewFrame") :
                   pSh->ISA(SfxViewShell) ? DEFINE_CONST_UNICODE("SfxViewShell") :
                   pSh->ISA(SfxObjectShell) ? DEFINE_CONST_UNICODE("SfxObjectShell") : DEFINE_CONST_UNICODE("SfxShell") );
    aIdent += '[';
    aIdent += pSh->GetName();
    aIdent += ']';
    return aIdent;
}

#endif
//====================================================================

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

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxShell initialisierung nur einfache
    Typen, das dazugeh"orige SbxObject wird erst on-demand erzeugt.
    Daher ist das Anlegen einer SfxShell Instanz sehr billig.
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
    pImp->bInAppBASIC = FALSE;
    pImp->nHelpId = 0L;
    pImp->bActive = FALSE;
    pImp->nDisableFlags = 0;
}

//-------------------------------------------------------------------------

SfxShell::SfxShell( SfxViewShell *pViewSh )

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxShell initialisierung nur einfache
    Typen, das dazugeh"orige SbxObject wird erst on-demand erzeugt.
    Daher ist das Anlegen einer SfxShell Instanz sehr billig.
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
    pImp->bInAppBASIC = FALSE;
    pImp->nHelpId = 0L;
    pImp->bActive = FALSE;
}

//--------------------------------------------------------------------

SfxShell::~SfxShell()

/*  [Beschreibung]

    Die Verbindungs zu einem ggf. zugeh"origen SbxObject wird gel"ost.
    Das SbxObject existiert ggf. weiter, kann aber keine Funktionen
    mehr ausf"uhren und keine Properties mehr bereitstellen.
*/

{
    DBG_DTOR(SfxShell, 0);
    delete pImp;
}

//--------------------------------------------------------------------

void SfxShell::SetName( const String &rName )

/*  [Beschreibung]

    Setzt den Namen des Shell-Objekts. Mit diesem Namen kann die
    SfxShell-Instanz vom BASIC aus angesprochen werden.
*/

{
    pImp->aObjectName = rName;
}

//--------------------------------------------------------------------

const String& SfxShell::GetName() const

/*  [Beschreibung]

    Liefert den Namen des Shell-Objekts. Mit diesem Namen kann die
    SfxShell-Instanz vom BASIC aus angesprochen werden.
*/

{
    return pImp->aObjectName;
}

//--------------------------------------------------------------------

SvGlobalName SfxShell::GetGlobalName() const

/*  [Beschreibung]

    Liefert den Global Unique Identifier des Shell-Objekts. Mit diesem
    Namen kann die SfxShell-Instanz z.B. via OLE Automation angesprochen
    werden, bzw. in der Registration-Database gefunden werden.
*/

{
    return SvGlobalName();
}

//--------------------------------------------------------------------

SfxDispatcher* SfxShell::GetDispatcher() const

/*  [Beschreibung]

    Diese Methode liefert einen Pointer auf den <SfxDispatcher>, in
    dem die SfxShell gerade <UI-aktiv> ist bzw. einen 0-Pointer, wenn
    sie gerade nicht UI-aktiv ist.

    Der zur"uckgegebene Pointer ist nur im unmittelbaren Kontext des
    Methodenaufrufs g"ultig.
*/

{
    return pImp->pFrame ? pImp->pFrame->GetDispatcher() : 0;
}

//--------------------------------------------------------------------

SfxViewShell* SfxShell::GetViewShell() const

/*  [Beschreibung]

    Liefert bei SubShells die SfxViewShell, in der sie liegen. Sonst und
    falls nicht vom App-Entwickler angegeben liefert diese Methode 0.
*/

{
    return pImp->pViewSh;
}

//--------------------------------------------------------------------

SfxViewFrame* SfxShell::GetFrame() const

/*  [Beschreibung]

    Diese Methode liefert einen Pointer auf den <SfxViewFrame>, dem diese
    SfxShell-Instanz zugeordnet ist oder in dem sie zur Zeit <UI-aktiv> ist.
    Ein 0-Pointer wird geliefert, wenn diese SfxShell-OInstanz gerade nicht
    UI-aktiv ist und auch keinem SfxViewFrame fest zugeordnet ist.

    Der zur"uckgegebene Pointer ist nur im unmittelbaren Kontext des
    Methodenaufrufs g"ultig.


    [Anmerkung]

    Nur Instanzen von Subklasse von SfxApplication und SfxObjectShell sollten
    hier einen 0-Pointer liefern. Ansonsten liegt ein Fehler im Anwendungs-
    programm vor (falscher Ctor von SfxShell gerufen).


    [Querverweise]

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
    USHORT  nSlotId         // Slot-Id des zu erfragenden <SfxPoolItem>s
)   const

/*  [Beschreibung]

    Mit dieser Methode kann auf beliebige Objekte von Subklassen von
    <SfxPoolItem> zugegriffen werden. Diese Austauschtechnik wird ben"otigt,
    wenn z.B. spezielle <SfxToolBoxControl> Subklassen Zugriff auf
    bestimmte Daten z.B. der <SfxObjectShell> ben"otigen.

    Die zur"uckgelieferte Instanz geh"ort der jeweilige SfxShell und
    darf nur im unmittelbaren Kontext des Methodenaufrufs verwendet werden.


    [Querverweise]

    <SfxShell::PutItem(const SfxPoolItem&)>
    <SfxShell::RemoveItem(USHORT)>
*/

{
    for ( USHORT nPos = 0; nPos < pImp->aItems.Count(); ++nPos )
        if ( pImp->aItems.GetObject(nPos)->Which() == nSlotId )
            return pImp->aItems.GetObject(nPos);
    return 0;
}

//--------------------------------------------------------------------

void SfxShell::RemoveItem
(
    USHORT  nSlotId         // Slot-Id des zu l"oschenden <SfxPoolItem>s
)

/*  [Beschreibung]

    Mit dieser Methode k"onnen die allgemein zur Verf"ugung gestellten
    Instanzen von Subklassen von <SfxPoolItem> aus der SfxShell entfernt
    werden.

    Die gespeicherte Instanz wird gel"oscht.


    [Querverweise]

    <SfxShell::PutItem(const SfxPoolItem&)>
    <SfxShell::GetItem(USHORT)>
*/

{
    for ( USHORT nPos = 0; nPos < pImp->aItems.Count(); ++nPos )
        if ( pImp->aItems.GetObject(nPos)->Which() == nSlotId )
        {
            // Item entfernen und l"oschen
            SfxPoolItem *pItem = pImp->aItems.GetObject(nPos);
            delete pItem;
            pImp->aItems.Remove(nPos);

            // falls aktiv Bindings benachrichtigen
            SfxDispatcher *pDispat = GetDispatcher();
            if ( pDispat )
            {
                SfxVoidItem aVoid( nSlotId );
                pDispat->GetBindings()->Broadcast( SfxPoolItemHint( &aVoid ) );
            }
        }
}

//--------------------------------------------------------------------

void SfxShell::PutItem
(
    const SfxPoolItem&  rItem   /*  Instanz, von der eine Kopie erstellt wird,
                                    die in der SfxShell in einer Liste
                                    gespeichert wird. */
)

/*  [Beschreibung]

    Mit dieser Methode k"onnen beliebige Objekte von Subklassen von
    <SfxPoolItem> zur Verf"ugung gestellt werden. Diese Austauschtechnik
    wird ben"otigt, wenn z.B. spezielle <SfxToolBoxControl> Subklassen
    Zugriff auf bestimmte Daten z.B. der <SfxObjectShell> ben"otigen.

    Falls ein SfxPoolItem mit derselben Slot-Id exisitert, wird dieses
    automatisch gel"oscht.


    [Querverweise]

    <SfxShell::RemoveItem(USHORT)>
    <SfxShell::GetItem(USHORT)>
*/

{
    DBG_ASSERT( !rItem.ISA(SfxSetItem), "SetItems aren't allowed here" );
    DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
                "items with Which-Ids aren't allowed here" );

    // MSC auf WNT/W95 machte hier Mist, Vorsicht bei Umstellungen
    const SfxPoolItem *pItem = rItem.Clone();
    SfxPoolItemHint aItemHint( (SfxPoolItem*) pItem );
    const USHORT nWhich = rItem.Which();
    SfxPoolItem **ppLoopItem = (SfxPoolItem**) pImp->aItems.GetData();
    USHORT nPos;
    for ( nPos = 0; nPos < pImp->aItems.Count(); ++nPos, ++ppLoopItem )
    {
        if ( (*ppLoopItem)->Which() == nWhich )
        {
            // Item austauschen
            delete *ppLoopItem;
            pImp->aItems.Remove(nPos);
            pImp->aItems.Insert( (SfxPoolItemPtr) pItem, nPos );

            // falls aktiv Bindings benachrichtigen
            SfxDispatcher *pDispat = GetDispatcher();
            if ( pDispat )
            {
                SfxBindings* pBindings = pDispat->GetBindings();
                pBindings->Broadcast( aItemHint );
                USHORT nSlotId = nWhich; //pItem->GetSlotId();
                SfxStateCache* pCache = pBindings->GetStateCache( nSlotId );
                if ( pCache )
                {
                    pCache->SetState( SFX_ITEM_AVAILABLE, pItem->Clone(), TRUE );
                    pCache->SetCachedState( TRUE );
                }
            }
            return;
        }
    }

    Broadcast( aItemHint );
    pImp->aItems.Insert((SfxPoolItemPtr)pItem, nPos );
}

//--------------------------------------------------------------------

SfxInterface* SfxShell::GetInterface() const

/*  [Beschreibung]

    Mit dieser virtuellen Methode, die durch das Makro <SFX_DECL_INTERFACE>
    von jeder Subclass mit eigenen Slots automatisch "uberladen wird, kann
    auf die zu der Subklasse geh"orende <SfxInterface>-Instanz zugegriffen
    werden.

    Die Klasse SfxShell selbst hat noch kein eigenes SfxInterface
    (keine Slots), daher wird ein 0-Pointer zur"uckgeliefert.
*/

{
    return GetStaticInterface();
}

//--------------------------------------------------------------------

SfxBroadcaster* SfxShell::GetBroadcaster()

/*  [Beschreibung]

    Liefert einen SfxBroadcaster f"ur diese SfxShell-Instanz bis die
    Klasse SfxShell von SfxBroadcaster abgeleitet ist.
*/

{
    return pImp;
}

//--------------------------------------------------------------------

SfxUndoManager* SfxShell::GetUndoManager()

/*  [Beschreibung]

    Jede Subclass von SfxShell kann "uber einen <SfxUndoManager> verf"ugen.
    Dieser kann in den abgeleiteten Klasse mit <SfxShell:SetUndoManager()>
    gesetzt werden.

    Die Klasse SfxShell selbst hat noch keinen SfxUndoManager, es wird
    daher ein 0-Pointer zur"uckgeliefert.
*/

{
    return pUndoMgr;
}

//--------------------------------------------------------------------

void SfxShell::SetUndoManager( SfxUndoManager *pNewUndoMgr )

/*  [Beschreibung]

    Setzt einen <SfxUndoManager> f"ur diese <SfxShell> Instanz. F"ur das
    Undo wird immer nur der Undo-Manager an der jeweils oben auf dem
    Stack des <SfxDispatcher> liegenden SfxShell verwendet.

    Am "ubergebenen <SfxUndoManager> wird automatisch die aktuelle
    Max-Undo-Action-Count Einstellung aus den Optionen gesetzt.

    'pNewUndoMgr' mu\s bis zum Dtor dieser SfxShell-Instanz oder bis
    zum n"achsten 'SetUndoManager()' existieren.
*/

{
    pUndoMgr = pNewUndoMgr;
    if ( pUndoMgr )
        pUndoMgr->SetMaxUndoActionCount( (USHORT) SvtUndoOptions().GetUndoCount() );
}

//--------------------------------------------------------------------

SfxRepeatTarget* SfxShell::GetRepeatTarget() const

/*  [Beschreibung]

    Liefert einen Pointer auf die <SfxRepeatTarget>-Instanz, die
    als RepeatTarget bei SID_REPEAT verwendet wird, wenn der
    von dieser SfxShell gelieferte <SfxUndoManager> angesprochen wird.
    Der R"uckgabewert kann 0 sein.


    [Anmerkung]

    Eine Ableitung von <SfxShell> oder einer ihrer Subklassen von
    <SfxRepeatTarget> ist nicht zu empfehlen, da Compiler-Fehler
    provoziert werden (wegen Call-to-Pointer-to-Member-Function to
    subclass).
*/

{
    return pImp->pRepeatTarget;
}

//--------------------------------------------------------------------

void SfxShell::SetRepeatTarget( SfxRepeatTarget *pTarget )

/*  [Beschreibung]

    Setzt den die <SfxRepeatTarget>-Instanz, die bei SID_REPEAT als
    RepeatTarget verwendet wird, wenn der von dieser SfxShell gelieferte
    <SfxUndoManager> angesprochen wird. Durch 'pTarget==0' wird SID_REPEAT
    f"ur diese SfxShell disabled. Die Instanz '*pTarget' mu\s so lange
    leben, wie sie angemeldet ist.


    [Anmerkung]

    Eine Ableitung von <SfxShell> oder einer ihrer Subklassen von
    <SfxRepeatTarget> ist nicht zu empfehlen, da Compiler-Fehler
    provoziert werden (wegen Call-to-Pointer-to-Member-Function to
    subclass).
*/

{
    pImp->pRepeatTarget = pTarget;
}

//--------------------------------------------------------------------

void SfxShell::Invalidate
(
    USHORT          nId     /* Zu invalidierende Slot-Id oder Which-Id.
                               Falls diese 0 ist (default), werden
                               alle z.Zt. von dieser Shell bedienten
                               Slot-Ids invalidiert. */
)

/*  [Beschreibung]

    Mit dieser Methode k"onnen Slots der Subclasses "uber die Slot-Id
    oder alternativ "uber die Which-Id invalidiert werden. Slot-Ids,
    die von der Subclass ererbt sind, werden ebenfalls invalidert.

    [Querverweise]
    <SfxBindings::Invalidate(USHORT)>
    <SfxBindings::InvalidateAll(BOOL)>
*/

{
    if ( !GetViewShell() )
    {
        DBG_ERROR( "wrong Invalidate method called!" );
        return;
    }

    Invalidate_Impl( GetViewShell()->GetViewFrame()->GetBindings(), nId );
}

void SfxShell::Invalidate_Impl( SfxBindings& rBindings, USHORT nId )
{
    if ( nId == 0 )
    {
        rBindings.InvalidateShell( *this, FALSE );
    }
    else
    {
        const SfxInterface *pIF = GetInterface();
        do
        {
            const SfxSlot *pSlot = pIF->GetSlot(nId);
            if ( pSlot )
            {
                // bei Enum-Slots ist der Master-Slot zu invalidieren
                if ( SFX_KIND_ENUM == pSlot->GetKind() )
                    pSlot = pSlot->GetLinkedSlot();

                // den Slot selbst und ggf. auch alle Slave-Slots invalidieren
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

void SfxShell::DoActivate_Impl( SfxViewFrame *pFrame, BOOL bMDI )

/*  [Beschreibung]

    Diese Methode steuert die Aktivierung der SfxShell-Instanz. Zun"achst
    wird durch Aufruf der virtuellen Methode <SfxShell::Activate(BOOL)>
    der Subclass die M"oglichkeit gegeben, auf das Event zu reagieren.

    Bei bMDI == TRUE wird das zugeh"orige SbxObject 'scharfgeschaltet',
    so da\s Methoden des Objekts unqualifiziert (ohne den Namen des Objekts)
    vom BASIC gefunden werden.
*/

{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
#ifdef DBG_UTIL_VB
        String aMsg("SfxShell::DoActivate() ");
        aMsg += (long)this;
        aMsg += "  ";
        aMsg += GetInterface()->GetName();
        aMsg += " bMDI ";
        if ( bMDI ) aMsg += "MDI";
        DbgTrace( aMsg.GetBuffer() );
#endif

    if ( bMDI )
    {
        // Frame merken, in dem aktiviert wird
        pImp->pFrame = pFrame;
        pImp->bActive = TRUE;
    }

    // Subklasse benachrichtigen
    Activate(bMDI);
}

//--------------------------------------------------------------------

void SfxShell::DoDeactivate_Impl( SfxViewFrame *pFrame, BOOL bMDI )

/*  [Beschreibung]

    Diese Methode steuert die Deaktivierung der SfxShell-Instanz. Bei
    bMDI == TRUE wird zun"achst das SbxObject in einen Status versetzt,
    so da\s Methoden vom BASIC aus nur noch qualifiziert gerufen werden
    k"onnen.

    Dann erh"alt in jedem Fall die Subclass durch Aufruf der virtuellen
    Methode <SfxShell::Deactivate(BOOL)> die M"oglichkeit auf das Event
    zu reagieren.
*/

{
#ifdef DBG_UTIL
    const SfxInterface *p_IF = GetInterface();
    if ( !p_IF )
        return;
#endif
#ifdef DBG_UTIL_VB
        String aMsg("SfxShell::DoDeactivate()");
        aMsg += (long)this;
        aMsg += "  ";
        aMsg += GetInterface()->GetName();
        aMsg += " bMDI ";
        if ( bMDI ) aMsg += "MDI";
        DbgTrace( aMsg.GetBuffer() );
#endif

    // nur wenn er vom Frame kommt (nicht z.B. pop der BASIC-IDE vom AppDisp)
    if ( bMDI && pImp->pFrame == pFrame )
    {
        // austragen
        pImp->pFrame = 0;
        pImp->bActive = FALSE;
    }

    // Subklasse benachrichtigen
    Deactivate(bMDI);
}

//--------------------------------------------------------------------

BOOL SfxShell::IsActive() const
{
    return pImp->bActive;
}

//--------------------------------------------------------------------

void SfxShell::Activate
(
    BOOL    /*bMDI*/        /*  TRUE
                            der <SfxDispatcher>, auf dem die SfxShell sich
                            befindet, ist aktiv geworden oder die SfxShell
                            Instanz wurde auf einen aktiven SfxDispatcher
                            gepusht. (vergl. SystemWindow::IsMDIActivate())

                            FALSE
                            das zum <SfxViewFrame>, auf dessen SfxDispatcher
                            sich die SfxShell Instanz befindet, wurde
                            aktiviert.
                            (z.B. durch einen geschlossenen Dialog) */
)

/*  [Beschreibung]

    Virtuelle Methode, die beim Aktivieren der SfxShell Instanz gerufen
    wird, um den Subclasses die Gelegenheit zu geben, auf das Aktivieren
    zu reagieren.

    Die Basisimplementation ist leer und braucht nicht gerufen zu werden.


    [Querverweise]
    StarView SystemWindow::Activate(BOOL)
*/

{
}

//--------------------------------------------------------------------

void SfxShell::Deactivate
(
    BOOL    /*bMDI*/        /*  TRUE
                            der <SfxDispatcher>, auf dem die SfxShell sich
                            befindet, ist inaktiv geworden oder die SfxShell
                            Instanz wurde auf einen aktiven SfxDispatcher
                            gepoppt. (vergl. SystemWindow::IsMDIActivate())

                            FALSE
                            das zum <SfxViewFrame>, auf dessen SfxDispatcher
                            sich die SfxShell Instanz befindet, wurde
                            deaktiviert. (z.B. durch einen Dialog) */

)

/*  [Beschreibung]

    Virtuelle Methode, die beim Deaktivieren der SfxShell Instanz gerufen
    wird, um den Subclasses die Gelegenheit zu geben, auf das Deaktivieren
    zu reagieren.

    Die Basisimplementation ist leer und braucht nicht gerufen zu werden.


    [Querverweise]
    StarView SystemWindow::Dectivate(BOOL)
*/

{
}

void SfxShell::ParentActivate
(
)

/*  [Beschreibung]

    Ein Parent des <SfxDispatcher>, auf dem die SfxShell sich befindet,
    ist aktiv geworden, oder die SfxShell Instanz wurde auf einen
    <SfxDispatcher> gepusht, dessen parent aktiv ist.

    Die Basisimplementation ist leer und braucht nicht gerufen zu werden.

    [Querverweise]
    SfxShell::Activate()
*/
{
}

//--------------------------------------------------------------------

void SfxShell::ParentDeactivate
(
)

/*  [Beschreibung]

    Der aktive Parent des <SfxDispatcher>, auf dem die SfxShell sich befindet,
    ist deaktiviert worden.

    Die Basisimplementation ist leer und braucht nicht gerufen zu werden.

    [Querverweise]
    SfxShell::Deactivate()
*/
{
}

//--------------------------------------------------------------------

ResMgr* SfxShell::GetResMgr() const

/*  [Beschreibung]

    Diese Methode liefert den ResMgr der <Resource-DLL>, die von der
    SfxShell-Instanz verwendet wird. Ist dies ein 0-Pointer, so
    ist der aktuelle Resource-Manager zu verwenden.
*/

{
    return GetInterface()->GetResMgr();
}

//--------------------------------------------------------------------

bool SfxShell::CanExecuteSlot_Impl( const SfxSlot &rSlot )

/*  [Beschreibung]

    Diese Methode stellt durch Aufruf der Statusfunktion fest,
    ob 'rSlot' aktuell ausgef"uhrt werden kann.
*/
{
    // Slot-Status holen
    SfxItemPool &rPool = GetPool();
    const USHORT nId = rSlot.GetWhich( rPool );
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

/*  [Beschreibung]
    Asynchrones ExecuteSlot fuer das RELOAD
 */

//--------------------------------------------------------------------
const SfxPoolItem* SfxShell::ExecuteSlot( SfxRequest& rReq, BOOL bAsync )
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
    SfxRequest &rReq,           // der weiterzuleitende <SfxRequest>
    const SfxInterface* pIF     // default = 0 bedeutet virtuell besorgen
)

/*  [Beschreibung]

    Diese Methode erm"oglicht das Weiterleiten eines <SfxRequest> an
    die angegebene Basis-<SfxShell>.


    [Beispiel]

    In einer von SfxViewShell abgeleiteten Klasse soll SID_PRINTDOCDIRECT
    abgefangen werden. Unter bestimmten Umst"anden soll vor dem Drucken
    eine Abfrage erscheinen, und der Request soll ggf. abgebrochen werden.

    Dazu ist in der IDL dieser Subklasse der o.g. Slot einzutragen. Die
    Execute-Methode enth"alt dann skizziert:

    void SubViewShell::Exec( SfxRequest &rReq )
    {
        if ( rReq.GetSlot() == SID_PRINTDOCDIRECT )
        {
            'dialog'
            if ( 'condition' )
                ExecuteSlot( rReq, SfxViewShell::GetInterface() );
        }
    }

    Es braucht i.d.R. kein rReq.Done() gerufen zu werden, da das bereits
    die Implementierung der SfxViewShell erledigt bzw. abgebrochen wurde.


    [Querverweise]

    <SfxShell::GetSlotState(USHORT,const SfxInterface*,SfxItemSet*)>
*/

{
    if ( !pIF )
        pIF = GetInterface();

    USHORT nSlot = rReq.GetSlot();
    const SfxSlot* pSlot = NULL;
    if ( nSlot >= SID_VERB_START && nSlot <= SID_VERB_END )
        pSlot = GetVerbSlot_Impl(nSlot);
    if ( !pSlot )
        pSlot = pIF->GetSlot(nSlot);
    if ( !pSlot && SfxMacroConfig::IsMacroSlot( nSlot ) )
    {
        SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo(nSlot);
        if ( pInfo )
            pSlot = pInfo->GetSlot();
    }

    DBG_ASSERT( pSlot, "slot not supported" );

    SfxExecFunc pFunc = pSlot->GetExecFnc();
    if ( pFunc )
        CallExec( pFunc, rReq );

    return rReq.GetReturnValue();
}

//--------------------------------------------------------------------

const SfxPoolItem* SfxShell::GetSlotState
(
    USHORT              nSlotId,    // Slot-Id des zu befragenden Slots
    const SfxInterface* pIF,        // default = 0 bedeutet virtuell besorgen
    SfxItemSet*         pStateSet   // SfxItemSet der Slot-State-Methode
)

/*  [Beschreibung]

    Diese Methode liefert den Status des Slots mit der angegebenen Slot-Id
    "uber das angegebene Interface.

    Ist der Slot disabled oder in dieser SfxShell (und deren Parent-Shells)
    nicht bekannt, wird ein 0-Pointer zur"uckgeliefert.

    Hat der Slot keinen Status, wird ein SfxVoidItem zur"uckgeliefert.

    Der Status wird bei pStateSet != 0 gleich in diesem Set gesetzt, so
    da\s <SfxShell>-Subklassen Slots-"uberladen und auch bei der
    Status-Methode die Basis-Implementierung rufen k"onnen.


    [Beispiel]

    In einer von SfxViewShell abgeleiteten Klasse soll SID_PRINTDOCDIRECT
    abgefangen werden. Unter bestimmten Umst"anden soll vor dem Drucken
    eine Abfrage erscheinen, und der Request soll ggf. abgebrochen werden.

    Dazu ist in der IDL dieser Subklasse der o.g. Slot einzutragen. Die
    Status-Methode enth"alt dann skizziert:

    void SubViewShell::PrintState( SfxItemSet &rState )
    {
        if ( rState.GetItemState( SID_PRINTDOCDIRECT ) != SFX_ITEM_UNKNOWN )
            GetSlotState( SID_PRINTDOCDIRECT, SfxViewShell::GetInterface(),
                    &rState );
        ...
    }


    [Querverweise]

    <SfxShell::ExecuteSlot(SfxRequest&)>
*/

{
    // Slot am angegebenen Interface besorgen
    if ( !pIF )
        pIF = GetInterface();
    SfxItemState eState;
    SfxItemPool &rPool = GetPool();

    const SfxSlot* pSlot = NULL;
    if ( nSlotId >= SID_VERB_START && nSlotId <= SID_VERB_END )
        pSlot = GetVerbSlot_Impl(nSlotId);
    if ( !pSlot )
        pSlot = pIF->GetSlot(nSlotId);
    if ( !pSlot && SfxMacroConfig::IsMacroSlot( nSlotId ) )
    {
        SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo(nSlotId);
        if ( pInfo )
            pSlot = pInfo->GetSlot();
    }

    if ( pSlot )
        // ggf. auf Which-Id mappen
        nSlotId = pSlot->GetWhich( rPool );

    // Item und Item-Status besorgen
    const SfxPoolItem *pItem = NULL;
    SfxItemSet aSet( rPool, nSlotId, nSlotId ); // pItem stirbt sonst zu fr"uh
    if ( pSlot )
    {
        // Status-Methode rufen
        SfxStateFunc pFunc = pSlot->GetStateFnc();
        if ( pFunc )
            CallState( pFunc, aSet );
        eState = aSet.GetItemState( nSlotId, TRUE, &pItem );

        // ggf. Default-Item besorgen
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

    // Item und Item-Status auswerten und ggf. in pStateSet mitpflegen
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

    DBG_ASSERT(pViewSh, "SetVerbs nur an der ViewShell aufrufen!");
    if ( !pViewSh )
        return;

    // Zun"achst alle Statecaches dirty machen, damit keiner mehr versucht,
    // die Slots zu benutzen
    {
        SfxBindings *pBindings =
            pViewSh->GetViewFrame()->GetDispatcher()->GetBindings();
        USHORT nCount = pImp->aSlotArr.Count();
        for (USHORT n1=0; n1<nCount ; n1++)
        {
            USHORT nId = SID_VERB_START + n1;
            pBindings->Invalidate(nId, FALSE, TRUE);
        }
    }

    USHORT nr=0;
    for (sal_Int32 n=0; n<aVerbs.getLength(); n++)
    {
        USHORT nSlotId = SID_VERB_START + nr++;
        DBG_ASSERT(nSlotId <= SID_VERB_END, "Zuviele Verben!");
        if (nSlotId > SID_VERB_END)
            break;

        SfxSlot *pNewSlot = new SfxSlot;
        pNewSlot->nSlotId = nSlotId;
        pNewSlot->nGroupId = 0;

        // Verb-Slots m"ussen asynchron ausgef"uhrt werden, da sie w"ahrend
        // des Ausf"uhrens zerst"ort werden k"onnten
        pNewSlot->nFlags = SFX_SLOT_ASYNCHRON | SFX_SLOT_CONTAINER;
        pNewSlot->nMasterSlotId = 0;
        pNewSlot->nValue = 0;
        pNewSlot->fnExec = SFX_STUB_PTR(SfxShell,VerbExec);
        pNewSlot->fnState = SFX_STUB_PTR(SfxShell,VerbState);
        pNewSlot->pType = 0; HACK(SFX_TYPE(SfxVoidItem))
        pNewSlot->pName = U2S(aVerbs[n].VerbName);
        pNewSlot->pLinkedSlot = 0;
        pNewSlot->nArgDefCount = 0;
        pNewSlot->pFirstArgDef = 0;
        pNewSlot->pUnoName = 0;

        if (pImp->aSlotArr.Count())
        {
            SfxSlot *pSlot = (pImp->aSlotArr)[0];
            pNewSlot->pNextSlot = pSlot->pNextSlot;
            pSlot->pNextSlot = pNewSlot;
        }
        else
            pNewSlot->pNextSlot = pNewSlot;

        pImp->aSlotArr.Insert(pNewSlot, (USHORT) n);
    }

    pImp->aVerbList = aVerbs;

    if (pViewSh)
    {
        // Der Status von SID_OBJECT wird im Controller direkt an der Shell
        // abgeholt, es reicht also, ein neues StatusUpdate anzuregen
        SfxBindings *pBindings = pViewSh->GetViewFrame()->GetDispatcher()->
                GetBindings();
        pBindings->Invalidate( SID_OBJECT, TRUE, TRUE );
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
    USHORT nId = rReq.GetSlot();
    SfxViewShell *pViewShell = GetViewShell();
    if ( pViewShell )
    {
        BOOL bReadOnly = pViewShell->GetObjectShell()->IsReadOnly();
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

const SfxSlot* SfxShell::GetVerbSlot_Impl(USHORT nId) const
{
    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > rList = pImp->aVerbList;

    DBG_ASSERT(nId >= SID_VERB_START && nId <= SID_VERB_END,"Falsche VerbId!");
    USHORT nIndex = nId - SID_VERB_START;
    DBG_ASSERT(nIndex < rList.getLength(),"Falsche VerbId!");

    if (nIndex < rList.getLength())
        return pImp->aSlotArr[nIndex];
    else
        return 0;
}

//--------------------------------------------------------------------

void SfxShell::SetHelpId(ULONG nId)
{
    pImp->nHelpId = nId;
}

//--------------------------------------------------------------------

ULONG SfxShell::GetHelpId() const
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
    ((SfxDispatcher*) pArg)->Update_Impl( TRUE );
    ((SfxDispatcher*) pArg)->GetBindings()->InvalidateAll(FALSE);
    return 0;
}

void SfxShell::UIFeatureChanged()
{
    SfxViewFrame *pFrame = GetFrame();
    if ( pFrame && pFrame->IsVisible() )
    {
        // Auch dann Update erzwingen, wenn Dispatcher schon geupdated ist,
        // sonst bleibt evtl. irgendwas in den gebunkerten Tools stecken.
        // Asynchron aufrufen, um Rekursionen zu vermeiden
        if ( !pImp->pUpdater )
            pImp->pUpdater = new svtools::AsynchronLink( Link( this, DispatcherUpdate_Impl ) );

        // Mehrfachaufrufe gestattet
        pImp->pUpdater->Call( pFrame->GetDispatcher(), TRUE );
    }
}

void SfxShell::SetDisableFlags( ULONG nFlags )
{
    pImp->nDisableFlags = nFlags;
}

ULONG SfxShell::GetDisableFlags() const
{
    return pImp->nDisableFlags;
}

SfxItemSet* SfxShell::CreateItemSet( USHORT )
{
    return NULL;
}

void SfxShell::ApplyItemSet( USHORT, const SfxItemSet& )
{
}

void SfxShell::SetViewShell_Impl( SfxViewShell* pView )
{
    pImp->pViewSh = pView;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
