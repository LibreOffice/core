/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_BASIC_SOURCE_INC_SCANNER_HXX
#define INCLUDED_BASIC_SOURCE_INC_SCANNER_HXX

#include <basic/sbxdef.hxx>
#include <vcl/errcode.hxx>

// The scanner is stand-alone, i. e. it can be used from everywhere.
// A BASIC-instance is necessary for error messages. Without BASIC
// the errors are only counted. Also the BASIC is necessary when an
// advanced SBX-variable shall be used for data type recognition etc.

class StarBASIC;

class SbiScanner
{
    OUString   aBuf;             // input buffer
    OUString   aLine;
    sal_Int32 nLineIdx;
    sal_Int32 nSaveLineIdx;
    StarBASIC* pBasic;                  // instance for error callbacks

    void scanAlphanumeric();
    void scanGoto();
    bool readLine();
protected:
    OUString aSym;
    OUString aError;
    SbxDataType eScanType;
    double nVal;                        // numeric value
    sal_Int32 nSavedCol1;
    sal_Int32 nCol;
    sal_Int32 nErrors;
    sal_Int32 nColLock;                    // lock counter for Col1
    sal_Int32 nBufPos;
    sal_Int32 nLine;
    sal_Int32 nCol1, nCol2;
    bool   bSymbol;                     // true: symbol scanned
    bool   bNumber;                     // true: number scanned
    bool   bSpaces;                     // true: whitespace before token
    bool   bAbort;
    bool   bHash;                       // true: # has been read in
    bool   bError;                      // true: generate error
    bool   bCompatible;                 // true: OPTION compatible
    bool   bVBASupportOn;               // true: OPTION VBASupport 1 otherwise default False
    bool   bPrevLineExtentsComment;     // true: Previous line is comment and ends on "... _"

    bool   bInStatement;
    void   GenError( ErrCode );
public:
    SbiScanner( const OUString&, StarBASIC* = nullptr );

    void  EnableErrors()            { bError = false; }
    bool  IsHash()                  { return bHash;   }
    bool  IsCompatible()            { return bCompatible; }
    void  SetCompatible( bool b )   { bCompatible = b; }        // #118206
    bool  IsVBASupportOn()          { return bVBASupportOn; }
    bool  WhiteSpace()              { return bSpaces; }
    sal_Int32 GetErrors()           { return nErrors; }
    sal_Int32 GetLine()             { return nLine;   }
    sal_Int32 GetCol1()             { return nCol1;   }
    void  SetCol1( sal_Int32 n )    { nCol1 = n;      }
    StarBASIC* GetBasic()           { return pBasic;  }
    void  SaveLine()                { nSaveLineIdx = nLineIdx; }
    void  RestoreLine()             { nLineIdx = nSaveLineIdx; }
    void  LockColumn();
    void  UnlockColumn();
    bool  DoesColonFollow();

    bool NextSym();
    const OUString& GetSym() { return aSym;  }
    SbxDataType GetType()           { return eScanType; }
    double    GetDbl()              { return nVal;  }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
