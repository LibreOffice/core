/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscpar.hxx,v $
 * $Revision: 1.6 $
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
    BOOL                bIncLine;   // Muss Zeilennummer incrementiert werden
    sal_uInt32              nLineNo;    // Zeile in der Eingabedatei
    ULONG               lFileIndex; // Index auf Eingabedatei
    ULONG               lSrcIndex;  // Index auf Basisdatei
    FILE *              fInputFile; // Eingabedatei
    char *              pInput;     // Lesepuffer
    sal_uInt32              nInputBufLen; // Laenge des Lesepuffers
    sal_uInt32              nInputPos;  // Position im Lesepuffer
    sal_uInt32              nInputEndPos;// Ende im Lesepuffer
    char *              pLine;      // Zeile
    sal_uInt32              nLineBufLen;//Lange des Zeilenpuffres
    sal_uInt32              nScanPos;   // Position in der Zeile
    int                 cLastChar;
    BOOL                bEof;

public:
    RscTypCont *        pTypCont;
    void    Init();  // ctor initialisieren
            RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                         ULONG lFileIndex, FILE * fFile );
            RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                         ULONG lFileIndex, const ByteString & );
            ~RscFileInst();
    BOOL    IsEof() const { return bEof; }
    void    SetFileIndex( ULONG lFIndex ) { lFileIndex = lFIndex;  }
    ULONG   GetFileIndex()                { return( lFileIndex );  }
    ULONG   GetSrcIndex()                 { return( lSrcIndex );   }
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
