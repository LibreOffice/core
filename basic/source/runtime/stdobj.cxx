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


#include <runtime.hxx>
#include <stdobj.hxx>
#include <sbstdobj.hxx>
#include <rtlproto.hxx>
#include <sbintern.hxx>
// The nArgs-field of a table entry is encrypted as follows:
// At the moment it is assumed that properties don't need any
// parameters!

// previously ARGSMASK_ was 0x007F ( e.g. up to 127 args ) however 63 should be
// enough, if not we need to increase the size of nArgs member in the Methods
// struct below.
// note: the limitation of 63 args is only for RTL functions defined here and
// does NOT impose a limit on User defined procedures ). This changes is to
// allow us space for a flag to denylist some functions in vba mode

#define ARGSMASK_   0x003F  // 63 Arguments
#define COMPTMASK_  0x00C0  // COMPATIBILITY mask
#define COMPATONLY_ 0x0080  // procedure is visible in vba mode only
#define NORMONLY_   0x0040  // procedure is visible in normal mode only

#define RWMASK_     0x0F00  // mask for R/W-bits
#define TYPEMASK_   0xF000  // mask for the entry's type

#define OPT_        0x0400  // parameter is optional
#define CONST_      0x0800  // property is const
#define METHOD_     0x3000
#define PROPERTY_   0x4000
#define OBJECT_     0x8000
                            // combination of bits above:
#define FUNCTION_   0x1100
#define LFUNCTION_  0x1300  // mask for function which also works as Lvalue
#define SUB_        0x2100
#define ROPROP_     0x4100  // mask Read Only-Property
#define RWPROP_     0x4300  // mask Read/Write-Property
#define CPROP_      0x4900  // mask for constant

namespace {

struct Methods {
    const char* pName;
    SbxDataType eType;
    short       nArgs;
    RtlCall     pFunc;
    sal_uInt16      nHash;
};

}

static Methods aMethods[] = {

{ "Abs",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Abs),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Array",          SbxOBJECT,        FUNCTION_, RTLNAME(Array),0           },
{ "Asc",            SbxLONG,      1 | FUNCTION_, RTLNAME(Asc),0             },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "AscW",           SbxLONG,      1 | FUNCTION_ | COMPATONLY_, RTLNAME(Asc),0},
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Atn",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Atn),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "ATTR_ARCHIVE",   SbxINTEGER,       CPROP_,    RTLNAME(ATTR_ARCHIVE),0    },
{ "ATTR_DIRECTORY", SbxINTEGER,       CPROP_,    RTLNAME(ATTR_DIRECTORY),0  },
{ "ATTR_HIDDEN",    SbxINTEGER,       CPROP_,    RTLNAME(ATTR_HIDDEN),0     },
{ "ATTR_NORMAL",    SbxINTEGER,       CPROP_,    RTLNAME(ATTR_NORMAL),0     },
{ "ATTR_READONLY",  SbxINTEGER,       CPROP_,    RTLNAME(ATTR_READONLY),0   },
{ "ATTR_SYSTEM",    SbxINTEGER,       CPROP_,    RTLNAME(ATTR_SYSTEM),0     },
{ "ATTR_VOLUME",    SbxINTEGER,       CPROP_,    RTLNAME(ATTR_VOLUME),0     },

{ "Beep",           SbxNULL,          FUNCTION_, RTLNAME(Beep),0            },
{ "Blue",        SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, RTLNAME(Blue),0               },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },

{ "CallByName",     SbxVARIANT,   3 | FUNCTION_, RTLNAME(CallByName),0 },
  { "Object",       SbxOBJECT,  0,nullptr,0 },
  { "ProcedureName",SbxSTRING,  0,nullptr,0 },
  { "CallType",     SbxINTEGER, 0,nullptr,0 },
{ "CBool",          SbxBOOL,      1 | FUNCTION_, RTLNAME(CBool),0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CByte",          SbxBYTE,      1 | FUNCTION_, RTLNAME(CByte),0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CCur",           SbxCURRENCY,  1 | FUNCTION_, RTLNAME(CCur),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDate",          SbxDATE,      1 | FUNCTION_, RTLNAME(CDate),0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDateFromUnoDate",   SbxDATE,      1 | FUNCTION_, RTLNAME(CDateFromUnoDate),0    },
  { "UnoDate",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoDate", SbxOBJECT,    1 | FUNCTION_, RTLNAME(CDateToUnoDate),0      },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "CDateFromUnoTime",   SbxDATE,      1 | FUNCTION_, RTLNAME(CDateFromUnoTime),0    },
  { "UnoTime",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoTime", SbxOBJECT,    1 | FUNCTION_, RTLNAME(CDateToUnoTime),0      },
  { "Time",         SbxDATE, 0,nullptr,0 },
{ "CDateFromUnoDateTime",   SbxDATE,      1 | FUNCTION_, RTLNAME(CDateFromUnoDateTime),0    },
  { "UnoDateTime",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoDateTime", SbxOBJECT,    1 | FUNCTION_, RTLNAME(CDateToUnoDateTime),0      },
  { "DateTime",         SbxDATE, 0,nullptr,0 },
{ "CDateFromIso",   SbxDATE,      1 | FUNCTION_, RTLNAME(CDateFromIso),0    },
  { "IsoDate",      SbxSTRING, 0,nullptr,0 },
{ "CDateToIso",     SbxSTRING,    1 | FUNCTION_, RTLNAME(CDateToIso),0      },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "CDec",           SbxDECIMAL,   1 | FUNCTION_, RTLNAME(CDec),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDbl",           SbxDOUBLE,    1 | FUNCTION_, RTLNAME(CDbl),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CF_BITMAP",      SbxINTEGER,       CPROP_,    RTLNAME(CF_BITMAP),0       },
{ "CF_METAFILEPICT",SbxINTEGER,       CPROP_,    RTLNAME(CF_METAFILEPICT),0 },
{ "CF_TEXT",        SbxINTEGER,       CPROP_,    RTLNAME(CF_TEXT),0         },
{ "ChDir",          SbxNULL,      1 | FUNCTION_, RTLNAME(ChDir),0           },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "ChDrive",        SbxNULL,      1 | FUNCTION_, RTLNAME(ChDrive),0         },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "Choose",         SbxVARIANT,   2 | FUNCTION_, RTLNAME(Choose),0          },
  { "Index",        SbxINTEGER, 0,nullptr,0 },
  { "Expression",   SbxVARIANT, 0,nullptr,0 },

