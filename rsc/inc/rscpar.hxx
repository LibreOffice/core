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
#ifndef _RSCPAR_HXX
#define _RSCPAR_HXX

#include <rsctools.hxx>
#include <rscerror.h>

/****************** C L A S S E S ****************************************/
class RscTypCont;
class RscExpression;
/*********** R s c F i l e I n s t ***************************************/

#define READBUFFER_MAX  256
class RscFileInst
{
    ERRTYPE             aFirstError;// Erster Fehler
    sal_uInt32              nErrorLine; // Zeile des ersten Fehlers
    sal_uInt32              nErrorPos;  // Position des ersten Fehlers
    sal_Bool                bIncLine;   // Muss Zeilennummer incrementiert werden
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
            RscFileInst( RscTypCont * pTC, sal_uLong lIndexSrc,
                         sal_uLong lFileIndex, const ByteString & );
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
RscExpression * MacroParser( RscFileInst & rFileInst );

#endif // _RSCPAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
