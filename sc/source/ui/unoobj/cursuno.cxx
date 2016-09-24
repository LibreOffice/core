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

#include "scitems.hxx"
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "cursuno.hxx"
#include "cellsuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "dociter.hxx"
#include "miscuno.hxx"

using namespace com::sun::star;

#define SCSHEETCELLCURSOR_SERVICE   "com.sun.star.sheet.SheetCellCursor"
#define SCCELLCURSOR_SERVICE        "com.sun.star.table.CellCursor"

ScCellCursorObj::ScCellCursorObj(ScDocShell* pDocSh, const ScRange& rR) :
    ScCellRangeObj( pDocSh, rR )
{
}

ScCellCursorObj::~ScCellCursorObj()
{
}

uno::Any SAL_CALL ScCellCursorObj::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException, std::exception)
{
    SC_QUERYINTERFACE( sheet::XSheetCellCursor )
    SC_QUERYINTERFACE( sheet::XUsedAreaCursor )
    SC_QUERYINTERFACE( table::XCellCursor )

    return ScCellRangeObj::queryInterface( rType );
}

void SAL_CALL ScCellCursorObj::acquire() throw()
{
    ScCellRangeObj::acquire();
}

void SAL_CALL ScCellCursorObj::release() throw()
{
    ScCellRangeObj::release();
}

uno::Sequence<uno::Type> SAL_CALL ScCellCursorObj::getTypes() throw(uno::RuntimeException, std::exception)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(ScCellRangeObj::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 3 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = cppu::UnoType<sheet::XSheetCellCursor>::get();
        pPtr[nParentLen + 1] = cppu::UnoType<sheet::XUsedAreaCursor>::get();
        pPtr[nParentLen + 2] = cppu::UnoType<table::XCellCursor>::get();

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScCellCursorObj::getImplementationId() throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XSheetCellCursor

void SAL_CALL ScCellCursorObj::collapseToCurrentRegion() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );

    aOneRange.PutInOrder();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCCOL nStartCol = aOneRange.aStart.Col();
        SCROW nStartRow = aOneRange.aStart.Row();
        SCCOL nEndCol = aOneRange.aEnd.Col();
        SCROW nEndRow = aOneRange.aEnd.Row();
        SCTAB nTab = aOneRange.aStart.Tab();

        pDocSh->GetDocument().GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, true, false );

        ScRange aNew( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::collapseToCurrentArray()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );

    aOneRange.PutInOrder();
    ScAddress aCursor(aOneRange.aStart);        //  use the start address of the range

    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        ScDocument& rDoc = pDocSh->GetDocument();
        ScRange aMatrix;

        // finding the matrix range is now in GetMatrixFormulaRange in the document
        if ( rDoc.GetMatrixFormulaRange( aCursor, aMatrix ) )
        {
            SetNewRange( aMatrix );
        }
    }
    // thats a Bug, that this assertion comes; the API Reference says, that
    // if there is no Matrix, the Range is left unchanged; they says nothing
    // about a exception
    /*if (!bFound)
    {
        OSL_FAIL("keine Matrix");
        //! Exception, oder was?
    }*/
}

void SAL_CALL ScCellCursorObj::collapseToMergedArea() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
        ScRange aNewRange( *rRanges[ 0 ] );

        ScDocument& rDoc = pDocSh->GetDocument();
        rDoc.ExtendOverlapped( aNewRange );
        rDoc.ExtendMerge( aNewRange );                 // after ExtendOverlapped!

        SetNewRange( aNewRange );
    }
}

void SAL_CALL ScCellCursorObj::expandToEntireColumns() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aNewRange( *rRanges[ 0 ] );

    aNewRange.aStart.SetRow( 0 );
    aNewRange.aEnd.SetRow( MAXROW );

    SetNewRange( aNewRange );
}

void SAL_CALL ScCellCursorObj::expandToEntireRows() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aNewRange( *rRanges[ 0 ] );

    aNewRange.aStart.SetCol( 0 );
    aNewRange.aEnd.SetCol( MAXCOL );

    SetNewRange( aNewRange );
}

