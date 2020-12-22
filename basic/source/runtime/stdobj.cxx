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

{ "Abs",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Abs,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Array",          SbxOBJECT,        FUNCTION_, SbRtl_Array,0           },
{ "Asc",            SbxLONG,      1 | FUNCTION_, SbRtl_Asc,0             },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "AscW",           SbxLONG,      1 | FUNCTION_ | COMPATONLY_, SbRtl_Asc,0},
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Atn",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Atn,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "ATTR_ARCHIVE",   SbxINTEGER,       CPROP_,    SbRtl_ATTR_ARCHIVE,0    },
{ "ATTR_DIRECTORY", SbxINTEGER,       CPROP_,    SbRtl_ATTR_DIRECTORY,0  },
{ "ATTR_HIDDEN",    SbxINTEGER,       CPROP_,    SbRtl_ATTR_HIDDEN,0     },
{ "ATTR_NORMAL",    SbxINTEGER,       CPROP_,    SbRtl_ATTR_NORMAL,0     },
{ "ATTR_READONLY",  SbxINTEGER,       CPROP_,    SbRtl_ATTR_READONLY,0   },
{ "ATTR_SYSTEM",    SbxINTEGER,       CPROP_,    SbRtl_ATTR_SYSTEM,0     },
{ "ATTR_VOLUME",    SbxINTEGER,       CPROP_,    SbRtl_ATTR_VOLUME,0     },

{ "Beep",           SbxNULL,          FUNCTION_, SbRtl_Beep,0            },
{ "Blue",        SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, SbRtl_Blue,0               },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },

{ "CallByName",     SbxVARIANT,   3 | FUNCTION_, SbRtl_CallByName,0 },
  { "Object",       SbxOBJECT,  0,nullptr,0 },
  { "ProcedureName",SbxSTRING,  0,nullptr,0 },
  { "CallType",     SbxINTEGER, 0,nullptr,0 },
{ "CBool",          SbxBOOL,      1 | FUNCTION_, SbRtl_CBool,0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CByte",          SbxBYTE,      1 | FUNCTION_, SbRtl_CByte,0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CCur",           SbxCURRENCY,  1 | FUNCTION_, SbRtl_CCur,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDate",          SbxDATE,      1 | FUNCTION_, SbRtl_CDate,0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDateFromUnoDate",   SbxDATE,      1 | FUNCTION_, SbRtl_CDateFromUnoDate,0    },
  { "UnoDate",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoDate", SbxOBJECT,    1 | FUNCTION_, SbRtl_CDateToUnoDate,0      },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "CDateFromUnoTime",   SbxDATE,      1 | FUNCTION_, SbRtl_CDateFromUnoTime,0    },
  { "UnoTime",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoTime", SbxOBJECT,    1 | FUNCTION_, SbRtl_CDateToUnoTime,0      },
  { "Time",         SbxDATE, 0,nullptr,0 },
{ "CDateFromUnoDateTime",   SbxDATE,      1 | FUNCTION_, SbRtl_CDateFromUnoDateTime,0    },
  { "UnoDateTime",      SbxOBJECT, 0,nullptr,0 },
{ "CDateToUnoDateTime", SbxOBJECT,    1 | FUNCTION_, SbRtl_CDateToUnoDateTime,0      },
  { "DateTime",         SbxDATE, 0,nullptr,0 },
{ "CDateFromIso",   SbxDATE,      1 | FUNCTION_, SbRtl_CDateFromIso,0    },
  { "IsoDate",      SbxSTRING, 0,nullptr,0 },
{ "CDateToIso",     SbxSTRING,    1 | FUNCTION_, SbRtl_CDateToIso,0      },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "CDec",           SbxDECIMAL,   1 | FUNCTION_, SbRtl_CDec,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CDbl",           SbxDOUBLE,    1 | FUNCTION_, SbRtl_CDbl,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CF_BITMAP",      SbxINTEGER,       CPROP_,    SbRtl_CF_BITMAP,0       },
{ "CF_METAFILEPICT",SbxINTEGER,       CPROP_,    SbRtl_CF_METAFILEPICT,0 },
{ "CF_TEXT",        SbxINTEGER,       CPROP_,    SbRtl_CF_TEXT,0         },
{ "ChDir",          SbxNULL,      1 | FUNCTION_, SbRtl_ChDir,0           },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "ChDrive",        SbxNULL,      1 | FUNCTION_, SbRtl_ChDrive,0         },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "Choose",         SbxVARIANT,   2 | FUNCTION_, SbRtl_Choose,0          },
  { "Index",        SbxINTEGER, 0,nullptr,0 },
  { "Expression",   SbxVARIANT, 0,nullptr,0 },

{ "Chr",            SbxSTRING,    1 | FUNCTION_, SbRtl_Chr,0             },
  { "string",       SbxINTEGER, 0,nullptr,0 },
{ "ChrW",           SbxSTRING,    1 | FUNCTION_ | COMPATONLY_, SbRtl_ChrW,0},
  { "string",       SbxINTEGER, 0,nullptr,0 },

