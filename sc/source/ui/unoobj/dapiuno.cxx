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

#include <algorithm>
#include <cmath>

#include <svl/hint.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <dapiuno.hxx>
#include <datauno.hxx>
#include <miscuno.hxx>
#include <convuno.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <pivot.hxx>
#include <rangeutl.hxx>
#include <dpobject.hxx>
#include <dpshttab.hxx>
#include <dpsdbtab.hxx>
#include <dpsave.hxx>
#include <dbdocfun.hxx>
#include <unonames.hxx>
#include <dpgroup.hxx>
#include <dpdimsave.hxx>
#include <hints.hxx>
#include <dputil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <generalfunction.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionData.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>

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
using ::com::sun::star::lang::NullPointerException;
using ::com::sun::star::lang::WrappedTargetException;

using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;

namespace {

const SfxItemPropertyMapEntry* lcl_GetDataPilotDescriptorBaseMap()
{
    static const SfxItemPropertyMapEntry aDataPilotDescriptorBaseMap_Impl[] =
    {
        {OUString(SC_UNO_DP_COLGRAND),     0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_DRILLDOWN),    0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_GRANDTOTAL_NAME),0,cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        {OUString(SC_UNO_DP_IGNORE_EMPTYROWS),   0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_IMPORTDESC),   0,  cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get(), 0, 0 },
        {OUString(SC_UNO_DP_REPEATEMPTY),     0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_ROWGRAND),     0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_SERVICEARG),   0,  cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get(), 0, 0 },
        {OUString(SC_UNO_DP_SHOWFILTER),     0,  cppu::UnoType<bool>::get(),  0, 0 },
        {OUString(SC_UNO_DP_SOURCESERVICE),   0,  cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotDescriptorBaseMap_Impl;
}

const SfxItemPropertyMapEntry* lcl_GetDataPilotFieldMap()
{
    using namespace ::com::sun::star::beans::PropertyAttribute;
    static const SfxItemPropertyMapEntry aDataPilotFieldMap_Impl[] =
    {
        {OUString(SC_UNONAME_AUTOSHOW),     0,  cppu::UnoType<DataPilotFieldAutoShowInfo>::get(),   MAYBEVOID, 0 },
        {OUString(SC_UNONAME_FUNCTION),     0,  cppu::UnoType<GeneralFunction>::get(),              0, 0 },
        {OUString(SC_UNONAME_FUNCTION2),    0,  cppu::UnoType<sal_Int16>::get(),             0, 0 },
        {OUString(SC_UNONAME_GROUPINFO),    0,  cppu::UnoType<DataPilotFieldGroupInfo>::get(),      MAYBEVOID, 0 },
        {OUString(SC_UNONAME_HASAUTOSHOW),  0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_HASLAYOUTINFO),0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_HASREFERENCE), 0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_HASSORTINFO),  0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_ISGROUP),      0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_LAYOUTINFO),   0,  cppu::UnoType<DataPilotFieldLayoutInfo>::get(),     MAYBEVOID, 0 },
        {OUString(SC_UNONAME_ORIENT),       0,  cppu::UnoType<DataPilotFieldOrientation>::get(),    MAYBEVOID, 0 },
        {OUString(SC_UNONAME_REFERENCE),    0,  cppu::UnoType<DataPilotFieldReference>::get(),      MAYBEVOID, 0 },
        {OUString(SC_UNONAME_SELPAGE),      0,  cppu::UnoType<OUString>::get(),                     0, 0 },
        {OUString(SC_UNONAME_SHOWEMPTY),    0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_REPEATITEMLABELS),    0,  cppu::UnoType<bool>::get(),                          0, 0 },
        {OUString(SC_UNONAME_SORTINFO),     0,  cppu::UnoType<DataPilotFieldSortInfo>::get(),       MAYBEVOID, 0 },
        {OUString(SC_UNONAME_SUBTOTALS),    0,  cppu::UnoType<Sequence<GeneralFunction>>::get(),    0, 0 },
        {OUString(SC_UNONAME_SUBTOTALS2),   0,  cppu::UnoType<Sequence<sal_Int16>>::get(),   0, 0 },
        {OUString(SC_UNONAME_USESELPAGE),   0,  cppu::UnoType<bool>::get(),                          0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotFieldMap_Impl;
}

const SfxItemPropertyMapEntry* lcl_GetDataPilotItemMap()
{
    static const SfxItemPropertyMapEntry aDataPilotItemMap_Impl[] =
    {
        {OUString(SC_UNONAME_ISHIDDEN),     0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_POS),          0,  cppu::UnoType<sal_Int32>::get(),    0, 0 },
        {OUString(SC_UNONAME_SHOWDETAIL),   0,  cppu::UnoType<bool>::get(),          0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataPilotItemMap_Impl;
}

bool lclCheckValidDouble( double fValue, bool bAuto )
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

} // namespace

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

// name that is used in the API for the data layout field
#define SC_DATALAYOUT_NAME  "Data"

ScGeneralFunction ScDataPilotConversion::FirstFunc( PivotFunc nBits )
{
    if ( nBits & PivotFunc::Sum )       return ScGeneralFunction::SUM;
    if ( nBits & PivotFunc::Count )     return ScGeneralFunction::COUNT;
    if ( nBits & PivotFunc::Average )   return ScGeneralFunction::AVERAGE;
    if ( nBits & PivotFunc::Median )    return ScGeneralFunction::MEDIAN;
    if ( nBits & PivotFunc::Max )       return ScGeneralFunction::MAX;
    if ( nBits & PivotFunc::Min )       return ScGeneralFunction::MIN;
    if ( nBits & PivotFunc::Product )   return ScGeneralFunction::PRODUCT;
    if ( nBits & PivotFunc::CountNum )  return ScGeneralFunction::COUNTNUMS;
    if ( nBits & PivotFunc::StdDev )    return ScGeneralFunction::STDEV;
    if ( nBits & PivotFunc::StdDevP )   return ScGeneralFunction::STDEVP;
    if ( nBits & PivotFunc::StdVar )    return ScGeneralFunction::VAR;
    if ( nBits & PivotFunc::StdVarP )   return ScGeneralFunction::VARP;
    if ( nBits & PivotFunc::Auto )      return ScGeneralFunction::AUTO;
    return ScGeneralFunction::NONE;
}

PivotFunc ScDataPilotConversion::FunctionBit( sal_Int16 eFunc )
{
    PivotFunc nRet = PivotFunc::NONE;  // 0
    switch (eFunc)
    {
        case GeneralFunction2::SUM:       nRet = PivotFunc::Sum;       break;
        case GeneralFunction2::COUNT:     nRet = PivotFunc::Count;     break;
        case GeneralFunction2::AVERAGE:   nRet = PivotFunc::Average;   break;
        case GeneralFunction2::MEDIAN:    nRet = PivotFunc::Median;    break;
        case GeneralFunction2::MAX:       nRet = PivotFunc::Max;       break;
        case GeneralFunction2::MIN:       nRet = PivotFunc::Min;       break;
        case GeneralFunction2::PRODUCT:   nRet = PivotFunc::Product;   break;
        case GeneralFunction2::COUNTNUMS: nRet = PivotFunc::CountNum; break;
        case GeneralFunction2::STDEV:     nRet = PivotFunc::StdDev;   break;
        case GeneralFunction2::STDEVP:    nRet = PivotFunc::StdDevP;  break;
        case GeneralFunction2::VAR:       nRet = PivotFunc::StdVar;   break;
        case GeneralFunction2::VARP:      nRet = PivotFunc::StdVarP;  break;
        case GeneralFunction2::AUTO:      nRet = PivotFunc::Auto;      break;
        default:
        {
            assert(false);
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
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
        {
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab &&
                     rDPObj.GetName() == rName )
                    return &rDPObj;
            }
        }
    }
    return nullptr;    // not found
}

static OUString lcl_CreatePivotName( ScDocShell* pDocShell )
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
            return pColl->CreateNewName();
    }
    return OUString();                    // shouldn't happen
}

static sal_Int32 lcl_GetObjectIndex( ScDPObject* pDPObj, const ScFieldIdentifier& rFieldId )
{
    // used for items - nRepeat in identifier can be ignored
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
    return -1;  // none
}

ScDataPilotTablesObj::ScDataPilotTablesObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScDataPilotTablesObj::~ScDataPilotTablesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScDataPilotTablesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! update of references

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XDataPilotTables

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByIndex_Impl( sal_Int32 nIndex )
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet
            sal_Int32 nFound = 0;
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab )
                {
                    if ( nFound == nIndex )
                    {
                        return new ScDataPilotTableObj( pDocShell, nTab, rDPObj.GetName() );
                    }
                    ++nFound;
                }
            }
        }
    }
    return nullptr;
}

ScDataPilotTableObj* ScDataPilotTablesObj::GetObjectByName_Impl(const OUString& rName)
{
    if (hasByName(rName))
        return new ScDataPilotTableObj( pDocShell, nTab, rName );
    return nullptr;
}

Reference<XDataPilotDescriptor> SAL_CALL ScDataPilotTablesObj::createDataPilotDescriptor()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScDataPilotDescriptor(pDocShell);
    return nullptr;
}

static bool lcl_IsDuplicated(const Reference<XPropertySet>& rDimProps)
{
    try
    {
        Any aAny = rDimProps->getPropertyValue( SC_UNO_DP_ORIGINAL );
        Reference< XNamed > xOriginal( aAny, UNO_QUERY );
        return xOriginal.is();
    }
    catch( Exception& )
    {
    }
    return false;
}

static OUString lcl_GetOriginalName(const Reference< XNamed >& rDim)
{
    Reference< XNamed > xOriginal;

    Reference< XPropertySet > xDimProps(rDim, UNO_QUERY);
    if ( xDimProps.is() )
    {
        try
        {
            Any aAny = xDimProps->getPropertyValue(SC_UNO_DP_ORIGINAL);
            aAny >>= xOriginal;
        }
        catch( Exception& )
        {
        }
    }

    if ( !xOriginal.is() )
        xOriginal = rDim;

    return xOriginal->getName();
}

