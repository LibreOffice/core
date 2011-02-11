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


#ifndef _SBXDEF_HXX
#define _SBXDEF_HXX


#ifndef __RSC
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#include "tools/errcode.hxx"

#ifndef _SBX_CLASS_TYPE
#define _SBX_CLASS_TYPE

enum SbxClassType {         // SBX-class-IDs (order is important!)
    SbxCLASS_DONTCARE = 1,  // don't care (search, not 0 due to StarBASIC)
    SbxCLASS_ARRAY,         // Array of SbxVariables
    SbxCLASS_VALUE,         // simple value
    SbxCLASS_VARIABLE,      // Variable (from here there is Broadcaster)
    SbxCLASS_METHOD,        // Method (Function or Sub)
    SbxCLASS_PROPERTY,      // Property
    SbxCLASS_OBJECT         // Object
};

#endif

#ifndef _SBX_DATA_TYPE
#define _SBX_DATA_TYPE

enum SbxDataType {
    SbxEMPTY      =  0, // * Uninitialized
    SbxNULL       =  1, // * Contains no valid data
    SbxINTEGER    =  2, // * Integer (sal_Int16)
    SbxLONG       =  3, // * Long integer (sal_Int32)
    SbxSINGLE     =  4, // * Single-precision floating point number (float)
    SbxDOUBLE     =  5, // * Double-precision floating point number (double)
    SbxCURRENCY   =  6, //   Currency (sal_Int64)
    SbxDATE       =  7, // * Date (double)
    SbxSTRING     =  8, // * String (StarView)
    SbxOBJECT     =  9, // * SbxBase object pointer
    SbxERROR      = 10, // * Error (sal_uInt16)
    SbxBOOL       = 11, // * Boolean (0 or -1)
    SbxVARIANT    = 12, // * Anzeige fuer varianten Datentyp
    SbxDATAOBJECT = 13, // * Common data object w/o ref count

    SbxCHAR       = 16, // * signed char
    SbxBYTE       = 17, // * unsigned char
    SbxUSHORT     = 18, // * unsigned short (sal_uInt16)
    SbxULONG      = 19, // * unsigned long (sal_uInt32)
    SbxLONG64     = 20, //   signed 64-bit int
    SbxULONG64    = 21, //   unsigned 64-bit int
    SbxINT        = 22, // * signed machine-dependent int
    SbxUINT       = 23, // * unsigned machine-dependent int
    SbxVOID       = 24, // * no value (= SbxEMPTY)
    SbxHRESULT    = 25, //   HRESULT
    SbxPOINTER    = 26, //   generic pointer
    SbxDIMARRAY   = 27, //   dimensioned array
    SbxCARRAY     = 28, //   C style array
    SbxUSERDEF    = 29, //   user defined
    SbxLPSTR      = 30, // * null terminated string
    SbxLPWSTR     = 31, //   wide null terminated string
    SbxCoreSTRING = 32, //   AB 10.4.97, fuer GetCoreString(), nur zum Konvertieren
    SbxWSTRING    = 33, //   AB 4.10.2000 Reimplemented for backwards compatibility (#78919)
    SbxWCHAR      = 34, //   AB 4.10.2000 Reimplemented for backwards compatibility (#78919)
    SbxSALINT64   = 35, //   for UNO hyper
    SbxSALUINT64  = 36, //   for UNO unsigned hyper
    SbxDECIMAL    = 37, //   for UNO/automation Decimal

    SbxVECTOR = 0x1000, // simple counted array
    SbxARRAY  = 0x2000, // array
    SbxBYREF  = 0x4000, // access by reference

    SbxSV1    = 128,    // first defined data type for StarView
    SbxMEMORYSTREAM,    // SvMemoryStream
    SbxSTORAGE,         // SvStorage

    SbxUSER1  = 256,    // first user defined data type
    SbxUSERn  = 2047    // last user defined data type
};