{ "CInt",           SbxINTEGER,   1 | FUNCTION_, SbRtl_CInt,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CLEAR_ALLTABS",  SbxINTEGER,       CPROP_,    SbRtl_CLEAR_ALLTABS,0   },
{ "CLEAR_TAB",      SbxINTEGER,       CPROP_,    SbRtl_CLEAR_TAB,0       },
{ "CLng",           SbxLONG,      1 | FUNCTION_, SbRtl_CLng,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CompatibilityMode",  SbxBOOL,  1 | FUNCTION_, SbRtl_CompatibilityMode,0},
  { "bEnable",      SbxBOOL, 0,nullptr,0 },
{ "ConvertFromUrl", SbxSTRING,    1 | FUNCTION_, SbRtl_ConvertFromUrl,0  },
  { "Url",          SbxSTRING, 0,nullptr,0 },
{ "ConvertToUrl",   SbxSTRING,    1 | FUNCTION_, SbRtl_ConvertToUrl,0    },
  { "SystemPath",   SbxSTRING, 0,nullptr,0 },
{ "Cos",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Cos,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "CreateObject",   SbxOBJECT,    1 | FUNCTION_, SbRtl_CreateObject,0  },
  { "class",        SbxSTRING, 0,nullptr,0 },
{ "CreateUnoListener",SbxOBJECT,   1 | FUNCTION_, SbRtl_CreateUnoListener,0 },
  { "prefix",  SbxSTRING, 0,nullptr,0 },
  { "typename",  SbxSTRING, 0,nullptr,0 },
{ "CreateUnoDialog",SbxOBJECT,    2 | FUNCTION_, SbRtl_CreateUnoDialog,0 },
  { "dialoglibrary",SbxOBJECT, 0,nullptr,0 },
  { "dialogname",   SbxSTRING, 0,nullptr,0 },
{ "CreateUnoService",SbxOBJECT,   1 | FUNCTION_, SbRtl_CreateUnoService,0 },
  { "servicename",  SbxSTRING, 0,nullptr,0 },
{ "CreateUnoServiceWithArguments",SbxOBJECT,   2 | FUNCTION_, SbRtl_CreateUnoServiceWithArguments,0 },
  { "servicename",  SbxSTRING, 0,nullptr,0 },
  { "arguments",  SbxARRAY, 0,nullptr,0 },
{ "CreateUnoStruct",SbxOBJECT,    1 | FUNCTION_, SbRtl_CreateUnoStruct,0 },
  { "classname",    SbxSTRING, 0,nullptr,0 },
{ "CreateUnoValue", SbxOBJECT,    2 | FUNCTION_, SbRtl_CreateUnoValue,0 },
  { "type",         SbxSTRING, 0,nullptr,0 },
  { "value",        SbxVARIANT, 0,nullptr,0 },
{ "CreatePropertySet",SbxOBJECT, 1 | FUNCTION_, SbRtl_CreatePropertySet,0 },
  { "values",  SbxARRAY, 0,nullptr,0 },
{ "CSng",           SbxSINGLE,    1 | FUNCTION_, SbRtl_CSng,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CStr",           SbxSTRING,    1 | FUNCTION_, SbRtl_CStr,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CurDir",         SbxSTRING,    1 | FUNCTION_, SbRtl_CurDir,0          },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "CVar",           SbxVARIANT,   1 | FUNCTION_, SbRtl_CVar,0            },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "CVErr",          SbxVARIANT,   1 | FUNCTION_, SbRtl_CVErr,0           },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
{ "DDB",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, SbRtl_DDB,0       },
  { "Cost",       SbxDOUBLE,  0, nullptr,0 },
  { "Salvage",       SbxDOUBLE,  0, nullptr,0 },
  { "Life",       SbxDOUBLE,  0, nullptr,0 },
  { "Period",       SbxDOUBLE,  0, nullptr,0 },
  { "Factor",     SbxVARIANT,  OPT_, nullptr,0 },
{ "Date",           SbxDATE,          LFUNCTION_,SbRtl_Date,0            },
{ "DateAdd",        SbxDATE,      3 | FUNCTION_, SbRtl_DateAdd,0         },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Number",       SbxLONG, 0,nullptr,0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "DateDiff",       SbxDOUBLE,    5 | FUNCTION_, SbRtl_DateDiff,0        },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Date1",        SbxDATE, 0,nullptr,0 },
  { "Date2",        SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek" , SbxINTEGER, OPT_,nullptr,0 },
  { "Firstweekofyear", SbxINTEGER, OPT_,nullptr,0 },
{ "DatePart",       SbxLONG,      4 | FUNCTION_, SbRtl_DatePart,0        },
  { "Interval",     SbxSTRING, 0,nullptr,0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek" , SbxINTEGER, OPT_, nullptr,0 },
  { "Firstweekofyear", SbxINTEGER, OPT_, nullptr,0 },
{ "DateSerial",     SbxDATE,      3 | FUNCTION_, SbRtl_DateSerial,0      },
  { "Year",         SbxINTEGER, 0,nullptr,0 },
  { "Month",        SbxINTEGER, 0,nullptr,0 },
  { "Day",          SbxINTEGER, 0,nullptr,0 },
{ "DateValue",      SbxDATE,      1 | FUNCTION_, SbRtl_DateValue,0       },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "Day",            SbxINTEGER,   1 | FUNCTION_, SbRtl_Day,0             },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "Ddeexecute",     SbxNULL,   2 | FUNCTION_, SbRtl_DDEExecute,0         },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Command",         SbxSTRING, 0,nullptr,0 },
{ "Ddeinitiate",    SbxINTEGER,   2 | FUNCTION_, SbRtl_DDEInitiate,0     },
  { "Application",     SbxSTRING, 0,nullptr,0 },
  { "Topic",           SbxSTRING, 0,nullptr,0 },
{ "Ddepoke",        SbxNULL,   3 | FUNCTION_, SbRtl_DDEPoke,0            },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Item",            SbxSTRING, 0,nullptr,0 },
  { "Data",            SbxSTRING, 0,nullptr,0 },
{ "Dderequest",     SbxSTRING,   2 | FUNCTION_, SbRtl_DDERequest,0       },
  { "Channel",         SbxLONG, 0,nullptr,0 },
  { "Item",            SbxSTRING, 0,nullptr,0 },
{ "Ddeterminate",   SbxNULL,   1 | FUNCTION_, SbRtl_DDETerminate,0       },
  { "Channel",         SbxLONG, 0,nullptr,0 },
{ "Ddeterminateall",   SbxNULL,   FUNCTION_, SbRtl_DDETerminateAll,0     },
{ "DimArray",       SbxOBJECT,    FUNCTION_, SbRtl_DimArray,0            },
{ "Dir",            SbxSTRING,    2 | FUNCTION_, SbRtl_Dir,0             },
  { "FileSpec",     SbxSTRING,        OPT_, nullptr,0 },
  { "attrmask",     SbxINTEGER,       OPT_, nullptr,0 },
{ "DoEvents",       SbxINTEGER,     FUNCTION_, SbRtl_DoEvents,0          },
{ "DumpAllObjects", SbxEMPTY,     2 | SUB_, SbRtl_DumpAllObjects,0       },
  { "FileSpec",     SbxSTRING, 0,nullptr,0 },
  { "DumpAll",      SbxINTEGER,       OPT_, nullptr,0 },