{ "Chr",            SbxSTRING,    1 | FUNCTION_, RTLNAME(Chr),0             },
  { "string",       SbxINTEGER, 0,nullptr,0 },
{ "ChrW",           SbxSTRING,    1 | FUNCTION_ | COMPATONLY_, RTLNAME(ChrW),0},
  { "string",       SbxINTEGER, 0,nullptr,0 },

{ "CInt",           SbxINTEGER,   1 | FUNCTION_, RTLNAME(CInt),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CLEAR_ALLTABS",  SbxINTEGER,       CPROP_,    RTLNAME(CLEAR_ALLTABS),0   },
{ "CLEAR_TAB",      SbxINTEGER,       CPROP_,    RTLNAME(CLEAR_TAB),0       },
{ "CLng",           SbxLONG,      1 | FUNCTION_, RTLNAME(CLng),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CompatibilityMode",  SbxBOOL,  1 | FUNCTION_, RTLNAME(CompatibilityMode),0},
  { "bEnable",      SbxBOOL, 0,nullptr,0 },
{ "ConvertFromUrl", SbxSTRING,    1 | FUNCTION_, RTLNAME(ConvertFromUrl),0  },
  { "Url",          SbxSTRING, 0,nullptr,0 },
{ "ConvertToUrl",   SbxSTRING,    1 | FUNCTION_, RTLNAME(ConvertToUrl),0    },
  { "SystemPath",   SbxSTRING, 0,nullptr,0 },
{ "Cos",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Cos),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "CreateObject",   SbxOBJECT,    1 | FUNCTION_, RTLNAME( CreateObject ),0  },
  { "class",        SbxSTRING, 0,nullptr,0 },
{ "CreateUnoListener",SbxOBJECT,   1 | FUNCTION_, RTLNAME( CreateUnoListener ),0 },
  { "prefix",  SbxSTRING, 0,nullptr,0 },
  { "typename",  SbxSTRING, 0,nullptr,0 },
{ "CreateUnoDialog",SbxOBJECT,    2 | FUNCTION_, RTLNAME( CreateUnoDialog ),0 },
  { "dialoglibrary",SbxOBJECT, 0,nullptr,0 },
  { "dialogname",   SbxSTRING, 0,nullptr,0 },
{ "CreateUnoService",SbxOBJECT,   1 | FUNCTION_, RTLNAME( CreateUnoService ),0 },
  { "servicename",  SbxSTRING, 0,nullptr,0 },
{ "CreateUnoServiceWithArguments",SbxOBJECT,   2 | FUNCTION_, RTLNAME( CreateUnoServiceWithArguments ),0 },
  { "servicename",  SbxSTRING, 0,nullptr,0 },
  { "arguments",  SbxARRAY, 0,nullptr,0 },
{ "CreateUnoStruct",SbxOBJECT,    1 | FUNCTION_, RTLNAME( CreateUnoStruct ),0 },
  { "classname",    SbxSTRING, 0,nullptr,0 },
{ "CreateUnoValue", SbxOBJECT,    2 | FUNCTION_, RTLNAME( CreateUnoValue ),0 },
  { "type",         SbxSTRING, 0,nullptr,0 },
  { "value",        SbxVARIANT, 0,nullptr,0 },
{ "CreatePropertySet",SbxOBJECT, 1 | FUNCTION_, RTLNAME( CreatePropertySet ),0 },
  { "values",  SbxARRAY, 0,nullptr,0 },
{ "CSng",           SbxSINGLE,    1 | FUNCTION_, RTLNAME(CSng),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CStr",           SbxSTRING,    1 | FUNCTION_, RTLNAME(CStr),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CurDir",         SbxSTRING,    1 | FUNCTION_, RTLNAME(CurDir),0          },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "CVar",           SbxVARIANT,   1 | FUNCTION_, RTLNAME(CVar),0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CVErr",          SbxVARIANT,   1 | FUNCTION_, RTLNAME(CVErr),0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "DDB",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, RTLNAME(DDB),0       },
  { "Cost",       SbxDOUBLE,  0, nullptr,0 },
  { "Salvage",       SbxDOUBLE,  0, nullptr,0 },
  { "Life",       SbxDOUBLE,  0, nullptr,0 },
  { "Period",       SbxDOUBLE,  0, nullptr,0 },
  { "Factor",     SbxVARIANT,  OPT_, nullptr,0 },
{ "Date",           SbxDATE,          LFUNCTION_,RTLNAME(Date),0            },
{ "DateAdd",        SbxDATE,      3 | FUNCTION_, RTLNAME(DateAdd),0         },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Number",       SbxLONG, 0,nullptr,0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "DateDiff",       SbxDOUBLE,    5 | FUNCTION_, RTLNAME(DateDiff),0        },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Date1",        SbxDATE, 0,nullptr,0 },
  { "Date2",        SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek" , SbxINTEGER, OPT_,nullptr,0 },
  { "Firstweekofyear", SbxINTEGER, OPT_,nullptr,0 },
{ "DatePart",       SbxLONG,      4 | FUNCTION_, RTLNAME(DatePart),0        },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek" , SbxINTEGER, OPT_, nullptr,0 },
  { "Firstweekofyear", SbxINTEGER, OPT_, nullptr,0 },
{ "DateSerial",     SbxDATE,      3 | FUNCTION_, RTLNAME(DateSerial),0      },
  { "Year",         SbxINTEGER, 0,nullptr,0 },
  { "Month",        SbxINTEGER, 0,nullptr,0 },
  { "Day",          SbxINTEGER, 0,nullptr,0 },
{ "DateValue",      SbxDATE,      1 | FUNCTION_, RTLNAME(DateValue),0       },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "Day",            SbxINTEGER,   1 | FUNCTION_, RTLNAME(Day),0             },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "Ddeexecute",     SbxNULL,   2 | FUNCTION_, RTLNAME(DDEExecute),0         },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Command",         SbxSTRING, 0,nullptr,0 },
{ "Ddeinitiate",    SbxINTEGER,   2 | FUNCTION_, RTLNAME(DDEInitiate),0     },
  { "Application",     SbxSTRING, 0,nullptr,0 },
  { "Topic",           SbxSTRING, 0,nullptr,0 },
{ "Ddepoke",        SbxNULL,   3 | FUNCTION_, RTLNAME(DDEPoke),0            },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Item",            SbxSTRING, 0,nullptr,0 },
  { "Data",            SbxSTRING, 0,nullptr,0 },
{ "Dderequest",     SbxSTRING,   2 | FUNCTION_, RTLNAME(DDERequest),0       },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Item",            SbxSTRING, 0,nullptr,0 },
{ "Ddeterminate",   SbxNULL,   1 | FUNCTION_, RTLNAME(DDETerminate),0       },
  { "Channel",         SbxLONG, 0,nullptr,0 },
{ "Ddeterminateall",   SbxNULL,   FUNCTION_, RTLNAME(DDETerminateAll),0     },
{ "DimArray",       SbxOBJECT,    FUNCTION_, RTLNAME(DimArray),0            },
{ "Dir",            SbxSTRING,    2 | FUNCTION_, RTLNAME(Dir),0             },
  { "FileSpec",     SbxSTRING,        OPT_, nullptr,0 },
  { "attrmask",     SbxINTEGER,       OPT_, nullptr,0 },
{ "DoEvents",       SbxINTEGER,     FUNCTION_, RTLNAME(DoEvents),0          },
{ "DumpAllObjects", SbxEMPTY,     2 | SUB_, RTLNAME(DumpAllObjects),0       },
  { "FileSpec",     SbxSTRING, 0,nullptr,0 },
  { "DumpAll",      SbxINTEGER,       OPT_, nullptr,0 },

