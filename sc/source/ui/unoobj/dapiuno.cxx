/*************************************************************************
 *
 *  $RCSfile: dapiuno.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:00:38 $
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
#include "dpsave.hxx"
#include "dbdocfun.hxx"
#include "unonames.hxx"

#ifndef _COM_SUN_STAR_SHEET_XHIERARCHIESSUPPLIER_HPP_
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLEVELSSUPPLIER_HPP_
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XMEMBERSSUPPLIER_HPP_
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

using namespace com::sun::star;


//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDataPilotDescriptorBaseMap()
{
    static SfxItemPropertyMap aDataPilotDescriptorBaseMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_COLGRAND), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_DRILLDOWN),0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_IGNEMPROWS),0, &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_RPTEMPTY), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_ROWGRAND), 0,  &getBooleanCppuType(),  0, 0 },
        {MAP_CHAR_LEN(SC_UNO_SHOWFILT), 0,  &getBooleanCppuType(),  0, 0 },
        {0,0,0,0}
    };
    return aDataPilotDescriptorBaseMap_Impl;
}

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetDataPilotFieldMap()
{
    static SfxItemPropertyMap aDataPilotFieldMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_AUTOSHOW),     0,  &getCppuType((sheet::DataPilotFieldAutoShowInfo*)0),0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SELPAGE),      0,  &getCppuType((rtl::OUString*)0),                    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FUNCTION),     0,  &getCppuType((sheet::GeneralFunction*)0),           0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LAYOUTINFO),   0,  &getCppuType((sheet::DataPilotFieldLayoutInfo*)0),  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ORIENT),       0,  &getCppuType((sheet::DataPilotFieldOrientation*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_REFERENCE),    0,  &getCppuType((sheet::DataPilotFieldReference*)0),   0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_SORTINFO),     0,  &getCppuType((sheet::DataPilotFieldSortInfo*)0),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_USESELPAGE),   0,  &getBooleanCppuType(),                              0, 0 },
        {0,0,0,0}
    };
    return aDataPilotFieldMap_Impl;
}

const SfxItemPropertyMap* lcl_GetDataPilotItemMap()
{
    static SfxItemPropertyMap aDataPilotItemMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_SHOWDETAIL),   0,  &getBooleanCppuType(),                              0, 0 },
        {0,0,0,0}
    };
    return aDataPilotItemMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScDataPilotDescriptor, "ScDataPilotDescriptor", "stardiv::one::sheet::DataPilotDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldObj, "ScDataPilotFieldObj", "com.sun.star.sheet.DataPilotField" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldsObj, "ScDataPilotFieldsObj", "com.sun.star.sheet.DataPilotFields" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTableObj, "ScDataPilotTableObj", "com.sun.star.sheet.DataPilotTable" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTablesObj, "ScDataPilotTablesObj", "com.sun.star.sheet.DataPilotTables" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemsObj, "ScDataPilotItemsObj", "com.sun.star.sheet.DataPilotItems" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemObj, "ScDataPilotItemObj", "com.sun.star.sheet.DataPilotItem" )

//------------------------------------------------------------------------

//! irgendwann ueberall die neuen enum-Werte benutzen
#define DATA_PILOT_HIDDEN   sheet::DataPilotFieldOrientation_HIDDEN
#define DATA_PILOT_COLUMN   sheet::DataPilotFieldOrientation_COLUMN
#define DATA_PILOT_ROW      sheet::DataPilotFieldOrientation_ROW
#define DATA_PILOT_PAGE     sheet::DataPilotFieldOrientation_PAGE
#define DATA_PILOT_DATA     sheet::DataPilotFieldOrientation_DATA

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

ScDPObject* lcl_GetDPObject( ScDocShell* pDocShell, SCTAB nTab, const String& rName )
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

ScDataPilotTablesObj::ScDataPilotTablesObj(ScDocShell* pDocSh, SCTAB nT) :
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

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByIndex_Impl(SCSIZE nIndex)
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
            SCSIZE nFound = 0;
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

void lcl_SetSaveData(const uno::Reference<container::XIndexAccess>& xFields, ScDPSaveData* pSaveData)
{
    if (xFields.is() && pSaveData)
    {
        sal_Int32 nFieldsCount(xFields->getCount());
        for (sal_Int32 i = 0; i < nFieldsCount; ++i)
        {
            uno::Any aDim = xFields->getByIndex(i);
            uno::Reference<container::XNamed> xDim;
            uno::Reference<beans::XPropertySet> xDimProps;
            if ((aDim >>= xDim) && (aDim >>= xDimProps))
            {
                rtl::OUString sName(xDim->getName());
                ScDPSaveDimension* pDim = sName.getLength() ? pSaveData->GetDimensionByName(sName) : 0;
                if (pDim)
                {
                    uno::Any aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT)));
                    sheet::DataPilotFieldOrientation eOrient;
                    if (aAny >>= eOrient)
                        pDim->SetOrientation( eOrient );

                    aAny = xDimProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FUNCTION)));
                    sheet::GeneralFunction eFunc;
                    if (aAny >>= eFunc)
                        pDim->SetFunction( eFunc );
                }
            }
        }
    }
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
        ScDPObject* pNewObj = pImp->GetDPObject();

        if (pNewObj)
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            ScRange aOutputRange((SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet,
                                (SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet);
            pNewObj->SetOutRange(aOutputRange);
            String aName = aNewName;
            if (!aName.Len())
                aName = lcl_CreatePivotName( pDocShell );
            pNewObj->SetName(aName);
            String aTag = xDescriptor->getTag();
            pNewObj->SetTag(aTag);

    // todo: handle double fields (for more information see ScDPObject

            ScDBDocFunc aFunc(*pDocShell);
            bDone = aFunc.DataPilotUpdate( NULL, pNewObj, TRUE, TRUE );
        }
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
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotTablesEnumeration")));
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
    uno::Reference<sheet::XDataPilotTable> xTable = GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex));
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
    aPropSet( lcl_GetDataPilotDescriptorBaseMap() ),
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
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( container::XNamed )                  // base of XDataPilotDescriptor
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XTypeProvider )
    SC_QUERYINTERFACE( lang::XServiceInfo )

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ScDataPilotDescriptorBase::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ScDataPilotDescriptorBase::release() throw()
{
    OWeakObject::release();
}

uno::Sequence<uno::Type> SAL_CALL ScDataPilotDescriptorBase::getTypes()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(5);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<sheet::XDataPilotDescriptor>*)0);
        pPtr[1] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[2] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[3] = getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
        pPtr[4] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
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

    table::CellRangeAddress aRet;

    ScDPObject* pDPObject(GetDPObject());
    if (pDPObject && pDPObject->IsSheetData())
    {
        const ScSheetSourceDesc* pSheetDesc = pDPObject->GetSheetDesc();
        aRet.StartColumn = pSheetDesc->aSourceRange.aStart.Col();
        aRet.EndColumn = pSheetDesc->aSourceRange.aEnd.Col();
        aRet.StartRow = pSheetDesc->aSourceRange.aStart.Row();
        aRet.EndRow = pSheetDesc->aSourceRange.aEnd.Row();
        aRet.Sheet = pSheetDesc->aSourceRange.aStart.Tab();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::setSourceRange(
                                const table::CellRangeAddress& aSourceRange )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    ScDPObject* pDPObject = GetDPObject();
    if (pDPObject)
    {
        ScSheetSourceDesc aSheetDesc;
        if (pDPObject->IsSheetData())
            aSheetDesc = *(pDPObject->GetSheetDesc());
        aSheetDesc.aSourceRange.aStart.SetCol((SCCOL)aSourceRange.StartColumn);
        aSheetDesc.aSourceRange.aEnd.SetCol((SCCOL)aSourceRange.EndColumn);
        aSheetDesc.aSourceRange.aStart.SetRow((SCROW)aSourceRange.StartRow);
        aSheetDesc.aSourceRange.aEnd.SetRow((SCROW)aSourceRange.EndRow);
        aSheetDesc.aSourceRange.aStart.SetTab((SCTAB)aSourceRange.Sheet);
        pDPObject->SetSheetDesc(aSheetDesc);
        SetDPObject(pDPObject);
    }
    else
        throw uno::RuntimeException();
}

uno::Reference<sheet::XSheetFilterDescriptor> SAL_CALL ScDataPilotDescriptorBase::getFilterDescriptor()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScDataPilotFilterDescriptor( pDocShell, this );
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

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScDataPilotDescriptorBase::getPropertySetInfo(  )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotDescriptorBase::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                const ::com::sun::star::uno::Any& aValue )
                                                throw(beans::UnknownPropertyException,
                                                    beans::PropertyVetoException,
                                                    lang::IllegalArgumentException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDPObject* pDPObject = GetDPObject();
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        DBG_ASSERT(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            String aNameString = aPropertyName;
            if ( aNameString.EqualsAscii( SC_UNO_COLGRAND ) )
            {
                aNewData.SetColumnGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_IGNEMPROWS ) )
            {
                aNewData.SetIgnoreEmptyRows(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_RPTEMPTY ) )
            {
                aNewData.SetRepeatIfEmpty(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_ROWGRAND ) )
            {
                aNewData.SetRowGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_SHOWFILT ) )
            {
                aNewData.SetFilterButton(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.EqualsAscii( SC_UNO_DRILLDOWN ) )
            {
                aNewData.SetDrillDown(::cppu::any2bool( aValue ));
            }
            else
                throw beans::UnknownPropertyException();

            pDPObject->SetSaveData( aNewData );
        }

        SetDPObject(pDPObject);
    }
}

uno::Any SAL_CALL ScDataPilotDescriptorBase::getPropertyValue(const ::rtl::OUString& aPropertyName )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;

    ScDPObject* pDPObject(GetDPObject());
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        DBG_ASSERT(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            String aNameString = aPropertyName;
            if ( aNameString.EqualsAscii( SC_UNO_COLGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetColumnGrand() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_IGNEMPROWS ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetIgnoreEmptyRows() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_RPTEMPTY ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRepeatIfEmpty() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_ROWGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRowGrand() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_SHOWFILT ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetFilterButton() );
            }
            else if ( aNameString.EqualsAscii( SC_UNO_DRILLDOWN ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetDrillDown() );
            }
            else
                throw beans::UnknownPropertyException();
        }
    }

    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                const uno::Reference<beans::XPropertyChangeListener >& xListener )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                const uno::Reference<beans::XPropertyChangeListener >& aListener )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                const uno::Reference<beans::XVetoableChangeListener >& aListener )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                const uno::Reference<beans::XVetoableChangeListener >& aListener )
                                                throw(beans::UnknownPropertyException,
                                                    lang::WrappedTargetException,
                                                    uno::RuntimeException)
{
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

ScDataPilotTableObj::ScDataPilotTableObj(ScDocShell* pDocSh, SCTAB nT, const String& rN) :
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

void SAL_CALL ScDataPilotTableObj::acquire() throw()
{
    ScDataPilotDescriptorBase::acquire();
}

void SAL_CALL ScDataPilotTableObj::release() throw()
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
ScDPObject* ScDataPilotTableObj::GetDPObject() const
{
    return lcl_GetDPObject(GetDocShell(), nTab, aName);
}

void ScDataPilotTableObj::SetDPObject( ScDPObject* pDPObject )
{
    ScDocShell* pDocShell = GetDocShell();
    ScDPObject* pDPObj = lcl_GetDPObject(pDocShell, nTab, aName);
    if ( pDPObj && pDocShell )
    {
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.DataPilotUpdate( pDPObj, pDPObject, TRUE, TRUE );
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
    ScDataPilotDescriptorBase( pDocSh ),
    mpDPObject(new ScDPObject(pDocSh ? pDocSh->GetDocument() : NULL) )
{
    mpDPObject->SetAlive(sal_True);
    ScDPSaveData aSaveData;
    // set defaults like in ScPivotParam constructor
     aSaveData.SetColumnGrand( sal_True );
    aSaveData.SetRowGrand( sal_True );
    aSaveData.SetIgnoreEmptyRows( sal_False );
    aSaveData.SetRepeatIfEmpty( sal_False );
    mpDPObject->SetSaveData(aSaveData);
    ScSheetSourceDesc aSheetDesc;
    mpDPObject->SetSheetDesc(aSheetDesc);
    mpDPObject->GetSource();
}

ScDataPilotDescriptor::~ScDataPilotDescriptor()
{
    delete mpDPObject;
}

ScDPObject* ScDataPilotDescriptor::GetDPObject() const
{
    return mpDPObject;
}

void ScDataPilotDescriptor::SetDPObject( ScDPObject* pDPObject )
{
    if (mpDPObject != pDPObject)
    {
        delete mpDPObject;
        mpDPObject = pDPObject;
        DBG_ERROR("replace DPObject should not happen");
    }
}

// "rest of XDataPilotDescriptor"

rtl::OUString SAL_CALL ScDataPilotDescriptor::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return mpDPObject->GetName();
}

void SAL_CALL ScDataPilotDescriptor::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    mpDPObject->SetName( aNewName );
}

rtl::OUString SAL_CALL ScDataPilotDescriptor::getTag() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return mpDPObject->GetTag();
}

void SAL_CALL ScDataPilotDescriptor::setTag( const ::rtl::OUString& aNewTag )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    mpDPObject->SetTag( aNewTag );
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

SCSIZE lcl_GetFieldCount( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& rSource, USHORT nType )
{
    SCSIZE nRet = 0;

    uno::Reference<container::XNameAccess> xDimsName(rSource->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (nType != SC_FIELDORIENT_ALL)
    {
        uno::Reference<beans::XPropertySet> xDim;
        sheet::DataPilotFieldOrientation aOrient;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if (xDim.is())
            {
                xDim->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIENTAT))) >>= aOrient;
                if (aOrient == nType)
                    ++nRet;
            }
        }
    }
    else
        nRet = static_cast<SCSIZE>(nIntCount);

    return nRet;
}

BOOL lcl_GetFieldDataByIndex( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& rSource,
                                USHORT nType, SCSIZE nIndex, SCSIZE& rPos )
{
    BOOL bOk = FALSE;
    SCSIZE nPos = 0;

    uno::Reference<container::XNameAccess> xDimsName(rSource->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (nType != SC_FIELDORIENT_ALL)
    {
        uno::Reference<beans::XPropertySet> xDim;
        sheet::DataPilotFieldOrientation aOrient;
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if (xDim.is())
            {
                xDim->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIENTAT))) >>= aOrient;
                if (aOrient == nType)
                {
                    if (nPos == nIndex)
                    {
                        bOk = sal_True;
                        rPos = static_cast<SCSIZE>(i);
                    }
                    else
                        ++nPos;
                }
            }
            ++i;
        }
    }
    else
    {
        if (nIndex < static_cast<SCSIZE>(nIntCount))
        {
            rPos = nIndex;
            bOk = sal_True;
        }
    }

    return bOk;
}

BOOL lcl_GetFieldDataByName( const com::sun::star::uno::Reference<com::sun::star::sheet::XDimensionsSupplier>& rSource,
                            USHORT nType, const rtl::OUString& sName, SCSIZE& nIndex )
{
    BOOL bOk = FALSE;
    SCSIZE nPos = 0;

    uno::Reference<container::XNameAccess> xDimsName(rSource->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (nType != SC_FIELDORIENT_ALL)
    {
        uno::Reference<container::XNamed> xDim;
        uno::Reference<beans::XPropertySet> xDimProp;
        sheet::DataPilotFieldOrientation aOrient;
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            xDimProp.set(xDim, uno::UNO_QUERY);
            if (xDim.is() && xDimProp.is())
            {
                xDimProp->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ORIENTAT))) >>= aOrient;
                if (aOrient == nType && xDim->getName() == sName)
                {
                    bOk = sal_True;
                    nIndex = nPos;
                }
                else
                    ++nPos;
            }
            ++i;
        }
    }
    else
    {
        uno::Reference<container::XNamed> xDim;
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), uno::UNO_QUERY);
            if (xDim.is())
            {
                if (xDim->getName() == sName)
                {
                    bOk = sal_True;
                    nIndex = nPos;
                }
                else
                    ++nPos;
            }
            ++i;
        }
    }

    return bOk;
}

// XDataPilotFields

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByIndex_Impl(SCSIZE nIndex) const
{
    ScDPObject* pObj = pParent->GetDPObject();

// TODO
    if (pObj)
    {
        ScDPSaveDimension* pDim = NULL;
        SCSIZE nSourceIndex;
        BOOL bOk = lcl_GetFieldDataByIndex( pObj->GetSource(), nType, nIndex, nSourceIndex );

        if (bOk)
            return new ScDataPilotFieldObj( pParent, nType, nSourceIndex );
    }

    return NULL;
}

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
    String aNameStr = aName;

// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());

    if (pDPObj)
    {
        ScDocShell* pDocSh = pParent->GetDocShell();
        SCSIZE nCount = lcl_GetFieldCount( pDPObj->GetSource(), nType );
        SCSIZE nIndex = 0;
        ScDPSaveDimension* pDim = NULL;
        for (SCSIZE i=0; i<nCount; i++)
        {
            if (lcl_GetFieldDataByName( pDPObj->GetSource(), nType, aName, nIndex ))
            {
                return new ScDataPilotFieldObj( pParent, nType, nIndex );
            }
        }
    }
    return NULL;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotFieldsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());

    return pDPObj ? static_cast<sal_Int32>(lcl_GetFieldCount( pDPObj->GetSource(), nType )) : 0;
}

uno::Any SAL_CALL ScDataPilotFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xField(GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex)));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
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
    uno::Reference<beans::XPropertySet> xField(GetObjectByName_Impl(aName));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotFieldsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    ScDPObject* pDPObj(pParent->GetDPObject());

    if (pDPObj)
    {
        ScDocShell* pDocSh = pParent->GetDocShell();

        uno::Sequence<rtl::OUString> aSeq(static_cast<sal_Int32>(lcl_GetFieldCount(pDPObj->GetSource(), nType)));
        rtl::OUString* pAry = aSeq.getArray();
        List aDimensions = pDPObj->GetSaveData()->GetDimensions();
        sal_Int32 nDimCount = aDimensions.Count();
        for (sal_Int32 nDim = 0; nDim < nDimCount; nDim++)
        {
            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimensions.GetObject(nDim);
            if(pDim->GetOrientation() == nType)
            {
                *pAry = pDim->GetName();
                ++pAry;
            }
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>();
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetObjectByName_Impl(aName) != NULL;
}

//------------------------------------------------------------------------

BOOL lcl_GetDim(ScDPObject* pDPObj, SCSIZE nSourcePos, ScDPSaveDimension*& rpDim)
{
    BOOL bRet = FALSE;
    uno::Reference<container::XNameAccess> xDimsName(pDPObj->GetSource()->getDimensions());
    uno::Reference<container::XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    uno::Reference<container::XNamed> xDim(xIntDims->getByIndex(static_cast<sal_Int32>(nSourcePos)), uno::UNO_QUERY);
    if (xDim.is())
    {
        ScDPSaveData* pSave = pDPObj->GetSaveData();
        if (pSave)
        {
            rpDim = pSave->GetDimensionByName(xDim->getName());
            if (rpDim)
                bRet = TRUE;
        }
    }
    return bRet;
}

ScDataPilotFieldObj::ScDataPilotFieldObj( ScDataPilotDescriptorBase* pPar,
                                            USHORT nST, SCSIZE nSP ) :
    aPropSet( lcl_GetDataPilotFieldMap() ),
    pParent( pPar ),
    nSourceType( nST ),
    nSourcePos( nSP ),
    nLastFunc( sheet::GeneralFunction_NONE )
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
// TODO
    rtl::OUString sRet;
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            if (pDim->IsDataLayout())
                return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Data"));     //! ???
            else
                sRet = pDim->GetLayoutName();
        }
    }
    return sRet;
}

void SAL_CALL ScDataPilotFieldObj::setName( const rtl::OUString& aNewName )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            if (!pDim->IsDataLayout())
            {
                String aName(aNewName);
                pDim->SetLayoutName(&aName);
                pParent->SetDPObject(pDPObj);
            }
        }
    }
}

// XPropertySet

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
    else if ( aNameString.EqualsAscii( SC_UNONAME_SELPAGE ) )
    {
        rtl::OUString sCurrentPage;
        if (aValue >>= sCurrentPage)
            setCurrentPage(sCurrentPage);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_USESELPAGE ) )
    {
        setUseCurrentPage(cppu::any2bool(aValue));
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_AUTOSHOW ) )
    {
        sheet::DataPilotFieldAutoShowInfo aInfo;
        if (aValue >>= aInfo)
            setAutoShowInfo(aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_LAYOUTINFO ) )
    {
        sheet::DataPilotFieldLayoutInfo aInfo;
        if (aValue >>= aInfo)
            setLayoutInfo(aInfo);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFERENCE ) )
    {
        sheet::DataPilotFieldReference aRef;
        if (aValue >>= aRef)
            setReference(aRef);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_SORTINFO ) )
    {
        sheet::DataPilotFieldSortInfo aInfo;
        if (aValue >>= aInfo)
            setSortInfo(aInfo);
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
        aRet <<= getFunction();
    else if ( aNameString.EqualsAscii( SC_UNONAME_ORIENT ) )
        aRet <<= getOrientation();
    else if ( aNameString.EqualsAscii( SC_UNONAME_SELPAGE ) )
        aRet <<= getCurrentPage();
    else if ( aNameString.EqualsAscii( SC_UNONAME_USESELPAGE ) )
        aRet <<= getUseCurrentPage();
    else if ( aNameString.EqualsAscii( SC_UNONAME_AUTOSHOW ) )
        aRet <<= getAutoShowInfo();
    else if ( aNameString.EqualsAscii( SC_UNONAME_LAYOUTINFO ) )
        aRet <<= getLayoutInfo();
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFERENCE ) )
        aRet <<= getReference();
    else if ( aNameString.EqualsAscii( SC_UNONAME_SORTINFO ) )
        aRet <<= getSortInfo();

    return aRet;
}

// XDatePilotField

uno::Reference<container::XIndexAccess> SAL_CALL ScDataPilotFieldObj::getItems()
                throw (uno::RuntimeException)
{
    if (!xItems.is())
        xItems.set(new ScDataPilotItemsObj(pParent, nSourcePos));
    return xItems;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDataPilotFieldObj )

sheet::DataPilotFieldOrientation ScDataPilotFieldObj::getOrientation(void) const
{
    sheet::DataPilotFieldOrientation eOrient = DATA_PILOT_HIDDEN;
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            eOrient = (sheet::DataPilotFieldOrientation)pDim->GetOrientation();
    }

    return eOrient;
}

void ScDataPilotFieldObj::setOrientation(sheet::DataPilotFieldOrientation eNew)
{
    if ( eNew == nSourceType )
        return;                     // nix

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            pDim->SetOrientation(eNew);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sheet::GeneralFunction ScDataPilotFieldObj::getFunction(void) const
{
    sheet::GeneralFunction eRet = sheet::GeneralFunction_NONE;

// TODO

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            eRet = (sheet::GeneralFunction)pDim->GetFunction();
    }

    return eRet;
}

void ScDataPilotFieldObj::setFunction(sheet::GeneralFunction eNewFunc)
{
// TODO
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            pDim->SetFunction(eNewFunc);
            pParent->SetDPObject(pDPObj);
        }
    }
}

rtl::OUString ScDataPilotFieldObj::getCurrentPage() const
{
    rtl::OUString sRet;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            if (pDim->HasCurrentPage())
                sRet = pDim->GetCurrentPage();
    }

    return sRet;
}

void ScDataPilotFieldObj::setCurrentPage(const rtl::OUString& sPage)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            String sCur(sPage);
            pDim->SetCurrentPage(&sCur);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sal_Bool ScDataPilotFieldObj::getUseCurrentPage() const
{
    sal_Bool bRet = sal_False;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            bRet = pDim->HasCurrentPage();
    }

    return bRet;
}

void ScDataPilotFieldObj::setUseCurrentPage(sal_Bool bUse)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
        {
            if (bUse)
            {
                String sCur;
                pDim->SetCurrentPage(&sCur);
            }
            else
                pDim->SetCurrentPage(NULL);
            pParent->SetDPObject(pDPObj);
        }
    }
}

sheet::DataPilotFieldAutoShowInfo ScDataPilotFieldObj::getAutoShowInfo()
{
    sheet::DataPilotFieldAutoShowInfo aInfo;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            aInfo = *(pDim->GetAutoShowInfo());
    }
    return aInfo;
}

void ScDataPilotFieldObj::setAutoShowInfo(const sheet::DataPilotFieldAutoShowInfo& aInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            pDim->SetAutoShowInfo(&aInfo);
    }
}

sheet::DataPilotFieldLayoutInfo ScDataPilotFieldObj::getLayoutInfo()
{
    sheet::DataPilotFieldLayoutInfo aInfo;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            aInfo = *(pDim->GetLayoutInfo());
    }

    return aInfo;
}

void ScDataPilotFieldObj::setLayoutInfo(const sheet::DataPilotFieldLayoutInfo& aInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            pDim->SetLayoutInfo(&aInfo);
    }
}

sheet::DataPilotFieldReference ScDataPilotFieldObj::getReference()
{
    sheet::DataPilotFieldReference aInfo;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            aInfo = *(pDim->GetReferenceValue());
    }

    return aInfo;
}

void ScDataPilotFieldObj::setReference(const sheet::DataPilotFieldReference& aInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            pDim->SetReferenceValue(&aInfo);
    }
}

sheet::DataPilotFieldSortInfo ScDataPilotFieldObj::getSortInfo()
{
    sheet::DataPilotFieldSortInfo aInfo;

    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            aInfo = *(pDim->GetSortInfo());
    }

    return aInfo;
}

void ScDataPilotFieldObj::setSortInfo(const sheet::DataPilotFieldSortInfo& aInfo)
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        if (lcl_GetDim(pDPObj, nSourcePos, pDim))
            pDim->SetSortInfo(&aInfo);
    }
}

//------------------------------------------------------------------------

ScDataPilotItemsObj::ScDataPilotItemsObj(ScDataPilotDescriptorBase* pPar, SCSIZE nSP) :
    pParent( pPar ),
    nSourcePos( nSP )
{
    pParent->acquire();
}

ScDataPilotItemsObj::~ScDataPilotItemsObj()
{
    pParent->release();
}

BOOL lcl_GetMembers( ScDataPilotDescriptorBase* pParent, SCSIZE nSP, uno::Reference<container::XNameAccess>& xMembers )
{
    ScDPObject* pDPObj(pParent->GetDPObject());
    return pDPObj && pDPObj->GetMembersNA( nSP, xMembers );
}

SCSIZE lcl_GetItemCount( ScDataPilotDescriptorBase* pParent, SCSIZE nSP )
{
    SCSIZE nRet = 0;

    uno::Reference<container::XNameAccess> xMembers;
    if (lcl_GetMembers(pParent, nSP, xMembers))
    {
        uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        nRet = static_cast<SCSIZE>(xMembersIndex->getCount());
    }

    return nRet;
}

// XDataPilotItemss

ScDataPilotItemObj* ScDataPilotItemsObj::GetObjectByIndex_Impl(SCSIZE nIndex) const
{
// TODO
    if (nIndex < lcl_GetItemCount(pParent, nSourcePos))
        return new ScDataPilotItemObj( pParent, nSourcePos, nIndex );

    return NULL;
}

// XNameAccess

uno::Any SAL_CALL ScDataPilotItemsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    {
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, nSourcePos, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            sal_Bool bFound(sal_False);
            sal_Int32 nItem = 0;
            while (nItem < nCount && !bFound )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(nItem), uno::UNO_QUERY);
                if (xMember.is() && aName == xMember->getName())
                    return uno::makeAny(uno::Reference<beans::XPropertySet> (GetObjectByIndex_Impl(static_cast<SCSIZE>(nItem))));
                else
                    nItem++;
            }
            if (!bFound)
                throw container::NoSuchElementException();
        }
    }

    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScDataPilotItemsObj::getElementNames()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO

    uno::Sequence<rtl::OUString> aSeq;
    if( ScDPObject* pDPObj = pParent->GetDPObject() )
        pDPObj->GetMembers( nSourcePos, aSeq );
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sal_Bool bFound(sal_False);
    {
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, nSourcePos, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            sal_Int32 nItem = 0;
            while (nItem < nCount && !bFound )
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(nItem), uno::UNO_QUERY);
                if (xMember.is() && aName == xMember->getName())
                    bFound = sal_True;
                else
                    nItem++;
            }
        }
    }

    return bFound;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDataPilotItemsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.DataPilotItemsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotItemsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
// TODO
    return static_cast<sal_Int32>(lcl_GetItemCount( pParent, nSourcePos ));
}

uno::Any SAL_CALL ScDataPilotItemsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xItem(GetObjectByIndex_Impl(static_cast<SCSIZE>(nIndex)));
    if (xItem.is())
        return uno::makeAny(xItem);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDataPilotItemsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

//------------------------------------------------------------------------

ScDataPilotItemObj::ScDataPilotItemObj(ScDataPilotDescriptorBase* pPar, SCSIZE nSP, SCSIZE nI)
    : pParent(pPar),
    aPropSet( lcl_GetDataPilotItemMap() ),
    nSourcePos (nSP),
    nIndex(nI)
{
    pParent->acquire();
}

ScDataPilotItemObj::~ScDataPilotItemObj()
{
    pParent->release();
}

                            // XNamed
::rtl::OUString SAL_CALL ScDataPilotItemObj::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString sRet;
    ScDPObject* pDPObj(pParent->GetDPObject());
    if (pDPObj)
    {
        ScDPSaveDimension* pDim = NULL;
        uno::Reference<container::XNameAccess> xMembers;
        if (lcl_GetMembers(pParent, nSourcePos, xMembers))
        {
            uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
            sal_Int32 nCount = xMembersIndex->getCount();
            if (nIndex < static_cast<SCSIZE>(nCount))
            {
                uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                sRet = xMember->getName();
            }
        }
    }
    return sRet;
}

void SAL_CALL ScDataPilotItemObj::setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException)
{
}

                            // XPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL ScDataPilotItemObj::getPropertySetInfo(  )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotItemObj::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    if ( aNameString.EqualsAscii( SC_UNONAME_SHOWDETAIL ) )
    {
        ScDPObject* pDPObj(pParent->GetDPObject());
        if (pDPObj)
        {
            uno::Reference<container::XNameAccess> xMembers;
            ScDPSaveDimension* pDim = NULL;
            if (lcl_GetMembers(pParent, nSourcePos, xMembers) && lcl_GetDim(pDPObj, nSourcePos, pDim))
            {
                uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
                sal_Int32 nCount = xMembersIndex->getCount();
                if (nIndex < static_cast<SCSIZE>(nCount) )
                {
                    uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                    String sName(xMember->getName());
                    ScDPSaveMember* pMember = pDim->GetMemberByName(sName);
                    if (pMember)
                    {
                        pMember->SetShowDetails(cppu::any2bool(aValue));
                        pParent->SetDPObject(pDPObj);
                    }
                }
            }
        }
    }
}

::com::sun::star::uno::Any SAL_CALL ScDataPilotItemObj::getPropertyValue(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;
    String aNameString = aPropertyName;
    if ( aNameString.EqualsAscii( SC_UNONAME_SHOWDETAIL ) )
    {
        ScDPObject* pDPObj(pParent->GetDPObject());
        if (pDPObj)
        {
            uno::Reference<container::XNameAccess> xMembers;
            ScDPSaveDimension* pDim = NULL;
            if (lcl_GetMembers(pParent, nSourcePos, xMembers) && lcl_GetDim(pDPObj, nSourcePos, pDim))
            {
                uno::Reference<container::XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
                sal_Int32 nCount = xMembersIndex->getCount();
                if (nIndex < static_cast<SCSIZE>(nCount) )
                {
                    uno::Reference<container::XNamed> xMember(xMembersIndex->getByIndex(static_cast<sal_Int32>(nIndex)), uno::UNO_QUERY);
                    String sName(xMember->getName());
                    ScDPSaveMember* pMember = pDim->GetExistingMemberByName(sName);
                    if (pMember && pMember->HasShowDetails())
                    {
                        aRet = cppu::bool2any(pMember->GetShowDetails());
                    }
                    else
                    {
                        uno::Reference<beans::XPropertySet> xMemberProps(xMember, uno::UNO_QUERY);
                        if(xMemberProps.is())
                        {
                            aRet = xMemberProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHOWDETA)));
                        }
                    }
                }
            }
        }
    }
    return aRet;
}

void SAL_CALL ScDataPilotItemObj::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------




