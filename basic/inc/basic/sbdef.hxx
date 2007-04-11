/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbdef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:51:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SB_SBDEF_HXX
#define _SB_SBDEF_HXX

#ifndef _SBXDEF_HXX
#include <basic/sbxdef.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#define _BASIC_TEXTPORTIONS

enum SbTextType {                   // Typ eines Textteils (Syntax Hilite)
    SB_KEYWORD = 1,                 // Keywords
    SB_SYMBOL,                      // Symbole
    SB_STRING,                      // Strings
    SB_NUMBER,                      // Zahlen
    SB_PUNCTUATION,                 // Klammern, Punkte etc
    SB_COMMENT,                     // Kommentare
    SB_DUMMY = 255                  // workaround for #i31479
};

enum SbLanguageMode {               // Aktive Sprache
    SB_LANG_GLOBAL,                 // wie in SbiGlobals-Struktur
    SB_LANG_BASIC,                  // StarBasic (Default)
    SB_LANG_VBSCRIPT,               // Visual-Basic-Script
    SB_LANG_JAVASCRIPT              // Java-Script
};

#ifdef _BASIC_TEXTPORTIONS
struct SbTextPortion
{                                   // Syntax Hiliting: eine Text-Portion
    xub_StrLen nLine;                   // Zeilennummer
    xub_StrLen nStart, nEnd;            // 1. und letzte Spalte
    SbTextType eType;               // Type der Portion
};

SV_DECL_VARARR(SbTextPortions, SbTextPortion,16,16)
#else
class SbTextPortions;
#endif

// Returns type name for Basic type, array flag is ignored
// implementation: basic/source/runtime/methods.cxx
String getBasicTypeName( SbxDataType eType );

// Returns type name for Basic objects, especially
// important for SbUnoObj instances
// implementation: basic/source/classes/sbunoobj.cxx
class SbxObject;
String getBasicObjectTypeName( SbxObject* pObj );

// Allows Basic IDE to set watch mode to suppress errors
// implementation: basic/source/runtime/runtime.cxx
void setBasicWatchMode( bool bOn );

// Debug-Flags:

#define SbDEBUG_BREAK       0x0001          // Break-Callback
#define SbDEBUG_STEPINTO    0x0002          // Single Step-Callback
#define SbDEBUG_STEPOVER    0x0004          // Zusatzflag Step Over
#define SbDEBUG_CONTINUE    0x0008          // Flags nicht aendern
#define SbDEBUG_STEPOUT     0x0010          // Aus Sub raus

#define SBXID_BASIC         0x6273          // sb: StarBASIC
#define SBXID_BASICMOD      0x6d62          // bm: StarBASIC-Modul
#define SBXID_BASICPROP     0x7262          // pr: StarBASIC-Property
#define SBXID_BASICMETHOD   0x6d65          // me: StarBASIC-Methode
#define SBXID_JSCRIPTMOD    0x6a62          // jm: JavaScript-Modul
#define SBXID_JSCRIPTMETH   0x6a64          // jm: JavaScript-Modul

#define SBX_HINT_BASICSTART     SFX_HINT_USER04
#define SBX_HINT_BASICSTOP      SFX_HINT_USER05

// #115826
enum PropertyMode
{
    PROPERTY_MODE_NONE,
    PROPERTY_MODE_GET,
    PROPERTY_MODE_LET,
    PROPERTY_MODE_SET
};

#endif
