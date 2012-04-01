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

#include <basic/sberrors.hxx>

// The scanner is stand-alone, i. e. it can be used from everywhere.
// A BASIC-instance is necessary for error messages. Without BASIC
// the errors are only counted. Also the BASIC is necessary when an
// advanced SBX-variable shall be used for data type recognition etc.

class StarBASIC;

class SbiScanner
{
    ::rtl::OUString   aBuf;             // input buffer
    ::rtl::OUString   aLine;
    const sal_Unicode* pLine;
    const sal_Unicode* pSaveLine;

    void scanAlphanumeric();
    void scanGoto();
    bool readLine();
protected:
    ::rtl::OUString aSym;
    ::rtl::OUString aError;
    SbxDataType eScanType;
    StarBASIC* pBasic;                  // instance for error callbacks
    double nVal;                        // numeric value
    sal_Int32 nCurCol1;
    sal_Int32 nSavedCol1;
    sal_Int32 nCol;
    sal_Int32 nErrors;
    sal_Int32 nColLock;                    // lock counter for Col1
    sal_Int32 nBufPos;
    sal_Int32 nLine;
    sal_Int32 nCol1, nCol2;
    bool   bSymbol;                     // sal_True: symbol scanned
    bool   bNumber;                     // sal_True: number scanned
    bool   bSpaces;                     // sal_True: whitespace before token
    bool   bErrors;                     // sal_True: generate errors
    bool   bAbort;
    bool   bHash;                       // sal_True: # has been read in
    bool   bError;                      // sal_True: generate error
    bool   bCompatible;                 // sal_True: OPTION compatible
    bool   bVBASupportOn;               // sal_True: OPTION VBASupport 1 otherwise default False
    bool   bPrevLineExtentsComment;     // sal_True: Previous line is comment and ends on "... _"

    bool   bInStatement;
    void   GenError( SbError );
public:
    SbiScanner( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiScanner();

    void  EnableErrors()            { bError = sal_False; }
    bool  IsHash()                  { return bHash;   }
    bool  IsCompatible()            { return bCompatible; }
    void  SetCompatible( bool b )   { bCompatible = b; }        // #118206
    bool  IsVBASupportOn()          { return bVBASupportOn; }
    void  SetVBASupportOn( bool b ) { bVBASupportOn = b; }
    bool  WhiteSpace()              { return bSpaces; }
    sal_Int32 GetErrors()           { return nErrors; }
    sal_Int32 GetLine()             { return nLine;   }
    sal_Int32 GetCol1()             { return nCol1;   }
    sal_Int32 GetCol2()             { return nCol2;   }
    void  SetCol1( sal_Int32 n )    { nCol1 = n;      }
    StarBASIC* GetBasic()           { return pBasic;  }
    void  SaveLine(void)            { pSaveLine = pLine; }
    void  RestoreLine(void)         { pLine = pSaveLine; }
    void  LockColumn();
    void  UnlockColumn();
    bool  DoesColonFollow();

    bool NextSym();
    const ::rtl::OUString& GetSym() { return aSym;  }
    SbxDataType GetType()           { return eScanType; }
    double    GetDbl()              { return nVal;  }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
