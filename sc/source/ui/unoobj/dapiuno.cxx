/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <algorithm>
#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>

#include "dapiuno.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "pivot.hxx"
#include "rangeutl.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dpsave.hxx"
#include "dbdocfun.hxx"
#include "unonames.hxx"
#include "dpgroup.hxx"
#include "dpdimsave.hxx"
#include "hints.hxx"

#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>

using namespace com::sun::star;
using namespace com::sun::star::sheet;


using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;

using ::com::sun::star::container::ElementExistException;
using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNamed;

using ::com::sun::star::beans::PropertyVetoException;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::beans::XVetoableChangeListener;

using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::lang::WrappedTargetException;

using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;



namespace {

const SfxItemPropertyMapEntry* lcl_GetDataPilotDescriptorBaseMap()
{
    static const SfxItemPropertyMapEntry aDataPilotDescriptorBaseMap_Impl[] =
    {
        {OUString(SC_UNO_DP_COLGRAND),     0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_DRILLDOWN),    0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_GRANDTOTAL_NAME),0,getCppuType((OUString*)0), beans::PropertyAttribute::MAYBEVOID, 0 },
        {OUString(SC_UNO_DP_IGNORE_EMPTYROWS),   0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_IMPORTDESC),   0,  getCppuType((uno::Sequence<beans::PropertyValue>*)0), 0, 0 },
        {OUString(SC_UNO_DP_REPEATEMPTY),     0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_ROWGRAND),     0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_SERVICEARG),   0,  getCppuType((uno::Sequence<beans::PropertyValue>*)0), 0, 0 },
        {OUString(SC_UNO_DP_SHOWFILTER),     0,  getBooleanCppuType(),  0, 0 },
        {OUString(SC_UNO_DP_SOURCESERVICE),   0,  getCppuType((OUString*)0), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotDescriptorBaseMap_Impl;
}



const SfxItemPropertyMapEntry* lcl_GetDataPilotFieldMap()
{
    using namespace ::com::sun::star::beans::PropertyAttribute;
    static const SfxItemPropertyMapEntry aDataPilotFieldMap_Impl[] =
    {
        {OUString(SC_UNONAME_AUTOSHOW),     0,  getCppuType((DataPilotFieldAutoShowInfo*)0),   MAYBEVOID, 0 },
        {OUString(SC_UNONAME_FUNCTION),     0,  getCppuType((GeneralFunction*)0),              0, 0 },
        {OUString(SC_UNONAME_GROUPINFO),    0,  getCppuType((DataPilotFieldGroupInfo*)0),      MAYBEVOID, 0 },
        {OUString(SC_UNONAME_HASAUTOSHOW),  0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_HASLAYOUTINFO),0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_HASREFERENCE), 0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_HASSORTINFO),  0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_ISGROUP),      0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_LAYOUTINFO),   0,  getCppuType((DataPilotFieldLayoutInfo*)0),     MAYBEVOID, 0 },
        {OUString(SC_UNONAME_ORIENT),       0,  getCppuType((DataPilotFieldOrientation*)0),    MAYBEVOID, 0 },
        {OUString(SC_UNONAME_REFERENCE),    0,  getCppuType((DataPilotFieldReference*)0),      MAYBEVOID, 0 },
        {OUString(SC_UNONAME_SELPAGE),      0,  getCppuType((OUString*)0),                     0, 0 },
        {OUString(SC_UNONAME_SHOWEMPTY),    0,  getBooleanCppuType(),                          0, 0 },
        {OUString(SC_UNONAME_SORTINFO),     0,  getCppuType((DataPilotFieldSortInfo*)0),       MAYBEVOID, 0 },
        {OUString(SC_UNONAME_SUBTOTALS),    0,  getCppuType((Sequence<GeneralFunction>*)0),    0, 0 },
        {OUString(SC_UNONAME_USESELPAGE),   0,  getBooleanCppuType(),                          0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotFieldMap_Impl;
}



const SfxItemPropertyMapEntry* lcl_GetDataPilotItemMap()
{
    static const SfxItemPropertyMapEntry aDataPilotItemMap_Impl[] =
    {
        {OUString(SC_UNONAME_ISHIDDEN),     0,  getBooleanCppuType(),          0, 0 },
        {OUString(SC_UNONAME_POS),          0,  getCppuType((sal_Int32*)0),    0, 0 },
        {OUString(SC_UNONAME_SHOWDETAIL),   0,  getBooleanCppuType(),          0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotItemMap_Impl;
}



inline bool lclCheckValidDouble( double fValue, sal_Bool bAuto )
{
    return bAuto || ::rtl::math::isFinite( fValue );
}

bool lclCheckMinMaxStep( const DataPilotFieldGroupInfo& rInfo )
{
    return
        lclCheckValidDouble( rInfo.Start, rInfo.HasAutoStart ) &&
        lclCheckValidDouble( rInfo.End, rInfo.HasAutoEnd ) &&
        (rInfo.HasAutoStart || rInfo.HasAutoEnd || (rInfo.Start <= rInfo.End)) &&
        lclCheckValidDouble( rInfo.Step, false ) &&
        (0.0 <= rInfo.Step);
}

} 



SC_SIMPLE_SERVICE_INFO( ScDataPilotDescriptor, "ScDataPilotDescriptor", "stardiv::one::sheet::DataPilotDescriptor" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldObj, "ScDataPilotFieldObj", "com.sun.star.sheet.DataPilotField" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldsObj, "ScDataPilotFieldsObj", "com.sun.star.sheet.DataPilotFields" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTableObj, "ScDataPilotTableObj", "com.sun.star.sheet.DataPilotTable" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotTablesObj, "ScDataPilotTablesObj", "com.sun.star.sheet.DataPilotTables" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemsObj, "ScDataPilotItemsObj", "com.sun.star.sheet.DataPilotItems" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotItemObj, "ScDataPilotItemObj", "com.sun.star.sheet.DataPilotItem" )

SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupsObj, "ScDataPilotFieldGroupsObj", "com.sun.star.sheet.DataPilotFieldGroups" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupObj, "ScDataPilotFieldGroupObj", "com.sun.star.sheet.DataPilotFieldGroup" )
SC_SIMPLE_SERVICE_INFO( ScDataPilotFieldGroupItemObj, "ScDataPilotFieldGroupItemObj", "com.sun.star.sheet.DataPilotFieldGroupItem" )




#define SC_DATALAYOUT_NAME  "Data"



GeneralFunction ScDataPilotConversion::FirstFunc( sal_uInt16 nBits )
{
    if ( nBits & PIVOT_FUNC_SUM )       return GeneralFunction_SUM;
    if ( nBits & PIVOT_FUNC_COUNT )     return GeneralFunction_COUNT;
    if ( nBits & PIVOT_FUNC_AVERAGE )   return GeneralFunction_AVERAGE;
    if ( nBits & PIVOT_FUNC_MAX )       return GeneralFunction_MAX;
    if ( nBits & PIVOT_FUNC_MIN )       return GeneralFunction_MIN;
    if ( nBits & PIVOT_FUNC_PRODUCT )   return GeneralFunction_PRODUCT;
    if ( nBits & PIVOT_FUNC_COUNT_NUM ) return GeneralFunction_COUNTNUMS;
    if ( nBits & PIVOT_FUNC_STD_DEV )   return GeneralFunction_STDEV;
    if ( nBits & PIVOT_FUNC_STD_DEVP )  return GeneralFunction_STDEVP;
    if ( nBits & PIVOT_FUNC_STD_VAR )   return GeneralFunction_VAR;
    if ( nBits & PIVOT_FUNC_STD_VARP )  return GeneralFunction_VARP;
    if ( nBits & PIVOT_FUNC_AUTO )      return GeneralFunction_AUTO;
    return GeneralFunction_NONE;
}

sal_uInt16 ScDataPilotConversion::FunctionBit( GeneralFunction eFunc )
{
    sal_uInt16 nRet = PIVOT_FUNC_NONE;  
    switch (eFunc)
    {
        case GeneralFunction_SUM:       nRet = PIVOT_FUNC_SUM;       break;
        case GeneralFunction_COUNT:     nRet = PIVOT_FUNC_COUNT;     break;
        case GeneralFunction_AVERAGE:   nRet = PIVOT_FUNC_AVERAGE;   break;
        case GeneralFunction_MAX:       nRet = PIVOT_FUNC_MAX;       break;
        case GeneralFunction_MIN:       nRet = PIVOT_FUNC_MIN;       break;
        case GeneralFunction_PRODUCT:   nRet = PIVOT_FUNC_PRODUCT;   break;
        case GeneralFunction_COUNTNUMS: nRet = PIVOT_FUNC_COUNT_NUM; break;
        case GeneralFunction_STDEV:     nRet = PIVOT_FUNC_STD_DEV;   break;
        case GeneralFunction_STDEVP:    nRet = PIVOT_FUNC_STD_DEVP;  break;
        case GeneralFunction_VAR:       nRet = PIVOT_FUNC_STD_VAR;   break;
        case GeneralFunction_VARP:      nRet = PIVOT_FUNC_STD_VARP;  break;
        case GeneralFunction_AUTO:      nRet = PIVOT_FUNC_AUTO;      break;
        default:
        {
            
        }
    }
    return nRet;
}

void ScDataPilotConversion::FillGroupInfo( DataPilotFieldGroupInfo& rInfo, const ScDPNumGroupInfo& rGroupInfo )
{
    rInfo.HasDateValues = rGroupInfo.mbDateValues;
    rInfo.HasAutoStart  = rGroupInfo.mbAutoStart;
    rInfo.Start         = rGroupInfo.mfStart;
    rInfo.HasAutoEnd    = rGroupInfo.mbAutoEnd;
    rInfo.End           = rGroupInfo.mfEnd;
    rInfo.Step          = rGroupInfo.mfStep;
}



static ScDPObject* lcl_GetDPObject( ScDocShell* pDocShell, SCTAB nTab, const OUString& rName )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == rName )
                    return pDPObj;
            }
        }
    }
    return NULL;    
}

static OUString lcl_CreatePivotName( ScDocShell* pDocShell )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
            return pColl->CreateNewName();
    }
    return OUString();                    
}

static sal_Int32 lcl_GetObjectIndex( ScDPObject* pDPObj, const ScFieldIdentifier& rFieldId )
{
    
    if ( pDPObj )
    {
        sal_Int32 nCount = pDPObj->GetDimCount();
        for ( sal_Int32 nDim = 0; nDim < nCount; ++nDim )
        {
            bool bIsDataLayout = false;
            OUString aDimName( pDPObj->GetDimName( nDim, bIsDataLayout ) );
            if ( rFieldId.mbDataLayout ? bIsDataLayout : (aDimName == rFieldId.maFieldName) )
                return nDim;
        }
    }
    return -1;  
}



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

void ScDataPilotTablesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByIndex_Impl( sal_Int32 nIndex )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            
            sal_Int32 nFound = 0;
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab )
                {
                    if ( nFound == nIndex )
                    {
                        OUString aName = pDPObj->GetName();
                        return new ScDataPilotTableObj( pDocShell, nTab, aName );
                    }
                    ++nFound;
                }
            }
        }
    }
    return NULL;
}

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByName_Impl(const OUString& rName)
{
    if (hasByName(rName))
        return new ScDataPilotTableObj( pDocShell, nTab, rName );
    return 0;
}

Reference<XDataPilotDescriptor> SAL_CALL ScDataPilotTablesObj::createDataPilotDescriptor()
                                            throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScDataPilotDescriptor(pDocShell);
    return NULL;
}

static bool lcl_IsDuplicated( const Reference<XPropertySet> xDimProps )
{
    try
    {
        Any aAny = xDimProps->getPropertyValue( OUString( SC_UNO_DP_ORIGINAL ) );
        Reference< XNamed > xOriginal( aAny, UNO_QUERY );
        return xOriginal.is();
    }
    catch( Exception& )
    {
    }
    return false;
}