void SAL_CALL ScDataPilotTablesObj::insertNewByName( const OUString& aNewName,
    const CellAddress& aOutputAddress,
    const Reference<XDataPilotDescriptor>& xDescriptor )
{
    SolarMutexGuard aGuard;
    if (!xDescriptor.is()) return;

    if ( !aNewName.isEmpty() && hasByName( aNewName ) )
        throw IllegalArgumentException("Name \"" + aNewName + "\" already exists", static_cast<cppu::OWeakObject*>(this), 0);

    if (!pDocShell)
        throw RuntimeException("DocShell is null", static_cast<cppu::OWeakObject*>(this));

    ScDataPilotDescriptorBase* pImp = ScDataPilotDescriptorBase::getImplementation( xDescriptor );
    if (!pImp)
        throw RuntimeException("Failed to get ScDataPilotDescriptor", static_cast<cppu::OWeakObject*>(this));

    ScDPObject* pNewObj = pImp->GetDPObject();
    if (!pNewObj)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));

    ScRange aOutputRange(static_cast<SCCOL>(aOutputAddress.Column), static_cast<SCROW>(aOutputAddress.Row), static_cast<SCTAB>(aOutputAddress.Sheet),
                        static_cast<SCCOL>(aOutputAddress.Column), static_cast<SCROW>(aOutputAddress.Row), static_cast<SCTAB>(aOutputAddress.Sheet));
    pNewObj->SetOutRange(aOutputRange);
    OUString aName = aNewName;
    if (aName.isEmpty())
        aName = lcl_CreatePivotName( pDocShell );
    pNewObj->SetName(aName);
    OUString aTag = xDescriptor->getTag();
    pNewObj->SetTag(aTag);

    // todo: handle double fields (for more information see ScDPObject)

    ScDBDocFunc aFunc(*pDocShell);
    if (!aFunc.CreatePivotTable(*pNewObj, true, true))
        throw RuntimeException("Failed to create pivot table", static_cast<cppu::OWeakObject*>(this));
}

void SAL_CALL ScDataPilotTablesObj::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject( pDocShell, nTab, aName );
    if (!pDPObj || !pDocShell)
        throw RuntimeException();       // no other exceptions specified

    ScDBDocFunc aFunc(*pDocShell);
    aFunc.RemovePivotTable(*pDPObj, true, true);  // remove - incl. undo etc.

}

// XEnumerationAccess

Reference< XEnumeration > SAL_CALL ScDataPilotTablesObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.DataPilotTablesEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotTablesObj::getCount()
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet

            sal_uInt16 nFound = 0;
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }
            return nFound;
        }
    }

    return 0;
}

Any SAL_CALL ScDataPilotTablesObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    Reference<XDataPilotTable2> xTable(GetObjectByIndex_Impl(nIndex));
    if (!xTable.is())
        throw IndexOutOfBoundsException();
    return Any( xTable );
}

uno::Type SAL_CALL ScDataPilotTablesObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<XDataPilotTable2>::get();
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

Any SAL_CALL ScDataPilotTablesObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    Reference<XDataPilotTable2> xTable(GetObjectByName_Impl(aName));
    if (!xTable.is())
        throw NoSuchElementException();
    return Any( xTable );
}

Sequence<OUString> SAL_CALL ScDataPilotTablesObj::getElementNames()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
        {
            //  count tables on this sheet

            sal_uInt16 nFound = 0;
            size_t nCount = pColl->GetCount();
            size_t i;
            for (i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab )
                    ++nFound;
            }

            sal_uInt16 nPos = 0;
            Sequence<OUString> aSeq(nFound);
            OUString* pAry = aSeq.getArray();
            for (i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab )
                    pAry[nPos++] = rDPObj.GetName();
            }

            return aSeq;
        }
    }
    return Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScDataPilotTablesObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDPCollection* pColl = rDoc.GetDPCollection();
        if ( pColl )
        {
            size_t nCount = pColl->GetCount();
            for (size_t i=0; i<nCount; ++i)
            {
                ScDPObject& rDPObj = (*pColl)[i];
                if ( rDPObj.GetOutRange().aStart.Tab() == nTab &&
                     rDPObj.GetName() == aName )
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
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScDataPilotDescriptorBase::~ScDataPilotDescriptorBase()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

Any SAL_CALL ScDataPilotDescriptorBase::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( XDataPilotDescriptor )
    SC_QUERYINTERFACE( XPropertySet )
    SC_QUERYINTERFACE( XDataPilotDataLayoutFieldSupplier )
    SC_QUERYINTERFACE( XNamed )                 // base of XDataPilotDescriptor
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
{
    static Sequence< uno::Type > const aTypes
    {
        cppu::UnoType<XDataPilotDescriptor>::get(),
        cppu::UnoType<XPropertySet>::get(),
        cppu::UnoType<XDataPilotDataLayoutFieldSupplier>::get(),
        cppu::UnoType<lang::XUnoTunnel>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<lang::XServiceInfo>::get(),
    };
    return aTypes;
}

Sequence<sal_Int8> SAL_CALL ScDataPilotDescriptorBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void ScDataPilotDescriptorBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! update of references ?

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XDataPilotDescriptor

CellRangeAddress SAL_CALL ScDataPilotDescriptorBase::getSourceRange()
{
    SolarMutexGuard aGuard;

    ScDPObject* pDPObject(GetDPObject());
    if (!pDPObject)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));

    CellRangeAddress aRet;
    if (pDPObject->IsSheetData())
        ScUnoConversion::FillApiRange( aRet, pDPObject->GetSheetDesc()->GetSourceRange() );
    return aRet;
}

void SAL_CALL ScDataPilotDescriptorBase::setSourceRange( const CellRangeAddress& aSourceRange )
{
    SolarMutexGuard aGuard;

    ScDPObject* pDPObject = GetDPObject();
    if (!pDPObject)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));

    ScSheetSourceDesc aSheetDesc(&pDocShell->GetDocument());
    if (pDPObject->IsSheetData())
        aSheetDesc = *pDPObject->GetSheetDesc();

    ScRange aRange;
    ScUnoConversion::FillScRange(aRange, aSourceRange);
    aSheetDesc.SetSourceRange(aRange);
    pDPObject->SetSheetDesc( aSheetDesc );
    SetDPObject( pDPObject );
}

Reference<XSheetFilterDescriptor> SAL_CALL ScDataPilotDescriptorBase::getFilterDescriptor()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFilterDescriptor( pDocShell, this );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataPilotFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getColumnFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_COLUMN );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getRowFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_ROW );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getPageFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_PAGE );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getDataFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_DATA );
}

