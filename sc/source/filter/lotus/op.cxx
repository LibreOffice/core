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
#include <rtl/character.hxx>

#include <string.h>

#include <scitems.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/configmgr.hxx>

#include <formulacell.hxx>
#include <document.hxx>
#include <postit.hxx>

#include <op.h>
#include <optab.h>
#include <tool.h>
#include "lotfilter.hxx"
#include <lotform.hxx>
#include <lotrange.hxx>
#include <ftools.hxx>

#include <vector>
#include <map>
#include <memory>

static sal_uInt16 nDefWidth = sal_uInt16( TWIPS_PER_CHAR * 10 );

void NI(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 n)
{
    r.SeekRel( n );
}

void OP_BOF(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 /*n*/)
{
    r.SeekRel( 2 );        // skip version number
}

void OP_EOF(LotusContext& rContext, SvStream& /*r*/, sal_uInt16 /*n*/)
{
    rContext.bEOF = true;
}

void OP_Integer(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt8 nFormat(0);
    sal_uInt16 nTmpCol(0), nTmpRow(0);
    sal_Int16 nValue(0);
    r.ReadUChar(nFormat).ReadUInt16(nTmpCol).ReadUInt16(nTmpRow).ReadInt16(nValue);
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    ScAddress aAddr(nCol, nRow, 0);
    if (rContext.rDoc.ValidAddress(aAddr))
    {
        rContext.rDoc.EnsureTable(0);
        // coverity[tainted_data : FALSE] - ValidAddress has sanitized aAddr
        rContext.rDoc.SetValue(aAddr, static_cast<double>(nValue));

        // 0 digits in fractional part!
        SetFormat(rContext, nCol, nRow, 0, nFormat, 0);
    }
}

void OP_Number(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt8 nFormat(0);
    sal_uInt16 nTmpCol(0), nTmpRow(0);
    double fValue(0.0);
    r.ReadUChar( nFormat ).ReadUInt16(nTmpCol).ReadUInt16(nTmpRow).ReadDouble(fValue);
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    ScAddress aAddr(nCol, nRow, 0);
    if (rContext.rDoc.ValidAddress(aAddr))
    {
        fValue = ::rtl::math::round( fValue, 15 );
        rContext.rDoc.EnsureTable(0);
        // coverity[tainted_data : FALSE] - ValidAddress has sanitized aAddr
        rContext.rDoc.SetValue(aAddr, fValue);

        SetFormat(rContext, nCol, nRow, 0, nFormat, nFractionalFloat);
    }
}

void OP_Label(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    sal_uInt8 nFormat(0);
    sal_uInt16 nTmpCol(0), nTmpRow(0);
    r.ReadUChar(nFormat).ReadUInt16(nTmpCol).ReadUInt16(nTmpRow);
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    n -= std::min<sal_uInt16>(n, 5);

    std::vector<char> aText(n + 1);
    n = r.ReadBytes(aText.data(), n);
    aText[n] = 0;

    if (rContext.rDoc.ValidColRow(nCol, nRow))
    {
        nFormat &= 0x80;    // don't change Bit 7
        nFormat |= 0x75;    // protected does not matter, special-text is set

        PutFormString(rContext, nCol, nRow, 0, aText.data());

        SetFormat(rContext, nCol, nRow, 0, nFormat, nFractionalStd);
    }
}

