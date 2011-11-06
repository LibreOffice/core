/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SCANNER_HXX
#define _SCANNER_HXX

#include <tools/string.hxx>
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
    sal_Int32  nBufPos;                     // aktuelle Buffer-Pos
    sal_uInt16 nLine;                       // aktuelle Zeile
    sal_uInt16 nCol1, nCol2;                // aktuelle 1. und 2. Spalte
    sal_Bool   bSymbol;                     // sal_True: Symbol gescannt
    sal_Bool   bNumber;                     // sal_True: Zahl gescannt
    sal_Bool   bSpaces;                     // sal_True: Whitespace vor Token
    sal_Bool   bErrors;                     // sal_True: Fehler generieren
    sal_Bool   bAbort;                      // sal_True: abbrechen
    sal_Bool   bHash;                       // sal_True: # eingelesen
    sal_Bool   bError;                      // sal_True: Fehler generieren
    sal_Bool   bUsedForHilite;              // sal_True: Nutzung fuer Highlighting
    sal_Bool   bCompatible;                 // sal_True: OPTION Compatibl
    sal_Bool   bVBASupportOn;               // sal_True: OPTION VBASupport 1 otherwise default False
    sal_Bool   bPrevLineExtentsComment;     // sal_True: Previous line is comment and ends on "... _"

    void   GenError( SbError );
public:
    SbiScanner( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiScanner();

    void  EnableErrors()            { bError = sal_False; }
    sal_Bool  IsHash()                  { return bHash;   }
    sal_Bool  IsCompatible()            { return bCompatible; }
    void  SetCompatible( bool b )   { bCompatible = b; }        // #118206
    sal_Bool  IsVBASupportOn()          { return bVBASupportOn; }
    void  SetVBASupportOn( bool b ) { bVBASupportOn = b; }
    sal_Bool  WhiteSpace()              { return bSpaces; }
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
    sal_Bool  DoesColonFollow();

    sal_Bool NextSym();                 // naechstes Symbol lesen
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
    static sal_Bool isAlpha( sal_Unicode c, bool bCompatible )
    {
        sal_Bool bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                    || (bCompatible && aLetterTable.isLetter( c ));
        return bRet;
    }

    static sal_Bool isDigit( sal_Unicode c )
    {
        sal_Bool bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    static sal_Bool isAlphaNumeric( sal_Unicode c, bool bCompatible )
    {
        sal_Bool bRet = isDigit( c ) || isAlpha( c, bCompatible );
        return bRet;
    }
};

#endif