Reference<XIndexAccess> SAL_CALL ScDataPilotDescriptorBase::getHiddenFields()
{
    SolarMutexGuard aGuard;
    return new ScDataPilotFieldsObj( *this, DataPilotFieldOrientation_HIDDEN );
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL ScDataPilotDescriptorBase::getPropertySetInfo(  )
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotDescriptorBase::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
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

            if ( aPropertyName == SC_UNO_DP_COLGRAND )
            {
                aNewData.SetColumnGrand(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_IGNORE_EMPTYROWS )
            {
                aNewData.SetIgnoreEmptyRows(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_REPEATEMPTY )
            {
                aNewData.SetRepeatIfEmpty(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_ROWGRAND )
            {
                aNewData.SetRowGrand(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_SHOWFILTER )
            {
                aNewData.SetFilterButton(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_DRILLDOWN )
            {
                aNewData.SetDrillDown(::cppu::any2bool( aValue ));
            }
            else if ( aPropertyName == SC_UNO_DP_GRANDTOTAL_NAME )
            {
                OUString aStrVal;
                if ( aValue >>= aStrVal )
                    aNewData.SetGrandTotalName(aStrVal);
            }
            else if ( aPropertyName == SC_UNO_DP_IMPORTDESC )
            {
                uno::Sequence<beans::PropertyValue> aArgSeq;
                if ( aValue >>= aArgSeq )
                {
                    ScImportSourceDesc aImportDesc(&pDocShell->GetDocument());

                    const ScImportSourceDesc* pOldDesc = pDPObject->GetImportSourceDesc();
                    if (pOldDesc)
                        aImportDesc = *pOldDesc;

                    ScImportParam aParam;
                    ScImportDescriptor::FillImportParam( aParam, aArgSeq );

                    sheet::DataImportMode nNewType = sheet::DataImportMode_NONE;
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
            else if ( aPropertyName == SC_UNO_DP_SOURCESERVICE )
            {
                OUString aStrVal;
                if ( aValue >>= aStrVal )
                {
                    ScDPServiceDesc aServiceDesc("", "", "", "", "");

                    const ScDPServiceDesc* pOldDesc = pDPObject->GetDPServiceDesc();
                    if (pOldDesc)
                        aServiceDesc = *pOldDesc;

                    aServiceDesc.aServiceName = aStrVal;

                    pDPObject->SetServiceData( aServiceDesc );
                }
            }
            else if ( aPropertyName == SC_UNO_DP_SERVICEARG )
            {
                uno::Sequence<beans::PropertyValue> aArgSeq;
                if ( aValue >>= aArgSeq )
                {
                    ScDPServiceDesc aServiceDesc("", "", "", "", "");

                    const ScDPServiceDesc* pOldDesc = pDPObject->GetDPServiceDesc();
                    if (pOldDesc)
                        aServiceDesc = *pOldDesc;

                    OUString aStrVal;
                    sal_Int32 nArgs = aArgSeq.getLength();
                    for (sal_Int32 nArgPos=0; nArgPos<nArgs; ++nArgPos)
                    {
                        const beans::PropertyValue& rProp = aArgSeq[nArgPos];
                        OUString aPropName(rProp.Name);

                        if (aPropName == SC_UNO_DP_SOURCENAME)
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParSource = aStrVal;
                        }
                        else if (aPropName == SC_UNO_DP_OBJECTNAME)
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParName = aStrVal;
                        }
                        else if (aPropName == SC_UNO_DP_USERNAME)
                        {
                            if ( rProp.Value >>= aStrVal )
                                aServiceDesc.aParUser = aStrVal;
                        }
                        else if (aPropName == SC_UNO_DP_PASSWORD)
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

            if ( aPropertyName == SC_UNO_DP_COLGRAND )
            {
                aRet <<= aNewData.GetColumnGrand();
            }
            else if ( aPropertyName == SC_UNO_DP_IGNORE_EMPTYROWS )
            {
                aRet <<= aNewData.GetIgnoreEmptyRows();
            }
            else if ( aPropertyName == SC_UNO_DP_REPEATEMPTY )
            {
                aRet <<= aNewData.GetRepeatIfEmpty();
            }
            else if ( aPropertyName == SC_UNO_DP_ROWGRAND )
            {
                aRet <<= aNewData.GetRowGrand();
            }
            else if ( aPropertyName == SC_UNO_DP_SHOWFILTER )
            {
                aRet <<= aNewData.GetFilterButton();
            }
            else if ( aPropertyName == SC_UNO_DP_DRILLDOWN )
            {
                aRet <<= aNewData.GetDrillDown();
            }
            else if ( aPropertyName == SC_UNO_DP_GRANDTOTAL_NAME )
            {
                const boost::optional<OUString> & pGrandTotalName = aNewData.GetGrandTotalName();
                if (pGrandTotalName)
                    aRet <<= *pGrandTotalName;      // same behavior as in ScDPSource
            }
            else if ( aPropertyName == SC_UNO_DP_IMPORTDESC )
            {
                const ScImportSourceDesc* pImportDesc = pDPObject->GetImportSourceDesc();
                if ( pImportDesc )
                {
                    // fill ScImportParam so ScImportDescriptor::FillProperties can be used
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
                    // empty sequence
                    uno::Sequence<beans::PropertyValue> aEmpty(0);
                    aRet <<= aEmpty;
                }
            }
            else if ( aPropertyName == SC_UNO_DP_SOURCESERVICE )
            {
                OUString aServiceName;
                const ScDPServiceDesc* pServiceDesc = pDPObject->GetDPServiceDesc();
                if (pServiceDesc)
                    aServiceName = pServiceDesc->aServiceName;
                aRet <<= aServiceName;      // empty string if no ServiceDesc set
            }
            else if ( aPropertyName == SC_UNO_DP_SERVICEARG )
            {
                const ScDPServiceDesc* pServiceDesc = pDPObject->GetDPServiceDesc();
                if (pServiceDesc)
                {
                    uno::Sequence<beans::PropertyValue> aSeq( comphelper::InitPropertySequence({
                            { SC_UNO_DP_SOURCENAME, Any(pServiceDesc->aParSource) },
                            { SC_UNO_DP_OBJECTNAME, Any(pServiceDesc->aParName) },
                            { SC_UNO_DP_USERNAME, Any(pServiceDesc->aParUser) },
                            { SC_UNO_DP_PASSWORD, Any(pServiceDesc->aParPass) }
                        }));
                    aRet <<= aSeq;
                }
                else
                {
                    // empty sequence
                    uno::Sequence<beans::PropertyValue> aEmpty;
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
{
}

void SAL_CALL ScDataPilotDescriptorBase::removePropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference<XPropertyChangeListener >& /* aListener */ )
{
}

void SAL_CALL ScDataPilotDescriptorBase::addVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference<XVetoableChangeListener >& /* aListener */ )
{
}

void SAL_CALL ScDataPilotDescriptorBase::removeVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference<XVetoableChangeListener >& /* aListener */ )
{
}

// XDataPilotDataLayoutFieldSupplier

Reference< XDataPilotField > SAL_CALL ScDataPilotDescriptorBase::getDataLayoutField()
{
    SolarMutexGuard aGuard;
    if( ScDPObject* pDPObject = GetDPObject() )
    {
        if( ScDPSaveData* pSaveData = pDPObject->GetSaveData() )
        {
            if( pSaveData->GetDataLayoutDimension() )
            {
                ScFieldIdentifier aFieldId( SC_DATALAYOUT_NAME, true );
                return new ScDataPilotFieldObj( *this, aFieldId );
            }
        }
    }
    return nullptr;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScDataPilotDescriptorBase::getSomething(
                const Sequence<sal_Int8 >& rId )
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

ScDataPilotDescriptorBase* ScDataPilotDescriptorBase::getImplementation(const Reference<XDataPilotDescriptor>& rObj )
{
    ScDataPilotDescriptorBase* pRet = nullptr;
    Reference<lang::XUnoTunnel> xUT(rObj, UNO_QUERY);
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
{
    // since we manually do resolve the query for XDataPilotTable2
    // we also need to do the same for XDataPilotTable
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

Sequence< uno::Type > SAL_CALL ScDataPilotTableObj::getTypes()
{
    return comphelper::concatSequences(
        ScDataPilotDescriptorBase::getTypes(),
        Sequence< uno::Type >
        {
            cppu::UnoType<XDataPilotTable2>::get(),
            cppu::UnoType<XModifyBroadcaster>::get()
        } );
}

Sequence<sal_Int8> SAL_CALL ScDataPilotTableObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
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

// "rest of XDataPilotDescriptor"

OUString SAL_CALL ScDataPilotTableObj::getName()
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetName();
    return OUString();
}

void SAL_CALL ScDataPilotTableObj::setName( const OUString& aNewName )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        //! test for existing names !!!

        pDPObj->SetName( aNewName );     //! Undo - DBDocFunc ???
        aName = aNewName;

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

OUString SAL_CALL ScDataPilotTableObj::getTag()
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
        return pDPObj->GetTag();
    return OUString();
}

void SAL_CALL ScDataPilotTableObj::setTag( const OUString& aNewTag )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName);
    if (pDPObj)
    {
        pDPObj->SetTag( aNewTag );      //! Undo - DBDocFunc ???

        //  DataPilotUpdate would do too much (output table is not changed)
        GetDocShell()->SetDocumentModified();
    }
}

// XDataPilotTable

CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRange()
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

void SAL_CALL ScDataPilotTableObj::refresh()
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
{
    SolarMutexGuard aGuard;
    Sequence< Sequence<Any> > aTabData;
    ScAddress aAddr2(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet);
    ScDPObject* pObj = GetDPObject();
    if (!pObj)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));

    pObj->GetDrillDownData(aAddr2, aTabData);
    return aTabData;
}

DataPilotTablePositionData SAL_CALL ScDataPilotTableObj::getPositionData(const CellAddress& aAddr)
{
    SolarMutexGuard aGuard;
    DataPilotTablePositionData aPosData;
    ScAddress aAddr2(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet);
    ScDPObject* pObj = GetDPObject();
    if (!pObj)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));

    pObj->GetPositionData(aAddr2, aPosData);
    return aPosData;
}

void SAL_CALL ScDataPilotTableObj::insertDrillDownSheet(const CellAddress& aAddr)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = GetDPObject();
    if (!pDPObj)
        throw RuntimeException("Failed to get DPObject", static_cast<cppu::OWeakObject*>(this));
    ScTabViewShell* pViewSh = GetDocShell()->GetBestViewShell();
    if (!pViewSh)
        throw RuntimeException("Failed to get ViewShell", static_cast<cppu::OWeakObject*>(this));

    Sequence<DataPilotFieldFilter> aFilters;
    pDPObj->GetDataFieldPositionData(
        ScAddress(static_cast<SCCOL>(aAddr.Column), static_cast<SCROW>(aAddr.Row), aAddr.Sheet), aFilters);
    pViewSh->ShowDataPilotSourceData(*pDPObj, aFilters);
}

CellRangeAddress SAL_CALL ScDataPilotTableObj::getOutputRangeByType( sal_Int32 nType )
{
    SolarMutexGuard aGuard;
    if (nType < 0 || nType > DataPilotOutputRangeType::RESULT)
        throw IllegalArgumentException("nType must be between 0 and " +
                OUString::number(DataPilotOutputRangeType::RESULT) + ", got " + OUString::number(nType),
                static_cast<cppu::OWeakObject*>(this), 0);

    CellRangeAddress aRet;
    if (ScDPObject* pDPObj = lcl_GetDPObject(GetDocShell(), nTab, aName))
        ScUnoConversion::FillApiRange( aRet, pDPObj->GetOutputRangeByType( nType ) );
    return aRet;
}

void SAL_CALL ScDataPilotTableObj::addModifyListener( const uno::Reference<util::XModifyListener>& aListener )
{
    SolarMutexGuard aGuard;

    aModifyListeners.emplace_back( aListener );

    if ( aModifyListeners.size() == 1 )
    {
        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScDataPilotTableObj::removeModifyListener( const uno::Reference<util::XModifyListener>& aListener )
{
    SolarMutexGuard aGuard;

    rtl::Reference<ScDataPilotTableObj> aSelfHold(this); // in case the listeners have the last ref

    sal_uInt16 nCount = aModifyListeners.size();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = aModifyListeners[n];
        if ( rObj == aListener )
        {
            aModifyListeners.erase( aModifyListeners.begin() + n );

            if ( aModifyListeners.empty() )
            {
                release();      // release the ref for the listeners
            }

            break;
        }
    }
}

void ScDataPilotTableObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScDataPilotModifiedHint*>(&rHint) &&
         static_cast<const ScDataPilotModifiedHint&>(rHint).GetName() == aName )
    {
        Refreshed_Impl();
    }
    else if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        ScRange aRange( 0, 0, nTab );
        ScRangeList aRanges( aRange );
        const ScUpdateRefHint& rRef = static_cast< const ScUpdateRefHint& >( rHint );
        if ( aRanges.UpdateReference( rRef.GetMode(), &GetDocShell()->GetDocument(), rRef.GetRange(),
                 rRef.GetDx(), rRef.GetDy(), rRef.GetDz() ) &&
             aRanges.size() == 1 )
        {
            nTab = aRanges.front().aStart.Tab();
        }
    }

    ScDataPilotDescriptorBase::Notify( rBC, rHint );
}

void ScDataPilotTableObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));

    // the EventObject holds a Ref to this object until after the listener calls

    ScDocument& rDoc = GetDocShell()->GetDocument();
    for (uno::Reference<util::XModifyListener> & xModifyListener : aModifyListeners)
        rDoc.AddUnoListenerCall( xModifyListener, aEvent );
}

ScDataPilotDescriptor::ScDataPilotDescriptor(ScDocShell* pDocSh) :
    ScDataPilotDescriptorBase( pDocSh ),
    mpDPObject(new ScDPObject(pDocSh ? &pDocSh->GetDocument() : nullptr) )
{
    ScDPSaveData aSaveData;
    // set defaults like in ScPivotParam constructor
    aSaveData.SetColumnGrand( true );
    aSaveData.SetRowGrand( true );
    aSaveData.SetIgnoreEmptyRows( false );
    aSaveData.SetRepeatIfEmpty( false );
    mpDPObject->SetSaveData(aSaveData);
    ScSheetSourceDesc aSheetDesc(pDocSh ? &pDocSh->GetDocument() : nullptr);
    mpDPObject->SetSheetDesc(aSheetDesc);
}

ScDataPilotDescriptor::~ScDataPilotDescriptor()
{
}

ScDPObject* ScDataPilotDescriptor::GetDPObject() const
{
    return mpDPObject.get();
}

void ScDataPilotDescriptor::SetDPObject( ScDPObject* pDPObject )
{
    if (mpDPObject.get() != pDPObject)
    {
        mpDPObject.reset( pDPObject );
        OSL_FAIL("replace DPObject should not happen");
    }
}

// "rest of XDataPilotDescriptor"

OUString SAL_CALL ScDataPilotDescriptor::getName()
{
    SolarMutexGuard aGuard;
    return mpDPObject->GetName();
}

void SAL_CALL ScDataPilotDescriptor::setName( const OUString& aNewName )
{
    SolarMutexGuard aGuard;
    mpDPObject->SetName( aNewName );
}

OUString SAL_CALL ScDataPilotDescriptor::getTag()
{
    SolarMutexGuard aGuard;
    return mpDPObject->GetTag();
}

void SAL_CALL ScDataPilotDescriptor::setTag( const OUString& aNewTag )
{
    SolarMutexGuard aGuard;
    mpDPObject->SetTag( aNewTag );
}

