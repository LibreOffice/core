/*************************************************************************
 *
 *  $RCSfile: rscpar.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscpar.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

**************************************************************************/
#ifndef _RSCPAR_HXX
#define _RSCPAR_HXX

#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif

/****************** C L A S S E S ****************************************/
class RscTypCont;
class RscExpression;
/*********** R s c F i l e I n s t ***************************************/

#define READBUFFER_MAX  256
class RscFileInst
{
    ERRTYPE             aFirstError;// Erster Fehler
    USHORT              nErrorLine; // Zeile des ersten Fehlers
    USHORT              nErrorPos;  // Position des ersten Fehlers
    rtl_TextEncoding    nCharSet;   // Zeichensatz der einzulesenden Datei
    BOOL                bIncLine;   // Muss Zeilennummer incrementiert werden
    USHORT              nLineNo;    // Zeile in der Eingabedatei
    ULONG               lFileIndex; // Index auf Eingabedatei
    ULONG               lSrcIndex;  // Index auf Basisdatei
    FILE *              fInputFile; // Eingabedatei
    char *              pInput;     // Lesepuffer
    USHORT              nInputBufLen; // Laenge des Lesepuffers
    USHORT              nInputPos;  // Position im Lesepuffer
    USHORT              nInputEndPos;// Ende im Lesepuffer
    char *              pLine;      // Zeile
    USHORT              nLineBufLen;//Lange des Zeilenpuffres
    USHORT              nScanPos;   // Position in der Zeile
    int                 cLastChar;
    BOOL                bEof;

public:
    RscTypCont *        pTypCont;
    void    Init();  // ctor initialisieren
            RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                         ULONG lFileIndex, FILE * fFile,
                         rtl_TextEncoding nSourceCharSet );
            RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                         ULONG lFileIndex, const ByteString &,
                         rtl_TextEncoding nSourceCharSet );
            ~RscFileInst();
    BOOL    IsEof() const { return bEof; }
    void    SetCharSet( rtl_TextEncoding nSet ) { nCharSet = nSet; }
    UINT16  GetCharSet() const { return (UINT16)nCharSet; }
    void    SetFileIndex( ULONG lFIndex ) { lFileIndex = lFIndex;  }
    ULONG   GetFileIndex()                { return( lFileIndex );  }
    ULONG   GetSrcIndex()                 { return( lSrcIndex );   }
    void    SetLineNo( USHORT nLine )     { nLineNo = nLine;       }
    USHORT  GetLineNo()                   { return( nLineNo );     }
    USHORT  GetScanPos()                  { return( nScanPos );    }
    char *  GetLine()                     { return( pLine );       }
    int     GetChar();
    int     GetFastChar() { return pLine[ nScanPos ] ?
                                pLine[ nScanPos++ ] : GetChar();
                          }
    void    GetNewLine();
            // Fehlerbehandlung
    void    SetError( ERRTYPE aError );
    ERRTYPE GetError()                    { return aFirstError;    }
    USHORT  GetErrorLine()                { return nErrorLine;     }
    USHORT  GetErrorPos()                 { return nErrorPos;      }
};

/******************* F u n c t i o n *************************************/
void IncludeParser( RscFileInst * pFileInst );
ERRTYPE parser( RscFileInst * pFileInst );
RscExpression * MacroParser( RscFileInst & rFileInst );

#endif // _RSCPAR_HXX
