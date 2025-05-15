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

#include <dif.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <formulacell.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <progress.hxx>
#include <rtl/tencinfo.h>
#include <ftools.hxx>
#include <cellvalue.hxx>
#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <formula/errorcodes.hxx>
#include <tools/stream.hxx>

void ScFormatFilterPluginImpl::ScExportDif( SvStream& rStream, ScDocument& rDoc,
    const ScAddress& rOutPos, const rtl_TextEncoding eNach )
{
    SCCOL       nEndCol;
    SCROW       nEndRow;
    rDoc.GetTableArea( rOutPos.Tab(), nEndCol, nEndRow );
    ScAddress   aEnd( nEndCol, nEndRow, rOutPos.Tab() );
    ScAddress   aStart( rOutPos );

    aStart.PutInOrder( aEnd );

    ScExportDif( rStream, rDoc, ScRange( aStart, aEnd ), eNach );
}

void ScFormatFilterPluginImpl::ScExportDif( SvStream& rOut, ScDocument& rDoc,
    const ScRange&rRange, const rtl_TextEncoding eCharSet )
{
    OSL_ENSURE( rRange.aStart <= rRange.aEnd, "*ScExportDif(): Range not sorted!" );
    OSL_ENSURE( rRange.aStart.Tab() == rRange.aEnd.Tab(),
        "ScExportDif(): only one table please!" );

    const rtl_TextEncoding eStreamCharSet = rOut.GetStreamCharSet();
    if ( eStreamCharSet != eCharSet )
        rOut.SetStreamCharSet( eCharSet );

    sal_Unicode cStrDelim('"');
    OString aStrDelimEncoded;    // only used if not Unicode
    OUString aStrDelimDecoded;     // only used if context encoding
    bool bContextOrNotAsciiEncoding;
    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
        rOut.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = false;
    }
    else
    {
        aStrDelimEncoded = OString(&cStrDelim, 1, eCharSet);
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
                aStrDelimDecoded = OStringToOUString(aStrDelimEncoded, eCharSet);
        }
        else
            bContextOrNotAsciiEncoding = false;
    }

    const char p2DoubleQuotes_LF[] = "\"\"\n";
    const char pSpecDataType_LF[] = "-1,0\n";
    const char pEmptyData[] = "1,0\n\"\"\n";
    const char pStringData[] = "1,0\n";
    const char pNumData[] = "0,";
    const char pNumDataERROR[] = "0,0\nERROR\n";

    OUStringBuffer aOS;
    OUString       aString;
    SCCOL               nEndCol = rRange.aEnd.Col();
    SCROW               nEndRow = rRange.aEnd.Row();
    SCCOL               nNumCols = nEndCol - rRange.aStart.Col() + 1;
    SCROW               nNumRows = nEndRow - rRange.aStart.Row() + 1;
    SCTAB               nTab = rRange.aStart.Tab();

    ScProgress          aPrgrsBar( rDoc.GetDocumentShell(), ScResId( STR_LOAD_DOC ), nNumRows, true );

    aPrgrsBar.SetState( 0 );

    // TABLE
    OSL_ENSURE( rDoc.HasTable( nTab ), "*ScExportDif(): Table not existent!" );

    rDoc.GetName( nTab, aString );
    aOS.append(OUString::Concat(pKeyTABLE)
        + "\n0,1\n\""
        + aString
        + "\"\n");
    rOut.WriteUnicodeOrByteText(aOS);
    aOS.setLength(0);

    // VECTORS
    aOS.append(OUString::Concat(pKeyVECTORS)
        + "\n0,"
        + OUString::number(static_cast<sal_Int32>(nNumCols))
        + "\n"
        + p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS);
    aOS.setLength(0);

    // TUPLES
    aOS.append(OUString::Concat(pKeyTUPLES)
        + "\n0,"
        + OUString::number(static_cast<sal_Int32>(nNumRows))
        + "\n"
        + p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS);
    aOS.setLength(0);

    // DATA
    aOS.append(OUString::Concat(pKeyDATA)
        + "\n0,0\n"
        + p2DoubleQuotes_LF);
    rOut.WriteUnicodeOrByteText(aOS);
    aOS.setLength(0);

    SCCOL               nColCnt;
    SCROW               nRowCnt;

    for( nRowCnt = rRange.aStart.Row() ; nRowCnt <= nEndRow ; nRowCnt++ )
    {
        assert( aOS.isEmpty() && "aOS should be empty");
        aOS.append(OUString::Concat(pSpecDataType_LF)
            + pKeyBOT
            + "\n");
        rOut.WriteUnicodeOrByteText(aOS);
        aOS.setLength(0);
        for( nColCnt = rRange.aStart.Col() ; nColCnt <= nEndCol ; nColCnt++ )
        {
            assert( aOS.isEmpty() && "aOS should be empty");
            bool bWriteStringData = false;
            ScRefCellValue aCell(rDoc, ScAddress(nColCnt, nRowCnt, nTab));

            switch (aCell.getType())
            {
                case CELLTYPE_NONE:
                    aOS.append(pEmptyData);
                break;
                case CELLTYPE_VALUE:
                    aString = rDoc.GetInputString( nColCnt, nRowCnt, nTab );
                    aOS.append(pNumData + aString + "\nV\n");
                break;
                case CELLTYPE_EDIT:
                case CELLTYPE_STRING:
                    aString = aCell.getString(rDoc);
                    bWriteStringData = true;
                break;
                case CELLTYPE_FORMULA:
                    if (aCell.getFormula()->GetErrCode() != FormulaError::NONE)
                        aOS.append(pNumDataERROR);
                    else if (aCell.getFormula()->IsValue())
                    {
                        aString = rDoc.GetInputString( nColCnt, nRowCnt, nTab );
                        aOS.append(pNumData + aString + "\nV\n");
                    }
                    else
                    {
                        aString = aCell.getFormula()->GetString().getString();
                        bWriteStringData = true;
                    }

                break;
                default:;
            }

            if ( !bWriteStringData )
            {
                rOut.WriteUnicodeOrByteText(aOS);
                aOS.setLength(0);
            }
            else
            {
                // for an explanation why this complicated, see
                // sc/source/ui/docshell/docsh.cxx:ScDocShell::AsciiSave()
                // In fact we should create a common method if this would be
                // needed just one more time...
                assert( aOS.isEmpty() && "aOS should be empty");
                OUString aTmpStr = aString;
                aOS.append(pStringData);
                rOut.WriteUnicodeOrByteText(aOS, eCharSet);
                aOS.setLength(0);
                if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                {
                    // the goal is to replace cStrDelim by cStrDelim+cStrDelim
                    OUString strFrom(cStrDelim);
                    OUString strTo = strFrom + strFrom;
                    aTmpStr = aTmpStr.replaceAll(strFrom, strTo);
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rOut.WriteUnicodeOrByteText(aTmpStr, eCharSet);
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else if ( bContextOrNotAsciiEncoding )
                {
                    // to byte encoding
                    OString aStrEnc = OUStringToOString(aTmpStr, eCharSet);
                    // back to Unicode
                    OUString aStrDec = OStringToOUString(aStrEnc, eCharSet);
                    // search on re-decoded string
                    OUString aStrTo = aStrDelimDecoded + aStrDelimDecoded;
                    aStrDec = aStrDec.replaceAll(aStrDelimDecoded, aStrTo);
                    // write byte re-encoded
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                    rOut.WriteUnicodeOrByteText( aStrDec, eCharSet );
                    rOut.WriteUniOrByteChar( cStrDelim, eCharSet );
                }
                else
                {
                    OString aStrEnc = OUStringToOString(aTmpStr, eCharSet);
                    // search on encoded string
                    OString aStrTo = aStrDelimEncoded + aStrDelimEncoded;
                    aStrEnc = aStrEnc.replaceAll(aStrDelimEncoded, aStrTo);
                    // write byte encoded
                    rOut.WriteBytes(aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                    rOut.WriteBytes(aStrEnc.getStr(), aStrEnc.getLength());
                    rOut.WriteBytes(aStrDelimEncoded.getStr(), aStrDelimEncoded.getLength());
                }
                rOut.WriteUniOrByteChar( '\n', eCharSet );
            }
        }
        aPrgrsBar.SetState( nRowCnt );
    }

    assert( aOS.isEmpty() && "aOS should be empty");
    aOS.append(OUString::Concat(pSpecDataType_LF)
        + pKeyEOD
        + "\n");
    rOut.WriteUnicodeOrByteText(aOS);
    aOS.setLength(0);

    // restore original value
    rOut.SetStreamCharSet( eStreamCharSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