const sal_uInt32 SBX_TYPE_WITH_EVENTS_FLAG = 0x10000;
const sal_uInt32 SBX_TYPE_DIM_AS_NEW_FLAG  = 0x20000;
const sal_uInt32 SBX_FIXED_LEN_STRING_FLAG = 0x10000;   // same value as above as no conflict possible

#endif

#ifndef _SBX_OPERATOR
#define _SBX_OPERATOR

enum SbxOperator {
                        // Arithmetical:
    SbxEXP,             // this ^ var
    SbxMUL,             // this * var
    SbxDIV,             // this / var
    SbxMOD,             // this MOD var
    SbxPLUS,            // this + var
    SbxMINUS,           // this - var
    SbxNEG,             // -this (var is ignored)
    SbxIDIV,            // this / var (both operands max. sal_Int32!)
                        // Boolean operators (max sal_Int32!):
    SbxAND,             // this & var
    SbxOR,              // this | var
    SbxXOR,             // this ^ var
    SbxEQV,             // ~this ^ var
    SbxIMP,             // ~this | var
    SbxNOT,             // ~this (var is ignored)
                        // String-concat:
    SbxCAT,             // this & var
                        // Comparisons:
    SbxEQ,              // this = var
    SbxNE,              // this <> var
    SbxLT,              // this < var
    SbxGT,              // this > var
    SbxLE,              // this <= var
    SbxGE               // this >= var
};

#endif

#ifndef _SBX_NAME_TYPE
#define _SBX_NAME_TYPE

enum SbxNameType {              // Type of the questioned name of a variable
    SbxNAME_NONE,               // plain name
    SbxNAME_SHORT,              // Name(A,B)
    SbxNAME_SHORT_TYPES,        // Name%(A%,B$)
    SbxNAME_LONG_TYPES          // Name(A As Integer, B As String) As Integer
};

#endif

// AB: 20.3.96: New error messages
typedef sal_uIntPtr SbxError;           // Preserve old type

#endif
// von #ifndef __RSC


// New error codes per define
#define ERRCODE_SBX_OK                          ERRCODE_NONE                    // processed
#define ERRCODE_SBX_SYNTAX                      (1UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_COMPILER)         // Syntaxerror in parser (where else could syntax errors happen? ;-)
#define ERRCODE_SBX_NOTIMP                      (2UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_NOTSUPPORTED)     // not possible
#define ERRCODE_SBX_OVERFLOW                    (3UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // overflow
#define ERRCODE_SBX_BOUNDS                      (4UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // Invalid array index
#define ERRCODE_SBX_ZERODIV                     (5UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // Division by zero
#define ERRCODE_SBX_CONVERSION                  (6UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // wrong data type
#define ERRCODE_SBX_BAD_PARAMETER               (7UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // invalid Parameter
#define ERRCODE_SBX_PROC_UNDEFINED              (8UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // BASIC-Sub or Function undefined
#define ERRCODE_SBX_ERROR                       (9UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_UNKNOWN)          // other object-related error
#define ERRCODE_SBX_NO_OBJECT                   (10UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Object variable unassigned
#define ERRCODE_SBX_CANNOT_LOAD                 (11UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_CREATE)           // Object cannot be loaded or initialized
#define ERRCODE_SBX_BAD_INDEX                   (12UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // Invalid object index
#define ERRCODE_SBX_NO_ACTIVE_OBJECT            (13UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_ACCESS)           // Object ist not activated
#define ERRCODE_SBX_BAD_PROP_VALUE              (14UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Bad property value
#define ERRCODE_SBX_PROP_READONLY               (15UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_READ)             // Property is read only
#define ERRCODE_SBX_PROP_WRITEONLY              (16UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_WRITE)            // Property is write only
#define ERRCODE_SBX_INVALID_OBJECT              (17UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_ACCESS)           // Invalid object reference
#define ERRCODE_SBX_NO_METHOD                   (18UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Property oder Methode unbekannt
#define ERRCODE_SBX_INVALID_USAGE_OBJECT        (19UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_ACCESS)           // Invalid object usage
#define ERRCODE_SBX_NO_OLE                      (20UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_ACCESS)           // No OLE-Object
#define ERRCODE_SBX_BAD_METHOD                  (21UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Method not supported
#define ERRCODE_SBX_OLE_ERROR                   (22UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // OLE Automation Error
#define ERRCODE_SBX_BAD_ACTION                  (23UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_NOTSUPPORTED)     // Action not supported
#define ERRCODE_SBX_NO_NAMED_ARGS               (24UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // No named arguments
#define ERRCODE_SBX_BAD_LOCALE                  (25UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_NOTSUPPORTED)     // Locale settings not supported
#define ERRCODE_SBX_NAMED_NOT_FOUND             (26UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Unknown named argument
#define ERRCODE_SBX_NOT_OPTIONAL                (27UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Argument not optional
#define ERRCODE_SBX_WRONG_ARGS                  (28UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_SBX)              // Invalid number of arguments
#define ERRCODE_SBX_NOT_A_COLL                  (29UL | ERRCODE_AREA_SBX | \
                                                ERRCODE_CLASS_RUNTIME)          // Object contains no elements
