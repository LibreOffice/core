/*************************************************************************
 *
 *  $RCSfile: cursuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <rtl/uuid.h>

#include "cursuno.hxx"
#include "cellsuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

#define SCSHEETCELLCURSOR_SERVICE   "com.sun.star.sheet.SheetCellCursor"
#define SCCELLCURSOR_SERVICE        "com.sun.star.table.CellCursor"

//------------------------------------------------------------------------

ScCellCursorObj::ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR) :
    ScCellRangeObj( pDocSh, rR )
{
}

ScCellCursorObj::~ScCellCursorObj()
{
}

uno::Any SAL_CALL ScCellCursorObj::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XSheetCellCursor )
    SC_QUERYINTERFACE( sheet::XUsedAreaCursor )
    SC_QUERYINTERFACE( table::XCellCursor )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScCellCursorObj::acquire() throw(uno::RuntimeException)
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScCellCursorObj::release() throw(uno::RuntimeException)
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellCursorObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScCellRangeObj::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 3 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSheetCellCursor>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<sheet::XUsedAreaCursor>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<table::XCellCursor>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellCursorObj::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XSheetCellCursor

void SAL_CALL ScCellCursorObj::collapseToCurrentRegion() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nStartCol = aRange.aStart.Col();
        USHORT nStartRow = aRange.aStart.Row();
        USHORT nEndCol = aRange.aEnd.Col();
        USHORT nEndRow = aRange.aEnd.Row();
        USHORT nTab = aRange.aStart.Tab();

        pDocSh->GetDocument()->GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, TRUE );

        ScRange aNew( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::collapseToCurrentArray() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScAddress aCursor = aRange.aStart;      //  bei Block immer den Start nehmen
    ScAddress aOrigin = aCursor;
    BOOL bFound = FALSE;

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        ScBaseCell* pCell = pDoc->GetCell( aCursor );
        if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            if ( ((ScFormulaCell*)pCell)->GetMatrixOrigin( aOrigin ) )
            {
                if ( aOrigin != aCursor )
                    pCell = pDoc->GetCell( aOrigin );
                if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    USHORT nSizeX, nSizeY;
                    ((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
                    if ( !(nSizeX && nSizeY) )
                    {
                        // GetMatrixEdge computes also dimensions of the matrix
                        // if not already done (may occur if document is loaded
                        // from old file format).
                        // Needs an "invalid" initialized address.
                        aOrigin = UINT32(0xffffffff);
                        ((ScFormulaCell*)pCell)->GetMatrixEdge(aOrigin);
                        ((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
                    }
                    if ( nSizeX && nSizeY )
                    {
                        ScAddress aEnd( aOrigin.Col() + nSizeX - 1,
                                        aOrigin.Row() + nSizeY - 1,
                                        aOrigin.Tab() );
                        ScRange aNew( aOrigin, aEnd );
                        SetNewRange( aNew );
                        bFound = TRUE;
                    }
                }
            }
        }
    }
    // thats a Bug, that this assertion comes; the API Reference says, that
    // if there is no Matrix, the Range is left unchanged; they says nothing
    // about a exception
    /*if (!bFound)
    {
        DBG_ERROR("keine Matrix");
        //! Exception, oder was?
    }*/
}

void SAL_CALL ScCellCursorObj::collapseToMergedArea() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
        ScRange aNewRange = *rRanges.GetObject(0);

        ScDocument* pDoc = pDocSh->GetDocument();
        pDoc->ExtendOverlapped( aNewRange );
        pDoc->ExtendMerge( aNewRange );                 // after ExtendOverlapped!

        SetNewRange( aNewRange );
    }
}

void SAL_CALL ScCellCursorObj::expandToEntireColumns() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aNewRange = *rRanges.GetObject(0);

    aNewRange.aStart.SetRow( 0 );
    aNewRange.aEnd.SetRow( MAXROW );

    SetNewRange( aNewRange );
}