{ "Empty",        SbxVARIANT,        CPROP_,    SbRtl_Empty,0         },
{ "EqualUnoObjects",SbxBOOL,      2 | FUNCTION_, SbRtl_EqualUnoObjects,0 },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "EnableReschedule",   SbxNULL,  1 | FUNCTION_, SbRtl_EnableReschedule,0},
  { "bEnable",      SbxBOOL, 0,nullptr,0 },
{ "Environ",            SbxSTRING,   1 | FUNCTION_, SbRtl_Environ,0      },
  { "Environmentstring",SbxSTRING, 0,nullptr,0 },
{ "EOF",            SbxBOOL,      1 | FUNCTION_, SbRtl_EOF,0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Erl",            SbxLONG,          ROPROP_,   SbRtl_Erl,0           },
{ "Err",            SbxVARIANT,       RWPROP_,   SbRtl_Err,0           },
{ "Error",          SbxSTRING,    1 | FUNCTION_, SbRtl_Error,0         },
  { "code",         SbxLONG, 0,nullptr,0 },
{ "Exp",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Exp,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },

{ "False",          SbxBOOL,          CPROP_,    SbRtl_False,0           },
{ "FileAttr",       SbxINTEGER,   2 | FUNCTION_, SbRtl_FileAttr,0        },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
  { "Attributes",   SbxINTEGER, 0,nullptr,0 },
{ "FileCopy",       SbxNULL,      2 | FUNCTION_, SbRtl_FileCopy,0        },
  { "Source",       SbxSTRING, 0,nullptr,0 },
  { "Destination",  SbxSTRING, 0,nullptr,0 },
{ "FileDateTime",   SbxSTRING,    1 | FUNCTION_, SbRtl_FileDateTime,0    },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FileExists",     SbxBOOL,   1 | FUNCTION_, SbRtl_FileExists,0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FileLen",        SbxLONG,      1 | FUNCTION_, SbRtl_FileLen,0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "FindObject",     SbxOBJECT,    1 | FUNCTION_, SbRtl_FindObject,0      },
  { "Name",  SbxSTRING, 0,nullptr,0 },
{ "FindPropertyObject",   SbxOBJECT,      2 | FUNCTION_, SbRtl_FindPropertyObject,0 },
  { "Object",       SbxOBJECT, 0,nullptr,0 },
  { "Name",  SbxSTRING, 0,nullptr,0 },
{ "Fix",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Fix,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Format",         SbxSTRING,    2 | FUNCTION_, SbRtl_Format,0          },
  { "expression",   SbxVARIANT, 0,nullptr,0 },
  { "format",       SbxSTRING,        OPT_, nullptr,0 },
{ "FormatDateTime", SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, SbRtl_FormatDateTime,0 },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "NamedFormat",  SbxINTEGER,        OPT_, nullptr,0 },
{ "FormatNumber",   SbxSTRING, 5 | FUNCTION_ | COMPATONLY_, SbRtl_FormatNumber, 0 },
  { "expression",                  SbxDOUBLE,  0,    nullptr, 0 },
  { "numDigitsAfterDecimal",       SbxINTEGER, OPT_, nullptr, 0 },
  { "includeLeadingDigit",         SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
  { "useParensForNegativeNumbers", SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
  { "groupDigits",                 SbxINTEGER, OPT_, nullptr, 0 }, // vbTriState
{ "Frac",           SbxDOUBLE,    1 | FUNCTION_, SbRtl_Frac,0            },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "FRAMEANCHORCHAR",        SbxINTEGER,       CPROP_,    SbRtl_FRAMEANCHORCHAR,0 },
{ "FRAMEANCHORPAGE",        SbxINTEGER,       CPROP_,    SbRtl_FRAMEANCHORPAGE,0 },
{ "FRAMEANCHORPARA",        SbxINTEGER,       CPROP_,    SbRtl_FRAMEANCHORPARA,0 },
{ "FreeFile",       SbxINTEGER,    FUNCTION_, SbRtl_FreeFile,0           },
{ "FreeLibrary",    SbxNULL,      1 | FUNCTION_, SbRtl_FreeLibrary,0     },
  { "Modulename",   SbxSTRING, 0,nullptr,0 },

{ "FV",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, SbRtl_FV,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "Get",            SbxNULL,   3 | FUNCTION_, SbRtl_Get,0                },
  { "filenumber",   SbxINTEGER, 0,nullptr,0 },
  { "recordnumber", SbxLONG, 0,nullptr,0 },
  { "variablename", SbxVARIANT, 0,nullptr,0 },
{ "GetAttr",        SbxINTEGER,   1 | FUNCTION_, SbRtl_GetAttr,0         },
  { "filename",     SbxSTRING, 0,nullptr,0 },
{ "GetDefaultContext", SbxOBJECT, 0 | FUNCTION_, SbRtl_GetDefaultContext,0 },
{ "GetDialogZoomFactorX",  SbxDOUBLE,     FUNCTION_,SbRtl_GetDialogZoomFactorX,0 },
{ "GetDialogZoomFactorY",  SbxDOUBLE,     FUNCTION_,SbRtl_GetDialogZoomFactorY,0 },
{ "GetGUIType",  SbxINTEGER,     FUNCTION_,SbRtl_GetGUIType,0            },
{ "GetGUIVersion",  SbxLONG,     FUNCTION_,SbRtl_GetGUIVersion,0         },
{ "GetPathSeparator",  SbxSTRING,     FUNCTION_,SbRtl_GetPathSeparator,0 },
{ "GetProcessServiceManager", SbxOBJECT, 0 | FUNCTION_, SbRtl_GetProcessServiceManager,0 },
{ "GetSolarVersion",  SbxLONG,     FUNCTION_,SbRtl_GetSolarVersion,0     },
{ "GetSystemTicks",  SbxLONG,      FUNCTION_,SbRtl_GetSystemTicks,0      },
{ "GetSystemType",  SbxINTEGER,    FUNCTION_,SbRtl_GetSystemType,0       },
{ "GlobalScope",    SbxOBJECT,     FUNCTION_,SbRtl_GlobalScope,0         },
{ "Green",          SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, SbRtl_Green,0           },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },

{ "HasUnoInterfaces",   SbxBOOL,  1 | FUNCTION_, SbRtl_HasUnoInterfaces,0},
  { "InterfaceName",SbxSTRING, 0,nullptr,0 },
{ "Hex",            SbxSTRING,    1 | FUNCTION_, SbRtl_Hex,0             },
  { "number",       SbxLONG, 0,nullptr,0 },
{ "Hour",           SbxINTEGER,   1 | FUNCTION_, SbRtl_Hour,0            },
  { "Date",         SbxDATE, 0,nullptr,0 },

{ "IDABORT",        SbxINTEGER,       CPROP_,    SbRtl_IDABORT,0         },
{ "IDCANCEL",       SbxINTEGER,       CPROP_,    SbRtl_IDCANCEL,0        },
{ "IDNO",           SbxINTEGER,       CPROP_,    SbRtl_IDNO,0            },
{ "IDOK",           SbxINTEGER,       CPROP_,    SbRtl_IDOK,0            },
{ "IDRETRY",        SbxINTEGER,       CPROP_,    SbRtl_IDRETRY,0         },
{ "IDYES",          SbxINTEGER,       CPROP_,    SbRtl_IDYES,0           },

{ "Iif",            SbxVARIANT,   3 | FUNCTION_, SbRtl_Iif,0             },
  { "Bool",         SbxBOOL, 0,nullptr,0 },
  { "Variant1",     SbxVARIANT, 0,nullptr,0 },
  { "Variant2",     SbxVARIANT, 0,nullptr,0 },

{ "Input",          SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, SbRtl_Input,0},
  { "Number",       SbxLONG, 0,nullptr,0 },
  { "FileNumber",   SbxLONG, 0,nullptr,0 },
{ "InputBox",       SbxSTRING,    5 | FUNCTION_, SbRtl_InputBox,0        },
  { "Prompt",       SbxSTRING, 0,nullptr,0 },
  { "Title",        SbxSTRING,        OPT_, nullptr,0 },
  { "Default",      SbxSTRING,        OPT_, nullptr,0 },
  { "XPosTwips",    SbxLONG,          OPT_, nullptr,0 },
  { "YPosTwips",    SbxLONG,          OPT_, nullptr,0 },
{ "InStr",          SbxLONG,      4 | FUNCTION_, SbRtl_InStr,0           },
  { "Start",        SbxSTRING,        OPT_, nullptr,0 },
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "InStrRev",       SbxLONG,      4 | FUNCTION_ | COMPATONLY_, SbRtl_InStrRev,0},
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Start",        SbxSTRING,        OPT_, nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "Int",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Int,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "IPmt",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, SbRtl_IPmt,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Per",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "IRR",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, SbRtl_IRR,0       },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
  { "Guess",       SbxVARIANT,  OPT_, nullptr,0 },
{ "IsArray",        SbxBOOL,      1 | FUNCTION_, SbRtl_IsArray,0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsDate",         SbxBOOL,      1 | FUNCTION_, SbRtl_IsDate,0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsEmpty",        SbxBOOL,      1 | FUNCTION_, SbRtl_IsEmpty,0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsError",        SbxBOOL,      1 | FUNCTION_, SbRtl_IsError,0         },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsMissing",        SbxBOOL,      1 | FUNCTION_, SbRtl_IsMissing,0     },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsNull",         SbxBOOL,      1 | FUNCTION_, SbRtl_IsNull,0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsNumeric",      SbxBOOL,      1 | FUNCTION_, SbRtl_IsNumeric,0       },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsObject",       SbxBOOL,      1 | FUNCTION_, SbRtl_IsObject,0        },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "IsUnoStruct",    SbxBOOL,      1 | FUNCTION_, SbRtl_IsUnoStruct,0     },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "Join",           SbxSTRING,      2 | FUNCTION_, SbRtl_Join,0          },
  { "list",         SbxOBJECT, 0,nullptr,0 },
  { "delimiter",    SbxSTRING, 0,nullptr,0 },
{ "Kill",           SbxNULL,      1 | FUNCTION_, SbRtl_Kill,0            },
  { "filespec",     SbxSTRING, 0,nullptr,0 },
{ "LBound",         SbxLONG,      1 | FUNCTION_, SbRtl_LBound,0          },
  { "Variant",      SbxVARIANT, 0,nullptr,0 },
{ "LCase",          SbxSTRING,    1 | FUNCTION_, SbRtl_LCase,0           },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Left",           SbxSTRING,    2 | FUNCTION_, SbRtl_Left,0            },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Len",            SbxLONG,      1 | FUNCTION_, SbRtl_Len,0             },
  { "StringOrVariant", SbxVARIANT, 0,nullptr,0 },
{ "LenB",           SbxLONG,      1 | FUNCTION_, SbRtl_Len,0             },
  { "StringOrVariant", SbxVARIANT, 0,nullptr,0 },
{ "Load",           SbxNULL,      1 | FUNCTION_, SbRtl_Load,0            },
  { "object",       SbxOBJECT, 0,nullptr,0 },
{ "LoadPicture",    SbxOBJECT,    1 | FUNCTION_, SbRtl_LoadPicture,0     },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Loc",            SbxLONG,      1 | FUNCTION_, SbRtl_Loc,0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Lof",            SbxLONG,      1 | FUNCTION_, SbRtl_Lof,0             },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "Log",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Log,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "LTrim",          SbxSTRING,    1 | FUNCTION_, SbRtl_LTrim,0           },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "MB_ABORTRETRYIGNORE", SbxINTEGER,  CPROP_,    SbRtl_MB_ABORTRETRYIGNORE,0},
{ "MB_APPLMODAL",   SbxINTEGER,       CPROP_,    SbRtl_MB_APPLMODAL,0    },
{ "MB_DEFBUTTON1",  SbxINTEGER,       CPROP_,    SbRtl_MB_DEFBUTTON1,0   },
{ "MB_DEFBUTTON2",  SbxINTEGER,       CPROP_,    SbRtl_MB_DEFBUTTON2,0   },
{ "MB_DEFBUTTON3",  SbxINTEGER,       CPROP_,    SbRtl_MB_DEFBUTTON3,0   },
{ "MB_ICONEXCLAMATION", SbxINTEGER,   CPROP_,    SbRtl_MB_ICONEXCLAMATION,0},
{ "MB_ICONINFORMATION", SbxINTEGER,   CPROP_,    SbRtl_MB_ICONINFORMATION,0},
{ "MB_ICONQUESTION",SbxINTEGER,       CPROP_,    SbRtl_MB_ICONQUESTION,0 },
{ "MB_ICONSTOP",    SbxINTEGER,       CPROP_,    SbRtl_MB_ICONSTOP,0     },
{ "MB_OK",          SbxINTEGER,       CPROP_,    SbRtl_MB_OK,0           },
{ "MB_OKCANCEL",    SbxINTEGER,       CPROP_,    SbRtl_MB_OKCANCEL,0     },
{ "MB_RETRYCANCEL", SbxINTEGER,       CPROP_,    SbRtl_MB_RETRYCANCEL,0  },
{ "MB_SYSTEMMODAL", SbxINTEGER,       CPROP_,    SbRtl_MB_SYSTEMMODAL,0  },
{ "MB_YESNO",       SbxINTEGER,       CPROP_,    SbRtl_MB_YESNO,0        },
{ "MB_YESNOCANCEL", SbxINTEGER,       CPROP_,    SbRtl_MB_YESNOCANCEL,0  },