#define LAST_SBX_ERROR_ID                       29UL


// Less important for resources
#ifndef __RSC

// Map old codes to new ones
#define SbxERR_OK                               ERRCODE_SBX_OK
#define SbxERR_SYNTAX                           ERRCODE_SBX_SYNTAX
#define SbxERR_NOTIMP                           ERRCODE_SBX_NOTIMP
#define SbxERR_OVERFLOW                         ERRCODE_SBX_OVERFLOW
#define SbxERR_BOUNDS                           ERRCODE_SBX_BOUNDS
#define SbxERR_ZERODIV                          ERRCODE_SBX_ZERODIV
#define SbxERR_CONVERSION                       ERRCODE_SBX_CONVERSION
#define SbxERR_BAD_PARAMETER                    ERRCODE_SBX_BAD_PARAMETER
#define SbxERR_PROC_UNDEFINED                   ERRCODE_SBX_PROC_UNDEFINED
#define SbxERR_ERROR                            ERRCODE_SBX_ERROR
#define SbxERR_NO_OBJECT                        ERRCODE_SBX_NO_OBJECT
#define SbxERR_CANNOT_LOAD                      ERRCODE_SBX_CANNOT_LOAD
#define SbxERR_BAD_INDEX                        ERRCODE_SBX_BAD_INDEX
#define SbxERR_NO_ACTIVE_OBJECT                 ERRCODE_SBX_NO_ACTIVE_OBJECT
#define SbxERR_BAD_PROP_VALUE                   ERRCODE_SBX_BAD_PROP_VALUE
#define SbxERR_PROP_READONLY                    ERRCODE_SBX_PROP_READONLY
#define SbxERR_PROP_WRITEONLY                   ERRCODE_SBX_PROP_WRITEONLY
#define SbxERR_INVALID_OBJECT                   ERRCODE_SBX_INVALID_OBJECT
#define SbxERR_NO_METHOD                        ERRCODE_SBX_NO_METHOD
#define SbxERR_INVALID_USAGE_OBJECT             ERRCODE_SBX_INVALID_USAGE_OBJECT
#define SbxERR_NO_OLE                           ERRCODE_SBX_NO_OLE
#define SbxERR_BAD_METHOD                       ERRCODE_SBX_BAD_METHOD
#define SbxERR_OLE_ERROR                        ERRCODE_SBX_OLE_ERROR
#define SbxERR_BAD_ACTION                       ERRCODE_SBX_BAD_ACTION
#define SbxERR_NO_NAMED_ARGS                    ERRCODE_SBX_NO_NAMED_ARGS
#define SbxERR_BAD_LOCALE                       ERRCODE_SBX_BAD_LOCALE
#define SbxERR_NAMED_NOT_FOUND                  ERRCODE_SBX_NAMED_NOT_FOUND
#define SbxERR_NOT_OPTIONAL                     ERRCODE_SBX_NOT_OPTIONAL
#define SbxERR_WRONG_ARGS                       ERRCODE_SBX_WRONG_ARGS
#define SbxERR_NOT_A_COLL                       ERRCODE_SBX_NOT_A_COLL


