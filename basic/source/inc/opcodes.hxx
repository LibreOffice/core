/*************************************************************************
 *
 *  $RCSfile: opcodes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2000-10-18 08:58:04 $
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
    _GET,               // TOS anfassen
    _SET,               // Speichern Objekt TOS ==> TOS-1
    _PUT,               // TOS ==> TOS-1
    _PUTC,              // TOS ==> TOS-1, dann ReadOnly
    _DIM,               // DIM
    _REDIM,             // REDIM
    _REDIMP,            // REDIM PRESERVE
    _ERASE,             // TOS loeschen
    // Verzweigen
    _STOP,              // Programmende
    _INITFOR,           // FOR-Variable initialisieren
    _NEXT,              // FOR-Variable inkrementieren
    _CASE,              // Anfang CASE
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
    _CHAN0,             // I/O-Kanal 0
    // Sonstiges
    _EMPTY,             // Leeren Ausdruck auf Stack
    _ERROR,             // TOS = Fehlercode
    _LSET,              // Speichern Objekt TOS ==> TOS-1
    _RSET,              // Speichern Objekt TOS ==> TOS-1
    _REDIMP_ERASE,      // Copies array to be later used by REDIM PRESERVE before erasing it
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
    _CLASS,             // Klassennamen testen (+StringId)
    _LIB,               // Libnamen fuer Declare-Procs setzen (+StringId)
    _BASED,             // TOS wird um BASE erhoeht, BASE davor gepusht (+base)
    // Typanpassung im Argv
    _ARGTYP,            // Letzten Parameter in Argv konvertieren (+Typ)

    SbOP1_END,

    // Alle Opcodes mit zwei Operanden

    _RTL = 0x80,        // Laden aus RTL (+StringID+Typ)

    SbOP2_START = _RTL,

    _FIND,              // Laden (+StringID+Typ)
    _ELEM,              // Laden Element (+StringID+Typ)
    _PARAM,             // Parameter (+Offset+Typ)
    // Verzweigen
    _CALL,              // DECLARE-Methode rufen (+StringID+Typ)
    _CALLC,             // Cdecl-DECLARE-Methode rufen (+StringID+Typ)
    _CASEIS,            // Case-Test (+Test-Opcode+True-Target)
    // Verwaltung
    _STMNT,             // Beginn eines Statements (+Line+Col)
    // E/A
    _OPEN,              // (+SvStreamFlags+Flags)
    // Objekte
    _LOCAL,             // Lokale Variable definieren (+StringID+Typ)
    _PUBLIC,            // Modulglobale Variable (+StringID+Typ)
    _GLOBAL,            // Globale Variable definieren (+StringID+Typ)
    _CREATE,            // Objekt kreieren (+StringId+StringID)
    _STATIC,            // Statische Variabl (+StringID+Typ) JSM
    _TCREATE,           // User Defined Objekt kreieren
    _DCREATE,           // Objekt-Array kreieren (+StringId+StringID)
    SbOP2_END

};



#endif