{ "Empty",        SbxVARIANT,        CPROP_,    RTLNAME(Empty),0         },
{ "EqualUnoObjects",SbxBOOL,      2 | FUNCTION_, RTLNAME(EqualUnoObjects),0 },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "EnableReschedule",   SbxNULL,  1 | FUNCTION_, RTLNAME(EnableReschedule),0},
  { "bEnable",      SbxBOOL, 0,nullptr,0 },
{ "Environ",            SbxSTRING,   1 | FUNCTION_, RTLNAME(Environ),0      },
  { "Environmentstring",SbxSTRING, 0,nullptr,0 },
{ "EOF",            SbxBOOL,      1 | FUNCTION_, RTLNAME(EOF),0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Erl",            SbxLONG,          ROPROP_,   RTLNAME( Erl ),0           },
{ "Err",            SbxVARIANT,       RWPROP_,   RTLNAME( Err ),0           },
{ "Error",          SbxSTRING,    1 | FUNCTION_, RTLNAME( Error ),0         },
  { "code",         SbxLONG, 0,nullptr,0 },
{ "Exp",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Exp),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },

{ "False",          SbxBOOL,          CPROP_,    RTLNAME(False),0           },
{ "FileAttr",       SbxINTEGER,   2 | FUNCTION_, RTLNAME(FileAttr),0        },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
  { "Attributes",   SbxINTEGER, 0,nullptr,0 },
{ "FileCopy",       SbxNULL,      2 | FUNCTION_, RTLNAME(FileCopy),0        },
  { "Source",       SbxSTRING, 0,nullptr,0 },
  { "Destination",  SbxSTRING, 0,nullptr,0 },
{ "FileDateTime",   SbxSTRING,    1 | FUNCTION_, RTLNAME(FileDateTime),0    },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FileExists",     SbxBOOL,   1 | FUNCTION_, RTLNAME(FileExists),0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FileLen",        SbxLONG,      1 | FUNCTION_, RTLNAME(FileLen),0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FindObject",     SbxOBJECT,    1 | FUNCTION_, RTLNAME(FindObject),0      },
  { "Name",  SbxSTRING, 0,nullptr,0 },
{ "FindPropertyObject",   SbxOBJECT,      2 | FUNCTION_, RTLNAME(FindPropertyObject),0 },
  { "Object",       SbxOBJECT, 0,nullptr,0 },
  { "Name",  SbxSTRING, 0,nullptr,0 },
{ "Fix",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Fix),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Format",         SbxSTRING,    2 | FUNCTION_, RTLNAME(Format),0          },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
  { "format",       SbxSTRING,        OPT_, nullptr,0 },
{ "FormatDateTime", SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, RTLNAME(FormatDateTime),0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "NamedFormat",  SbxINTEGER,        OPT_, nullptr,0 },
{ "FormatNumber",   SbxSTRING, 5 | FUNCTION_ | COMPATONLY_, RTLNAME(FormatNumber), 0 },
  { "expression",                  SbxDOUBLE,  0,    nullptr, 0 },
  { "numDigitsAfterDecimal",       SbxINTEGER, OPT_, nullptr, 0 },
  { "includeLeadingDigit",         SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
  { "useParensForNegativeNumbers", SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
  { "groupDigits",                 SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
{ "Frac",           SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Frac),0            },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "FRAMEANCHORCHAR",        SbxINTEGER,       CPROP_,    RTLNAME(FRAMEANCHORCHAR),0 },
{ "FRAMEANCHORPAGE",        SbxINTEGER,       CPROP_,    RTLNAME(FRAMEANCHORPAGE),0 },
{ "FRAMEANCHORPARA",        SbxINTEGER,       CPROP_,    RTLNAME(FRAMEANCHORPARA),0 },
{ "FreeFile",       SbxINTEGER,    FUNCTION_, RTLNAME(FreeFile),0           },
{ "FreeLibrary",    SbxNULL,      1 | FUNCTION_, RTLNAME(FreeLibrary),0     },
  { "Modulename",   SbxSTRING, 0,nullptr,0 },

{ "FV",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, RTLNAME(FV),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "Get",            SbxNULL,   3 | FUNCTION_, RTLNAME(Get),0                },
  { "filenumber",   SbxINTEGER, 0,nullptr,0 },
  { "recordnumber", SbxLONG, 0,nullptr,0 },
  { "variablename", SbxVARIANT, 0,nullptr,0 },
{ "GetAttr",        SbxINTEGER,   1 | FUNCTION_, RTLNAME(GetAttr),0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "GetDefaultContext", SbxOBJECT, 0 | FUNCTION_, RTLNAME(GetDefaultContext),0 },
{ "GetDialogZoomFactorX",  SbxDOUBLE,     FUNCTION_,RTLNAME(GetDialogZoomFactorX),0 },
{ "GetDialogZoomFactorY",  SbxDOUBLE,     FUNCTION_,RTLNAME(GetDialogZoomFactorY),0 },
{ "GetGUIType",  SbxINTEGER,     FUNCTION_,RTLNAME(GetGUIType),0            },
{ "GetGUIVersion",  SbxLONG,     FUNCTION_,RTLNAME(GetGUIVersion),0         },
{ "GetPathSeparator",  SbxSTRING,     FUNCTION_,RTLNAME(GetPathSeparator),0 },
{ "GetProcessServiceManager", SbxOBJECT, 0 | FUNCTION_, RTLNAME(GetProcessServiceManager),0 },
{ "GetSolarVersion",  SbxLONG,     FUNCTION_,RTLNAME(GetSolarVersion),0     },
{ "GetSystemTicks",  SbxLONG,      FUNCTION_,RTLNAME(GetSystemTicks),0      },
{ "GetSystemType",  SbxINTEGER,    FUNCTION_,RTLNAME(GetSystemType),0       },
{ "GlobalScope",    SbxOBJECT,     FUNCTION_,RTLNAME(GlobalScope),0         },
{ "Green",          SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, RTLNAME(Green),0           },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },

