/*************************************************************************
 *
 *  $RCSfile: shell.cxx,v $
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

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _PSEUDO_HXX //autogen
#include <so3/pseudo.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _SVTOOLS_ITEMDEL_HXX
#include <svtools/itemdel.hxx>
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#include <svtools/undoopt.hxx>

#pragma hdrstop

#include "shell.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "sfxbasic.hxx"
#include "objface.hxx"
#include "objsh.hxx"
#include "viewsh.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "request.hxx"
#include "mnumgr.hxx"
#include "statcach.hxx"

//====================================================================

DBG_NAME(SfxShell);

//====================================================================

TYPEINIT0(SfxShell);

//====================================================================
typedef SfxSlot* SfxSlotPtr;
SV_DECL_PTRARR_DEL( SfxVerbSlotArr_Impl, SfxSlotPtr, 4, 4);
SV_IMPL_PTRARR( SfxVerbSlotArr_Impl, SfxSlotPtr);

struct SfxVerbList
{
    SfxVerbSlotArr_Impl       aSlotArr;
    SvVerbList                aVerbList;
};

struct SfxShell_Impl: public SfxBroadcaster
{
    String                      aObjectName;// Name des Sbx-Objects
    SfxItemArray_Impl           aItems;     // Datenaustausch auf Item-Basis
    SfxViewShell*               pViewSh;    // SfxViewShell falls Shell ViewFrame/ViewShell/SubShell ist
    SfxViewFrame*               pFrame;     // Frame, falls <UI-aktiv>
    SfxVerbList*                pVerbs;
    SfxRepeatTarget*            pRepeatTarget;
//    SbxObjectRef                xParent;
    BOOL                        bInAppBASIC;
    BOOL                        bActive;
    ULONG                       nDisableFlags;
    ULONG                       nHelpId;
    AsynchronLink*              pExecuter;
    AsynchronLink*              pUpdater;
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
    aIdent += 0x005B; // '[' = 5Bh
    aIdent += pSh->GetName();
    aIdent += 0x005D; // ']' = 5Dh
    return aIdent;
}

#endif
//====================================================================

void __EXPORT SfxShell::EmptyExecStub(SfxShell *, SfxRequest &)
{
}

void __EXPORT SfxShell::EmptyStateStub(SfxShell *, SfxItemSet &)
{
}

SfxShell::SfxShell()

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxShell initialisierung nur einfache
    Typen, das dazugeh"orige SbxObject wird erst on-demand erzeugt.
    Daher ist das Anlegen einer SfxShell Instanz sehr billig.
*/

