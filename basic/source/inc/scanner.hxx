/*************************************************************************
 *
 *  $RCSfile: scanner.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#ifndef _SCANNER_HXX
#define _SCANNER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SBERRORS_HXX
#include "sberrors.hxx"
#endif

// Der Scanner ist stand-alone, d.h. er kann von ueberallher verwendet
// werden. Eine BASIC-Instanz ist fuer Fehlermeldungen notwendig. Ohne
// BASIC werden die Fehler nur gezaehlt. Auch ist Basic notwendig, wenn
// eine erweiterte SBX-Variable zur Erkennung von Datentypen etc. verwendet
// werden soll.

class StarBASIC;

class SbiScanner
{
    String      aBuf;                   // Input-Puffer
    String      aLine;                  // aktuelle Zeile
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
    USHORT nBufPos;                     // aktuelle Buffer-Pos
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

    void   GenError( SbError );
public:
    SbiScanner( const String&, StarBASIC* = NULL );
   ~SbiScanner();

    void  EnableErrors()            { bError = FALSE; }
    BOOL  IsHash()                  { return bHash;   }
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


#endif
