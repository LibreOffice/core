/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#include "vbatablehelper.hxx"
#include <swtable.hxx>
#include <unotbl.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define UNO_TABLE_COLUMN_SUM    10000

SwVbaTableHelper::SwVbaTableHelper( const uno::Reference< text::XTextTable >& xTextTable ) throw (uno::RuntimeException) : mxTextTable( xTextTable )
{
    pTable = GetSwTable( mxTextTable );
}

SwTable* SwVbaTableHelper::GetSwTable( const uno::Reference< text::XTextTable >& xTextTable ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XUnoTunnel > xTunnel( xTextTable, uno::UNO_QUERY_THROW );
    SwXTextTable* pXTextTable = reinterpret_cast< SwXTextTable * >( sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextTable::getUnoTunnelId())));
    if( !pXTextTable )
        throw uno::RuntimeException();

    SwFrmFmt* pFrmFmt = pXTextTable->GetFrmFmt();
    if( !pFrmFmt )
        throw uno::RuntimeException();

    SwTable* pTable = SwTable::FindTable( pFrmFmt );
    return pTable;
}

sal_Int32 SwVbaTableHelper::getTabColumnsCount( sal_Int32 nRowIndex ) throw (uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    if(!pTable->IsTblComplex())
    {
        SwTableLines& rLines = pTable->GetTabLines();
        SwTableLine* pLine = rLines[ nRowIndex ];
        nRet = pLine->GetTabBoxes().size();
    }
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabColumnsMaxCount( ) throw (uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    //sal_Int32 nRowCount = mxTextTable->getRows()->getCount();
    sal_Int32 nRowCount = pTable->GetTabLines().size();
    for( sal_Int32 index = 0; index < nRowCount; index++ )
    {
        sal_Int32 nColCount = getTabColumnsCount( index );
        if( nRet < nColCount )
            nRet = nColCount;
    }
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabRowIndex( const rtl::OUString& CellName ) throw (uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    String sCellName(CellName);
    SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if( !pBox )
        throw uno::RuntimeException();

    const SwTableLine* pLine = pBox->GetUpper();
    const SwTableLines* pLines = pLine->GetUpper()
                     ? &pLine->GetUpper()->GetTabLines() : &pTable->GetTabLines();
    nRet = pLines->GetPos( pLine );
    return nRet;
}

sal_Int32 SwVbaTableHelper::getTabColIndex( const rtl::OUString& CellName ) throw (uno::RuntimeException)
{
    sal_Int32 nRet = 0;
    String sCellName(CellName);
    const SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if( !pBox )
        throw uno::RuntimeException();
    const SwTableBoxes* pBoxes = &pBox->GetUpper()->GetTabBoxes();
    nRet = pBoxes->GetPos( pBox );
    return nRet;
}

rtl::OUString SwVbaTableHelper::getColumnStr( sal_Int32 nCol )
{
    const sal_Int32 coDiff = 52; // 'A'-'Z' 'a' - 'z'
    sal_Int32 nCalc = 0;

    String sRet;
    do{
        nCalc = nCol % coDiff;
        if( nCalc >= 26 )
            sRet.Insert( sal_Unicode('a' - 26 + nCalc ), 0 );
        else
            sRet.Insert( sal_Unicode('A' + nCalc ), 0 );

        if( 0 == ( nCol = nCol - nCalc ) )
            break;
        nCol /= coDiff;
        --nCol;
    }while(1);
    return sRet;
}

sal_Int32 SwVbaTableHelper::getTableWidth( ) throw (uno::RuntimeException)
{
    sal_Int32 nWidth = 0;
    sal_Bool isWidthRelatvie = sal_False;
    uno::Reference< beans::XPropertySet > xTableProps( mxTextTable, uno::UNO_QUERY_THROW );
    xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsWidthRelative") ) ) >>= isWidthRelatvie;
    if( isWidthRelatvie )
    {
        xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("RelativeWidth") ) ) >>= nWidth;
    }
    else
    {
        xTableProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ) ) >>= nWidth;
    }
    return nWidth;
}

SwTableBox* SwVbaTableHelper::GetTabBox( sal_Int32 nCol, sal_Int32 nRow ) throw (css::uno::RuntimeException)
{
    SwTableLines& rLines = pTable->GetTabLines();
    sal_Int32 nRowCount = rLines.size();
    if( nRowCount < nRow )
        throw uno::RuntimeException();

    SwTableBox* pStart = NULL;
    SwTableLine* pLine = rLines[ nRow ];
    if( (sal_Int32)pLine->GetTabBoxes().size() < nCol )
        throw uno::RuntimeException();

    pStart = pLine->GetTabBoxes()[ nCol ];

    if( !pStart )
        throw uno::RuntimeException();

    return pStart;
}