void OP_Formula(LotusContext &rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt8 nFormat(0);
    sal_uInt16 nTmpCol(0), nTmpRow(0);
    r.ReadUChar(nFormat).ReadUInt16(nTmpCol).ReadUInt16(nTmpRow);
    r.SeekRel(8);    // skip result
    sal_uInt16 nFormulaSize(0);
    r.ReadUInt16(nFormulaSize);

    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    std::unique_ptr<ScTokenArray> pResult;
    sal_Int32 nBytesLeft = nFormulaSize;
    ScAddress aAddress(nCol, nRow, 0);

    svl::SharedStringPool& rSPool = rContext.rDoc.GetSharedStringPool();
    LotusToSc aConv(rContext, r, rSPool, rContext.eCharset, false);
    aConv.Reset( aAddress );
    aConv.Convert( pResult, nBytesLeft );
    if (!aConv.good())
        return;

    if (rContext.rDoc.ValidColRow(nCol, nRow))
    {
        ScFormulaCell* pCell = new ScFormulaCell(rContext.rDoc, aAddress, std::move(pResult));
        pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE );
        rContext.rDoc.EnsureTable(0);
        // coverity[tainted_data : FALSE] - ValidColRow has sanitized aAddr
        rContext.rDoc.SetFormulaCell(ScAddress(nCol, nRow, 0), pCell);

        // nFormat = Default -> number of digits in fractional part like Float
        SetFormat(rContext, nCol, nRow, 0, nFormat, nFractionalFloat);
    }
}

void OP_ColumnWidth(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt16 nTmpCol(0);
    sal_uInt8 nWidthSpaces(0);
    r.ReadUInt16(nTmpCol).ReadUChar(nWidthSpaces);
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));

    if (!rContext.rDoc.ValidCol(nCol))
        return;

    nCol = rContext.rDoc.SanitizeCol(nCol);

    sal_uInt16 nBreite;
    if( nWidthSpaces )
        // assuming 10cpi character set
        nBreite = static_cast<sal_uInt16>( TWIPS_PER_CHAR * nWidthSpaces );
    else
    {
        rContext.rDoc.SetColHidden(nCol, nCol, 0, true);
        nBreite = nDefWidth;
    }

    rContext.rDoc.SetColWidth(nCol, 0, nBreite);
}

void OP_NamedRange(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    // POST: don't save for invalid coordinates
    sal_uInt16              nColSt, nRowSt, nColEnd, nRowEnd;

    char cBuffer[ 16+1 ];
    r.ReadBytes(cBuffer, 16);
    cBuffer[ 16 ] = 0;

    r.ReadUInt16( nColSt ).ReadUInt16( nRowSt ).ReadUInt16( nColEnd ).ReadUInt16( nRowEnd );

    if (!r.good())
        return;

    if (!rContext.rDoc.ValidColRow(static_cast<SCCOL>(nColSt), nRowSt) || !rContext.rDoc.ValidColRow(static_cast<SCCOL>(nColEnd), nRowEnd))
        return;

    std::unique_ptr<LotusRange> pRange;

    if( nColSt == nColEnd && nRowSt == nRowEnd )
        pRange.reset(new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt) ));
    else
        pRange.reset(new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt),
                static_cast<SCCOL> (nColEnd), static_cast<SCROW> (nRowEnd) ));

    char cBuf[sizeof(cBuffer)+1];
    if( rtl::isAsciiDigit( static_cast<unsigned char>(*cBuffer) ) )
    {  // first char in name is a number -> prepend 'A'
        cBuf[0] = 'A';
        strcpy( cBuf + 1, cBuffer );       // #100211# - checked
    }
    else
        strcpy( cBuf, cBuffer );           // #100211# - checked

    OUString      aTmp( cBuf, strlen(cBuf), rContext.eCharset );

    aTmp = ScfTools::ConvertToScDefinedName( aTmp );

    rContext.maRangeNames.Append( &rContext.rDoc, std::move(pRange) );
}

