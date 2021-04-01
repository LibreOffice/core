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


#ifndef INCLUDED_BASIC_SBXDEF_HXX
#define INCLUDED_BASIC_SBXDEF_HXX

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

enum class SbxClassType {         // SBX-class-IDs (order is important!)
    DontCare = 1,  // don't care (search, not 0 due to StarBASIC)
    Array,         // Array of SbxVariables
    Value,         // simple value
    Variable,      // Variable (from here there is Broadcaster)
    Method,        // Method (Function or Sub)
    Property,      // Property
    Object         // Object
};

enum SbxDataType {
    SbxEMPTY    =  0,    // * Uninitialized
    SbxNULL     =  1,    // * Contains no valid data
    SbxINTEGER  =  2,    // * Integer (sal_Int16)
    SbxLONG     =  3,    // * Long integer (sal_Int32)
    SbxSINGLE   =  4,    // * Single-precision floating point number (float)
    SbxDOUBLE   =  5,    // * Double-precision floating point number (double)
    SbxCURRENCY =  6,    //   Currency (sal_Int64)
    SbxDATE     =  7,    // * Date (double)
    SbxSTRING   =  8,    // * String (StarView)
    SbxOBJECT   =  9,    // * SbxBase object pointer
    SbxERROR    = 10,    // * Error (sal_uInt16)
    SbxBOOL     = 11,    // * Boolean (0 or -1)

    SbxVARIANT    = 12,  // * Display for variant datatype
    SbxDATAOBJECT = 13,  // * Common data object w/o ref count

    SbxCHAR     = 16,    // * signed char
    SbxBYTE     = 17,    // * unsigned char
    SbxUSHORT   = 18,    // * unsigned short (sal_uInt16)
    SbxULONG    = 19,    // * unsigned long (sal_uInt32)

    SbxINT      = 22,    // * signed machine-dependent int
    SbxUINT     = 23,    // * unsigned machine-dependent int

    SbxVOID     = 24,    // * no value (= SbxEMPTY)
    SbxHRESULT  = 25,    //   HRESULT
    SbxPOINTER  = 26,    //   generic pointer
    SbxDIMARRAY = 27,    //   dimensioned array
    SbxCARRAY   = 28,    //   C style array
    SbxUSERDEF  = 29,    //   user defined
    SbxLPSTR    = 30,    // * null terminated string

    SbxLPWSTR   = 31,    //   wide null terminated string
    SbxCoreSTRING = 32,  //   from 1997-4-10 for GetCoreString(), only for converting<

    SbxWSTRING  = 33,    //   from 2000-10-4 Reimplemented for backwards compatibility (#78919)
    SbxWCHAR    = 34,    //   from 2000-10-4 Reimplemented for backwards compatibility (#78919)
    SbxSALINT64 = 35,    //   for currency internal, signed 64-bit int   and UNO hyper
    SbxSALUINT64= 36,    //   for currency internal, unsigned 64-bit int and UNO unsigned hyper
    SbxDECIMAL  = 37,    //   for UNO/automation Decimal

    SbxVECTOR = 0x1000,  // simple counted array
    SbxARRAY  = 0x2000,  // array
    SbxBYREF  = 0x4000,  // access by reference

    // tdf#79426, tdf#125180
    SbxMISSING = 0x8000, // Parameter is missing
};

const sal_uInt32 SBX_TYPE_WITH_EVENTS_FLAG = 0x10000;
const sal_uInt32 SBX_TYPE_DIM_AS_NEW_FLAG  = 0x20000;
const sal_uInt32 SBX_FIXED_LEN_STRING_FLAG = 0x10000;   // same value as above as no conflict possible
const sal_uInt32 SBX_TYPE_VAR_TO_DIM_FLAG  = 0x40000;

enum SbxOperator {
    // Arithmetical:
    SbxEXP,     // this ^ var
    SbxMUL,     // this * var
    SbxDIV,     // this / var
    SbxMOD,     // this MOD var     (max INT32!)
    SbxPLUS,    // this + var
    SbxMINUS,   // this - var
    SbxNEG,     // -this (var is ignored)
    SbxIDIV,            // this / var (both operands max. sal_Int32!)
                        // Boolean operators (max sal_Int32!):
    // Boolean operators (TODO deprecate this limit: max INT32!)
    SbxAND,     // this & var
    SbxOR,      // this | var
    SbxXOR,     // this ^ var
    SbxEQV,     // ~this ^ var
    SbxIMP,     // ~this | var
    SbxNOT,     // ~this (var is ignored)

    // String concatenation:
    SbxCAT,     // this & var (VBA: this + var)

    // Comparisons:
    SbxEQ,      // this = var
    SbxNE,      // this <> var
    SbxLT,      // this < var
    SbxGT,      // this > var
    SbxLE,      // this <= var
    SbxGE       // this >= var
};