static OUString lcl_GetOriginalName( const Reference< XNamed > xDim )
{
    Reference< XNamed > xOriginal;

    Reference< XPropertySet > xDimProps( xDim, UNO_QUERY );
    if ( xDimProps.is() )
    {
        try
        {
            Any aAny = xDimProps->getPropertyValue(OUString(SC_UNO_DP_ORIGINAL));
            aAny >>= xOriginal;
        }
        catch( Exception& )
        {
        }
    }

    if ( !xOriginal.is() )
        xOriginal = xDim;

    return xOriginal->getName();
}

void SAL_CALL ScDataPilotTablesObj::insertNewByName( const OUString& aNewName,
    const CellAddress& aOutputAddress,
    const Reference<XDataPilotDescriptor>& xDescriptor )
        throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!xDescriptor.is()) return;

    
    if ( !aNewName.isEmpty() && hasByName( aNewName ) )
        throw RuntimeException();       

    sal_Bool bDone = false;
    ScDataPilotDescriptorBase* pImp = ScDataPilotDescriptorBase::getImplementation( xDescriptor );
    if ( pDocShell && pImp )
    {
        ScDPObject* pNewObj = pImp->GetDPObject();

        if (pNewObj)
        {
            ScRange aOutputRange((SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet,
                                (SCCOL)aOutputAddress.Column, (SCROW)aOutputAddress.Row, (SCTAB)aOutputAddress.Sheet);
            pNewObj->SetOutRange(aOutputRange);
            OUString aName = aNewName;
            if (aName.isEmpty())
                aName = lcl_CreatePivotName( pDocShell );
            pNewObj->SetName(aName);
            OUString aTag = xDescriptor->getTag();
            pNewObj->SetTag(aTag);

    

            ScDBDocFunc aFunc(*pDocShell);
            bDone = aFunc.CreatePivotTable(*pNewObj, true, true);
        }
    }

    if (!bDone)
        throw RuntimeException();       
}

void SAL_CALL ScDataPilotTablesObj::removeByName( const OUString& aName )
                                        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameStr(aName);
    ScDPObject* pDPObj = lcl_GetDPObject( pDocShell, nTab, aNameStr );
    if (pDPObj && pDocShell)
    {
        ScDBDocFunc aFunc(*pDocShell);
        aFunc.RemovePivotTable(*pDPObj, true, true);  
    }
    else
        throw RuntimeException();       
}



Reference< XEnumeration > SAL_CALL ScDataPilotTablesObj::createEnumeration() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.DataPilotTablesEnumeration"));
}



sal_Int32 SAL_CALL ScDataPilotTablesObj::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            

            sal_uInt16 nFound = 0;
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }
            return nFound;
        }
    }

    return 0;
}

Any SAL_CALL ScDataPilotTablesObj::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XDataPilotTable2> xTable(GetObjectByIndex_Impl(nIndex));
    if (!xTable.is())
        throw IndexOutOfBoundsException();
    return Any( xTable );
}

uno::Type SAL_CALL ScDataPilotTablesObj::getElementType() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((Reference<XDataPilotTable2>*)0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



Any SAL_CALL ScDataPilotTablesObj::getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XDataPilotTable2> xTable(GetObjectByName_Impl(aName));
    if (!xTable.is())
        throw NoSuchElementException();
    return Any( xTable );
}

Sequence<OUString> SAL_CALL ScDataPilotTablesObj::getElementNames()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            

            sal_uInt16 nFound = 0;
            size_t nCount = pColl->GetCount();
            size_t i;
            for (i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }

            sal_uInt16 nPos = 0;
            Sequence<OUString> aSeq(nFound);
            OUString* pAry = aSeq.getArray();
            for (i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab )
                    pAry[nPos++] = pDPObj->GetName();
            }

            return aSeq;
        }
    }
    return Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasByName( const OUString& aName )
                                        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = pDoc->GetDPCollection();
        if ( pColl )
        {
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject* pDPObj = (*pColl)[i];
                if ( pDPObj->GetOutRange().aStart.Tab() == nTab &&
                     pDPObj->GetName() == aName )
                    return true;
            }
        }
    }
    return false;
}



ScDataPilotDescriptorBase::ScDataPilotDescriptorBase(ScDocShell* pDocSh) :
    maPropSet( lcl_GetDataPilotDescriptorBaseMap() ),
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDataPilotDescriptorBase::~ScDataPilotDescriptorBase()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

Any SAL_CALL ScDataPilotDescriptorBase::queryInterface( const uno::Type& rType )
                                                throw(RuntimeException)
{
    SC_QUERYINTERFACE( XDataPilotDescriptor )
    SC_QUERYINTERFACE( XPropertySet )
    SC_QUERYINTERFACE( XDataPilotDataLayoutFieldSupplier )
    SC_QUERYINTERFACE( XNamed )                 
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

Sequence< uno::Type > SAL_CALL ScDataPilotDescriptorBase::getTypes()
                                                    throw(RuntimeException)
{
    static Sequence< uno::Type > aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc( 6 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[ 0 ] = getCppuType( (const Reference< XDataPilotDescriptor >*)0 );
        pPtr[ 1 ] = getCppuType( (const Reference< XPropertySet >*)0 );
        pPtr[ 2 ] = getCppuType( (const Reference< XDataPilotDataLayoutFieldSupplier >*)0 );
        pPtr[ 3 ] = getCppuType( (const Reference< lang::XUnoTunnel >*)0 );
        pPtr[ 4 ] = getCppuType( (const Reference< lang::XTypeProvider >*)0 );
        pPtr[ 5 ] = getCppuType( (const Reference< lang::XServiceInfo >*)0 );
    }
    return aTypes;
}

namespace
{
    class theScDataPilotDescriptorBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theScDataPilotDescriptorBaseImplementationId > {};
}

Sequence<sal_Int8> SAL_CALL ScDataPilotDescriptorBase::getImplementationId()
                                                    throw(RuntimeException)
{
    return theScDataPilotDescriptorBaseImplementationId::get().getSeq();
}

void ScDataPilotDescriptorBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       
    }
}



CellRangeAddress SAL_CALL ScDataPilotDescriptorBase::getSourceRange()
                                            throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    ScDPObject* pDPObject(GetDPObject());
    if (!pDPObject)
        throw RuntimeException();

    CellRangeAddress aRet;
    if (pDPObject->IsSheetData())
        ScUnoConversion::FillApiRange( aRet, pDPObject->GetSheetDesc()->GetSourceRange() );
    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::setSourceRange( const CellRangeAddress& aSourceRange ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    ScDPObject* pDPObject = GetDPObject();
    if (!pDPObject)
        throw RuntimeException();

    ScSheetSourceDesc aSheetDesc(pDocShell->GetDocument());
    if (pDPObject->IsSheetData())
        aSheetDesc = *pDPObject->GetSheetDesc();

    ScRange aRange;
    ScUnoConversion::FillScRange(aRange, aSourceRange);
    aSheetDesc.SetSourceRange(aRange);
    pDPObject->SetSheetDesc( aSheetDesc );
    SetDPObject( pDPObject );
}

Reference<XSheetFilterDescriptor> SAL_CALL ScDataPilotDescriptorBase::getFilterDescriptor()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFilterDescriptor( pDocShell, this );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataPilotFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getColumnFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_COLUMN );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getRowFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_ROW );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getPageFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_PAGE );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_DATA );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getHiddenFields()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_HIDDEN );
}


Reference< XPropertySetInfo > SAL_CALL ScDataPilotDescriptorBase::getPropertySetInfo(  )
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotDescriptorBase::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
                WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObject = GetDPObject();
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        OSL_ENSURE(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            OUString aNameString = aPropertyName;
            if ( aNameString.equalsAscii( SC_UNO_DP_COLGRAND ) )
            {
                aNewData.SetColumnGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_IGNORE_EMPTYROWS ) )
            {
                aNewData.SetIgnoreEmptyRows(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_REPEATEMPTY ) )
            {
                aNewData.SetRepeatIfEmpty(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_ROWGRAND ) )
            {
                aNewData.SetRowGrand(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SHOWFILTER ) )
            {
                aNewData.SetFilterButton(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_DRILLDOWN ) )
            {
                aNewData.SetDrillDown(::cppu::any2bool( aValue ));
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_GRANDTOTAL_NAME ) )
            {
                OUString aStrVal;
                if ( aValue >>= aStrVal )
                    aNewData.SetGrandTotalName(aStrVal);
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_IMPORTDESC ) )
            {
                uno::Sequence<beans::PropertyValue> aArgSeq;
                if ( aValue >>= aArgSeq )
                {
                    ScImportSourceDesc aImportDesc(pDocShell->GetDocument());

                    const ScImportSourceDesc* pOldDesc = pDPObject->GetImportSourceDesc();
                    if (pOldDesc)
                        aImportDesc = *pOldDesc;

                    ScImportParam aParam;
                    ScImportDescriptor::FillImportParam( aParam, aArgSeq );

                    sal_uInt16 nNewType = sheet::DataImportMode_NONE;
                    if ( aParam.bImport )
                    {
                        if ( aParam.bSql )
                            nNewType = sheet::DataImportMode_SQL;
                        else if ( aParam.nType == ScDbQuery )
                            nNewType = sheet::DataImportMode_QUERY;
                        else
                            nNewType = sheet::DataImportMode_TABLE;
                    }
                    aImportDesc.nType   = nNewType;
                    aImportDesc.aDBName = aParam.aDBName;
                    aImportDesc.aObject = aParam.aStatement;
                    aImportDesc.bNative = aParam.bNative;

                    pDPObject->SetImportDesc( aImportDesc );
                }
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SOURCESERVICE ) )
            {
                OUString aStrVal;
                if ( aValue >>= aStrVal )
                {
                    OUString aEmpty;
                    ScDPServiceDesc aServiceDesc(aEmpty, aEmpty, aEmpty, aEmpty, aEmpty);

                    const ScDPServiceDesc* pOldDesc = pDPObject->GetDPServiceDesc();
                    if (pOldDesc)
                        aServiceDesc = *pOldDesc;

                    aServiceDesc.aServiceName = aStrVal;

                    pDPObject->SetServiceData( aServiceDesc );
                }
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SERVICEARG ) )
            {
                uno::Sequence<beans::PropertyValue> aArgSeq;
                if ( aValue >>= aArgSeq )
                {
                    OUString aEmpty;
                    ScDPServiceDesc aServiceDesc(aEmpty, aEmpty, aEmpty, aEmpty, aEmpty);

                    const ScDPServiceDesc* pOldDesc = pDPObject->GetDPServiceDesc();
                    if (pOldDesc)
                        aServiceDesc = *pOldDesc;

                    OUString aStrVal;
                    sal_Int32 nArgs = aArgSeq.getLength();
                    for (sal_Int32 nArgPos=0; nArgPos<nArgs; ++nArgPos)
                    {
                        const beans::PropertyValue& rProp = aArgSeq[nArgPos];
                        OUString aPropName(rProp.Name);

                        if (aPropName.equalsAscii( SC_UNO_DP_SOURCENAME ))
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParSource = aStrVal;
                        }
                        else if (aPropName.equalsAscii( SC_UNO_DP_OBJECTNAME ))
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParName = aStrVal;
                        }
                        else if (aPropName.equalsAscii( SC_UNO_DP_USERNAME ))
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParUser = aStrVal;
                        }
                        else if (aPropName.equalsAscii( SC_UNO_DP_PASSWORD ))
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParPass = aStrVal;
                        }
                    }

                    pDPObject->SetServiceData( aServiceDesc );
                }
            }
            else
                throw UnknownPropertyException();

            pDPObject->SetSaveData( aNewData );
        }

        SetDPObject(pDPObject);
    }
}

