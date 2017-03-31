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
#ifndef INCLUDED_RSC_INC_RSCPAR_HXX
#define INCLUDED_RSC_INC_RSCPAR_HXX

#include <rsctools.hxx>
#include <rscerror.h>
#include <rscdef.hxx>
#include <tools/solar.h>

class RscTypCont;

#define READBUFFER_MAX  256

class RscFileInst
{
    ERRTYPE             aFirstError;
    sal_uInt32          nErrorLine;
    sal_uInt32          nErrorPos;
    sal_uInt32          nLineNo;    // line in input file
    RscFileTab::Index   lFileIndex; // index input file
    RscFileTab::Index   lSrcIndex;  // index base file
    FILE *              fInputFile;
    char *              pInput;     // read buffer
    static const sal_uInt32 nInputBufLen = READBUFFER_MAX;
    sal_uInt32          nInputPos;
    sal_uInt32          nInputEndPos;
    char *              pLine;
    sal_uInt32          nLineBufLen;
    sal_uInt32          nScanPos;   // line position
    int                 cLastChar;
    bool                bEof;

public:
    RscTypCont *        pTypCont;
    void        Init();  // init ctor
                RscFileInst( RscTypCont * pTC, RscFileTab::Index lIndexSrc,
                         RscFileTab::Index lFileIndex, FILE * fFile );
                ~RscFileInst();
    bool        IsEof() const { return bEof; }
    void        SetFileIndex( RscFileTab::Index lFIndex ) { lFileIndex = lFIndex;  }
    RscFileTab::Index GetFileIndex()                { return lFileIndex;  }
    void        SetLineNo( sal_uInt32 nLine ) { nLineNo = nLine;    }
    sal_uInt32  GetLineNo()                   { return nLineNo;     }
    sal_uInt32  GetScanPos()                  { return nScanPos;    }
    char *      GetLine()                     { return pLine;       }
    char        GetChar();
    char        GetFastChar()
                    {
                        return pLine[ nScanPos ] ?
                            pLine[ nScanPos++ ] : GetChar();
                    }
    void        GetNewLine();
                // error handling
    void        SetError( ERRTYPE aError );
};

void IncludeParser( RscFileInst * pFileInst );
ERRTYPE parser( RscFileInst * pFileInst );

#endif // INCLUDED_RSC_INC_RSCPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