ScDataPilotChildObjBase::ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent ) :
    mxParent( &rParent )
{
}

ScDataPilotChildObjBase::ScDataPilotChildObjBase( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId ) :
    mxParent( &rParent ),
    maFieldId( rFieldId )
{
}

ScDataPilotChildObjBase::~ScDataPilotChildObjBase()
{
}

ScDPObject* ScDataPilotChildObjBase::GetDPObject() const
{
    return mxParent->GetDPObject();
}

void ScDataPilotChildObjBase::SetDPObject( ScDPObject* pDPObject )
{
    mxParent->SetDPObject( pDPObject );
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

            // find dimension with specified index (search in duplicated dimensions)
            const ScDPSaveData::DimsType& rDims = pSaveData->GetDimensions();

            sal_Int32 nFoundIdx = 0;
            for (auto const& it : rDims)
            {
                if (it->IsDataLayout())
                    continue;

                OUString aSrcName = ScDPUtil::getSourceDimensionName(it->GetName());
                if (aSrcName == maFieldId.maFieldName)
                {
                    if( nFoundIdx == maFieldId.mnFieldIdx )
                        return it.get();
                    ++nFoundIdx;
                }
            }
        }
    }
    return nullptr;
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

Reference< XMembersAccess > ScDataPilotChildObjBase::GetMembers() const
{
    Reference< XMembersAccess > xMembersNA;
    if( ScDPObject* pDPObj = GetDPObject() )
        pDPObj->GetMembersNA( lcl_GetObjectIndex( pDPObj, maFieldId ), xMembersNA );
    return xMembersNA;
}

ScDocShell* ScDataPilotChildObjBase::GetDocShell() const
{
    return mxParent->GetDocShell();
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
        throw NullPointerException();

    sal_Int32 nRet = 0;

    Reference<XNameAccess> xDimsName(rSource->getDimensions());
    Reference<XIndexAccess> xIntDims(new ScNameToIndexAccess( xDimsName ));
    sal_Int32 nIntCount = xIntDims->getCount();
    if (rOrient.hasValue())
    {
        // all fields of the specified orientation, including duplicated
        Reference<XPropertySet> xDim;
        for (sal_Int32 i = 0; i < nIntCount; ++i)
        {
            xDim.set(xIntDims->getByIndex(i), UNO_QUERY);
            if (xDim.is() && (xDim->getPropertyValue(SC_UNO_DP_ORIENTATION) == rOrient))
                ++nRet;
        }
    }
    else
    {
        // count all non-duplicated fields

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

static bool lcl_GetFieldDataByIndex( const Reference<XDimensionsSupplier>& rSource,
                                const Any& rOrient, SCSIZE nIndex, ScFieldIdentifier& rFieldId )
{
    if (!rSource.is())
        throw NullPointerException();

    bool bOk = false;
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
            if (xDim.is() && (xDim->getPropertyValue(SC_UNO_DP_ORIENTATION) == rOrient))
            {
                if (nPos == nIndex)
                {
                    bOk = true;
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
                    bOk = true;
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
                        SC_UNO_DP_ISDATALAYOUT );

            sal_Int32 nRepeat = 0;
            if ( rOrient.hasValue() && lcl_IsDuplicated( xDim ) )
            {
                // find the repeat count
                // (this relies on the original dimension always being before the duplicates)

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

static bool lcl_GetFieldDataByName( ScDPObject* pDPObj, const OUString& rFieldName, ScFieldIdentifier& rFieldId )
{
    // "By name" is always the first match.
    // The name "Data" always refers to the data layout field.
    rFieldId.maFieldName = rFieldName;
    rFieldId.mnFieldIdx = 0;
    rFieldId.mbDataLayout = rFieldName == SC_DATALAYOUT_NAME;

    pDPObj->GetSource();    // IsDimNameInUse doesn't update source data

    // check if the named field exists (not for data layout)
    return rFieldId.mbDataLayout || pDPObj->IsDimNameInUse( rFieldName );
}

// XDataPilotFields

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
    if (ScDPObject* pObj = GetDPObject())
    {
        ScFieldIdentifier aFieldId;
        if (lcl_GetFieldDataByIndex( pObj->GetSource(), maOrient, nIndex, aFieldId ))
            return new ScDataPilotFieldObj( *mxParent, aFieldId, maOrient );
    }
    return nullptr;
}

ScDataPilotFieldObj* ScDataPilotFieldsObj::GetObjectByName_Impl(const OUString& aName) const
{
    if (ScDPObject* pDPObj = GetDPObject())
    {
        ScFieldIdentifier aFieldId;
        if (lcl_GetFieldDataByName( pDPObj, aName, aFieldId ))
            return new ScDataPilotFieldObj( *mxParent, aFieldId, maOrient );
    }
    return nullptr;
}

// XEnumerationAccess

Reference<XEnumeration> SAL_CALL ScDataPilotFieldsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.DataPilotFieldsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldsObj::getCount()
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = GetDPObject();
    return pDPObj ? lcl_GetFieldCount( pDPObj->GetSource(), maOrient ) : 0;
}

Any SAL_CALL ScDataPilotFieldsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    Reference< XPropertySet > xField( GetObjectByIndex_Impl( nIndex ) );
    if (!xField.is())
        throw IndexOutOfBoundsException();
    return Any( xField );
}

// XElementAccess

uno::Type SAL_CALL ScDataPilotFieldsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL ScDataPilotFieldsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

Any SAL_CALL ScDataPilotFieldsObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xField(GetObjectByName_Impl(aName));
    if (!xField.is())
        throw NoSuchElementException();
    return Any( xField );
}

Sequence<OUString> SAL_CALL ScDataPilotFieldsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    if (ScDPObject* pDPObj = GetDPObject())
    {
        Sequence< OUString > aSeq( lcl_GetFieldCount( pDPObj->GetSource(), maOrient ) );
        OUString* pAry = aSeq.getArray();

        const ScDPSaveData::DimsType& rDimensions = pDPObj->GetSaveData()->GetDimensions();
        for (auto const& it : rDimensions)
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
{
    SolarMutexGuard aGuard;

    return GetObjectByName_Impl(aName) != nullptr;
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

// XNamed

OUString SAL_CALL ScDataPilotFieldObj::getName()
{
    SolarMutexGuard aGuard;
    OUString aName;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->IsDataLayout() )
            aName = SC_DATALAYOUT_NAME;
        else
        {
            const boost::optional<OUString> & pLayoutName = pDim->GetLayoutName();
            if (pLayoutName)
                aName = *pLayoutName;
            else
                aName = pDim->GetName();
        }                                                                }
    return aName;
}

void SAL_CALL ScDataPilotFieldObj::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    ScDPSaveDimension* pDim = GetDPDimension( &pDPObj );
    if( pDim && !pDim->IsDataLayout() )
    {
        pDim->SetLayoutName(rName);
        SetDPObject( pDPObj );
    }
}

// XPropertySet

Reference<XPropertySetInfo> SAL_CALL ScDataPilotFieldObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDataPilotFieldObj::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    SolarMutexGuard aGuard;
    if ( aPropertyName == SC_UNONAME_FUNCTION )
    {
        // #i109350# use GetEnumFromAny because it also allows sal_Int32
        ScGeneralFunction eFunction = static_cast<ScGeneralFunction>(ScUnoHelpFunctions::GetEnumFromAny( aValue ));
        setFunction( eFunction );
    }
    else if ( aPropertyName == SC_UNONAME_FUNCTION2 )
    {
        ScGeneralFunction eFunction = static_cast<ScGeneralFunction>(ScUnoHelpFunctions::GetInt16FromAny( aValue ));
        setFunction( eFunction );
    }
    else if ( aPropertyName == SC_UNONAME_SUBTOTALS )
    {
        uno::Sequence<sheet::GeneralFunction> aSeq;
        if( aValue >>= aSeq)
        {
            std::vector< ScGeneralFunction > aSubTotals(aSeq.getLength());
            for (sal_Int32 nIndex = 0; nIndex < aSeq.getLength(); nIndex++)
            {
                const int nValAsInt = static_cast<int>(aSeq[nIndex]);
                aSubTotals[nIndex] = static_cast<ScGeneralFunction>(nValAsInt);
            }
            setSubtotals( aSubTotals );
        }
    }
    else if ( aPropertyName == SC_UNONAME_SUBTOTALS2 )
    {
        Sequence< sal_Int16 > aSeq;
        if( aValue >>= aSeq )
        {
            std::vector< ScGeneralFunction > aSubTotals(aSeq.getLength());
            for (sal_Int32 nIndex = 0; nIndex < aSeq.getLength(); nIndex++)
            {
                aSubTotals[nIndex] = static_cast<ScGeneralFunction>(aSeq[nIndex]);
            }
            setSubtotals( aSubTotals );
        }
    }
    else if ( aPropertyName == SC_UNONAME_ORIENT )
    {
        //! test for correct enum type?
        DataPilotFieldOrientation eOrient = static_cast<DataPilotFieldOrientation>(ScUnoHelpFunctions::GetEnumFromAny( aValue ));
        setOrientation( eOrient );
    }
    else if ( aPropertyName == SC_UNONAME_SELPAGE )
    {
        OUString sCurrentPage;
        if (aValue >>= sCurrentPage)
            setCurrentPage(sCurrentPage);
    }
    else if ( aPropertyName == SC_UNONAME_USESELPAGE )
    {
        setUseCurrentPage(cppu::any2bool(aValue));
    }
    else if ( aPropertyName == SC_UNONAME_HASAUTOSHOW )
    {
        if (!cppu::any2bool(aValue))
            setAutoShowInfo(nullptr);
    }
    else if ( aPropertyName == SC_UNONAME_AUTOSHOW )
    {
        DataPilotFieldAutoShowInfo aInfo;
        if (aValue >>= aInfo)
            setAutoShowInfo(&aInfo);
    }
    else if ( aPropertyName == SC_UNONAME_HASLAYOUTINFO )
    {
        if (!cppu::any2bool(aValue))
            setLayoutInfo(nullptr);
    }
    else if ( aPropertyName == SC_UNONAME_LAYOUTINFO )
    {
        DataPilotFieldLayoutInfo aInfo;
        if (aValue >>= aInfo)
            setLayoutInfo(&aInfo);
    }
    else if ( aPropertyName == SC_UNONAME_HASREFERENCE )
    {
        if (!cppu::any2bool(aValue))
            setReference(nullptr);
    }
    else if ( aPropertyName == SC_UNONAME_REFERENCE )
    {
        DataPilotFieldReference aRef;
        if (aValue >>= aRef)
            setReference(&aRef);
    }
    else if ( aPropertyName == SC_UNONAME_HASSORTINFO )
    {
        if (!cppu::any2bool(aValue))
            setSortInfo(nullptr);
    }
    else if ( aPropertyName == SC_UNONAME_SORTINFO )
    {
        DataPilotFieldSortInfo aInfo;
        if (aValue >>= aInfo)
            setSortInfo(&aInfo);
    }
    else if ( aPropertyName == SC_UNONAME_ISGROUP )
    {
        if (!cppu::any2bool(aValue))
            setGroupInfo(nullptr);
    }
    else if ( aPropertyName == SC_UNONAME_GROUPINFO )
    {
        DataPilotFieldGroupInfo aInfo;
        if (aValue >>= aInfo)
            setGroupInfo(&aInfo);
    }
    else if ( aPropertyName == SC_UNONAME_SHOWEMPTY )
    {
        setShowEmpty(cppu::any2bool(aValue));
    }
    else if ( aPropertyName == SC_UNONAME_REPEATITEMLABELS )
    {
        setRepeatItemLabels(cppu::any2bool(aValue));
    }
}

