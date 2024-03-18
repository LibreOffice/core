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

#pragma once

#include "global.hxx"
#include "queryparam.hxx"
#include "subtotalparam.hxx"

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor2.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor3.hpp>
#include <com/sun/star/sheet/XConsolidationDescriptor.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>

#include <vector>

namespace com::sun::star::sheet { struct TableFilterField2; }
namespace com::sun::star::sheet { struct TableFilterField3; }
namespace com::sun::star::sheet { struct TableFilterField; }

class ScDBData;
class ScDocShell;

class ScSubTotalFieldObj;
class ScDatabaseRangeObj;
class ScDataPilotDescriptorBase;

struct ScSortParam;

class ScDataUnoConversion
{
public:
    static css::sheet::GeneralFunction SubTotalToGeneral( ScSubTotalFunc eSubTotal );
};

//  ImportDescriptor is not available as Uno-object any longer, only Property-Sequence

class ScImportDescriptor
{
public:
    static void FillImportParam(
                    ScImportParam& rParam,
                    const css::uno::Sequence<css::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    css::uno::Sequence<css::beans::PropertyValue>& rSeq,
                    const ScImportParam& rParam );
    static tools::Long GetPropertyCount() { return 4; }
};

//  SortDescriptor is not available as Uno-object any longer, only Property-Sequence

class ScSortDescriptor
{
public:
    static void FillSortParam(
                    ScSortParam& rParam,
                    const css::uno::Sequence<css::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    css::uno::Sequence<css::beans::PropertyValue>& rSeq,
                    const ScSortParam& rParam );
    //! SortAscending needs to get out of the SheetSortDescriptor service description
    static tools::Long GetPropertyCount()
    {
        return 9;       // TableSortDescriptor and SheetSortDescriptor
    }

};

//  ScSubTotalDescriptorBase - base class for SubTotalDescriptor stand alone and in DB area (context?)

//  to uno, both look the same

class ScSubTotalDescriptorBase : public cppu::WeakImplHelper<
                                        css::sheet::XSubTotalDescriptor,
                                        css::container::XEnumerationAccess,
                                        css::container::XIndexAccess,
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;

    rtl::Reference<ScSubTotalFieldObj> GetObjectByIndex_Impl(sal_uInt16 nIndex);

public:
                            ScSubTotalDescriptorBase();
    virtual                 ~ScSubTotalDescriptorBase() override;

                            // in derived classes:
                            // (Fields are within the range)
    virtual void            GetData( ScSubTotalParam& rParam ) const = 0;
    virtual void            PutData( const ScSubTotalParam& rParam ) = 0;

                            // XSubTotalDescriptor
    virtual void SAL_CALL   addNew( const css::uno::Sequence< css::sheet::SubTotalColumn >& aSubTotalColumns,
                                sal_Int32 nGroupColumn ) override;
    virtual void SAL_CALL   clear() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

//  ScSubTotalDescriptor - dummy container to use with XImportTarget

class ScSubTotalDescriptor final : public ScSubTotalDescriptorBase
{
private:
    ScSubTotalParam         aStoredParam;

public:
                            ScSubTotalDescriptor();
    virtual                 ~ScSubTotalDescriptor() override;

                            // from ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const override;
    virtual void            PutData( const ScSubTotalParam& rParam ) override;

                            // external access:
    void                    SetParam( const ScSubTotalParam& rNew );
};

//  ScRangeSubTotalDescriptor - SubTotalDescriptor of a data base area