void SAL_CALL ScCellCursorObj::collapseToSize( sal_Int32 nColumns, sal_Int32 nRows )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( nColumns <= 0 || nRows <= 0 )
    {
        OSL_FAIL("leerer Range geht nicht");
        //! und dann?
    }
    else
    {
        const ScRangeList& rRanges = GetRangeList();
        OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
        ScRange aNewRange( *rRanges[ 0 ] );

        aNewRange.PutInOrder();    //! wirklich?

        long nEndX = aNewRange.aStart.Col() + nColumns - 1;
        long nEndY = aNewRange.aStart.Row() + nRows - 1;
        if ( nEndX < 0 )      nEndX = 0;
        if ( nEndX > MAXCOL ) nEndX = MAXCOL;
        if ( nEndY < 0 )      nEndY = 0;
        if ( nEndY > MAXROW ) nEndY = MAXROW;
        //! Fehler/Exception oder so, wenn zu gross/zu klein?

        aNewRange.aEnd.SetCol((SCCOL)nEndX);
        aNewRange.aEnd.SetRow((SCROW)nEndY);

        aNewRange.PutInOrder();    //! wirklich?

        SetNewRange( aNewRange );
    }
}

// XUsedAreaCursor

void SAL_CALL ScCellCursorObj::gotoStartOfUsedArea(sal_Bool bExpand)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
        ScRange aNewRange( *rRanges[0] );
        SCTAB nTab = aNewRange.aStart.Tab();

        SCCOL nUsedX = 0;       // Anfang holen
        SCROW nUsedY = 0;
        if (!pDocSh->GetDocument().GetDataStart( nTab, nUsedX, nUsedY ))
        {
            nUsedX = 0;
            nUsedY = 0;
        }

        aNewRange.aStart.SetCol( nUsedX );
        aNewRange.aStart.SetRow( nUsedY );
        if (!bExpand)
            aNewRange.aEnd = aNewRange.aStart;
        SetNewRange( aNewRange );
    }
}

void SAL_CALL ScCellCursorObj::gotoEndOfUsedArea( sal_Bool bExpand )
                                            throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        const ScRangeList& rRanges = GetRangeList();
        OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
        ScRange aNewRange( *rRanges[ 0 ]);
        SCTAB nTab = aNewRange.aStart.Tab();

        SCCOL nUsedX = 0;       // Ende holen
        SCROW nUsedY = 0;
        if (!pDocSh->GetDocument().GetTableArea( nTab, nUsedX, nUsedY ))
        {
            nUsedX = 0;
            nUsedY = 0;
        }

        aNewRange.aEnd.SetCol( nUsedX );
        aNewRange.aEnd.SetRow( nUsedY );
        if (!bExpand)
            aNewRange.aStart = aNewRange.aEnd;
        SetNewRange( aNewRange );
    }
}

// XCellCursor

void SAL_CALL ScCellCursorObj::gotoStart() throw(uno::RuntimeException, std::exception)
{
    //  this is similar to collapseToCurrentRegion
    //! something like gotoEdge with 4 possible directions is needed

    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ]);

    aOneRange.PutInOrder();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCCOL nStartCol = aOneRange.aStart.Col();
        SCROW nStartRow = aOneRange.aStart.Row();
        SCCOL nEndCol = aOneRange.aEnd.Col();
        SCROW nEndRow = aOneRange.aEnd.Row();
        SCTAB nTab = aOneRange.aStart.Tab();

        pDocSh->GetDocument().GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, false, false );

        ScRange aNew( nStartCol, nStartRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::gotoEnd() throw(uno::RuntimeException, std::exception)
{
    //  this is similar to collapseToCurrentRegion
    //! something like gotoEdge with 4 possible directions is needed

    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );

    aOneRange.PutInOrder();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
    {
        SCCOL nStartCol = aOneRange.aStart.Col();
        SCROW nStartRow = aOneRange.aStart.Row();
        SCCOL nEndCol = aOneRange.aEnd.Col();
        SCROW nEndRow = aOneRange.aEnd.Row();
        SCTAB nTab = aOneRange.aStart.Tab();

        pDocSh->GetDocument().GetDataArea(
                        nTab, nStartCol, nStartRow, nEndCol, nEndRow, false, false );

        ScRange aNew( nEndCol, nEndRow, nTab );
        SetNewRange( aNew );
    }
}

