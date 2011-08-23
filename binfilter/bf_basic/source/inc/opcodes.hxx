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

#ifndef _OPCODES_HXX
#define _OPCODES_HXX

#include "sbintern.hxx"

#ifdef MTW
#undef _NUMBER
#endif

// Ein Opcode ist entweder 1, 3 oder 5 Bytes lang, je nach numerischen
// Wert des Opcodes (s.u.).

enum SbiOpcode {
    // Alle Opcodes ohne Operanden
    _NOP = 0,

    SbOP0_START = _NOP,

    // Operatoren
    // die folgenden Operatoren sind genauso angeordnet
    // wie der enum SbxVarOp
    _EXP, _MUL, _DIV, _MOD, _PLUS, _MINUS, _NEG,
    _EQ,  _NE,  _LT,  _GT, _LE, _GE,
    _IDIV, _AND, _OR, _XOR, _EQV, _IMP, _NOT,
    _CAT,
    // Ende enum SbxVarOp
    _LIKE, _IS,
    // Laden/speichern
    _ARGC,              // neuen Argv einrichten
    _ARGV,              // TOS ==> aktueller Argv
    _INPUT,             // Input ==> TOS
    _LINPUT,            // Line Input ==> TOS
    _GET,				// TOS anfassen
    _SET,               // Speichern Objekt TOS ==> TOS-1
    _PUT,				// TOS ==> TOS-1
    _PUTC,				// TOS ==> TOS-1, dann ReadOnly
    _DIM,               // DIM
    _REDIM,             // REDIM
    _REDIMP,            // REDIM PRESERVE
    _ERASE,             // TOS loeschen
    // Verzweigen
    _STOP,              // Programmende
    _INITFOR,           // FOR-Variable initialisieren
    _NEXT,              // FOR-Variable inkrementieren
    _CASE,				// Anfang CASE
    _ENDCASE,           // Ende CASE
    _STDERROR,          // Standard-Fehlerbehandlung
    _NOERROR,           // keine Fehlerbehandlung
    _LEAVE,             // UP verlassen
    // E/A
    _CHANNEL,           // TOS = Kanalnummer
    _BPRINT,            // print TOS
    _PRINTF,            // print TOS in field
    _BWRITE,            // write TOS
    _RENAME,            // Rename Tos+1 to Tos
    _PROMPT,            // TOS = Prompt for Input
    _RESTART,           // Restartpunkt definieren
    _CHAN0,				// I/O-Kanal 0
    // Sonstiges
    _EMPTY,             // Leeren Ausdruck auf Stack
    _ERROR,				// TOS = Fehlercode
    _LSET,				// Speichern Objekt TOS ==> TOS-1
    _RSET,              // Speichern Objekt TOS ==> TOS-1
    _REDIMP_ERASE,		// Copies array to be later used by REDIM PRESERVE before erasing it
    _INITFOREACH,
    _VBASET,            // VBA-like Set
    SbOP0_END,

    // Alle Opcodes mit einem Operanden

    _NUMBER = 0x40,     // Laden einer numerischen Konstanten (+ID)

    SbOP1_START = _NUMBER,

    _SCONST,            // Laden einer Stringkonstanten (+ID)
    _CONST,             // Immediate Load (+Wert)
    _ARGN,              // Speichern eines named Args in Argv (+StringID)
    _PAD,               // String auf feste Laenge bringen (+Laenge)
    // Verzweigungen
    _JUMP,              // Sprung (+Target)
    _JUMPT,             // TOS auswerten, bedingter Sprung (+Target)
    _JUMPF,             // TOS auswerten, bedingter Sprung (+Target)
    _ONJUMP,            // TOS auswerten, Sprung in JUMP-Tabelle (+MaxVal)
    _GOSUB,             // UP-Aufruf (+Target)
    _RETURN,            // UP-Return (+0 oder Target)
    _TESTFOR,           // FOR-Variable testen, inkrementieren (+Endlabel)
    _CASETO,            // Tos+1 <= Case <= Tos, 2xremove (+Target)
    _ERRHDL,            // Fehler-Handler (+Offset)
    _RESUME,            // Resume nach Fehlern (+0 or 1 or Label)
    // E/A
    _CLOSE,             // (+Kanal/0)
    _PRCHAR,            // (+char)
    // Verwaltung
    _SETCLASS,          // Set + Klassennamen testen (+StringId)
    _TESTCLASS,         // Check TOS class (+StringId)
    _LIB,				// Libnamen fuer Declare-Procs setzen (+StringId)
    _BASED,          	// TOS wird um BASE erhoeht, BASE davor gepusht (+base)
    // Typanpassung im Argv
    _ARGTYP,          	// Letzten Parameter in Argv konvertieren (+Typ)
    SbOP1_END,

    // Alle Opcodes mit zwei Operanden

    _RTL = 0x80,   		// Laden aus RTL (+StringID+Typ)

    SbOP2_START = _RTL,

    _FIND,       		// Laden (+StringID+Typ)
    _ELEM,       		// Laden Element (+StringID+Typ)
    _PARAM,				// Parameter (+Offset+Typ)
    // Verzweigen
    _CALL,				// DECLARE-Methode rufen (+StringID+Typ)
    _CALLC,				// Cdecl-DECLARE-Methode rufen (+StringID+Typ)
    _CASEIS,            // Case-Test (+Test-Opcode+True-Target)
    // Verwaltung
    _STMNT,             // Beginn eines Statements (+Line+Col)
    // E/A
    _OPEN,              // (+SvStreamFlags+Flags)
    // Objekte
    _LOCAL,				// Lokale Variable definieren (+StringID+Typ)
    _PUBLIC, 			// Modulglobale Variable (+StringID+Typ)
    _GLOBAL,			// Globale Variable definieren, public-Anweisung (+StringID+Typ)
    _CREATE,			// Objekt kreieren (+StringId+StringID)
    _STATIC,			// Statische Variabl (+StringID+Typ) JSM
    _TCREATE,			// User Defined Objekt kreieren
    _DCREATE,			// Objekt-Array kreieren (+StringId+StringID)
    _GLOBAL_P,      	// Globale Variable definieren, die beim Neustart von Basic
                        // nicht ueberschrieben wird, P=PERSIST (+StringID+Typ)
    _FIND_G,        	// Sucht globale Variable mit Spezialbehandlung wegen _GLOBAL_P
    _DCREATE_REDIMP,	// Objekt-Array redimensionieren (+StringId+StringID)
    _FIND_CM,        	// Search inside a class module (CM) to enable global search in time
    SbOP2_END

};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
