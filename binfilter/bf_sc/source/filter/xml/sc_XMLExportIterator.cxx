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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "XMLExportIterator.hxx"

#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>

#include <tools/debug.hxx>


#include "dociter.hxx"
#include "convuno.hxx"
#include "xmlexprt.hxx"
#include "XMLExportSharedData.hxx"
#include "XMLStylesExportHelper.hxx"

#include <algorithm>
namespace binfilter {

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

sal_Bool ScMyShape::operator<(const ScMyShape& aShape) const
{
    if( aAddress.Tab() != aShape.aAddress.Tab() )
        return (aAddress.Tab() < aShape.aAddress.Tab());
    else if( aAddress.Row() != aShape.aAddress.Row() )
        return (aAddress.Row() < aShape.aAddress.Row());
    else
        return (aAddress.Col() < aShape.aAddress.Col());
}

ScMyShapesContainer::ScMyShapesContainer()
    : aShapeList()
{
}

ScMyShapesContainer::~ScMyShapesContainer()
{
}

void ScMyShapesContainer::AddNewShape( const ScMyShape& aShape )
{
    aShapeList.push_back(aShape);
}

sal_Bool ScMyShapesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aShapeList.empty() )
    {
        ScUnoConversion::FillApiAddress( rCellAddress, aShapeList.begin()->aAddress );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyShapesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aShapeList.clear();
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rMyCell.aCellAddress );

    ScMyShapeList::iterator aItr = aShapeList.begin();
    while( (aItr != aShapeList.end()) && (aItr->aAddress == aAddress) )
    {
        rMyCell.aShapeList.push_back(*aItr);
        aItr = aShapeList.erase(aItr);
    }
    rMyCell.bHasShape = !rMyCell.aShapeList.empty();
}

void ScMyShapesContainer::Sort()
{
    aShapeList.sort();
}

//==============================================================================

sal_Bool ScMyMergedRange::operator<(const ScMyMergedRange& aRange) const
{
    if( aCellRange.Sheet != aRange.aCellRange.Sheet )
        return (aCellRange.Sheet < aRange.aCellRange.Sheet);
    else if( aCellRange.StartRow != aRange.aCellRange.StartRow )
        return (aCellRange.StartRow < aRange.aCellRange.StartRow);
    else
        return (aCellRange.StartColumn < aRange.aCellRange.StartColumn);
}


ScMyMergedRangesContainer::ScMyMergedRangesContainer()
    : aRangeList()
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
    aRangeList.push_back( aRange );

    aRange.bIsFirst = sal_False;
    aRange.nRows = 0;
    for( sal_Int32 nRow = nStartRow + 1; nRow <= nEndRow; nRow++ )
    {
        aRange.aCellRange.StartRow = aRange.aCellRange.EndRow = nRow;
        aRangeList.push_back(aRange);
    }
}

sal_Bool ScMyMergedRangesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aRangeList.empty() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, aRangeList.begin()->aCellRange );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyMergedRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bIsMergedBase = rMyCell.bIsCovered = sal_False;
    ScMyMergedRangeList::iterator aItr = aRangeList.begin();
    if( aItr != aRangeList.end() )
    {
        table::CellAddress aFirstAddress;
        ScUnoConversion::FillApiStartAddress( aFirstAddress, aItr->aCellRange );
        if( aFirstAddress == rMyCell.aCellAddress )
        {
            rMyCell.aMergeRange = aItr->aCellRange;
            if (aItr->bIsFirst)
                rMyCell.aMergeRange.EndRow = rMyCell.aMergeRange.StartRow + aItr->nRows - 1;
            rMyCell.bIsMergedBase = aItr->bIsFirst;
            rMyCell.bIsCovered = !aItr->bIsFirst;
            if( aItr->aCellRange.StartColumn < aItr->aCellRange.EndColumn )
            {
                aItr->aCellRange.StartColumn++;
                aItr->bIsFirst = sal_False;
            }
            else
                aRangeList.erase(aItr);
        }
    }
}

