/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/string.hxx>
#include <basic/sberrors.hxx>

// The scanner is stand-alone, i. e. it can be used from everywhere.
// A BASIC-instance is necessary for error messages. Without BASIC
// the errors are only counted. Also the BASICC is necessary when an
// advanced SBX-variable shall be used for data type recognition etc.

class StarBASIC;

class SbiScanner
{
    ::rtl::OUString   aBuf;             // input buffer
    ::rtl::OUString   aLine;
    const sal_Unicode* pLine;
    const sal_Unicode* pSaveLine;
protected:
    String aSym;
    String aError;
    SbxDataType eScanType;
    StarBASIC* pBasic;                  // instance for error callbacks
    double nVal;                        // numeric value
    short  nCurCol1;
    short  nSavedCol1;
    short  nCol;
    short  nErrors;
    short  nColLock;                    // lock counter for Col1
    sal_Int32  nBufPos;
    sal_uInt16 nLine;
    sal_uInt16 nCol1, nCol2;
    sal_Bool   bSymbol;                     // sal_True: symbol scanned
    sal_Bool   bNumber;                     // sal_True: number scanned
    sal_Bool   bSpaces;                     // sal_True: whitespace before token
    sal_Bool   bErrors;                     // sal_True: generate errors
    sal_Bool   bAbort;
    sal_Bool   bHash;                       // sal_True: # has been read in
    sal_Bool   bError;                      // sal_True: generate error
    sal_Bool   bUsedForHilite;
    sal_Bool   bCompatible;                 // sal_True: OPTION compatible
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

    sal_Bool NextSym();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
