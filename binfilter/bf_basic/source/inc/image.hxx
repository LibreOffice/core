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
#include "filefmt.hxx"

// Diese Klasse liest das vom Compiler erzeugte Image ein und verwaltet
// den Zugriff auf die einzelnen Elemente.

namespace binfilter {

struct SbPublicEntry;

class SbiImage {
    friend class SbiCodeGen;			// Compiler-Klassen, die die private-

    SbxArrayRef    rTypes;			// User defined types
    SbxArrayRef    rEnums;			// Enum types
    UINT32*		   pStringOff;		// StringId-Offsets
    sal_Unicode*   pStrings;		// StringPool
    char* 		   pCode;			// Code-Image
    char* 		   pLegacyPCode;		// Code-Image
    BOOL		   bError;			// TRUE: Fehler
    USHORT		   nFlags;			// Flags (s.u.)
    short		   nStrings;   		// Anzahl Strings
    UINT32		   nStringSize;		// Groesse des String-Puffers
    UINT32		   nCodeSize;		// Groesse des Code-Blocks
    UINT16		   nLegacyCodeSize;		// Groesse des Code-Blocks
    UINT16		   nDimBase;		// OPTION BASE-Wert
    rtl_TextEncoding eCharSet;		// Zeichensatz fuer Strings
                                    // temporaere Verwaltungs-Variable:
    short		   nStringIdx;		// aktueller String-Index
    UINT32		   nStringOff;		// aktuelle Pos im Stringpuffer
                                    // Routinen fuer Compiler:
    void MakeStrings( short );		// StringPool einrichten

public:
    String aName;					// Makroname
    ::rtl::OUString aOUSource;		// Quellcode
    String aComment;				// Kommentar
    BOOL   bInit;					// TRUE: Init-Code ist gelaufen
    BOOL   bFirstInit;      		// TRUE, wenn das Image das erste mal nach
                                    // dem Compilieren initialisiert wird.
    SbiImage();
   ~SbiImage();
    void Clear();					// Inhalt loeschen
    BOOL Load( SvStream&, UINT32& nVer );		// Loads image from stream
                            // nVer is set to version
                            // of image
    BOOL Save( SvStream&, UINT32 = B_CURVERSION );
    BOOL IsError() 					{ return bError;    }

    const char* GetCode() const 	{ return pCode; 	}
    UINT32		GetCodeSize() const	{ return nCodeSize;	}
    ::rtl::OUString& GetSource32() 	{ return aOUSource; }
    USHORT		GetBase() const		{ return nDimBase;	}
    const SbxObject*  FindType (String aTypeName) const;

    SbxArrayRef GetEnums()			{ return rEnums; }

    void		SetFlag( USHORT n )	{ nFlags |= n;		}
    USHORT		GetFlag( USHORT n )	const { return nFlags & n; }
    UINT16		CalcLegacyOffset( INT32 nOffset );
    UINT32		CalcNewOffset( INT16 nOffset );
    void		ReleaseLegacyBuffer();
    BOOL		ExceedsLegacyLimits();
    
};

#define	SBIMG_EXPLICIT		0x0001	// OPTION EXPLICIT ist aktiv
#define	SBIMG_COMPARETEXT	0x0002	// OPTION COMPARE TEXT ist aktiv
#define	SBIMG_INITCODE		0x0004	// Init-Code vorhanden
#define	SBIMG_CLASSMODULE	0x0008	// OPTION ClassModule is active
#define	SBIMG_VBASUPPORT	0x0020	// OPTION VBASupport is 1

}

#endif