Any SAL_CALL ScDataPilotFieldObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    Any aRet;

    if ( aPropertyName == SC_UNONAME_FUNCTION )
    {
        sheet::GeneralFunction eVal;
        sal_Int16 nFunction = getFunction();
        if (nFunction == sheet::GeneralFunction2::MEDIAN)
        {
            eVal = sheet::GeneralFunction_NONE;
        }
        else
        {
            eVal = static_cast<sheet::GeneralFunction>(nFunction);
        }
        aRet <<= eVal;
    }
    else if ( aPropertyName == SC_UNONAME_FUNCTION2 )
        aRet <<= getFunction();
    else if ( aPropertyName == SC_UNONAME_SUBTOTALS )
    {
        uno::Sequence<sal_Int16> aSeq = getSubtotals();
        uno::Sequence<sheet::GeneralFunction>  aNewSeq;
        aNewSeq.realloc(aSeq.getLength());
        for (sal_Int32 nIndex = 0; nIndex < aSeq.getLength(); nIndex++)
        {
            if (aSeq[nIndex] == sheet::GeneralFunction2::MEDIAN)
                aNewSeq[nIndex] = sheet::GeneralFunction_NONE;
            else
                aNewSeq[nIndex] = static_cast<sheet::GeneralFunction>(aSeq[nIndex]);
        }
        aRet <<= aNewSeq;
    }
    else if ( aPropertyName == SC_UNONAME_SUBTOTALS2 )
    {
        aRet <<= getSubtotals();
    }
    else if ( aPropertyName == SC_UNONAME_ORIENT )
        aRet <<= getOrientation();
    else if ( aPropertyName == SC_UNONAME_SELPAGE )
        aRet <<= OUString();
    else if ( aPropertyName == SC_UNONAME_USESELPAGE )
        aRet <<= false;
    else if ( aPropertyName == SC_UNONAME_HASAUTOSHOW )
        aRet <<= (getAutoShowInfo() != nullptr);
    else if ( aPropertyName == SC_UNONAME_AUTOSHOW )
    {
        const DataPilotFieldAutoShowInfo* pInfo = getAutoShowInfo();
        if (pInfo)
            aRet <<= *pInfo;
    }
    else if ( aPropertyName == SC_UNONAME_HASLAYOUTINFO )
        aRet <<= (getLayoutInfo() != nullptr);
    else if ( aPropertyName == SC_UNONAME_LAYOUTINFO )
    {
        const DataPilotFieldLayoutInfo* pInfo = getLayoutInfo();
        if (pInfo)
            aRet <<= *pInfo;
    }
    else if ( aPropertyName == SC_UNONAME_HASREFERENCE )
        aRet <<= (getReference() != nullptr);
    else if ( aPropertyName == SC_UNONAME_REFERENCE )
    {
        const DataPilotFieldReference* pRef = getReference();
        if (pRef)
            aRet <<= *pRef;
    }
    else if ( aPropertyName == SC_UNONAME_HASSORTINFO )
        aRet <<= (getSortInfo() != nullptr);
    else if ( aPropertyName == SC_UNONAME_SORTINFO )
    {
        const DataPilotFieldSortInfo* pInfo = getSortInfo();
        if (pInfo)
            aRet <<= *pInfo;
    }
    else if ( aPropertyName == SC_UNONAME_ISGROUP )
        aRet <<= hasGroupInfo();
    else if ( aPropertyName == SC_UNONAME_GROUPINFO )
    {
        aRet <<= getGroupInfo();
    }
    else if ( aPropertyName == SC_UNONAME_SHOWEMPTY )
        aRet <<= getShowEmpty();
    else if ( aPropertyName == SC_UNONAME_REPEATITEMLABELS )
        aRet <<= getRepeatItemLabels();

    return aRet;
}

// XDatePilotField

Reference<XIndexAccess> SAL_CALL ScDataPilotFieldObj::getItems()
{
    SolarMutexGuard aGuard;
    if (!mxItems.is())
        mxItems.set( new ScDataPilotItemsObj( *mxParent, maFieldId ) );
    return mxItems;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDataPilotFieldObj )

DataPilotFieldOrientation ScDataPilotFieldObj::getOrientation() const
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetOrientation() : DataPilotFieldOrientation_HIDDEN;
}

void ScDataPilotFieldObj::setOrientation(DataPilotFieldOrientation eNew)
{
    SolarMutexGuard aGuard;
    if (maOrient.hasValue() && (eNew == maOrient.get< DataPilotFieldOrientation >()))
        return;

    ScDPObject* pDPObj = nullptr;
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

            ScDPSaveDimension* pNewDim = nullptr;

            // look for existing duplicate with orientation "hidden"

            sal_Int32 nFound = 0;
            const ScDPSaveData::DimsType& rDimensions = pSaveData->GetDimensions();
            for (auto const& it : rDimensions)
            {
                if ( !it->IsDataLayout() && (it->GetName() == maFieldId.maFieldName) )
                {
                    if ( it->GetOrientation() == DataPilotFieldOrientation_HIDDEN )
                    {
                        pNewDim = it.get();     // use this one
                        break;
                    }
                    else
                        ++nFound;               // count existing non-hidden occurrences
                }
            }

            if ( !pNewDim )     // if none found, create a new duplicated dimension
                pNewDim = &pSaveData->DuplicateDimension( *pDim );

            maFieldId.mnFieldIdx = nFound;      // keep accessing the new one
            pDim = pNewDim;
        }

        pDim->SetOrientation(eNew);

        // move changed field behind all other fields (make it the last field in dimension)
        pSaveData->SetPosition( pDim, pSaveData->GetDimensions().size() );

        SetDPObject( pDPObj );

        maOrient <<= eNew;   // modifying the same object's orientation again doesn't create another duplicate
    }
}

sal_Int16 ScDataPilotFieldObj::getFunction() const
{
    SolarMutexGuard aGuard;
    sal_Int16 eRet = GeneralFunction2::NONE;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            // for non-data fields, property Function is the subtotals
            long nSubCount = pDim->GetSubTotalsCount();
            if ( nSubCount > 0 )
                eRet = static_cast<sal_Int16>(pDim->GetSubTotalFunc(0));    // always use the first one
            // else keep NONE
        }
        else
            eRet = static_cast<sal_Int16>(pDim->GetFunction());
    }
    return eRet;
}

void ScDataPilotFieldObj::setFunction(ScGeneralFunction eNewFunc)
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            // for non-data fields, property Function is the subtotals
            std::vector<ScGeneralFunction> nSubTotalFuncs;
            if ( eNewFunc != ScGeneralFunction::NONE )
            {
                nSubTotalFuncs.push_back( eNewFunc );
            }
            pDim->SetSubTotals( nSubTotalFuncs );
        }
        else
            pDim->SetFunction( eNewFunc );
        SetDPObject( pDPObj );
    }
}

Sequence< sal_Int16 > ScDataPilotFieldObj::getSubtotals() const
{
    SolarMutexGuard aGuard;
    Sequence< sal_Int16 > aRet;
    if( ScDPSaveDimension* pDim = GetDPDimension() )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            // for non-data fields, property Functions is the sequence of subtotals
            sal_Int32 nCount = static_cast< sal_Int32 >( pDim->GetSubTotalsCount() );
            if ( nCount > 0 )
            {
                aRet.realloc( nCount );
                for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
                    aRet[ nIdx ] = static_cast<sal_Int16>(pDim->GetSubTotalFunc( nIdx ));
            }
        }
    }
    return aRet;
}

void ScDataPilotFieldObj::setSubtotals( const std::vector< ScGeneralFunction >& rSubtotals )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( pDim->GetOrientation() != DataPilotFieldOrientation_DATA )
        {
            sal_Int32 nCount = rSubtotals.size();
            if( nCount == 1 )
            {
                // count 1: all values are allowed (including NONE and AUTO)
                std::vector<ScGeneralFunction> nTmpFuncs;
                if( rSubtotals[ 0 ] != ScGeneralFunction::NONE )
                {
                    nTmpFuncs.push_back( rSubtotals[ 0 ] );
                }
                pDim->SetSubTotals( nTmpFuncs );
            }
            else if( nCount > 1 )
            {
                // set multiple functions, ignore NONE and AUTO in this case
                ::std::vector< ScGeneralFunction > aSubt;
                for( sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx )
                {
                    ScGeneralFunction eFunc = rSubtotals[ nIdx ];
                    if( (eFunc != ScGeneralFunction::NONE) && (eFunc != ScGeneralFunction::AUTO) )
                    {
                        // do not insert functions twice
                        if( ::std::find( aSubt.begin(), aSubt.end(), eFunc ) == aSubt.end() )
                            aSubt.push_back( eFunc );
                    }
                }
                // set values from vector to ScDPSaveDimension
                pDim->SetSubTotals( aSubt );
            }
        }
        SetDPObject( pDPObj );
    }
}

void ScDataPilotFieldObj::setCurrentPage( const OUString& rPage )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetCurrentPage( &rPage );
        SetDPObject( pDPObj );
    }
}

void ScDataPilotFieldObj::setUseCurrentPage( bool bUse )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
            pDim->SetCurrentPage( nullptr );
        SetDPObject( pDPObj );
    }
}

const DataPilotFieldAutoShowInfo* ScDataPilotFieldObj::getAutoShowInfo()
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim ? pDim->GetAutoShowInfo() : nullptr;
}

void ScDataPilotFieldObj::setAutoShowInfo( const DataPilotFieldAutoShowInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
    return pDim ? pDim->GetLayoutInfo() : nullptr;
}

void ScDataPilotFieldObj::setLayoutInfo( const DataPilotFieldLayoutInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
    return pDim ? pDim->GetReferenceValue() : nullptr;
}

void ScDataPilotFieldObj::setReference( const DataPilotFieldReference* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
    return pDim ? pDim->GetSortInfo() : nullptr;
}

