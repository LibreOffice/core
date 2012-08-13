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
#ifndef _RSCPAR_HXX
#define _RSCPAR_HXX

#include <rsctools.hxx>
#include <rscerror.h>

/****************** C L A S S E S ****************************************/
class RscTypCont;
/*********** R s c F i l e I n s t ***************************************/

#define READBUFFER_MAX  256
class RscFileInst
{
    ERRTYPE             aFirstError;// Erster Fehler
    sal_uInt32              nErrorLine; // Zeile des ersten Fehlers
    sal_uInt32              nErrorPos;  // Position des ersten Fehlers
    sal_uInt32              nLineNo;    // Zeile in der Eingabedatei
    sal_uLong               lFileIndex; // Index auf Eingabedatei
    sal_uLong               lSrcIndex;  // Index auf Basisdatei
    FILE *              fInputFile; // Eingabedatei
    char *              pInput;     // Lesepuffer
    sal_uInt32              nInputBufLen; // Laenge des Lesepuffers
    sal_uInt32              nInputPos;  // Position im Lesepuffer
    sal_uInt32              nInputEndPos;// Ende im Lesepuffer
    char *              pLine;      // Zeile
    sal_uInt32              nLineBufLen;//Lange des Zeilenpuffres
    sal_uInt32              nScanPos;   // Position in der Zeile
    int                 cLastChar;
    sal_Bool                bEof;

public:
    RscTypCont *        pTypCont;
    void    Init();  // ctor initialisieren
            RscFileInst( RscTypCont * pTC, sal_uLong lIndexSrc,
                         sal_uLong lFileIndex, FILE * fFile );
            ~RscFileInst();
    sal_Bool    IsEof() const { return bEof; }
    void    SetFileIndex( sal_uLong lFIndex ) { lFileIndex = lFIndex;  }
    sal_uLong   GetFileIndex()                { return( lFileIndex );  }
    sal_uLong   GetSrcIndex()                 { return( lSrcIndex );   }
    void    SetLineNo( sal_uInt32 nLine )     { nLineNo = nLine;       }
    sal_uInt32  GetLineNo()                   { return( nLineNo );     }
    sal_uInt32  GetScanPos()                  { return( nScanPos );    }
    char *  GetLine()                     { return( pLine );       }
    int     GetChar();
    int     GetFastChar() { return pLine[ nScanPos ] ?
                                pLine[ nScanPos++ ] : GetChar();
                          }
    void    GetNewLine();
            // Fehlerbehandlung
    void    SetError( ERRTYPE aError );
    ERRTYPE GetError()                    { return aFirstError;    }
    sal_uInt32  GetErrorLine()                { return nErrorLine;     }
    sal_uInt32  GetErrorPos()                 { return nErrorPos;      }
};

/******************* F u n c t i o n *************************************/
void IncludeParser( RscFileInst * pFileInst );
ERRTYPE parser( RscFileInst * pFileInst );

#endif // _RSCPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
