/*************************************************************************
 *
 *  $RCSfile: rscpar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/parser/rscpar.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.19  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.18  2000/07/26 17:13:21  willem.vandorp
    Headers/footers replaced

    Revision 1.17  2000/07/12 11:35:51  th
    Unicode

    Revision 1.16  1999/09/07 13:30:04  mm
    UniCode

    Revision 1.15  1997/08/27 18:17:18  MM
    neue Headerstruktur

**************************************************************************/
/****************** I N C L U D E S **************************************/
#pragma hdrstop

// C and C++ Includes.
#include <string.h>

#ifndef _RSCPAR_HXX
#include <rscpar.hxx>
#endif
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif

/****************** R s c F i l e I n s t ********************************/
/****************** C O D E **********************************************/
/*************************************************************************
|*
|*    RscFileInst::Init()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.11.91
|*    Letzte Aenderung  MM 17.02.93
|*
*************************************************************************/
void RscFileInst::Init()
{
    SetCharSet( RTL_TEXTENCODING_MS_1252 );

    nLineNo = 0;
    nLineBufLen = 256;
    pLine = (char *)RscMem::Malloc( nLineBufLen );
    *pLine = '\0';
    nScanPos = 0;
    cLastChar = '\0';
    bEof = FALSE;
};

/*************************************************************************
|*
|*    RscFileInst::RscFileInst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.06.91
|*    Letzte Aenderung  MM 06.06.91
|*
*************************************************************************/
RscFileInst::RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                          ULONG lFIndex, FILE * fFile,
                          rtl_TextEncoding nSourceCharSet )
{
    pTypCont = pTC;
    Init();

    lFileIndex = lFIndex;
    lSrcIndex = lIndexSrc;
    fInputFile = fFile;

    SetCharSet( nSourceCharSet );

    //Status: Zeiger am Ende des Lesepuffers
    nInputPos = nInputEndPos = nInputBufLen = READBUFFER_MAX;
    pInput    = (char *)RscMem::Malloc( nInputBufLen );
}

RscFileInst::RscFileInst( RscTypCont * pTC, ULONG lIndexSrc,
                          ULONG lFIndex, const ByteString& rBuf,
                          rtl_TextEncoding nSourceCharSet )
{
    pTypCont     = pTC;
    Init();
    lFileIndex   = lFIndex;
    lSrcIndex    = lIndexSrc;
    fInputFile   = NULL;
    nInputPos    = 0;
    nInputEndPos = rBuf.Len();

    SetCharSet( nSourceCharSet );

    // Muss groesser sein wegen Eingabeende bei nInputBufLen < nInputEndPos
    nInputBufLen = nInputEndPos +1;
    pInput       = (char *)RscMem::Malloc( nInputBufLen +100 );
    memcpy( pInput, rBuf.GetBuffer(), nInputEndPos );
}

/*************************************************************************
|*
|*    RscFileInst::~RscFileInst()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.06.91
|*    Letzte Aenderung  MM 06.06.91
|*
*************************************************************************/
RscFileInst::~RscFileInst(){
    if( pInput )
        RscMem::Free( pInput );
    if( pLine )
        RscMem::Free( pLine );
}

/*************************************************************************
|*
|*    RscFileInst::GetChar()
|*
|*    Beschreibung
|*    Ersterstellung    MM 01.06.91
|*    Letzte Aenderung  MM 09.08.91
|*
*************************************************************************/
int RscFileInst::GetChar()
{
    if( pLine[ nScanPos ] )
        return( pLine[ nScanPos++ ] );
    else if( nInputPos >= nInputEndPos && nInputEndPos != nInputBufLen )
    {
        // Dateiende
        bEof = TRUE;
        return 0;
    }
    else
    {
        GetNewLine();
        return( '\n' );
    }
}

/*************************************************************************
|*
|*    RscFileInst::GetNewLine()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.06.91
|*    Letzte Aenderung  MM 06.06.91
|*
*************************************************************************/
void RscFileInst::GetNewLine()
{
    nLineNo++;
    nScanPos = 0;

    //laeuft bis Dateiende
    USHORT nLen = 0;
    while( (nInputPos < nInputEndPos) || (nInputEndPos == nInputBufLen) )
    {
        if( (nInputPos >= nInputEndPos) && fInputFile )
        {
            nInputEndPos = fread( pInput, 1, nInputBufLen, fInputFile );
            nInputPos = 0;
        }

        while( nInputPos < nInputEndPos )
        {
            //immer eine Zeile lesen
            if( nLen >= nLineBufLen )
            {
                nLineBufLen += 256;
                // einen dazu fuer '\0'
                pLine = RscMem::Realloc( pLine, nLineBufLen +1 );
            }

            // cr lf, lf cr, lf oder cr wird '\0'
            if( pInput[ nInputPos ] == '\n' ){
                nInputPos++;
                if( cLastChar != '\r' ){
                    cLastChar = '\n';
                    pLine[ nLen++ ] = '\0';
                    goto END;
                }
            }
            else if( pInput[ nInputPos ] == '\r' ){
                nInputPos++;
                if( cLastChar != '\n' ){
                    cLastChar = '\r';
                    pLine[ nLen++ ] = '\0';
                    goto END;
                }
            }
            else
                pLine[ nLen++ ] = pInput[ nInputPos++ ];
        };
    };

    // Abbruch ueber EOF
    pLine[ nLen ] = '\0';

END:
    if( pTypCont->pEH->GetListFile() ){
        char buf[ 10 ];

        sprintf( buf, "%5d ", GetLineNo() );
        pTypCont->pEH->LstOut( buf );
        pTypCont->pEH->LstOut( GetLine() );
        pTypCont->pEH->LstOut( "\n" );
    }
}

/*************************************************************************
|*
|*    RscFileInst::SetError()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.11.91
|*    Letzte Aenderung  MM 05.11.91
|*
*************************************************************************/
void RscFileInst::SetError( ERRTYPE aError )
{
    if( aError.IsOk() )
    {
        aFirstError = aError;
        nErrorLine  = GetLineNo();
        nErrorPos   = GetScanPos() -1;
    };
};