void ScMyMergedRangesContainer::Sort()
{
    aRangeList.sort();
}

//==============================================================================

sal_Bool ScMyAreaLink::Compare( const ScMyAreaLink& rAreaLink ) const
{
    return	(GetRowCount() == rAreaLink.GetRowCount()) &&
            (sFilter == rAreaLink.sFilter) &&
            (sFilterOptions == rAreaLink.sFilterOptions) &&
            (sURL == rAreaLink.sURL) &&
            (sSourceStr == rAreaLink.sSourceStr);
}

sal_Bool ScMyAreaLink::operator<(const ScMyAreaLink& rAreaLink ) const
{
    if( aDestRange.Sheet != rAreaLink.aDestRange.Sheet )
        return (aDestRange.Sheet < rAreaLink.aDestRange.Sheet);
    else if( aDestRange.StartRow != rAreaLink.aDestRange.StartRow )
        return (aDestRange.StartRow < rAreaLink.aDestRange.StartRow);
    else
        return (aDestRange.StartColumn < rAreaLink.aDestRange.StartColumn);
}

ScMyAreaLinksContainer::ScMyAreaLinksContainer() :
    aAreaLinkList()
{
}

ScMyAreaLinksContainer::~ScMyAreaLinksContainer()
{
}

sal_Bool ScMyAreaLinksContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aAreaLinkList.empty() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, aAreaLinkList.begin()->aDestRange );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyAreaLinksContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasAreaLink = sal_False;
    ScMyAreaLinkList::iterator aItr = aAreaLinkList.begin();
    if( aItr != aAreaLinkList.end() )
    {
        table::CellAddress aAddress;
        ScUnoConversion::FillApiStartAddress( aAddress, aItr->aDestRange );
        if( aAddress == rMyCell.aCellAddress )
        {
            rMyCell.bHasAreaLink = sal_True;
            rMyCell.aAreaLink = *aItr;
            aAreaLinkList.erase( aItr );
        }
    }
}

void ScMyAreaLinksContainer::Sort()
{
    aAreaLinkList.sort();
}

//==============================================================================

ScMyCellRangeAddress::ScMyCellRangeAddress(const table::CellRangeAddress& rRange)
    : table::CellRangeAddress(rRange)
{
}

sal_Bool ScMyCellRangeAddress::operator<(const ScMyCellRangeAddress& rRange ) const
{
    if( Sheet != rRange.Sheet )
        return (Sheet < rRange.Sheet);
    else if( StartRow != rRange.StartRow )
        return (StartRow < rRange.StartRow);
    else
        return (StartColumn < rRange.StartColumn);
}

ScMyEmptyDatabaseRangesContainer::ScMyEmptyDatabaseRangesContainer()
    : aDatabaseList()
{
}

ScMyEmptyDatabaseRangesContainer::~ScMyEmptyDatabaseRangesContainer()
{
}

void ScMyEmptyDatabaseRangesContainer::AddNewEmptyDatabaseRange(const table::CellRangeAddress& aCellRange)
{
    sal_Int32 nStartRow = aCellRange.StartRow;
    sal_Int32 nEndRow = aCellRange.EndRow;
    ScMyCellRangeAddress aRange( aCellRange );
    for( sal_Int32 nRow = nStartRow; nRow <= nEndRow; nRow++ )
    {
        aRange.StartRow = aRange.EndRow = nRow;
        aDatabaseList.push_back( aRange );
    }
}

sal_Bool ScMyEmptyDatabaseRangesContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aDatabaseList.empty() )
    {
        ScUnoConversion::FillApiStartAddress( rCellAddress, *(aDatabaseList.begin()) );
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyEmptyDatabaseRangesContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.bHasEmptyDatabase = sal_False;
    sal_Int16 nTable = rMyCell.aCellAddress.Sheet;
    ScMyEmptyDatabaseRangeList::iterator aItr = aDatabaseList.begin();
    if( aItr != aDatabaseList.end() )
    {
        table::CellAddress aFirstAddress;
        ScUnoConversion::FillApiStartAddress( aFirstAddress, *aItr );
        if( aFirstAddress == rMyCell.aCellAddress )
        {
            rMyCell.bHasEmptyDatabase = sal_True;
            if( aItr->StartColumn < aItr->EndColumn )
                aItr->StartColumn++;
            else
                aDatabaseList.erase(aItr);
        }
    }
}

