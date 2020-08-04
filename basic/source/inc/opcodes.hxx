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

#pragma once

// An opcode can have a length of 1, 3 or 5 bytes,
// depending on its numeric value (see below).

enum class SbiOpcode {
    // all opcodes without operands
    NOP_ = 0,

    SbOP0_START = NOP_,

    // operators
    // the following operators are ordered
    // the same way as the enum SbxVarOp
    EXP_, MUL_, DIV_, MOD_, PLUS_, MINUS_, NEG_,
    EQ_,  NE_,  LT_,  GT_, LE_, GE_,
    IDIV_, AND_, OR_, XOR_, EQV_, IMP_, NOT_,
    CAT_,
    // end of enum SbxVarOp
    LIKE_, IS_,
    // load/save
    ARGC_,              // establish new Argv
    ARGV_,              // TOS ==> current Argv
    INPUT_,             // Input ==> TOS
    LINPUT_,            // Line Input ==> TOS
    GET_,               // touch TOS
    SET_,               // save object TOS ==> TOS-1
    PUT_,               // TOS ==> TOS-1
    PUTC_,              // TOS ==> TOS-1, then ReadOnly
    DIM_,               // DIM
    REDIM_,             // REDIM
    REDIMP_,            // REDIM PRESERVE
    ERASE_,             // delete TOS
    // branch
    STOP_,              // end of program
    INITFOR_,           // initialize FOR-variable
    NEXT_,              // increment FOR-variable
    CASE_,              // beginning CASE
    ENDCASE_,           // end CASE
    STDERROR_,          // standard error handling
    NOERROR_,           // no error handling
    LEAVE_,             // leave UP
    // E/A
    CHANNEL_,           // TOS = channel number
    BPRINT_,            // print TOS
    PRINTF_,            // print TOS in field
    BWRITE_,            // write TOS
    RENAME_,            // Rename Tos+1 to Tos
    PROMPT_,            // TOS = Prompt for Input
    RESTART_,           // define restart point
    CHAN0_,             // I/O-channel 0
    // miscellaneous
    EMPTY_,             // empty expression on stack
    ERROR_,             // TOS = error code
    LSET_,              // saving object TOS ==> TOS-1
    RSET_,              // saving object TOS ==> TOS-1
    REDIMP_ERASE_,      // Copies array to be later used by REDIM PRESERVE before erasing it
    INITFOREACH_,
    VBASET_,            // VBA-like Set
    ERASE_CLEAR_,       // Erase array and clear variable
    ARRAYACCESS_,       // Assign parameters to TOS and get value, used for array of arrays
    BYVAL_,             // byref -> byval for lvalue parameter passed in call

    SbOP0_END = BYVAL_,

    // all opcodes with one operand

    NUMBER_ = 0x40,     // loading a numeric constant (+ID)

    SbOP1_START = NUMBER_,

    SCONST_,            // loading a string constant (+ID)
    CONST_,             // Immediate Load (+ value)
    ARGN_,              // saving a named Arg in Argv (+StringID)
    PAD_,               // bring string to a firm length (+length)
    // branch
    JUMP_,              // jump (+target)
    JUMPT_,             // evaluate TOS, conditional jump (+target)
    JUMPF_,             // evaluate TOS, conditional jump  (+target)
    ONJUMP_,            // evaluate TOS, jump into JUMP-table (+MaxVal)
    GOSUB_,             // UP-call (+Target)
    RETURN_,            // UP-return (+0 or Target)
    TESTFOR_,           // test FOR-variable, increment (+Endlabel)
    CASETO_,            // Tos+1 <= Case <= Tos, 2xremove (+Target)
    ERRHDL_,            // error handler (+Offset)
    RESUME_,            // Resume after errors (+0 or 1 or Label)
    // E/A
    CLOSE_,             // (+channel/0)
    PRCHAR_,            // (+char)
    // management
    SETCLASS_,          // test set + class names (+StringId)
    TESTCLASS_,         // Check TOS class (+StringId)
    LIB_,               // set lib name for declare-procs (+StringId)
    BASED_,             // TOS is incremented by BASE, BASE is pushed before (+base)
    // type adjustment in the Argv
    ARGTYP_,            // convert last parameter in Argv (+type)
    VBASETCLASS_,       // VBA-like Set

    SbOP1_END = VBASETCLASS_,

    // all opcodes with two operands

    RTL_ = 0x80,        // load from the RTL (+StringID+Typ)

    SbOP2_START = RTL_,

    FIND_,              // load (+StringID+Typ)
    ELEM_,              // load element (+StringID+Typ)
    PARAM_,             // parameters (+Offset+Typ)
    // branch
    CALL_,              // call DECLARE-method (+StringID+Typ)
    CALLC_,             // call Cdecl-DECLARE-Method (+StringID+Typ)
    CASEIS_,            // case-test (+Test-Opcode+True-Target)
    // management
    STMNT_,             // begin of a statement (+Line+Col)
    // E/A
    OPEN_,              // (+StreamMode+Flags)
    // objects
    LOCAL_,             // define locals variables (+StringID+Typ)
    PUBLIC_,            // module global variables (+StringID+Typ)
    GLOBAL_,            // define global variables, public command (+StringID+Typ)
    CREATE_,            // create object (+StringId+StringID)
    STATIC_,            // static variable (+StringID+Typ) JSM
    TCREATE_,           // create user-defined object
    DCREATE_,           // create object-array (+StringId+StringID)
    GLOBAL_P_,          // define global variable that's not overwritten on restarting
                        // the Basic, P=PERSIST (+StringID+Typ)
    FIND_G_,            // finds global variable with special treatment due to GLOBAL_P_
    DCREATE_REDIMP_,    // redimension object-array (+StringId+StringID)
    FIND_CM_,           // Search inside a class module (CM) to enable global search in time
    PUBLIC_P_,          //  Module global Variable (persisted between calls)(+StringID+Typ)
    FIND_STATIC_,           //  local static var lookup (+StringID+Typ)

    SbOP2_END = FIND_STATIC_
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