{ "Me",             SbxOBJECT,    0 | FUNCTION_ | COMPATONLY_, SbRtl_Me,0 },
{ "Mid",            SbxSTRING,    3 | LFUNCTION_,SbRtl_Mid,0             },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "StartPos",     SbxLONG, 0,nullptr,0 },
  { "Length",       SbxLONG,          OPT_, nullptr,0 },
{ "Minute",         SbxINTEGER,   1 | FUNCTION_, SbRtl_Minute,0          },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "MIRR",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, SbRtl_MIRR,0       },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
  { "FinanceRate",       SbxDOUBLE,  0, nullptr,0 },
  { "ReinvestRate",       SbxDOUBLE,  0, nullptr,0 },
{ "MkDir",          SbxNULL,      1 | FUNCTION_, SbRtl_MkDir,0           },
  { "pathname",     SbxSTRING, 0,nullptr,0 },
{ "Month",          SbxINTEGER,   1 | FUNCTION_, SbRtl_Month,0           },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "MonthName",      SbxSTRING,    2 | FUNCTION_ | COMPATONLY_, SbRtl_MonthName,0 },
  { "Month",        SbxINTEGER, 0,nullptr,0 },
  { "Abbreviate",   SbxBOOL,          OPT_, nullptr,0 },
{ "MsgBox",         SbxINTEGER,    5 | FUNCTION_, SbRtl_MsgBox,0         },
  { "Prompt",       SbxSTRING, 0,nullptr,0 },
  { "Buttons",      SbxINTEGER,       OPT_, nullptr,0 },
  { "Title",        SbxSTRING,        OPT_, nullptr,0 },
  { "Helpfile",     SbxSTRING,        OPT_, nullptr,0 },
  { "Context",      SbxINTEGER,       OPT_, nullptr,0 },