void ScMyEmptyDatabaseRangesContainer::Sort()
{
    aDatabaseList.sort();
}

//==============================================================================

sal_Bool ScMyDetectiveObj::operator<( const ScMyDetectiveObj& rDetObj) const
{
    if( aPosition.Sheet != rDetObj.aPosition.Sheet )
        return (aPosition.Sheet < rDetObj.aPosition.Sheet);
    else if( aPosition.Row != rDetObj.aPosition.Row )
        return (aPosition.Row < rDetObj.aPosition.Row);
    else
        return (aPosition.Column < rDetObj.aPosition.Column);
}

ScMyDetectiveObjContainer::ScMyDetectiveObjContainer() :
    aDetectiveObjList()
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
        (eObjType == SC_DETOBJ_TOOTHERTAB) ||
        (eObjType == SC_DETOBJ_CIRCLE) )
    {
        ScMyDetectiveObj aDetObj;
        aDetObj.eObjType = eObjType;
        if( eObjType == SC_DETOBJ_TOOTHERTAB )
            ScUnoConversion::FillApiAddress( aDetObj.aPosition, rSourceRange.aStart );
        else
            ScUnoConversion::FillApiAddress( aDetObj.aPosition, rPosition );
        ScUnoConversion::FillApiRange( aDetObj.aSourceRange, rSourceRange );
        aDetObj.bHasError = bHasError;
        aDetectiveObjList.push_back( aDetObj );
    }
}

sal_Bool ScMyDetectiveObjContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aDetectiveObjList.empty() )
    {
        rCellAddress = aDetectiveObjList.begin()->aPosition;
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyDetectiveObjContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveObjVec.clear();
    ScMyDetectiveObjList::iterator aItr = aDetectiveObjList.begin();
    while( (aItr != aDetectiveObjList.end()) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveObjVec.push_back( *aItr );
        aItr = aDetectiveObjList.erase( aItr );
    }
    rMyCell.bHasDetectiveObj = (rMyCell.aDetectiveObjVec.size() != 0);
}

void ScMyDetectiveObjContainer::Sort()
{
    aDetectiveObjList.sort();
}

//==============================================================================

sal_Bool ScMyDetectiveOp::operator<( const ScMyDetectiveOp& rDetOp) const
{
    if( aPosition.Sheet != rDetOp.aPosition.Sheet )
        return (aPosition.Sheet < rDetOp.aPosition.Sheet);
    else if( aPosition.Row != rDetOp.aPosition.Row )
        return (aPosition.Row < rDetOp.aPosition.Row);
    else
        return (aPosition.Column < rDetOp.aPosition.Column);
}

ScMyDetectiveOpContainer::ScMyDetectiveOpContainer() :
    aDetectiveOpList()
{
}

ScMyDetectiveOpContainer::~ScMyDetectiveOpContainer()
{
}

void ScMyDetectiveOpContainer::AddOperation( ScDetOpType eOpType, const ScAddress& rPosition, sal_uInt32 nIndex )
{
    ScMyDetectiveOp aDetOp;
    aDetOp.eOpType = eOpType;
    ScUnoConversion::FillApiAddress( aDetOp.aPosition, rPosition );
    aDetOp.nIndex = nIndex;
    aDetectiveOpList.push_back( aDetOp );
}

sal_Bool ScMyDetectiveOpContainer::GetFirstAddress( table::CellAddress& rCellAddress )
{
    sal_Int16 nTable = rCellAddress.Sheet;
    if( !aDetectiveOpList.empty() )
    {
        rCellAddress = aDetectiveOpList.begin()->aPosition;
        return (nTable == rCellAddress.Sheet);
    }
    return sal_False;
}

