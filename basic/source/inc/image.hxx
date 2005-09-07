/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: image.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:33:08 $
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

#ifndef _SBIMAGE_HXX
#define _SBIMAGE_HXX

#include "sbintern.hxx"
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

// Diese Klasse liest das vom Compiler erzeugte Image ein und verwaltet
// den Zugriff auf die einzelnen Elemente.

struct SbPublicEntry;

class SbiImage {
    friend class SbiCodeGen;            // Compiler-Klassen, die die private-

    SbxArrayRef    rTypes;          // User defined types
    SbxArrayRef    rEnums;          // Enum types
    UINT16*        pStringOff;      // StringId-Offsets
    sal_Unicode*   pStrings;        // StringPool
    char*          pCode;           // Code-Image
    BOOL           bError;          // TRUE: Fehler
    USHORT         nFlags;          // Flags (s.u.)
    short          nStrings;        // Anzahl Strings
    UINT16         nStringSize;     // Groesse des String-Puffers
    UINT16         nCodeSize;       // Groesse des Code-Blocks
    UINT16         nDimBase;        // OPTION BASE-Wert
    rtl_TextEncoding eCharSet;      // Zeichensatz fuer Strings
                                    // temporaere Verwaltungs-Variable:
    short          nStringIdx;      // aktueller String-Index
    UINT16         nStringOff;      // aktuelle Pos im Stringpuffer
                                    // Routinen fuer Compiler:
    void MakeStrings( short );      // StringPool einrichten
    void AddString( const String& );// String zufuegen
    void AddCode( char*, USHORT );  // Codeblock dazu
    void AddType(SbxObject *);      // User-Type mit aufnehmen
    void AddEnum(SbxObject *);      // Register enum type

public:
    String aName;                   // Makroname
    ::rtl::OUString aOUSource;      // Quellcode
    String aComment;                // Kommentar
    BOOL   bInit;                   // TRUE: Init-Code ist gelaufen
    BOOL   bFirstInit;              // TRUE, wenn das Image das erste mal nach
                                    // dem Compilieren initialisiert wird.
    SbiImage();
   ~SbiImage();
    void Clear();                   // Inhalt loeschen
    BOOL Load( SvStream& );
    BOOL Save( SvStream& );
    BOOL IsError()                  { return bError;    }

    const char* GetCode() const     { return pCode;     }
    USHORT      GetCodeSize() const { return nCodeSize; }
    ::rtl::OUString& GetSource32()  { return aOUSource; }
    USHORT      GetBase() const     { return nDimBase;  }
    String      GetString( short nId ) const;
    //const char* GetString( short nId ) const;
    const SbxObject*  FindType (String aTypeName) const;

    SbxArrayRef GetEnums()          { return rEnums; }

    void        SetFlag( USHORT n ) { nFlags |= n;      }
    USHORT      GetFlag( USHORT n ) const { return nFlags & n; }
};

#define SBIMG_EXPLICIT      0x0001  // OPTION EXPLICIT ist aktiv
#define SBIMG_COMPARETEXT   0x0002  // OPTION COMPARE TEXT ist aktiv
#define SBIMG_INITCODE      0x0004  // Init-Code vorhanden
#define SBIMG_CLASSMODULE   0x0008  // OPTION ClassModule is active

#endif