:   pPool(0),
    pUndoMgr(0),
    pImp(0)
{
    DBG_CTOR(SfxShell, 0);
    pImp = new SfxShell_Impl;
    pImp->pViewSh = 0;
    pImp->pFrame = 0;
    pImp->pVerbs = 0;
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

:   pPool(0),
    pUndoMgr(0),
    pImp(0)
{
    DBG_CTOR(SfxShell, 0);
    pImp = new SfxShell_Impl;
    pImp->pViewSh = pViewSh;
    pImp->pFrame = 0;
    pImp->pVerbs = 0;
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
    delete pImp->pVerbs;
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
                pDispat->GetBindings()->Broadcast( aItemHint );
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
    return 0;
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
        pUndoMgr->SetMaxUndoActionCount( SvtUndoOptions().GetUndoCount() );
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
#if SUPD>603
        DBG_ERROR( "wrong Invalidate method called!" );
#endif
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

void SfxShell::DoActivate( SfxViewFrame *pFrame, BOOL bMDI )

/*  [Beschreibung]

    Diese Methode steuert die Aktivierung der SfxShell-Instanz. Zun"achst
    wird durch Aufruf der virtuellen Methode <SfxShell::Activate(BOOL)>
    der Subclass die M"oglichkeit gegeben, auf das Event zu reagieren.

    Bei bMDI == TRUE wird das zugeh"orige SbxObject 'scharfgeschaltet',
    so da\s Methoden des Objekts unqualifiziert (ohne den Namen des Objekts)
    vom BASIC gefunden werden.
*/

{
#ifndef PRODUCT
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

void SfxShell::DoDeactivate( SfxViewFrame *pFrame, BOOL bMDI )

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
#ifndef PRODUCT
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
    BOOL    bMDI        /*  TRUE
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
    BOOL    bMDI        /*  TRUE
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
#if 0
// falls noch mal "pure virtual function called" auf dem MAC:
//  Bitte MAC nicht mehr unterstuetzen
// der MPW-Linker will die immer haben, obwohl pure virtual

#ifdef MAC
extern "C"
{
    void debugstr( char *pMsg );
}
#endif

SfxItemPool& SfxShell::GetPool()
{
    debugstr( "pure virtual function called: SfxShell::GetPool()" );
    SfxItemPool aPool(0,0);
    return aPool;
}

#endif

//--------------------------------------------------------------------

void SfxShell::FillStatusBar( StatusBar& )

/*  [Beschreibung]

    Mit dieser Factory-Methode kann der Applikationsprogrammierer einer
    SfxShell-Subklasse eine eigene Statusbar zuweisen. Der SFx bevorzugt
    jeweils die Statusbar einer weiter oben auf den <SfxDispatcher>-Stack
    liegenden SfxShell.

    Die Basisimplementation ist leer und braucht nicht gerufen zu werden.

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

FASTBOOL SfxShell::CanExecuteSlot_Impl( const SfxSlot &rSlot )

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
#if 0
ULONG SfxShell::ExecuteSlot
(
    USHORT              nSlot,      // In: auszuf"uhrender Slot
    USHORT              nMemberId,  // In: MemberId f"ur strukturierte Items
    SbxVariable&        rRet,       // Out: Returnwert
    SbxBase*            pArgs       // In: 0, einzelner Parameter/Parameterliste
)

/*  [Beschreibung]

    Methode zum Ausf"uhren eines <SfxSlot>s von einem SfxShellObject
    (BASIC oder andere API).

    Aufrufe von APIs werden nicht recorded (ist ja auch kein <SfxDispatcher>).


    [R"uckgabewert]

    ULONG                       ErrorCode
*/

{
    // Slot suchen
    const SfxSlot *pSlot = GetInterface()->GetSlot(nSlot);
    if ( !pSlot )
        return SbxERR_PROC_UNDEFINED;

    // ggf. disabled
    if ( !pSlot->IsMode(SFX_SLOT_FASTCALL) && !CanExecuteSlot_Impl(*pSlot) )
        return SbERR_METHOD_FAILED;

    // "ofters ben"otigte Werte
    SfxItemPool &rPool = GetPool();
    SfxMapUnit eUserMetric = SFX_APP()->GetOptions().GetUserMetric();

    // Returnwert zum sp"ateren Aufbereiten als SbxVariable merken
    BOOL bDone = FALSE; // Return nur auswerten, wenn nicht abgebrochen wurde
    const SfxPoolItem *pRetItem = 0;
    SbxObject *pOwnObject = GetSbxObject(); // Hack fuer #22783

    // aktuelle Parameter oder Property (impliziter Parameter)?
    if ( pArgs || pSlot->IsMode(SFX_SLOT_PROPSET) )
    {
        // aktuelle Parameter in diesem ItemSet sammeln
        SfxAllItemSet aSet( rPool );

        // mehrere Parameter?
        if ( pSlot->IsMode(SFX_SLOT_METHOD) )
        {
            // das kann nur ein Methoden-Slot sein
            DBG_ASSERT( !pArgs || pArgs->ISA(SbxArray), "not an SbxArray as arguments" );

            // "uber die formalen Parameter iterieren
            USHORT nFormalArgs = pSlot->GetFormalArgumentCount();
            USHORT nActualArgs = pArgs ? ((SbxArray*)pArgs)->Count() - 1 : 0;
            USHORT nBasicArg = 0;
            for ( USHORT nArg = 0;
                  nArg < nFormalArgs && nBasicArg < nActualArgs;
                  ++nArg )
            {
                // formale Paramterbeschreibung besorgen
                const SfxFormalArgument &rArg = pSlot->GetFormalArgument(nArg);

                // Item per Factory erzeugen
                SfxPoolItem *pItem = rArg.CreateItem();
                DBG_ASSERT( pItem, "item without factory" );
                pItem->SetWhich( rPool.GetWhich(rArg.nSlotId) );

                // Konvertieren / ggf. strukturierte Items zusammensetzen
                ULONG eErr = 0;
                USHORT nSubCount = rArg.pType->nAttribs;
                if ( !nSubCount )
                {
                    // einfaches Item
                    SbxVariable *pSubArg = ((SbxArray*)pArgs)->Get(++nBasicArg);
                    eErr = rPool.SetVariable( *pItem, *pSubArg, eUserMetric );
                }
                else
                {
                    // strukturiertes Item
                    for ( USHORT n = 1; n <= nSubCount && !eErr; ++n )
                    {
                        SbxVariable *pSubArg = ((SbxArray*)pArgs)->Get(++nBasicArg);
                        pSubArg->SetUserData(
                                    long(rArg.pType->aAttrib[n-1].nAID) << 20 );
                        eErr = rPool.SetVariable( *pItem, *pSubArg, eUserMetric );
                    }
                }

                // falls Item erzeugt werden konnte, an Request anh"angen
                if ( 0 == eErr )
                    aSet.Put( *pItem,pItem->Which() );
                delete pItem; //! Optimieren (Put mit direkter Uebernahme)
                if ( 0 != eErr )
                    return eErr;
            }

            // nicht alle Parameter abger"aumt (zuviele)?
            if ( nBasicArg < nActualArgs )
                return SbxERR_WRONG_ARGS;
        }
        else
        {
            // Instanz von SfxPoolItem-Subklasse besorgen/erzeugen
            SfxPoolItem *pItem = 0;

            // Teil eines strukturiertes Items?
            if ( nMemberId )
            {
                //  => Status-Item besorgen
                const SfxPoolItem *pState = GetSlotState(nSlot);
                if ( pState && !pState->ISA(SfxVoidItem) )
                    pItem = pState->Clone();
            }

            // kein StatusItem oder einfaches/komplexes Item
            if ( !pItem )
            {
                // per Factory erzeugen
                pItem = pSlot->GetType()->CreateItem();
                DBG_ASSERT( pItem, "item without factory" );
                pItem->SetWhich( rPool.GetWhich(nSlot) );
            }

            // Daten aus Variable in das Item "ubertragen
            ULONG eErr = rPool.SetVariable( *pItem, rRet, eUserMetric );
            if ( 0 == eErr )
                aSet.Put( *pItem,pItem->Which() );
            delete pItem; //! Optimieren (Put mit direkter Uebernahme)
            if ( 0 != eErr )
                return eErr;
        }

        // via Request mit 'normaler' Execute-Methode ausf"uhren
        SfxRequest aReq( nSlot, SFX_CALLMODE_API, aSet );
        aSet.ClearItem(); // pPool k"onnte in _Execute sterben
        SfxExecFunc pFunc = pSlot->GetExecFnc();
        CallExec( pFunc, aReq );
        bDone = aReq.IsDone();
        pRetItem = aReq.GetReturnValue();
    }
    else
    {
        // ohne Parameter direkt ausf"uhren
        SfxRequest aReq( nSlot, SFX_CALLMODE_API, GetPool() );
//      SfxRequest aReq( nSlot, GetPool(), TRUE );
        SfxExecFunc pFunc = pSlot->GetExecFnc();
        CallExec( pFunc, aReq );
        bDone = aReq.IsDone();
        pRetItem = aReq.GetReturnValue();
    }

    // in IDL ein Return-Wert angegeben?
    if ( pSlot->IsMode(SFX_SLOT_METHOD) && rRet.GetType() != SbxVOID )
    {
        // ist aber kein Returnwert gesetzt worden?
        if ( !pRetItem || pRetItem->ISA(SfxVoidItem) )
        {
            // Ignore ohne Parameter ist erlaubt und auch normal
            if ( !bDone )
                return 0;

            // sonst mu\s einer gesetzt worden sein
            DBG_ERROR( "IDL hat Return-Wert, Execute-Methode setzt aber keinen!" );
            return SbxERR_CONVERSION;
        }

        // Returnwert in SbxVariable packen
        HACK( #22783 - lebt *this noch? )
        if ( ((SfxShellObject*)pOwnObject)->GetShell() )
            GetPool().FillVariable( *pRetItem, rRet, eUserMetric );
        else
            pRetItem->FillVariable( rRet, eUserMetric, eUserMetric );
    }

    return 0;
}
#endif

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
            pImp->pExecuter = new AsynchronLink(
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
    const SfxSlot *pSlot = pIF->GetSlot(rReq.GetSlot());
    DBG_ASSERT( pSlot, "slot not supported" );

    SfxExecFunc pFunc = pSlot->GetExecFnc();
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
    const SfxSlot *pSlot = pIF->GetSlot(nSlotId);
    if ( pSlot )
        // ggf. auf Which-Id mappen
        nSlotId = pSlot->GetWhich( rPool );

    // Item und Item-Status besorgen
    const SfxPoolItem *pItem;
    SfxItemSet aSet( rPool, nSlotId, nSlotId ); // pItem stirbt sonst zu fr"uh
    if ( pSlot )
    {
        // Status-Methode rufen
        SfxStateFunc pFunc = pSlot->GetStateFnc();
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


void SfxShell::SetVerbs(const SvVerbList* pVerbs)
{
    SfxViewShell *pViewSh = PTR_CAST ( SfxViewShell, this);

    DBG_ASSERT(pViewSh, "SetVerbs nur an der ViewShell aufrufen!");

    if (pImp->pVerbs)
    {
        // Zun"achst alle Statecaches dirty machen, damit keiner mehr versucht,
        // die Slots zu benutzen

        SfxBindings *pBindings =
            pViewSh->GetViewFrame()->GetDispatcher()->GetBindings();
        USHORT nCount = pImp->pVerbs->aSlotArr.Count();
        for (USHORT n=0; n<nCount ; n++)
        {
            USHORT nId = SID_VERB_START + n;
            pBindings->Invalidate(nId, FALSE, TRUE);
        }

        DELETEZ (pImp->pVerbs);
    }

    if (pVerbs)
        pImp->pVerbs = new SfxVerbList();

    if (pVerbs)
    {
        USHORT nr=0;
        for (USHORT n=0; n<pVerbs->Count(); n++)
        {
            SfxSlot *pNewSlot = new SfxSlot;
            USHORT nSlotId = SID_VERB_START + nr++;
            DBG_ASSERT(nSlotId <= SID_VERB_END, "Zuviele Verben!");
            if (nSlotId > SID_VERB_END)
                break;

            pNewSlot->nSlotId = nSlotId;
            pNewSlot->nGroupId = 0;

            // Verb-Slots m"ussen asynchron ausgef"uhrt werden, da sie w"ahrend
            // des Ausf"uhrens zerst"ort werden k"onnten
            pNewSlot->nFlags = SFX_SLOT_ASYNCHRON;
            pNewSlot->nMasterSlotId = 0;
            pNewSlot->nValue = 0;
            pNewSlot->fnExec = SFX_STUB_PTR(SfxShell,VerbExec);
            pNewSlot->fnState = SFX_STUB_PTR(SfxShell,VerbState);
            pNewSlot->pType = 0; HACK(SFX_TYPE(SfxVoidItem))
            pNewSlot->pName = U2S((*pVerbs)[n].GetName()).getStr();
            pNewSlot->pLinkedSlot = 0;
            pNewSlot->nArgDefCount = 0;
            pNewSlot->pFirstArgDef = 0;
            pNewSlot->pUnoName = 0;

            if (pImp->pVerbs->aSlotArr.Count())
            {
                SfxSlot *pSlot = (pImp->pVerbs->aSlotArr)[0];
                pNewSlot->pNextSlot = pSlot->pNextSlot;
                pSlot->pNextSlot = pNewSlot;
            }
            else
                pNewSlot->pNextSlot = pNewSlot;

            pImp->pVerbs->aSlotArr.Insert(pNewSlot, n);
            pImp->pVerbs->aVerbList.Insert((*pVerbs)[n], n);
        }
    }

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

const SvVerbList* SfxShell::GetVerbs() const
{
    return pImp->pVerbs ? &pImp->pVerbs->aVerbList : 0;
}

//--------------------------------------------------------------------

void SfxShell::VerbExec(SfxRequest& rReq)
{
    USHORT nId = rReq.GetSlot();
    SfxViewShell *pViewShell = SfxViewShell::Current();
    const SvVerbList* pList = pViewShell->GetVerbs();

    for (USHORT n=0; n<pList->Count(); n++)
    {
        if (nId == SID_VERB_START + n)
        {
            pViewShell->DoVerb((*pList)[n].GetId());
            rReq.Done();
            return;
        }
    }
}

//--------------------------------------------------------------------

void SfxShell::VerbState(SfxItemSet &rSet)
{
}

//--------------------------------------------------------------------

const SfxSlot* SfxShell::GetVerbSlot_Impl(USHORT nId) const
{
//    DBG_ASSERT(pImp->pVerbs, "Keine Verben bekannt!");
    if (!pImp->pVerbs)
        return 0;

    const SvVerbList& rList = pImp->pVerbs->aVerbList;

    DBG_ASSERT(nId >= SID_VERB_START && nId <= SID_VERB_END,"Falsche VerbId!");
    USHORT nIndex = nId - SID_VERB_START;
    DBG_ASSERT(nIndex < rList.Count(),"Falsche VerbId!");

    if (nIndex < rList.Count())
        return pImp->pVerbs->aSlotArr[nIndex];
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

SfxSlotPool& SfxShell::GetSlotPool_Impl() const
{
    return SFX_APP()->GetSlotPool( GetFrame() );
}

BOOL SfxShell::HasUIFeature( ULONG )
{
    return FALSE;
}

long DispatcherUpdate_Impl( void* pObj, void* pArg )
{
    ((SfxDispatcher*) pArg)->Update_Impl( TRUE );
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
            pImp->pUpdater = new AsynchronLink( Link( this, DispatcherUpdate_Impl ) );

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

SfxItemSet* SfxShell::CreateItemSet( USHORT nId )
{
    return NULL;
}

void SfxShell::ApplyItemSet( USHORT nId, const SfxItemSet& rSet )
{
}

void SfxShell::SetViewShell_Impl( SfxViewShell* pView )
{
    pImp->pViewSh = pView;
}