void SAL_CALL ScCellCursorObj::expandToEntireRows() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aNewRange = *rRanges.GetObject(0);

    aNewRange.aStart.SetCol( 0 );
    aNewRange.aEnd.SetCol( MAXCOL );

    SetNewRange( aNewRange );
}

void SAL_CALL ScCellCursorObj::collapseToSize( sal_Int32 nColumns, sal_Int32 nRows )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( nColumns <= 0 || nRows <= 0 )
    {
        DBG_ERROR("leerer Range geht nicht");
        //! und dann?
    }
    else
    {
        const ScRangeList& rRanges = GetRangeList();
        DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
        ScRange aNewRange = *rRanges.GetObject(0);

        aNewRange.Justify();    //! wirklich?

        long nEndX = aNewRange.aStart.Col() + nColumns - 1;
        long nEndY = aNewRange.aStart.Row() + nRows - 1;
        if ( nEndX < 0 )      nEndX = 0;
        if ( nEndX > MAXCOL ) nEndX = MAXCOL;
        if ( nEndY < 0 )      nEndY = 0;
        if ( nEndY > MAXROW ) nEndY = MAXROW;
        //! Fehler/Exception oder so, wenn zu gross/zu klein?

        aNewRange.aEnd.SetCol(nEndX);
        aNewRange.aEnd.SetRow(nEndY);

        aNewRange.Justify();    //! wirklich?

        SetNewRange( aNewRange );
    }
}

// XUsedAreaCursor

void SAL_CALL ScCellCursorObj::gotoStartOfUsedArea( sal_Bool bExpand )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
        ScRange aNewRange = *rRanges.GetObject(0);
        USHORT nTab = aNewRange.aStart.Tab();

        USHORT nUsedX = 0;      // Anfang holen
        USHORT nUsedY = 0;
        if (!pDocSh->GetDocument()->GetDataStart( nTab, nUsedX, nUsedY ))
            nUsedX = nUsedY = 0;

        aNewRange.aStart.SetCol( nUsedX );
        aNewRange.aStart.SetRow( nUsedY );
        if (!bExpand)
            aNewRange.aEnd = aNewRange.aStart;
        SetNewRange( aNewRange );
    }
}

void SAL_CALL ScCellCursorObj::gotoEndOfUsedArea( sal_Bool bExpand )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
        ScRange aNewRange = *rRanges.GetObject(0);
        USHORT nTab = aNewRange.aStart.Tab();

        USHORT nUsedX = 0;      // Ende holen
        USHORT nUsedY = 0;
        if (!pDocSh->GetDocument()->GetTableArea( nTab, nUsedX, nUsedY ))
            nUsedX = nUsedY = 0;

        aNewRange.aEnd.SetCol( nUsedX );
        aNewRange.aEnd.SetRow( nUsedY );
        if (!bExpand)
            aNewRange.aStart = aNewRange.aEnd;
        SetNewRange( aNewRange );
    }
}

// XCellCursor