void ScMyDetectiveOpContainer::SetCellData( ScMyCell& rMyCell )
{
    rMyCell.aDetectiveOpVec.clear();
    ScMyDetectiveOpList::iterator aItr = aDetectiveOpList.begin();
    while( (aItr != aDetectiveOpList.end()) && (aItr->aPosition == rMyCell.aCellAddress) )
    {
        rMyCell.aDetectiveOpVec.push_back( *aItr );
        aItr = aDetectiveOpList.erase( aItr );
    }
    rMyCell.bHasDetectiveOp = (rMyCell.aDetectiveOpVec.size() != 0);
}

void ScMyDetectiveOpContainer::Sort()
{
    aDetectiveOpList.sort();
}

//==============================================================================

ScMyCell::ScMyCell() :
    aShapeList(),
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
    bHasAnnotation( sal_False ),
    bIsAutoStyle( sal_False ),
    bIsEditCell( sal_False ),
    bKnowWhetherIsEditCell( sal_False ),
    bHasStringValue( sal_False ),
    bHasDoubleValue( sal_False ),
    bHasXText( sal_False )
{
}

ScMyCell::~ScMyCell()
{
}

//==============================================================================

sal_Bool ScMyExportAnnotation::operator<(const ScMyExportAnnotation& rAnno) const
{
    if( aCellAddress.Row != rAnno.aCellAddress.Row )
        return (aCellAddress.Row < rAnno.aCellAddress.Row);
    else
        return (aCellAddress.Column < rAnno.aCellAddress.Column);
}


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
    Clear();
}

void ScMyNotEmptyCellsIterator::Clear()
{
    if (pCellItr)
        delete pCellItr;
    if (!aAnnotations.empty())
    {
        DBG_ERROR("not all Annotations saved");
        aAnnotations.clear();
    }
    pCellItr = NULL;
    pShapes = NULL;
    pMergedRanges = NULL;
    pAreaLinks = NULL;
    pEmptyDatabaseRanges = NULL;
    pDetectiveObj = NULL;
    pDetectiveOp = NULL;
    nCurrentTable = -1;
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
    rMyCell.bHasStringValue = sal_False;
    rMyCell.bHasDoubleValue = sal_False;
    rMyCell.bHasXText = sal_False;
    rMyCell.bKnowWhetherIsEditCell = sal_False;
    rMyCell.bIsEditCell = sal_False;
    if( (nCellCol == rAddress.Column) && (nCellRow == rAddress.Row) )
        pCellItr->GetNext( nCellCol, nCellRow );
}

void ScMyNotEmptyCellsIterator::SetMatrixCellData( ScMyCell& rMyCell )
{
    rMyCell.bIsMatrixCovered = sal_False;
    rMyCell.bIsMatrixBase = sal_False;

    sal_Bool bIsMatrixBase(sal_False);
    rMyCell.nType = rMyCell.xCell->getType();
    if (rMyCell.nType == table::CellContentType_FORMULA)
        if( rExport.IsMatrix( rMyCell.xCell, xTable, rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row,
                rMyCell.aMatrixRange, bIsMatrixBase ) )
        {
            rMyCell.bIsMatrixBase = bIsMatrixBase;
            rMyCell.bIsMatrixCovered = !bIsMatrixBase;
        }
}

void ScMyNotEmptyCellsIterator::HasAnnotation(ScMyCell& aCell)
{
    aCell.bHasAnnotation = sal_False;
    if (!aAnnotations.empty())
    {
        ScMyExportAnnotationList::iterator aItr = aAnnotations.begin();
        if ((aCell.aCellAddress.Column == aItr->aCellAddress.Column) &&
            (aCell.aCellAddress.Row == aItr->aCellAddress.Row))
        {
            aCell.xAnnotation = aItr->xAnnotation;
            uno::Reference<text::XSimpleText> xSimpleText(aCell.xAnnotation, uno::UNO_QUERY);
            if (aCell.xAnnotation.is() && xSimpleText.is())
            {
                aCell.sAnnotationText = xSimpleText->getString();
                if (aCell.sAnnotationText.getLength())
                    aCell.bHasAnnotation = sal_True;
            }
            aAnnotations.erase(aItr);
        }
    }
    if (xCellRange.is())
        aCell.xCell = xCellRange->getCellByPosition(aCell.aCellAddress.Column, aCell.aCellAddress.Row);
}