{ "HasUnoInterfaces",   SbxBOOL,  1 | FUNCTION_, RTLNAME(HasUnoInterfaces),0},
  { "InterfaceName",SbxSTRING, 0,nullptr,0 },
{ "Hex",            SbxSTRING,    1 | FUNCTION_, RTLNAME(Hex),0             },
  { "number",       SbxLONG, 0,nullptr,0 },
{ "Hour",           SbxINTEGER,   1 | FUNCTION_, RTLNAME(Hour),0            },
  { "Date",         SbxDATE, 0,nullptr,0 },

{ "IDABORT",        SbxINTEGER,       CPROP_,    RTLNAME(IDABORT),0         },
{ "IDCANCEL",       SbxINTEGER,       CPROP_,    RTLNAME(IDCANCEL),0        },
{ "IDNO",           SbxINTEGER,       CPROP_,    RTLNAME(IDNO),0            },
{ "IDOK",           SbxINTEGER,       CPROP_,    RTLNAME(IDOK),0            },
{ "IDRETRY",        SbxINTEGER,       CPROP_,    RTLNAME(IDRETRY),0         },
{ "IDYES",          SbxINTEGER,       CPROP_,    RTLNAME(IDYES),0           },

{ "Iif",            SbxVARIANT,   3 | FUNCTION_, RTLNAME(Iif),0             },
  { "Bool",         SbxBOOL, 0,nullptr,0 },
  { "Variant1",     SbxVARIANT, 0,nullptr,0 },
  { "Variant2",     SbxVARIANT, 0,nullptr,0 },

{ "Input",          SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, RTLNAME(Input),0},
  { "Number",       SbxLONG, 0,nullptr,0 },
  { "FileNumber",   SbxLONG, 0,nullptr,0 },
{ "InputBox",       SbxSTRING,    5 | FUNCTION_, RTLNAME(InputBox),0        },
  { "Prompt",       SbxSTRING, 0,nullptr,0 },
  { "Title",        SbxSTRING,        OPT_, nullptr,0 },
  { "Default",      SbxSTRING,        OPT_, nullptr,0 },
  { "XPosTwips",    SbxLONG,          OPT_, nullptr,0 },
  { "YPosTwips",    SbxLONG,          OPT_, nullptr,0 },
{ "InStr",          SbxLONG,      4 | FUNCTION_, RTLNAME(InStr),0           },
  { "Start",        SbxSTRING,        OPT_, nullptr,0 },
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "InStrRev",       SbxLONG,      4 | FUNCTION_ | COMPATONLY_, RTLNAME(InStrRev),0},
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Start",        SbxSTRING,        OPT_, nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "Int",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Int),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "IPmt",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, RTLNAME(IPmt),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Per",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "IRR",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, RTLNAME(IRR),0       },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
  { "Guess",       SbxVARIANT,  OPT_, nullptr,0 },
{ "IsArray",        SbxBOOL,      1 | FUNCTION_, RTLNAME(IsArray),0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsDate",         SbxBOOL,      1 | FUNCTION_, RTLNAME(IsDate),0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsEmpty",        SbxBOOL,      1 | FUNCTION_, RTLNAME(IsEmpty),0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsError",        SbxBOOL,      1 | FUNCTION_, RTLNAME(IsError),0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsMissing",        SbxBOOL,      1 | FUNCTION_, RTLNAME(IsMissing),0     },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsNull",         SbxBOOL,      1 | FUNCTION_, RTLNAME(IsNull),0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsNumeric",      SbxBOOL,      1 | FUNCTION_, RTLNAME(IsNumeric),0       },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsObject",       SbxBOOL,      1 | FUNCTION_, RTLNAME(IsObject),0        },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsUnoStruct",    SbxBOOL,      1 | FUNCTION_, RTLNAME(IsUnoStruct),0     },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "Join",           SbxSTRING,      2 | FUNCTION_, RTLNAME(Join),0          },
  { "list",         SbxOBJECT, 0,nullptr,0 },
  { "delimiter",    SbxSTRING, 0,nullptr,0 },
{ "Kill",           SbxNULL,      1 | FUNCTION_, RTLNAME(Kill),0            },
  { "filespec",     SbxSTRING, 0,nullptr,0 },
{ "LBound",         SbxLONG,      1 | FUNCTION_, RTLNAME(LBound),0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "LCase",          SbxSTRING,    1 | FUNCTION_, RTLNAME(LCase),0           },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Left",           SbxSTRING,    2 | FUNCTION_, RTLNAME(Left),0            },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Len",            SbxLONG,      1 | FUNCTION_, RTLNAME(Len),0             },
  { "StringOrVariant", SbxVARIANT, 0,nullptr,0 },
{ "LenB",           SbxLONG,      1 | FUNCTION_, RTLNAME(Len),0             },
  { "StringOrVariant", SbxVARIANT, 0,nullptr,0 },
{ "Load",           SbxNULL,      1 | FUNCTION_, RTLNAME(Load),0            },
  { "object",       SbxOBJECT, 0,nullptr,0 },
{ "LoadPicture",    SbxOBJECT,    1 | FUNCTION_, RTLNAME(LoadPicture),0     },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Loc",            SbxLONG,      1 | FUNCTION_, RTLNAME(Loc),0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Lof",            SbxLONG,      1 | FUNCTION_, RTLNAME(Lof),0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Log",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Log),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "LTrim",          SbxSTRING,    1 | FUNCTION_, RTLNAME(LTrim),0           },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "MB_ABORTRETRYIGNORE", SbxINTEGER,  CPROP_,    RTLNAME(MB_ABORTRETRYIGNORE),0},
{ "MB_APPLMODAL",   SbxINTEGER,       CPROP_,    RTLNAME(MB_APPLMODAL),0    },
{ "MB_DEFBUTTON1",  SbxINTEGER,       CPROP_,    RTLNAME(MB_DEFBUTTON1),0   },
{ "MB_DEFBUTTON2",  SbxINTEGER,       CPROP_,    RTLNAME(MB_DEFBUTTON2),0   },
{ "MB_DEFBUTTON3",  SbxINTEGER,       CPROP_,    RTLNAME(MB_DEFBUTTON3),0   },
{ "MB_ICONEXCLAMATION", SbxINTEGER,   CPROP_,    RTLNAME(MB_ICONEXCLAMATION),0},
{ "MB_ICONINFORMATION", SbxINTEGER,   CPROP_,    RTLNAME(MB_ICONINFORMATION),0},
{ "MB_ICONQUESTION",SbxINTEGER,       CPROP_,    RTLNAME(MB_ICONQUESTION),0 },
{ "MB_ICONSTOP",    SbxINTEGER,       CPROP_,    RTLNAME(MB_ICONSTOP),0     },
{ "MB_OK",          SbxINTEGER,       CPROP_,    RTLNAME(MB_OK),0           },
{ "MB_OKCANCEL",    SbxINTEGER,       CPROP_,    RTLNAME(MB_OKCANCEL),0     },
{ "MB_RETRYCANCEL", SbxINTEGER,       CPROP_,    RTLNAME(MB_RETRYCANCEL),0  },
{ "MB_SYSTEMMODAL", SbxINTEGER,       CPROP_,    RTLNAME(MB_SYSTEMMODAL),0  },
{ "MB_YESNO",       SbxINTEGER,       CPROP_,    RTLNAME(MB_YESNO),0        },
{ "MB_YESNOCANCEL", SbxINTEGER,       CPROP_,    RTLNAME(MB_YESNOCANCEL),0  },