void SAL_CALL ScCellCursorObj::gotoStart() throw(uno::RuntimeException)
{
    //  this is similar to collapseToCurrentRegion
    //! something like gotoEdge with 4 possible directions is needed

    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nStartCol = aRange.aStart.Col();
        USHORT nStartRow = aRange.aStart.Row();
        USHORT nEndCol = aRange.aEnd.Col();
        USHORT nEndRow = aRange.aEnd.Row();
        USHORT nTab = aRange.aStart.Tab();

        pDocSh->GetDocument()->GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE );

        ScRange aNew( nStartCol, nStartRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::gotoEnd() throw(uno::RuntimeException)
{
    //  this is similar to collapseToCurrentRegion
    //! something like gotoEdge with 4 possible directions is needed

    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        USHORT nStartCol = aRange.aStart.Col();
        USHORT nStartRow = aRange.aStart.Row();
        USHORT nEndCol = aRange.aEnd.Col();
        USHORT nEndRow = aRange.aEnd.Row();
        USHORT nTab = aRange.aStart.Tab();

        pDocSh->GetDocument()->GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, FALSE );

        ScRange aNew( nEndCol, nEndRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::gotoNext() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScAddress aCursor = aRange.aStart;      //  bei Block immer den Start nehmen

    ScMarkData aMark;   // not used with bMarked=FALSE
    USHORT nNewX = aCursor.Col();
    USHORT nNewY = aCursor.Row();
    USHORT nTab  = aCursor.Tab();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument()->GetNextPos( nNewX,nNewY, nTab,  1,0, FALSE,TRUE, aMark );
    //! sonst Exception oder so

    SetNewRange( ScRange( nNewX, nNewY, nTab ) );
}

void SAL_CALL ScCellCursorObj::gotoPrevious() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);

    aRange.Justify();
    ScAddress aCursor = aRange.aStart;      //  bei Block immer den Start nehmen

    ScMarkData aMark;   // not used with bMarked=FALSE
    USHORT nNewX = aCursor.Col();
    USHORT nNewY = aCursor.Row();
    USHORT nTab  = aCursor.Tab();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument()->GetNextPos( nNewX,nNewY, nTab, -1,0, FALSE,TRUE, aMark );
    //! sonst Exception oder so

    SetNewRange( ScRange( nNewX, nNewY, nTab ) );
}

void SAL_CALL ScCellCursorObj::gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    DBG_ASSERT( rRanges.Count() == 1, "Range? Ranges?" );
    ScRange aRange = *rRanges.GetObject(0);
    aRange.Justify();

    if ( aRange.aStart.Col() + nColumnOffset >= 0 &&
         aRange.aEnd.Col()   + nColumnOffset <= MAXCOL &&
         aRange.aStart.Row() + nRowOffset    >= 0 &&
         aRange.aEnd.Row()   + nRowOffset    <= MAXROW )
    {
        ScRange aNew( aRange.aStart.Col() + nColumnOffset,
                      aRange.aStart.Row() + nRowOffset,
                      aRange.aStart.Tab(),
                      aRange.aEnd.Col() + nColumnOffset,
                      aRange.aEnd.Row() + nRowOffset,
                      aRange.aEnd.Tab() );
        SetNewRange( aNew );
    }
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScCellCursorObj::getSpreadsheet()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getSpreadsheet();
}

// XCellRange

uno::Reference<table::XCell> SAL_CALL ScCellCursorObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellByPosition(nColumn,nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellCursorObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellRangeByPosition(nLeft,nTop,nRight,nBottom);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellCursorObj::getCellRangeByName(
                        const rtl::OUString& aRange ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ScCellRangeObj::getCellRangeByName(aRange);
}

// XServiceInfo

rtl::OUString SAL_CALL ScCellCursorObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScCellCursorObj" );
}

sal_Bool SAL_CALL ScCellCursorObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCSHEETCELLCURSOR_SERVICE ) ||
           aServiceStr.EqualsAscii( SCCELLCURSOR_SERVICE ) ||
           ScCellRangeObj::supportsService(rServiceName);
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellCursorObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    //  get all service names from cell range
    uno::Sequence<rtl::OUString> aParentSeq = ScCellRangeObj::getSupportedServiceNames();
    UINT32 nParentLen = aParentSeq.getLength();
    const rtl::OUString* pParentArr = aParentSeq.getConstArray();

    //  SheetCellCursor should be first (?)
    uno::Sequence<rtl::OUString> aTotalSeq( nParentLen + 2 );
    rtl::OUString* pTotalArr = aTotalSeq.getArray();
    pTotalArr[0] = rtl::OUString::createFromAscii( SCSHEETCELLCURSOR_SERVICE );
    pTotalArr[1] = rtl::OUString::createFromAscii( SCCELLCURSOR_SERVICE );

    //  append cell range services
    for (long i=0; i<nParentLen; i++)
        pTotalArr[i+2] = pParentArr[i];

    return aTotalSeq;
}