Any SAL_CALL ScDataPilotDescriptorBase::getPropertyValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Any aRet;

    ScDPObject* pDPObject(GetDPObject());
    if (pDPObject)
    {
        ScDPSaveData* pOldData = pDPObject->GetSaveData();
        OSL_ENSURE(pOldData, "Here should be a SaveData");
        if ( pOldData )
        {
            ScDPSaveData aNewData( *pOldData );

            OUString aNameString = aPropertyName;
            if ( aNameString.equalsAscii( SC_UNO_DP_COLGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetColumnGrand() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_IGNORE_EMPTYROWS ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetIgnoreEmptyRows() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_REPEATEMPTY ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRepeatIfEmpty() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_ROWGRAND ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetRowGrand() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SHOWFILTER ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetFilterButton() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_DRILLDOWN ) )
            {
                aRet = ::cppu::bool2any( aNewData.GetDrillDown() );
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_GRANDTOTAL_NAME ) )
            {
                const OUString* pGrandTotalName = aNewData.GetGrandTotalName();
                if (pGrandTotalName)
                    aRet <<= *pGrandTotalName;      
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_IMPORTDESC ) )
            {
                const ScImportSourceDesc* pImportDesc = pDPObject->GetImportSourceDesc();
                if ( pImportDesc )
                {
                    
                    ScImportParam aParam;
                    aParam.bImport    = ( pImportDesc->nType != sheet::DataImportMode_NONE );
                    aParam.aDBName    = pImportDesc->aDBName;
                    aParam.aStatement = pImportDesc->aObject;
                    aParam.bNative    = pImportDesc->bNative;
                    aParam.bSql       = ( pImportDesc->nType == sheet::DataImportMode_SQL );
                    aParam.nType      = static_cast<sal_uInt8>(( pImportDesc->nType == sheet::DataImportMode_QUERY ) ? ScDbQuery : ScDbTable);

                    uno::Sequence<beans::PropertyValue> aSeq( ScImportDescriptor::GetPropertyCount() );
                    ScImportDescriptor::FillProperties( aSeq, aParam );
                    aRet <<= aSeq;
                }
                else
                {
                    
                    uno::Sequence<beans::PropertyValue> aEmpty(0);
                    aRet <<= aEmpty;
                }
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SOURCESERVICE ) )
            {
                OUString aServiceName;
                const ScDPServiceDesc* pServiceDesc = pDPObject->GetDPServiceDesc();
                if (pServiceDesc)
                    aServiceName = pServiceDesc->aServiceName;
                aRet <<= aServiceName;      
            }
            else if ( aNameString.equalsAscii( SC_UNO_DP_SERVICEARG ) )
            {
                const ScDPServiceDesc* pServiceDesc = pDPObject->GetDPServiceDesc();
                if (pServiceDesc)
                {
                    uno::Sequence<beans::PropertyValue> aSeq( 4 );
                    beans::PropertyValue* pArray = aSeq.getArray();
                    pArray[0].Name = OUString( SC_UNO_DP_SOURCENAME );
                    pArray[0].Value <<= pServiceDesc->aParSource;
                    pArray[1].Name = OUString( SC_UNO_DP_OBJECTNAME );
                    pArray[1].Value <<= pServiceDesc->aParName;
                    pArray[2].Name = OUString( SC_UNO_DP_USERNAME );
                    pArray[2].Value <<= pServiceDesc->aParUser;
                    pArray[3].Name = OUString( SC_UNO_DP_PASSWORD );
                    pArray[3].Value <<= pServiceDesc->aParPass;
                    aRet <<= aSeq;
                }
                else
                {
                    
                    uno::Sequence<beans::PropertyValue> aEmpty(0);
                    aRet <<= aEmpty;
                }
            }
            else
                throw UnknownPropertyException();
        }
    }

    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::addPropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference<XPropertyChangeListener >& /* xListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removePropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference<XPropertyChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::addVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference<XVetoableChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotDescriptorBase::removeVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference<XVetoableChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}



Reference< XDataPilotField > SAL_CALL ScDataPilotDescriptorBase::getDataLayoutField() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if( ScDPObject* pDPObject = GetDPObject() )
    {
        if( ScDPSaveData* pSaveData = pDPObject->GetSaveData() )
        {
            if( pSaveData->GetDataLayoutDimension() )
            {
                ScFieldIdentifier aFieldId( OUString( SC_DATALAYOUT_NAME ), 0, true );
                return new ScDataPilotFieldObj( *this, aFieldId );
            }
        }
    }
    return 0;
}



sal_Int64 SAL_CALL ScDataPilotDescriptorBase::getSomething(
                const Sequence<sal_Int8 >& rId ) throw(RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScDataPilotDescriptorBaseUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScDataPilotDescriptorBaseUnoTunnelId> {};
}

const Sequence<sal_Int8>& ScDataPilotDescriptorBase::getUnoTunnelId()
{
    return theScDataPilotDescriptorBaseUnoTunnelId::get().getSeq();
}

ScDataPilotDescriptorBase* ScDataPilotDescriptorBase::getImplementation(
                                const Reference<XDataPilotDescriptor> xObj )
{
    ScDataPilotDescriptorBase* pRet = NULL;
    Reference<lang::XUnoTunnel> xUT( xObj, UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScDataPilotDescriptorBase*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}



ScDataPilotTableObj::ScDataPilotTableObj(ScDocShell* pDocSh, SCTAB nT, const OUString& rN) :
    ScDataPilotDescriptorBase( pDocSh ),
    nTab( nT ),
    aName( rN ),
    aModifyListeners( 0 )
{
}

ScDataPilotTableObj::~ScDataPilotTableObj()
{
}

Any SAL_CALL ScDataPilotTableObj::queryInterface( const uno::Type& rType )
                                                throw(RuntimeException)
{
    
    
    SC_QUERYINTERFACE( XDataPilotTable )
    SC_QUERYINTERFACE( XDataPilotTable2 )
    SC_QUERYINTERFACE( XModifyBroadcaster )

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

Sequence< uno::Type > SAL_CALL ScDataPilotTableObj::getTypes() throw(RuntimeException)
{
    static Sequence< uno::Type > aTypes;
    if ( aTypes.getLength() == 0 )
    {
        Sequence< uno::Type > aParentTypes = ScDataPilotDescriptorBase::getTypes();
        sal_Int32 nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 2 );
        uno::Type* pPtr = aTypes.getArray();
        for (sal_Int32 i = 0; i < nParentLen; ++i)
            pPtr[ i ] = pParentPtr[ i ];               

        pPtr[ nParentLen ] = getCppuType( (const Reference< XDataPilotTable2 >*)0 );
        pPtr[ nParentLen+1 ] = getCppuType( (const Reference< XModifyBroadcaster >*)0 );
    }
    return aTypes;
}

namespace
{
    class theScDataPilotTableObjImplementationId : public rtl::Static< UnoTunnelIdInit, theScDataPilotTableObjImplementationId > {};
}

Sequence<sal_Int8> SAL_CALL ScDataPilotTableObj::getImplementationId()
                                                    throw(RuntimeException)
{
    return theScDataPilotTableObjImplementationId::get().getSeq();
}


ScDPObject* ScDataPilotTableObj::GetDPObject() const
{
    return lcl_GetDPObject(GetDocShell(), nTab, aName);
}

void ScDataPilotTableObj::SetDPObject( ScDPObject* pDPObject )
{
    ScDocShell* pDocSh = GetDocShell();
    ScDPObject* pDPObj = lcl_GetDPObject(pDocSh, nTab, aName);
    if ( pDPObj && pDocSh )
    {
        ScDBDocFunc aFunc(*pDocSh);
        aFunc.DataPilotUpdate( pDPObj, pDPObject, true, true );
    }
}



OUString SAL_CALL ScDataPilotTableObj::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetName();
    return OUString();
}

void SAL_CALL ScDataPilotTableObj::setName( const OUString& aNewName )
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        

        OUString aString(aNewName);
        pDPObj->SetName( aString );     
        aName = aString;

        
        GetDocShell()->SetDocumentModified();
    }
}

OUString SAL_CALL ScDataPilotTableObj::getTag() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetTag();
    return OUString();
}

void SAL_CALL ScDataPilotTableObj::setTag( const OUString& aNewTag )
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        pDPObj->SetTag( aNewTag );      

        
        GetDocShell()->SetDocumentModified();
    }
}



CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRange() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    CellRangeAddress aRet;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScRange aRange(pDPObj->GetOutRange());
        aRet.Sheet       = aRange.aStart.Tab();
        aRet.StartColumn = aRange.aStart.Col();
        aRet.StartRow    = aRange.aStart.Row();
        aRet.EndColumn   = aRange.aEnd.Col();
        aRet.EndRow      = aRange.aEnd.Row();
    }
    return aRet;
}

void SAL_CALL ScDataPilotTableObj::refresh() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        ScDBDocFunc aFunc(*GetDocShell());
        aFunc.RefreshPivotTables(pDPObj, true);
    }
}

Sequence< Sequence<Any> > SAL_CALL ScDataPilotTableObj::getDrillDownData(const CellAddress& aAddr)
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< Sequence<Any> > aTabData;
    ScAddress aAddr2(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet);
    ScDPObject* pObj = GetDPObject();
    if (!pObj)
        throw RuntimeException();

    pObj->GetDrillDownData(aAddr2, aTabData);
    return aTabData;
}

DataPilotTablePositionData SAL_CALL ScDataPilotTableObj::getPositionData(const CellAddress& aAddr)
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    DataPilotTablePositionData aPosData;
    ScAddress aAddr2(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet);
    ScDPObject* pObj = GetDPObject();
    if (!pObj)
        throw RuntimeException();

    pObj->GetPositionData(aAddr2, aPosData);
    return aPosData;
}

void SAL_CALL ScDataPilotTableObj::insertDrillDownSheet(const CellAddress& aAddr)
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = GetDPObject();
    if (!pDPObj)
        throw RuntimeException();

    Sequence<DataPilotFieldFilter> aFilters;
    pDPObj->GetDataFieldPositionData(
        ScAddress(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet), aFilters);
    GetDocShell()->GetBestViewShell()->ShowDataPilotSourceData(*pDPObj, aFilters);
}

CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRangeByType( sal_Int32 nType )
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nType < 0 || nType > DataPilotOutputRangeType::RESULT)
        throw IllegalArgumentException();

    CellRangeAddress aRet;
    if (ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName))
        ScUnoConversion::FillApiRange( aRet, pDPObj->GetOutputRangeByType( nType ) );
    return aRet;
}

void SAL_CALL ScDataPilotTableObj::addModifyListener( const uno::Reference<util::XModifyListener>& aListener )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<util::XModifyListener> *pObj = new uno::Reference<util::XModifyListener>( aListener );
    aModifyListeners.push_back( pObj );

    if ( aModifyListeners.size() == 1 )
    {
        acquire();  
    }
}

void SAL_CALL ScDataPilotTableObj::removeModifyListener( const uno::Reference<util::XModifyListener>& aListener )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    acquire();      

    sal_uInt16 nCount = aModifyListeners.size();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = aModifyListeners[n];
        if ( rObj == aListener )
        {
            aModifyListeners.erase( aModifyListeners.begin() + n );

            if ( aModifyListeners.empty() )
            {
                release();      
            }

            break;
        }
    }

    release();      
}

void ScDataPilotTableObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA(ScDataPilotModifiedHint) &&
         static_cast<const ScDataPilotModifiedHint&>(rHint).GetName() == aName )
    {
        Refreshed_Impl();
    }
    else if ( rHint.ISA( ScUpdateRefHint ) )
    {
        ScRange aRange( 0, 0, nTab );
        ScRangeList aRanges;
        aRanges.Append( aRange );
        const ScUpdateRefHint& rRef = static_cast< const ScUpdateRefHint& >( rHint );
        if ( aRanges.UpdateReference( rRef.GetMode(), GetDocShell()->GetDocument(), rRef.GetRange(),
                 rRef.GetDx(), rRef.GetDy(), rRef.GetDz() ) &&
             aRanges.size() == 1 )
        {
            const ScRange* pRange = aRanges.front();
            if ( pRange )
            {
                nTab = pRange->aStart.Tab();
            }
        }
    }

    ScDataPilotDescriptorBase::Notify( rBC, rHint );
}

void ScDataPilotTableObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source.set((cppu::OWeakObject*)this);

    

    ScDocument* pDoc = GetDocShell()->GetDocument();
    for ( sal_uInt16 n=0; n<aModifyListeners.size(); n++ )
        pDoc->AddUnoListenerCall( aModifyListeners[n], aEvent );
}



ScDataPilotDescriptor::ScDataPilotDescriptor(ScDocShell* pDocSh) :
    ScDataPilotDescriptorBase( pDocSh ),
    mpDPObject(new ScDPObject(pDocSh ? pDocSh->GetDocument() : NULL) )
{
    ScDPSaveData aSaveData;
    
     aSaveData.SetColumnGrand( true );
    aSaveData.SetRowGrand( true );
    aSaveData.SetIgnoreEmptyRows( false );
    aSaveData.SetRepeatIfEmpty( false );
    mpDPObject->SetSaveData(aSaveData);
    ScSheetSourceDesc aSheetDesc(pDocSh ? pDocSh->GetDocument() : NULL);
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
        OSL_FAIL("replace DPObject should not happen");
    }
}



OUString SAL_CALL ScDataPilotDescriptor::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpDPObject->GetName();
}

void SAL_CALL ScDataPilotDescriptor::setName( const OUString& aNewName )
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    mpDPObject->SetName( aNewName );
}

OUString SAL_CALL ScDataPilotDescriptor::getTag() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpDPObject->GetTag();
}

void SAL_CALL ScDataPilotDescriptor::setTag( const OUString& aNewTag )
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    mpDPObject->SetTag( aNewTag );
}



ScDataPilotChildObjBase::ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent ) :
    mrParent( rParent )
{
    mrParent.acquire();
}

ScDataPilotChildObjBase::ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId ) :
    mrParent( rParent ),
    maFieldId( rFieldId )
{
    mrParent.acquire();
}

ScDataPilotChildObjBase::~ScDataPilotChildObjBase()
{
    mrParent.release();
}

ScDPObject* ScDataPilotChildObjBase::GetDPObject() const
{
    return mrParent.GetDPObject();
}

void ScDataPilotChildObjBase::SetDPObject( ScDPObject* pDPObject )
{
    mrParent.SetDPObject( pDPObject );
}

ScDPSaveDimension* ScDataPilotChildObjBase::GetDPDimension( ScDPObject** ppDPObject ) const
{
    if( ScDPObject* pDPObj = GetDPObject() )
    {
        if( ppDPObject ) *ppDPObject = pDPObj;
        if( ScDPSaveData* pSaveData = pDPObj->GetSaveData() )
        {
            if( maFieldId.mbDataLayout )
                return pSaveData->GetDataLayoutDimension();

            if( maFieldId.mnFieldIdx == 0 )
                return pSaveData->GetDimensionByName( maFieldId.maFieldName );

            
            const boost::ptr_vector<ScDPSaveDimension>& rDimensions = pSaveData->GetDimensions();

            sal_Int32 nFoundIdx = 0;
            boost::ptr_vector<ScDPSaveDimension>::const_iterator it;
            for(it = rDimensions.begin(); it != rDimensions.end(); ++it)
            {
                if( !it->IsDataLayout() && (it->GetName() == maFieldId.maFieldName) )
                {
                    if( nFoundIdx == maFieldId.mnFieldIdx )
                        return const_cast<ScDPSaveDimension*>(&(*it));
                    ++nFoundIdx;
                }
            }
        }
    }
    return 0;
}

sal_Int32 ScDataPilotChildObjBase::GetMemberCount() const
{
    sal_Int32 nRet = 0;
    Reference<XNameAccess> xMembersNA = GetMembers();
    if (xMembersNA.is())
    {
        Reference< XIndexAccess > xMembersIA( new ScNameToIndexAccess( xMembersNA ) );
        nRet = xMembersIA->getCount();
    }
    return nRet;
}

Reference< XNameAccess > ScDataPilotChildObjBase::GetMembers() const
{
    Reference< XNameAccess > xMembersNA;
    if( ScDPObject* pDPObj = GetDPObject() )
        pDPObj->GetMembersNA( lcl_GetObjectIndex( pDPObj, maFieldId ), xMembersNA );
    return xMembersNA;
}

ScDocShell* ScDataPilotChildObjBase::GetDocShell() const
{
    return mrParent.GetDocShell();
}



ScDataPilotFieldsObj::ScDataPilotFieldsObj( ScDataPilotDescriptorBase& rParent ) :
    ScDataPilotChildObjBase( rParent )
{
}

ScDataPilotFieldsObj::ScDataPilotFieldsObj( ScDataPilotDescriptorBase& rParent, DataPilotFieldOrientation eOrient ) :
    ScDataPilotChildObjBase( rParent ),
    maOrient( eOrient )
{
}

ScDataPilotFieldsObj::~ScDataPilotFieldsObj()
{
}

static sal_Int32 lcl_GetFieldCount( const Reference<XDimensionsSupplier>& rSource, const Any& rOrient )
{
    if (!rSource.is())
        throw RuntimeException();

    sal_Int32 nRet = 0;

    Reference<XNameAccess> xDimsName(rSource->getDimensions());
    Reference<XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (rOrient.hasValue())
    {
        
        Reference<XPropertySet> xDim;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), UNO_QUERY);
            if (xDim.is() && (xDim->getPropertyValue(OUString(SC_UNO_DP_ORIENTATION)) == rOrient))
                ++nRet;
        }
    }
    else
    {
        

        Reference<XPropertySet> xDim;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), UNO_QUERY);
            if ( xDim.is() && !lcl_IsDuplicated( xDim ) )
                ++nRet;
        }
    }

    return nRet;
}

static sal_Bool lcl_GetFieldDataByIndex( const Reference<XDimensionsSupplier>& rSource,
                                const Any& rOrient, SCSIZE nIndex, ScFieldIdentifier& rFieldId )
{
    if (!rSource.is())
        throw RuntimeException();

    sal_Bool bOk = false;
    SCSIZE nPos = 0;
    sal_Int32 nDimIndex = 0;

    Reference<XNameAccess> xDimsName(rSource->getDimensions());
    Reference<XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    Reference<XPropertySet> xDim;
    if (rOrient.hasValue())
    {
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), UNO_QUERY);
            if (xDim.is() && (xDim->getPropertyValue(OUString(SC_UNO_DP_ORIENTATION)) == rOrient))
            {
                if (nPos == nIndex)
                {
                    bOk = sal_True;
                    nDimIndex = i;
                }
                else
                    ++nPos;
            }
            ++i;
        }
    }
    else
    {
        sal_Int32 i = 0;
        while (i < nIntCount && !bOk)
        {
            xDim.set(xIntDims->getByIndex(i), UNO_QUERY);
            if ( xDim.is() && !lcl_IsDuplicated( xDim ) )
            {
                if (nPos == nIndex)
                {
                    bOk = sal_True;
                    nDimIndex = i;
                }
                else
                    ++nPos;
            }
            ++i;
        }
    }

    if ( bOk )
    {
        xDim.set( xIntDims->getByIndex(nDimIndex), UNO_QUERY );
        Reference<XNamed> xDimName( xDim, UNO_QUERY );
        if ( xDimName.is() )
        {
            OUString sOriginalName( lcl_GetOriginalName( xDimName ) );
            rFieldId.maFieldName = sOriginalName;
            rFieldId.mbDataLayout = ScUnoHelpFunctions::GetBoolProperty( xDim,
                        OUString(SC_UNO_DP_ISDATALAYOUT) );

            sal_Int32 nRepeat = 0;
            if ( rOrient.hasValue() && lcl_IsDuplicated( xDim ) )
            {
                
                

                Reference<XNamed> xPrevName;
                for (sal_Int32 i = 0; i < nDimIndex; ++i)
                {
                    xPrevName.set( xIntDims->getByIndex(i), UNO_QUERY );
                    if ( xPrevName.is() && lcl_GetOriginalName( xPrevName ) == sOriginalName )
                        ++nRepeat;
                }
            }
            rFieldId.mnFieldIdx = nRepeat;
        }
        else
            bOk = false;
    }

    return bOk;
}

static sal_Bool lcl_GetFieldDataByName( ScDPObject* pDPObj, const OUString& rFieldName, ScFieldIdentifier& rFieldId )
{
    
    
    rFieldId.maFieldName = rFieldName;
    rFieldId.mnFieldIdx = 0;
    rFieldId.mbDataLayout = rFieldName == SC_DATALAYOUT_NAME;

    pDPObj->GetSource();    

    
    return rFieldId.mbDataLayout || pDPObj->IsDimNameInUse( rFieldName );
}



ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{

    if (ScDPObject* pObj = GetDPObject())
    {
        ScFieldIdentifier aFieldId;
        if (lcl_GetFieldDataByIndex( pObj->GetSource(), maOrient, nIndex, aFieldId ))
            return new ScDataPilotFieldObj( mrParent, aFieldId, maOrient );
    }
    return 0;
}

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByName_Impl(const OUString& aName) const
{
    if (ScDPObject* pDPObj = GetDPObject())
    {
        ScFieldIdentifier aFieldId;
        if (lcl_GetFieldDataByName( pDPObj, aName, aFieldId ))
            return new ScDataPilotFieldObj( mrParent, aFieldId, maOrient );
    }
    return 0;
}



Reference<XEnumeration> SAL_CALL ScDataPilotFieldsObj::createEnumeration()
                                                    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.DataPilotFieldsEnumeration"));
}



sal_Int32 SAL_CALL ScDataPilotFieldsObj::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    ScDPObject* pDPObj = GetDPObject();
    return pDPObj ? lcl_GetFieldCount( pDPObj->GetSource(), maOrient ) : 0;
}

Any SAL_CALL ScDataPilotFieldsObj::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference< XPropertySet > xField( GetObjectByIndex_Impl( nIndex ) );
    if (!xField.is())
        throw IndexOutOfBoundsException();
    return Any( xField );
}

uno::Type SAL_CALL ScDataPilotFieldsObj::getElementType() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((Reference<XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

Any SAL_CALL ScDataPilotFieldsObj::getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xField(GetObjectByName_Impl(aName));
    if (!xField.is())
        throw NoSuchElementException();
    return Any( xField );
}

Sequence<OUString> SAL_CALL ScDataPilotFieldsObj::getElementNames()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if (ScDPObject* pDPObj = GetDPObject())
    {
        Sequence< OUString > aSeq( lcl_GetFieldCount( pDPObj->GetSource(), maOrient ) );
        OUString* pAry = aSeq.getArray();

        const boost::ptr_vector<ScDPSaveDimension>& rDimensions = pDPObj->GetSaveData()->GetDimensions();
        boost::ptr_vector<ScDPSaveDimension>::const_iterator it;
        for (it = rDimensions.begin(); it != rDimensions.end(); ++it)
        {
            if(maOrient.hasValue() && (it->GetOrientation() == maOrient.get< DataPilotFieldOrientation >()))
            {
                *pAry = it->GetName();
                ++pAry;
            }
        }
        return aSeq;
    }
    return Sequence<OUString>();
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasByName( const OUString& aName )
                                        throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    return GetObjectByName_Impl(aName) != NULL;
}



ScDataPilotFieldObj::ScDataPilotFieldObj(
        ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId ) :
    ScDataPilotChildObjBase( rParent, rFieldId ),
    maPropSet( lcl_GetDataPilotFieldMap() )
{
}

ScDataPilotFieldObj::ScDataPilotFieldObj( ScDataPilotDescriptorBase& rParent,
        const ScFieldIdentifier& rFieldId, const Any& rOrient ) :
    ScDataPilotChildObjBase( rParent, rFieldId ),
    maPropSet( lcl_GetDataPilotFieldMap() ),
    maOrient( rOrient )
{
}

ScDataPilotFieldObj::~ScDataPilotFieldObj()
{
}



OUString SAL_CALL ScDataPilotFieldObj::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aName;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->IsDataLayout() )
            aName = OUString( SC_DATALAYOUT_NAME );
        else
        {
            const OUString* pLayoutName = pDim->GetLayoutName();
            if (pLayoutName)
                aName = *pLayoutName;
            else
                aName = pDim->GetName();
        }                                                                }
    return aName;
}

void SAL_CALL ScDataPilotFieldObj::setName( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    ScDPSaveDimension* pDim = GetDPDimension( &pDPObj );
    if( pDim && !pDim->IsDataLayout() )
    {
        pDim->SetLayoutName(rName);
        SetDPObject( pDPObj );
    }
}



Reference<XPropertySetInfo> SAL_CALL ScDataPilotFieldObj::getPropertySetInfo()
                                                        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDataPilotFieldObj::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    if ( aNameString.equalsAscii( SC_UNONAME_FUNCTION ) )
    {
        
        GeneralFunction eFunction = (GeneralFunction)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setFunction( eFunction );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_SUBTOTALS ) )
    {
        Sequence< GeneralFunction > aSubtotals;
        if( aValue >>= aSubtotals )
            setSubtotals( aSubtotals );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_ORIENT ) )
    {
        
        DataPilotFieldOrientation eOrient = (DataPilotFieldOrientation)
                            ScUnoHelpFunctions::GetEnumFromAny( aValue );
        setOrientation( eOrient );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_SELPAGE ) )
    {
        OUString sCurrentPage;
        if (aValue >>= sCurrentPage)
            setCurrentPage(sCurrentPage);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_USESELPAGE ) )
    {
        setUseCurrentPage(cppu::any2bool(aValue));
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASAUTOSHOW ) )
    {
        if (!cppu::any2bool(aValue))
            setAutoShowInfo(NULL);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_AUTOSHOW ) )
    {
        DataPilotFieldAutoShowInfo aInfo;
        if (aValue >>= aInfo)
            setAutoShowInfo(&aInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASLAYOUTINFO ) )
    {
        if (!cppu::any2bool(aValue))
            setLayoutInfo(NULL);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_LAYOUTINFO ) )
    {
        DataPilotFieldLayoutInfo aInfo;
        if (aValue >>= aInfo)
            setLayoutInfo(&aInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASREFERENCE ) )
    {
        if (!cppu::any2bool(aValue))
            setReference(NULL);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_REFERENCE ) )
    {
        DataPilotFieldReference aRef;
        if (aValue >>= aRef)
            setReference(&aRef);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASSORTINFO ) )
    {
        if (!cppu::any2bool(aValue))
            setSortInfo(NULL);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_SORTINFO ) )
    {
        DataPilotFieldSortInfo aInfo;
        if (aValue >>= aInfo)
            setSortInfo(&aInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_ISGROUP ) )
    {
        if (!cppu::any2bool(aValue))
            setGroupInfo(NULL);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_GROUPINFO ) )
    {
        DataPilotFieldGroupInfo aInfo;
        if (aValue >>= aInfo)
            setGroupInfo(&aInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_SHOWEMPTY ) )
    {
        setShowEmpty(cppu::any2bool(aValue));
    }
}

Any SAL_CALL ScDataPilotFieldObj::getPropertyValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    Any aRet;

    if ( aNameString.equalsAscii( SC_UNONAME_FUNCTION ) )
        aRet <<= getFunction();
    else if ( aNameString.equalsAscii( SC_UNONAME_SUBTOTALS ) )
        aRet <<= getSubtotals();
    else if ( aNameString.equalsAscii( SC_UNONAME_ORIENT ) )
        aRet <<= getOrientation();
    else if ( aNameString.equalsAscii( SC_UNONAME_SELPAGE ) )
        aRet <<= getCurrentPage();
    else if ( aNameString.equalsAscii( SC_UNONAME_USESELPAGE ) )
        aRet <<= getUseCurrentPage();
    else if ( aNameString.equalsAscii( SC_UNONAME_HASAUTOSHOW ) )
        aRet = ::cppu::bool2any(getAutoShowInfo() != NULL);
    else if ( aNameString.equalsAscii( SC_UNONAME_AUTOSHOW ) )
    {
        const DataPilotFieldAutoShowInfo* pInfo = getAutoShowInfo();
        if (pInfo)
            aRet <<= DataPilotFieldAutoShowInfo(*pInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASLAYOUTINFO ) )
        aRet = ::cppu::bool2any(getLayoutInfo() != NULL);
    else if ( aNameString.equalsAscii( SC_UNONAME_LAYOUTINFO ) )
    {
        const DataPilotFieldLayoutInfo* pInfo = getLayoutInfo();
        if (pInfo)
            aRet <<= DataPilotFieldLayoutInfo(*pInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASREFERENCE ) )
        aRet = ::cppu::bool2any(getReference() != NULL);
    else if ( aNameString.equalsAscii( SC_UNONAME_REFERENCE ) )
    {
        const DataPilotFieldReference* pRef = getReference();
        if (pRef)
            aRet <<= DataPilotFieldReference(*pRef);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_HASSORTINFO ) )
        aRet = ::cppu::bool2any(getSortInfo() != NULL);
    else if ( aNameString.equalsAscii( SC_UNONAME_SORTINFO ) )
    {
        const DataPilotFieldSortInfo* pInfo = getSortInfo();
        if (pInfo)
            aRet <<= DataPilotFieldSortInfo(*pInfo);
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_ISGROUP ) )
        aRet = ::cppu::bool2any(hasGroupInfo());
    else if ( aNameString.equalsAscii( SC_UNONAME_GROUPINFO ) )
    {
        aRet <<= getGroupInfo();
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_SHOWEMPTY ) )
        aRet <<= getShowEmpty();

    return aRet;
}



Reference<XIndexAccess> SAL_CALL ScDataPilotFieldObj::getItems()
                throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mxItems.is())
        mxItems.set( new ScDataPilotItemsObj( mrParent, maFieldId ) );
    return mxItems;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDataPilotFieldObj )

DataPilotFieldOrientation ScDataPilotFieldObj::getOrientation() const
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? static_cast< DataPilotFieldOrientation >( pDim->GetOrientation() ) : DataPilotFieldOrientation_HIDDEN;
}

void ScDataPilotFieldObj::setOrientation(DataPilotFieldOrientation eNew)
{
    SolarMutexGuard aGuard;
    if (maOrient.hasValue() && (eNew == maOrient.get< DataPilotFieldOrientation >()))
        return;

    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        ScDPSaveData* pSaveData = pDPObj->GetSaveData();

        /*  If the field was taken from getDataPilotFields(), don't reset the
            orientation for an existing use, but create a duplicated field
            instead (for "Data" orientation only). */
        if ( !maOrient.hasValue() && !maFieldId.mbDataLayout &&
             (pDim->GetOrientation() != DataPilotFieldOrientation_HIDDEN) &&
             (eNew == DataPilotFieldOrientation_DATA) )
        {

            ScDPSaveDimension* pNewDim = 0;

            

            sal_Int32 nFound = 0;
            const boost::ptr_vector<ScDPSaveDimension>& rDimensions = pSaveData->GetDimensions();
            boost::ptr_vector<ScDPSaveDimension>::const_iterator it;
            for ( it = rDimensions.begin(); it != rDimensions.end() && !pNewDim; ++it )
            {
                if ( !it->IsDataLayout() && (it->GetName() == maFieldId.maFieldName) )
                {
                    if ( it->GetOrientation() == DataPilotFieldOrientation_HIDDEN )
                        pNewDim = const_cast<ScDPSaveDimension*>(&(*it));      
                    else
                        ++nFound;               
                }
            }

            if ( !pNewDim )     
                pNewDim = &pSaveData->DuplicateDimension( *pDim );

            maFieldId.mnFieldIdx = nFound;      
            pDim = pNewDim;
        }

        pDim->SetOrientation(sal::static_int_cast<sal_uInt16>(eNew));

        
        pSaveData->SetPosition( pDim, pSaveData->GetDimensions().size() );

        SetDPObject( pDPObj );

        maOrient <<= eNew;   
    }
}

