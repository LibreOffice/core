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

#ifndef _SB_FILEFMT_HXX
#define _SB_FILEFMT_HXX

#include <tools/solar.h>

class SvStream;

namespace binfilter {

// Version  2: Datentyp des Returnwerts fuer Publics
// Version  3: neue Opcodes
// Version  4: neue Opcodes
// Version  5: Bug (Ansprung von STATIC-Variablen im Init-Code)
// Version  6: Neue Opcodes und Bug (Globals anlegen, ohne BASIC zu beenden)
// Version  7: Korrektur im WITH-Parsing
// Version  8: Korrektur im IF-Parsing
// Version  9: Init-Code auch mit LEAVE beenden, wenn keine SUB/FUNCTION folgt
// Version  A: #36374 Bei DIM AS NEW... auch Variablen anlegen
// Version  B: #40689 Static umgestellt
// Version  C: #41606 Bug bei Static
// Version  D: #42678 Bug bei RTL-Function spc
// Version  E: #56204 DCREATE, um auch bei DIM AS NEW Arrays anzulegen
// Version  F: #57844 Einfuehrung von SvNumberformat::StringToDouble
// Version 10: #29955 For-Schleifen-Level in Statement-PCodes generieren
// Version 11: #29955 Wegen Build-Inkonsistenzen Neu-Compilieren erzwingen

#define B_LEGACYVERSION 0x00000011L
#define B_CURVERSION 0x00000012L
#define B_EXT_IMG_VERSION 0x00000012L

// Eine Datei enthaelt entweder einen Modul- oder einen Library-Record.
// Diese Records enthalten wiederum weitere Records. Jeder Record hat
// den folgenden Header:

//  UINT16 Kennung
//  UINT32 Laenge des Records ohne Header
//  UINT16 Anzahl Unterelemente

// Alle Datei-Offsets in Records sind relativ zum Start des Moduls!

#define B_LIBRARY		0x4C42		// BL Library Record
#define	B_MODULE		0x4D42		// BM Module Record
#define	B_NAME			0x4E4D		// MN module name
#define	B_COMMENT		0x434D		// MC comment
#define	B_SOURCE		0x4353		// SC source code
#define	B_PCODE			0x4350		// PC p-code
#define	B_OLDPUBLICS 	0x7550		// Pu publics
#define	B_PUBLICS 		0x5550		// PU publics
#define	B_POOLDIR 		0x4450		// PD symbol pool directory
#define	B_SYMPOOL		0x5953		// SY symbol pool
#define	B_STRINGPOOL	0x5453		// ST symbol pool
#define	B_LINERANGES	0x524C		// LR line ranges for publics
#define	B_MODEND		0x454D		// ME module end
#define	B_SBXOBJECTS	0x5853		// SX SBX objects

#define	EXTENDED_BINARY_MODULES
#ifdef  EXTENDED_BINARY_MODULES
#define	B_EXTSOURCE		0x5345		// ES extended source
#endif

// Ein Library Record enthaelt nur Module Records
//  UINT16 Kennung BL
//  UINT32 Laenge des Records
//  UINT16 Anzahl Module

// Ein Modul-Record enthaelt alle anderen Recordtypen
//  UINT16 Kennung BM
//  UINT32 Laenge des Records
//  UINT16 1
// Daten:
//  UINT32 Versionsnummer
//  UINT32 Zeichensatz
//  UINT32 Startadresse Initialisierungscode
//  UINT32 Startadresse Sub Main
//  UINT32 Reserviert
//  UINT32 Reserviert

// Modulname, Kommentar und Quellcode:
//  UINT16 Kennung MN, MC oder SC
//  UINT32 Laenge des Records
//  UINT16 1
// Daten:
//  String-Instanz

// P-Code:
//  UINT16 Kennung PC
//  UINT32 Laenge des Records
//  UINT16 1
// Daten:
//  Der P-Code als Bytesack

// Alle Symbole und Strings werden in einem String-Pool gehalten.
// Verweise auf diese Strings sind in Form eines Indexes in diesen Pool.

// Liste aller Publics:
//  UINT16 Kennung PU oder Pu
//  UINT32 Laenge des Records
//  UINT16 Anzahl der Publics
// Daten fuer jeden Public-Eintrag:
//  UINT16 String-Index
//  UINT32 Startadresse im P-Code-Image (UINT16 fuer alte Publics)
//  UINT16 Datentyp des Returnwertes (ab Version 2)

// Verzeichnis der Symbol-Tabellen:
//  UINT16 Kennung SP
//  UINT32 Laenge des Records
//  UINT16 Anzahl der Symboltabellen
// Daten fuer jede Symboltabelle:
//  UINT16 Stringindex des Namens
//  UINT16 Anzahl Symbole
//  UINT16 Scope-Kennung

// Symboltabelle:
//  UINT16 Kennung SY
//  UINT32 Laenge des Records
//  UINT16 Anzahl der Symbole
// Daten:
//  UINT16 Stringindex des Namens
//  UINT16 Anzahl Symbole
// Daten fuer jedes Symbol:
//  UINT16 Stringindex des Namens
//  UINT16 Datentyp
//  UINT16 Laenge bei STRING*n-Symbolen (0x8000: STATIC-Variable)

// Stringpool:
//  UINT16 Kennung ST
//  UINT32 Laenge des Records
//  UINT16 Anzahl der Strings
// Daten fuer jeden String:
//  UINT32 Offset in den Block aller Strings
// Danach folgt der Block aller Strings, die dort als ASCIIZ-Strings liegen.

// Line Ranges:
//  UINT16 Kennung LR
//  UINT32 Laenge des Records
//  UINT16 Anzahl der Strings
// Daten fuer jedes Public:
//  UINT16 1. Zeile (Sub XXX)
//  UINT16 2. Zeile (End Sub)

// SBX-Objekte:
// UINT16 Anzahl Objekte
// ....   Objektdaten

////////////////////////////////////////////////////////////////////////////

// Service-Routinen (in IMAGE.CXX)

BOOL  SbGood( SvStream& r );
ULONG SbOpenRecord( SvStream&, UINT16 nSignature, UINT16 nElem );
void  SbCloseRecord( SvStream&, ULONG );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