void OP_SymphNamedRange(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    // POST:don't save for invalid coordinates
    sal_uInt16              nColSt, nRowSt, nColEnd, nRowEnd;
    sal_uInt8               nType;

    char cBuffer[ 16+1 ];
    r.ReadBytes(cBuffer, 16);
    cBuffer[ 16 ] = 0;

    r.ReadUInt16( nColSt ).ReadUInt16( nRowSt ).ReadUInt16( nColEnd ).ReadUInt16( nRowEnd ).ReadUChar( nType );

    if (!r.good())
        return;

    if (!rContext.rDoc.ValidColRow(static_cast<SCCOL>(nColSt), nRowSt) || !rContext.rDoc.ValidColRow(static_cast<SCCOL>(nColEnd), nRowEnd))
        return;

    std::unique_ptr<LotusRange> pRange;

    if( nType )
        pRange.reset(new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt) ));
    else
        pRange.reset(new LotusRange( static_cast<SCCOL> (nColSt), static_cast<SCROW> (nRowSt),
                static_cast<SCCOL> (nColEnd), static_cast<SCROW> (nRowEnd) ));

    char cBuf[sizeof(cBuffer)+1];
    if( rtl::isAsciiDigit( static_cast<unsigned char>(*cBuffer) ) )
    {  // first char in name is a number -> prepend 'A'
        cBuf[0] = 'A';
        strcpy( cBuf + 1, cBuffer );       // #100211# - checked
    }
    else
        strcpy( cBuf, cBuffer );           // #100211# - checked

    OUString  aTmp( cBuf, strlen(cBuf), rContext.eCharset );
    aTmp = ScfTools::ConvertToScDefinedName( aTmp );

    rContext.maRangeNames.Append( &rContext.rDoc, std::move(pRange) );
}

void OP_Footer(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 n)
{
    r.SeekRel( n );
}

void OP_Header(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 n)
{
    r.SeekRel( n );
}

void OP_Margins(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 n)
{
    r.SeekRel( n );
}

void OP_HiddenCols(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    SCCOL nCount = 0;
    for (sal_uInt16 nByte = 0; nByte < 32; ++nByte) // 32 Bytes with ...
    {
        sal_uInt8 nCurrent(0);
        r.ReadUChar(nCurrent);
        for (sal_uInt16 nBit = 0; nBit < 8; ++nBit) // ...each 8 Bits = 256 Bits
        {
            if( nCurrent & 0x01 )   // is lowest Bit set?
            {
                // -> Hidden Col
                rContext.rDoc.SetColHidden(nCount, nCount, 0, true);
            }

            nCount++;
            nCurrent = nCurrent / 2;    // the next please...
        }
    }
}

void OP_Window1(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    r.SeekRel( 4 );    // skip Cursor Pos

    sal_uInt8  nDefaultFormat; // -> op.cpp, standard cell format
    r.ReadUChar(nDefaultFormat);

    r.SeekRel( 1 );    // skip 'unused'

    r.ReadUInt16( nDefWidth );
    if (!r.good())
        return;

    r.SeekRel( n - 8 );  // skip the rest

    nDefWidth = static_cast<sal_uInt16>( TWIPS_PER_CHAR * nDefWidth );

    const bool bFuzzing = utl::ConfigManager::IsFuzzing();

    // instead of default, set all Cols in SC by hand
    for (SCCOL nCol = 0 ; nCol <= rContext.rDoc.MaxCol() ; nCol++)
    {
        rContext.rDoc.SetColWidth( nCol, 0, nDefWidth );
        if (bFuzzing)
            break;
    }
}

void OP_Blank(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt8 nFormat(0);
    sal_uInt16 nTmpCol(0), nTmpRow(0);
    r.ReadUChar( nFormat ).ReadUInt16(nTmpCol).ReadUInt16(nTmpRow);
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    SetFormat(rContext, nCol, nRow, 0, nFormat, nFractionalFloat);
}

void OP_BOF123(LotusContext& /*rContext*/, SvStream& r, sal_uInt16 /*n*/)
{
    r.SeekRel( 26 );
}

void OP_EOF123(LotusContext& rContext, SvStream& /*r*/, sal_uInt16 /*n*/)
{
    rContext.bEOF = true;
}

void OP_Label123(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    sal_uInt8 nTmpTab(0), nTmpCol(0);
    sal_uInt16 nTmpRow(0);
    r.ReadUInt16(nTmpRow).ReadUChar(nTmpTab).ReadUChar(nTmpCol);
    SCTAB nTab(static_cast<SCTAB>(nTmpTab));
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    n -= std::min<sal_uInt16>(n, 4);

    std::unique_ptr<char[]> pText(new char[n + 1]);
    r.ReadBytes(pText.get(), n);
    pText[ n ] = 0;

    PutFormString(rContext, nCol, nRow, nTab, pText.get());
}

