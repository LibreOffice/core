/*************************************************************************
 *
 *  $RCSfile: dapiuno.cxx,v $
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

#include <svtools/smplhint.hxx>
#include <rtl/uuid.h>

#include "dapiuno.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "pivot.hxx"
#include "rangeutl.hxx"
#include "unoguard.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDataPilotFieldMap()
{
    static SfxItemPropertyMap aDataPilotFieldMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_FUNCTION), 0,  &getCppuType((sheet::GeneralFunction*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ORIENT),   0,  &getCppuType((sheet::DataPilotFieldOrientation*)0), 0, 0 },
        {0,0,0,0}
    };
    return aDataPilotFieldMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScDataPilotDescriptor, "ScDataPilotDescriptor", "stardiv::one::sheet::DataPilotDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldObj, "ScDataPilotFieldObj", "com.sun.star.sheet.DataPilotField" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldsObj, "ScDataPilotFieldsObj", "com.sun.star.sheet.DataPilotFields" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTableObj, "ScDataPilotTableObj", "com.sun.star.sheet.DataPilotTable" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTablesObj, "ScDataPilotTablesObj", "com.sun.star.sheet.DataPilotTables" )

//------------------------------------------------------------------------

//! irgendwann ueberall die neuen enum-Werte benutzen
#define DATA_PILOT_HIDDEN   sheet::DataPilotFieldOrientation_HIDDEN
#define DATA_PILOT_COLUMN   sheet::DataPilotFieldOrientation_COLUMN
#define DATA_PILOT_ROW      sheet::DataPilotFieldOrientation_ROW
#define DATA_PILOT_PAGE     sheet::DataPilotFieldOrientation_PAGE
#define DATA_PILOT_DATA     sheet::DataPilotFieldOrientation_DATA

//------------------------------------------------------------------------

USHORT lcl_BitCount( USHORT nBits )
{
    if (!nBits) return 0;

    USHORT nCount = 0;
    USHORT nMask = 1;
    for (USHORT i=0; i<16; i++)
    {
        if ( nBits & nMask )
            ++nCount;
        nMask <<= 1;
    }
    return nCount;
}

USHORT lcl_DataCount( const ScPivotParam& rParam )
{
    USHORT nRet = 0;
    for ( USHORT i=0; i<rParam.nDataCount; i++ )
        nRet += lcl_BitCount( rParam.aDataArr[i].nFuncMask );   // nFuncCount stimmt nicht
    return nRet;
}

BOOL lcl_GetDataArrayPos( const ScPivotParam& rParam, USHORT nIndex,
                            USHORT& rArrayPos, USHORT& rFuncBit )
{
    USHORT nPos = 0;
    for ( USHORT i=0; i<rParam.nDataCount; i++ )
    {
        USHORT nBits = rParam.aDataArr[i].nFuncMask;
        USHORT nMask = 1;
        for (USHORT nBP=0; nBP<16; nBP++)
        {
            if ( nBits & nMask )
            {
                if ( nPos == nIndex )
                {
                    rArrayPos = nPos;
                    rFuncBit = nMask;
                    return TRUE;
                }
                ++nPos;
            }
            nMask <<= 1;
        }
    }
    rArrayPos = 0;
    rFuncBit = 0;
    return FALSE;
}

ScDPObject* lcl_GetDPObject( ScDocShell* pDocShell, USHORT nTab, const String& rName )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() &&
                     pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == rName )
                    return pDPObj;
            }
        }
    }
    return NULL;    // nicht gefunden
}

String lcl_ColumnTitle( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab )
{
    //  Spaltennamen, wie sie in der Pivottabelle angezeigt werden
    String aStr;
    pDoc->GetString(nCol, nRow, nTab, aStr);
    if (aStr.Len() == 0)
        aStr = ColToAlpha( nCol );
    return aStr;
}

sheet::GeneralFunction ScDataPilotConversion::FirstFunc( USHORT nBits )
{
    if ( nBits & PIVOT_FUNC_SUM )       return sheet::GeneralFunction_SUM;
    if ( nBits & PIVOT_FUNC_COUNT )     return sheet::GeneralFunction_COUNT;
    if ( nBits & PIVOT_FUNC_AVERAGE )   return sheet::GeneralFunction_AVERAGE;
    if ( nBits & PIVOT_FUNC_MAX )       return sheet::GeneralFunction_MAX;
    if ( nBits & PIVOT_FUNC_MIN )       return sheet::GeneralFunction_MIN;
    if ( nBits & PIVOT_FUNC_PRODUCT )   return sheet::GeneralFunction_PRODUCT;
    if ( nBits & PIVOT_FUNC_COUNT_NUM ) return sheet::GeneralFunction_COUNTNUMS;
    if ( nBits & PIVOT_FUNC_STD_DEV )   return sheet::GeneralFunction_STDEV;
    if ( nBits & PIVOT_FUNC_STD_DEVP )  return sheet::GeneralFunction_STDEVP;
    if ( nBits & PIVOT_FUNC_STD_VAR )   return sheet::GeneralFunction_VAR;
    if ( nBits & PIVOT_FUNC_STD_VARP )  return sheet::GeneralFunction_VARP;
    if ( nBits & PIVOT_FUNC_AUTO )      return sheet::GeneralFunction_AUTO;
    return sheet::GeneralFunction_NONE;
}

USHORT ScDataPilotConversion::FunctionBit( sheet::GeneralFunction eFunc )
{
    USHORT nRet = PIVOT_FUNC_NONE;  // 0
    switch (eFunc)
    {
        case sheet::GeneralFunction_SUM:        nRet = PIVOT_FUNC_SUM;       break;
        case sheet::GeneralFunction_COUNT:      nRet = PIVOT_FUNC_COUNT;     break;
        case sheet::GeneralFunction_AVERAGE:    nRet = PIVOT_FUNC_AVERAGE;   break;
        case sheet::GeneralFunction_MAX:        nRet = PIVOT_FUNC_MAX;       break;
        case sheet::GeneralFunction_MIN:        nRet = PIVOT_FUNC_MIN;       break;
        case sheet::GeneralFunction_PRODUCT:    nRet = PIVOT_FUNC_PRODUCT;   break;
        case sheet::GeneralFunction_COUNTNUMS:  nRet = PIVOT_FUNC_COUNT_NUM; break;
        case sheet::GeneralFunction_STDEV:      nRet = PIVOT_FUNC_STD_DEV;   break;
        case sheet::GeneralFunction_STDEVP:     nRet = PIVOT_FUNC_STD_DEVP;  break;
        case sheet::GeneralFunction_VAR:        nRet = PIVOT_FUNC_STD_VAR;   break;
        case sheet::GeneralFunction_VARP:       nRet = PIVOT_FUNC_STD_VARP;  break;
        case sheet::GeneralFunction_AUTO:       nRet = PIVOT_FUNC_AUTO;      break;
    }
    return nRet;
}

String lcl_CreatePivotName( ScDocShell* pDocShell )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
            return pColl->CreateNewName();
    }
    return String();                    // sollte nicht vorkommen
}

//------------------------------------------------------------------------

ScDataPilotTablesObj::ScDataPilotTablesObj(ScDocShell* pDocSh, USHORT nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDataPilotTablesObj::~ScDataPilotTablesObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDataPilotTablesObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //! Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDataPilotTables

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByIndex_Impl(USHORT nIndex)
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!
            USHORT nFound = 0;
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                {
                    if ( nFound == nIndex )
                    {
                        String aName = pDPObj->GetName();
                        return new ScDataPilotTableObj( pDocShell, nTab, aName );
                    }
                    ++nFound;
                }
            }
        }
    }
    return NULL;
}

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    if (hasByName(aName))
    {
        String aNamStr = aName;
        return new ScDataPilotTableObj( pDocShell, nTab, aNamStr );
    }
    return NULL;
}

uno::Reference<sheet::XDataPilotDescriptor> SAL_CALL ScDataPilotTablesObj::createDataPilotDescriptor()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
        return new ScDataPilotDescriptor(pDocShell);
    return NULL;
}

void SAL_CALL ScDataPilotTablesObj::insertNewByName( const rtl::OUString& aNewName,
                                    const table::CellAddress& aOutputAddress,
                                    const uno::Reference<sheet::XDataPilotDescriptor>& xDescriptor )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!xDescriptor.is()) return;

    // inserting with already existing name?
    if ( aNewName.getLength() && hasByName( aNewName ) )
        throw uno::RuntimeException();      // no other exceptions specified

    BOOL bDone = FALSE;
    ScDataPilotDescriptorBase* pImp = ScDataPilotDescriptorBase::getImplementation( xDescriptor );
    if ( pDocShell && pImp )
    {
        ScPivotParam aParam;
        ScQueryParam aQuery;
        ScArea aSrcArea;
        pImp->GetParam( aParam, aQuery, aSrcArea );

        aParam.nCol = aOutputAddress.Column;
        aParam.nRow = aOutputAddress.Row;
        aParam.nTab = aOutputAddress.Sheet;

        //  in den Uno-Objekten sind alle Fields in den Descriptoren innerhalb des Bereichs gezaehlt

        short nFieldStart = aSrcArea.nColStart;
        USHORT i;
        for ( i=0; i<aParam.nColCount; i++ )
            if ( aParam.aColArr[i].nCol != PIVOT_DATA_FIELD )
                aParam.aColArr[i].nCol += nFieldStart;
        for ( i=0; i<aParam.nRowCount; i++ )
            if ( aParam.aRowArr[i].nCol != PIVOT_DATA_FIELD )
                aParam.aRowArr[i].nCol += nFieldStart;
        for ( i=0; i<aParam.nDataCount; i++ )
            if ( aParam.aDataArr[i].nCol != PIVOT_DATA_FIELD )
                aParam.aDataArr[i].nCol += nFieldStart;

        //  Daten-Eintrag hinzufuegen, wenn noch nicht vorhanden

        BOOL bDataFound = FALSE;
        for ( i=0; i<aParam.nColCount; i++ )
            if ( aParam.aColArr[i].nCol == PIVOT_DATA_FIELD )
                bDataFound = TRUE;
        for ( i=0; i<aParam.nRowCount; i++ )
            if ( aParam.aRowArr[i].nCol == PIVOT_DATA_FIELD )
                bDataFound = TRUE;
        if (!bDataFound)
        {
            if ( aParam.nRowCount < PIVOT_MAXFIELD )
                aParam.aRowArr[aParam.nRowCount++].nCol = PIVOT_DATA_FIELD;
            else if ( aParam.nColCount < PIVOT_MAXFIELD )
                aParam.aColArr[aParam.nColCount++].nCol = PIVOT_DATA_FIELD;
            else
                throw uno::RuntimeException();      // no space for data field
        }

        String aName = aNewName;
        if (!aName.Len())
            aName = lcl_CreatePivotName( pDocShell );
        String aTag = xDescriptor->getTag();

        ScDocument* pDoc = pDocShell->GetDocument();
        ScPivot* pNewPivot = new ScPivot( pDoc );
        pNewPivot->SetName( aName );
        pNewPivot->SetTag( aTag );
        pNewPivot->SetParam( aParam, aQuery, aSrcArea );

        ScDPObject* pNewObj = new ScDPObject( pDoc );
        pNewObj->InitFromOldPivot( *pNewPivot, pDoc, TRUE );

        ScDBDocFunc aFunc(*pDocShell);
        bDone = aFunc.DataPilotUpdate( NULL, pNewObj, TRUE, TRUE );

        delete pNewObj;     // DataPilotUpdate copies settings from "new" object
        delete pNewPivot;
    }

    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScDataPilotTablesObj::removeByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr = aName;
    ScDPObject* pDPObj = lcl_GetDPObject( pDocShell, nTab, aNameStr );
    if (pDPObj && pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.DataPilotUpdate( pDPObj, NULL, TRUE, TRUE );  // remove - incl. undo etc.
    }
    else
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotTablesObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotTablesObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!

            USHORT nFound = 0;
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }
            return nFound;
        }
    }

    return 0;
}

uno::Any SAL_CALL ScDataPilotTablesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDataPilotTable> xTable = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xTable.is())
        aAny <<= xTable;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScDataPilotTablesObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XDataPilotTable>*)0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotTablesObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDataPilotTable> xTable = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xTable.is())
        aAny <<= xTable;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotTablesObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            //  api only handles sheet data at this time
            //! allow all data sources!!!

            USHORT nFound = 0;
            USHORT nCount = pColl->GetCount();
            USHORT i;
            for (i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }

            USHORT nPos = 0;
            uno::Sequence<rtl::OUString> aSeq(nFound);
            rtl::OUString* pAry = aSeq.getArray();
            for (i=0; i<nCount; i++)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() && pDPObj->GetOutRange().aStart.Tab() == nTab )
                    pAry[nPos++] = pDPObj->GetName();
            }

            return aSeq;
        }
    }
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            String aNamStr = aName;
            USHORT nCount = pColl->GetCount();
            for (USHORT i=0; i<nCount; i++)
            {
                //  api only handles sheet data at this time
                //! allow all data sources!!!

                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->IsSheetData() &&
                     pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == aNamStr )
                    return TRUE;
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------

ScDataPilotDescriptorBase::ScDataPilotDescriptorBase(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDataPilotDescriptorBase::~ScDataPilotDescriptorBase()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

uno::Any SAL_CALL ScDataPilotDescriptorBase::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XDataPilotDescriptor )
    SC_QUERYINTERFACE( container::XNamed )                  // base of XDataPilotDescriptor
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XTypeProvider )
    SC_QUERYINTERFACE( lang::XServiceInfo )

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ScDataPilotDescriptorBase::acquire() throw(uno::RuntimeException)
{
    OWeakObject::acquire();
}

void SAL_CALL ScDataPilotDescriptorBase::release() throw(uno::RuntimeException)
{
    OWeakObject::release();
}

uno::Sequence<uno::Type> SAL_CALL ScDataPilotDescriptorBase::getTypes()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(4);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<sheet::XDataPilotDescriptor>*)0);
        pPtr[1] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[2] = getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
        pPtr[3] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScDataPilotDescriptorBase::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

void ScDataPilotDescriptorBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //! Referenz-Update?

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDataPilotDescriptor

table::CellRangeAddress SAL_CALL ScDataPilotDescriptorBase::getSourceRange()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    GetParam( aParam, aQuery, aSrcArea );

    table::CellRangeAddress aRet;
    aRet.Sheet       = aSrcArea.nTab;
    aRet.StartColumn = aSrcArea.nColStart;
    aRet.StartRow    = aSrcArea.nRowStart;
    aRet.EndColumn   = aSrcArea.nColEnd;
    aRet.EndRow      = aSrcArea.nRowEnd;
    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::setSourceRange(
                                const table::CellRangeAddress& aSourceRange )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    GetParam( aParam, aQuery, aSrcArea );

    aSrcArea.nTab       = aSourceRange.Sheet;
    aSrcArea.nColStart  = aSourceRange.StartColumn;
    aSrcArea.nRowStart  = aSourceRange.StartRow;
    aSrcArea.nColEnd    = aSourceRange.EndColumn;
    aSrcArea.nRowEnd    = aSourceRange.EndRow;

    SetParam( aParam, aQuery, aSrcArea );
}

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScDataPilotDescriptorBase::getFilterDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFilterDescriptor( this );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataPilotFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, SC_FIELDORIENT_ALL );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getColumnFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_COLUMN );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getRowFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_ROW );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getPageFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_PAGE );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_DATA );
}

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getHiddenFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFieldsObj( this, DATA_PILOT_HIDDEN );
}

// XUnoTunnel

sal_Int64 SAL_CALL ScDataPilotDescriptorBase::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScDataPilotDescriptorBase::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScDataPilotDescriptorBase* ScDataPilotDescriptorBase::getImplementation(
                                const uno::Reference<sheet::XDataPilotDescriptor> xObj )
{
    ScDataPilotDescriptorBase* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScDataPilotDescriptorBase*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScDataPilotTableObj::ScDataPilotTableObj(ScDocShell* pDocSh, USHORT nT, const String& rN) :
    ScDataPilotDescriptorBase( pDocSh ),
    nTab( nT ),
    aName( rN )
{
}

ScDataPilotTableObj::~ScDataPilotTableObj()
{
}

uno::Any SAL_CALL ScDataPilotTableObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XDataPilotTable )

    return ScDataPilotDescriptorBase::queryInterface( rType );
}

void SAL_CALL ScDataPilotTableObj::acquire() throw(uno::RuntimeException)
{
    ScDataPilotDescriptorBase::acquire();
}

void SAL_CALL ScDataPilotTableObj::release() throw(uno::RuntimeException)
{
    ScDataPilotDescriptorBase::release();
}

uno::Sequence<uno::Type> SAL_CALL ScDataPilotTableObj::getTypes()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes = ScDataPilotDescriptorBase::getTypes();
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 1 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XDataPilotTable>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScDataPilotTableObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// ---

void ScDataPilotTableObj::GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const
{
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        const ScSheetSourceDesc* pSheetDesc = pDPObj->GetSheetDesc();
        if ( pSheetDesc )
        {
            // FALSE -> fields are counted within the source range
            pDPObj->FillOldParam( rParam, FALSE );

            rSrcArea = ScArea( pSheetDesc->aSourceRange.aStart.Tab(),
                                pSheetDesc->aSourceRange.aStart.Col(),
                                pSheetDesc->aSourceRange.aStart.Row(),
                                pSheetDesc->aSourceRange.aEnd.Col(),
                                pSheetDesc->aSourceRange.aEnd.Row() );
            rQuery = pSheetDesc->aQueryParam;

            //  in the Uno objects all fields in descriptors are counted within the source range
            //  fields in ScPivotParam are correct (bForFile=FALSE in FillOldParam),
            //  ScQueryParam still has to be adjusted:

            short nFieldStart = rSrcArea.nColStart;
            USHORT nQueryCount = rQuery.GetEntryCount();
            for ( USHORT i=0; i<nQueryCount; i++ )
            {
                ScQueryEntry& rEntry = rQuery.GetEntry(i);
                if (rEntry.bDoQuery && rEntry.nField >= nFieldStart)
                    rEntry.nField -= nFieldStart;
            }
        }
    }
}

void ScDataPilotTableObj::SetParam( const ScPivotParam& rParam,
                                const ScQueryParam& rQuery, const ScArea& rSrcArea )
{
    ScDocShell* pDocShell = GetDocShell();
    ScDPObject* pDPObj = lcl_GetDPObject(pDocShell, nTab, aName);
    if ( pDPObj && pDocShell )
    {
        //  in den Uno-Objekten sind alle Fields in den Descriptoren innerhalb des Bereichs gezaehlt

        ScPivotParam aNewParam( rParam );
        short nFieldStart = rSrcArea.nColStart;
        USHORT i;
        for ( i=0; i<aNewParam.nColCount; i++ )
            if ( aNewParam.aColArr[i].nCol != PIVOT_DATA_FIELD )
                aNewParam.aColArr[i].nCol += nFieldStart;
        for ( i=0; i<aNewParam.nRowCount; i++ )
            if ( aNewParam.aRowArr[i].nCol != PIVOT_DATA_FIELD )
                aNewParam.aRowArr[i].nCol += nFieldStart;
        for ( i=0; i<aNewParam.nDataCount; i++ )
            if ( aNewParam.aDataArr[i].nCol != PIVOT_DATA_FIELD )
                aNewParam.aDataArr[i].nCol += nFieldStart;

        ScQueryParam aNewQuery( rQuery );
        USHORT nQueryCount = aNewQuery.GetEntryCount();
        for ( i=0; i<nQueryCount; i++ )
        {
            ScQueryEntry& rEntry = aNewQuery.GetEntry(i);
            if (rEntry.bDoQuery)
                rEntry.nField += nFieldStart;
        }

//      ScPivot* pNew = new ScPivot(*pPivot);   //? behaelt falsche Groessenangaben bei...

        ScDocument* pDoc = pDocShell->GetDocument();
        ScPivot* pNew = new ScPivot( pDoc );
        pNew->SetName( pDPObj->GetName() );
        pNew->SetTag( pDPObj->GetTag() );
        pNew->SetParam( aNewParam, aNewQuery, rSrcArea );

        ScDPObject* pNewObj = new ScDPObject( pDoc );
        pNewObj->InitFromOldPivot( *pNew, pDoc, TRUE );

        ScDBDocFunc aFunc(*pDocShell);
        aFunc.DataPilotUpdate( pDPObj, pNewObj, TRUE, TRUE );

        delete pNewObj;     // DataPilotUpdate copies settings from "new" object
        delete pNew;
    }
}

// "rest of XDataPilotDescriptor"

rtl::OUString SAL_CALL ScDataPilotTableObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetName();
    return rtl::OUString();
}

void SAL_CALL ScDataPilotTableObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        //! test for existing names !!!

        String aString = aNewName;
        pDPObj->SetName( aString );     //! Undo - DBDocFunc ???
        aName = aString;

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

rtl::OUString SAL_CALL ScDataPilotTableObj::getTag() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetTag();
    return rtl::OUString();
}

void SAL_CALL ScDataPilotTableObj::setTag( const ::rtl::OUString& aNewTag )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        String aString = aNewTag;
        pDPObj->SetTag( aString );      //! Undo - DBDocFunc ???

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

// XDataPilotTable

table::CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRange()
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScRange aRange = pDPObj->GetOutRange();
        aRet.Sheet       = aRange.aStart.Tab();
        aRet.StartColumn = aRange.aStart.Col();
        aRet.StartRow    = aRange.aStart.Row();
        aRet.EndColumn   = aRange.aEnd.Col();
        aRet.EndRow      = aRange.aEnd.Row();
    }
    return aRet;
}

void SAL_CALL ScDataPilotTableObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScDPObject* pNew = new ScDPObject(*pDPObj);
        ScDBDocFunc aFunc(*GetDocShell());
        aFunc.DataPilotUpdate( pDPObj, pNew, TRUE, TRUE );
        delete pNew;        // DataPilotUpdate copies settings from "new" object
    }
}

//------------------------------------------------------------------------

ScDataPilotDescriptor::ScDataPilotDescriptor(ScDocShell* pDocSh) :
    ScDataPilotDescriptorBase( pDocSh )
{
}

ScDataPilotDescriptor::~ScDataPilotDescriptor()
{
}

void ScDataPilotDescriptor::GetParam( ScPivotParam& rParam, ScQueryParam& rQuery,
                                        ScArea& rSrcArea ) const
{
    //  Fields sind und bleiben innerhalb des Bereichs

    rParam = aParam;
    rQuery = aQuery;
    rSrcArea = aSrcArea;
}

void ScDataPilotDescriptor::SetParam( const ScPivotParam& rParam,
                                const ScQueryParam& rQuery, const ScArea& rSrcArea )
{
    //  Fields sind und bleiben innerhalb des Bereichs

    aParam = rParam;
    aQuery = rQuery;
    aSrcArea = rSrcArea;
}

// "rest of XDataPilotDescriptor"

rtl::OUString SAL_CALL ScDataPilotDescriptor::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aNameStr;
}

void SAL_CALL ScDataPilotDescriptor::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aNameStr = String( aNewName );
}

rtl::OUString SAL_CALL ScDataPilotDescriptor::getTag() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aTagStr;
}

void SAL_CALL ScDataPilotDescriptor::setTag( const ::rtl::OUString& aNewTag )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aTagStr = String( aNewTag );
}

//------------------------------------------------------------------------

ScDataPilotFieldsObj::ScDataPilotFieldsObj(ScDataPilotDescriptorBase* pPar, USHORT nTy) :
    pParent( pPar ),
    nType( nTy )
{
    pParent->acquire();
}

ScDataPilotFieldsObj::~ScDataPilotFieldsObj()
{
    pParent->release();
}

USHORT lcl_GetFieldCount( const ScPivotParam& rParam, const ScArea& rSrcArea, USHORT nType )
{
    USHORT nRet = 0;

    USHORT nDataCount = lcl_DataCount(rParam);
    //  Daten-Feld bei Spalten oder Zeile, wenn mehr als 1 Eintrag

    USHORT i;
    switch (nType)
    {
        case SC_FIELDORIENT_ALL:
            //  Anzahl Spalten im Datenbereich
            //  plus eins fuer das Datenfeld (immer)
            nRet = rSrcArea.nColEnd - rSrcArea.nColStart + 2;
            break;
        case DATA_PILOT_HIDDEN:
            {
                //! Datenfeld auch hier?
                USHORT nColCount = rSrcArea.nColEnd - rSrcArea.nColStart + 1;
                for (USHORT nSrcField=0; nSrcField<nColCount; nSrcField++)
                {
                    BOOL bUsed = FALSE;
                    for (i=0; i<rParam.nColCount; i++)
                        if ( rParam.aColArr[i].nCol == nSrcField ) bUsed = TRUE;
                    for (i=0; i<rParam.nRowCount; i++)
                        if ( rParam.aRowArr[i].nCol == nSrcField ) bUsed = TRUE;
                    for (i=0; i<rParam.nDataCount; i++)
                        if ( rParam.aDataArr[i].nCol == nSrcField ) bUsed = TRUE;
                    if (!bUsed)
                        ++nRet;
                }
            }
            break;
        case DATA_PILOT_COLUMN:
            for (i=0; i<rParam.nColCount; i++)
                if ( rParam.aColArr[i].nCol != PIVOT_DATA_FIELD || nDataCount > 1 )
                    ++nRet;
            break;
        case DATA_PILOT_ROW:
            for (i=0; i<rParam.nRowCount; i++)
                if ( rParam.aRowArr[i].nCol != PIVOT_DATA_FIELD || nDataCount > 1 )
                    ++nRet;
            break;
        case DATA_PILOT_PAGE:
            nRet = 0;               // Page-Fields sind nicht implementiert
            break;
        case DATA_PILOT_DATA:
            nRet = nDataCount;
            break;
    }

    return nRet;
}

BOOL lcl_GetFieldDataByIndex( const ScPivotParam& rParam, const ScArea& rSrcArea,
                                USHORT nType, USHORT nIndex, USHORT& rField )
{
    BOOL bOk = FALSE;
    USHORT nPos = 0;

    USHORT nDataCount = lcl_DataCount(rParam);
    //  Daten-Feld bei Spalten oder Zeile, wenn mehr als 1 Eintrag

    USHORT i;
    switch (nType)
    {
        case SC_FIELDORIENT_ALL:
            {
                USHORT nSourceCount = rSrcArea.nColEnd - rSrcArea.nColStart + 1;
                if ( nIndex < nSourceCount )
                {
                    rField = nIndex;
                    bOk = TRUE;
                }
                else if ( nIndex == nSourceCount )
                {
                    rField = PIVOT_DATA_FIELD;
                    bOk = TRUE;
                }
            }
            break;
        case DATA_PILOT_HIDDEN:
            {
                //! Datenfeld auch hier?
                USHORT nColCount = rSrcArea.nColEnd - rSrcArea.nColStart + 1;
                for (USHORT nSrcField=0; nSrcField<nColCount; nSrcField++)
                {
                    BOOL bUsed = FALSE;
                    for (i=0; i<rParam.nColCount; i++)
                        if ( rParam.aColArr[i].nCol == nSrcField ) bUsed = TRUE;
                    for (i=0; i<rParam.nRowCount; i++)
                        if ( rParam.aRowArr[i].nCol == nSrcField ) bUsed = TRUE;
                    for (i=0; i<rParam.nDataCount; i++)
                        if ( rParam.aDataArr[i].nCol == nSrcField ) bUsed = TRUE;
                    if (!bUsed)
                    {
                        if ( nPos == nIndex )
                        {
                            rField = nSrcField;
                            bOk = TRUE;
                        }
                        ++nPos;
                    }
                }
            }
            break;
        case DATA_PILOT_COLUMN:
            for (i=0; i<rParam.nColCount; i++)
                if ( rParam.aColArr[i].nCol != PIVOT_DATA_FIELD || nDataCount > 1 )
                {
                    if ( nPos == nIndex )
                    {
                        rField = rParam.aColArr[i].nCol;
                        bOk = TRUE;
                    }
                    ++nPos;
                }
            break;
        case DATA_PILOT_ROW:
            for (i=0; i<rParam.nRowCount; i++)
                if ( rParam.aRowArr[i].nCol != PIVOT_DATA_FIELD || nDataCount > 1 )
                {
                    if ( nPos == nIndex )
                    {
                        rField = rParam.aRowArr[i].nCol;
                        bOk = TRUE;
                    }
                    ++nPos;
                }
            break;
        case DATA_PILOT_PAGE:
            break;                  // Page-Fields sind nicht implementiert
        case DATA_PILOT_DATA:
            {
                USHORT nArrayPos, nFuncBit;
                if ( lcl_GetDataArrayPos( rParam, nIndex, nArrayPos, nFuncBit ) )
                {
                    rField = rParam.aDataArr[nArrayPos].nCol;
                    bOk = TRUE;
                }
            }
            break;
    }
    return bOk;
}

String lcl_FieldName( ScDocShell* pDocSh, const ScPivotParam& rParam,
                        const ScArea& rSrcArea, USHORT nField )
{
    //! Gespeicherte Namen im Pivot beruecksichtigen !!!!

    String aRet;
    if ( nField == PIVOT_DATA_FIELD )
        aRet = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Data"));     //! ???
    else
    {
        USHORT nCol = rSrcArea.nColStart + nField;
        if ( pDocSh && nCol <= rSrcArea.nColEnd )
        {
            USHORT nRow = rSrcArea.nRowStart;
            aRet = lcl_ColumnTitle( pDocSh->GetDocument(), nCol, nRow, rSrcArea.nTab );
        }
    }
    return aRet;
}

// XDataPilotFields

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByIndex_Impl(USHORT nIndex) const
{
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    USHORT nField = 0;
    BOOL bOk = lcl_GetFieldDataByIndex( aParam, aSrcArea, nType, nIndex, nField );

    if (bOk)
        return new ScDataPilotFieldObj( pParent, nField, nType, nIndex );

    return NULL;
}

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
    String aNameStr = aName;

    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    ScDocShell* pDocSh = pParent->GetDocShell();
    USHORT nCount = lcl_GetFieldCount( aParam, aSrcArea, nType );
    USHORT nField = 0;
    for (USHORT i=0; i<nCount; i++)
    {
        if (lcl_GetFieldDataByIndex( aParam, aSrcArea, nType, i, nField ))
        {
            if ( aNameStr == lcl_FieldName( pDocSh, aParam, aSrcArea, nField ) )
                return new ScDataPilotFieldObj( pParent, nField, nType, i );
        }
    }
    return NULL;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    return lcl_GetFieldCount( aParam, aSrcArea, nType );
}

uno::Any SAL_CALL ScDataPilotFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xField = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xField.is())
        aAny <<= xField;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScDataPilotFieldsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScDataPilotFieldsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xField = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xField.is())
        aAny <<= xField;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotFieldsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    ScDocShell* pDocSh = pParent->GetDocShell();
    USHORT nCount = lcl_GetFieldCount( aParam, aSrcArea, nType );
    USHORT nField = 0;

    uno::Sequence<rtl::OUString> aSeq(nCount);
    rtl::OUString* pAry = aSeq.getArray();
    for (USHORT i=0; i<nCount; i++)
    {
        lcl_GetFieldDataByIndex( aParam, aSrcArea, nType, i, nField );
        String aName = lcl_FieldName( pDocSh, aParam, aSrcArea, nField );
        pAry[i] = aName;
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameStr = aName;

    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    ScDocShell* pDocSh = pParent->GetDocShell();
    USHORT nCount = lcl_GetFieldCount( aParam, aSrcArea, nType );
    USHORT nField = 0;
    for (USHORT i=0; i<nCount; i++)
    {
        if (lcl_GetFieldDataByIndex( aParam, aSrcArea, nType, i, nField ))
        {
            if ( aNameStr == lcl_FieldName( pDocSh, aParam, aSrcArea, nField ) )
                return TRUE;
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------

ScDataPilotFieldObj::ScDataPilotFieldObj( ScDataPilotDescriptorBase* pPar,
                                            USHORT nF, USHORT nST, USHORT nSP ) :
    aPropSet( lcl_GetDataPilotFieldMap() ),
    pParent( pPar ),
    nField( nF ),
    nSourceType( nST ),
    nSourcePos( nSP )
{
    pParent->acquire();
}

ScDataPilotFieldObj::~ScDataPilotFieldObj()
{
    pParent->release();
}

// XNamed

rtl::OUString SAL_CALL ScDataPilotFieldObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    String aRet = lcl_FieldName( pParent->GetDocShell(), aParam, aSrcArea, nField );

    return aRet;
}

void SAL_CALL ScDataPilotFieldObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    //! muss noch
    //! (Namen fuer Spalten vergeben, setzen und ueber den Namen wiederfinden)
    //! -> spaeter...
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDataPilotFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    if ( aNameString.EqualsAscii( SC_UNONAME_FUNCTION ) )
    {
        //! test for correct enum type?
        sheet::GeneralFunction eFunction = (sheet::GeneralFunction)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setFunction( eFunction );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ORIENT ) )
    {
        //! test for correct enum type?
        sheet::DataPilotFieldOrientation eOrient = (sheet::DataPilotFieldOrientation)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setOrientation( eOrient );
    }
}

uno::Any SAL_CALL ScDataPilotFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    uno::Any aRet;

    if ( aNameString.EqualsAscii( SC_UNONAME_FUNCTION ) )
    {
        sheet::GeneralFunction eFunction = getFunction();
        aRet <<= eFunction;
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ORIENT ) )
    {
        sheet::DataPilotFieldOrientation eOrient = getOrientation();
        aRet <<= eOrient;
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDataPilotFieldObj )

void lcl_FindUsage( const ScPivotParam& rParam, USHORT nField, USHORT& rType, USHORT& rPos )
{
    USHORT i;
    for (i=0; i<rParam.nColCount; i++)
        if ( rParam.aColArr[i].nCol == nField )
        {
            rType = DATA_PILOT_COLUMN;
            rPos = i;
            return;
        }
    for (i=0; i<rParam.nRowCount; i++)
        if ( rParam.aRowArr[i].nCol == nField )
        {
            rType = DATA_PILOT_ROW;
            rPos = i;
            return;
        }
    USHORT nBitCount = 0;
    for (i=0; i<rParam.nDataCount; i++)
    {
        if ( rParam.aDataArr[i].nCol == nField )
        {
            rType = DATA_PILOT_DATA;
            rPos = nBitCount;               // Uno-Index der ersten Funktion
            return;
        }
        nBitCount += lcl_BitCount( rParam.aDataArr[i].nFuncMask );
    }
    //! DATA_PILOT_HIDDEN zurueck - Position suchen
}

sheet::DataPilotFieldOrientation ScDataPilotFieldObj::getOrientation(void) const
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    USHORT nType = nSourceType;
    USHORT nPos  = nSourcePos;
    if ( nType == SC_FIELDORIENT_ALL )
        lcl_FindUsage( aParam, nField, nType, nPos );

    if ( nType == SC_FIELDORIENT_ALL )      // bei FindUsage nicht gefunden
        nType = DATA_PILOT_HIDDEN;          // nicht verwendet

    return (sheet::DataPilotFieldOrientation)nType;
}

void lcl_RemoveField( PivotField* pFields, USHORT& rCount, USHORT nField )
{
    for (USHORT i=0; i<rCount; i++)
        if (pFields[i].nCol == nField)
        {
            for (USHORT j=i; j+1<rCount; j++)
                pFields[j] = pFields[j+1];
            --rCount;
            return;         // nicht weitersuchen
        }
}

void ScDataPilotFieldObj::setOrientation(sheet::DataPilotFieldOrientation eNew)
{
    ScUnoGuard aGuard;
    if ( eNew == nSourceType )
        return;                     // nix

    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    //  aus altem Array entfernen

    switch (nSourceType)
    {
        case DATA_PILOT_COLUMN:
            if ( nSourcePos < aParam.nColCount )
            {
                for (USHORT i=nSourcePos; i+1<aParam.nColCount; i++)
                    aParam.aColArr[i] = aParam.aColArr[i+1];
                --aParam.nColCount;
            }
            break;
        case DATA_PILOT_ROW:
            if ( nSourcePos < aParam.nRowCount )
            {
                for (USHORT i=nSourcePos; i+1<aParam.nRowCount; i++)
                    aParam.aRowArr[i] = aParam.aRowArr[i+1];
                --aParam.nRowCount;
            }
            break;
        case DATA_PILOT_DATA:
            {
                USHORT nArrayPos, nFuncBit;
                if ( lcl_GetDataArrayPos( aParam, nSourcePos, nArrayPos, nFuncBit ) )
                {
                    aParam.aDataArr[nArrayPos].nFuncMask &= ~nFuncBit;  // Funktion weglassen
                    if (!aParam.aDataArr[nArrayPos].nFuncMask)          // war's die letzte?
                    {
                        for (USHORT j=nArrayPos; j+1<aParam.nDataCount; j++)
                            aParam.aDataArr[j] = aParam.aDataArr[j+1];
                        --aParam.nDataCount;
                    }
                }
            }
            break;

        //  SC_FIELDORIENT_ALL, DATA_PILOT_HIDDEN: nichts
    }

    //  in neues Array eintragen

    switch (eNew)
    {
        case DATA_PILOT_COLUMN:
            if ( aParam.nColCount < PIVOT_MAXFIELD )
            {
                lcl_RemoveField( aParam.aColArr, aParam.nColCount, nField );    // nicht doppelt
                lcl_RemoveField( aParam.aRowArr, aParam.nRowCount, nField );
                aParam.aColArr[aParam.nColCount].nCol = nField;
                aParam.aColArr[aParam.nColCount].nFuncMask = 0;
                nSourceType = DATA_PILOT_COLUMN;
                nSourcePos  = aParam.nColCount;
                ++aParam.nColCount;
            }
            break;
        case DATA_PILOT_ROW:
            if ( aParam.nRowCount < PIVOT_MAXFIELD )
            {
                lcl_RemoveField( aParam.aColArr, aParam.nColCount, nField );    // nicht doppelt
                lcl_RemoveField( aParam.aRowArr, aParam.nRowCount, nField );
                aParam.aRowArr[aParam.nRowCount].nCol = nField;
                aParam.aRowArr[aParam.nRowCount].nFuncMask = 0;
                nSourceType = DATA_PILOT_ROW;
                nSourcePos  = aParam.nRowCount;
                ++aParam.nRowCount;
            }
            break;
        case DATA_PILOT_DATA:
            if ( aParam.nDataCount < PIVOT_MAXFIELD )
            {
                //! ggf. mit bestehendem Eintrag zusammenfassen (Funktionen verodern) ???
                aParam.aDataArr[aParam.nDataCount].nCol = nField;
                aParam.aDataArr[aParam.nDataCount].nFuncMask = PIVOT_FUNC_SUM;
                nSourceType = DATA_PILOT_DATA;
                nSourcePos  = aParam.nDataCount;
                ++aParam.nDataCount;
            }
            break;
    }

    pParent->SetParam( aParam, aQuery, aSrcArea );
}

sheet::GeneralFunction ScDataPilotFieldObj::getFunction(void) const
{
    ScUnoGuard aGuard;
    sheet::GeneralFunction eRet = sheet::GeneralFunction_NONE;

    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    USHORT nType = nSourceType;
    USHORT nPos  = nSourcePos;
    if ( nType == SC_FIELDORIENT_ALL )
        lcl_FindUsage( aParam, nField, nType, nPos );

    switch ( nType )
    {
        case DATA_PILOT_COLUMN:
            if ( nPos < aParam.nColCount )
                eRet = ScDataPilotConversion::FirstFunc( aParam.aColArr[nPos].nFuncMask );
            break;
        case DATA_PILOT_ROW:
            if ( nPos < aParam.nRowCount )
                eRet = ScDataPilotConversion::FirstFunc( aParam.aRowArr[nPos].nFuncMask );
            break;
        case DATA_PILOT_DATA:       // Bits zaehlen...
            {
                USHORT nArrayPos, nFuncBit;
                if ( lcl_GetDataArrayPos( aParam, nPos, nArrayPos, nFuncBit ) )
                    eRet = ScDataPilotConversion::FirstFunc( nFuncBit );
            }
            break;
    }
    return eRet;
}

void ScDataPilotFieldObj::setFunction(sheet::GeneralFunction eNewFunc)
{
    ScUnoGuard aGuard;
    ScPivotParam aParam;
    ScQueryParam aQuery;
    ScArea aSrcArea;
    pParent->GetParam( aParam, aQuery, aSrcArea );

    USHORT nType = nSourceType;
    USHORT nPos  = nSourcePos;
    if ( nType == SC_FIELDORIENT_ALL )
        lcl_FindUsage( aParam, nField, nType, nPos );

    switch ( nType )
    {
        case DATA_PILOT_COLUMN:
            if ( nPos < aParam.nColCount )
                aParam.aColArr[nPos].nFuncMask = ScDataPilotConversion::FunctionBit(eNewFunc);
            break;
        case DATA_PILOT_ROW:
            if ( nPos < aParam.nRowCount )
                aParam.aRowArr[nPos].nFuncMask = ScDataPilotConversion::FunctionBit(eNewFunc);
            break;
        case DATA_PILOT_DATA:       // Bits zaehlen...
            {
                USHORT nArrayPos, nFuncBit;
                if ( lcl_GetDataArrayPos( aParam, nPos, nArrayPos, nFuncBit ) )
                {
                    //  alte Funktion weglassen, neue eintragen
                    aParam.aDataArr[nArrayPos].nFuncMask &= ~nFuncBit;
                    aParam.aDataArr[nArrayPos].nFuncMask |= ScDataPilotConversion::FunctionBit(eNewFunc);
                }
            }
            break;
    }

    pParent->SetParam( aParam, aQuery, aSrcArea );
}


//------------------------------------------------------------------------




