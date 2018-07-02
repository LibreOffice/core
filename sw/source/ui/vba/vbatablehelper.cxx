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
#include "vbatablehelper.hxx"
#include <swtable.hxx>
#include <unotbl.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define UNO_TABLE_COLUMN_SUM    10000

SwVbaTableHelper::SwVbaTableHelper( const uno::Reference< text::XTextTable >& xTextTable ) : mxTextTable( xTextTable )
{
    pTable = GetSwTable( mxTextTable );
}

SwTable* SwVbaTableHelper::GetSwTable( const uno::Reference< text::XTextTable >& xTextTable )
{
    uno::Reference< lang::XUnoTunnel > xTunnel( xTextTable, uno::UNO_QUERY_THROW );
    SwXTextTable* pXTextTable = reinterpret_cast< SwXTextTable * >( sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextTable::getUnoTunnelId())));
    if( !pXTextTable )
        throw uno::RuntimeException();

    SwFrameFormat* pFrameFormat = pXTextTable->GetFrameFormat();
    if( !pFrameFormat )
        throw uno::RuntimeException();

    SwTable* pTable = SwTable::FindTable( pFrameFormat );
    return pTable;
}

sal_Int32 SwVbaTableHelper::getTabColumnsCount( sal_Int32 nRowIndex )
{
    sal_Int32 nRet = 0;
    if(!pTable->IsTableComplex())
    {
        SwTableLines& rLines = pTable->GetTabLines();
        SwTableLine* pLine = rLines[ nRowIndex ];
        nRet = pLine->GetTabBoxes().size();
    }
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabColumnsMaxCount( )
{
    sal_Int32 nRet = 0;
    sal_Int32 nRowCount = pTable->GetTabLines().size();
    for( sal_Int32 index = 0; index < nRowCount; index++ )
    {
        sal_Int32 nColCount = getTabColumnsCount( index );
        if( nRet < nColCount )
            nRet = nColCount;
    }
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabRowIndex( const OUString& rCellName )
{
    sal_Int32 nRet = 0;
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox( rCellName ));
    if( !pBox )
        throw uno::RuntimeException();

    const SwTableLine* pLine = pBox->GetUpper();
    const SwTableLines* pLines = pLine->GetUpper()
                     ? &pLine->GetUpper()->GetTabLines() : &pTable->GetTabLines();
    nRet = pLines->GetPos( pLine );
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabColIndex( const OUString& rCellName )
{
    const SwTableBox* pBox = pTable->GetTableBox( rCellName );
    if( !pBox )
        throw uno::RuntimeException();
    return pBox->GetUpper()->GetBoxPos( pBox );
}

OUString SwVbaTableHelper::getColumnStr( sal_Int32 nCol )
{
    const sal_Int32 coDiff = 52; // 'A'-'Z' 'a' - 'z'
    sal_Int32 nCalc = 0;

    OUString sRet;
    do{
        nCalc = nCol % coDiff;
        if( nCalc >= 26 )
            sRet = OUStringLiteral1( 'a' - 26 + nCalc ) + sRet;
        else
            sRet = OUStringLiteral1( 'A' + nCalc ) + sRet;

        if( 0 == ( nCol = nCol - nCalc ) )
            break;
        nCol /= coDiff;
        --nCol;
    }while(true);
    return sRet;
}

sal_Int32 SwVbaTableHelper::getTableWidth( )
{
    sal_Int32 nWidth = 0;
    bool isWidthRelatvie = false;
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    xTableProps->getPropertyValue("IsWidthRelative") >>= isWidthRelatvie;
    if( isWidthRelatvie )
    {
        xTableProps->getPropertyValue("RelativeWidth") >>= nWidth;
    }
    else
    {
        xTableProps->getPropertyValue("Width") >>= nWidth;
    }
    return nWidth;
}

SwTableBox* SwVbaTableHelper::GetTabBox( sal_Int32 nCol, sal_Int32 nRow )
{
    SwTableLines& rLines = pTable->GetTabLines();
    sal_Int32 nRowCount = rLines.size();
    if (nRow < 0 || nRow >= nRowCount)
        throw uno::RuntimeException();

    SwTableLine* pLine = rLines[ nRow ];
    sal_Int32 nColCount = pLine->GetTabBoxes().size();
    if (nCol < 0 || nCol >= nColCount)
        throw uno::RuntimeException();

    SwTableBox* pStart = pLine->GetTabBoxes()[ nCol ].get();

    if( !pStart )
        throw uno::RuntimeException();

    return pStart;
}

void SwVbaTableHelper::InitTabCols( SwTabCols& rCols, const SwTableBox *pStart )
{
    rCols.SetLeftMin ( 0 );
    rCols.SetLeft    ( 0 );
    rCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    rCols.SetRightMax( UNO_TABLE_COLUMN_SUM );
    pTable->GetTabCols( rCols, pStart );
}

sal_Int32 SwVbaTableHelper::GetColCount( SwTabCols const & rCols )
{
    sal_Int32 nCount = 0;
    for( size_t i = 0; i < rCols.Count(); ++i )
        if(rCols.IsHidden(i))
            nCount ++;
    return rCols.Count() - nCount;
}

sal_Int32 SwVbaTableHelper::GetRightSeparator( SwTabCols const & rCols, sal_Int32 nNum)
{
    OSL_ENSURE( nNum < GetColCount( rCols ) ,"Index out of range");
    sal_Int32 i = 0;
    while( nNum >= 0 )
    {
        if( !rCols.IsHidden(i) )
            nNum--;
        i++;
    }
    return i - 1;
}

sal_Int32 SwVbaTableHelper::GetColWidth( sal_Int32 nCol, sal_Int32 nRow )
{
    SwTableBox* pStart = GetTabBox( nCol, nRow );
    SwTabCols aCols;
    InitTabCols( aCols, pStart );
    sal_Int32 nWidth = GetColWidth( aCols, nCol );

    sal_Int32 nTableWidth = getTableWidth( );
    double dAbsWidth = ( static_cast<double>(nWidth) / UNO_TABLE_COLUMN_SUM ) * static_cast<double>(nTableWidth);
    return static_cast<sal_Int32>(Millimeter::getInPoints( static_cast<int>(dAbsWidth) ));
}

sal_Int32 SwVbaTableHelper::GetColWidth( SwTabCols& rCols, sal_Int32 nNum )
{
    SwTwips nWidth = 0;

    if( rCols.Count() > 0 )
    {
        if(rCols.Count() == static_cast<size_t>(GetColCount( rCols )))
        {
            if(static_cast<size_t>(nNum) == rCols.Count())
                nWidth = rCols.GetRight() - rCols[nNum-1];
            else
            {
                nWidth = rCols[nNum];
                if(nNum == 0)
                    nWidth -= rCols.GetLeft();
                else
                    nWidth -= rCols[nNum-1];
            }
        }
        else
        {
            SwTwips nRValid = nNum < GetColCount( rCols ) ?
                            rCols[GetRightSeparator( rCols, nNum )]:
                                    rCols.GetRight();
            SwTwips nLValid = nNum ?
                            rCols[GetRightSeparator( rCols, nNum - 1 )]:
                                    rCols.GetLeft();
            nWidth = nRValid - nLValid;
        }
    }
    else
        nWidth = rCols.GetRight();

    return nWidth;
}

void SwVbaTableHelper::SetColWidth( sal_Int32 _width, sal_Int32 nCol, sal_Int32 nRow, bool bCurRowOnly )
{
    double dAbsWidth = Millimeter::getInHundredthsOfOneMillimeter( _width );
    sal_Int32 nTableWidth = getTableWidth( );
    sal_Int32 nNewWidth = dAbsWidth/nTableWidth * UNO_TABLE_COLUMN_SUM;

    SwTableBox* pStart = GetTabBox( nCol, nRow );
    SwTabCols aOldCols;
    InitTabCols( aOldCols, pStart );

    SwTabCols aCols( aOldCols );
    if ( aCols.Count() > 0 )
    {
        SwTwips nWidth = GetColWidth( aCols, nCol);

        int nDiff = nNewWidth - nWidth;
        if( !nCol )
            aCols[ GetRightSeparator(aCols, 0) ] += nDiff;
        else if( nCol < GetColCount( aCols )  )
        {
            if(nDiff < GetColWidth( aCols, nCol + 1) - MINLAY)
                aCols[ GetRightSeparator( aCols, nCol ) ] += nDiff;
            else
            {
                int nDiffLeft = nDiff - static_cast<int>(GetColWidth( aCols, nCol + 1)) + int(MINLAY);
                aCols[ GetRightSeparator( aCols, nCol ) ] += (nDiff - nDiffLeft);
                aCols[ GetRightSeparator( aCols, nCol - 1 ) ] -= nDiffLeft;
            }
        }
        else
            aCols[ GetRightSeparator( aCols, nCol-1 ) ] -= nDiff;
    }
    else
        aCols.SetRight( std::min( static_cast<long>(nNewWidth), aCols.GetRightMax()) );

    pTable->SetTabCols(aCols, aOldCols, pStart, bCurRowOnly );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