/* Old codes with VB error codes
enum SbxError {                 // Ergebnis einer Rechenoperation/Konversion
    SbxERR_OK = 0,              // durchgefuehrt
    SbxERR_SYNTAX = 2,          // Syntaxfehler im Parser
    SbxERR_NOTIMP = 5,          // nicht moeglich
    SbxERR_OVERFLOW = 6,        // Ueberlauf
    SbxERR_BOUNDS = 9,          // Array-Index ungueltig
    SbxERR_ZERODIV = 11,        // Division durch Null
    SbxERR_CONVERSION = 13,     // falscher Datentyp
    SbxERR_BAD_PARAMETER = 14,  // ungÅltiger Parameter
    SbxERR_PROC_UNDEFINED = 35, // BASIC-Sub oder Function undefiniert
    SbxERR_ERROR = 51,          // andere Fehler
                                // Objektbezogene Fehler
    SbxERR_NO_OBJECT = 91,      // Objektvariable nicht belegt
    SbxERR_CANNOT_LOAD = 323,   // Objekt kann nicht geladen/eingerichtet werden
    SbxERR_BAD_INDEX = 341,     // Invalid object index
    SbxERR_NO_ACTIVE_OBJECT=366,// Objekt ist nicht aktiviert
    SbxERR_BAD_PROP_VALUE = 380,// Bad property value
    SbxERR_PROP_READONLY = 382, // Property is read only
    SbxERR_PROP_WRITEONLY = 394,// Property is write only
    SbxERR_INVALID_OBJECT = 420,// Invalid object reference
    SbxERR_NO_METHOD = 423,     // Property oder Methode unbekannt
    SbxERR_INVALID_USAGE_OBJECT=425,// Falsche Verwendung eines Objekts
    SbxERR_NO_OLE = 430,        // Kein OLE-Objekt
    SbxERR_BAD_METHOD = 438,    // Methode nicht unterstÅtzt
    SbxERR_OLE_ERROR = 440,     // OLE Automation-Fehler
    SbxERR_BAD_ACTION = 445,    // Aktion nicht unterstÅtzt
    SbxERR_NO_NAMED_ARGS = 446, // Keine benannten Argumente
    SbxERR_BAD_LOCALE = 447,    // Laenderspezifische Einstellungen nicht unterstÅtzt
    SbxERR_NAMED_NOT_FOUND = 448,// Unbekanntes benanntes Argument
    SbxERR_NOT_OPTIONAL = 449,  // Argument nicht optional
    SbxERR_WRONG_ARGS = 450,    // Falsche Zahl von Argumenten
    SbxERR_NOT_A_COLL = 451     // Objekt enthÑlt keine Elemente
};
*/

// Flag-Bits:
#define SBX_READ      0x0001    // Read permission
#define SBX_WRITE     0x0002    // Write permission
#define SBX_READWRITE 0x0003    // Read/Write permission
#define SBX_DONTSTORE 0x0004    // Don't store object
#define SBX_MODIFIED  0x0008    // Object was changed
#define SBX_FIXED     0x0010    // Fixed data type (SbxVariable)
#define SBX_CONST     0x0020    // Definition of const value
#define SBX_OPTIONAL  0x0040    // Parameter is optional
#define SBX_HIDDEN    0x0080    // Element is invisible
#define SBX_INVISIBLE 0x0100    // Element is not found by Find()
#define SBX_EXTSEARCH 0x0200    // Object is searched completely
#define SBX_EXTFOUND  0x0400    // Variable was found through extended search
#define SBX_GBLSEARCH 0x0800    // Global search via Parents
#define SBX_RESERVED  0x1000    // reserved
#define SBX_PRIVATE   0x1000    // #110004, #112015, cannot conflict with SBX_RESERVED
#define SBX_NO_BROADCAST 0x2000 // No broadcast on Get/Put
#define SBX_REFERENCE 0x4000    // Parameter is Reference (DLL-call)
#define SBX_NO_MODIFY 0x8000    // SetModified is suppressed
#define SBX_WITH_EVENTS 0x0080  // Same value as unused SBX_HIDDEN
#define SBX_DIM_AS_NEW  0x0800  // Same value as SBX_GBLSEARCH, cannot conflict as one
                                // is used for objects, the other for variables only