{ "Nothing",        SbxOBJECT,        CPROP_,    SbRtl_Nothing,0         },
{ "Now",            SbxDATE,          FUNCTION_, SbRtl_Now,0             },
{ "NPer",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, SbRtl_NPer,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",       SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
{ "NPV",      SbxDOUBLE,      2 | FUNCTION_ | COMPATONLY_, SbRtl_NPV,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "ValueArray",       SbxARRAY,  0, nullptr,0 },
{ "Null",           SbxNULL,          CPROP_,    SbRtl_Null,0            },

{ "Oct",            SbxSTRING,    1 | FUNCTION_, SbRtl_Oct,0             },
  { "number",       SbxLONG, 0,nullptr,0 },

{ "Partition",      SbxSTRING,    4 | FUNCTION_, SbRtl_Partition,0       },
  { "number",       SbxLONG,    0,nullptr,0 },
  { "start",        SbxLONG,    0,nullptr,0 },
  { "stop",         SbxLONG,    0,nullptr,0 },
  { "interval",     SbxLONG,    0,nullptr,0 },
{ "Pi",             SbxDOUBLE,        CPROP_,    SbRtl_PI,0              },

{ "Pmt",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, SbRtl_Pmt,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "PPmt",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, SbRtl_PPmt,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "Per",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "Put",            SbxNULL,   3 | FUNCTION_, SbRtl_Put,0                },
  { "filenumber",   SbxINTEGER, 0,nullptr,0 },
  { "recordnumber", SbxLONG, 0,nullptr,0 },
  { "variablename", SbxVARIANT, 0,nullptr,0 },

{ "PV",      SbxDOUBLE,      5 | FUNCTION_ | COMPATONLY_, SbRtl_PV,0       },
  { "Rate",       SbxDOUBLE,  0, nullptr,0 },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",     SbxDOUBLE,  0, nullptr,0 },
  { "FV",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },

{ "QBColor",        SbxLONG,      1 | FUNCTION_, SbRtl_QBColor,0         },
  { "number",       SbxINTEGER, 0,nullptr,0 },

{ "Randomize",      SbxNULL,      1 | FUNCTION_, SbRtl_Randomize,0       },
  { "Number",       SbxDOUBLE,        OPT_, nullptr,0 },
{ "Rate",      SbxDOUBLE,      6 | FUNCTION_ | COMPATONLY_, SbRtl_Rate,0       },
  { "NPer",       SbxDOUBLE,  0, nullptr,0 },
  { "Pmt",       SbxDOUBLE,  0, nullptr,0 },
  { "PV",       SbxDOUBLE,  0, nullptr,0 },
  { "FV",       SbxVARIANT,  OPT_, nullptr,0 },
  { "Due",     SbxVARIANT,  OPT_, nullptr,0 },
  { "Guess",    SbxVARIANT,  OPT_, nullptr,0 },
{ "Red",        SbxINTEGER,   1 | FUNCTION_ | NORMONLY_, SbRtl_Red,0                 },
  { "RGB-Value",     SbxLONG, 0,nullptr,0 },
{ "Reset",          SbxNULL,      0 | FUNCTION_, SbRtl_Reset,0           },
{ "ResolvePath",    SbxSTRING,    1 | FUNCTION_, SbRtl_ResolvePath,0     },
  { "Path",         SbxSTRING, 0,nullptr,0 },
{ "RGB",            SbxLONG,      3 | FUNCTION_, SbRtl_RGB,0             },
  { "Red",          SbxINTEGER, 0,nullptr,0 },
  { "Green",        SbxINTEGER, 0,nullptr,0 },
  { "Blue",         SbxINTEGER, 0,nullptr,0 },
{ "Replace",        SbxSTRING,    6 | FUNCTION_, SbRtl_Replace,0         },
  { "Expression",   SbxSTRING, 0,nullptr,0 },
  { "Find",         SbxSTRING, 0,nullptr,0 },
  { "Replace",      SbxSTRING, 0,nullptr,0 },
  { "Start",        SbxINTEGER,     OPT_, nullptr,0 },
  { "Count",        SbxINTEGER,     OPT_, nullptr,0 },
  { "Compare",      SbxINTEGER,     OPT_, nullptr,0 },
{ "Right",          SbxSTRING,    2 | FUNCTION_, SbRtl_Right,0           },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "RmDir",          SbxNULL,      1 | FUNCTION_, SbRtl_RmDir,0           },
  { "pathname",     SbxSTRING, 0,nullptr,0 },
{ "Round",          SbxDOUBLE,    2 | FUNCTION_ | COMPATONLY_, SbRtl_Round,0},
  { "Expression",   SbxDOUBLE, 0,nullptr,0 },
  { "Numdecimalplaces", SbxINTEGER,   OPT_, nullptr,0 },
{ "Rnd",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Rnd,0             },
  { "Number",       SbxDOUBLE,        OPT_, nullptr,0 },
{ "RTL",            SbxOBJECT,    0 | FUNCTION_ | COMPATONLY_, SbRtl_RTL,0},
{ "RTrim",          SbxSTRING,    1 | FUNCTION_, SbRtl_RTrim,0           },
  { "string",       SbxSTRING, 0,nullptr,0 },

