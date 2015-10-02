/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_BASIC_SOURCE_INC_OPCODES_HXX
#define INCLUDED_BASIC_SOURCE_INC_OPCODES_HXX

#include "sbintern.hxx"

// An opcode can have a length of 1, 3 or 5 bytes,
// depending on its numeric value (see below).

enum SbiOpcode {
    // all opcodes without operands
    _NOP = 0,

    SbOP0_START = _NOP,

    // operators
    // the following operators are ordered
    // the same way as the enum SbxVarOp
    _EXP, _MUL, _DIV, _MOD, _PLUS, _MINUS, _NEG,
    _EQ,  _NE,  _LT,  _GT, _LE, _GE,
    _IDIV, _AND, _OR, _XOR, _EQV, _IMP, _NOT,
    _CAT,
    // end of enum SbxVarOp
    _LIKE, _IS,
    // load/save
    _ARGC,              // establish new Argv
    _ARGV,              // TOS ==> current Argv
    _INPUT,             // Input ==> TOS
    _LINPUT,            // Line Input ==> TOS
    _GET,               // touch TOS
    _SET,               // save object TOS ==> TOS-1
    _PUT,               // TOS ==> TOS-1
    _PUTC,              // TOS ==> TOS-1, then ReadOnly
    _DIM,               // DIM
    _REDIM,             // REDIM
    _REDIMP,            // REDIM PRESERVE
    _ERASE,             // delete TOS
    // branch
    _STOP,              // end of program
    _INITFOR,           // initialize FOR-variable
    _NEXT,              // increment FOR-variable
    _CASE,              // beginning CASE
    _ENDCASE,           // end CASE
    _STDERROR,          // standard error handling
    _NOERROR,           // no error handling
    _LEAVE,             // leave UP
    // E/A
    _CHANNEL,           // TOS = channel number
    _BPRINT,            // print TOS
    _PRINTF,            // print TOS in field
    _BWRITE,            // write TOS
    _RENAME,            // Rename Tos+1 to Tos
    _PROMPT,            // TOS = Prompt for Input
    _RESTART,           // define restart point
    _CHAN0,             // I/O-channel 0
    // miscellaneous
    _EMPTY,             // empty expression on stack
    _ERROR,             // TOS = error code
    _LSET,              // saving object TOS ==> TOS-1
    _RSET,              // saving object TOS ==> TOS-1
    _REDIMP_ERASE,      // Copies array to be later used by REDIM PRESERVE before erasing it
    _INITFOREACH,
    _VBASET,            // VBA-like Set
    _ERASE_CLEAR,       // Erase array and clear variable
    _ARRAYACCESS,       // Assign parameters to TOS and get value, used for array of arrays
    _BYVAL,             // byref -> byval for lvalue parameter passed in call

    SbOP0_END = _BYVAL,

    // all opcodes with one operand

    _NUMBER = 0x40,     // loading a numeric constant (+ID)

    SbOP1_START = _NUMBER,

    _SCONST,            // loading a stringconstant (+ID)
    _CONST,             // Immediate Load (+ value)
    _ARGN,              // saving a named Arg in Argv (+StringID)
    _PAD,               // bring string to a firm length (+length)
    // Verzweigungen
    _JUMP,              // jump (+target)
    _JUMPT,             // evaluate TOS, conditional jump (+target)
    _JUMPF,             // evaluate TOS, conditional jump  (+target)
    _ONJUMP,            // evaluate TOS, jump into JUMP-table (+MaxVal)
    _GOSUB,             // UP-call (+Target)
    _RETURN,            // UP-return (+0 or Target)
    _TESTFOR,           // test FOR-variable, increment (+Endlabel)
    _CASETO,            // Tos+1 <= Case <= Tos, 2xremove (+Target)
    _ERRHDL,            // error handler (+Offset)
    _RESUME,            // Resume after errors (+0 or 1 or Label)
    // E/A
    _CLOSE,             // (+channel/0)
    _PRCHAR,            // (+char)
    // Verwaltung
    _SETCLASS,          // test set + class names (+StringId)
    _TESTCLASS,         // Check TOS class (+StringId)
    _LIB,               // set lib name for declare-procs (+StringId)
    _BASED,             // TOS is incremented by BASE, BASE is pushed before (+base)
    // type adjustment in the Argv
    _ARGTYP,            // convert last parameter in Argv (+type)
    _VBASETCLASS,       // VBA-like Set

    SbOP1_END = _VBASETCLASS,

    // all opcodes with two operands

    _RTL = 0x80,        // load from the RTL (+StringID+Typ)

    SbOP2_START = _RTL,

    _FIND,              // load (+StringID+Typ)
    _ELEM,              // load element (+StringID+Typ)
    _PARAM,             // parameters (+Offset+Typ)
    // branch
    _CALL,              // call DECLARE-method (+StringID+Typ)
    _CALLC,             // call Cdecl-DECLARE-Method (+StringID+Typ)
    _CASEIS,            // case-test (+Test-Opcode+True-Target)
    // management
    _STMNT,             // begin of a statement (+Line+Col)
    // E/A
    _OPEN,              // (+StreamMode+Flags)
    // objects
    _LOCAL,             // define locals variables (+StringID+Typ)
    _PUBLIC,            // module global variables (+StringID+Typ)
    _GLOBAL,            // define global variables, public command (+StringID+Typ)
    _CREATE,            // create object (+StringId+StringID)
    _STATIC,            // static variable (+StringID+Typ) JSM
    _TCREATE,           // create user-defined object
    _DCREATE,           // create object-array (+StringId+StringID)
    _GLOBAL_P,          // define global variable that's not overwritten on restarting
                        // the Basic, P=PERSIST (+StringID+Typ)
    _FIND_G,            // finds global variable with special treatment due to _GLOBAL_P
    _DCREATE_REDIMP,    // redimension object-array (+StringId+StringID)
    _FIND_CM,           // Search inside a class module (CM) to enable global search in time
    _PUBLIC_P,          //  Module global Variable (persisted between calls)(+StringID+Typ)
    _FIND_STATIC,           //  local static var lookup (+StringID+Typ)

    SbOP2_END = _FIND_STATIC
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
