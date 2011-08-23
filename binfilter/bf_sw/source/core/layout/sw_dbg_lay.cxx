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

/* -----------------08.01.99 14:55-------------------
 * Und hier die Beschreibung:
 *
 * Durch die PROTOCOL-Makros wird es ermoeglicht, Ereignisse im Frame-Methoden zu protokollieren.
 * In protokollwuerdigen Stellen in Frame-Methoden muss entweder ein PROTOCOL(...) oder bei Methoden,
 * bei denen auch das Verlassen der Methode mitprotokolliert werden soll, ein PROTOCOL_ENTER(...)-Makro
 * stehen.
 * Die Parameter der PROTOCOL-Makros sind
 * 1.	Ein Pointer auf einen SwFrm, also meist "this" oder "rThis"
 * 2.	Die Funktionsgruppe z.B. PROT_MAKEALL, hierueber wird (inline) entschieden, ob dies
 * 		zur Zeit protokolliert werden soll oder nicht.
 * 3.	Die Aktion, im Normalfall 0, aber z.B. ein ACT_START bewirkt eine Einrueckung in der
 * 		Ausgabedatei, ein ACT_END nimmt dies wieder zurueck. Auf diese Art wird z.B. durch
 * 		PROTOCOL_ENTER am Anfang einer Methode eingerueckt und beim Verlassen wieder zurueck.
 * 4.	Der vierte Parameter ist ein void-Pointer, damit man irgendetwas uebergeben kann,
 * 		was in das Protokoll einfliessen kann, typesches Beispiel bei PROT_GROW muss man
 * 		einen Pointer auf den Wert, um den gegrowt werden soll, uebergeben.
 *
 *
 * Das Protokoll ist die Datei "dbg_lay.out" im aktuellen (BIN-)Verzeichnis.
 * Es enthaelt Zeilen mit FrmId, Funktionsgruppe sowie weiteren Infos.
 *
 * Was genau protokolliert wird, kann auf folgende Arten eingestellt werden:
 * 1.	Die statische Variable SwProtokoll::nRecord enthaelt die Funktionsgruppen,
 * 		die aufgezeichnet werden sollen.
 * 		Ein Wert von z.B. PROT_GROW bewirkt, das Aufrufe von SwFrm::Grow dokumentiert werden,
 *		PROT_MAKEALL protokolliert Aufrufe von xxx::MakeAll.
 *		Die PROT_XY-Werte koennen oderiert werden.
 * 		Default ist Null, es wird keine Methode aufgezeichnet.
 * 2.	In der SwImplProtocol-Klasse gibt es einen Filter fuer Frame-Typen,
 * 		nur die Methodenaufrufe von Frame-Typen, die dort gesetzt sind, werden protokolliert.
 *		Der Member nTypes kann auf Werte wie FRM_PAGE, FRM_SECTION gesetzt und oderiert werden.
 * 		Default ist 0xFFFF, d.h. alle Frame-Typen.
 * 3.	In der SwImplProtocol-Klasse gibt es einen ArrayPointer auf FrmIds, die zu ueberwachen sind.
 * 		Ist der Pointer Null, so werden alle Frames protokolliert, ansonsten nur Frames,
 * 		die in dem Array vermerkt sind.
 *
 * Eine Aufzeichnung in Gang zu setzen, erfordert entweder Codemanipulation, z.B. in
 * SwProtocol::Init() einen anderen Default fuer nRecord setzen oder Debuggermanipulation.
 * Im Debugger gibt verschiedene, sich anbietende Stellen:
 * 1.	In SwProtocol::Init() einen Breakpoint setzen und dort nRecord manipulieren, ggf.
 *		FrmIds eintragen, dann beginnt die Aufzeichnung bereits beim Programmstart.
 * 2.	Waehrend des Programmlaufs einen Breakpoint vor irgendein PROTOCOL oder PROTOCOL_ENTER-
 * 		Makro setzen, dann am SwProtocol::nRecord das unterste Bit setzen (PROT_INIT). Dies
 * 		bewirkt, dass die Funktionsgruppe des folgenden Makros aktiviert und in Zukunft
 * 		protokolliert wird.
 * 3.	Spezialfall von 2.: Wenn man 2. in der Methode SwRootFrm::Paint(..) anwendet, werden
 * 		die Aufzeichnungseinstellung aus der Datei "dbg_lay.ini" ausgelesen!
 * 		In dieser INI-Datei kann es Kommentarzeilen geben, diese beginnen mit '#', dann
 * 		sind die Sektionen "[frmid]", "[frmtype]" und "[record]" relevant.
 * 		Nach [frmid] koennen die FrameIds der zu protokollierenden Frames folgen. Gibt es
 * 		dort keine Eintraege, werden alle Frames aufgezeichnet.
 * 		Nach [frmtype] koennen FrameTypen folgen, die aufgezeichnet werden sollen, da der
 * 		Default hier allerdings USHRT_MAX ist, werden sowieso alle aufgezeichnet. Man kann
 * 		allerdings auch Typen entfernen, in dem man ein '!' vor den Wert setzt, z.B.
 * 		!0xC000 nimmt die SwCntntFrms aus der Aufzeichnung heraus.
 * 		Nach [record] folgen die Funktionsgruppen, die aufgezeichnet werden sollen, Default
 * 		ist hier 0, also keine. Auch hier kann man mit einem vorgestellten '!' Funktionen
 * 		wieder entfernen.
 * 		Hier mal ein Beispiel fuer eine INI-Datei:
 * 		------------------------------------------
 * 			#Funktionen: Alle, ausser PRTAREA
 *	 		[record] 0xFFFFFFE !0x200
 * 			[frmid]
 * 			#folgende FrmIds:
 *	 		1 2 12 13 14 15
 * 			#keine Layoutframes ausser ColumnFrms
 * 			[frmtype] !0x3FFF 0x4
 * 		------------------------------------------
 *
 * Wenn die Aufzeichnung erstmal laeuft, kann man in SwImplProtocol::_Record(...) mittels
 * Debugger vielfaeltige Manipulationen vornehmen, z.B. bezueglich FrameTypen oder FrmIds.
 *
 * --------------------------------------------------*/

