/*************************************************************************
 *
 *  $RCSfile: difexp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:25 $
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

#include <rtl/math.hxx>

#include <stdio.h>

#include "dif.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "progress.hxx"

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

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
    const ScRange&rRange, const CharSet eCharSet, UINT32 nDifOption )
{
    DBG_ASSERT( rRange.aStart <= rRange.aEnd, "*ScExportDif(): Range unsortiert!" );
    DBG_ASSERTWARNING( rRange.aStart.Tab() == rRange.aEnd.Tab(),
        "ScExportDif(): nur eine Tabelle bidde!" );

    const CharSet eStreamCharSet = rOut.GetStreamCharSet();
    if ( eStreamCharSet != eCharSet )
        rOut.SetStreamCharSet( eCharSet );

    sal_Unicode cStrDelim('"');
    ByteString aStrDelimEncoded;    // only used if not Unicode
    UniString aStrDelimDecoded;     // only used if context encoding
    BOOL bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rOut.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = FALSE;
    }
    else
    {
        aStrDelimEncoded = ByteString( cStrDelim, eCharSet );
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
                aStrDelimDecoded = String( aStrDelimEncoded, eCharSet );
        }
        else
            bContextOrNotAsciiEncoding = FALSE;
    }

    const sal_Char*     p2DoubleQuotes_LF = "\"\"\n";
    const sal_Char*     pSpecDataType_LF = "-1,0\n";
    const sal_Char*     pEmptyData = "1,0\n\"\"\n";
    const sal_Char*     pStringData = "1,0\n";
    const sal_Char*     pNumData = "0,";
    const sal_Char*     pNumDataERROR = "0,0\nERROR\n";

    FltError            eRet = eERR_OK;
    String              aOS;
    String              aString;
    UINT16              nEndCol = rRange.aEnd.Col();
    UINT16              nEndRow = rRange.aEnd.Row();
    UINT16              nNumCols = nEndCol - rRange.aStart.Col() + 1;
    UINT16              nNumRows = nEndRow - rRange.aStart.Row() + 1;
    UINT16              nTab = rRange.aStart.Tab();

    double              fVal;

    const BOOL          bPlain = ( nDifOption == SC_DIFOPT_PLAIN );

    ScProgress          aPrgrsBar( pDoc->GetDocumentShell(), ScGlobal::GetRscString( STR_LOAD_DOC ), nNumRows );

    aPrgrsBar.SetState( 0 );

    // TABLE
    DBG_ASSERT( pDoc->HasTable( nTab ), "*ScExportDif(): Tabelle nicht vorhanden!" );

    aOS = pKeyTABLE;
    aOS.AppendAscii( "\n0,1\n\"" );

    pDoc->GetName( nTab, aString );
    aOS += aString;
    aOS.AppendAscii( "\"\n" );
    rOut.WriteUnicodeOrByteText( aOS );

    // VECTORS
    aOS = pKeyVECTORS;
    aOS.AppendAscii( "\n0," );
    aOS += String::CreateFromInt32( nNumCols );
    aOS += sal_Unicode('\n');
    aOS.AppendAscii( p2DoubleQuotes_LF );
    rOut.WriteUnicodeOrByteText( aOS );

    // TUPLES
    aOS = pKeyTUPLES;
    aOS.AppendAscii( "\n0," );
    aOS += String::CreateFromInt32( nNumRows );
    aOS += sal_Unicode('\n');
    aOS.AppendAscii( p2DoubleQuotes_LF );
    rOut.WriteUnicodeOrByteText( aOS );

    // DATA
    aOS = pKeyDATA;
    aOS.AppendAscii( "\n0,0\n" );
    aOS.AppendAscii( p2DoubleQuotes_LF );
    rOut.WriteUnicodeOrByteText( aOS );

    UINT16              nColCnt, nRowCnt;
    ScBaseCell*         pAkt;

    for( nRowCnt = rRange.aStart.Row() ; nRowCnt <= nEndRow ; nRowCnt++ )
    {
        aOS.AssignAscii( pSpecDataType_LF );
        aOS += pKeyBOT;
        aOS += sal_Unicode('\n');
        rOut.WriteUnicodeOrByteText( aOS );
        for( nColCnt = rRange.aStart.Col() ; nColCnt <= nEndCol ; nColCnt++ )
        {
            bool bWriteStringData = false;
            pDoc->GetCell( nColCnt, nRowCnt, nTab, pAkt );
            if( pAkt )
            {
                switch( pAkt->GetCellType() )
                {
                    case CELLTYPE_NONE:
                    case CELLTYPE_NOTE:
                        aOS.AssignAscii( pEmptyData );
                        break;
                    case CELLTYPE_VALUE:
                        aOS.AssignAscii( pNumData );
                        if( bPlain )
                        {
                            fVal = ( ( ScValueCell * ) pAkt )->GetValue();
                            aOS += String( ::rtl::math::doubleToUString(
                                        fVal, rtl_math_StringFormat_G, 14, '.',
                                        TRUE));
                        }
                        else
                        {
                            pDoc->GetInputString( nColCnt, nRowCnt, nTab, aString );
                            aOS += aString;
                        }
                        aOS.AppendAscii( "\nV\n" );
                        break;
                    case CELLTYPE_EDIT:
                        ( ( ScEditCell* ) pAkt )->GetString( aString );
                        bWriteStringData = true;
                        break;
                    case CELLTYPE_STRING:
                        ( ( ScStringCell* ) pAkt )->GetString( aString );
                        bWriteStringData = true;
                        break;
                    case CELLTYPE_FORMULA:
                        if ( ((ScFormulaCell*)pAkt)->GetErrCode() )
                            aOS.AssignAscii( pNumDataERROR );
                        else if( pAkt->HasValueData() )
                        {
                            aOS.AssignAscii( pNumData );
                            if( bPlain )
                            {
                                fVal = ( ( ScFormulaCell * ) pAkt )->GetValue();
                                aOS += String( ::rtl::math::doubleToUString(
                                            fVal, rtl_math_StringFormat_G, 14,
                                            '.', TRUE));
                            }
                            else
                            {
                                pDoc->GetInputString( nColCnt, nRowCnt, nTab, aString );
                                aOS += aString;
                            }
                            aOS.AppendAscii( "\nV\n" );
                        }
                        else if( pAkt->HasStringData() )
                        {
                            ( ( ScFormulaCell * ) pAkt )->GetString( aString );
                            bWriteStringData = true;
                        }
                        else
                            aOS.AssignAscii( pNumDataERROR );

                        break;
                }
            }
            else
                aOS.AssignAscii( pEmptyData );

            if ( !bWriteStringData )
                rOut.WriteUnicodeOrByteText( aOS );
            else
            {
                // for an explanation why this complicated, see
                // sc/source/ui/docsh.cxx:ScDocShell::AsciiSave()
                // In fact we should create a common method if this would be
                // needed just one more time..
                aOS.AssignAscii( pStringData );
                rOut.WriteUnicodeOrByteText( aOS, eCharSet );
                if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                {
                    xub_StrLen nPos = aString.Search( cStrDelim );
                    while ( nPos != STRING_NOTFOUND )
                    {
                        aString.Insert( cStrDelim, nPos );
                        nPos = aString.Search( cStrDelim, nPos+2 );
                    }
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rOut.WriteUnicodeText( aString );
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else if ( bContextOrNotAsciiEncoding )
                {
                    // to byte encoding
                    ByteString aStrEnc( aString, eCharSet );
                    // back to Unicode
                    UniString aStrDec( aStrEnc, eCharSet );
                    // search on re-decoded string
                    xub_StrLen nPos = aStrDec.Search( aStrDelimDecoded );
                    while ( nPos != STRING_NOTFOUND )
                    {
                        aStrDec.Insert( aStrDelimDecoded, nPos );
                        nPos = aStrDec.Search( aStrDelimDecoded,
                                nPos+1+aStrDelimDecoded.Len() );
                    }
                    // write byte re-encoded
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rOut.WriteUnicodeOrByteText( aStrDec, eCharSet );
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else
                {
                    ByteString aStrEnc( aString, eCharSet );
                    // search on encoded string
                    xub_StrLen nPos = aStrEnc.Search( aStrDelimEncoded );
                    while ( nPos != STRING_NOTFOUND )
                    {
                        aStrEnc.Insert( aStrDelimEncoded, nPos );
                        nPos = aStrEnc.Search( aStrDelimEncoded,
                                nPos+1+aStrDelimEncoded.Len() );
                    }
                    // write byte encoded
                    rOut.Write( aStrDelimEncoded.GetBuffer(),
                            aStrDelimEncoded.Len() );
                    rOut.Write( aStrEnc.GetBuffer(), aStrEnc.Len() );
                    rOut.Write( aStrDelimEncoded.GetBuffer(),
                            aStrDelimEncoded.Len() );
                }
                rOut.WriteUniOrByteChar( '\n', eCharSet );
            }
        }
        aPrgrsBar.SetState( nRowCnt );
    }

    aOS.AssignAscii( pSpecDataType_LF );
    aOS += pKeyEOD;
    aOS += sal_Unicode('\n');
    rOut.WriteUnicodeOrByteText( aOS );

    // restore original value
    rOut.SetStreamCharSet( eStreamCharSet );

    return eRet;
}