void ScDataPilotFieldObj::setSortInfo( const DataPilotFieldSortInfo* pInfo )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetShowEmpty( bShow );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::getRepeatItemLabels() const
{
    SolarMutexGuard aGuard;
    ScDPSaveDimension* pDim = GetDPDimension();
    return pDim && pDim->GetRepeatItemLabels();
}

void ScDataPilotFieldObj::setRepeatItemLabels( bool bShow )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        pDim->SetRepeatItemLabels( bShow );
        SetDPObject( pDPObj );
    }
}

bool ScDataPilotFieldObj::hasGroupInfo()
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
        if( const ScDPDimensionSaveData* pDimData = pDPObj->GetSaveData()->GetExistingDimensionData() )
            return pDimData->GetNamedGroupDim( pDim->GetName() ) || pDimData->GetNumGroupDim( pDim->GetName() );
    return false;
}

DataPilotFieldGroupInfo ScDataPilotFieldObj::getGroupInfo()
{
    SolarMutexGuard aGuard;
    DataPilotFieldGroupInfo aInfo;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        if( const ScDPDimensionSaveData* pDimData = pDPObj->GetSaveData()->GetExistingDimensionData() )
        {
            if( const ScDPSaveGroupDimension* pGroupDim = pDimData->GetNamedGroupDim( pDim->GetName() ) )
            {
                // grouped by ...
                aInfo.GroupBy = pGroupDim->GetDatePart();

                // find source field
                try
                {
                    Reference< XNameAccess > xFields( mxParent->getDataPilotFields(), UNO_QUERY_THROW );
                    aInfo.SourceField.set( xFields->getByName( pGroupDim->GetSourceDimName() ), UNO_QUERY );
                }
                catch( Exception& )
                {
                }

                ScDataPilotConversion::FillGroupInfo( aInfo, pGroupDim->GetDateInfo() );
                if( pGroupDim->GetDatePart() == 0 )
                {
                    // fill vector of group and group member information
                    ScFieldGroups aGroups;
                    for( sal_Int32 nIdx = 0, nCount = pGroupDim->GetGroupCount(); nIdx < nCount; ++nIdx )
                    {
                        const ScDPSaveGroupItem& rGroup = pGroupDim->GetGroupByIndex( nIdx );
                        ScFieldGroup aGroup;
                        aGroup.maName = rGroup.GetGroupName();
                        for( sal_Int32 nMemIdx = 0, nMemCount = rGroup.GetElementCount(); nMemIdx < nMemCount; ++nMemIdx )
                            if (const OUString* pMem = rGroup.GetElementByIndex(nMemIdx))
                                aGroup.maMembers.push_back( *pMem );
                        aGroups.push_back( aGroup );
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
    ScDPObject* pDPObj = nullptr;
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

                // get dimension savedata or create new if none
                ScDPDimensionSaveData& rDimSaveData = *pSaveData->GetDimensionData();
                rDimSaveData.ReplaceGroupDimension( aGroupDim );
            }
            else    // no source field in group info -> numeric group
            {
                ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();     // created if not there

                ScDPSaveNumGroupDimension* pExisting = pDimData->GetNumGroupDimAcc( getName() );
                if ( pExisting )
                {
                    if (pInfo->GroupBy)
                        pExisting->SetDateInfo(aInfo, pInfo->GroupBy);
                    // modify existing group dimension
                    pExisting->SetGroupInfo( aInfo );
                }
                else if (pInfo->GroupBy)
                {
                    // create new group dimension
                    ScDPSaveNumGroupDimension aNumGroupDim( getName(), aInfo, pInfo->GroupBy );
                    pDimData->AddNumGroupDimension( aNumGroupDim );
                }
                else
                {
                    // create new group dimension
                    ScDPSaveNumGroupDimension aNumGroupDim( getName(), aInfo );
                    pDimData->AddNumGroupDimension( aNumGroupDim );
                }
            }
        }
        else    // null passed as argument
        {
            pSaveData->SetDimensionData( nullptr );
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

// XDataPilotFieldGrouping
Reference< XDataPilotField > SAL_CALL ScDataPilotFieldObj::createNameGroup( const Sequence< OUString >& rItems )
{
    SolarMutexGuard aGuard;

    if( !rItems.hasElements() )
        throw IllegalArgumentException("rItems is empty", static_cast<cppu::OWeakObject*>(this), 0);

    Reference< XMembersAccess > xMembers = GetMembers();
    if (!xMembers.is())
    {
        SAL_WARN("sc.ui", "Cannot access members of the field object.");
        throw RuntimeException("Cannot access members of the field object", static_cast<cppu::OWeakObject*>(this));
    }

    for (const OUString& aEntryName : rItems)
    {
        if (!xMembers->hasByName(aEntryName))
        {
            SAL_WARN("sc.ui", "There is no member with that name: " + aEntryName + ".");
            throw IllegalArgumentException("There is no member with name \"" + aEntryName + "\"", static_cast<cppu::OWeakObject*>(this), 0);
        }
    }

    Reference< XDataPilotField > xRet;
    OUString sNewDim;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        const OUString& aDimName = pDim->GetName();

        ScDPSaveData aSaveData = *pDPObj->GetSaveData();
        ScDPDimensionSaveData* pDimData = aSaveData.GetDimensionData();     // created if not there

        // find original base
        OUString aBaseDimName( aDimName );
        const ScDPSaveGroupDimension* pBaseGroupDim = pDimData->GetNamedGroupDim( aDimName );
        if ( pBaseGroupDim )
        {
            // any entry's SourceDimName is the original base
            aBaseDimName = pBaseGroupDim->GetSourceDimName();
        }

        // find existing group dimension
        // (using the selected dim, can be intermediate group dim)
        ScDPSaveGroupDimension* pGroupDimension = pDimData->GetGroupDimAccForBase( aDimName );

        // remove the selected items from their groups
        // (empty groups are removed, too)
        if ( pGroupDimension )
        {
            for (sal_Int32 nEntry = 0; nEntry < rItems.getLength(); nEntry++)
            {
                const OUString& aEntryName = rItems[nEntry];
                if ( pBaseGroupDim )
                {
                    // for each selected (intermediate) group, remove all its items
                    // (same logic as for adding, below)
                    const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                    if ( pBaseGroup )
                        pBaseGroup->RemoveElementsFromGroups( *pGroupDimension );   // remove all elements
                    else
                        pGroupDimension->RemoveFromGroups( aEntryName );
                }
                else
                    pGroupDimension->RemoveFromGroups( aEntryName );
            }
        }

        std::unique_ptr<ScDPSaveGroupDimension> pNewGroupDim;
        if ( !pGroupDimension )
        {
            // create a new group dimension
            sNewDim = pDimData->CreateGroupDimName( aBaseDimName, *pDPObj, false, nullptr );
            pNewGroupDim.reset(new ScDPSaveGroupDimension( aBaseDimName, sNewDim ));

            pGroupDimension = pNewGroupDim.get();     // make changes to the new dim if none existed

            if ( pBaseGroupDim )
            {
                // If it's a higher-order group dimension, pre-allocate groups for all
                // non-selected original groups, so the individual base members aren't
                // used for automatic groups (this would make the original groups hard
                // to find).
                //! Also do this when removing groups?
                //! Handle this case dynamically with automatic groups?

                long nGroupCount = pBaseGroupDim->GetGroupCount();
                for ( long nGroup = 0; nGroup < nGroupCount; nGroup++ )
                {
                    const ScDPSaveGroupItem& rBaseGroup = pBaseGroupDim->GetGroupByIndex( nGroup );

                    if (!HasString(rItems, rBaseGroup.GetGroupName()))    //! ignore case?
                    {
                        // add an additional group for each item that is not in the selection
                        ScDPSaveGroupItem aGroup( rBaseGroup.GetGroupName() );
                        aGroup.AddElementsFromGroup( rBaseGroup );
                        pGroupDimension->AddGroupItem( aGroup );
                    }
                }
            }
        }
        OUString aGroupDimName = pGroupDimension->GetGroupDimName();

        OUString aGroupName = pGroupDimension->CreateGroupName( ScResId(STR_PIVOT_GROUP) );
        ScDPSaveGroupItem aGroup( aGroupName );
        for (sal_Int32 nEntry = 0; nEntry < rItems.getLength(); nEntry++)
        {
            OUString aEntryName(rItems[nEntry]);
            if ( pBaseGroupDim )
            {
                // for each selected (intermediate) group, add all its items
                const ScDPSaveGroupItem* pBaseGroup = pBaseGroupDim->GetNamedGroup( aEntryName );
                if ( pBaseGroup )
                    aGroup.AddElementsFromGroup( *pBaseGroup );
                else
                    aGroup.AddElement( aEntryName );    // no group found -> automatic group, add the item itself
            }
            else
                aGroup.AddElement( aEntryName );        // no group dimension, add all items directly
        }

        pGroupDimension->AddGroupItem( aGroup );

        if ( pNewGroupDim )
        {
            pDimData->AddGroupDimension( *pNewGroupDim );
            pNewGroupDim.reset();        // AddGroupDimension copies the object
            // don't access pGroupDimension after here
        }
        pGroupDimension = nullptr;

        // set orientation
        ScDPSaveDimension* pSaveDimension = aSaveData.GetDimensionByName( aGroupDimName );
        if ( pSaveDimension->GetOrientation() == DataPilotFieldOrientation_HIDDEN )
        {
            ScDPSaveDimension* pOldDimension = aSaveData.GetDimensionByName( aDimName );
            pSaveDimension->SetOrientation( pOldDimension->GetOrientation() );
            aSaveData.SetPosition( pSaveDimension, 0 ); //! before (immediate) base
        }

        // apply changes
        pDPObj->SetSaveData( aSaveData );
        ScDBDocFunc(*GetDocShell()).RefreshPivotTableGroups(pDPObj);
    }

    // if new grouping field has been created (on first group), return it
    if( !sNewDim.isEmpty() )
    {
        Reference< XNameAccess > xFields(mxParent->getDataPilotFields(), UNO_QUERY);
        if (xFields.is())
        {
            try
            {
                xRet.set(xFields->getByName(sNewDim), UNO_QUERY);
                SAL_WARN_IF(!xRet.is(), "sc.ui", "there is a name, so there should be also a field");
            }
            catch (const container::NoSuchElementException&)
            {
                css::uno::Any anyEx = cppu::getCaughtException();
                SAL_WARN("sc.ui", "Cannot find field with that name: " + sNewDim + ".");
                // Avoid throwing exception that's not specified in the method signature.
                throw css::lang::WrappedTargetRuntimeException(
                        "Cannot find field with name \"" + sNewDim + "\"",
                        static_cast<cppu::OWeakObject*>(this), anyEx );
            }
        }
    }
    return xRet;
}

Reference < XDataPilotField > SAL_CALL ScDataPilotFieldObj::createDateGroup( const DataPilotFieldGroupInfo& rInfo )
{
    SolarMutexGuard aGuard;
    using namespace ::com::sun::star::sheet::DataPilotFieldGroupBy;

    if( !rInfo.HasDateValues )
        throw IllegalArgumentException("HasDateValues is not set", static_cast<cppu::OWeakObject*>(this), 0);
    if( !lclCheckMinMaxStep( rInfo ) )
        throw IllegalArgumentException("min/max/step", static_cast<cppu::OWeakObject*>(this), 0);

    // only a single date flag is allowed
    if( (rInfo.GroupBy == 0) || (rInfo.GroupBy > YEARS) || ((rInfo.GroupBy & (rInfo.GroupBy - 1)) != 0) )
        throw IllegalArgumentException("Invalid GroupBy value: " + OUString::number(rInfo.GroupBy), static_cast<cppu::OWeakObject*>(this), 0);

    // step must be zero, if something else than DAYS is specified
    if( rInfo.Step >= ((rInfo.GroupBy == DAYS) ? 32768.0 : 1.0) )
        throw IllegalArgumentException("Invalid step value: " + OUString::number(rInfo.Step), static_cast<cppu::OWeakObject*>(this), 0);

    OUString aGroupDimName;
    ScDPObject* pDPObj = nullptr;
    if( ScDPSaveDimension* pDim = GetDPDimension( &pDPObj ) )
    {
        ScDPNumGroupInfo aInfo;
        aInfo.mbEnable = true;
        aInfo.mbDateValues = (rInfo.GroupBy == DAYS) && (rInfo.Step >= 1.0);
        aInfo.mbAutoStart = rInfo.HasAutoStart;
        aInfo.mbAutoEnd = rInfo.HasAutoEnd;
        aInfo.mfStart = rInfo.Start;
        aInfo.mfEnd = rInfo.End;
        aInfo.mfStep = std::trunc( rInfo.Step );

        // create a local copy of the entire save data (will be written back below)
        ScDPSaveData aSaveData = *pDPObj->GetSaveData();
        // get or create dimension save data
        ScDPDimensionSaveData& rDimData = *aSaveData.GetDimensionData();

        // find source dimension name
        const OUString& rDimName = pDim->GetName();
        const ScDPSaveGroupDimension* pGroupDim = rDimData.GetNamedGroupDim( rDimName );
        OUString aSrcDimName = pGroupDim ? pGroupDim->GetSourceDimName() : rDimName;

        // find a group dimension for the base field, or get numeric grouping
        pGroupDim = rDimData.GetFirstNamedGroupDim( aSrcDimName );
        const ScDPSaveNumGroupDimension* pNumGroupDim = rDimData.GetNumGroupDim( aSrcDimName );

        // do not group by dates, if named groups or numeric grouping is present
        bool bHasNamedGrouping = pGroupDim && !pGroupDim->GetDateInfo().mbEnable;
        bool bHasNumGrouping = pNumGroupDim && pNumGroupDim->GetInfo().mbEnable && !pNumGroupDim->GetInfo().mbDateValues && !pNumGroupDim->GetDateInfo().mbEnable;
        if( bHasNamedGrouping || bHasNumGrouping )
            throw IllegalArgumentException();

        if( aInfo.mbDateValues )  // create day ranges grouping
        {
            // first remove all named group dimensions
            while( pGroupDim )
            {
                OUString aGroupDimName2 = pGroupDim->GetGroupDimName();
                // find next group dimension before deleting this group
                pGroupDim = rDimData.GetNextNamedGroupDim( aGroupDimName2 );
                // remove from dimension save data
                rDimData.RemoveGroupDimension( aGroupDimName2 );
                // also remove save data settings for the dimension that no longer exists
                aSaveData.RemoveDimensionByName( aGroupDimName2 );
            }
            // create or replace the number grouping dimension
            ScDPSaveNumGroupDimension aNumGroupDim( aSrcDimName, aInfo );
            rDimData.ReplaceNumGroupDimension( aNumGroupDim );
        }
        else    // create date grouping
        {
            // collect all existing date flags
            sal_Int32 nDateParts = rDimData.CollectDateParts( aSrcDimName );
            if( nDateParts == 0 )
            {
                // insert numeric group dimension, if no date groups exist yet (or replace day range grouping)
                ScDPSaveNumGroupDimension aNumGroupDim( aSrcDimName, aInfo, rInfo.GroupBy );
                rDimData.ReplaceNumGroupDimension( aNumGroupDim );
            }
            else if( (nDateParts & rInfo.GroupBy) == 0 )    // do nothing if date field exists already
            {
                // create new named group dimension for additional date groups
                aGroupDimName = rDimData.CreateDateGroupDimName( rInfo.GroupBy, *pDPObj, true, nullptr );
                ScDPSaveGroupDimension aGroupDim( aSrcDimName, aGroupDimName, aInfo, rInfo.GroupBy );
                rDimData.AddGroupDimension( aGroupDim );

                // set orientation of new named group dimension
                ScDPSaveDimension& rSaveDim = *aSaveData.GetDimensionByName( aGroupDimName );
                if( rSaveDim.GetOrientation() == DataPilotFieldOrientation_HIDDEN )
                {
                    ScDPSaveDimension& rOldDim = *aSaveData.GetDimensionByName( aSrcDimName );
                    rSaveDim.SetOrientation( rOldDim.GetOrientation() );
                    aSaveData.SetPosition( &rSaveDim, 0 );  //! before (immediate) base
                }
            }
        }

        // apply changes
        pDPObj->SetSaveData( aSaveData );
        ScDBDocFunc(*GetDocShell()).RefreshPivotTableGroups(pDPObj);
    }

    // return the UNO object of the new dimension, after writing back saved data
    Reference< XDataPilotField > xRet;
    if( !aGroupDimName.isEmpty() )
        try
        {
           Reference< XNameAccess > xFields( mxParent->getDataPilotFields(), UNO_QUERY_THROW );
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
    // allow empty value to create a new group
    if( !rElement.hasValue() )
        return true;

    // try to extract a simple sequence of strings
    Sequence< OUString > aSeq;
    if( rElement >>= aSeq )
    {
        if( aSeq.hasElements() )
            rMembers.insert( rMembers.end(), aSeq.begin(), aSeq.end() );
        return true;
    }

    // try to use XIndexAccess providing objects that support XNamed
    Reference< XIndexAccess > xItemsIA( rElement, UNO_QUERY );
    if( xItemsIA.is() )
    {
        for( sal_Int32 nIdx = 0, nCount = xItemsIA->getCount(); nIdx < nCount; ++nIdx )
        {
            try // getByIndex() should not throw, but we cannot be sure
            {
                Reference< XNamed > xItemName( xItemsIA->getByIndex( nIdx ), UNO_QUERY_THROW );
                rMembers.push_back( xItemName->getName() );
            }
            catch( Exception& )
            {
                // ignore exceptions, go ahead with next element in the array
            }
        }
        return true;
    }

    // nothing valid inside the Any -> return false
    return false;
}

} // namespace

ScDataPilotFieldGroupsObj::ScDataPilotFieldGroupsObj( const ScFieldGroups& rGroups ) :
    maGroups( rGroups )
{
}

ScDataPilotFieldGroupsObj::~ScDataPilotFieldGroupsObj()
{
}

// XNameAccess

Any SAL_CALL ScDataPilotFieldGroupsObj::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    if( implFindByName( rName ) == maGroups.end() )
        throw NoSuchElementException();
    return Any( Reference< XNameAccess >( new ScDataPilotFieldGroupObj( *this, rName ) ) );
}

Sequence< OUString > SAL_CALL ScDataPilotFieldGroupsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aSeq;
    if( !maGroups.empty() )
    {
        aSeq.realloc( static_cast< sal_Int32 >( maGroups.size() ) );
        OUString* pName = aSeq.getArray();
        for( const auto& rGroup : maGroups )
        {
            *pName = rGroup.maName;
            ++pName;
        }
    }
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    return implFindByName( rName ) != maGroups.end();
}

// XNameReplace

void SAL_CALL ScDataPilotFieldGroupsObj::replaceByName( const OUString& rName, const Any& rElement )
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw NoSuchElementException("Name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));

    // read all item names provided by the passed object
    ScFieldGroupMembers aMembers;
    if( !lclExtractGroupMembers( aMembers, rElement ) )
        throw IllegalArgumentException("Invalid element object", static_cast<cppu::OWeakObject*>(this), 0);

    // copy and forget, faster than vector assignment
    aIt->maMembers.swap( aMembers );
}

// XNameContainer

void SAL_CALL ScDataPilotFieldGroupsObj::insertByName( const OUString& rName, const Any& rElement )
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt != maGroups.end() )
        throw ElementExistException("Name \"" + rName + "\" already exists", static_cast<cppu::OWeakObject*>(this));

    // read all item names provided by the passed object
    ScFieldGroupMembers aMembers;
    if( !lclExtractGroupMembers( aMembers, rElement ) )
        throw IllegalArgumentException("Invalid element object", static_cast<cppu::OWeakObject*>(this), 0);

    // create the new entry if no error has been occurred
    maGroups.emplace_back();
    ScFieldGroup& rGroup = maGroups.back();
    rGroup.maName = rName;
    rGroup.maMembers.swap( aMembers );
}

