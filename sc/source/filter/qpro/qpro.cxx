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
#include <sal/log.hxx>

#include <qproform.hxx>
#include <qpro.hxx>
#include <qprostyle.hxx>

#include <scerrors.hxx>
#include <ftools.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>
#include <docoptio.hxx>
#include <scdll.hxx>
#include <memory>

ErrCode ScQProReader::readSheet( SCTAB nTab, ScDocument* pDoc, ScQProStyle *pStyle )
{
    ErrCode eRet = ERRCODE_NONE;
    sal_uInt8  nCol, nDummy;
    sal_uInt16 nRow;
    sal_uInt16 nStyle;
    bool bEndOfSheet = false;

    SAL_INFO("sc", "Read sheet " << nTab);

    while( ERRCODE_NONE == eRet && !bEndOfSheet && nextRecord() )
    {
        switch( getId() )
        {
            case 0x000f:{ // Label cell
                mpStream->ReadUChar( nCol ).ReadUChar( nDummy ).ReadUInt16( nRow ).ReadUInt16( nStyle ).ReadUChar( nDummy );
                sal_uInt16 nLen = getLength();
                if (nLen >= 7)
                {
                    OUString aLabel(readString(nLen - 7));
                    nStyle = nStyle >> 3;
                    pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                    pDoc->EnsureTable(nTab);
                    pDoc->SetTextCell(ScAddress(nCol,nRow,nTab), aLabel);
                }
                else
                    eRet = SCERR_IMPORT_FORMAT;
                }
                break;

            case 0x00cb: // End of sheet
                bEndOfSheet = true;
                break;

            case 0x000c: // Blank cell
                mpStream->ReadUChar( nCol ).ReadUChar( nDummy ).ReadUInt16( nRow ).ReadUInt16( nStyle );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                break;

            case 0x000d:{ // Integer cell
                sal_Int16 nValue;
                mpStream->ReadUChar( nCol ).ReadUChar( nDummy ).ReadUInt16( nRow ).ReadUInt16( nStyle ).ReadInt16( nValue );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->EnsureTable(nTab);
                pDoc->SetValue(ScAddress(nCol,nRow,nTab), static_cast<double>(nValue));
                }
                break;

            case 0x000e:{ // Floating point cell
                double nValue;
                mpStream->ReadUChar( nCol ).ReadUChar( nDummy ).ReadUInt16( nRow ).ReadUInt16( nStyle ).ReadDouble( nValue );
                nStyle = nStyle >> 3;
                pStyle->SetFormat( pDoc, nCol, nRow, nTab, nStyle );
                pDoc->EnsureTable(nTab);
                pDoc->SetValue(ScAddress(nCol,nRow,nTab), nValue);
                }
                break;

            case 0x0010:
            {
                // Formula cell
                double nValue;
                sal_uInt16 nState, nLen;
                mpStream->ReadUChar( nCol ).ReadUChar( nDummy ).ReadUInt16( nRow ).ReadUInt16( nStyle ).ReadDouble( nValue ).ReadUInt16( nState ).ReadUInt16( nLen );
                if (!mpStream->good())
                {
                    eRet = SCERR_IMPORT_FORMAT;
                    break;
                }
                ScAddress aAddr( nCol, nRow, nTab );
                std::unique_ptr<ScTokenArray> pArray;

                QProToSc aConv(*mpStream, pDoc->GetSharedStringPool(), aAddr);
                if (ConvErr::OK != aConv.Convert( pArray ))
                    eRet = SCERR_IMPORT_FORMAT;
                else
                {
                    ScFormulaCell* pFormula = new ScFormulaCell(pDoc, aAddr, std::move(pArray));
                    nStyle = nStyle >> 3;
                    pFormula->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE );
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

ErrCode ScFormatFilterPluginImpl::ScImportQuattroPro(SvStream *pStream, ScDocument *pDoc)
{
    ScQProReader aReader(pStream);
    ErrCode eRet = aReader.import( pDoc );
    return eRet;
}

ScQProReader::ScQProReader(SvStream* pStream)
    : mnId(0)
    , mnLength(0)
    , mnOffset(0)
    , mpStream(pStream)
    , mbEndOfFile(false)
    , mnMaxTab(utl::ConfigManager::IsFuzzing() ? 128 : MAXTAB)
{
    if( mpStream )
    {
        mpStream->SetBufferSize( 65535 );
        mpStream->SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
    }
}

ScQProReader::~ScQProReader()
{
    if( mpStream )
        mpStream->SetBufferSize( 0 );
}

ErrCode ScQProReader::parse( ScDocument *pDoc )
{
    ErrCode eRet = ERRCODE_NONE;
    sal_uInt16 nVersion;
    sal_uInt16 i = 1, j = 1;
    SCTAB nTab = 0;
    SetEof( false );

    if( !recordsLeft() )
        return SCERR_IMPORT_OPEN;

    std::unique_ptr<ScQProStyle> pStyleElement( new ScQProStyle );

    while( nextRecord() && eRet == ERRCODE_NONE)
    {
        switch( getId() )
        {
            case 0x0000: // Beginning of file
                mpStream->ReadUInt16( nVersion );
                break;

            case 0x00ca: // Beginning of sheet
                if (nTab <= mnMaxTab)
                {
                    if( nTab < 26 )
                    {
                        OUString aName;
                        aName += OUStringLiteral1( 'A' + nTab );
                        if (!nTab)
                            pDoc->RenameTab( nTab, aName );
                        else
                            pDoc->InsertTab( nTab, aName );
                    }
                    eRet = readSheet( nTab, pDoc, pStyleElement.get() );
                    nTab++;
                }
                break;

            case 0x0001: // End of file
                SetEof( true );
                break;

            case 0x00ce:{ // Attribute cell
                sal_uInt8 nFormat, nAlign, nFont;
                sal_Int16 nColor;
                mpStream->ReadUChar( nFormat ).ReadUChar( nAlign ).ReadInt16( nColor ).ReadUChar( nFont );
                pStyleElement->setAlign( i, nAlign );
                pStyleElement->setFont( i, nFont );
                i++;
                }
                break;

            case 0x00cf:{ // Font description
                sal_uInt16 nPtSize, nFontAttr;
                OUString aLabel;
                mpStream->ReadUInt16( nPtSize ).ReadUInt16( nFontAttr );
                pStyleElement->setFontRecord( j, nFontAttr, nPtSize );
                sal_uInt16 nLen = getLength();
                if (nLen >= 4)
                    aLabel = readString(nLen - 4);
                else
                    eRet = SCERR_IMPORT_FORMAT;
                pStyleElement->setFontType( j, aLabel );
                j++;
                }
                break;
        }
    }
    return eRet;
}

ErrCode ScQProReader::import( ScDocument *pDoc )
{
    ErrCode eRet = parse(pDoc);
    pDoc->CalcAfterLoad();
    return eRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportQPW(SvStream &rStream)
{
    ScDLL::Init();
    ScDocument aDocument;
    ScDocOptions aDocOpt = aDocument.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    aDocument.SetDocOptions(aDocOpt);
    aDocument.MakeTable(0);
    aDocument.EnableExecuteLink(false);
    aDocument.SetInsertingFromOtherDoc(true);
    aDocument.SetImportingXML(true);

    ScQProReader aReader(&rStream);
    ErrCode eRet = aReader.parse(&aDocument);
    return eRet == ERRCODE_NONE;
}

bool ScQProReader::recordsLeft()
{
    return mpStream && mpStream->good();
}

bool ScQProReader::nextRecord()
{
    if( !recordsLeft() )
        return false;

    if( mbEndOfFile )
        return false;

    sal_uInt32 nPos = mpStream->Tell();
    if( nPos != mnOffset + mnLength )
        mpStream->Seek( mnOffset + mnLength );

    mnLength = mnId = 0;
    mpStream->ReadUInt16( mnId ).ReadUInt16( mnLength );

    mnOffset = mpStream->Tell();
#ifdef DEBUG_SC_QPRO
    fprintf( stderr, "Read record 0x%x length 0x%x at offset 0x%x\n",
        (unsigned)mnId, (unsigned)mnLength, (unsigned)mnOffset );

#if 1  // rather verbose
    int len = mnLength;
    while (len > 0) {
        int i, chunk = std::min(len, 16);
        unsigned char data[16];
        mpStream->Read( data, chunk );

        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%.2x ", data[i] );
        fprintf( stderr, "| " );
        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%c", data[i] < 127 && data[i] > 30 ? data[i] : '.' );
        fprintf( stderr, "\n" );

        len -= chunk;
    }
    mpStream->Seek( mnOffset );
#endif
#endif
    return true;
}

OUString ScQProReader::readString(sal_uInt16 nLength)
{
    return read_uInt8s_ToOUString(*mpStream, nLength, mpStream->GetStreamCharSet());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
