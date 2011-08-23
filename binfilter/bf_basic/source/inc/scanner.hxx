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

#ifndef _SCANNER_HXX
#define _SCANNER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SBERRORS_HXX
#include "sberrors.hxx"
#endif

namespace binfilter {

// Der Scanner ist stand-alone, d.h. er kann von ueberallher verwendet
// werden. Eine BASIC-Instanz ist fuer Fehlermeldungen notwendig. Ohne
// BASIC werden die Fehler nur gezaehlt. Auch ist Basic notwendig, wenn
// eine erweiterte SBX-Variable zur Erkennung von Datentypen etc. verwendet
// werden soll.

class StarBASIC;

class SbiScanner
{
    ::rtl::OUString   aBuf;				// Input-Puffer
    ::rtl::OUString   aLine;			// aktuelle Zeile
    const sal_Unicode* pLine;			// Pointer
    const sal_Unicode* pSaveLine;		// Merker fuer Line
protected:
    String aSym;						// Symbolpuffer
    String aError;						// Fehler-String
    SbxDataType eScanType;				// evtl. Datentyp
    StarBASIC* pBasic;					// Instanz fuer Fehler-Callbacks
    double nVal; 						// numerischer Wert
    short  nCurCol1;  			   		// aktuelle Spalte 1
    short  nSavedCol1;					// gerettete Spalte 1
    short  nCol; 						// aktuelle Spaltennummer
    short  nErrors;						// Anzahl Fehler
    short  nColLock;					// Lock-Zaehler fuer Col1
    INT32  nBufPos;						// aktuelle Buffer-Pos
    USHORT nLine;						// aktuelle Zeile
    USHORT nCol1, nCol2;				// aktuelle 1. und 2. Spalte
    BOOL   bSymbol;						// TRUE: Symbol gescannt
    BOOL   bNumber;						// TRUE: Zahl gescannt
    BOOL   bSpaces;						// TRUE: Whitespace vor Token
    BOOL   bErrors;						// TRUE: Fehler generieren
    BOOL   bAbort;						// TRUE: abbrechen
    BOOL   bHash;						// TRUE: # eingelesen
    BOOL   bError;						// TRUE: Fehler generieren
    BOOL   bUsedForHilite;				// TRUE: Nutzung fuer Highlighting
    BOOL   bCompatible; 				// TRUE: OPTION Compatible
    BOOL   bPrevLineExtentsComment;		// TRUE: Previous line is comment and ends on "... _"

    void   GenError( SbError );
public:
    SbiScanner( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiScanner();

    void  EnableErrors()   			{ bError = FALSE; }
    BOOL  IsHash()					{ return bHash;   }
    BOOL  IsCompatible()			{ return bCompatible; }
    void  SetCompatible( bool b )	{ bCompatible = b; }		// #118206
    BOOL  WhiteSpace()				{ return bSpaces; }
    short GetErrors()				{ return nErrors; }
    short GetLine()					{ return nLine;   }
    short GetCol1()					{ return nCol1;   }
    short GetCol2()					{ return nCol2;   }
    void  SetCol1( short n )		{ nCol1 = n; 	  }
    StarBASIC* GetBasic()			{ return pBasic;  }
    void  SaveLine(void)			{ pSaveLine = pLine; }
    void  RestoreLine(void)			{ pLine = pSaveLine; }
    void  LockColumn();
    void  UnlockColumn();

    BOOL NextSym();					// naechstes Symbol lesen
    const String& GetSym()			{ return aSym;	}
    SbxDataType GetType()		   	{ return eScanType; }
    double	  GetDbl()				{ return nVal;	}
};

class LetterTable
{
    bool		IsLetterTab[256];

public:
    LetterTable( void );

    inline bool isLetter( sal_Unicode c )
    {
        bool bRet = (c < 256) ? IsLetterTab[c] : isLetterUnicode( c );
        return bRet;
    }
    bool isLetterUnicode( sal_Unicode c );
};

class BasicSimpleCharClass
{
    static LetterTable aLetterTable;

public:
    static BOOL isAlpha( sal_Unicode c, bool bCompatible )
    {
        BOOL bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') 
                    || (bCompatible && aLetterTable.isLetter( c ));
        return bRet;
    }

    static BOOL isDigit( sal_Unicode c )
    {
        BOOL bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    static BOOL isAlphaNumeric( sal_Unicode c, bool bCompatible )
    {
        BOOL bRet = isDigit( c ) || isAlpha( c, bCompatible );
        return bRet;
    }
};

}

#endif