enum class SbxNameType {          // Type of the questioned name of a variable
    NONE,           // plain name
    ShortTypes,     // Name%(A%,B$)
};


// Flag-Bits:
enum class SbxFlagBits {
    NONE         = 0x0000,
    Read         = 0x0001,  // Read permission
    Write        = 0x0002,  // Write permission
    ReadWrite    = 0x0003,  // Read/Write permission
    DontStore    = 0x0004,  // Don't store object
    Modified     = 0x0008,  // Object was changed
    Fixed        = 0x0010,  // Fixed data type (SbxVariable)
    Const        = 0x0020,  // Definition of const value
    Optional     = 0x0040,  // Parameter is optional
    Hidden       = 0x0080,  // Element is invisible
    Invisible    = 0x0100,  // Element is not found by Find()
    ExtSearch    = 0x0200,  // Object is searched completely
    ExtFound     = 0x0400,  // Variable was found through extended search
    GlobalSearch = 0x0800,  // Global search via Parents
    Reserved     = 0x1000,  // reserved
    Private      = 0x1000,  // #110004, #112015, cannot conflict with SbxFlagBits::Reserved
    NoBroadcast  = 0x2000,  // No broadcast on Get/Put
    Reference    = 0x4000,  // Parameter is Reference (DLL-call)
    NoModify     = 0x8000,  // SetModified is suppressed
    WithEvents   = 0x0080,  // Same value as unused SbxFlagBits::Hidden
    DimAsNew     = 0x0800,  // Same value as SbxFlagBits::GlobalSearch, cannot conflict as one
                                         // is used for objects, the other for variables only
    VarToDim     = 0x2000,  // Same value as SbxFlagBits::NoBroadcast, cannot conflict as
                                         // used for variables without broadcaster only
};
namespace o3tl
{
    template<> struct typed_flags<SbxFlagBits> : is_typed_flags<SbxFlagBits, 0xffff> {};
}

// List of all creators for Load/Store

constexpr auto SBXCR_SBX = 0x20584253;        // SBX(blank)

// List of predefined SBX-IDs. New SBX-IDs must be precisely defined so that
// they are unique within the Stream and appropriate Factory.

constexpr auto SBXID_VALUE = 0x4E4E;  // NN: SbxValue
constexpr auto SBXID_VARIABLE = 0x4156;  // VA: SbxVariable
constexpr auto SBXID_ARRAY = 0x5241;  // AR: SbxArray
constexpr auto SBXID_DIMARRAY = 0x4944;  // DI: SbxDimArray
constexpr auto SBXID_OBJECT = 0x424F;  // OB: SbxObject
constexpr auto SBXID_COLLECTION = 0x4F43;  // CO: SbxCollection
constexpr auto SBXID_FIXCOLLECTION = 0x4346;  // FC: SbxStdCollection
constexpr auto SBXID_METHOD = 0x454D;  // ME: SbxMethod
constexpr auto SBXID_PROPERTY = 0x5250;  // PR: SbxProperty

// StarBASIC restricts the base data type to different intervals.
// These intervals are fixed to create 'portability and independent
// of the implementation. Only type double is greedy and takes
// what it gets.

constexpr auto SbxMAXCHAR = u'\xFFFF';
constexpr auto SbxMINCHAR = 0;
constexpr auto SbxMAXBYTE = 255;
constexpr auto SbxMAXINT = 32767;
constexpr auto SbxMININT = -32768;
constexpr sal_uInt16 SbxMAXUINT = 65535;
constexpr auto SbxMAXLNG = 2147483647;
constexpr sal_Int32 SbxMINLNG = -2147483647-1;
constexpr sal_uInt32 SbxMAXULNG = 0xffffffff;

        // Currency stored as SbxSALINT64 == sal_Int64
        // value range limits are ~(2^63 - 1)/10000
        // fixed precision has 4 digits right of decimal pt
constexpr auto CURRENCY_FACTOR = 10000;
constexpr auto CURRENCY_FACTOR_SQUARE = 100000000;

// TODO effective MAX/MINCURR limits:
// true value ( 922337203685477.5807) is too precise for correct comparison to 64bit double
constexpr auto SbxMAXCURR = 922337203685477.5807;
constexpr auto SbxMINCURR = -922337203685477.5808;

constexpr auto SbxMAXSNG = 3.402823e+38;
constexpr auto SbxMINSNG = -3.402823e+38;
constexpr auto SbxMAXSNG2 = 1.175494351e-38;
constexpr auto SbxMINSNG2 = -1.175494351e-38;

// Max valid offset index of a Sbx-Array (due to 64K limit)
constexpr auto SBX_MAXINDEX = 0x3FF0;
constexpr auto SBX_MAXINDEX32 = SbxMAXLNG;

// The numeric values of sal_True and FALSE
enum SbxBOOL { SbxFALSE = 0, SbxTRUE = -1 };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