#ifndef DBG_UTIL
#error Wer fummelt denn an den makefiles rum?
#endif


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "dbg_lay.hxx"


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_LONGS
#endif

#include <stdio.h>


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "txtfrm.hxx"
#include "dflyobj.hxx"
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
namespace binfilter {

/*N*/ ULONG SwProtocol::nRecord = 0;
/*N*/ SwImplProtocol* SwProtocol::pImpl = NULL;


/*N*/ class SwImplProtocol
/*N*/ {
/*N*/ public:
    SwImplProtocol(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SwImplProtocol();
/*N*/ };

/* -----------------11.01.99 10:43-------------------
 * Durch das PROTOCOL_ENTER-Makro wird ein SwEnterLeave-Objekt erzeugt,
 * wenn die aktuelle Funktion aufgezeichnet werden soll, wird ein
 * SwImplEnterLeave-Objekt angelegt. Der Witz dabei ist, das der Ctor
 * des Impl-Objekt am Anfang der Funktion und automatisch der Dtor beim
 * Verlassen der Funktion gerufen wird. In der Basis-Implementierung ruft
 * der Ctor lediglich ein PROTOCOL(..) mit ACT_START und im Dtor ein
 * PROTOCOL(..) mit ACT_END.
 * Es lassen sich Ableitungen der Klasse bilden, um z.B. beim Verlassen
 * einer Funktion Groessenaenderungen des Frames zu dokumentieren u.v.a.m.
 * Dazu braucht dann nur noch in SwEnterLeave::Ctor(...) die gewuenschte
 * SwImplEnterLeave-Klasse angelegt zu werden.
 *
 * --------------------------------------------------*/






// Die folgende Funktion wird beim Anziehen der Writer-DLL durch TxtInit(..) aufgerufen
// und ermoeglicht dem Debuggenden Funktionen und/oder FrmIds freizuschalten

/*N*/ void SwProtocol::Init()
/*N*/ {
/*N*/ 	nRecord = 0;
/*N*/ 	XubString aName( "dbg_lay.go", RTL_TEXTENCODING_MS_1252 );
/*N*/ 	SvFileStream aStream( aName, STREAM_READ );
/*N*/ 	if( aStream.IsOpen() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pImpl = new SwImplProtocol();
/*N*/ 	}
/*N*/     aStream.Close();
/*N*/ }

// Ende der Aufzeichnung

/*N*/ void SwProtocol::Stop()
/*N*/ {
/*N*/ 	 if( pImpl )
/*N*/ 	 {
/*?*/ 		delete pImpl;
/*?*/ 		pImpl = NULL;
/*?*/         if( pFntCache )
/*?*/             pFntCache->Flush();
/*N*/ 	 }
/*N*/ 	 nRecord = 0;
/*N*/ }

// Creates a more or less detailed snapshot of the layout structur






/* -----------------11.01.99 11:03-------------------
 * SwImplProtocol::CheckLine analysiert eine Zeile der INI-Datei
 * --------------------------------------------------*/


/* -----------------11.01.99 11:17-------------------
 * SwImplProtocol::FileInit() liest die Datei "dbg_lay.ini"
 * im aktuellen Verzeichnis und wertet sie aus.
 * --------------------------------------------------*/

/* -----------------11.01.99 11:20-------------------
 * lcl_Start sorgt fuer Einrueckung um zwei Blanks bei ACT_START
 * und nimmt diese bei ACT_END wieder zurueck.
 * --------------------------------------------------*/

/* -----------------11.01.99 11:21-------------------
 * lcl_Flags gibt das ValidSize-, ValidPos- und ValidPrtArea-Flag ("Sz","Ps","PA")
 * des Frames aus, "+" fuer valid, "-" fuer invalid.
 * --------------------------------------------------*/


/* -----------------11.01.99 11:23-------------------
 * lcl_FrameType gibt den Typ des Frames in Klartext aus.
 * --------------------------------------------------*/


/* -----------------11.01.99 11:25-------------------
 * SwImplProtocol::Record(..) wird nur gerufen, wenn das PROTOCOL-Makro
 * feststellt, dass die Funktion aufgezeichnet werden soll ( SwProtocol::nRecord ).
 * In dieser Methode werden noch die beiden weiteren Einschraenkungen ueberprueft,
 * ob die FrmId und der FrameType zu den aufzuzeichnenden gehoeren.
 * --------------------------------------------------*/


/* -----------------13.01.99 11:39-------------------
 * SwImplProtocol::SectFunc(...) wird von SwImplProtocol::_Record(..) gerufen,
 * hier werden die Ausgaben rund um SectionFrms abgehandelt.
 * --------------------------------------------------*/


/* -----------------11.01.99 11:31-------------------
 * SwImplProtocol::InsertFrm(..) nimmt eine neue FrmId zum Aufzeichnen auf,
 * wenn pFrmIds==NULL, werden alle aufgezeichnet, sobald durch InsertFrm(..)
 * pFrmIds angelegt wird, werden nur noch die enthaltenen FrmIds aufgezeichnet.
 * --------------------------------------------------*/


/* -----------------11.01.99 11:52-------------------
 * SwImplProtocol::DeleteFrm(..) entfernt eine FrmId aus dem pFrmIds-Array,
 * so dass diese Frame nicht mehr aufgezeichnet wird.
 * --------------------------------------------------*/

/*-----------------20.9.2001 10:29------------------
 * SwProtocol::SnapShot(..)
 * creates a snapshot of the given frame and its content.
 * --------------------------------------------------*/

/* -----------------11.01.99 11:53-------------------
 * SwEnterLeave::Ctor(..) wird vom eigentlichen (inline-)Kontruktor gerufen,
 * wenn die Funktion aufgezeichnet werden soll.
 * Die Aufgabe ist es abhaengig von der Funktion das richtige SwImplEnterLeave-Objekt
 * zu erzeugen, alles weitere geschieht dann in dessen Ctor/Dtor.
 * --------------------------------------------------*/

/* -----------------11.01.99 11:56-------------------
 * SwEnterLeave::Dtor() ruft lediglich den Destruktor des SwImplEnterLeave-Objekts,
 * ist nur deshalb nicht inline, damit die SwImplEnterLeave-Definition nicht
 * im dbg_lay.hxx zu stehen braucht.
 * --------------------------------------------------*/










}