{ "Me",             SbxOBJECT,    0 | FUNCTION_ | COMPATONLY_, RTLNAME(Me),0 },
{ "Mid",            SbxSTRING,    3 | LFUNCTION_,RTLNAME(Mid),0             },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "StartPos",     SbxLONG, 0,nullptr,0 },
  { "Length",       SbxLONG,          OPT_, nullptr,0 },
{ "Minute",         SbxINTEGER,   1 | FUNCTION_, RTLNAME(Minute),0          },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "MIRR",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, RTLNAME(MIRR),0       },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
  { "FinanceRate",       SbxDOUBLE,  0, nullptr,0 },
  { "ReinvestRate",       SbxDOUBLE,  0, nullptr,0 },
{ "MkDir",          SbxNULL,      1 | FUNCTION_, RTLNAME(MkDir),0           },
  { "pathname",     SbxSTRING, 0,nullptr,0 },
{ "Month",          SbxINTEGER,   1 | FUNCTION_, RTLNAME(Month),0           },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "MonthName",      SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, RTLNAME(MonthName),0 },
  { "Month",        SbxINTEGER, 0,nullptr,0 },
  { "Abbreviate",   SbxBOOL,          OPT_, nullptr,0 },
{ "MsgBox",         SbxINTEGER,    5 | FUNCTION_, RTLNAME(MsgBox),0         },
  { "Prompt",       SbxSTRING, 0,nullptr,0 },
  { "Buttons",      SbxINTEGER,       OPT_, nullptr,0 },
  { "Title",        SbxSTRING,        OPT_, nullptr,0 },
  { "Helpfile",     SbxSTRING,        OPT_, nullptr,0 },
  { "Context",      SbxINTEGER,       OPT_, nullptr,0 },

