/*************************************************************************
 *
 *  $RCSfile: difexp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:11 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <stdio.h>

#include "dif.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "progress.hxx"


FltError ScExportDif( SvStream& rStream, ScDocument* pDoc,
    const ScAddress& rOutPos, const CharSet eNach, UINT32 nDifOption )
{
    UINT16      nEndCol, nEndRow;
    pDoc->GetTableArea( rOutPos.Tab(), nEndCol, nEndRow );
    ScAddress   aEnd( nEndCol, nEndRow, rOutPos.Tab() );
    ScAddress   aStart( rOutPos );

    aStart.PutInOrder( aEnd );

    return ScExportDif( rStream, pDoc, ScRange( aStart, aEnd ), eNach, nDifOption );
}


FltError ScExportDif( SvStream& rOut, ScDocument* pDoc,
    const ScRange&rRange, const CharSet eNach, UINT32 nDifOption )
{
    DBG_ASSERT( rRange.aStart <= rRange.aEnd, "*ScExportDif(): Range unsortiert!" );
    DBG_ASSERTWARNING( rRange.aStart.Tab() == rRange.aEnd.Tab(),
        "ScExportDif(): nur eine Tabelle bidde!" );

    const sal_Char*     p2DoubleQuotes_LF = "\"\"\n";
    const sal_Char*     pSpecDataType_LF = "-1,0\n";
    const sal_Char*     pEmptyData = "1,0\n\"\"\n";
    const sal_Char*     pStringData = "1,0\n\"";
    const sal_Char*     pNumData = "0,";
    const sal_Char*     pNumDataERROR = "0,0\nERROR\n";

    FltError            eRet = eERR_OK;
    ByteString          aTmp;
    ByteString          aOS;
    String              aUniString;
    UINT16              nEndCol = rRange.aEnd.Col();
    UINT16              nEndRow = rRange.aEnd.Row();
    UINT16              nNumCols = nEndCol - rRange.aStart.Col() + 1;
    UINT16              nNumRows = nEndRow - rRange.aStart.Row() + 1;
    UINT16              nTab = rRange.aStart.Tab();

    double              fVal;
    sal_Char*           pBuffer = new sal_Char[ 256 ];

    const BOOL          bPlain = ( nDifOption == SC_DIFOPT_PLAIN );

    ScProgress          aPrgrsBar( NULL, ScGlobal::GetRscString( STR_LOAD_DOC ), nNumRows );

    aPrgrsBar.SetState( 0 );

    // TABLE
    DBG_ASSERT( pDoc->HasTable( nTab ), "*ScExportDif(): Tabelle nicht vorhanden!" );

    aOS = pKeyTABLE;
    aOS += "\n0,1\n\"";

    pDoc->GetName( nTab, aUniString );
    aOS += ByteString( aUniString, eNach );
    aOS += "\"\n";
    rOut.Write( aOS.GetBuffer(), aOS.Len() );

    // VECTORS
    aOS = pKeyVECTORS;
    aOS += "\n0,";
    aOS += ByteString::CreateFromInt32( nNumCols );
    aOS += '\n';
    aOS += p2DoubleQuotes_LF;
    rOut.Write( aOS.GetBuffer(), aOS.Len() );

    // TUPLES
    aOS = pKeyTUPLES;
    aOS += "\n0,";
    aOS += ByteString::CreateFromInt32( nNumRows );
    aOS += '\n';
    aOS += p2DoubleQuotes_LF;
    rOut.Write( aOS.GetBuffer(), aOS.Len() );

    // DATA
    rOut << pKeyDATA << "\n0,0\n" << p2DoubleQuotes_LF;

    UINT16              nColCnt, nRowCnt;
    ScBaseCell*         pAkt;
    const sal_Char*     pOutString;

    for( nRowCnt = rRange.aStart.Row() ; nRowCnt <= nEndRow ; nRowCnt++ )
    {
        rOut << pSpecDataType_LF << pKeyBOT << '\n';
        for( nColCnt = rRange.aStart.Col() ; nColCnt <= nEndCol ; nColCnt++ )
        {
            pDoc->GetCell( nColCnt, nRowCnt, nTab, pAkt );
            if( pAkt )
            {
                switch( pAkt->GetCellType() )
                {
                    case CELLTYPE_NONE:
                    case CELLTYPE_NOTE:
                        pOutString = pEmptyData;
                        break;
                    case CELLTYPE_VALUE:
                        aOS = pNumData;
                        if( bPlain )
                        {
                            fVal = ( ( ScValueCell * ) pAkt )->GetValue();
                            sprintf( pBuffer, "%.14G", fVal );
                            aOS += pBuffer;
                        }
                        else
                        {
                            pDoc->GetInputString( nColCnt, nRowCnt, nTab, aUniString );
                            aOS += ByteString( aUniString, eNach );
                        }
                        aOS += "\nV\n";
                        pOutString = aOS.GetBuffer();
                        break;
                    case CELLTYPE_EDIT:
                        aOS = pStringData;
                        ( ( ScEditCell* ) pAkt )->GetString( aUniString );
                        aOS += ByteString( aUniString, eNach );
                        aOS += "\"\n";
                        pOutString = aOS.GetBuffer();
                        break;
                    case CELLTYPE_STRING:
                        aOS = pStringData;
                        ( ( ScStringCell* ) pAkt )->GetString( aUniString );
                        aOS += ByteString( aUniString, eNach );
                        aOS += "\"\n";
                        pOutString = aOS.GetBuffer();
                        break;
                    case CELLTYPE_FORMULA:
                        if( pAkt->HasValueData() )
                        {
                            aOS = pNumData;
                            if( bPlain )
                            {
                                fVal = ( ( ScFormulaCell * ) pAkt )->GetValue();
                                sprintf( pBuffer, "%.14G", fVal );
                                aOS += pBuffer;
                            }
                            else
                            {
                                pDoc->GetInputString( nColCnt, nRowCnt, nTab, aUniString );
                                aOS += ByteString( aUniString, eNach );
                            }
                            aOS += "\nV\n";
                            pOutString = aOS.GetBuffer();
                        }
                        else if( pAkt->HasStringData() )
                        {
                            aOS = pStringData;
                            ( ( ScFormulaCell * ) pAkt )->GetString( aUniString );
                            aOS += ByteString( aUniString, eNach );
                            aOS += "\"\n";
                            pOutString = aOS.GetBuffer();
                        }
                        else
                            pOutString = pNumDataERROR;

                        break;
                }
            }
            else
                pOutString = pEmptyData;

            rOut << pOutString;
        }
        aPrgrsBar.SetState( nRowCnt );
    }

    rOut << pSpecDataType_LF << pKeyEOD << '\n';

    delete[] pBuffer;

    return eRet;
}




