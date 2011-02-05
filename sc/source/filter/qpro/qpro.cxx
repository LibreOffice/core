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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
                readString( aLabel, getLength() - 7 );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->PutCell( nCol, nRow, nTab, ScBaseCell::CreateTextCell( aLabel, pDoc ), (BOOL) TRUE );
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
                ScValueCell* pInteger = new ScValueCell( ( double ) nValue );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->PutCell(nCol ,nRow, nTab ,pInteger,(BOOL) TRUE);
                }
                break;

            case 0x000e:{ // Floating point cell
                double nValue;
                *mpStream >> nCol >> nDummy >> nRow >> nStyle >> nValue;
                ScValueCell* pFloat = new ScValueCell( nValue );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->PutCell( nCol, nRow, nTab, pFloat, (BOOL) TRUE );
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
                    pDoc->PutCell( nCol, nRow, nTab, pFormula, ( BOOL ) TRUE );
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
    SetEof( FALSE );

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
                            pDoc->RenameTab( nTab, aName, FALSE, FALSE);
                        else
                            pDoc->InsertTab( nTab, aName );
                    }
                    eRet = readSheet( nTab, pDoc, pStyleElement );
                    nTab++;
                }
                break;

            case 0x0001: // End of file
                SetEof( TRUE );
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
                readString( aLabel, getLength() - 4 );
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