{ "Nothing",        SbxOBJECT,        CPROP_,    RTLNAME(Nothing),0         },
{ "Now",            SbxDATE,          FUNCTION_, RTLNAME(Now),0             },
{ "NPer",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, RTLNAME(NPer),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",       SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "NPV",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, RTLNAME(NPV),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
{ "Null",           SbxNULL,          CPROP_,    RTLNAME(Null),0            },

{ "Oct",            SbxSTRING,    1 | FUNCTION_, RTLNAME(Oct),0             },
  { "number",       SbxLONG, 0,nullptr,0 },

{ "Partition",      SbxSTRING,    4 | FUNCTION_, RTLNAME(Partition),0       },
  { "number",       SbxLONG,    0,nullptr,0 },
  { "start",        SbxLONG,    0,nullptr,0 },
  { "stop",         SbxLONG,    0,nullptr,0 },
  { "interval",     SbxLONG,    0,nullptr,0 },
{ "Pi",             SbxDOUBLE,        CPROP_,    RTLNAME(PI),0              },

{ "Pmt",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, RTLNAME(Pmt),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "PPmt",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, RTLNAME(PPmt),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Per",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "Put",            SbxNULL,   3 | FUNCTION_, RTLNAME(Put),0                },
  { "filenumber",   SbxINTEGER, 0,nullptr,0 },
  { "recordnumber", SbxLONG, 0,nullptr,0 },
  { "variablename", SbxVARIANT, 0,nullptr,0 },

{ "PV",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, RTLNAME(PV),0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "QBColor",        SbxLONG,      1 | FUNCTION_, RTLNAME(QBColor),0         },
  { "number",       SbxINTEGER, 0,nullptr,0 },

{ "Randomize",      SbxNULL,      1 | FUNCTION_, RTLNAME(Randomize),0       },
  { "Number",       SbxDOUBLE,        OPT_, nullptr,0 },
{ "Rate",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, RTLNAME(Rate),0       },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",       SbxDOUBLE,  0, nullptr,0 },
  { "FV",       SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Guess",    SbxVARIANT,  OPT_, nullptr,0 },
{ "Red",        SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, RTLNAME(Red),0                 },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },
{ "Reset",          SbxNULL,      0 | FUNCTION_, RTLNAME(Reset),0           },
{ "ResolvePath",    SbxSTRING,    1 | FUNCTION_, RTLNAME(ResolvePath),0     },
  { "Path",         SbxSTRING, 0,nullptr,0 },
{ "RGB",            SbxLONG,      3 | FUNCTION_, RTLNAME(RGB),0             },
  { "Red",          SbxINTEGER, 0,nullptr,0 },
  { "Green",        SbxINTEGER, 0,nullptr,0 },
  { "Blue",         SbxINTEGER, 0,nullptr,0 },
{ "Replace",        SbxSTRING,    6 | FUNCTION_, RTLNAME(Replace),0         },
  { "Expression",   SbxSTRING, 0,nullptr,0 },
  { "Find",         SbxSTRING, 0,nullptr,0 },
  { "Replace",      SbxSTRING, 0,nullptr,0 },
  { "Start",        SbxINTEGER,     OPT_, nullptr,0 },
  { "Count",        SbxINTEGER,     OPT_, nullptr,0 },
  { "Compare",      SbxINTEGER,     OPT_, nullptr,0 },
{ "Right",          SbxSTRING,    2 | FUNCTION_, RTLNAME(Right),0           },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "RmDir",          SbxNULL,      1 | FUNCTION_, RTLNAME(RmDir),0           },
  { "pathname",     SbxSTRING, 0,nullptr,0 },
{ "Round",          SbxDOUBLE,    2 | FUNCTION_ | COMPATONLY_, RTLNAME(Round),0},
  { "Expression",   SbxDOUBLE, 0,nullptr,0 },
  { "Numdecimalplaces", SbxINTEGER,   OPT_, nullptr,0 },
{ "Rnd",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Rnd),0             },
  { "Number",       SbxDOUBLE,        OPT_, nullptr,0 },
{ "RTL",            SbxOBJECT,    0 | FUNCTION_ | COMPATONLY_, RTLNAME(RTL),0},
{ "RTrim",          SbxSTRING,    1 | FUNCTION_, RTLNAME(RTrim),0           },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "SavePicture",    SbxNULL,      2 | FUNCTION_, RTLNAME(SavePicture),0     },
  { "object",       SbxOBJECT, 0,nullptr,0 },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Second",         SbxINTEGER,   1 | FUNCTION_, RTLNAME(Second),0          },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "Seek",           SbxLONG,      1 | FUNCTION_, RTLNAME(Seek),0            },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "SendKeys",       SbxNULL,      2 | FUNCTION_, RTLNAME(SendKeys),0        },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Wait",         SbxBOOL,          OPT_, nullptr,0 },
{ "SetAttr",        SbxNULL,      2 | FUNCTION_, RTLNAME(SetAttr),0         },
  { "File"  ,       SbxSTRING, 0,nullptr,0 },
  { "Attributes",   SbxINTEGER, 0,nullptr,0 },
{ "SET_OFF",        SbxINTEGER,       CPROP_,    RTLNAME(SET_OFF),0         },
{ "SET_ON",         SbxINTEGER,       CPROP_,    RTLNAME(SET_ON),0          },
{ "SET_TAB",        SbxINTEGER,       CPROP_,    RTLNAME(SET_TAB),0         },
{ "Sgn",            SbxINTEGER,   1 | FUNCTION_, RTLNAME(Sgn),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Shell",          SbxLONG,      2 | FUNCTION_, RTLNAME(Shell),0           },
  { "Commandstring",SbxSTRING, 0,nullptr,0 },
  { "WindowStyle",  SbxINTEGER,       OPT_, nullptr,0 },
{ "Sin",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Sin),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "SLN",            SbxDOUBLE,    2 |  FUNCTION_ | COMPATONLY_, RTLNAME(SLN),0             },
  { "Cost",       SbxDOUBLE, 0,nullptr,0 },
  { "Double",       SbxDOUBLE, 0,nullptr,0 },
  { "Life",       SbxDOUBLE, 0,nullptr,0 },
{ "SYD",            SbxDOUBLE,    2 |  FUNCTION_ | COMPATONLY_, RTLNAME(SYD),0             },
  { "Cost",       SbxDOUBLE, 0,nullptr,0 },
  { "Salvage",       SbxDOUBLE, 0,nullptr,0 },
  { "Life",       SbxDOUBLE, 0,nullptr,0 },
  { "Period",       SbxDOUBLE, 0,nullptr,0 },
{ "Space",          SbxSTRING,      1 | FUNCTION_, RTLNAME(Space),0         },
  { "string",       SbxLONG, 0,nullptr,0 },
{ "Spc",            SbxSTRING,      1 | FUNCTION_, RTLNAME(Spc),0           },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Split",          SbxOBJECT,      3 | FUNCTION_, RTLNAME(Split),0         },
  { "expression",   SbxSTRING, 0,nullptr,0 },
  { "delimiter",    SbxSTRING, 0,nullptr,0 },
  { "count",        SbxLONG, 0,nullptr,0 },
{ "Sqr",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Sqr),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Str",            SbxSTRING,    1 | FUNCTION_, RTLNAME(Str),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "StrComp",        SbxINTEGER,   3 | FUNCTION_, RTLNAME(StrComp),0         },
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "StrConv",        SbxOBJECT,   3 | FUNCTION_, RTLNAME(StrConv),0          },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Conversion",   SbxSTRING, 0,nullptr,0 },
  { "LCID",         SbxINTEGER, OPT_,nullptr,0 },
{ "String",         SbxSTRING,    2 | FUNCTION_, RTLNAME(String),0          },
  { "Count",        SbxLONG, 0,nullptr,0 },
  { "Filler",       SbxVARIANT, 0,nullptr,0 },
{ "StrReverse",     SbxSTRING,    1 | FUNCTION_ | COMPATONLY_, RTLNAME(StrReverse),0 },
  { "String1",      SbxSTRING, 0,nullptr,0 },
{ "Switch",         SbxVARIANT,   2 | FUNCTION_, RTLNAME(Switch),0          },
  { "Expression",   SbxVARIANT, 0,nullptr,0 },
  { "Value",        SbxVARIANT, 0,nullptr,0 },
{ "Tab",            SbxSTRING,    1 | FUNCTION_, RTLNAME(Tab),0             },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Tan",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Tan),0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Time",           SbxVARIANT,       LFUNCTION_,RTLNAME(Time),0            },
{ "Timer",          SbxDATE,          FUNCTION_, RTLNAME(Timer),0           },
{ "TimeSerial",     SbxDATE,      3 | FUNCTION_, RTLNAME(TimeSerial),0      },
  { "Hour",         SbxLONG, 0,nullptr,0 },
  { "Minute",       SbxLONG, 0,nullptr,0 },
  { "Second",       SbxLONG, 0,nullptr,0 },
{ "TimeValue",      SbxDATE,      1 | FUNCTION_, RTLNAME(TimeValue),0       },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "TOGGLE",         SbxINTEGER,       CPROP_,    RTLNAME(TOGGLE),0          },
{ "Trim",           SbxSTRING,    1 | FUNCTION_, RTLNAME(Trim),0            },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "True",           SbxBOOL,          CPROP_,    RTLNAME(True),0            },
{ "TwipsPerPixelX", SbxLONG,          FUNCTION_, RTLNAME(TwipsPerPixelX),0  },
{ "TwipsPerPixelY", SbxLONG,          FUNCTION_, RTLNAME(TwipsPerPixelY),0  },