GeneralFunction ScDataPilotFieldObj::getFunction() const
{
    SolarMutexGuard aGuard;
    GeneralFunction eRet = GeneralFunction_NONE;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            
            long nSubCount = pDim->GetSubTotalsCount();
            if ( nSubCount > 0 )
                eRet = (GeneralFunction)pDim->GetSubTotalFunc(0);    
            
        }
        else
            eRet = (GeneralFunction)pDim->GetFunction();
    }
    return eRet;
}

void ScDataPilotFieldObj::setFunction(GeneralFunction eNewFunc)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            
            if ( eNewFunc == GeneralFunction_NONE )
                pDim->SetSubTotals( 0, NULL );
            else
            {
                sal_uInt16 nFunc = sal::static_int_cast<sal_uInt16>( eNewFunc );
                pDim->SetSubTotals( 1, &nFunc );
            }
        }
        else
            pDim->SetFunction( sal::static_int_cast<sal_uInt16>( eNewFunc ) );
        SetDPObject( pDPObj );
    }
}

Sequence< GeneralFunction > ScDataPilotFieldObj::getSubtotals() const
{
    SolarMutexGuard aGuard;
    Sequence< GeneralFunction > aRet;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            
            sal_Int32 nCount = static_cast< sal_Int32 >( pDim->GetSubTotalsCount() );
            if ( nCount > 0 )
            {
                aRet.realloc( nCount );
                for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
                    aRet[ nIdx ] = (GeneralFunction)pDim->GetSubTotalFunc( nIdx );
            }
        }
    }
    return aRet;
}