// Broadcaster-IDs:
#define SBX_HINT_DYING          SFX_HINT_DYING
#define SBX_HINT_DATAWANTED     SFX_HINT_USER00
#define SBX_HINT_DATACHANGED    SFX_HINT_DATACHANGED
#define SBX_HINT_CONVERTED      SFX_HINT_USER01
#define SBX_HINT_INFOWANTED     SFX_HINT_USER02
#define SBX_HINT_OBJECTCHANGED  SFX_HINT_USER03

// List of all creators for Load/Store

#define SBXCR_SBX           0x20584253      // SBX(blank)

// List of predefined SBX-IDs. New SBX-IDs must be precisly defined so that
// they are unique within the Stream and appropriate Factory.

#define SBXID_VALUE         0x4E4E  // NN: SbxValue
#define SBXID_VARIABLE      0x4156  // VA: SbxVariable
#define SBXID_ARRAY         0x5241  // AR: SbxArray
#define SBXID_DIMARRAY      0x4944  // DI: SbxDimArray
#define SBXID_OBJECT        0x424F  // OB: SbxObject
#define SBXID_COLLECTION    0x4F43  // CO: SbxCollection
#define SBXID_FIXCOLLECTION 0x4346  // FC: SbxStdCollection
#define SBXID_METHOD        0x454D  // ME: SbxMethod
#define SBXID_PROPERTY      0x5250  // PR: SbxProperty

// StarBASIC restricts the base data type to different intervals.
// These intervals are fixed due to portability and independent
// of the implementation. Only type double is greedy and takes
// what it gets.

#define SbxMAXCHAR          ((sal_Unicode)65535)
#define SbxMINCHAR          (0)
#define SbxMAXBYTE          ( 255)
#define SbxMAXINT           ( 32767)
#define SbxMININT           (-32768)
#define SbxMAXUINT          ((sal_uInt16) 65535)
#define SbxMAXLNG           ( 2147483647)
#define SbxMINLNG           ((sal_Int32)(-2147483647-1))
#define SbxMAXULNG          ((sal_uInt32) 0xffffffff)

#define SbxMAXSALINT64      SAL_MAX_INT64
#define SbxMINSALINT64      SAL_MIN_INT64
#define SbxMAXSALUINT64     SAL_MAX_UINT64

#define SbxMAXSNG           ( 3.402823e+38)
#define SbxMINSNG           (-3.402823e+38)
#define SbxMAXSNG2          ( 1.175494351e-38)
#define SbxMINSNG2          (-1.175494351e-38)
#define SbxMAXCURR          ( 922337203685477.5807)
#define SbxMINCURR          (-922337203685477.5808)
#define CURRENCY_FACTOR     10000
#define SbxMAXCURRLNG       (SbxMAXLNG/CURRENCY_FACTOR)
#define SbxMINCURRLNG       (SbxMINLNG/CURRENCY_FACTOR)

// Max valid offset index of a Sbx-Array (due to 64K limit)
#define SBX_MAXINDEX        0x3FF0
#define SBX_MAXINDEX32      SbxMAXLNG

// The numeric values of sal_True and FALSE
enum SbxBOOL { SbxFALSE = 0, SbxTRUE = -1 };

#endif // __RSC

#endif
