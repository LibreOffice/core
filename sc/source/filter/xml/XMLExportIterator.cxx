/*************************************************************************
 *
 *  $RCSfile: XMLExportIterator.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-02 17:28:42 $
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
#ifndef SC_XMLEXPORTSHAREDDATA_HXX
#include "XMLExportSharedData.hxx"
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

sal_Bool ScMyShape::operator<(const ScMyShape& aShape)
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
    rMyCell.aShapeVec.clear();
    ScAddress aAddress;
    ScUnoConversion::FillScAddress( aAddress, rMyCell.aCellAddress );

    ScMyShapeList::iterator aItr = aShapeList.begin();
    while( (aItr != aShapeList.end()) && (aItr->aAddress == aAddress) )
    {
        rMyCell.aShapeVec.push_back(*aItr);
        aItr = aShapeList.erase(aItr);
    }
    rMyCell.bHasShape = (rMyCell.aShapeVec.size() != 0);
}

void ScMyShapesContainer::Sort()
{
    aShapeList.sort();
}

//==============================================================================

sal_Bool ScMyMergedRange::operator<(const ScMyMergedRange& aRange)
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
    ScMyMergedRangeList::iterator aItr2_old = aRangeList.begin();
    ScMyMergedRangeList::iterator aItr2 = aItr2_old;
    while(aItr2 != aRangeList.end())
    {
        if (aItr2 != aItr2_old)
        {
            if ((aItr2->aCellRange.StartColumn == aItr2_old->aCellRange.StartColumn) &&
                (aItr2->aCellRange.StartRow == aItr2_old->aCellRange.StartRow))
                aItr2 = aRangeList.erase(aItr2);
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

sal_Bool ScMyAreaLink::operator<(const ScMyAreaLink& rAreaLink )
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

sal_Bool ScMyCellRangeAddress::operator<(const ScMyCellRangeAddress& rRange )
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

sal_Bool ScMyDetectiveObj::operator<( const ScMyDetectiveObj& rDetObj)
{
    if( aPosition.Sheet != rDetObj.aPosition.Sheet )
        return (aPosition.Sheet < rDetObj.aPosition.Sheet);
    else if( aPosition.Row != rDetObj.aPosition.Row )
        return (aPosition.Row < rDetObj.aPosition.Row);
    else
        return (aPosition.Column != rDetObj.aPosition.Column);
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

sal_Bool ScMyDetectiveOp::operator<( const ScMyDetectiveOp& rDetOp)
{
    if( aPosition.Sheet != rDetOp.aPosition.Sheet )
        return (aPosition.Sheet < rDetOp.aPosition.Sheet);
    else if( aPosition.Row != rDetOp.aPosition.Row )
        return (aPosition.Row < rDetOp.aPosition.Row);
    else
        return (aPosition.Column != rDetOp.aPosition.Column);
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
        sal_Bool bIsMatrixBase(sal_False);
        if( rExport.IsMatrix( xCellRange, xTable, rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row,
                rMyCell.aMatrixRange, bIsMatrixBase ) )
        {
            rMyCell.bIsMatrixBase = bIsMatrixBase;
            rMyCell.bIsMatrixCovered = !bIsMatrixBase;
        }
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
        nCurrentTable = static_cast<sal_Int16>(nTable);
        if (pCellItr)
            delete pCellItr;
        pCellItr = new ScHorizontalCellIterator(rExport.GetDocument(), nCurrentTable, 0, 0,
            static_cast<USHORT>(rExport.GetSharedData()->GetLastColumn(nCurrentTable)), static_cast<USHORT>(rExport.GetSharedData()->GetLastRow(nCurrentTable)));
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