{ "SavePicture",    SbxNULL,      2 | FUNCTION_, SbRtl_SavePicture,0     },
  { "object",       SbxOBJECT, 0,nullptr,0 },
  { "string",       SbxSTRING, 0,nullptr,0 },
{ "Second",         SbxINTEGER,   1 | FUNCTION_, SbRtl_Second,0          },
  { "Date",         SbxDATE, 0,nullptr,0 },
{ "Seek",           SbxLONG,      1 | FUNCTION_, SbRtl_Seek,0            },
  { "Channel",      SbxINTEGER, 0,nullptr,0 },
{ "SendKeys",       SbxNULL,      2 | FUNCTION_, SbRtl_SendKeys,0        },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Wait",         SbxBOOL,          OPT_, nullptr,0 },
{ "SetAttr",        SbxNULL,      2 | FUNCTION_, SbRtl_SetAttr,0         },
  { "File"  ,       SbxSTRING, 0,nullptr,0 },
  { "Attributes",   SbxINTEGER, 0,nullptr,0 },
{ "SET_OFF",        SbxINTEGER,       CPROP_,    SbRtl_SET_OFF,0         },
{ "SET_ON",         SbxINTEGER,       CPROP_,    SbRtl_SET_ON,0          },
{ "SET_TAB",        SbxINTEGER,       CPROP_,    SbRtl_SET_TAB,0         },
{ "Sgn",            SbxINTEGER,   1 | FUNCTION_, SbRtl_Sgn,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Shell",          SbxLONG,      2 | FUNCTION_, SbRtl_Shell,0           },
  { "Commandstring",SbxSTRING, 0,nullptr,0 },
  { "WindowStyle",  SbxINTEGER,       OPT_, nullptr,0 },
{ "Sin",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Sin,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "SLN",            SbxDOUBLE,    2 |  FUNCTION_ | COMPATONLY_, SbRtl_SLN,0             },
  { "Cost",       SbxDOUBLE, 0,nullptr,0 },
  { "Double",       SbxDOUBLE, 0,nullptr,0 },
  { "Life",       SbxDOUBLE, 0,nullptr,0 },
{ "SYD",            SbxDOUBLE,    2 |  FUNCTION_ | COMPATONLY_, SbRtl_SYD,0             },
  { "Cost",       SbxDOUBLE, 0,nullptr,0 },
  { "Salvage",       SbxDOUBLE, 0,nullptr,0 },
  { "Life",       SbxDOUBLE, 0,nullptr,0 },
  { "Period",       SbxDOUBLE, 0,nullptr,0 },
{ "Space",          SbxSTRING,      1 | FUNCTION_, SbRtl_Space,0         },
  { "string",       SbxLONG, 0,nullptr,0 },
{ "Spc",            SbxSTRING,      1 | FUNCTION_, SbRtl_Spc,0           },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Split",          SbxOBJECT,      3 | FUNCTION_, SbRtl_Split,0         },
  { "expression",   SbxSTRING, 0,nullptr,0 },
  { "delimiter",    SbxSTRING, 0,nullptr,0 },
  { "count",        SbxLONG, 0,nullptr,0 },
{ "Sqr",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Sqr,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Str",            SbxSTRING,    1 | FUNCTION_, SbRtl_Str,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "StrComp",        SbxINTEGER,   3 | FUNCTION_, SbRtl_StrComp,0         },
  { "String1",      SbxSTRING, 0,nullptr,0 },
  { "String2",      SbxSTRING, 0,nullptr,0 },
  { "Compare",      SbxINTEGER,       OPT_, nullptr,0 },
{ "StrConv",        SbxOBJECT,   3 | FUNCTION_, SbRtl_StrConv,0          },
  { "String",       SbxSTRING, 0,nullptr,0 },
  { "Conversion",   SbxSTRING, 0,nullptr,0 },
  { "LCID",         SbxINTEGER, OPT_,nullptr,0 },
{ "String",         SbxSTRING,    2 | FUNCTION_, SbRtl_String,0          },
  { "Count",        SbxLONG, 0,nullptr,0 },
  { "Filler",       SbxVARIANT, 0,nullptr,0 },
{ "StrReverse",     SbxSTRING,    1 | FUNCTION_ | COMPATONLY_, SbRtl_StrReverse,0 },
  { "String1",      SbxSTRING, 0,nullptr,0 },
{ "Switch",         SbxVARIANT,   2 | FUNCTION_, SbRtl_Switch,0          },
  { "Expression",   SbxVARIANT, 0,nullptr,0 },
  { "Value",        SbxVARIANT, 0,nullptr,0 },
{ "Tab",            SbxSTRING,    1 | FUNCTION_, SbRtl_Tab,0             },
  { "Count",        SbxLONG, 0,nullptr,0 },
{ "Tan",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Tan,0             },
  { "number",       SbxDOUBLE, 0,nullptr,0 },
{ "Time",           SbxVARIANT,       LFUNCTION_,SbRtl_Time,0            },
{ "Timer",          SbxDATE,          FUNCTION_, SbRtl_Timer,0           },
{ "TimeSerial",     SbxDATE,      3 | FUNCTION_, SbRtl_TimeSerial,0      },
  { "Hour",         SbxLONG, 0,nullptr,0 },
  { "Minute",       SbxLONG, 0,nullptr,0 },
  { "Second",       SbxLONG, 0,nullptr,0 },
{ "TimeValue",      SbxDATE,      1 | FUNCTION_, SbRtl_TimeValue,0       },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "TOGGLE",         SbxINTEGER,       CPROP_,    SbRtl_TOGGLE,0          },
{ "Trim",           SbxSTRING,    1 | FUNCTION_, SbRtl_Trim,0            },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "True",           SbxBOOL,          CPROP_,    SbRtl_True,0            },
{ "TwipsPerPixelX", SbxLONG,          FUNCTION_, SbRtl_TwipsPerPixelX,0  },
{ "TwipsPerPixelY", SbxLONG,          FUNCTION_, SbRtl_TwipsPerPixelY,0  },

