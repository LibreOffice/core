/*************************************************************************
 *
 *  $RCSfile: XMLExportIterator.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-16 18:14:35 $
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

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLEXPORTITERATOR_HXX
#include "XMLExportIterator.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_XSHEETANNOTATIONANCHOR_HPP_
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XSIMPLETEXT_HPP_
#include <com/sun/star/text/XSimpleText.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef SC_DOCITER_HXX
#include "dociter.hxx"
#endif
#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif
#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif

#include <algorithm>

using namespace ::rtl;
using namespace ::com::sun::star;

//==============================================================================

ScMyIteratorBase::ScMyIteratorBase()
{
}

ScMyIteratorBase::~ScMyIteratorBase()
{
}

void ScMyIteratorBase::UpdateAddress( table::CellAddress& rCellAddress )
{
    table::CellAddress aNewAddr( rCellAddress );
    if( GetFirstAddress( aNewAddr ) )
    {
        if( (aNewAddr.Sheet == rCellAddress.Sheet) &&
            ((aNewAddr.Row < rCellAddress.Row) ||
            ((aNewAddr.Row == rCellAddress.Row) && (aNewAddr.Column < rCellAddress.Column))) )
            rCellAddress = aNewAddr;
    }
}


//==============================================================================

ScMyShapesContainer::ScMyShapesContainer()
    : aShapeVec()
{
}

ScMyShapesContainer::~ScMyShapesContainer()
{
}

void ScMyShapesContainer::AddNewShape( const ScMyShape& aShape )
{
    aShapeVec.push_back(aShape);
}

sal_Bool ScMyShapesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aShapeVec.size() )
    {
        ScUnoConversion::FillApiAddress( rCellAddress, aShapeVec[0].aAddress );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyShapesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aShapeVec.clear();
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rMyCell.aCellAddress );

    ScMyShapeVec::iterator aItr = aShapeVec.begin();
    while( (aItr != aShapeVec.end()) && (aItr->aAddress == aAddress) )
    {
        rMyCell.aShapeVec.push_back(*aItr);
        aItr = aShapeVec.erase(aItr);
    }
    rMyCell.bHasShape = (rMyCell.aShapeVec.size() != 0);
}

sal_Bool LessMyShape(const ScMyShape& aShape1, const ScMyShape& aShape2)
{
    if( aShape1.aAddress.Tab() != aShape2.aAddress.Tab() )
        return (aShape1.aAddress.Tab() < aShape2.aAddress.Tab());
    else if( aShape1.aAddress.Row() != aShape2.aAddress.Row() )
        return (aShape1.aAddress.Row() < aShape2.aAddress.Row());
    else
        return (aShape1.aAddress.Col() < aShape2.aAddress.Col());
}

void ScMyShapesContainer::Sort()
{
    std::sort(aShapeVec.begin(), aShapeVec.end(), LessMyShape);
}

//==============================================================================

ScMyMergedRangesContainer::ScMyMergedRangesContainer()
    : aRangeVec()
{
}

ScMyMergedRangesContainer::~ScMyMergedRangesContainer()
{
}

void ScMyMergedRangesContainer::AddRange(const table::CellRangeAddress aMergedRange)
{
    sal_Int32 nStartRow = aMergedRange.StartRow;
    sal_Int32 nEndRow = aMergedRange.EndRow;

    ScMyMergedRange aRange;
    aRange.bIsFirst = sal_True;
    aRange.aCellRange = aMergedRange;
    aRange.aCellRange.EndRow = nStartRow;
    aRange.nRows = nEndRow - nStartRow + 1;
    aRangeVec.push_back( aRange );

    aRange.bIsFirst = sal_False;
    aRange.nRows = 0;
    for( sal_Int32 nRow = nStartRow + 1; nRow <= nEndRow; nRow++ )
    {
        aRange.aCellRange.StartRow = aRange.aCellRange.EndRow = nRow;
        aRangeVec.push_back(aRange);
    }
}

sal_Bool ScMyMergedRangesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aRangeVec.size() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, aRangeVec[0].aCellRange );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyMergedRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bIsMergedBase = rMyCell.bIsCovered = sal_False;
    ScMyMergedRangeVec::iterator aItr = aRangeVec.begin();
    if( aItr != aRangeVec.end() )
    {
        table::CellAddress aFirstAddress;
        ScUnoConversion::FillApiStartAddress( aFirstAddress, aItr->aCellRange );
        if( aFirstAddress == rMyCell.aCellAddress )
        {
            rMyCell.aMergeRange = aItr->aCellRange;
            rMyCell.bIsMergedBase = aItr->bIsFirst;
            rMyCell.bIsCovered = !aItr->bIsFirst;
            if( aItr->aCellRange.StartColumn < aItr->aCellRange.EndColumn )
            {
                aItr->aCellRange.StartColumn++;
                aItr->bIsFirst = sal_False;
            }
            else
                aRangeVec.erase(aItr);
        }
    }
}

sal_Bool LessMyMergedRange(const ScMyMergedRange& aRange1, const ScMyMergedRange& aRange2)
{
    if( aRange1.aCellRange.Sheet != aRange2.aCellRange.Sheet )
        return (aRange1.aCellRange.Sheet < aRange2.aCellRange.Sheet);
    else if( aRange1.aCellRange.StartRow != aRange2.aCellRange.StartRow )
        return (aRange1.aCellRange.StartRow < aRange2.aCellRange.StartRow);
    else
        return (aRange1.aCellRange.StartColumn < aRange2.aCellRange.StartColumn);
}

void ScMyMergedRangesContainer::Sort()
{
    std::sort(aRangeVec.begin(), aRangeVec.end(), LessMyMergedRange);
    ScMyMergedRangeVec::iterator aItr2_old = aRangeVec.begin();
    ScMyMergedRangeVec::iterator aItr2 = aItr2_old;
    while(aItr2 != aRangeVec.end())
    {
        if (aItr2 != aItr2_old)
        {
            if ((aItr2->aCellRange.StartColumn == aItr2_old->aCellRange.StartColumn) &&
                (aItr2->aCellRange.StartRow == aItr2_old->aCellRange.StartRow))
                aItr2 = aRangeVec.erase(aItr2);
            else
            {
                aItr2_old = aItr2;
                aItr2++;
            }
        }
        else
            aItr2++;
    }
}

//==============================================================================

sal_Bool ScMyAreaLink::Compare( const ScMyAreaLink& rAreaLink ) const
{
    return  (GetRowCount() == rAreaLink.GetRowCount()) &&
            (sFilter == rAreaLink.sFilter) &&
            (sFilterOptions == rAreaLink.sFilterOptions) &&
            (sURL == rAreaLink.sURL) &&
            (sSourceStr == rAreaLink.sSourceStr);
}

ScMyAreaLinksContainer::ScMyAreaLinksContainer() :
    aAreaLinkVec()
{
}

ScMyAreaLinksContainer::~ScMyAreaLinksContainer()
{
}

sal_Bool ScMyAreaLinksContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aAreaLinkVec.size() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, aAreaLinkVec[0].aDestRange );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyAreaLinksContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasAreaLink = sal_False;
    ScMyAreaLinkVec::iterator aItr = aAreaLinkVec.begin();
    if( aItr != aAreaLinkVec.end() )
    {
        table::CellAddress aAddress;
        ScUnoConversion::FillApiStartAddress( aAddress, aItr->aDestRange );
        if( aAddress == rMyCell.aCellAddress )
        {
            rMyCell.bHasAreaLink = sal_True;
            rMyCell.aAreaLink = *aItr;
            aAreaLinkVec.erase( aItr );
        }
    }
}

sal_Bool LessMyAreaLink( const ScMyAreaLink& rAreaLink1, const ScMyAreaLink& rAreaLink2 )
{
    if( rAreaLink1.aDestRange.Sheet != rAreaLink2.aDestRange.Sheet )
        return (rAreaLink1.aDestRange.Sheet < rAreaLink2.aDestRange.Sheet);
    else if( rAreaLink1.aDestRange.StartRow != rAreaLink2.aDestRange.StartRow )
        return (rAreaLink1.aDestRange.StartRow < rAreaLink2.aDestRange.StartRow);
    else
        return (rAreaLink1.aDestRange.StartColumn < rAreaLink2.aDestRange.StartColumn);
}

void ScMyAreaLinksContainer::Sort()
{
    ::std::sort( aAreaLinkVec.begin(), aAreaLinkVec.end(), LessMyAreaLink );
}

//==============================================================================

ScMyEmptyDatabaseRangesContainer::ScMyEmptyDatabaseRangesContainer()
    : aDatabaseVec()
{
}

ScMyEmptyDatabaseRangesContainer::~ScMyEmptyDatabaseRangesContainer()
{
}

void ScMyEmptyDatabaseRangesContainer::AddNewEmptyDatabaseRange(const table::CellRangeAddress& aCellRange)
{
    sal_Int32 nStartRow = aCellRange.StartRow;
    sal_Int32 nEndRow = aCellRange.EndRow;
    table::CellRangeAddress aRange( aCellRange );
    for( sal_Int32 nRow = nStartRow; nRow <= nEndRow; nRow++ )
    {
        aRange.StartRow = aRange.EndRow = nRow;
        aDatabaseVec.push_back( aRange );
    }
}

sal_Bool ScMyEmptyDatabaseRangesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aDatabaseVec.size() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, aDatabaseVec[0] );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyEmptyDatabaseRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasEmptyDatabase = sal_False;
    sal_Int16 nTable = rMyCell.aCellAddress.Sheet;
    ScMyEmptyDatabaseRangeVec::iterator aItr = aDatabaseVec.begin();
    if( aItr != aDatabaseVec.end() )
    {
        table::CellAddress aFirstAddress;
        ScUnoConversion::FillApiStartAddress( aFirstAddress, *aItr );
        if( aFirstAddress == rMyCell.aCellAddress )
        {
            rMyCell.bHasEmptyDatabase = sal_True;
            if( aItr->StartColumn < aItr->EndColumn )
                aItr->StartColumn++;
            else
                aDatabaseVec.erase(aItr);
        }
    }
}

sal_Bool LessMyEmptyDatabaseRange(const table::CellRangeAddress& aRange1, const table::CellRangeAddress& aRange2)
{
    if( aRange1.Sheet != aRange2.Sheet )
        return (aRange1.Sheet < aRange2.Sheet);
    else if( aRange1.StartRow != aRange2.StartRow )
        return (aRange1.StartRow < aRange2.StartRow);
    else
        return (aRange1.StartColumn < aRange2.StartColumn);
}

void ScMyEmptyDatabaseRangesContainer::Sort()
{
    std::sort(aDatabaseVec.begin(), aDatabaseVec.end(), LessMyEmptyDatabaseRange);
}

//==============================================================================

ScMyDetectiveObjContainer::ScMyDetectiveObjContainer() :
    aDetectiveObjVec()
{
}

ScMyDetectiveObjContainer::~ScMyDetectiveObjContainer()
{
}

void ScMyDetectiveObjContainer::AddObject( ScDetectiveObjType eObjType, const ScAddress& rPosition,
                                            const ScRange& rSourceRange, sal_Bool bHasError )
{
    if( (eObjType == SC_DETOBJ_ARROW) ||
        (eObjType == SC_DETOBJ_FROMOTHERTAB) ||
        (eObjType == SC_DETOBJ_TOOTHERTAB) )
    {
        ScMyDetectiveObj aDetObj;
        aDetObj.eObjType = eObjType;
        if( eObjType == SC_DETOBJ_TOOTHERTAB )
            ScUnoConversion::FillApiAddress( aDetObj.aPosition, rSourceRange.aStart );
        else
            ScUnoConversion::FillApiAddress( aDetObj.aPosition, rPosition );
        ScUnoConversion::FillApiRange( aDetObj.aSourceRange, rSourceRange );
        aDetObj.bHasError = bHasError;
        aDetectiveObjVec.push_back( aDetObj );
    }
}

sal_Bool ScMyDetectiveObjContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aDetectiveObjVec.size() )
    {
        rCellAddress = aDetectiveObjVec[0].aPosition;
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyDetectiveObjContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveObjVec.clear();
    ScMyDetectiveObjVec::iterator aItr = aDetectiveObjVec.begin();
    while( (aItr != aDetectiveObjVec.end()) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveObjVec.push_back( *aItr );
        aItr = aDetectiveObjVec.erase( aItr );
    }
    rMyCell.bHasDetectiveObj = (rMyCell.aDetectiveObjVec.size() != 0);
}

sal_Bool LessMyDetectiveObj( const ScMyDetectiveObj& rDetObj1, const ScMyDetectiveObj& rDetObj2 )
{
    if( rDetObj1.aPosition.Sheet != rDetObj2.aPosition.Sheet )
        return (rDetObj1.aPosition.Sheet < rDetObj2.aPosition.Sheet);
    else if( rDetObj1.aPosition.Row != rDetObj2.aPosition.Row )
        return (rDetObj1.aPosition.Row < rDetObj2.aPosition.Row);
    else
        return (rDetObj1.aPosition.Column != rDetObj2.aPosition.Column);
}

void ScMyDetectiveObjContainer::Sort()
{
    ::std::sort( aDetectiveObjVec.begin(), aDetectiveObjVec.end(), LessMyDetectiveObj );
}

//==============================================================================

ScMyDetectiveOpContainer::ScMyDetectiveOpContainer() :
    aDetectiveOpVec()
{
}

ScMyDetectiveOpContainer::~ScMyDetectiveOpContainer()
{
}

void ScMyDetectiveOpContainer::AddOperation( ScDetOpType eOpType, const ScAddress& rPosition )
{
    ScMyDetectiveOp aDetOp;
    aDetOp.eOpType = eOpType;
    ScUnoConversion::FillApiAddress( aDetOp.aPosition, rPosition );
    aDetOp.nIndex = aDetectiveOpVec.size();
    aDetectiveOpVec.push_back( aDetOp );
}

sal_Bool ScMyDetectiveOpContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( aDetectiveOpVec.size() )
    {
        rCellAddress = aDetectiveOpVec[0].aPosition;
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyDetectiveOpContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveOpVec.clear();
    ScMyDetectiveOpVec::iterator aItr = aDetectiveOpVec.begin();
    while( (aItr != aDetectiveOpVec.end()) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveOpVec.push_back( *aItr );
        aItr = aDetectiveOpVec.erase( aItr );
    }
    rMyCell.bHasDetectiveOp = (rMyCell.aDetectiveOpVec.size() != 0);
}

sal_Bool LessMyDetectiveOp( const ScMyDetectiveOp& rDetOp1, const ScMyDetectiveOp& rDetOp2 )
{
    if( rDetOp1.aPosition.Sheet != rDetOp2.aPosition.Sheet )
        return (rDetOp1.aPosition.Sheet < rDetOp2.aPosition.Sheet);
    else if( rDetOp1.aPosition.Row != rDetOp2.aPosition.Row )
        return (rDetOp1.aPosition.Row < rDetOp2.aPosition.Row);
    else
        return (rDetOp1.aPosition.Column != rDetOp2.aPosition.Column);
}

void ScMyDetectiveOpContainer::Sort()
{
    ::std::sort( aDetectiveOpVec.begin(), aDetectiveOpVec.end(), LessMyDetectiveOp );
}

//==============================================================================

ScMyCell::ScMyCell() :
    aShapeVec(),
    aDetectiveObjVec(),
    bHasShape( sal_False ),
    bIsMergedBase( sal_False ),
    bIsCovered( sal_False ),
    bHasAreaLink( sal_False ),
    bHasEmptyDatabase( sal_False ),
    bHasDetectiveObj( sal_False ),
    bHasDetectiveOp( sal_False ),
    bIsMatrixBase( sal_False ),
    bIsMatrixCovered( sal_False ),
    bHasAnnotation( sal_False )
{
}

ScMyCell::~ScMyCell()
{
}

//==============================================================================

ScMyNotEmptyCellsIterator::ScMyNotEmptyCellsIterator(ScXMLExport& rTempXMLExport)
    : rExport(rTempXMLExport),
    pCellItr(NULL),
    pShapes(NULL),
    pMergedRanges(NULL),
    pAreaLinks(NULL),
    pEmptyDatabaseRanges(NULL),
    pDetectiveObj(NULL),
    pDetectiveOp(NULL),
    nCurrentTable(-1)
{
}

ScMyNotEmptyCellsIterator::~ScMyNotEmptyCellsIterator()
{
    if (pCellItr)
        delete pCellItr;
}

void ScMyNotEmptyCellsIterator::UpdateAddress( table::CellAddress& rAddress )
{
    if( pCellItr->ReturnNext( nCellCol, nCellRow ) )
    {
        rAddress.Column = nCellCol;
        rAddress.Row = nCellRow;
    }
}

void ScMyNotEmptyCellsIterator::SetCellData( ScMyCell& rMyCell, table::CellAddress& rAddress )
{
    rMyCell.aCellAddress = rAddress;
    if( (nCellCol == rAddress.Column) && (nCellRow == rAddress.Row) )
        pCellItr->GetNext( nCellCol, nCellRow );
}

void ScMyNotEmptyCellsIterator::SetMatrixCellData( ScMyCell& rMyCell )
{
    rMyCell.bIsMatrixCovered = sal_False;
    rMyCell.bIsMatrixBase = sal_False;

    uno::Reference< table::XCellRange > xCellRange( xTable, uno::UNO_QUERY );
    if( xCellRange.is() )
    {
        if( rExport.IsMatrix( xCellRange, xTable, rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row,
                rMyCell.aMatrixRange, rMyCell.bIsMatrixBase ) )
            rMyCell.bIsMatrixCovered = !rMyCell.bIsMatrixBase;
    }
}

void ScMyNotEmptyCellsIterator::HasAnnotation(ScMyCell& aCell)
{
    aCell.bHasAnnotation = sal_False;
    uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY);
    if (xCellRange.is())
    {
        aCell.xCell = xCellRange->getCellByPosition(aCell.aCellAddress.Column, aCell.aCellAddress.Row);
        uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor(aCell.xCell, uno::UNO_QUERY);
        if (xSheetAnnotationAnchor.is())
        {
            uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation = xSheetAnnotationAnchor->getAnnotation();
            uno::Reference<text::XSimpleText> xSimpleText(xSheetAnnotation, uno::UNO_QUERY);
            if (xSheetAnnotation.is() && xSimpleText.is())
            {
                OUString sText = xSimpleText->getString();
                if (sText.getLength())
                    aCell.bHasAnnotation = sal_True;
            }
        }
    }
}

void ScMyNotEmptyCellsIterator::SetCurrentTable(const sal_Int32 nTable)
{
    if (nCurrentTable != nTable)
    {
        nCurrentTable = nTable;
        if (pCellItr)
            delete pCellItr;
        pCellItr = new ScHorizontalCellIterator(rExport.GetDocument(), nCurrentTable, 0, 0,
            rExport.GetLastColumn(nCurrentTable), rExport.GetLastRow(nCurrentTable));
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rExport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                sal_Int32 nTableCount = xIndex->getCount();
                if(nCurrentTable < nTableCount)
                {
                    uno::Any aTable = xIndex->getByIndex(nCurrentTable);
                    aTable>>=xTable;
                }
            }
        }
    }
}

sal_Bool ScMyNotEmptyCellsIterator::GetNext(ScMyCell& aCell)
{
    table::CellAddress  aAddress( nCurrentTable, MAXCOL + 1, MAXROW + 1 );

    UpdateAddress( aAddress );
    if( pShapes )
        pShapes->UpdateAddress( aAddress );
    if( pEmptyDatabaseRanges )
        pEmptyDatabaseRanges->UpdateAddress( aAddress );
    if( pMergedRanges )
        pMergedRanges->UpdateAddress( aAddress );
    if( pAreaLinks )
        pAreaLinks->UpdateAddress( aAddress );
    if( pDetectiveObj )
        pDetectiveObj->UpdateAddress( aAddress );
    if( pDetectiveOp )
        pDetectiveOp->UpdateAddress( aAddress );

    sal_Bool bFoundCell = (aAddress.Column <= MAXCOL) && (aAddress.Row <= MAXROW);
    if( bFoundCell )
    {
        SetCellData( aCell, aAddress );
        if( pShapes )
            pShapes->SetCellData( aCell );
        if( pEmptyDatabaseRanges )
            pEmptyDatabaseRanges->SetCellData( aCell );
        if( pMergedRanges )
            pMergedRanges->SetCellData( aCell );
        if( pAreaLinks )
            pAreaLinks->SetCellData( aCell );
        if( pDetectiveObj )
            pDetectiveObj->SetCellData( aCell );
        if( pDetectiveOp )
            pDetectiveOp->SetCellData( aCell );

        SetMatrixCellData( aCell );
        HasAnnotation( aCell );
    }
    return bFoundCell;
}