void ScDataPilotFieldObj::setSubtotals( const Sequence< GeneralFunction >& rSubtotals )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            sal_Int32 nCount = rSubtotals.getLength();
            if( nCount == 1 )
            {
                
                if( rSubtotals[ 0 ] == GeneralFunction_NONE )
                    pDim->SetSubTotals( 0, NULL );
                else
                {
                    sal_uInt16 nFunc = sal::static_int_cast<sal_uInt16>( rSubtotals[ 0 ] );
                    pDim->SetSubTotals( 1, &nFunc );
                }
            }
            else if( nCount > 1 )
            {
                
                ::std::vector< sal_uInt16 > aSubt;
                for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
                {
                    GeneralFunction eFunc = rSubtotals[ nIdx ];
                    if( (eFunc != GeneralFunction_NONE) && (eFunc != GeneralFunction_AUTO) )
                    {
                        
                        sal_uInt16 nFunc = static_cast< sal_uInt16 >( eFunc );
                        if( ::std::find( aSubt.begin(), aSubt.end(), nFunc ) == aSubt.end() )
                            aSubt.push_back( nFunc );
                    }
                }
                
                if ( aSubt.empty() )
                    pDim->SetSubTotals( 0, NULL );
                else
                    pDim->SetSubTotals( static_cast< long >( aSubt.size() ), &aSubt.front() );
            }
        }
        SetDPObject( pDPObj );
    }
}