{ "TYP_AUTHORFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_AUTHORFLD,0       },
{ "TYP_CHAPTERFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_CHAPTERFLD,0      },
{ "TYP_CONDTXTFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_CONDTXTFLD,0      },
{ "TYP_DATEFLD",            SbxINTEGER,       CPROP_,    SbRtl_TYP_DATEFLD,0         },
{ "TYP_DBFLD",              SbxINTEGER,       CPROP_,    SbRtl_TYP_DBFLD,0           },
{ "TYP_DBNAMEFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_DBNAMEFLD,0       },
{ "TYP_DBNEXTSETFLD",       SbxINTEGER,       CPROP_,    SbRtl_TYP_DBNEXTSETFLD,0    },
{ "TYP_DBNUMSETFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_DBNUMSETFLD,0     },
{ "TYP_DBSETNUMBERFLD",     SbxINTEGER,       CPROP_,    SbRtl_TYP_DBSETNUMBERFLD,0  },
{ "TYP_DDEFLD",             SbxINTEGER,       CPROP_,    SbRtl_TYP_DDEFLD,0          },
{ "TYP_DOCINFOFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_DOCINFOFLD,0      },
{ "TYP_DOCSTATFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_DOCSTATFLD,0      },
{ "TYP_EXTUSERFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_EXTUSERFLD,0      },
{ "TYP_FILENAMEFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_FILENAMEFLD,0     },
{ "TYP_FIXDATEFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_FIXDATEFLD,0      },
{ "TYP_FIXTIMEFLD",         SbxINTEGER,       CPROP_,    SbRtl_TYP_FIXTIMEFLD,0      },
{ "TYP_FORMELFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_FORMELFLD,0       },
{ "TYP_GETFLD",             SbxINTEGER,       CPROP_,    SbRtl_TYP_GETFLD,0          },
{ "TYP_GETREFFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_GETREFFLD,0       },
{ "TYP_GETREFPAGEFLD",      SbxINTEGER,       CPROP_,    SbRtl_TYP_GETREFPAGEFLD,0   },
{ "TYP_HIDDENPARAFLD",      SbxINTEGER,       CPROP_,    SbRtl_TYP_HIDDENPARAFLD,0   },
{ "TYP_HIDDENTXTFLD",       SbxINTEGER,       CPROP_,    SbRtl_TYP_HIDDENTXTFLD,0    },
{ "TYP_INPUTFLD",           SbxINTEGER,       CPROP_,    SbRtl_TYP_INPUTFLD,0        },
{ "TYP_INTERNETFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_INTERNETFLD,0     },
{ "TYP_JUMPEDITFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_JUMPEDITFLD,0     },
{ "TYP_MACROFLD",           SbxINTEGER,       CPROP_,    SbRtl_TYP_MACROFLD,0        },
{ "TYP_NEXTPAGEFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_NEXTPAGEFLD,0     },
{ "TYP_PAGENUMBERFLD",      SbxINTEGER,       CPROP_,    SbRtl_TYP_PAGENUMBERFLD,0   },
{ "TYP_POSTITFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_POSTITFLD,0       },
{ "TYP_PREVPAGEFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_PREVPAGEFLD,0     },
{ "TYP_SEQFLD",             SbxINTEGER,       CPROP_,    SbRtl_TYP_SEQFLD,0          },
{ "TYP_SETFLD",             SbxINTEGER,       CPROP_,    SbRtl_TYP_SETFLD,0          },
{ "TYP_SETINPFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_SETINPFLD,0       },
{ "TYP_SETREFFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_SETREFFLD,0       },
{ "TYP_SETREFPAGEFLD",      SbxINTEGER,       CPROP_,    SbRtl_TYP_SETREFPAGEFLD,0   },
{ "TYP_TEMPLNAMEFLD",           SbxINTEGER,       CPROP_,    SbRtl_TYP_TEMPLNAMEFLD,0},
{ "TYP_TIMEFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_TIMEFLD,0             },
{ "TYP_USERFLD",        SbxINTEGER,       CPROP_,    SbRtl_TYP_USERFLD,0             },
{ "TYP_USRINPFLD",          SbxINTEGER,       CPROP_,    SbRtl_TYP_USRINPFLD,0       },

{ "TypeLen",        SbxINTEGER,    1 | FUNCTION_, SbRtl_TypeLen,0        },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "TypeName",       SbxSTRING,    1 | FUNCTION_, SbRtl_TypeName,0        },
  { "Var",          SbxVARIANT, 0,nullptr,0 },

{ "UBound",         SbxLONG,      1 | FUNCTION_, SbRtl_UBound,0          },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "UCase",          SbxSTRING,    1 | FUNCTION_, SbRtl_UCase,0           },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "Unload",         SbxNULL,      1 | FUNCTION_, SbRtl_Unload,0          },
  { "Dialog",       SbxOBJECT, 0,nullptr,0 },

{ "Val",            SbxDOUBLE,    1 | FUNCTION_, SbRtl_Val,0             },
  { "String",       SbxSTRING, 0,nullptr,0 },
{ "VarType",        SbxINTEGER,   1 | FUNCTION_, SbRtl_VarType,0         },
  { "Var",          SbxVARIANT, 0,nullptr,0 },
{ "V_EMPTY",        SbxINTEGER,       CPROP_,    SbRtl_V_EMPTY,0         },
{ "V_NULL",         SbxINTEGER,       CPROP_,    SbRtl_V_NULL,0          },
{ "V_INTEGER",      SbxINTEGER,       CPROP_,    SbRtl_V_INTEGER,0       },
{ "V_LONG",         SbxINTEGER,       CPROP_,    SbRtl_V_LONG,0          },
{ "V_SINGLE",       SbxINTEGER,       CPROP_,    SbRtl_V_SINGLE,0        },
{ "V_DOUBLE",       SbxINTEGER,       CPROP_,    SbRtl_V_DOUBLE,0        },
{ "V_CURRENCY",     SbxINTEGER,       CPROP_,    SbRtl_V_CURRENCY,0      },
{ "V_DATE",         SbxINTEGER,       CPROP_,    SbRtl_V_DATE,0          },
{ "V_STRING",       SbxINTEGER,       CPROP_,    SbRtl_V_STRING,0        },

{ "Wait",           SbxNULL,      1 | FUNCTION_, SbRtl_Wait,0            },
  { "Milliseconds", SbxLONG, 0,nullptr,0 },
{ "FuncCaller",          SbxVARIANT,      FUNCTION_, SbRtl_FuncCaller,0      },
//#i64882#
{ "WaitUntil",          SbxNULL,      1 | FUNCTION_, SbRtl_WaitUntil,0      },
  { "Date", SbxDOUBLE, 0,nullptr,0 },
{ "Weekday",        SbxINTEGER,   2 | FUNCTION_, SbRtl_Weekday,0         },
  { "Date",         SbxDATE, 0,nullptr,0 },
  { "Firstdayofweek", SbxINTEGER, OPT_, nullptr,0 },
{ "WeekdayName",    SbxSTRING,    3 | FUNCTION_ | COMPATONLY_, SbRtl_WeekdayName,0 },
  { "Weekday",      SbxINTEGER, 0,nullptr,0 },
  { "Abbreviate",   SbxBOOL,      OPT_, nullptr,0 },
  { "Firstdayofweek", SbxINTEGER, OPT_, nullptr,0 },
{ "Year",           SbxINTEGER,   1 | FUNCTION_, SbRtl_Year,0            },
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