{ "TYP_AUTHORFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_AUTHORFLD),0       },
{ "TYP_CHAPTERFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_CHAPTERFLD),0      },
{ "TYP_CONDTXTFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_CONDTXTFLD),0      },
{ "TYP_DATEFLD",            SbxINTEGER,       CPROP_,    RTLNAME(TYP_DATEFLD),0         },
{ "TYP_DBFLD",              SbxINTEGER,       CPROP_,    RTLNAME(TYP_DBFLD),0           },
{ "TYP_DBNAMEFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_DBNAMEFLD),0       },
{ "TYP_DBNEXTSETFLD",       SbxINTEGER,       CPROP_,    RTLNAME(TYP_DBNEXTSETFLD),0    },
{ "TYP_DBNUMSETFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_DBNUMSETFLD),0     },
{ "TYP_DBSETNUMBERFLD",     SbxINTEGER,       CPROP_,    RTLNAME(TYP_DBSETNUMBERFLD),0  },
{ "TYP_DDEFLD",             SbxINTEGER,       CPROP_,    RTLNAME(TYP_DDEFLD),0          },
{ "TYP_DOCINFOFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_DOCINFOFLD),0      },
{ "TYP_DOCSTATFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_DOCSTATFLD),0      },
{ "TYP_EXTUSERFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_EXTUSERFLD),0      },
{ "TYP_FILENAMEFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_FILENAMEFLD),0     },
{ "TYP_FIXDATEFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_FIXDATEFLD),0      },
{ "TYP_FIXTIMEFLD",         SbxINTEGER,       CPROP_,    RTLNAME(TYP_FIXTIMEFLD),0      },
{ "TYP_FORMELFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_FORMELFLD),0       },
{ "TYP_GETFLD",             SbxINTEGER,       CPROP_,    RTLNAME(TYP_GETFLD),0          },
{ "TYP_GETREFFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_GETREFFLD),0       },
{ "TYP_GETREFPAGEFLD",      SbxINTEGER,       CPROP_,    RTLNAME(TYP_GETREFPAGEFLD),0   },
{ "TYP_HIDDENPARAFLD",      SbxINTEGER,       CPROP_,    RTLNAME(TYP_HIDDENPARAFLD),0   },
{ "TYP_HIDDENTXTFLD",       SbxINTEGER,       CPROP_,    RTLNAME(TYP_HIDDENTXTFLD),0    },
{ "TYP_INPUTFLD",           SbxINTEGER,       CPROP_,    RTLNAME(TYP_INPUTFLD),0        },
{ "TYP_INTERNETFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_INTERNETFLD),0     },
{ "TYP_JUMPEDITFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_JUMPEDITFLD),0     },
{ "TYP_MACROFLD",           SbxINTEGER,       CPROP_,    RTLNAME(TYP_MACROFLD),0        },
{ "TYP_NEXTPAGEFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_NEXTPAGEFLD),0     },
{ "TYP_PAGENUMBERFLD",      SbxINTEGER,       CPROP_,    RTLNAME(TYP_PAGENUMBERFLD),0   },
{ "TYP_POSTITFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_POSTITFLD),0       },
{ "TYP_PREVPAGEFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_PREVPAGEFLD),0     },
{ "TYP_SEQFLD",             SbxINTEGER,       CPROP_,    RTLNAME(TYP_SEQFLD),0          },
{ "TYP_SETFLD",             SbxINTEGER,       CPROP_,    RTLNAME(TYP_SETFLD),0          },
{ "TYP_SETINPFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_SETINPFLD),0       },
{ "TYP_SETREFFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_SETREFFLD),0       },
{ "TYP_SETREFPAGEFLD",      SbxINTEGER,       CPROP_,    RTLNAME(TYP_SETREFPAGEFLD),0   },
{ "TYP_TEMPLNAMEFLD",           SbxINTEGER,       CPROP_,    RTLNAME(TYP_TEMPLNAMEFLD),0},
{ "TYP_TIMEFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_TIMEFLD),0             },
{ "TYP_USERFLD",        SbxINTEGER,       CPROP_,    RTLNAME(TYP_USERFLD),0             },
{ "TYP_USRINPFLD",          SbxINTEGER,       CPROP_,    RTLNAME(TYP_USRINPFLD),0       },

{ "TypeLen",        SbxINTEGER,    1 | FUNCTION_, RTLNAME(TypeLen),0        },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "TypeName",       SbxSTRING,    1 | FUNCTION_, RTLNAME(TypeName),0        },
  { "Var",          SbxVARIANT, 0,nullptr,0 },

{ "UBound",         SbxLONG,      1 | FUNCTION_, RTLNAME(UBound),0          },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "UCase",          SbxSTRING,    1 | FUNCTION_, RTLNAME(UCase),0           },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "Unload",         SbxNULL,      1 | FUNCTION_, RTLNAME(Unload),0          },
  { "Dialog",       SbxOBJECT, 0,nullptr,0 },

{ "Val",            SbxDOUBLE,    1 | FUNCTION_, RTLNAME(Val),0             },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "VarType",        SbxINTEGER,   1 | FUNCTION_, RTLNAME(VarType),0         },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "V_EMPTY",        SbxINTEGER,       CPROP_,    RTLNAME(V_EMPTY),0         },
{ "V_NULL",         SbxINTEGER,       CPROP_,    RTLNAME(V_NULL),0          },
{ "V_INTEGER",      SbxINTEGER,       CPROP_,    RTLNAME(V_INTEGER),0       },
{ "V_LONG",         SbxINTEGER,       CPROP_,    RTLNAME(V_LONG),0          },
{ "V_SINGLE",       SbxINTEGER,       CPROP_,    RTLNAME(V_SINGLE),0        },
{ "V_DOUBLE",       SbxINTEGER,       CPROP_,    RTLNAME(V_DOUBLE),0        },
{ "V_CURRENCY",     SbxINTEGER,       CPROP_,    RTLNAME(V_CURRENCY),0      },
{ "V_DATE",         SbxINTEGER,       CPROP_,    RTLNAME(V_DATE),0          },
{ "V_STRING",       SbxINTEGER,       CPROP_,    RTLNAME(V_STRING),0        },

{ "Wait",           SbxNULL,      1 | FUNCTION_, RTLNAME(Wait),0            },
  { "Milliseconds", SbxLONG, 0,nullptr,0 },
{ "FuncCaller",          SbxVARIANT,      FUNCTION_, RTLNAME(FuncCaller),0      },
//#i64882#
{ "WaitUntil",          SbxNULL,      1 | FUNCTION_, RTLNAME(WaitUntil),0      },
  { "Date", SbxDOUBLE, 0,nullptr,0 },
{ "Weekday",        SbxINTEGER,   2 | FUNCTION_, RTLNAME(Weekday),0         },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek", SbxINTEGER, OPT_, nullptr,0 },
{ "WeekdayName",    SbxSTRING,    3 | FUNCTION_ | COMPATONLY_, RTLNAME(WeekdayName),0 },
  { "Weekday",      SbxINTEGER, 0,nullptr,0 },
  { "Abbreviate",   SbxBOOL,      OPT_, nullptr,0 },
  { "Firstdayofweek", SbxINTEGER, OPT_, nullptr,0 },
{ "Year",           SbxINTEGER,   1 | FUNCTION_, RTLNAME(Year),0            },
  { "Date",         SbxDATE, 0,nullptr,0 },

{ nullptr,             SbxNULL,     -1,nullptr,0 }};  // end of the table

SbiStdObject::SbiStdObject( const OUString& r, StarBASIC* pb ) : SbxObject( r )
{
    // do we have to initialize the hashcodes?
    Methods* p = aMethods;
    if( !p->nHash )
        while( p->nArgs != -1 )
        {
            OUString aName_ = OUString::createFromAscii( p->pName );
            p->nHash = SbxVariable::MakeHashCode( aName_ );
            p += ( p->nArgs & ARGSMASK_ ) + 1;
        }

    // #i92642: Remove default properties
    Remove( "Name", SbxClassType::DontCare );
    Remove( "Parent", SbxClassType::DontCare );

    SetParent( pb );

    pStdFactory.reset( new SbStdFactory );
    SbxBase::AddFactory( pStdFactory.get() );

    Insert( new SbStdClipboard );
}

SbiStdObject::~SbiStdObject()
{
    SbxBase::RemoveFactory( pStdFactory.get() );
    pStdFactory.reset();
}

// Finding an element:
// It runs linearly through the method table here until an
// adequate method is has been found. Because of the bits in
// the nArgs-field the adequate instance of an SbxObjElement
// is created then. If the method/property hasn't been found,
// return NULL without error code, so that a whole chain of
// objects can be asked for the method/property.

SbxVariable* SbiStdObject::Find( const OUString& rName, SbxClassType t )
{
    // entered already?
    SbxVariable* pVar = SbxObject::Find( rName, t );
    if( !pVar )
    {
        // else search one
        sal_uInt16 nHash_ = SbxVariable::MakeHashCode( rName );
        Methods* p = aMethods;
        bool bFound = false;
        short nIndex = 0;
        sal_uInt16 nSrchMask = TYPEMASK_;
        switch( t )
        {
            case SbxClassType::Method:   nSrchMask = METHOD_; break;
            case SbxClassType::Property: nSrchMask = PROPERTY_; break;
            case SbxClassType::Object:   nSrchMask = OBJECT_; break;
            default: break;
        }
        while( p->nArgs != -1 )
        {
            if( ( p->nArgs & nSrchMask )
             && ( p->nHash == nHash_ )
             && ( rName.equalsIgnoreAsciiCaseAscii( p->pName ) ) )
            {
                bFound = true;
                if( p->nArgs & COMPTMASK_ )
                {
                    bool bCompatibility = false;
                    SbiInstance* pInst = GetSbData()->pInst;
                    if (pInst)
                    {
                        bCompatibility = pInst->IsCompatibility();
                    }
                    else
                    {
                        // No instance running => compiling a source on module level.
                        const SbModule* pModule = GetSbData()->pCompMod;
                        if (pModule)
                            bCompatibility = pModule->IsVBACompat();
                    }
                    if ((bCompatibility && (NORMONLY_ & p->nArgs)) || (!bCompatibility && (COMPATONLY_ & p->nArgs)))
                        bFound = false;
                }
                break;
            }
            nIndex += ( p->nArgs & ARGSMASK_ ) + 1;
            p = aMethods + nIndex;
        }

        if( bFound )
        {
            // isolate Args-fields:
            SbxFlagBits nAccess = static_cast<SbxFlagBits>(( p->nArgs & RWMASK_ ) >> 8);
            short nType   = ( p->nArgs & TYPEMASK_ );
            if( p->nArgs & CONST_ )
                nAccess |= SbxFlagBits::Const;
            OUString aName_ = OUString::createFromAscii( p->pName );
            SbxClassType eCT = SbxClassType::Object;
            if( nType & PROPERTY_ )
            {
                eCT = SbxClassType::Property;
            }
            else if( nType & METHOD_ )
            {
                eCT = SbxClassType::Method;
            }
            pVar = Make( aName_, eCT, p->eType, ( p->nArgs & FUNCTION_ ) == FUNCTION_ );
            pVar->SetUserData( nIndex + 1 );
            pVar->SetFlags( nAccess );
        }
    }
    return pVar;
}

// SetModified must be pinched off at the RTL
void SbiStdObject::SetModified( bool )
{
}


void SbiStdObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )

{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( !pHint )
        return;

    SbxVariable* pVar = pHint->GetVar();
    SbxArray* pPar_ = pVar->GetParameters();
    const sal_uInt16 nCallId = static_cast<sal_uInt16>(pVar->GetUserData());
    if( nCallId )
    {
        const SfxHintId t = pHint->GetId();
        if( t == SfxHintId::BasicInfoWanted )
            pVar->SetInfo( GetInfo( static_cast<short>(pVar->GetUserData()) ) );
        else
        {
            bool bWrite = false;
            if( t == SfxHintId::BasicDataChanged )
                bWrite = true;
            if( t == SfxHintId::BasicDataWanted || bWrite )
            {
                RtlCall p = aMethods[ nCallId-1 ].pFunc;
                SbxArrayRef rPar( pPar_ );
                if( !pPar_ )
                {
                    rPar = pPar_ = new SbxArray;
                    pPar_->Put32( pVar, 0 );
                }
                p( static_cast<StarBASIC*>(GetParent()), *pPar_, bWrite );
                return;
            }
        }
    }
    SbxObject::Notify( rBC, rHint );
}

// building the info-structure for single elements
// if nIdx = 0, don't create anything (Std-Props!)

SbxInfo* SbiStdObject::GetInfo( short nIdx )
{
    if( !nIdx )
        return nullptr;
    Methods* p = &aMethods[ --nIdx ];
    SbxInfo* pInfo_ = new SbxInfo;
    short nPar = p->nArgs & ARGSMASK_;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        OUString aName_ = OUString::createFromAscii( p->pName );
        SbxFlagBits nFlags_ = static_cast<SbxFlagBits>(( p->nArgs >> 8 ) & 0x03);
        if( p->nArgs & OPT_ )
        {
            nFlags_ |= SbxFlagBits::Optional;
        }
        pInfo_->AddParam( aName_, p->eType, nFlags_ );
    }
    return pInfo_;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
