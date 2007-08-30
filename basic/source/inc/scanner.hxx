/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scanner.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 10:01:17 $
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

#ifndef _SCANNER_HXX
#define _SCANNER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SBERRORS_HXX
#include <basic/sberrors.hxx>
#endif

// Der Scanner ist stand-alone, d.h. er kann von ueberallher verwendet
// werden. Eine BASIC-Instanz ist fuer Fehlermeldungen notwendig. Ohne
// BASIC werden die Fehler nur gezaehlt. Auch ist Basic notwendig, wenn
// eine erweiterte SBX-Variable zur Erkennung von Datentypen etc. verwendet
// werden soll.

class StarBASIC;

class SbiScanner
{
    ::rtl::OUString   aBuf;             // Input-Puffer
    ::rtl::OUString   aLine;            // aktuelle Zeile
    const sal_Unicode* pLine;           // Pointer
    const sal_Unicode* pSaveLine;       // Merker fuer Line
protected:
    String aSym;                        // Symbolpuffer
    String aError;                      // Fehler-String
    SbxDataType eScanType;              // evtl. Datentyp
    StarBASIC* pBasic;                  // Instanz fuer Fehler-Callbacks
    double nVal;                        // numerischer Wert
    short  nCurCol1;                    // aktuelle Spalte 1
    short  nSavedCol1;                  // gerettete Spalte 1
    short  nCol;                        // aktuelle Spaltennummer
    short  nErrors;                     // Anzahl Fehler
    short  nColLock;                    // Lock-Zaehler fuer Col1
    INT32  nBufPos;                     // aktuelle Buffer-Pos
    USHORT nLine;                       // aktuelle Zeile
    USHORT nCol1, nCol2;                // aktuelle 1. und 2. Spalte
    BOOL   bSymbol;                     // TRUE: Symbol gescannt
    BOOL   bNumber;                     // TRUE: Zahl gescannt
    BOOL   bSpaces;                     // TRUE: Whitespace vor Token
    BOOL   bErrors;                     // TRUE: Fehler generieren
    BOOL   bAbort;                      // TRUE: abbrechen
    BOOL   bHash;                       // TRUE: # eingelesen
    BOOL   bError;                      // TRUE: Fehler generieren
    BOOL   bUsedForHilite;              // TRUE: Nutzung fuer Highlighting
    BOOL   bCompatible;                 // TRUE: OPTION Compatibl
    BOOL   bVBASupportOn;               // TRUE: OPTION VBASupport 1 otherwise default False
    BOOL   bPrevLineExtentsComment;     // TRUE: Previous line is comment and ends on "... _"

    void   GenError( SbError );
public:
    SbiScanner( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiScanner();

    void  EnableErrors()            { bError = FALSE; }
    BOOL  IsHash()                  { return bHash;   }
    BOOL  IsCompatible()            { return bCompatible; }
    void  SetCompatible( bool b )   { bCompatible = b; }        // #118206
    BOOL  IsVBASupportOn()          { return bVBASupportOn; }
    void  SetVBASupportOn( bool b ) { bVBASupportOn = b; }
    BOOL  WhiteSpace()              { return bSpaces; }
    short GetErrors()               { return nErrors; }
    short GetLine()                 { return nLine;   }
    short GetCol1()                 { return nCol1;   }
    short GetCol2()                 { return nCol2;   }
    void  SetCol1( short n )        { nCol1 = n;      }
    StarBASIC* GetBasic()           { return pBasic;  }
    void  SaveLine(void)            { pSaveLine = pLine; }
    void  RestoreLine(void)         { pLine = pSaveLine; }
    void  LockColumn();
    void  UnlockColumn();
    BOOL  DoesColonFollow();

    BOOL NextSym();                 // naechstes Symbol lesen
    const String& GetSym()          { return aSym;  }
    SbxDataType GetType()           { return eScanType; }
    double    GetDbl()              { return nVal;  }
};

class LetterTable
{
    bool        IsLetterTab[256];

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

#endif