OUString ScDataPilotFieldObj::getCurrentPage() const
{
    return OUString();
}

void ScDataPilotFieldObj::setCurrentPage( const OUString& rPage )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetCurrentPage( &rPage );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::getUseCurrentPage() const
{
    return false;
}

void ScDataPilotFieldObj::setUseCurrentPage( bool bUse )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( bUse )
        {
            /*  It is somehow useless to set the property "HasSelectedPage" to
                true, because it is still needed to set an explicit page name. */
            const OUString aPage;
            pDim->SetCurrentPage( &aPage );
        }
        else
            pDim->SetCurrentPage( 0 );
        SetDPObject( pDPObj );
    }
}

const DataPilotFieldAutoShowInfo* ScDataPilotFieldObj::getAutoShowInfo()
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetAutoShowInfo() : 0;
}

void ScDataPilotFieldObj::setAutoShowInfo( const DataPilotFieldAutoShowInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetAutoShowInfo( pInfo );
        SetDPObject( pDPObj );
    }
}

const DataPilotFieldLayoutInfo* ScDataPilotFieldObj::getLayoutInfo()
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetLayoutInfo() : 0;
}

void ScDataPilotFieldObj::setLayoutInfo( const DataPilotFieldLayoutInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetLayoutInfo( pInfo );
        SetDPObject( pDPObj );
    }
}

const DataPilotFieldReference* ScDataPilotFieldObj::getReference()
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetReferenceValue() : 0;
}

void ScDataPilotFieldObj::setReference( const DataPilotFieldReference* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetReferenceValue( pInfo );
        SetDPObject( pDPObj );
    }
}

const DataPilotFieldSortInfo* ScDataPilotFieldObj::getSortInfo()
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetSortInfo() : 0;
}

void ScDataPilotFieldObj::setSortInfo( const DataPilotFieldSortInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetSortInfo( pInfo );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::getShowEmpty() const
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim && pDim->GetShowEmpty();
}

void ScDataPilotFieldObj::setShowEmpty( bool bShow )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetShowEmpty( bShow );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::hasGroupInfo()
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
        if( const ScDPDimensionSaveData* pDimData = pDPObj->GetSaveData()->GetExistingDimensionData() )
            return pDimData->GetNamedGroupDim( pDim->GetName() ) || pDimData->GetNumGroupDim( pDim->GetName() );
    return false;
}

DataPilotFieldGroupInfo ScDataPilotFieldObj::getGroupInfo()
{
    SolarMutexGuard aGuard;
    DataPilotFieldGroupInfo aInfo;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( const ScDPDimensionSaveData* pDimData = pDPObj->GetSaveData()->GetExistingDimensionData() )
        {
            if( const ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDim( pDim->GetName() ) )
            {
                
                aInfo.GroupBy = pGroupDim->GetDatePart();

                
                try
                {
                    Reference< XNameAccess > xFields( mrParent.getDataPilotFields(), UNO_QUERY_THROW );
                    aInfo.SourceField.set( xFields->getByName( pGroupDim->GetSourceDimName() ), UNO_QUERY );
                }
                catch( Exception& )
                {
                }

                ScDataPilotConversion::FillGroupInfo( aInfo, pGroupDim->GetDateInfo() );
                if( pGroupDim->GetDatePart() == 0 )
                {
                    
                    ScFieldGroups aGroups;
                    for( sal_Int32 nIdx = 0, nCount = pGroupDim->GetGroupCount(); nIdx < nCount; ++nIdx )
                    {
                        if( const ScDPSaveGroupItem* pGroup = pGroupDim->GetGroupByIndex( nIdx ) )
                        {
                            ScFieldGroup aGroup;
                            aGroup.maName = pGroup->GetGroupName();
                            for( sal_Int32 nMemIdx = 0, nMemCount = pGroup->GetElementCount(); nMemIdx < nMemCount; ++nMemIdx )
                                if (const OUString* pMem = pGroup->GetElementByIndex(nMemIdx))
                                    aGroup.maMembers.push_back( *pMem );
                            aGroups.push_back( aGroup );
                        }
                    }
                    aInfo.Groups = new ScDataPilotFieldGroupsObj( aGroups );
                }
            }
            else if( const ScDPSaveNumGroupDimension* pNumGroupDim = pDimData->GetNumGroupDim( pDim->GetName() ) )
            {
                if (pNumGroupDim->GetDatePart())
                {
                    ScDataPilotConversion::FillGroupInfo( aInfo, pNumGroupDim->GetDateInfo() );
                    aInfo.GroupBy = pNumGroupDim->GetDatePart();
                }
                else
                {
                    ScDataPilotConversion::FillGroupInfo( aInfo, pNumGroupDim->GetInfo() );
                }
            }
        }
    }
    return aInfo;
}

void ScDataPilotFieldObj::setGroupInfo( const DataPilotFieldGroupInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( /*ScDPSaveDimension* pDim =*/ GetDPDimension( &pDPObj ) )
    {
        ScDPSaveData* pSaveData = pDPObj->GetSaveData();
        if( pInfo && lclCheckMinMaxStep( *pInfo ) )
        {
            ScDPNumGroupInfo aInfo;
            aInfo.mbEnable = true;
            aInfo.mbDateValues = pInfo->HasDateValues;
            aInfo.mbAutoStart = pInfo->HasAutoStart;
            aInfo.mbAutoEnd = pInfo->HasAutoEnd;
            aInfo.mfStart = pInfo->Start;
            aInfo.mfEnd = pInfo->End;
            aInfo.mfStep = pInfo->Step;
            Reference< XNamed > xNamed( pInfo->SourceField, UNO_QUERY );
            if( xNamed.is() )
            {
                ScDPSaveGroupDimension aGroupDim( xNamed->getName(), getName() );
                if( pInfo->GroupBy )
                    aGroupDim.SetDateInfo(aInfo, pInfo->GroupBy);
                else
                {
                    Reference<XIndexAccess> xIndex(pInfo->Groups, UNO_QUERY);
                    if (xIndex.is())
                    {
                        sal_Int32 nCount(xIndex->getCount());
                        for(sal_Int32 i = 0; i < nCount; i++)
                        {
                            Reference<XNamed> xGroupNamed(xIndex->getByIndex(i), UNO_QUERY);
                            if (xGroupNamed.is())
                            {
                                ScDPSaveGroupItem aItem(xGroupNamed->getName());
                                Reference<XIndexAccess> xGroupIndex(xGroupNamed, UNO_QUERY);
                                if (xGroupIndex.is())
                                {
                                    sal_Int32 nItemCount(xGroupIndex->getCount());
                                    for (sal_Int32 j = 0; j < nItemCount; ++j)
                                    {
                                        Reference<XNamed> xItemNamed(xGroupIndex->getByIndex(j), UNO_QUERY);
                                        if (xItemNamed.is())
                                            aItem.AddElement(xItemNamed->getName());
                                    }
                                }
                                aGroupDim.AddGroupItem(aItem);
                            }
                        }
                    }
                }

                
                ScDPDimensionSaveData& rDimSaveData = *pSaveData->GetDimensionData();
                rDimSaveData.ReplaceGroupDimension( aGroupDim );
            }
            else    
            {
                ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();     

                ScDPSaveNumGroupDimension* pExisting = pDimData->GetNumGroupDimAcc( getName() );
                if ( pExisting )
                {
                    if (pInfo->GroupBy)
                        pExisting->SetDateInfo(aInfo, pInfo->GroupBy);
                    
                    pExisting->SetGroupInfo( aInfo );
                }
                else if (pInfo->GroupBy)
                {
                    
                    ScDPSaveNumGroupDimension aNumGroupDim( getName(), aInfo, pInfo->GroupBy );
                    pDimData->AddNumGroupDimension( aNumGroupDim );
                }
                else
                {
                    
                    ScDPSaveNumGroupDimension aNumGroupDim( getName(), aInfo );
                    pDimData->AddNumGroupDimension( aNumGroupDim );
                }
            }
        }
        else    
        {
            pSaveData->SetDimensionData( 0 );
        }

        pDPObj->SetSaveData( *pSaveData );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::HasString(const Sequence< OUString >& rItems, const OUString& aString)
{
    bool bRet = false;

    sal_Int32 nCount(rItems.getLength());
    sal_Int32 nItem(0);
    while (nItem < nCount && !bRet)
    {
        bRet = rItems[nItem] == aString;
        ++nItem;
    }

    return bRet;
}


Reference< XDataPilotField > SAL_CALL ScDataPilotFieldObj::createNameGroup( const Sequence< OUString >& rItems )
         throw (RuntimeException, IllegalArgumentException)
{
    SolarMutexGuard aGuard;

    Reference< XDataPilotField > xRet;
    OUString sNewDim;

    if( !rItems.hasElements() )
        throw IllegalArgumentException();

    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        OUString aDimName = pDim->GetName();

        ScDPSaveData aSaveData = *pDPObj->GetSaveData();
        ScDPDimensionSaveData* pDimData = aSaveData.GetDimensionData();     

        
        OUString aBaseDimName( aDimName );
        const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName );
        if ( pBaseGroupDim )
        {
            
            aBaseDimName = pBaseGroupDim->GetSourceDimName();
        }

        
        
        ScDPSaveGroupDimension* pGroupDimension = pDimData->GetGroupDimAccForBase( aDimName );

        
        
        sal_Int32 nEntryCount = rItems.getLength();
        sal_Int32 nEntry;
        if ( pGroupDimension )
        {
            for (nEntry=0; nEntry<nEntryCount; nEntry++)
            {
                const OUString& aEntryName = rItems[nEntry];
                if ( pBaseGroupDim )
                {
                    
                    
                    const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                    if ( pBaseGroup )
                        pBaseGroup->RemoveElementsFromGroups( *pGroupDimension );   
                    else
                        pGroupDimension->RemoveFromGroups( aEntryName );
                }
                else
                    pGroupDimension->RemoveFromGroups( aEntryName );
            }
        }

        ScDPSaveGroupDimension* pNewGroupDim = 0;
        if ( !pGroupDimension )
        {
            
            sNewDim = pDimData->CreateGroupDimName( aBaseDimName, *pDPObj, false, NULL );
            pNewGroupDim = new ScDPSaveGroupDimension( aBaseDimName, sNewDim );

            pGroupDimension = pNewGroupDim;     

            if ( pBaseGroupDim )
            {
                
                
                
                
                
                

                long nGroupCount = pBaseGroupDim->GetGroupCount();
                for ( long nGroup = 0; nGroup < nGroupCount; nGroup++ )
                {
                    const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetGroupByIndex( nGroup );

                    if (!HasString(rItems, pBaseGroup->GetGroupName()))    
                    {
                        
                        ScDPSaveGroupItem aGroup( pBaseGroup->GetGroupName() );
                        aGroup.AddElementsFromGroup( *pBaseGroup );
                        pGroupDimension->AddGroupItem( aGroup );
                    }
                }
            }
        }
        OUString aGroupDimName = pGroupDimension->GetGroupDimName();

        
        OUString aGroupName = pGroupDimension->CreateGroupName( OUString( "Group" ) );
        ScDPSaveGroupItem aGroup( aGroupName );
        Reference< XNameAccess > xMembers = GetMembers();
        if (!xMembers.is())
        {
            delete pNewGroupDim;
            throw RuntimeException();
        }

        for (nEntry=0; nEntry<nEntryCount; nEntry++)
        {
            OUString aEntryName(rItems[nEntry]);

            if (!xMembers->hasByName(aEntryName))
            {
                delete pNewGroupDim;
                throw IllegalArgumentException();
            }

            if ( pBaseGroupDim )
            {
                
                const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                if ( pBaseGroup )
                    aGroup.AddElementsFromGroup( *pBaseGroup );
                else
                    aGroup.AddElement( aEntryName );    
            }
            else
                aGroup.AddElement( aEntryName );        
        }

        pGroupDimension->AddGroupItem( aGroup );

        if ( pNewGroupDim )
        {
            pDimData->AddGroupDimension( *pNewGroupDim );
            delete pNewGroupDim;        
            
        }
        pGroupDimension = pNewGroupDim = NULL;

        
        ScDPSaveDimension* pSaveDimension = aSaveData.GetDimensionByName( aGroupDimName );
        if ( pSaveDimension->GetOrientation() == DataPilotFieldOrientation_HIDDEN )
        {
            ScDPSaveDimension* pOldDimension = aSaveData.GetDimensionByName( aDimName );
            pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
            long nPosition = 0;     
            aSaveData.SetPosition( pSaveDimension, nPosition );
        }

        
        pDPObj->SetSaveData( aSaveData );
        ScDBDocFunc(*GetDocShell()).RefreshPivotTableGroups(pDPObj);
    }

    
    if( !sNewDim.isEmpty() )
    {
        Reference< XNameAccess > xFields(mrParent.getDataPilotFields(), UNO_QUERY);
        if (xFields.is())
        {
            try
            {
                xRet.set(xFields->getByName(sNewDim), UNO_QUERY);
                OSL_ENSURE(xRet.is(), "there is a name, so there should be also a field");
            }
            catch (const container::NoSuchElementException&)
            {
                
                throw RuntimeException();
            }
        }
    }
    return xRet;
}