void ScMyNotEmptyCellsIterator::SetCurrentTable(const sal_Int32 nTable,
    uno::Reference<sheet::XSpreadsheet>& rxTable)
{
    DBG_ASSERT(aAnnotations.empty(), "not all Annotations saved");
    aLastAddress.Row = 0;
    aLastAddress.Column = 0;
    aLastAddress.Sheet = static_cast<sal_Int16>(nTable);
    if (nCurrentTable != nTable)
    {
        nCurrentTable = static_cast<sal_Int16>(nTable);
        if (pCellItr)
            delete pCellItr;
        pCellItr = new ScHorizontalCellIterator(rExport.GetDocument(), nCurrentTable, 0, 0,
            static_cast<USHORT>(rExport.GetSharedData()->GetLastColumn(nCurrentTable)), static_cast<USHORT>(rExport.GetSharedData()->GetLastRow(nCurrentTable)));
        xTable = rxTable;
        xCellRange = uno::Reference<table::XCellRange>(xTable, uno::UNO_QUERY);
        uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery (xTable, uno::UNO_QUERY);
        uno::Reference<sheet::XSheetAnnotationsSupplier> xSheetAnnotationsSupplier (xTable, uno::UNO_QUERY);
        if (xSheetAnnotationsSupplier.is())
        {
            uno::Reference<container::XEnumerationAccess> xAnnotationAccess ( xSheetAnnotationsSupplier->getAnnotations(), uno::UNO_QUERY);
            if (xAnnotationAccess.is())
            {
                uno::Reference<container::XEnumeration> xAnnotations = xAnnotationAccess->createEnumeration();
                if (xAnnotations.is())
                {
                    while (xAnnotations->hasMoreElements())
                    {
                        uno::Any aAny = xAnnotations->nextElement();
                        ScMyExportAnnotation aAnnotation;
                        if (aAny >>= aAnnotation.xAnnotation)
                        {
                            aAnnotation.aCellAddress = aAnnotation.xAnnotation->getPosition();
                            aAnnotations.push_back(aAnnotation);
                        }
                    }
                    if (!aAnnotations.empty())
                        aAnnotations.sort();
                }
            }
        }
    }
}

sal_Bool ScMyNotEmptyCellsIterator::GetNext(ScMyCell& aCell, ScFormatRangeStyles* pCellStyles)
{
    table::CellAddress	aAddress( nCurrentTable, MAXCOL + 1, MAXROW + 1 );

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

        HasAnnotation( aCell );
        SetMatrixCellData( aCell );
        sal_Bool bIsAutoStyle;
        sal_Bool bRemoveStyleRange((aLastAddress.Row == aCell.aCellAddress.Row) &&
            (aLastAddress.Column + 1 == aCell.aCellAddress.Column));
        aCell.nStyleIndex = pCellStyles->GetStyleNameIndex(aCell.aCellAddress.Sheet,
            aCell.aCellAddress.Column, aCell.aCellAddress.Row,
            bIsAutoStyle, aCell.nValidationIndex, aCell.nNumberFormat, bRemoveStyleRange);
        aLastAddress = aCell.aCellAddress;
        aCell.bIsAutoStyle = bIsAutoStyle;

        //#102799#; if the cell is in a DatabaseRange which should saved empty, the cell should have the type empty
        if (aCell.bHasEmptyDatabase)
            aCell.nType = table::CellContentType_EMPTY;
    }
    return bFoundCell;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
