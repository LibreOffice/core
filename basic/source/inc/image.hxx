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

#ifndef _SBIMAGE_HXX
#define _SBIMAGE_HXX

#include "sbintern.hxx"
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#include <filefmt.hxx>

// Diese Klasse liest das vom Compiler erzeugte Image ein und verwaltet
// den Zugriff auf die einzelnen Elemente.

struct SbPublicEntry;

class SbiImage {
    friend class SbiCodeGen;            // Compiler-Klassen, die die private-

    SbxArrayRef    rTypes;          // User defined types
    SbxArrayRef    rEnums;          // Enum types
    sal_uInt32*        pStringOff;      // StringId-Offsets
    sal_Unicode*   pStrings;        // StringPool
    char*          pCode;           // Code-Image
    char*          pLegacyPCode;        // Code-Image
    sal_Bool           bError;          // sal_True: Fehler
    sal_uInt16         nFlags;          // Flags (s.u.)
    short          nStrings;        // Anzahl Strings
    sal_uInt32         nStringSize;     // Groesse des String-Puffers
    sal_uInt32         nCodeSize;       // Groesse des Code-Blocks
    sal_uInt16         nLegacyCodeSize;     // Groesse des Code-Blocks
    sal_uInt16         nDimBase;        // OPTION BASE-Wert
    rtl_TextEncoding eCharSet;      // Zeichensatz fuer Strings
                                    // temporaere Verwaltungs-Variable:
    short          nStringIdx;      // aktueller String-Index
    sal_uInt32         nStringOff;      // aktuelle Pos im Stringpuffer
                                    // Routinen fuer Compiler:
    void MakeStrings( short );      // StringPool einrichten
    void AddString( const String& );// String zufuegen
    void AddCode( char*, sal_uInt32 );  // Codeblock dazu
    void AddType(SbxObject *);      // User-Type mit aufnehmen
    void AddEnum(SbxObject *);      // Register enum type

public:
    String aName;                   // Makroname
    ::rtl::OUString aOUSource;      // Quellcode
    String aComment;                // Kommentar
    sal_Bool   bInit;                   // sal_True: Init-Code ist gelaufen
    sal_Bool   bFirstInit;              // sal_True, wenn das Image das erste mal nach
                                    // dem Compilieren initialisiert wird.
    SbiImage();
   ~SbiImage();
    void Clear();                   // Inhalt loeschen
    sal_Bool Load( SvStream&, sal_uInt32& nVer );       // Loads image from stream
                            // nVer is set to version
                            // of image
    sal_Bool Load( SvStream& );
    sal_Bool Save( SvStream&, sal_uInt32 = B_CURVERSION );
    sal_Bool IsError()                  { return bError;    }

    const char* GetCode() const     { return pCode;     }
    sal_uInt32      GetCodeSize() const { return nCodeSize; }
    ::rtl::OUString& GetSource32()  { return aOUSource; }
    sal_uInt16      GetBase() const     { return nDimBase;  }
    String      GetString( short nId ) const;
    //const char* GetString( short nId ) const;
    const SbxObject*  FindType (String aTypeName) const;

    SbxArrayRef GetEnums()          { return rEnums; }

    void        SetFlag( sal_uInt16 n ) { nFlags |= n;      }
    sal_uInt16      GetFlag( sal_uInt16 n ) const { return nFlags & n; }
    sal_uInt16      CalcLegacyOffset( sal_Int32 nOffset );
    sal_uInt32      CalcNewOffset( sal_Int16 nOffset );
    void        ReleaseLegacyBuffer();
    sal_Bool        ExceedsLegacyLimits();

};

#define SBIMG_EXPLICIT      0x0001  // OPTION EXPLICIT ist aktiv
#define SBIMG_COMPARETEXT   0x0002  // OPTION COMPARE TEXT ist aktiv
#define SBIMG_INITCODE      0x0004  // Init-Code vorhanden
#define SBIMG_CLASSMODULE   0x0008  // OPTION ClassModule is active

#endif