Reference < XDataPilotField > SAL_CALL ScDataPilotFieldObj::createDateGroup( const DataPilotFieldGroupInfo& rInfo )
        throw (RuntimeException, IllegalArgumentException)
{
    SolarMutexGuard aGuard;
    using namespace ::com::sun::star::sheet::DataPilotFieldGroupBy;

    
    if( !rInfo.HasDateValues || !lclCheckMinMaxStep( rInfo ) )
        throw IllegalArgumentException();
    
    if( (rInfo.GroupBy == 0) || (rInfo.GroupBy > YEARS) || ((rInfo.GroupBy & (rInfo.GroupBy - 1)) != 0) )
        throw IllegalArgumentException();
    
    if( rInfo.Step >= ((rInfo.GroupBy == DAYS) ? 32768.0 : 1.0) )
        throw IllegalArgumentException();

    OUString aGroupDimName;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        ScDPNumGroupInfo aInfo;
        aInfo.mbEnable = true;
        aInfo.mbDateValues = (rInfo.GroupBy == DAYS) && (rInfo.Step >= 1.0);
        aInfo.mbAutoStart = rInfo.HasAutoStart;
        aInfo.mbAutoEnd = rInfo.HasAutoEnd;
        aInfo.mfStart = rInfo.Start;
        aInfo.mfEnd = rInfo.End;
        aInfo.mfStep = static_cast< sal_Int32 >( rInfo.Step );

        
        ScDPSaveData aSaveData = *pDPObj->GetSaveData();
        
        ScDPDimensionSaveData& rDimData = *aSaveData.GetDimensionData();

        
        const OUString& rDimName = pDim->GetName();
        const ScDPSaveGroupDimension* pGroupDim = rDimData.GetNamedGroupDim( rDimName );
        OUString aSrcDimName = pGroupDim ? pGroupDim->GetSourceDimName() : rDimName;

        
        pGroupDim = rDimData.GetFirstNamedGroupDim( aSrcDimName );
        const ScDPSaveNumGroupDimension* pNumGroupDim = rDimData.GetNumGroupDim( aSrcDimName );

        
        bool bHasNamedGrouping = pGroupDim && !pGroupDim->GetDateInfo().mbEnable;
        bool bHasNumGrouping = pNumGroupDim && pNumGroupDim->GetInfo().mbEnable && !pNumGroupDim->GetInfo().mbDateValues && !pNumGroupDim->GetDateInfo().mbEnable;
        if( bHasNamedGrouping || bHasNumGrouping )
            throw IllegalArgumentException();

        if( aInfo.mbDateValues )  
        {
            
            while( pGroupDim )
            {
                OUString aGroupDimName2 = pGroupDim->GetGroupDimName();
                
                pGroupDim = rDimData.GetNextNamedGroupDim( aGroupDimName2 );
                
                rDimData.RemoveGroupDimension( aGroupDimName2 );
                
                aSaveData.RemoveDimensionByName( aGroupDimName2 );
            }
            
            ScDPSaveNumGroupDimension aNumGroupDim( aSrcDimName, aInfo );
            rDimData.ReplaceNumGroupDimension( aNumGroupDim );
        }
        else    
        {
            
            sal_Int32 nDateParts = rDimData.CollectDateParts( aSrcDimName );
            if( nDateParts == 0 )
            {
                
                ScDPSaveNumGroupDimension aNumGroupDim( aSrcDimName, aInfo, rInfo.GroupBy );
                rDimData.ReplaceNumGroupDimension( aNumGroupDim );
            }
            else if( (nDateParts & rInfo.GroupBy) == 0 )    
            {
                
                aGroupDimName = rDimData.CreateDateGroupDimName( rInfo.GroupBy, *pDPObj, true, 0 );
                ScDPSaveGroupDimension aGroupDim( aSrcDimName, aGroupDimName, aInfo, rInfo.GroupBy );
                rDimData.AddGroupDimension( aGroupDim );

                
                ScDPSaveDimension& rSaveDim = *aSaveData.GetDimensionByName( aGroupDimName );
                if( rSaveDim.GetOrientation() == DataPilotFieldOrientation_HIDDEN )
                {
                    ScDPSaveDimension& rOldDim = *aSaveData.GetDimensionByName( aSrcDimName );
                    rSaveDim.SetOrientation( rOldDim.GetOrientation() );
                    aSaveData.SetPosition( &rSaveDim, 0 );  
                }
            }
        }

        
        pDPObj->SetSaveData( aSaveData );
        SetDPObject( pDPObj );
    }

    
    Reference< XDataPilotField > xRet;
    if( !aGroupDimName.isEmpty() )
        try
        {
           Reference< XNameAccess > xFields( mrParent.getDataPilotFields(), UNO_QUERY_THROW );
           xRet.set( xFields->getByName( aGroupDimName ), UNO_QUERY );
        }
        catch( Exception& )
        {
        }
    return xRet;
}



namespace {

bool lclExtractGroupMembers( ScFieldGroupMembers& rMembers, const Any& rElement )
{
    
    if( !rElement.hasValue() )
        return true;

    
    Sequence< OUString > aSeq;
    if( rElement >>= aSeq )
    {
        if( aSeq.hasElements() )
            rMembers.insert( rMembers.end(), aSeq.getConstArray(), aSeq.getConstArray() + aSeq.getLength() );
        return true;
    }

    
    Reference< XIndexAccess > xItemsIA( rElement, UNO_QUERY );
    if( xItemsIA.is() )
    {
        for( sal_Int32 nIdx = 0, nCount = xItemsIA->getCount(); nIdx < nCount; ++nIdx )
        {
            try 
            {
                Reference< XNamed > xItemName( xItemsIA->getByIndex( nIdx ), UNO_QUERY_THROW );
                rMembers.push_back( xItemName->getName() );
            }
            catch( Exception& )
            {
                
            }
        }
        return true;
    }

    
    return false;
}

} 



ScDataPilotFieldGroupsObj::ScDataPilotFieldGroupsObj( const ScFieldGroups& rGroups ) :
    maGroups( rGroups )
{
}

ScDataPilotFieldGroupsObj::~ScDataPilotFieldGroupsObj()
{
}



Any SAL_CALL ScDataPilotFieldGroupsObj::getByName( const OUString& rName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if( implFindByName( rName ) == maGroups.end() )
        throw NoSuchElementException();
    return Any( Reference< XNameAccess >( new ScDataPilotFieldGroupObj( *this, rName ) ) );
}

Sequence< OUString > SAL_CALL ScDataPilotFieldGroupsObj::getElementNames() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aSeq;
    if( !maGroups.empty() )
    {
        aSeq.realloc( static_cast< sal_Int32 >( maGroups.size() ) );
        OUString* pName = aSeq.getArray();
        for( ScFieldGroups::iterator aIt = maGroups.begin(), aEnd = maGroups.end(); aIt != aEnd; ++aIt, ++pName )
            *pName = aIt->maName;
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasByName( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return implFindByName( rName ) != maGroups.end();
}



void SAL_CALL ScDataPilotFieldGroupsObj::replaceByName( const OUString& rName, const Any& rElement )
        throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException();

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw NoSuchElementException();

    
    ScFieldGroupMembers aMembers;
    if( !lclExtractGroupMembers( aMembers, rElement ) )
        throw IllegalArgumentException();

    
    aIt->maMembers.swap( aMembers );
}



void SAL_CALL ScDataPilotFieldGroupsObj::insertByName( const OUString& rName, const Any& rElement )
        throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException();

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt != maGroups.end() )
        throw ElementExistException();

    
    ScFieldGroupMembers aMembers;
    if( !lclExtractGroupMembers( aMembers, rElement ) )
        throw IllegalArgumentException();

    
    maGroups.resize( maGroups.size() + 1 );
    ScFieldGroup& rGroup = maGroups.back();
    rGroup.maName = rName;
    rGroup.maMembers.swap( aMembers );
}

void SAL_CALL ScDataPilotFieldGroupsObj::removeByName( const OUString& rName )
        throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException();

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw NoSuchElementException();

    maGroups.erase( aIt );
}



sal_Int32 SAL_CALL ScDataPilotFieldGroupsObj::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return static_cast< sal_Int32 >( maGroups.size() );
}

Any SAL_CALL ScDataPilotFieldGroupsObj::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if ((nIndex < 0) || (nIndex >= static_cast< sal_Int32 >( maGroups.size() )))
        throw IndexOutOfBoundsException();
    return Any( Reference< XNameAccess >( new ScDataPilotFieldGroupObj( *this, maGroups[ nIndex ].maName ) ) );
}



Reference<XEnumeration> SAL_CALL ScDataPilotFieldGroupsObj::createEnumeration() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration( this, OUString( "com.sun.star.sheet.DataPilotFieldGroupsEnumeration" ) );
}



uno::Type SAL_CALL ScDataPilotFieldGroupsObj::getElementType() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType( (Reference< XNameAccess >*)0 );
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return !maGroups.empty();
}



ScFieldGroup& ScDataPilotFieldGroupsObj::getFieldGroup( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw RuntimeException();
     return *aIt;
}

void ScDataPilotFieldGroupsObj::renameFieldGroup( const OUString& rOldName, const OUString& rNewName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScFieldGroups::iterator aOldIt = implFindByName( rOldName );
    ScFieldGroups::iterator aNewIt = implFindByName( rNewName );
    
    if( (aOldIt == maGroups.end()) || ((aNewIt != maGroups.end()) && (aNewIt != aOldIt)) )
        throw RuntimeException();
    aOldIt->maName = rNewName;
}

ScFieldGroups::iterator ScDataPilotFieldGroupsObj::implFindByName( const OUString& rName )
{
    for( ScFieldGroups::iterator aIt = maGroups.begin(), aEnd = maGroups.end(); aIt != aEnd; ++aIt )
        if( aIt->maName == rName )
            return aIt;
    return maGroups.end();
}



namespace {

OUString lclExtractMember( const Any& rElement )
{
    if( rElement.has< OUString >() )
        return rElement.get< OUString >();

    Reference< XNamed > xNamed( rElement, UNO_QUERY );
    if( xNamed.is() )
        return xNamed->getName();

    return OUString();
}

} 



