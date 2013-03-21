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


#include <sal/config.h>
#include <stdio.h>
#include <sfx2/docfile.hxx>

#include "qproform.hxx"
#include "qpro.hxx"
#include "qprostyle.hxx"

#include "global.hxx"
#include "scerrors.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "biff.hxx"
#include <tools/stream.hxx>

FltError ScQProReader::readSheet( SCTAB nTab, ScDocument* pDoc, ScQProStyle *pStyle )
{
    FltError eRet = eERR_OK;
    sal_uInt8  nCol, nDummy;
    sal_uInt16 nRow;
    sal_uInt16 nStyle;
    bool bEndOfSheet = false;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Read sheet (%d)\n", nTab );
#endif

    while( eERR_OK == eRet && !bEndOfSheet && nextRecord() )
    {
        switch( getId() )
        {
            case 0x000f:{ // Label cell
                String aLabel;
                *mpStream >> nCol >> nDummy >> nRow >> nStyle >> nDummy;
                sal_uInt16 nLen = getLength();
                if (nLen >= 7)
                {
                    readString( aLabel, nLen - 7 );
                    nStyle = nStyle >> 3;
                    pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                    pDoc->EnsureTable(nTab);
                    pDoc->SetTextCell(ScAddress(nCol,nRow,nTab), aLabel);
                }
                else
                    eRet = eERR_FORMAT;
                }
                break;

            case 0x00cb: // End of sheet
                bEndOfSheet = true;
                break;

            case 0x000c: // Blank cell
                *mpStream >> nCol >> nDummy >> nRow >> nStyle;
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                break;

            case 0x000d:{ // Integer cell
                sal_Int16 nValue;
                *mpStream >> nCol >> nDummy >> nRow >> nStyle >> nValue;
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->EnsureTable(nTab);
                pDoc->SetValue(ScAddress(nCol,nRow,nTab), static_cast<double>(nValue));
                }
                break;

            case 0x000e:{ // Floating point cell
                double nValue;
                *mpStream >> nCol >> nDummy >> nRow >> nStyle >> nValue;
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->EnsureTable(nTab);
                pDoc->SetValue(ScAddress(nCol,nRow,nTab), nValue);
                }
                break;

            case 0x0010:{ // Formula cell
                double nValue;
                sal_uInt16 nState, nLen;
                *mpStream >> nCol >> nDummy >> nRow >> nStyle >> nValue >> nState >> nLen;
                ScAddress aAddr( nCol, nRow, nTab );
                const ScTokenArray *pArray;
                QProToSc aConv( *mpStream, aAddr );
                if (ConvOK != aConv.Convert( pArray, nLen ))
                    eRet = eERR_FORMAT;
                else
                {
                    ScFormulaCell *pFormula = new ScFormulaCell( pDoc, aAddr, pArray );
                    nStyle = nStyle >> 3;
                    pFormula->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );
                    pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                    pDoc->EnsureTable(nTab);
                    pDoc->SetFormulaCell(ScAddress(nCol,nRow,nTab), pFormula);
                }
                }
                break;
        }
    }
    return eRet;
}

FltError ScFormatFilterPluginImpl::ScImportQuattroPro( SfxMedium &rMedium, ScDocument *pDoc )
{
    FltError   eRet = eERR_OK;
    ScQProReader aReader( rMedium );
    eRet = aReader.import( pDoc );
    return eRet;
}

ScQProReader::ScQProReader( SfxMedium &rMedium ):
    ScBiffReader( rMedium )
{
}

FltError ScQProReader::import( ScDocument *pDoc )
{
    FltError eRet = eERR_OK;
    sal_uInt16 nVersion;
    sal_uInt16 i = 1, j = 1;
    SCTAB nTab = 0;
    SetEof( false );

    if( !recordsLeft() )
        return eERR_OPEN;

    ScQProStyle *pStyleElement = new ScQProStyle;

    while( nextRecord() && eRet == eERR_OK)
    {
        switch( getId() )
        {
            case 0x0000: // Begginning of file
                *mpStream >> nVersion;
                break;

            case 0x00ca: // Beginning of sheet
                if( nTab <= MAXTAB )
                {
                    if( nTab < 26 )
                    {
                        String aName;
                        aName.Append( sal_Unicode( 'A' + nTab ) );
                        if (!nTab)
                            pDoc->RenameTab( nTab, aName, false, false);
                        else
                            pDoc->InsertTab( nTab, aName );
                    }
                    eRet = readSheet( nTab, pDoc, pStyleElement );
                    nTab++;
                }
                break;

            case 0x0001: // End of file
                SetEof( sal_True );
                break;

            case 0x00ce:{ // Attribute cell
                sal_uInt8 nFormat, nAlign, nFont;
                sal_Int16 nColor;
                *mpStream >> nFormat >> nAlign >> nColor >> nFont;
                pStyleElement->setAlign( i, nAlign );
                pStyleElement->setFont( i, nFont );
                i++;
                }
                break;

            case 0x00cf:{ // Font description
                sal_uInt16 nPtSize, nFontAttr;
                String aLabel;
                *mpStream >> nPtSize >> nFontAttr;
                pStyleElement->setFontRecord( j, nFontAttr, nPtSize );
                sal_uInt16 nLen = getLength();
                if (nLen >= 4)
                    readString( aLabel, nLen - 4 );
                else
                    eRet = eERR_FORMAT;
                pStyleElement->setFontType( j, aLabel );
                j++;
                }
                break;
        }
    }
    pDoc->CalcAfterLoad();
    delete pStyleElement;
    return eRet;
}

bool ScQProReader::recordsLeft()
{
    bool bValue = ScBiffReader::recordsLeft();
    return bValue;
}

bool ScQProReader::nextRecord()
{
    bool bValue = ScBiffReader::nextRecord();
    return bValue;
}

void ScQProReader::readString( String &rString, sal_uInt16 nLength )
{
    sal_Char* pText = new sal_Char[ nLength + 1 ];
    mpStream->Read( pText, nLength );
    pText[ nLength ] = 0;
    rString = String( pText, mpStream->GetStreamCharSet() );
    delete [] pText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
