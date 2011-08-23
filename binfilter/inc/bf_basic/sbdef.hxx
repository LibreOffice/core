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

#ifndef _SB_SBDEF_HXX
#define _SB_SBDEF_HXX


#ifndef _SBXDEF_HXX
#include <sbxdef.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#if _SOLAR__PRIVATE
#define _BASIC_TEXTPORTIONS
#endif

namespace binfilter {

enum SbTextType {					// Typ eines Textteils (Syntax Hilite)
    SB_KEYWORD = 1,					// Keywords
    SB_SYMBOL,						// Symbole
    SB_STRING,						// Strings
    SB_NUMBER,						// Zahlen
    SB_PUNCTUATION,					// Klammern, Punkte etc
    SB_COMMENT,						// Kommentare
    SB_DUMMY = 255                  // workaround for #i31479
};

enum SbLanguageMode {				// Aktive Sprache
    SB_LANG_GLOBAL,					// wie in SbiGlobals-Struktur
    SB_LANG_BASIC,					// StarBasic (Default)
    SB_LANG_VBSCRIPT,				// Visual-Basic-Script
    SB_LANG_JAVASCRIPT				// Java-Script
};

#ifdef _BASIC_TEXTPORTIONS
struct SbTextPortion
{									// Syntax Hiliting: eine Text-Portion
    xub_StrLen nLine;					// Zeilennummer
    xub_StrLen nStart, nEnd;			// 1. und letzte Spalte
    SbTextType eType;				// Type der Portion
};
#endif

// Returns type name for Basic type, array flag is ignored
// implementation: basic/source/runtime/methods.cxx
String getBasicTypeName( SbxDataType eType );

// Returns type name for Basic objects, especially
// important for SbUnoObj instances
// implementation: basic/source/classes/sbunoobj.cxx
class SbxObject;

// Allows Basic IDE to set watch mode to suppress errors
// implementation: basic/source/runtime/runtime.cxx
void setBasicWatchMode( bool bOn );

// Debug-Flags:

#define SbDEBUG_BREAK       0x0001          // Break-Callback
#define SbDEBUG_STEPINTO    0x0002          // Single Step-Callback
#define SbDEBUG_STEPOVER    0x0004          // Zusatzflag Step Over
#define	SbDEBUG_CONTINUE	0x0008			// Flags nicht aendern
#define SbDEBUG_STEPOUT		0x0010          // Aus Sub raus

#define SBXID_BASIC			0x6273			// sb: StarBASIC
#define	SBXID_BASICMOD		0x6d62			// bm: StarBASIC-Modul
#define SBXID_BASICPROP		0x7262			// pr: StarBASIC-Property
#define	SBXID_BASICMETHOD	0x6d65          // me: StarBASIC-Methode
#define	SBXID_JSCRIPTMOD	0x6a62			// jm: JavaScript-Modul
#define	SBXID_JSCRIPTMETH	0x6a64			// jm: JavaScript-Modul


#define SBX_HINT_BASICSTART		SFX_HINT_USER04
#define SBX_HINT_BASICSTOP		SFX_HINT_USER05

// #115826
enum PropertyMode
{
    PROPERTY_MODE_NONE,
    PROPERTY_MODE_GET,
    PROPERTY_MODE_LET,
    PROPERTY_MODE_SET
};

}

#endif