ScDataPilotFieldGroupObj::ScDataPilotFieldGroupObj( ScDataPilotFieldGroupsObj& rParent, const OUString& rGroupName ) :
    mrParent( rParent ),
    maGroupName( rGroupName )
{
    mrParent.acquire();
}

ScDataPilotFieldGroupObj::~ScDataPilotFieldGroupObj()
{
    mrParent.release();
}



Any SAL_CALL ScDataPilotFieldGroupObj::getByName( const OUString& rName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    if( aIt == rMembers.end() )
        throw NoSuchElementException();
    return Any( Reference< XNamed >( new ScDataPilotFieldGroupItemObj( *this, *aIt ) ) );
}

Sequence< OUString > SAL_CALL ScDataPilotFieldGroupObj::getElementNames() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return ::comphelper::containerToSequence( mrParent.getFieldGroup( maGroupName ).maMembers );
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasByName( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    return ::std::find( rMembers.begin(), rMembers.end(), rName ) != rMembers.end();
}



void SAL_CALL ScDataPilotFieldGroupObj::replaceByName( const OUString& rName, const Any& rElement )
        throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    
    OUString aNewName = lclExtractMember( rElement );
    if( rName.isEmpty() || aNewName.isEmpty() )
        throw IllegalArgumentException();
    if( rName == aNewName )
        return;

    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aOldIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    ScFieldGroupMembers::iterator aNewIt = ::std::find( rMembers.begin(), rMembers.end(), aNewName );
    
    if( aOldIt == rMembers.end() )
        throw NoSuchElementException();
    
    if( aNewIt != rMembers.end() )
        throw IllegalArgumentException();
    *aOldIt = aNewName;
}



void SAL_CALL ScDataPilotFieldGroupObj::insertByName( const OUString& rName, const Any& /*rElement*/ )
        throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    
    if( rName.isEmpty() )
        throw IllegalArgumentException();

    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    
    if( aIt != rMembers.end() )
        throw IllegalArgumentException();
    rMembers.push_back( rName );
}

void SAL_CALL ScDataPilotFieldGroupObj::removeByName( const OUString& rName )
        throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException();
    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    
    if( aIt == rMembers.end() )
        throw NoSuchElementException();
    rMembers.erase( aIt );
}



sal_Int32 SAL_CALL ScDataPilotFieldGroupObj::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return static_cast< sal_Int32 >( mrParent.getFieldGroup( maGroupName ).maMembers.size() );
}

Any SAL_CALL ScDataPilotFieldGroupObj::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mrParent.getFieldGroup( maGroupName ).maMembers;
    if ((nIndex < 0) || (nIndex >= static_cast< sal_Int32 >( rMembers.size() )))
        throw IndexOutOfBoundsException();
    return Any( Reference< XNamed >( new ScDataPilotFieldGroupItemObj( *this, rMembers[ nIndex ] ) ) );
}



Reference< XEnumeration > SAL_CALL ScDataPilotFieldGroupObj::createEnumeration() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration( this, OUString( "com.sun.star.sheet.DataPilotFieldGroupEnumeration" ) );
}



uno::Type SAL_CALL ScDataPilotFieldGroupObj::getElementType() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType( (Reference< XNamed >*)0 );
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return !mrParent.getFieldGroup( maGroupName ).maMembers.empty();
}



OUString SAL_CALL ScDataPilotFieldGroupObj::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return maGroupName;
}

void SAL_CALL ScDataPilotFieldGroupObj::setName( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    mrParent.renameFieldGroup( maGroupName, rName );
    
    maGroupName = rName;
}



ScDataPilotFieldGroupItemObj::ScDataPilotFieldGroupItemObj( ScDataPilotFieldGroupObj& rParent, const OUString& rName ) :
    mrParent( rParent ),
    maName( rName )
{
    mrParent.acquire();
}

ScDataPilotFieldGroupItemObj::~ScDataPilotFieldGroupItemObj()
{
    mrParent.release();
}



OUString SAL_CALL ScDataPilotFieldGroupItemObj::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return maName;
}

void SAL_CALL ScDataPilotFieldGroupItemObj::setName( const OUString& rName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    mrParent.replaceByName( maName, Any( rName ) );
    
    maName = rName;
}



ScDataPilotItemsObj::ScDataPilotItemsObj( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId ) :
    ScDataPilotChildObjBase( rParent, rFieldId )
{
}

ScDataPilotItemsObj::~ScDataPilotItemsObj()
{
}



ScDataPilotItemObj* ScDataPilotItemsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
    return ((0 <= nIndex) && (nIndex < GetMemberCount())) ?
        new ScDataPilotItemObj( mrParent, maFieldId, nIndex ) : 0;
}



Any SAL_CALL ScDataPilotItemsObj::getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XNameAccess> xMembers = GetMembers();
    if (xMembers.is())
    {
        Reference<XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        sal_Int32 nCount = xMembersIndex->getCount();
        sal_Bool bFound(false);
        sal_Int32 nItem = 0;
        while (nItem < nCount && !bFound )
        {
            Reference<XNamed> xMember(xMembersIndex->getByIndex(nItem), UNO_QUERY);
            if (xMember.is() && (aName == xMember->getName()))
                return Any( Reference< XPropertySet >( GetObjectByIndex_Impl( nItem ) ) );
            ++nItem;
        }
        if (!bFound)
            throw NoSuchElementException();
    }
    return Any();
}

Sequence<OUString> SAL_CALL ScDataPilotItemsObj::getElementNames()
                                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aSeq;
    if( ScDPObject* pDPObj = GetDPObject() )
        pDPObj->GetMemberNames( lcl_GetObjectIndex( pDPObj, maFieldId ), aSeq );
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasByName( const OUString& aName )
                                        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Bool bFound = false;
    Reference<XNameAccess> xMembers = GetMembers();
    if (xMembers.is())
    {
        Reference<XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        sal_Int32 nCount = xMembersIndex->getCount();
        sal_Int32 nItem = 0;
        while (nItem < nCount && !bFound )
        {
            Reference<XNamed> xMember(xMembersIndex->getByIndex(nItem), UNO_QUERY);
            if (xMember.is() && aName == xMember->getName())
                bFound = sal_True;
            else
                nItem++;
        }
    }
    return bFound;
}



Reference<XEnumeration> SAL_CALL ScDataPilotItemsObj::createEnumeration()
                                                    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.DataPilotItemsEnumeration"));
}



sal_Int32 SAL_CALL ScDataPilotItemsObj::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return GetMemberCount();
}

Any SAL_CALL ScDataPilotItemsObj::getByIndex( sal_Int32 nIndex )
        throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference< XPropertySet > xItem( GetObjectByIndex_Impl( nIndex ) );
    if (!xItem.is())
        throw IndexOutOfBoundsException();
    return Any( xItem );
}

uno::Type SAL_CALL ScDataPilotItemsObj::getElementType() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((Reference<XPropertySet>*)0);
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



ScDataPilotItemObj::ScDataPilotItemObj( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId, sal_Int32 nIndex ) :
    ScDataPilotChildObjBase( rParent, rFieldId ),
    maPropSet( lcl_GetDataPilotItemMap() ),
    mnIndex( nIndex )
{
}

ScDataPilotItemObj::~ScDataPilotItemObj()
{
}

                            
OUString SAL_CALL ScDataPilotItemObj::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString sRet;
    Reference<XNameAccess> xMembers = GetMembers();
    if (xMembers.is())
    {
        Reference<XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        sal_Int32 nCount = xMembersIndex->getCount();
        if (mnIndex < nCount)
        {
            Reference<XNamed> xMember(xMembersIndex->getByIndex(mnIndex), UNO_QUERY);
            sRet = xMember->getName();
        }
    }
    return sRet;
}

void SAL_CALL ScDataPilotItemObj::setName( const OUString& /* aName */ )
                                throw(RuntimeException)
{
}

                            
Reference< XPropertySetInfo >
                            SAL_CALL ScDataPilotItemObj::getPropertySetInfo(  )
                                throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotItemObj::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = 0;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        Reference<XNameAccess> xMembers = GetMembers();
        if( xMembers.is() )
        {
            Reference<XIndexAccess> xMembersIndex( new ScNameToIndexAccess( xMembers ) );
            sal_Int32 nCount = xMembersIndex->getCount();
            if( mnIndex < nCount )
            {
                Reference<XNamed> xMember(xMembersIndex->getByIndex(mnIndex), UNO_QUERY);
                OUString sName(xMember->getName());
                ScDPSaveMember* pMember = pDim->GetMemberByName(sName);
                if (pMember)
                {
                    bool bGetNewIndex = false;
                    if ( aPropertyName == SC_UNONAME_SHOWDETAIL )
                        pMember->SetShowDetails(cppu::any2bool(aValue));
                    else if ( aPropertyName == SC_UNONAME_ISHIDDEN )
                        pMember->SetIsVisible(!cppu::any2bool(aValue));
                    else if ( aPropertyName == SC_UNONAME_POS )
                    {
                        sal_Int32 nNewPos = 0;
                        if ( ( aValue >>= nNewPos ) && nNewPos >= 0 && nNewPos < nCount )
                        {
                            pDim->SetMemberPosition( sName, nNewPos );
                            
                            bGetNewIndex = true;
                        }
                        else
                            throw IllegalArgumentException();
                    }
                    SetDPObject( pDPObj );

                    if ( bGetNewIndex )     
                    {
                        OUString aOUName( sName );
                        Sequence< OUString > aItemNames = xMembers->getElementNames();
                        sal_Int32 nItemCount = aItemNames.getLength();
                        for (sal_Int32 nItem=0; nItem<nItemCount; ++nItem)
                            if (aItemNames[nItem] == aOUName)
                                mnIndex = nItem;
                    }
                }
            }
        }
    }
}

Any SAL_CALL ScDataPilotItemObj::getPropertyValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        Reference< XNameAccess > xMembers = GetMembers();
        if( xMembers.is() )
        {
            Reference< XIndexAccess > xMembersIndex( new ScNameToIndexAccess( xMembers ) );
            sal_Int32 nCount = xMembersIndex->getCount();
            if( mnIndex < nCount )
            {
                Reference< XNamed > xMember( xMembersIndex->getByIndex( mnIndex ), UNO_QUERY );
                OUString sName( xMember->getName() );
                ScDPSaveMember* pMember = pDim->GetExistingMemberByName( sName );
                if ( aPropertyName == SC_UNONAME_SHOWDETAIL )
                {
                    if (pMember && pMember->HasShowDetails())
                    {
                        aRet <<= (bool)pMember->GetShowDetails();
                    }
                    else
                    {
                        Reference< XPropertySet > xMemberProps( xMember, UNO_QUERY );
                        if( xMemberProps.is() )
                            aRet = xMemberProps->getPropertyValue( OUString( SC_UNO_DP_SHOWDETAILS ) );
                        else
                            aRet <<= true;
                    }
                }
                else if ( aPropertyName == SC_UNONAME_ISHIDDEN )
                {
                    if (pMember && pMember->HasIsVisible())
                    {
                        aRet <<= !pMember->GetIsVisible();
                    }
                    else
                    {
                        Reference< XPropertySet > xMemberProps( xMember, UNO_QUERY );
                        if( xMemberProps.is() )
                            aRet <<= !cppu::any2bool( xMemberProps->getPropertyValue( OUString( SC_UNO_DP_ISVISIBLE ) ) );
                        else
                            aRet <<= false;
                    }
                }
                else if ( aPropertyName == SC_UNONAME_POS )
                {
                    aRet <<= mnIndex;
                }
            }
        }
    }
    return aRet;
}

void SAL_CALL ScDataPilotItemObj::addPropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* xListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removePropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::addVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SAL_CALL ScDataPilotItemObj::removeVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
