/*************************************************************************
 *
 *  $RCSfile: image.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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

#ifndef _SBIMAGE_HXX
#define _SBIMAGE_HXX

#include "sbintern.hxx"

// Diese Klasse liest das vom Compiler erzeugte Image ein und verwaltet
// den Zugriff auf die einzelnen Elemente.

struct SbPublicEntry;

class SbiImage {
    friend class SbiCodeGen;            // Compiler-Klassen, die die private-

    SbxArrayRef    rTypes;          //
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

public:
    String aName;                   // Makroname
    String aSource;                 // Quellcode
    String aComment;                // Kommentar
    BOOL   bInit;                   // TRUE: Init-Code ist gelaufen
    SbiImage();
   ~SbiImage();
    void Clear();                   // Inhalt loeschen
    BOOL Load( SvStream& );
    BOOL Save( SvStream& );
    BOOL IsError()                  { return bError;    }

    const char* GetCode() const     { return pCode;     }
    USHORT      GetCodeSize() const { return nCodeSize; }
    String&     GetSource()         { return aSource;   }
    USHORT      GetBase() const     { return nDimBase;  }
    String      GetString( short nId ) const;
    //const char* GetString( short nId ) const;
    const SbxObject*  FindType (String aTypeName) const;

    void        SetFlag( USHORT n ) { nFlags |= n;      }
    USHORT      GetFlag( USHORT n ) const { return nFlags & n; }
};

#define SBIMG_EXPLICIT      0x0001  // OPTION EXPLICIT ist aktiv
#define SBIMG_COMPARETEXT   0x0002  // OPTION COMPARE TEXT ist aktiv
#define SBIMG_INITCODE      0x0004  // Init-Code vorhanden

#endif