void SAL_CALL ScDataPilotFieldGroupsObj::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);

    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw NoSuchElementException("Name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));

    maGroups.erase( aIt );
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldGroupsObj::getCount()
{
    SolarMutexGuard aGuard;
    return static_cast< sal_Int32 >( maGroups.size() );
}

Any SAL_CALL ScDataPilotFieldGroupsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    if ((nIndex < 0) || (nIndex >= static_cast< sal_Int32 >( maGroups.size() )))
        throw IndexOutOfBoundsException();
    return Any( Reference< XNameAccess >( new ScDataPilotFieldGroupObj( *this, maGroups[ nIndex ].maName ) ) );
}

// XEnumerationAccess

Reference<XEnumeration> SAL_CALL ScDataPilotFieldGroupsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration( this, "com.sun.star.sheet.DataPilotFieldGroupsEnumeration" );
}

// XElementAccess

uno::Type SAL_CALL ScDataPilotFieldGroupsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<XNameAccess>::get();
}

sal_Bool SAL_CALL ScDataPilotFieldGroupsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return !maGroups.empty();
}

// implementation

ScFieldGroup& ScDataPilotFieldGroupsObj::getFieldGroup( const OUString& rName )
{
    SolarMutexGuard aGuard;
    ScFieldGroups::iterator aIt = implFindByName( rName );
    if( aIt == maGroups.end() )
        throw RuntimeException("Field Group with name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    return *aIt;
}

void ScDataPilotFieldGroupsObj::renameFieldGroup( const OUString& rOldName, const OUString& rNewName )
{
    SolarMutexGuard aGuard;
    ScFieldGroups::iterator aOldIt = implFindByName( rOldName );
    ScFieldGroups::iterator aNewIt = implFindByName( rNewName );
    if( aOldIt == maGroups.end() )
        throw RuntimeException("Field Group with name \"" + rOldName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    // new name must not exist yet
    if( (aNewIt != maGroups.end()) && (aNewIt != aOldIt) )
        throw RuntimeException("Field Group with name \"" + rOldName + "\" already exists", static_cast<cppu::OWeakObject*>(this));
    aOldIt->maName = rNewName;
}

ScFieldGroups::iterator ScDataPilotFieldGroupsObj::implFindByName( const OUString& rName )
{
    return std::find_if(maGroups.begin(), maGroups.end(),
        [&rName](const ScFieldGroup& rGroup) { return rGroup.maName == rName; });
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

} // namespace

ScDataPilotFieldGroupObj::ScDataPilotFieldGroupObj( ScDataPilotFieldGroupsObj& rParent, const OUString& rGroupName ) :
    mxParent( &rParent ),
    maGroupName( rGroupName )
{
}

ScDataPilotFieldGroupObj::~ScDataPilotFieldGroupObj()
{
}

// XNameAccess

Any SAL_CALL ScDataPilotFieldGroupObj::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    if( aIt == rMembers.end() )
        throw NoSuchElementException("Name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    return Any( Reference< XNamed >( new ScDataPilotFieldGroupItemObj( *this, *aIt ) ) );
}

Sequence< OUString > SAL_CALL ScDataPilotFieldGroupObj::getElementNames()
{
    SolarMutexGuard aGuard;
    return ::comphelper::containerToSequence( mxParent->getFieldGroup( maGroupName ).maMembers );
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasByName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    return ::std::find( rMembers.begin(), rMembers.end(), rName ) != rMembers.end();
}

// XNameReplace

void SAL_CALL ScDataPilotFieldGroupObj::replaceByName( const OUString& rName, const Any& rElement )
{
    SolarMutexGuard aGuard;

    // it should be possible to quickly rename an item -> accept string or XNamed
    OUString aNewName = lclExtractMember( rElement );
    if( rName.isEmpty() || aNewName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);
    if( rName == aNewName )
        return;

    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aOldIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    ScFieldGroupMembers::iterator aNewIt = ::std::find( rMembers.begin(), rMembers.end(), aNewName );
    if( aOldIt == rMembers.end() )
        throw NoSuchElementException("Name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    if( aNewIt != rMembers.end() )
        throw IllegalArgumentException("Name \"" + rName + "\" already exists", static_cast<cppu::OWeakObject*>(this), 0);
    *aOldIt = aNewName;
}

// XNameContainer

void SAL_CALL ScDataPilotFieldGroupObj::insertByName( const OUString& rName, const Any& /*rElement*/ )
{
    SolarMutexGuard aGuard;

    // we will ignore the passed element and just try to insert the name
    if( rName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);

    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    if( aIt != rMembers.end() )
        throw IllegalArgumentException("Name \"" + rName + "\" already exists", static_cast<cppu::OWeakObject*>(this), 0);
    rMembers.push_back( rName );
}

void SAL_CALL ScDataPilotFieldGroupObj::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    if( rName.isEmpty() )
        throw IllegalArgumentException("Name is empty", static_cast<cppu::OWeakObject*>(this), 0);
    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    ScFieldGroupMembers::iterator aIt = ::std::find( rMembers.begin(), rMembers.end(), rName );
    if( aIt == rMembers.end() )
        throw NoSuchElementException("Name \"" + rName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    rMembers.erase( aIt );
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotFieldGroupObj::getCount()
{
    SolarMutexGuard aGuard;
    return static_cast< sal_Int32 >( mxParent->getFieldGroup( maGroupName ).maMembers.size() );
}

Any SAL_CALL ScDataPilotFieldGroupObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    ScFieldGroupMembers& rMembers = mxParent->getFieldGroup( maGroupName ).maMembers;
    if ((nIndex < 0) || (nIndex >= static_cast< sal_Int32 >( rMembers.size() )))
        throw IndexOutOfBoundsException();
    return Any( Reference< XNamed >( new ScDataPilotFieldGroupItemObj( *this, rMembers[ nIndex ] ) ) );
}

// XEnumerationAccess

Reference< XEnumeration > SAL_CALL ScDataPilotFieldGroupObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration( this, "com.sun.star.sheet.DataPilotFieldGroupEnumeration" );
}

// XElementAccess

uno::Type SAL_CALL ScDataPilotFieldGroupObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<XNamed>::get();
}

sal_Bool SAL_CALL ScDataPilotFieldGroupObj::hasElements()
{
    SolarMutexGuard aGuard;
    return !mxParent->getFieldGroup( maGroupName ).maMembers.empty();
}

// XNamed

OUString SAL_CALL ScDataPilotFieldGroupObj::getName()
{
    SolarMutexGuard aGuard;
    return maGroupName;
}

void SAL_CALL ScDataPilotFieldGroupObj::setName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    mxParent->renameFieldGroup( maGroupName, rName );
    // if call to renameFieldGroup() did not throw, remember the new name
    maGroupName = rName;
}