void OP_Number123(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt16 nTmpRow(0);
    sal_uInt8 nTmpCol(0), nTmpTab(0);
    sal_uInt32 nValue(0);
    r.ReadUInt16(nTmpRow).ReadUChar(nTmpTab).ReadUChar(nTmpCol).ReadUInt32(nValue);
    SCTAB nTab(static_cast<SCTAB>(nTmpTab));
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    ScAddress aAddr(nCol, nRow, nTab);
    if (rContext.rDoc.ValidAddress(aAddr) && nTab <= rContext.rDoc.GetMaxTableNumber())
    {
        double fValue = Snum32ToDouble( nValue );
        rContext.rDoc.EnsureTable(nTab);
        // coverity[tainted_data : FALSE] - ValidAddress has sanitized aAddr
        rContext.rDoc.SetValue(aAddr, fValue);
    }
}

void OP_Formula123(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    sal_uInt16 nTmpRow(0);
    sal_uInt8 nTmpCol(0), nTmpTab(0);
    r.ReadUInt16(nTmpRow).ReadUChar(nTmpTab).ReadUChar(nTmpCol);
    SCTAB nTab(static_cast<SCTAB>(nTmpTab));
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));
    r.SeekRel( 8 );    // skip Result

    std::unique_ptr<ScTokenArray> pResult;
    sal_Int32 nBytesLeft = (n > 12) ? n - 12 : 0;
    ScAddress aAddress(nCol, nRow, nTab);

    svl::SharedStringPool& rSPool = rContext.rDoc.GetSharedStringPool();
    LotusToSc aConv(rContext, r, rSPool, rContext.eCharset, true);
    aConv.Reset( aAddress );
    aConv.Convert( pResult, nBytesLeft );
    if (!aConv.good())
        return;

    if (rContext.rDoc.ValidAddress(aAddress) && nTab <= rContext.rDoc.GetMaxTableNumber())
    {
        ScFormulaCell* pCell = new ScFormulaCell(rContext.rDoc, aAddress, std::move(pResult));
        pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE );
        rContext.rDoc.EnsureTable(nTab);
        // coverity[tainted_data : FALSE] - ValidAddress has sanitized aAddr
        rContext.rDoc.SetFormulaCell(aAddress, pCell);
    }
}

void OP_IEEENumber123(LotusContext& rContext, SvStream& r, sal_uInt16 /*n*/)
{
    sal_uInt16 nTmpRow(0);
    sal_uInt8 nTmpCol(0), nTmpTab(0);
    double dValue(0.0);
    r.ReadUInt16(nTmpRow).ReadUChar(nTmpTab).ReadUChar(nTmpCol).ReadDouble(dValue);
    SCTAB nTab(static_cast<SCTAB>(nTmpTab));
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    ScAddress aAddr(nCol, nRow, nTab);
    if (rContext.rDoc.ValidAddress(aAddr) && nTab <= rContext.rDoc.GetMaxTableNumber())
    {
        rContext.rDoc.EnsureTable(nTab);
        // coverity[tainted_data : FALSE] - ValidAddress has sanitized aAddr
        rContext.rDoc.SetValue(aAddr, dValue);
    }
}

void OP_Note123(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    sal_uInt16 nTmpRow(0);
    sal_uInt8 nTmpTab(0), nTmpCol(0);
    r.ReadUInt16(nTmpRow).ReadUChar(nTmpTab).ReadUChar(nTmpCol);
    SCTAB nTab(static_cast<SCTAB>(nTmpTab));
    SCCOL nCol(static_cast<SCCOL>(nTmpCol));
    SCROW nRow(static_cast<SCROW>(nTmpRow));

    n -= std::min<sal_uInt16>(n, 4);

    std::unique_ptr<char[]> pText(new char[n + 1]);
    r.ReadBytes(pText.get(), n);
    pText[ n ] = 0;

    OUString aNoteText(pText.get(), strlen(pText.get()), rContext.eCharset);
    pText.reset();

    ScAddress aPos(nCol, nRow, nTab);
    ScNoteUtil::CreateNoteFromString( rContext.rDoc, aPos, aNoteText, false, false );
}