void SwVbaTableHelper::InitTabCols( SwTabCols& rCols, const SwTableBox *pStart, sal_Bool /*bCurRowOnly*/ )
{
    rCols.SetLeftMin ( 0 );
    rCols.SetLeft    ( 0 );
    rCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    rCols.SetRightMax( UNO_TABLE_COLUMN_SUM );
    pTable->GetTabCols( rCols, pStart, sal_False, sal_False );
}

sal_Int32 SwVbaTableHelper::GetColCount( SwTabCols& rCols ) const
{
    sal_Int32 nCount = 0;
    for(sal_Int32 i = 0; i < rCols.Count(); i++ )
        if(rCols.IsHidden(i))
            nCount ++;
    return rCols.Count() - nCount;
}

sal_Int32 SwVbaTableHelper::GetRightSeparator( SwTabCols& rCols, sal_Int32 nNum) const
{
    OSL_ENSURE( nNum < (sal_Int32)GetColCount( rCols ) ,"Index out of range");
    sal_Int32 i = 0;
    while( nNum >= 0 )
    {
        if( !rCols.IsHidden( static_cast< sal_uInt16 >(i)) )
            nNum--;
        i++;
    }
    return i - 1;
}

sal_Int32 SwVbaTableHelper::GetColWidth( sal_Int32 nCol, sal_Int32 nRow, sal_Bool bCurRowOnly ) throw (uno::RuntimeException)
{
    SwTableBox* pStart = GetTabBox( nCol, nRow );
    SwTabCols aCols;
    InitTabCols( aCols, pStart, bCurRowOnly );
    sal_Int32 nWidth = GetColWidth( aCols, nCol );

    sal_Int32 nTableWidth = getTableWidth( );
    double dAbsWidth = ( (double)nWidth / UNO_TABLE_COLUMN_SUM ) * (double) nTableWidth;
    return ( sal_Int32 )Millimeter::getInPoints( static_cast<int>(dAbsWidth) );
}

sal_Int32 SwVbaTableHelper::GetColWidth( SwTabCols& rCols, sal_Int32 nNum ) throw (uno::RuntimeException)
{
    SwTwips nWidth = 0;

    if( rCols.Count() > 0 )
    {
        if(rCols.Count() == GetColCount( rCols ))
        {
            nWidth = (SwTwips)((nNum == rCols.Count()) ?
                    rCols.GetRight() - rCols[nNum-1] :
                    nNum == 0 ? rCols[nNum] - rCols.GetLeft() :
                                rCols[nNum] - rCols[nNum-1]);
        }
        else
        {
            SwTwips nRValid = nNum < GetColCount( rCols ) ?
                            rCols[(sal_uInt16)GetRightSeparator( rCols, nNum)]:
                                    rCols.GetRight();
            SwTwips nLValid = nNum ?
                            rCols[(sal_uInt16)GetRightSeparator( rCols, nNum - 1)]:
                                    rCols.GetLeft();
            nWidth = nRValid - nLValid;
        }
    }
    else
        nWidth = rCols.GetRight();

    return nWidth;
}

void SwVbaTableHelper::SetColWidth( sal_Int32 _width, sal_Int32 nCol, sal_Int32 nRow, sal_Bool bCurRowOnly ) throw (css::uno::RuntimeException)
{
    double dAbsWidth = Millimeter::getInHundredthsOfOneMillimeter( _width );
    sal_Int32 nTableWidth = getTableWidth( );
    sal_Int32 nNewWidth = (sal_Int32)( dAbsWidth/nTableWidth * UNO_TABLE_COLUMN_SUM );

    SwTableBox* pStart = GetTabBox( nCol, nRow );
    SwTabCols aOldCols;
    InitTabCols( aOldCols, pStart, bCurRowOnly );

    SwTabCols aCols( aOldCols );
    if ( aCols.Count() > 0 )
    {
        SwTwips nWidth = GetColWidth( aCols, nCol);

        int nDiff = (int)(nNewWidth - nWidth);
        if( !nCol )
            aCols[ static_cast< sal_uInt16 >(GetRightSeparator(aCols, 0)) ] += nDiff;
        else if( nCol < GetColCount( aCols )  )
        {
            if(nDiff < GetColWidth( aCols, nCol + 1) - MINLAY)
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator( aCols, nCol)) ] += nDiff;
            else
            {
                int nDiffLeft = nDiff - (int)GetColWidth( aCols, nCol + 1) + (int)MINLAY;
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator( aCols, nCol)) ] += (nDiff - nDiffLeft);
                aCols[ static_cast< sal_uInt16 >(GetRightSeparator( aCols, nCol - 1)) ] -= nDiffLeft;
            }
        }
        else
            aCols[ static_cast< sal_uInt16 >(GetRightSeparator( aCols, nCol-1)) ] -= nDiff;
    }
    else
        aCols.SetRight( Min( (long)nNewWidth, aCols.GetRightMax()) );

    pTable->SetTabCols(aCols, aOldCols, pStart, bCurRowOnly );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
