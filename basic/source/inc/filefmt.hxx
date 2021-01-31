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

// Version  2: data type of the return value for publics
// Version  3: new opcodes
// Version  4: new opcodes
// Version  5: bug (entry of STATIC-variables in the init code)
// Version  6: new opcodes and bug (construct globals, without ending the BASIC)
// Version  7: correction concerning the WITH-Parsing
// Version  8: correction concerning the IF-Parsing
// Version  9: end init code with LEAVE, too, if no SUB/FUNCTION follows
// Version  A: #36374 at DIM AS NEW... construct variable too
// Version  B: #40689 reorganized static
// Version  C: #41606 bug at static
// Version  D: #42678 bug at RTL-function spc
// Version  E: #56204 DCREATE, to also construct arrays at DIM AS NEW
// Version  F: #57844 introduction of SvNumberformat::StringToDouble
// Version 10: #29955 generate for-loop-level in Statement-PCodes
// Version 11: #29955 force anew compilation because of build-inconsistences
// Version 12: aoo#64377 increase code size that basic can handle
//             tdf#75973 support user defined types B_USERTYPES in password protected macros
// Version 13: tdf#94617 store methods nStart information greater than sal_Int16 limit
//             tdf#57113 store UTF-16 strings after legacy 1-byte-encoded strings in pool (no
//                       version number bump for backward compatibility; relies on magic number)
//

#define B_LEGACYVERSION 0x00000011
#define B_EXT_IMG_VERSION 0x00000012
#define B_CURVERSION 0x00000013

// The file contains either a module- or a library-record.
// Those records contain further records. Every record's got
// the following header:

//  sal_uInt16 identifier
//  sal_uInt32 the record's length without the header
//  sal_uInt16 number of sub-elements

// all the file-offsets in records are relative to the module's start!

enum class FileOffset {
    Library     =  0x4C42,      // BL Library Record
    Module      =  0x4D42,      // BM Module Record
    Name        =  0x4E4D,      // MN module name
    Comment     =  0x434D,      // MC comment
    Source      =  0x4353,      // SC source code
    PCode       =  0x4350,      // PC p-code
    OldPublics  =  0x7550,      // Pu publics
    Publics     =  0x5550,      // PU publics
    PoolDir     =  0x4450,      // PD symbol pool directory
    SymPool     =  0x5953,      // SY symbol pool
    StringPool  =  0x5453,      // ST symbol pool
    LineRanges  =  0x524C,      // LR line ranges for publics
    ModEnd      =  0x454D,      // ME module end
    SbxObjects  =  0x5853,      // SX SBX objects
    ExtSource   =  0x5345,      // ES extended source
    UserTypes   =  0x4369,      // UT user defined types

    LastValue   = UserTypes
};


// A library record contains only module records
//  sal_uInt16 identifier BL
//  sal_uInt32 the record's length
//  sal_uInt16 number of modules

// A module-record contains all the other record types
//  sal_uInt16 identifier BM
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  sal_uInt32 version number
//  sal_uInt32 character set
//  sal_uInt32 starting address initialisation code
//  sal_uInt32 starting address sub main
//  sal_uInt32 reserved
//  sal_uInt32 reserved

// module name, comment and source code:
//  sal_uInt16 identifier MN, MC or SC
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  string instance

// P-Code:
//  sal_uInt16 identifier PC
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  the P-Code as bytesack

// All symbols and strings are kept in a string-pool.
// References to these strings are in this pool in the form of an index.

// List of all publics:
//  sal_uInt16 identifier PU or Pu
//  sal_uInt32 the record's length
//  sal_uInt16 number of publics
// Data for every public-entry:
//  sal_uInt16 string index
//  sal_uInt32 starting address in the p-code-image (sal_uInt16 for old publics)
//  sal_uInt16 data type of the return value (from version 2)

// Register of the symbol tables:
//  sal_uInt16 identifier SP
//  sal_uInt32 the record's length
//  sal_uInt16 number of symbol tables
// Data for every symbol table:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 number of symbols
//  sal_uInt16 scope identifier

// symbol table:
//  sal_uInt16 identifier SY
//  sal_uInt32 the record's length
//  sal_uInt16 number of symbols
// Data:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 number of symbols
// Data for every symbol:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 data type
//  sal_uInt16 length for STRING*n-symbols (0x8000: STATIC variable)

// Stringpool:
//  sal_uInt16 identifier ST
//  sal_uInt32 the record's length
//  sal_uInt16 number of strings
// Data for every string:
//  sal_uInt32 Offset in the block of all strings
// the block of all strings (ASCIIZ) follows then

// line ranges:
//  sal_uInt16 identifier LR
//  sal_uInt32 the record's length
//  sal_uInt16 number of strings
// Data for every public:
//  sal_uInt16 1st line (Sub XXX)
//  sal_uInt16 2nd line (End Sub)

// SBX-objects:
// sal_uInt16 number of objects
// ...   object data

// user defined types B_USERTYPES :
//  sal_uInt16  identifier UT
//  sal_uInt32  the record's length
//  sal_uInt16  number of types
//  Data for every user defined type:
//    string instance type name
//    sal_Int16  number of type members
//    Data for every type member:
//      string     name
//      sal_Int16 type
//      sal_uInt32 flags
//      sal_Int16 hasObjects (0/1)
//      If hasObjects
//        If member type is nested type
//          string   nested type name
//        Else (array declaration)
//          sal_Int16  isFixedSize (0/1)
//          sal_Int32  number of dimensions
//          Data for every dimension:
//            sal_Int32  lower bound
//            sal_Int32  upper bound

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