void OP_HorAlign123(LotusContext& /*rContext*/, sal_uInt8 nAlignPattern, SfxItemSet& rPatternItemSet)
{
//      pre:  Pattern is stored in the last 3 bites of the 21st byte
//      post: Appropriate Horizontal Alignment is set in rPattern according to the bit pattern.
//
//      LEFT:001, RIGHT:010, CENTER:011, JUSTIFY:110,
//      LEFT-Text/RIGHT-NUMBER:100, DEFAULT:000

    nAlignPattern = ( nAlignPattern & 0x07);

    switch (nAlignPattern)
    {
        case 1:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY ) );
            break;
          case 2:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Right, ATTR_HOR_JUSTIFY ) );
            break;
        case 3:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY) );
            break;
        case 4:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY ) );
            break;
        case 6:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Block, ATTR_HOR_JUSTIFY ) );
            break;
        default:
            rPatternItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY ) );
            break;
      }
}

void OP_VerAlign123(LotusContext& /*rContext*/, sal_uInt8 nAlignPattern, SfxItemSet& rPatternItemSet)
{
//      pre:  Pattern is stored in the last 3 bites of the 22nd byte
//      post: Appropriate Vertical Alignment is set in rPattern according to the bit pattern.
//
//      TOP:001, MIDDLE:010, DOWN:100, DEFAULT:000

    nAlignPattern = ( nAlignPattern & 0x07);

    switch (nAlignPattern)
    {
        case 0:
            rPatternItemSet.Put( SvxVerJustifyItem(SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY) );
            break;
        case 1:
            rPatternItemSet.Put( SvxVerJustifyItem(SvxCellVerJustify::Top, ATTR_VER_JUSTIFY) );
            break;
        case 2:
            rPatternItemSet.Put( SvxVerJustifyItem(SvxCellVerJustify::Center, ATTR_VER_JUSTIFY) );
            break;
        case 4:
            rPatternItemSet.Put( SvxVerJustifyItem(SvxCellVerJustify::Bottom, ATTR_VER_JUSTIFY) );
            break;
        default:
            rPatternItemSet.Put( SvxVerJustifyItem(SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY) );
            break;
    }
}

void OP_CreatePattern123(LotusContext& rContext, SvStream& r, sal_uInt16 n)
{
    sal_uInt16 nCode;

    ScPatternAttr aPattern(rContext.rDoc.GetPool());
    SfxItemSet& rItemSet = aPattern.GetItemSet();

    r.ReadUInt16( nCode );
    n -= std::min<sal_uInt16>(n, 2);

    if ( nCode == 0x0fd2 )
    {
        sal_uInt16 nPatternId;
        r.ReadUInt16( nPatternId );

        sal_uInt8 Hor_Align, Ver_Align, temp;
        bool bIsBold,bIsUnderLine,bIsItalics;

        r.SeekRel(12);

        // Read 17th Byte
        r.ReadUChar( temp );

        bIsBold = (temp & 0x01);
        bIsItalics = (temp & 0x02);
        bIsUnderLine = (temp & 0x04);

        if ( bIsBold )
            rItemSet.Put( SvxWeightItem(WEIGHT_BOLD,ATTR_FONT_WEIGHT) );
        if ( bIsItalics )
            rItemSet.Put( SvxPostureItem(ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
        if ( bIsUnderLine )
            rItemSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE, ATTR_FONT_UNDERLINE ) );

        r.SeekRel(3);

        // Read 21st Byte
        r.ReadUChar( Hor_Align );
        OP_HorAlign123(rContext, Hor_Align, rItemSet );

        r.ReadUChar( Ver_Align );
        OP_VerAlign123(rContext, Ver_Align, rItemSet );

        rContext.aLotusPatternPool.emplace( nPatternId, aPattern );
        n -= std::min<sal_uInt16>(n, 20);
    }
    r.SeekRel(n);
}