class ScRangeSubTotalDescriptor final : public ScSubTotalDescriptorBase
{
private:
    rtl::Reference<ScDatabaseRangeObj>  mxParent;

public:
                            ScRangeSubTotalDescriptor(ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeSubTotalDescriptor() override;

                            // from ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const override;
    virtual void            PutData( const ScSubTotalParam& rParam ) override;
};

class ScSubTotalFieldObj final : public cppu::WeakImplHelper<
                                css::sheet::XSubTotalField,
                                css::lang::XServiceInfo >
{
private:
    rtl::Reference<ScSubTotalDescriptorBase> xParent;
    sal_uInt16                      nPos;

public:
                            ScSubTotalFieldObj( ScSubTotalDescriptorBase* pDesc, sal_uInt16 nP );
    virtual                 ~ScSubTotalFieldObj() override;

                            // XSubTotalField
    virtual sal_Int32 SAL_CALL getGroupColumn() override;
    virtual void SAL_CALL   setGroupColumn( sal_Int32 nGroupColumn ) override;
    virtual css::uno::Sequence< css::sheet::SubTotalColumn > SAL_CALL
                            getSubTotalColumns() override;
    virtual void SAL_CALL   setSubTotalColumns( const css::uno::Sequence<
                                css::sheet::SubTotalColumn >& aSubTotalColumns ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScConsolidationDescriptor final : public cppu::WeakImplHelper<
                                        css::sheet::XConsolidationDescriptor,
                                        css::lang::XServiceInfo >
{
private:
    ScConsolidateParam      aParam;

public:
                            ScConsolidationDescriptor();
    virtual                 ~ScConsolidationDescriptor() override;

    void                    SetParam( const ScConsolidateParam& rNew );
    const ScConsolidateParam& GetParam() const  { return aParam; }

                            // XConsolidationDescriptor
    virtual css::sheet::GeneralFunction SAL_CALL getFunction() override;
    virtual void SAL_CALL   setFunction( css::sheet::GeneralFunction nFunction ) override;
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getSources(  ) override;
    virtual void SAL_CALL   setSources( const css::uno::Sequence< css::table::CellRangeAddress >& aSources ) override;
    virtual css::table::CellAddress SAL_CALL getStartOutputPosition() override;
    virtual void SAL_CALL   setStartOutputPosition(
                                const css::table::CellAddress& aStartOutputPosition ) override;
    virtual sal_Bool SAL_CALL getUseColumnHeaders() override;
    virtual void SAL_CALL   setUseColumnHeaders( sal_Bool bUseColumnHeaders ) override;
    virtual sal_Bool SAL_CALL getUseRowHeaders() override;
    virtual void SAL_CALL   setUseRowHeaders( sal_Bool bUseRowHeaders ) override;
    virtual sal_Bool SAL_CALL getInsertLinks() override;
    virtual void SAL_CALL   setInsertLinks( sal_Bool bInsertLinks ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

//  ScFilterDescriptorBase - base class for FilterDescriptor
//                           stand alone, in a DB area (or context?) and in the DataPilot

//  to uno, all three look the same

class ScFilterDescriptorBase : public cppu::WeakImplHelper<
                                    css::sheet::XSheetFilterDescriptor,
                                    css::sheet::XSheetFilterDescriptor2,
                                    css::sheet::XSheetFilterDescriptor3,
                                    css::beans::XPropertySet,
                                    css::lang::XServiceInfo >,
                               public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocSh;

public:
                            ScFilterDescriptorBase(ScDocShell* pDocShell);
    virtual                 ~ScFilterDescriptorBase() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // in the derived classes(?):
                            // (nField[] here within the area)
    virtual void            GetData( ScQueryParam& rParam ) const = 0;
    virtual void            PutData( const ScQueryParam& rParam ) = 0;

                            // XSheetFilterDescriptor
    virtual css::uno::Sequence< css::sheet::TableFilterField > SAL_CALL
                            getFilterFields() override;
    virtual void SAL_CALL   setFilterFields( const css::uno::Sequence<
                                css::sheet::TableFilterField >& aFilterFields ) override;

                            // XSheetFilterDescriptor2
    virtual css::uno::Sequence< css::sheet::TableFilterField2 > SAL_CALL
                            getFilterFields2() override;
    virtual void SAL_CALL   setFilterFields2( const css::uno::Sequence<
                                css::sheet::TableFilterField2 >& aFilterFields ) override;

                            // XSheetFilterDescriptor3
    virtual css::uno::Sequence< css::sheet::TableFilterField3 > SAL_CALL
                            getFilterFields3() override;
    virtual void SAL_CALL   setFilterFields3( const css::uno::Sequence<
                                css::sheet::TableFilterField3 >& aFilterFields ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

//  ScFilterDescriptor - dummy container to use with XFilterable

class ScFilterDescriptor final : public ScFilterDescriptorBase
{
private:
    ScQueryParam            aStoredParam;       // nField[] here within the area

public:
                            ScFilterDescriptor(ScDocShell* pDocSh);
    virtual                 ~ScFilterDescriptor() override;

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;

                            // external access:
    void                    SetParam( const ScQueryParam& rNew );
    const ScQueryParam&     GetParam() const    { return aStoredParam; }
};

//  ScRangeFilterDescriptor - FilterDescriptor of a data base area

class ScRangeFilterDescriptor final : public ScFilterDescriptorBase
{
private:
    rtl::Reference<ScDatabaseRangeObj>  mxParent;

public:
                            ScRangeFilterDescriptor(ScDocShell* pDocSh, ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeFilterDescriptor() override;

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;
};

//  ScDataPilotFilterDescriptor - FilterDescriptor of a DataPilotDescriptors

class ScDataPilotFilterDescriptor final : public ScFilterDescriptorBase
{
private:
    rtl::Reference<ScDataPilotDescriptorBase>  mxParent;

public:
                            ScDataPilotFilterDescriptor(ScDocShell* pDocSh, ScDataPilotDescriptorBase* pPar);
    virtual                 ~ScDataPilotFilterDescriptor() override;

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;
};

class ScDatabaseRangeObj final : public cppu::WeakImplHelper<
                                css::sheet::XDatabaseRange,
                                css::util::XRefreshable,
                                css::container::XNamed,
                                css::sheet::XCellRangeReferrer,
                                css::beans::XPropertySet,
                                css::lang::XServiceInfo >,
                           public SfxListener
{
private:
    ScDocShell*             pDocShell;
    OUString                aName;
    SfxItemPropertySet      aPropSet;
    std::vector< css::uno::Reference< css::util::XRefreshListener > >
                            aRefreshListeners;
    bool                    bIsUnnamed;
    SCTAB                   aTab;

private:
    ScDBData*               GetDBData_Impl() const;
    void                    Refreshed_Impl();

public:
    ScDatabaseRangeObj(ScDocShell* pDocSh, OUString aNm);
    SC_DLLPUBLIC ScDatabaseRangeObj(ScDocShell* pDocSh, const SCTAB nTab);
    virtual ~ScDatabaseRangeObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // nField[] here within the area:
    void                    GetQueryParam(ScQueryParam& rQueryParam) const;
    void                    SetQueryParam(const ScQueryParam& rQueryParam);
    void                    GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
    void                    SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XDatabaseRange
    virtual css::table::CellRangeAddress SAL_CALL getDataArea() override;
    virtual void SAL_CALL   setDataArea( const css::table::CellRangeAddress& aDataArea ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getSortDescriptor() override;
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() override;
    virtual css::uno::Reference< css::sheet::XSubTotalDescriptor > SAL_CALL
                            getSubTotalDescriptor() override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getImportDescriptor() override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference<
                                    css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference<
                                    css::util::XRefreshListener >& l ) override;

                            // XCellRangeReferrer
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getReferredCells() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScDatabaseRangesObj final : public cppu::WeakImplHelper<
                                css::sheet::XDatabaseRanges,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScDatabaseRangeObj> GetObjectByIndex_Impl(size_t nIndex);
    rtl::Reference<ScDatabaseRangeObj> GetObjectByName_Impl(const OUString& aName);

public:
                            ScDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScDatabaseRangesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XDatabaseRanges
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const css::table::CellRangeAddress& aRange ) override;
    virtual void SAL_CALL   removeByName( const OUString& aName ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScUnnamedDatabaseRangesObj final : public cppu::WeakImplHelper<
                                css::sheet::XUnnamedDatabaseRanges>,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
                            ScUnnamedDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScUnnamedDatabaseRangesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XUnnamedDatabaseRanges
    virtual void SAL_CALL setByTable( const css::table::CellRangeAddress& aRange ) override;
    virtual css::uno::Any SAL_CALL getByTable( sal_Int32 nTab ) override;
    virtual sal_Bool SAL_CALL hasByTable( sal_Int32 nTab ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
