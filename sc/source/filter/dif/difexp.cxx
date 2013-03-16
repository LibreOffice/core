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

#include <rtl/math.hxx>

#include <stdio.h>

#include "dif.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "progress.hxx"
#include <rtl/tencinfo.h>
#include "ftools.hxx"
#include "rtl/strbuf.hxx"

FltError ScFormatFilterPluginImpl::ScExportDif( SvStream& rStream, ScDocument* pDoc,
    const ScAddress& rOutPos, const CharSet eNach, sal_uInt32 nDifOption )
{
    SCCOL       nEndCol;
    SCROW       nEndRow;
    pDoc->GetTableArea( rOutPos.Tab(), nEndCol, nEndRow );
    ScAddress   aEnd( nEndCol, nEndRow, rOutPos.Tab() );
    ScAddress   aStart( rOutPos );

    aStart.PutInOrder( aEnd );

    return ScExportDif( rStream, pDoc, ScRange( aStart, aEnd ), eNach, nDifOption );
}


FltError ScFormatFilterPluginImpl::ScExportDif( SvStream& rOut, ScDocument* pDoc,
    const ScRange&rRange, const CharSet eCharSet, sal_uInt32 nDifOption )
{
    OSL_ENSURE( rRange.aStart <= rRange.aEnd, "*ScExportDif(): Range not sorted!" );
    OSL_ENSURE( rRange.aStart.Tab() == rRange.aEnd.Tab(),
        "ScExportDif(): only one table please!" );

    const CharSet eStreamCharSet = rOut.GetStreamCharSet();
    if ( eStreamCharSet != eCharSet )
        rOut.SetStreamCharSet( eCharSet );

    sal_Unicode cStrDelim('"');
    rtl::OString aStrDelimEncoded;    // only used if not Unicode
    rtl::OUString aStrDelimDecoded;     // only used if context encoding
    bool bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rOut.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = false;
    }
    else
    {
        aStrDelimEncoded = rtl::OString(&cStrDelim, 1, eCharSet);
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
                aStrDelimDecoded = rtl::OStringToOUString(aStrDelimEncoded, eCharSet);
        }
        else
            bContextOrNotAsciiEncoding = false;
    }

    const sal_Char*     p2DoubleQuotes_LF = "\"\"\n";
    const sal_Char*     pSpecDataType_LF = "-1,0\n";
    const sal_Char*     pEmptyData = "1,0\n\"\"\n";
    const sal_Char*     pStringData = "1,0\n";
    const sal_Char*     pNumData = "0,";
    const sal_Char*     pNumDataERROR = "0,0\nERROR\n";

    FltError            eRet = eERR_OK;
    rtl::OUStringBuffer aOS;
    rtl::OUString       aString;
    SCCOL               nEndCol = rRange.aEnd.Col();
    SCROW               nEndRow = rRange.aEnd.Row();
    SCCOL               nNumCols = nEndCol - rRange.aStart.Col() + 1;
    SCROW               nNumRows = nEndRow - rRange.aStart.Row() + 1;
    SCTAB               nTab = rRange.aStart.Tab();

    double              fVal;

    const bool bPlain = ( nDifOption == SC_DIFOPT_PLAIN );

    ScProgress          aPrgrsBar( pDoc->GetDocumentShell(), ScGlobal::GetRscString( STR_LOAD_DOC ), nNumRows );

    aPrgrsBar.SetState( 0 );

    // TABLE
    OSL_ENSURE( pDoc->HasTable( nTab ), "*ScExportDif(): Table not existent!" );

    aOS.append(pKeyTABLE);
    aOS.appendAscii("\n0,1\n\"");

    pDoc->GetName( nTab, aString );
    aOS.append(aString);
    aOS.appendAscii("\"\n");
    rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());

    // VECTORS
    aOS.append(pKeyVECTORS);
    aOS.appendAscii("\n0,");
    aOS.append(static_cast<sal_Int32>(nNumCols));
    aOS.append(sal_Unicode('\n'));
    aOS.appendAscii(p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());

    // TUPLES
    aOS.append(pKeyTUPLES);
    aOS.appendAscii("\n0,");
    aOS.append(static_cast<sal_Int32>(nNumRows));
    aOS.append(sal_Unicode('\n'));
    aOS.appendAscii(p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());

    // DATA
    aOS.append(pKeyDATA);
    aOS.appendAscii("\n0,0\n");
    aOS.appendAscii(p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());

    SCCOL               nColCnt;
    SCROW               nRowCnt;
    ScBaseCell*         pAkt;

    for( nRowCnt = rRange.aStart.Row() ; nRowCnt <= nEndRow ; nRowCnt++ )
    {
        OSL_ASSERT(aOS.getLength() == 0);
        aOS.appendAscii(pSpecDataType_LF);
        aOS.append(pKeyBOT);
        aOS.append(sal_Unicode('\n'));
        rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());
        for( nColCnt = rRange.aStart.Col() ; nColCnt <= nEndCol ; nColCnt++ )
        {
            OSL_ASSERT(aOS.getLength() == 0);
            bool bWriteStringData = false;
            pDoc->GetCell( nColCnt, nRowCnt, nTab, pAkt );
            if( pAkt )
            {
                switch( pAkt->GetCellType() )
                {
                    case CELLTYPE_NONE:
                    case CELLTYPE_NOTE:
                        aOS.appendAscii(pEmptyData);
                        break;
                    case CELLTYPE_VALUE:
                        aOS.appendAscii(pNumData);
                        if( bPlain )
                        {
                            fVal = static_cast<ScValueCell*>(pAkt)->GetValue();
                            aOS.append(
                                rtl::math::doubleToUString(
                                    fVal, rtl_math_StringFormat_G, 14, '.', true));
                        }
                        else
                        {
                            pDoc->GetInputString( nColCnt, nRowCnt, nTab, aString );
                            aOS.append(aString);
                        }
                        aOS.appendAscii("\nV\n");
                        break;
                    case CELLTYPE_EDIT:
                        aString = static_cast<ScEditCell*>(pAkt)->GetString();
                        bWriteStringData = true;
                        break;
                    case CELLTYPE_STRING:
                        aString = static_cast<ScStringCell*>(pAkt)->GetString();
                        bWriteStringData = true;
                        break;
                    case CELLTYPE_FORMULA:
                        if (static_cast<ScFormulaCell*>(pAkt)->GetErrCode())
                            aOS.appendAscii(pNumDataERROR);
                        else if( pAkt->HasValueData() )
                        {
                            aOS.appendAscii(pNumData);
                            if( bPlain )
                            {
                                fVal = static_cast<ScFormulaCell*>(pAkt)->GetValue();
                                aOS.append(
                                    rtl::math::doubleToUString(
                                        fVal, rtl_math_StringFormat_G, 14, '.', true));
                            }
                            else
                            {
                                pDoc->GetInputString( nColCnt, nRowCnt, nTab, aString );
                                aOS.append(aString);
                            }
                            aOS.appendAscii("\nV\n");
                        }
                        else if( pAkt->HasStringData() )
                        {
                            aString = static_cast<ScFormulaCell*>(pAkt)->GetString();
                            bWriteStringData = true;
                        }
                        else
                            aOS.appendAscii(pNumDataERROR);

                        break;
                    default:;
                }
            }
            else
                aOS.appendAscii(pEmptyData);

            if ( !bWriteStringData )
                rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());
            else
            {
                // for an explanation why this complicated, see
                // sc/source/ui/docsh.cxx:ScDocShell::AsciiSave()
                // In fact we should create a common method if this would be
                // needed just one more time..
                OSL_ASSERT(aOS.getLength() == 0);
                String aTmpStr = aString;
                aOS.appendAscii(pStringData);
                rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear(), eCharSet);
                if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                {
                    xub_StrLen nPos = aTmpStr.Search( cStrDelim );
                    while ( nPos != STRING_NOTFOUND )
                    {
                        aTmpStr.Insert( cStrDelim, nPos );
                        nPos = aTmpStr.Search( cStrDelim, nPos+2 );
                    }
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    write_uInt16s_FromOUString(rOut, aTmpStr);
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else if ( bContextOrNotAsciiEncoding )
                {
                    // to byte encoding
                    rtl::OString aStrEnc = rtl::OUStringToOString(aTmpStr, eCharSet);
                    // back to Unicode
                    rtl::OUString aStrDec = rtl::OStringToOUString(aStrEnc, eCharSet);
                    // search on re-decoded string
                    sal_Int32 nPos = aStrDec.indexOf(aStrDelimDecoded);
                    while (nPos >= 0)
                    {
                        rtl::OUStringBuffer aBuf(aStrDec);
                        aBuf.insert(nPos, aStrDelimDecoded);
                        aStrDec = aBuf.makeStringAndClear();
                        nPos = aStrDec.indexOf(
                            aStrDelimDecoded, nPos+1+aStrDelimDecoded.getLength());
                    }
                    // write byte re-encoded
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rOut.WriteUnicodeOrByteText( aStrDec, eCharSet );
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else
                {
                    rtl::OString aStrEnc = rtl::OUStringToOString(aTmpStr, eCharSet);
                    // search on encoded string
                    sal_Int32 nPos = aStrEnc.indexOf(aStrDelimEncoded);
                    while (nPos >= 0)
                    {
                        rtl::OStringBuffer aBuf(aStrEnc);
                        aBuf.insert(nPos, aStrDelimEncoded);
                        aStrEnc = aBuf.makeStringAndClear();
                        nPos = aStrEnc.indexOf(
                            aStrDelimEncoded, nPos+1+aStrDelimEncoded.getLength());
                    }
                    // write byte encoded
                    rOut.Write(aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                    rOut.Write(aStrEnc.getStr(), aStrEnc.getLength());
                    rOut.Write(aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                }
                rOut.WriteUniOrByteChar( '\n', eCharSet );
            }
        }
        aPrgrsBar.SetState( nRowCnt );
    }

    OSL_ASSERT(aOS.getLength() == 0);
    aOS.appendAscii(pSpecDataType_LF);
    aOS.append(pKeyEOD);
    aOS.append(sal_Unicode('\n'));
    rOut.WriteUnicodeOrByteText(aOS.makeStringAndClear());

    // restore original value
    rOut.SetStreamCharSet( eStreamCharSet );

    return eRet;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