ScDataPilotFieldGroupItemObj::ScDataPilotFieldGroupItemObj( ScDataPilotFieldGroupObj& rParent, const OUString& rName ) :
    mxParent( &rParent ),
    maName( rName )
{
}

ScDataPilotFieldGroupItemObj::~ScDataPilotFieldGroupItemObj()
{
}

// XNamed

OUString SAL_CALL ScDataPilotFieldGroupItemObj::getName()
{
    SolarMutexGuard aGuard;
    return maName;
}

void SAL_CALL ScDataPilotFieldGroupItemObj::setName( const OUString& rName )
{
    SolarMutexGuard aGuard;
    mxParent->replaceByName( maName, Any( rName ) );
    // if call to replaceByName() did not throw, remember the new name
    maName = rName;
}

ScDataPilotItemsObj::ScDataPilotItemsObj( ScDataPilotDescriptorBase& rParent, const ScFieldIdentifier& rFieldId ) :
    ScDataPilotChildObjBase( rParent, rFieldId )
{
}

ScDataPilotItemsObj::~ScDataPilotItemsObj()
{
}

// XDataPilotItems

ScDataPilotItemObj* ScDataPilotItemsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
    return ((0 <= nIndex) && (nIndex < GetMemberCount())) ?
        new ScDataPilotItemObj( *mxParent, maFieldId, nIndex ) : nullptr;
}

// XNameAccess

Any SAL_CALL ScDataPilotItemsObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    Reference<XNameAccess> xMembers = GetMembers();
    if (xMembers.is())
    {
        Reference<XIndexAccess> xMembersIndex(new ScNameToIndexAccess( xMembers ));
        sal_Int32 nCount = xMembersIndex->getCount();
        sal_Int32 nItem = 0;
        while (nItem < nCount)
        {
            Reference<XNamed> xMember(xMembersIndex->getByIndex(nItem), UNO_QUERY);
            if (xMember.is() && (aName == xMember->getName()))
            {
                return Any( Reference< XPropertySet >( GetObjectByIndex_Impl( nItem ) ) );
            }
            ++nItem;
        }
        throw NoSuchElementException("Name \"" + aName + "\" not found", static_cast<cppu::OWeakObject*>(this));
    }
    return Any();
}

Sequence<OUString> SAL_CALL ScDataPilotItemsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aSeq;
    if( ScDPObject* pDPObj = GetDPObject() )
        pDPObj->GetMemberNames( lcl_GetObjectIndex( pDPObj, maFieldId ), aSeq );
    return aSeq;
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    bool bFound = false;
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
                bFound = true;
            else
                nItem++;
        }
    }
    return bFound;
}

// XEnumerationAccess

Reference<XEnumeration> SAL_CALL ScDataPilotItemsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.DataPilotItemsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScDataPilotItemsObj::getCount()
{
    SolarMutexGuard aGuard;
    return GetMemberCount();
}

Any SAL_CALL ScDataPilotItemsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    Reference< XPropertySet > xItem( GetObjectByIndex_Impl( nIndex ) );
    if (!xItem.is())
        throw IndexOutOfBoundsException();
    return Any( xItem );
}

uno::Type SAL_CALL ScDataPilotItemsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL ScDataPilotItemsObj::hasElements()
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

                            // XNamed
OUString SAL_CALL ScDataPilotItemObj::getName()
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
{
}

                            // XPropertySet
Reference< XPropertySetInfo >
                            SAL_CALL ScDataPilotItemObj::getPropertySetInfo(  )
{
    SolarMutexGuard aGuard;
    static Reference<XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDataPilotItemObj::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    SolarMutexGuard aGuard;
    ScDPObject* pDPObj = nullptr;
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
                        if ( !( aValue >>= nNewPos ) || nNewPos < 0 || nNewPos >= nCount )
                            throw IllegalArgumentException();

                        pDim->SetMemberPosition( sName, nNewPos );
                        // get new effective index (depends on sorting mode, which isn't modified)
                        bGetNewIndex = true;

                    }
                    SetDPObject( pDPObj );

                    if ( bGetNewIndex )     // after SetDPObject, get the new index
                    {
                        Sequence< OUString > aItemNames = xMembers->getElementNames();
                        sal_Int32 nItemCount = aItemNames.getLength();
                        for (sal_Int32 nItem=0; nItem<nItemCount; ++nItem)
                            if (aItemNames[nItem] == sName)
                                mnIndex = nItem;
                    }
                }
            }
        }
    }
}

Any SAL_CALL ScDataPilotItemObj::getPropertyValue( const OUString& aPropertyName )
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
                        aRet <<= pMember->GetShowDetails();
                    }
                    else
                    {
                        Reference< XPropertySet > xMemberProps( xMember, UNO_QUERY );
                        if( xMemberProps.is() )
                            aRet = xMemberProps->getPropertyValue( SC_UNO_DP_SHOWDETAILS );
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
                            aRet <<= !cppu::any2bool( xMemberProps->getPropertyValue( SC_UNO_DP_ISVISIBLE ) );
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
{
}

void SAL_CALL ScDataPilotItemObj::removePropertyChangeListener(
        const OUString& /* aPropertyName */, const Reference< XPropertyChangeListener >& /* aListener */ )
{
}

void SAL_CALL ScDataPilotItemObj::addVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
{
}

void SAL_CALL ScDataPilotItemObj::removeVetoableChangeListener(
        const OUString& /* PropertyName */, const Reference< XVetoableChangeListener >& /* aListener */ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
