/*************************************************************************
 *
 *  $RCSfile: stdobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-03 15:51:15 $
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

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#include "runtime.hxx"
#pragma hdrstop
#include "stdobj.hxx"
#include "stdobj1.hxx"
#include "rtlproto.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

// Das nArgs-Feld eines Tabelleneintrags ist wie folgt verschluesselt:
// Zur Zeit wird davon ausgegangen, dass Properties keine Parameter
// benoetigen!

#define _ARGSMASK   0x00FF  // Bis zu 255 Argumente
#define _RWMASK     0x0F00  // Maske fuer R/W-Bits
#define _TYPEMASK   0xF000  // Maske fuer den Typ des Eintrags

#define _READ       0x0100  // kann gelesen werden
#define _BWRITE     0x0200  // kann as Lvalue verwendet werden
#define _LVALUE     _BWRITE // kann as Lvalue verwendet werden
#define _READWRITE  0x0300  // beides
#define _OPT        0x0400  // Parameter ist optional
#define _CONST      0x0800  // Property ist const
#define _METHOD     0x3000  // Masken-Bits fuer eine Methode
#define _PROPERTY   0x4000  // Masken-Bit fuer eine Property
#define _OBJECT     0x8000  // Masken-Bit fuer ein Objekt
                            // Kombination von oberen Bits:
#define _FUNCTION   0x1100  // Maske fuer Function
#define _LFUNCTION  0x1300  // Maske fuer Function, die auch als Lvalue geht
#define _SUB        0x2100  // Maske fuer Sub
#define _ROPROP     0x4100  // Maske Read Only-Property
#define _WOPROP     0x4200  // Maske Write Only-Property
#define _RWPROP     0x4300  // Maske Read/Write-Property
#define _CPROP      0x4900  // Maske fuer Konstante

struct Methods {
    const char* pName;      // Name des Eintrags
    SbxDataType eType;      // Datentyp
    short       nArgs;      // Argumente und Flags
    RtlCall     pFunc;      // Function Pointer
    USHORT      nHash;      // Hashcode
};

static Methods aMethods[] = {

{ "AboutStarBasic", SbxNULL,      1 | _FUNCTION, RTLNAME(AboutStarBasic)    },
  { "Name",         SbxSTRING },
{ "Abs",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Abs)               },
  { "number",       SbxDOUBLE },
{ "Array",          SbxOBJECT,        _FUNCTION, RTLNAME(Array)             },
{ "Asc",            SbxINTEGER,   1 | _FUNCTION, RTLNAME(Asc)               },
  { "string",       SbxSTRING },
{ "Atn",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Atn)               },
  { "number",       SbxDOUBLE },
{ "ATTR_ARCHIVE",   SbxINTEGER,       _CPROP,    RTLNAME(ATTR_ARCHIVE)      },
{ "ATTR_DIRECTORY", SbxINTEGER,       _CPROP,    RTLNAME(ATTR_DIRECTORY)    },
{ "ATTR_HIDDEN",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_HIDDEN)       },
{ "ATTR_NORMAL",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_NORMAL)       },
{ "ATTR_READONLY",  SbxINTEGER,       _CPROP,    RTLNAME(ATTR_READONLY)     },
{ "ATTR_SYSTEM",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_SYSTEM)       },
{ "ATTR_VOLUME",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_VOLUME)       },
{ "Beep",           SbxNULL,          _FUNCTION, RTLNAME(Beep)              },
{ "Blue",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Blue)           },
  { "RGB-Value",     SbxLONG },

{ "CBool",          SbxBOOL,      1 | _FUNCTION, RTLNAME(CBool)             },
  { "expression",   SbxVARIANT },
{ "CByte",          SbxBYTE,      1 | _FUNCTION, RTLNAME(CByte)             },
  { "expression",   SbxVARIANT },
{ "CCur",           SbxCURRENCY,  1 | _FUNCTION, RTLNAME(CCur)              },
  { "expression",   SbxVARIANT },
{ "CDate",          SbxDATE,      1 | _FUNCTION, RTLNAME(CDate)             },
  { "expression",   SbxVARIANT },
{ "CDbl",           SbxDOUBLE,    1 | _FUNCTION, RTLNAME(CDbl)              },
  { "expression",   SbxVARIANT },
{ "CF_BITMAP",      SbxINTEGER,       _CPROP,    RTLNAME(CF_BITMAP)         },
{ "CF_METAFILEPICT",SbxINTEGER,       _CPROP,    RTLNAME(CF_METAFILEPICT)   },
{ "CF_TEXT",        SbxINTEGER,       _CPROP,    RTLNAME(CF_TEXT)           },
{ "ChDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(ChDir)             },
  { "string",       SbxSTRING  },
{ "ChDrive",        SbxNULL,      1 | _FUNCTION, RTLNAME(ChDrive)           },
  { "string",       SbxSTRING  },

{ "Choose",         SbxVARIANT,   2 | _FUNCTION, RTLNAME(Choose)            },
  { "Index",        SbxINTEGER },
  { "Expression",   SbxVARIANT },

{ "Chr",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Chr)               },
  { "string",       SbxINTEGER },

{ "CInt",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(CInt)              },
  { "expression",   SbxVARIANT },
{ "CLEAR_ALLTABS",          SbxINTEGER,       _CPROP,    RTLNAME(CLEAR_ALLTABS)         },
{ "CLEAR_TAB",          SbxINTEGER,       _CPROP,    RTLNAME(CLEAR_TAB)         },

{ "CLng",           SbxLONG,      1 | _FUNCTION, RTLNAME(CLng)              },
  { "expression",   SbxVARIANT },
{ "Cos",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Cos)               },
  { "number",       SbxDOUBLE },
{ "CreateObject",   SbxOBJECT,    1 | _FUNCTION, RTLNAME( CreateObject )    },
  { "class",        SbxSTRING },
{ "CreateUnoListener",SbxOBJECT,   1 | _FUNCTION, RTLNAME( CreateUnoListener ) },
  { "prefix",  SbxSTRING },
  { "typename",  SbxSTRING },
{ "CreateUnoService",SbxOBJECT,   1 | _FUNCTION, RTLNAME( CreateUnoService ) },
  { "servicename",  SbxSTRING },
{ "CreateUnoDialog",SbxOBJECT,    2 | _FUNCTION, RTLNAME( CreateUnoDialog ) },
  { "dialoglibrary",SbxOBJECT },
  { "dialogname",   SbxSTRING },
{ "CreateUnoStruct",SbxOBJECT,    1 | _FUNCTION, RTLNAME( CreateUnoStruct ) },
  { "classname",    SbxSTRING },
{ "CreatePropertySet",SbxOBJECT, 1 | _FUNCTION, RTLNAME( CreatePropertySet ) },
  { "values",  SbxARRAY },
{ "CSng",           SbxSINGLE,    1 | _FUNCTION, RTLNAME(CSng)              },
  { "expression",   SbxVARIANT },
{ "CStr",           SbxSTRING,    1 | _FUNCTION, RTLNAME(CStr)              },
  { "expression",   SbxVARIANT },
{ "CurDir",         SbxSTRING,    1 | _FUNCTION, RTLNAME(CurDir)            },
  { "string",       SbxSTRING },
{ "CVar",           SbxVARIANT,   1 | _FUNCTION, RTLNAME(CVar)              },
  { "expression",   SbxVARIANT },
{ "CVErr",          SbxVARIANT,   1 | _FUNCTION, RTLNAME(CVErr)             },
  { "expression",   SbxVARIANT },
{ "Date",           SbxSTRING,        _LFUNCTION,RTLNAME(Date)              },
{ "DateAdd",        SbxDATE,   1 | _FUNCTION, RTLNAME(DateAdd)               },
  { "Interval",     SbxSTRING },
  { "Number",       SbxLONG },
  { "Date",         SbxDATE },
{ "DateDiff",       SbxLONG,   1 | _FUNCTION, RTLNAME(DateDiff)             },
  { "Interval",     SbxSTRING },
  { "Date1",        SbxDATE },
  { "Date2",        SbxDATE },
{ "DatePart",       SbxLONG,   1 | _FUNCTION, RTLNAME(DatePart)             },
  { "Interval",     SbxSTRING },
  { "Date",         SbxDATE },
{ "DateSerial",     SbxDATE,      3 | _FUNCTION, RTLNAME(DateSerial)        },
  { "Year",         SbxINTEGER },
  { "Month",        SbxINTEGER },
  { "Day",          SbxINTEGER },
{ "DateValue",      SbxDATE,      1 | _FUNCTION, RTLNAME(DateValue)         },
  { "String",       SbxSTRING },
{ "Day",            SbxINTEGER,   1 | _FUNCTION, RTLNAME(Day)               },
  { "Date",         SbxDATE },

{ "Ddeexecute",     SbxNULL,   2 | _FUNCTION, RTLNAME(DDEExecute)           },
  { "Channel",         SbxLONG },
  { "Command",         SbxSTRING },
{ "Ddeinitiate",    SbxINTEGER,   2 | _FUNCTION, RTLNAME(DDEInitiate)          },
  { "Application",     SbxSTRING },
  { "Topic",           SbxSTRING },
{ "Ddepoke",        SbxNULL,   3 | _FUNCTION, RTLNAME(DDEPoke)          },
  { "Channel",         SbxLONG },
  { "Item",            SbxSTRING },
  { "Data",            SbxSTRING },
{ "Dderequest",     SbxSTRING,   2 | _FUNCTION, RTLNAME(DDERequest)           },
  { "Channel",         SbxLONG },
  { "Item",            SbxSTRING },
{ "Ddeterminate",   SbxNULL,   1 | _FUNCTION, RTLNAME(DDETerminate)           },
  { "Channel",         SbxLONG },
{ "Ddeterminateall",   SbxNULL,   _FUNCTION, RTLNAME(DDETerminateAll)       },
{ "DimArray",       SbxOBJECT,    _FUNCTION, RTLNAME(DimArray)              },
{ "Dir",            SbxSTRING,    2 | _FUNCTION, RTLNAME(Dir)               },
  { "FileSpec",     SbxSTRING,        _OPT },
  { "attrmask",     SbxINTEGER,       _OPT },
{ "DumpAllObjects", SbxEMPTY,     2 | _SUB, RTLNAME(DumpAllObjects)         },
  { "FileSpec",     SbxSTRING },
  { "DumpAll",      SbxINTEGER,       _OPT },

{ "EqualUnoObjects",SbxBOOL,      2 | _FUNCTION, RTLNAME(EqualUnoObjects)   },
  { "Variant",      SbxVARIANT },
  { "Variant",      SbxVARIANT },
{ "EnableReschedule",   SbxNULL,  1 | _FUNCTION, RTLNAME(EnableReschedule)  },
  { "bEnable",      SbxBOOL  },
{ "Environ",            SbxSTRING,   1 | _FUNCTION, RTLNAME(Environ)        },
  { "Environmentstring",SbxSTRING },
{ "EOF",            SbxBOOL,      1 | _FUNCTION, RTLNAME(EOF)               },
  { "Channel",      SbxINTEGER },
{ "Erl",            SbxLONG,          _ROPROP,   RTLNAME( Erl )             },
{ "Err",            SbxLONG,          _RWPROP,   RTLNAME( Err )             },
{ "Error",          SbxSTRING,    1 | _FUNCTION, RTLNAME( Error )           },
  { "code",         SbxLONG },
{ "Exp",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Exp)               },
  { "number",       SbxDOUBLE },
{ "False",          SbxBOOL,          _CPROP,    RTLNAME(False)             },
{ "FileAttr",       SbxINTEGER,   2 | _FUNCTION, RTLNAME(FileAttr)          },
  { "Channel",      SbxINTEGER },
  { "Attributes",   SbxINTEGER },
{ "FileCopy",       SbxNULL,      2 | _FUNCTION, RTLNAME(FileCopy)          },
  { "Source",       SbxSTRING },
  { "Destination",  SbxSTRING },
{ "FileDateTime",   SbxSTRING,    1 | _FUNCTION, RTLNAME(FileDateTime)      },
  { "filename",     SbxSTRING },
{ "FileExists",     SbxBOOL,   1 | _FUNCTION, RTLNAME(FileExists)           },
  { "filename",     SbxSTRING },
{ "FileLen",        SbxLONG,      1 | _FUNCTION, RTLNAME(FileLen)           },
  { "filename",     SbxSTRING },
{ "FindObject",     SbxOBJECT,    1 | _FUNCTION, RTLNAME(FindObject) },
  { "Name",  SbxSTRING },
{ "FindPropertyObject",   SbxOBJECT,      2 | _FUNCTION, RTLNAME(FindPropertyObject) },
  { "Object",       SbxOBJECT },
  { "Name",  SbxSTRING },
{ "Fix",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Fix)               },
  { "number",       SbxDOUBLE },
{ "Format",         SbxSTRING,    2 | _FUNCTION, RTLNAME(Format)            },
  { "expression",   SbxVARIANT },
  { "format",       SbxSTRING,        _OPT },

{ "FRAMEANCHORCHAR",        SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORCHAR)       },
{ "FRAMEANCHORPAGE",        SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORPAGE)       },
{ "FRAMEANCHORPARA",        SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORPARA)       },

{ "FreeFile",       SbxINTEGER,    _FUNCTION, RTLNAME(FreeFile)           },
{ "FreeLibrary",    SbxNULL,      1 | _FUNCTION, RTLNAME(FreeLibrary)             },
  { "Modulename",   SbxSTRING  },

{ "Get",            SbxNULL,   3 | _FUNCTION, RTLNAME(Get)               },
  { "filenumber",   SbxINTEGER },
  { "recordnumber", SbxLONG },
  { "variablename", SbxVARIANT },

{ "GetAttr",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(GetAttr)           },
  { "filename",     SbxSTRING },
{ "GetDialogZoomFactorX",  SbxDOUBLE,     _FUNCTION,RTLNAME(GetDialogZoomFactorX)      },
{ "GetDialogZoomFactorY",  SbxDOUBLE,     _FUNCTION,RTLNAME(GetDialogZoomFactorY)      },
{ "GetGUIType",  SbxINTEGER,     _FUNCTION,RTLNAME(GetGUIType)      },
{ "GetGUIVersion",  SbxLONG,     _FUNCTION,RTLNAME(GetGUIVersion)      },
{ "GetPathSeparator",  SbxSTRING,     _FUNCTION,RTLNAME(GetPathSeparator)  },
{ "GetProcessServiceManager", SbxOBJECT, 0 | _FUNCTION, RTLNAME(GetProcessServiceManager) },
{ "GetSolarVersion",  SbxLONG,     _FUNCTION,RTLNAME(GetSolarVersion)      },
{ "GetSystemTicks",  SbxLONG,     _FUNCTION,RTLNAME(GetSystemTicks)      },
{ "GetSystemType",  SbxINTEGER,     _FUNCTION,RTLNAME(GetSystemType)      },
{ "Green",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Green)           },
  { "RGB-Value",     SbxLONG },

{ "HasUnoInterfaces",   SbxBOOL,  1 | _FUNCTION, RTLNAME(HasUnoInterfaces)  },
  { "InterfaceName",SbxSTRING },
{ "Hex",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Hex)               },
  { "number",       SbxLONG },
{ "Hour",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(Hour)              },
  { "Date",         SbxDATE },

{ "IDABORT",        SbxINTEGER,       _CPROP,    RTLNAME(IDABORT)           },
{ "IDCANCEL",       SbxINTEGER,       _CPROP,    RTLNAME(IDCANCEL)          },
{ "IDNO",           SbxINTEGER,       _CPROP,    RTLNAME(IDNO)              },
{ "IDOK",           SbxINTEGER,       _CPROP,    RTLNAME(IDOK)              },
{ "IDRETRY",        SbxINTEGER,       _CPROP,    RTLNAME(IDRETRY)           },
{ "IDYES",          SbxINTEGER,       _CPROP,    RTLNAME(IDYES)             },

{ "Iif",            SbxVARIANT,   3 | _FUNCTION, RTLNAME(Iif)               },
  { "Bool",         SbxBOOL },
  { "Variant1",     SbxVARIANT },
  { "Variant2",     SbxVARIANT },

{ "InputBox",       SbxSTRING,    5 | _FUNCTION, RTLNAME(InputBox)          },
  { "Prompt",       SbxSTRING },
  { "Title",        SbxSTRING,        _OPT },
  { "Default",      SbxSTRING,        _OPT },
  { "XPosTwips",    SbxLONG,          _OPT },
  { "YPosTwips",    SbxLONG,          _OPT },
{ "InStr",          SbxINTEGER,   4 | _FUNCTION, RTLNAME(InStr)             },
  { "StartPos",     SbxSTRING,        _OPT },
  { "String1",      SbxSTRING },
  { "String2",      SbxSTRING },
  { "Compare",      SbxINTEGER,       _OPT },
{ "Int",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Int)               },
  { "number",       SbxDOUBLE },
{ "IsArray",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsArray)           },
  { "Variant",      SbxVARIANT },
{ "IsDate",         SbxBOOL,      1 | _FUNCTION, RTLNAME(IsDate)            },
  { "Variant",      SbxVARIANT },
{ "IsEmpty",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsEmpty)           },
  { "Variant",      SbxVARIANT },
{ "IsError",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsError)           },
  { "Variant",      SbxVARIANT },
{ "IsMissing",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsMissing)       },
  { "Variant",      SbxVARIANT },
{ "IsNull",         SbxBOOL,      1 | _FUNCTION, RTLNAME(IsNull)            },
  { "Variant",      SbxVARIANT },
{ "IsNumeric",      SbxBOOL,      1 | _FUNCTION, RTLNAME(IsNumeric)         },
  { "Variant",      SbxVARIANT },
{ "IsObject",       SbxBOOL,      1 | _FUNCTION, RTLNAME(IsObject)          },
  { "Variant",      SbxVARIANT },
{ "IsUnoStruct",    SbxBOOL,      1 | _FUNCTION, RTLNAME(IsUnoStruct)       },
  { "Variant",      SbxVARIANT },
{ "Kill",           SbxNULL,      1 | _FUNCTION, RTLNAME(Kill)              },
  { "filespec",     SbxSTRING },
{ "LBound",         SbxINTEGER,      1 | _FUNCTION, RTLNAME(LBound)         },
  { "Variant",      SbxVARIANT },
{ "LCase",          SbxSTRING,    1 | _FUNCTION, RTLNAME(LCase)             },
  { "string",       SbxSTRING },
{ "Left",           SbxSTRING,    2 | _FUNCTION, RTLNAME(Left)              },
  { "String",       SbxSTRING },
  { "Count",        SbxLONG },
{ "Len",            SbxLONG,      1 | _FUNCTION, RTLNAME(Len)               },
  { "StringOrVariant", SbxVARIANT },
{ "Load",           SbxNULL,      1 | _FUNCTION, RTLNAME(Load)              },
  { "object",       SbxOBJECT },
{ "LoadPicture",    SbxOBJECT,    1 | _FUNCTION, RTLNAME(LoadPicture)       },
  { "string",       SbxSTRING },
{ "Loc",            SbxLONG,      1 | _FUNCTION, RTLNAME(Loc)               },
  { "Channel",      SbxINTEGER },
{ "Lof",            SbxLONG,      1 | _FUNCTION, RTLNAME(Lof)               },
  { "Channel",      SbxINTEGER },
{ "Log",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Log)               },
  { "number",       SbxDOUBLE },
{ "LTrim",          SbxSTRING,    1 | _FUNCTION, RTLNAME(LTrim)             },
  { "string",       SbxSTRING },

{ "MB_ABORTRETRYIGNORE", SbxINTEGER,  _CPROP,    RTLNAME(MB_ABORTRETRYIGNORE)},
{ "MB_APPLMODAL",   SbxINTEGER,       _CPROP,    RTLNAME(MB_APPLMODAL)     },
{ "MB_DEFBUTTON1",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON1)     },
{ "MB_DEFBUTTON2",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON2)     },
{ "MB_DEFBUTTON3",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON3)     },
{ "MB_ICONEXCLAMATION", SbxINTEGER,   _CPROP,    RTLNAME(MB_ICONEXCLAMATION)},
{ "MB_ICONINFORMATION", SbxINTEGER,   _CPROP,    RTLNAME(MB_ICONINFORMATION)},
{ "MB_ICONQUESTION",SbxINTEGER,       _CPROP,    RTLNAME(MB_ICONQUESTION)   },
{ "MB_ICONSTOP",    SbxINTEGER,       _CPROP,    RTLNAME(MB_ICONSTOP)       },
{ "MB_OK",          SbxINTEGER,       _CPROP,    RTLNAME(MB_OK)             },
{ "MB_OKCANCEL",    SbxINTEGER,       _CPROP,    RTLNAME(MB_OKCANCEL)       },
{ "MB_RETRYCANCEL", SbxINTEGER,       _CPROP,    RTLNAME(MB_RETRYCANCEL)    },
{ "MB_SYSTEMMODAL", SbxINTEGER,       _CPROP,    RTLNAME(MB_SYSTEMMODAL)    },
{ "MB_YESNO",       SbxINTEGER,       _CPROP,    RTLNAME(MB_YESNO)          },
{ "MB_YESNOCANCEL", SbxINTEGER,       _CPROP,    RTLNAME(MB_YESNOCANCEL)    },


{ "Mid",            SbxSTRING,    3 | _LFUNCTION,RTLNAME(Mid)               },
  { "String",       SbxSTRING },
  { "StartPos",     SbxLONG } ,
  { "Length",       SbxLONG,          _OPT } ,
{ "Minute",         SbxINTEGER,   1 | _FUNCTION, RTLNAME(Minute)            },
  { "Date",         SbxDATE },
{ "MkDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(MkDir)             },
  { "pathname",     SbxSTRING },
{ "Month",          SbxINTEGER,   1 | _FUNCTION, RTLNAME(Month)             },
  { "Date",         SbxDATE },
{ "MsgBox",         SbxINTEGER,    3 | _FUNCTION, RTLNAME(MsgBox)          },
  { "Message",      SbxSTRING },
  { "Type",         SbxINTEGER,       _OPT },
  { "Title",        SbxSTRING,        _OPT },

{ "Nothing",        SbxOBJECT,        _CPROP,    RTLNAME(Nothing)           },
{ "Now",            SbxDATE,          _FUNCTION, RTLNAME(Now)               },
{ "Null",           SbxOBJECT,        _CPROP,    RTLNAME(Null)              },
{ "Oct",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Oct)               },
  { "number",       SbxLONG },
{ "Pi",             SbxDOUBLE,        _CPROP,    RTLNAME(PI)                },

{ "Put",            SbxNULL,   3 | _FUNCTION, RTLNAME(Put)               },
  { "filenumber",   SbxINTEGER },
  { "recordnumber", SbxLONG },
  { "variablename", SbxVARIANT },

{ "QBColor",        SbxLONG,      1 | _FUNCTION, RTLNAME(QBColor)           },
  { "number",       SbxINTEGER },
{ "Randomize",      SbxNULL,      1 | _FUNCTION, RTLNAME(Randomize)         },
  { "Number",       SbxDOUBLE,        _OPT },
{ "Red",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Red)           },
  { "RGB-Value",     SbxLONG },
{ "Reset",          SbxNULL,      0 | _FUNCTION, RTLNAME(Reset)             },
{ "ResolvePath",    SbxSTRING,    1 | _FUNCTION, RTLNAME(ResolvePath)       },
  { "Path",         SbxSTRING },
{ "RGB",            SbxLONG,      3 | _FUNCTION, RTLNAME(RGB)               },
  { "Red",          SbxINTEGER },
  { "Green",        SbxINTEGER },
  { "Blue",         SbxINTEGER },

{ "Right",          SbxSTRING,    2 | _FUNCTION, RTLNAME(Right)             },
  { "String",       SbxSTRING },
  { "Count",        SbxLONG } ,
{ "RmDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(RmDir)             },
  { "pathname",     SbxSTRING },
{ "Rnd",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Rnd)               },
  { "Number",       SbxDOUBLE,        _OPT },
{ "RTrim",          SbxSTRING,    1 | _FUNCTION, RTLNAME(RTrim)             },
  { "string",       SbxSTRING },
{ "SavePicture",    SbxNULL,      2 | _FUNCTION, RTLNAME(SavePicture)       },
  { "object",       SbxOBJECT },
  { "string",       SbxSTRING },
{ "Second",         SbxINTEGER,   1 | _FUNCTION, RTLNAME(Second)            },
  { "Date",         SbxDATE },
{ "Seek",           SbxLONG,      1 | _FUNCTION, RTLNAME(Seek)              },
  { "Channel",      SbxINTEGER },

{ "SendKeys",       SbxNULL,      2 | _FUNCTION, RTLNAME(SendKeys)          },
  { "String",       SbxSTRING },
  { "Wait",         SbxBOOL,          _OPT } ,
{ "SetAttr",        SbxNULL,      2 | _FUNCTION, RTLNAME(SetAttr)           },
  { "File"  ,       SbxSTRING },
  { "Attributes",   SbxINTEGER } ,
{ "SET_OFF",        SbxINTEGER,       _CPROP,    RTLNAME(SET_OFF)       },
{ "SET_ON",         SbxINTEGER,       _CPROP,    RTLNAME(SET_ON)        },
{ "SET_TAB",        SbxINTEGER,       _CPROP,    RTLNAME(SET_TAB)       },

{ "Sgn",            SbxINTEGER,   1 | _FUNCTION, RTLNAME(Sgn)               },
  { "number",       SbxDOUBLE },
{ "Shell",          SbxLONG,      2 | _FUNCTION, RTLNAME(Shell)             },
  { "Commandstring",SbxSTRING },
  { "WindowStyle",  SbxINTEGER,       _OPT },
{ "Sin",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Sin)               },
  { "number",       SbxDOUBLE },
{ "Space",          SbxSTRING,      1 | _FUNCTION, RTLNAME(Space)             },
  { "string",       SbxLONG },
{ "Spc",            SbxSTRING,      1 | _FUNCTION, RTLNAME(Spc)             },
  { "Count",        SbxLONG },
{ "Sqr",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Sqr)               },
  { "number",       SbxDOUBLE },
{ "Str",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Str)               },
  { "number",       SbxDOUBLE },
{ "StrComp",        SbxINTEGER,   3 | _FUNCTION, RTLNAME(StrComp)           },
  { "String1",      SbxSTRING },
  { "String2",      SbxSTRING },
  { "Compare",      SbxINTEGER,       _OPT },
{ "StrConv",        SbxSTRING,   2 | _FUNCTION, RTLNAME(StrConv)           },
  { "String",       SbxSTRING },
  { "Conversion",   SbxSTRING },
{ "String",         SbxSTRING,    2 | _FUNCTION, RTLNAME(String)            },
  { "Count",        SbxLONG },
  { "Filler",       SbxVARIANT },

{ "Switch",         SbxVARIANT,   2 | _FUNCTION, RTLNAME(Switch)            },
  { "Expression",   SbxVARIANT },
  { "Value",        SbxVARIANT },

{ "Tan",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Tan)               },
  { "number",       SbxDOUBLE },
{ "Time",           SbxVARIANT,       _LFUNCTION,RTLNAME(Time)              },
{ "Timer",          SbxDATE,          _FUNCTION, RTLNAME(Timer)             },
{ "TimeSerial",     SbxDATE,      3 | _FUNCTION, RTLNAME(TimeSerial)        },
  { "Hour",         SbxLONG },
  { "Minute",       SbxLONG },
  { "Second",       SbxLONG },
{ "TimeValue",      SbxDATE,      1 | _FUNCTION, RTLNAME(TimeValue)         },
  { "String",       SbxSTRING },

{ "TOGGLE",         SbxINTEGER,       _CPROP,    RTLNAME(TOGGLE)        },

{ "Trim",           SbxSTRING,    1 | _FUNCTION, RTLNAME(Trim)             },
  { "String",       SbxSTRING },
{ "True",           SbxBOOL,          _CPROP,    RTLNAME(True)              },
{ "TwipsPerPixelX", SbxLONG,          _FUNCTION, RTLNAME(TwipsPerPixelX)   },
{ "TwipsPerPixelY", SbxLONG,          _FUNCTION, RTLNAME(TwipsPerPixelY)   },

{ "TYP_AUTHORFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_AUTHORFLD)         },
{ "TYP_CHAPTERFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_CHAPTERFLD)        },
{ "TYP_CONDTXTFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_CONDTXTFLD)        },
{ "TYP_DATEFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_DATEFLD)       },
{ "TYP_DBFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBFLD)         },
{ "TYP_DBNAMEFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNAMEFLD)         },
{ "TYP_DBNEXTSETFLD",           SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNEXTSETFLD)          },
{ "TYP_DBNUMSETFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNUMSETFLD)       },
{ "TYP_DBSETNUMBERFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBSETNUMBERFLD)        },
{ "TYP_DDEFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_DDEFLD)        },
{ "TYP_DOCINFOFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_DOCINFOFLD)        },
{ "TYP_DOCSTATFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_DOCSTATFLD)        },
{ "TYP_EXTUSERFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_EXTUSERFLD)        },
{ "TYP_FILENAMEFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_FILENAMEFLD)       },
{ "TYP_FIXDATEFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_FIXDATEFLD)        },
{ "TYP_FIXTIMEFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_FIXTIMEFLD)        },
{ "TYP_FORMELFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_FORMELFLD)         },
{ "TYP_GETFLD",             SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETFLD)        },
{ "TYP_GETREFFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETREFFLD)         },
{ "TYP_GETREFPAGEFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETREFPAGEFLD)         },
{ "TYP_HIDDENPARAFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_HIDDENPARAFLD)         },
{ "TYP_HIDDENTXTFLD",           SbxINTEGER,       _CPROP,    RTLNAME(TYP_HIDDENTXTFLD)          },
{ "TYP_INPUTFLD",           SbxINTEGER,       _CPROP,    RTLNAME(TYP_INPUTFLD)          },
{ "TYP_INTERNETFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_INTERNETFLD)       },
{ "TYP_JUMPEDITFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_JUMPEDITFLD)       },
{ "TYP_MACROFLD",           SbxINTEGER,       _CPROP,    RTLNAME(TYP_MACROFLD)          },
{ "TYP_NEXTPAGEFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_NEXTPAGEFLD)       },
{ "TYP_PAGENUMBERFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_PAGENUMBERFLD)         },
{ "TYP_POSTITFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_POSTITFLD)         },
{ "TYP_PREVPAGEFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_PREVPAGEFLD)       },
{ "TYP_SEQFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_SEQFLD)        },
{ "TYP_SETFLD",         SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETFLD)        },
{ "TYP_SETINPFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETINPFLD)         },
{ "TYP_SETREFFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETREFFLD)         },
{ "TYP_SETREFPAGEFLD",      SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETREFPAGEFLD)     },
{ "TYP_TEMPLNAMEFLD",           SbxINTEGER,       _CPROP,    RTLNAME(TYP_TEMPLNAMEFLD)          },
{ "TYP_TIMEFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_TIMEFLD)       },
{ "TYP_USERFLD",        SbxINTEGER,       _CPROP,    RTLNAME(TYP_USERFLD)       },
{ "TYP_USRINPFLD",          SbxINTEGER,       _CPROP,    RTLNAME(TYP_USRINPFLD)         },

{ "TypeLen",        SbxINTEGER,    1 | _FUNCTION, RTLNAME(TypeLen)          },
  { "Var",          SbxVARIANT },
{ "TypeName",       SbxSTRING,    1 | _FUNCTION, RTLNAME(TypeName)          },
  { "Var",          SbxVARIANT },
{ "UBound",         SbxINTEGER,   1 | _FUNCTION, RTLNAME(UBound)           },
  { "Var",          SbxVARIANT },
{ "UCase",          SbxSTRING,    1 | _FUNCTION, RTLNAME(UCase)             },
  { "String",       SbxSTRING },
{ "Unload",         SbxNULL,      1 | _FUNCTION, RTLNAME(Unload)            },
  { "Dialog",       SbxOBJECT },
{ "Val",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Val)               },
  { "String",       SbxSTRING },
{ "VarType",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(VarType)           },
  { "Var",          SbxVARIANT },
{ "V_EMPTY",        SbxINTEGER,       _CPROP,    RTLNAME(V_EMPTY)           },
{ "V_NULL",         SbxINTEGER,       _CPROP,    RTLNAME(V_NULL)            },
{ "V_INTEGER",      SbxINTEGER,       _CPROP,    RTLNAME(V_INTEGER)         },
{ "V_LONG",         SbxINTEGER,       _CPROP,    RTLNAME(V_LONG)            },
{ "V_SINGLE",       SbxINTEGER,       _CPROP,    RTLNAME(V_SINGLE)          },
{ "V_DOUBLE",       SbxINTEGER,       _CPROP,    RTLNAME(V_DOUBLE)          },
{ "V_CURRENCY",     SbxINTEGER,       _CPROP,    RTLNAME(V_CURRENCY)        },
{ "V_DATE",         SbxINTEGER,       _CPROP,    RTLNAME(V_DATE)            },
{ "V_STRING",       SbxINTEGER,       _CPROP,    RTLNAME(V_STRING)          },

{ "Wait",           SbxNULL,      1 | _FUNCTION, RTLNAME(Wait)           },
  { "Milliseconds", SbxLONG  },
{ "Weekday",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Weekday)           },
  { "Date",         SbxDATE },
{ "Year",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(Year)              },
  { "Date",         SbxDATE },

{ NULL,             SbxNULL,     -1 }};  // Tabellenende

SbiStdObject::SbiStdObject( const String& r, StarBASIC* pb ) : SbxObject( r )
{
    // Muessen wir die Hashcodes initialisieren?
    Methods* p = aMethods;
    if( !p->nHash )
      while( p->nArgs != -1 )
    {
        String aName = String::CreateFromAscii( p->pName );
        p->nHash = SbxVariable::MakeHashCode( aName );
        p += ( p->nArgs & _ARGSMASK ) + 1;
    }

    SetParent( pb );

    pStdFactory = new SbStdFactory;
    SbxBase::AddFactory( pStdFactory );

    Insert( new SbStdClipboard );
}

SbiStdObject::~SbiStdObject()
{
    SbxBase::RemoveFactory( pStdFactory );
    delete pStdFactory;
}

// Suche nach einem Element:
// Hier wird linear durch die Methodentabelle gegangen, bis eine
// passende Methode gefunden wurde. Auf Grund der Bits im nArgs-Feld
// wird dann die passende Instanz eines SbxObjElement generiert.
// Wenn die Methode/Property nicht gefunden wurde, nur NULL ohne
// Fehlercode zurueckliefern, da so auch eine ganze Chain von
// Objekten nach der Methode/Property befragt werden kann.

SbxVariable* SbiStdObject::Find( const String& rName, SbxClassType t )
{
    // Bereits eingetragen?
    SbxVariable* pVar = SbxObject::Find( rName, t );
    if( !pVar )
    {
        // sonst suchen
        USHORT nHash = SbxVariable::MakeHashCode( rName );
        Methods* p = aMethods;
        BOOL bFound = FALSE;
        short nIndex = 0;
        USHORT nSrchMask = _TYPEMASK;
        switch( t )
        {
            case SbxCLASS_METHOD:   nSrchMask = _METHOD; break;
            case SbxCLASS_PROPERTY: nSrchMask = _PROPERTY; break;
            case SbxCLASS_OBJECT:   nSrchMask = _OBJECT; break;
        }
        while( p->nArgs != -1 )
        {
            if( ( p->nArgs & nSrchMask )
             && ( p->nHash == nHash )
             && ( rName.EqualsIgnoreCaseAscii( p->pName ) ) )
            {
                bFound = TRUE; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = aMethods + nIndex;
        }
        if( bFound )
        {
            // Args-Felder isolieren:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            if( p->nArgs & _CONST )
                nAccess |= SBX_CONST;
            String aName = String::CreateFromAscii( p->pName );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pVar = Make( aName, eCT, p->eType );
            pVar->SetUserData( nIndex + 1 );
            pVar->SetFlags( nAccess );
        }
    }
    return pVar;
}

// SetModified muß bei der RTL abgklemmt werden
void SbiStdObject::SetModified( BOOL )
{
}

// Aufruf einer Property oder Methode.

void SbiStdObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType )

{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar = pVar->GetParameters();
        ULONG t = pHint->GetId();
        USHORT nCallId = (USHORT) pVar->GetUserData();
        if( nCallId )
        {
            if( t == SBX_HINT_INFOWANTED )
                pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
            else
            {
                BOOL bWrite = FALSE;
                if( t == SBX_HINT_DATACHANGED )
                    bWrite = TRUE;
                if( t == SBX_HINT_DATAWANTED || bWrite )
                {
                    RtlCall p = (RtlCall) aMethods[ nCallId-1 ].pFunc;
                    SbxArrayRef rPar( pPar );
                    if( !pPar )
                    {
                        rPar = pPar = new SbxArray;
                        pPar->Put( pVar, 0 );
                    }
                    p( (StarBASIC*) GetParent(), *pPar, bWrite );
                    return;
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

// Zusammenbau der Infostruktur fuer einzelne Elemente
// Falls nIdx = 0, nix erzeugen (sind Std-Props!)

SbxInfo* SbiStdObject::GetInfo( short nIdx )
{
    if( !nIdx )
        return NULL;
    Methods* p = &aMethods[ --nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName = String::CreateFromAscii( p->pName );
        USHORT nFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nFlags |= SBX_OPTIONAL;
        pInfo->AddParam( aName, p->eType, nFlags );
    }
    return pInfo;
}