void OP_SheetName123(LotusContext& rContext, SvStream& rStream, sal_uInt16 nLength)
{
    if (nLength <= 4)
    {
        rStream.SeekRel(nLength);
        return;
    }

    // B0 36 [sheet number (2 bytes?)] [sheet name (null terminated char array)]
    rStream.SeekRel(2); // ignore the first 2 bytes (B0 36).
    sal_uInt16 nSheetNum(0);
    rStream.ReadUInt16(nSheetNum);

    ::std::vector<char> sSheetName;
    sSheetName.reserve(nLength-4);
    for (sal_uInt16 i = 4; i < nLength; ++i)
    {
        char c;
        rStream.ReadChar( c );
        sSheetName.push_back(c);
    }

    if (!ValidTab(nSheetNum))
        return;
    // coverity[tainted_data : FALSE] - ValidTab has sanitized nSheetNum
    rContext.rDoc.MakeTable(nSheetNum);
    if (!sSheetName.empty())
    {
        OUString aName(sSheetName.data(), strlen(sSheetName.data()), rContext.eCharset);
        rContext.rDoc.RenameTab(nSheetNum, aName);
    }
}

void OP_ApplyPatternArea123(LotusContext& rContext, SvStream& rStream)
{
    sal_uInt16 nOpcode, nLength;
    sal_uInt16 nCol = 0, nColCount = 0, nRow = 0, nRowCount = 0, nTab = 0, nData, nTabCount = 0, nLevel = 0;

    do
    {
        rStream.ReadUInt16( nOpcode ).ReadUInt16( nLength );
        switch ( nOpcode )
        {
            case ROW_FORMAT_MARKER:
                nLevel++;
                break;
            case COL_FORMAT_MARKER:
                nLevel--;
                if( nLevel == 1 )
                {
                    nTab = nTab + nTabCount;
                    nCol = 0; nColCount = 0;
                    nRow = 0; nRowCount = 0;
                }
                break;
            case LOTUS_FORMAT_INDEX:
                if( nLength >= 2 )
                {
                    rStream.ReadUInt16( nData );
                    rStream.SeekRel( nLength - 2 );
                    if( nLevel == 1 )
                        nTabCount = SanitizeTab(nData);
                    else if( nLevel == 2 )
                    {
                        nCol = nCol + nColCount;
                        nColCount = nData;
                        if ( nCol > 0xff ) // 256 is the max col size supported by 123
                            nCol = 0;
                    }
                    else if( nLevel == 3 )
                    {
                        nRow = nRow + nRowCount;
                        nRowCount = nData;
                        if ( nRow > 0x1fff ) // 8192 is the max row size supported by 123
                            nRow = 0;
                    }
                }
                else
                    rStream.SeekRel( nLength );
                break;
            case LOTUS_FORMAT_INFO:
                if( nLength >= 2 )
                {
                    rStream.ReadUInt16( nData );
                    rStream.SeekRel( nLength - 2 );
                    std::map<sal_uInt16, ScPatternAttr>::iterator loc = rContext.aLotusPatternPool.find( nData );
                    // #126338# apparently, files with invalid index occur in the wild -> don't crash then
                    if ( loc != rContext.aLotusPatternPool.end() )
                        for( int i = 0; i < nTabCount; i++)
                        {
                            rContext.rDoc.ApplyPatternAreaTab( nCol, nRow, nCol +  nColCount - 1, nRow + nRowCount - 1, static_cast< SCTAB >( nTab + i ), loc->second );
                        }
                }
                else
                    rStream.SeekRel( nLength );
                break;
            default:
                rStream.SeekRel( nLength );
                break;
        }
    }
    while( nLevel && rStream.good() );

    rContext.aLotusPatternPool.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