void SAL_CALL ScCellCursorObj::gotoNext() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );

    aOneRange.PutInOrder();
    ScAddress aCursor(aOneRange.aStart);        //  bei Block immer den Start nehmen

    ScMarkData aMark;   // not used with bMarked=FALSE
    SCCOL nNewX = aCursor.Col();
    SCROW nNewY = aCursor.Row();
    SCTAB nTab  = aCursor.Tab();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument().GetNextPos( nNewX,nNewY, nTab,  1,0, false,true, aMark );
    //! sonst Exception oder so

    SetNewRange( ScRange( nNewX, nNewY, nTab ) );
}

void SAL_CALL ScCellCursorObj::gotoPrevious() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );

    aOneRange.PutInOrder();
    ScAddress aCursor(aOneRange.aStart);        //  bei Block immer den Start nehmen

    ScMarkData aMark;   // not used with bMarked=FALSE
    SCCOL nNewX = aCursor.Col();
    SCROW nNewY = aCursor.Row();
    SCTAB nTab  = aCursor.Tab();
    ScDocShell* pDocSh = GetDocShell();
    if ( pDocSh )
        pDocSh->GetDocument().GetNextPos( nNewX,nNewY, nTab, -1,0, false,true, aMark );
    //! sonst Exception oder so

    SetNewRange( ScRange( nNewX, nNewY, nTab ) );
}

void SAL_CALL ScCellCursorObj::gotoOffset( sal_Int32 nColumnOffset, sal_Int32 nRowOffset )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const ScRangeList& rRanges = GetRangeList();
    OSL_ENSURE( rRanges.size() == 1, "Range? Ranges?" );
    ScRange aOneRange( *rRanges[ 0 ] );
    aOneRange.PutInOrder();

    if ( aOneRange.aStart.Col() + nColumnOffset >= 0 &&
         aOneRange.aEnd.Col()   + nColumnOffset <= MAXCOL &&
         aOneRange.aStart.Row() + nRowOffset    >= 0 &&
         aOneRange.aEnd.Row()   + nRowOffset    <= MAXROW )
    {
        ScRange aNew( (SCCOL)(aOneRange.aStart.Col() + nColumnOffset),
                      (SCROW)(aOneRange.aStart.Row() + nRowOffset),
                      aOneRange.aStart.Tab(),
                      (SCCOL)(aOneRange.aEnd.Col() + nColumnOffset),
                      (SCROW)(aOneRange.aEnd.Row() + nRowOffset),
                      aOneRange.aEnd.Tab() );
        SetNewRange( aNew );
    }
}

// XSheetCellRange

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScCellCursorObj::getSpreadsheet()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getSpreadsheet();
}

// XCellRange

uno::Reference<table::XCell> SAL_CALL ScCellCursorObj::getCellByPosition(
                                        sal_Int32 nColumn, sal_Int32 nRow )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getCellByPosition(nColumn,nRow);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellCursorObj::getCellRangeByPosition(
                sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
                                throw(lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getCellRangeByPosition(nLeft,nTop,nRight,nBottom);
}

uno::Reference<table::XCellRange> SAL_CALL ScCellCursorObj::getCellRangeByName(
                        const OUString& rRange ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ScCellRangeObj::getCellRangeByName(rRange);
}

// XServiceInfo

OUString SAL_CALL ScCellCursorObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString( "ScCellCursorObj" );
}

sal_Bool SAL_CALL ScCellCursorObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellCursorObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    //  get all service names from cell range
    uno::Sequence<OUString> aParentSeq(ScCellRangeObj::getSupportedServiceNames());
    sal_Int32 nParentLen = aParentSeq.getLength();
    const OUString* pParentArr = aParentSeq.getConstArray();

    //  SheetCellCursor should be first (?)
    uno::Sequence<OUString> aTotalSeq( nParentLen + 2 );
    OUString* pTotalArr = aTotalSeq.getArray();
    pTotalArr[0] = SCSHEETCELLCURSOR_SERVICE;
    pTotalArr[1] = SCCELLCURSOR_SERVICE;

    //  append cell range services
    for (long i=0; i<nParentLen; i++)
        pTotalArr[i+2] = pParentArr[i];

    return aTotalSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
