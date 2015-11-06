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

#ifndef INCLUDED_SC_INC_DATAUNO_HXX
#define INCLUDED_SC_INC_DATAUNO_HXX

#include "global.hxx"
#include "queryparam.hxx"
#include "subtotalparam.hxx"

#include <com/sun/star/sheet/TableFilterField.hpp>
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
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>

#include <vector>

class ScDBData;
class ScDocShell;

class ScSubTotalFieldObj;
class ScDatabaseRangeObj;
class ScDataPilotDescriptorBase;

struct ScSortParam;

typedef std::vector< css::uno::Reference< css::util::XRefreshListener > > XDBRefreshListenerArr_Impl;

class ScDataUnoConversion
{
public:
    static css::sheet::GeneralFunction SubTotalToGeneral( ScSubTotalFunc eSubTotal );
};

//  ImportDescriptor is not available as Uno-Objekt any longer, only Property-Sequence

class ScImportDescriptor
{
public:
    static void FillImportParam(
                    ScImportParam& rParam,
                    const css::uno::Sequence<css::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    css::uno::Sequence<css::beans::PropertyValue>& rSeq,
                    const ScImportParam& rParam );
    static long GetPropertyCount() { return 4; }
};

//  SortDescriptor is not available as Uno-Objekt any longer, only Property-Sequence

class ScSortDescriptor
{
public:
    static void FillSortParam(
                    ScSortParam& rParam,
                    const css::uno::Sequence<css::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    css::uno::Sequence<css::beans::PropertyValue>& rSeq,
                    const ScSortParam& rParam );
    //! SortAscending muss aus der SheetSortDescriptor service-Beschreibung raus
    static long GetPropertyCount()
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
                                        css::lang::XUnoTunnel,
                                        css::lang::XServiceInfo >
{
private:
    SfxItemPropertySet      aPropSet;

    ScSubTotalFieldObj*     GetObjectByIndex_Impl(sal_uInt16 nIndex);

public:
                            ScSubTotalDescriptorBase();
    virtual                 ~ScSubTotalDescriptorBase();

                            // in derived classes:
                            // (Fields are within the range)
    virtual void            GetData( ScSubTotalParam& rParam ) const = 0;
    virtual void            PutData( const ScSubTotalParam& rParam ) = 0;

                            // XSubTotalDescriptor
    virtual void SAL_CALL   addNew( const css::uno::Sequence< css::sheet::SubTotalColumn >& aSubTotalColumns,
                                sal_Int32 nGroupColumn )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   clear() throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScSubTotalDescriptorBase* getImplementation(const css::uno::Reference<css::sheet::XSubTotalDescriptor>& rObj);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//  ScSubTotalDescriptor - dummy container to use with XImportTarget

class ScSubTotalDescriptor : public ScSubTotalDescriptorBase
{
private:
    ScSubTotalParam         aStoredParam;

public:
                            ScSubTotalDescriptor();
    virtual                 ~ScSubTotalDescriptor();

                            // from ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const override;
    virtual void            PutData( const ScSubTotalParam& rParam ) override;

                            // external access:
    void                    SetParam( const ScSubTotalParam& rNew );
};

//  ScRangeSubTotalDescriptor - SubTotalDescriptor of a data base area

class ScRangeSubTotalDescriptor : public ScSubTotalDescriptorBase
{
private:
    ScDatabaseRangeObj*     pParent;

public:
                            ScRangeSubTotalDescriptor(ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeSubTotalDescriptor();

                            // from ScSubTotalDescriptorBase:
    virtual void            GetData( ScSubTotalParam& rParam ) const override;
    virtual void            PutData( const ScSubTotalParam& rParam ) override;
};

class ScSubTotalFieldObj : public cppu::WeakImplHelper<
                                css::sheet::XSubTotalField,
                                css::lang::XServiceInfo >
{
private:
    css::uno::Reference<css::sheet::XSubTotalDescriptor> xRef;
    ScSubTotalDescriptorBase&       rParent;
    sal_uInt16                      nPos;

public:
                            ScSubTotalFieldObj( ScSubTotalDescriptorBase* pDesc, sal_uInt16 nP );
    virtual                 ~ScSubTotalFieldObj();

                            // XSubTotalField
    virtual sal_Int32 SAL_CALL getGroupColumn() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setGroupColumn( sal_Int32 nGroupColumn )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::sheet::SubTotalColumn > SAL_CALL
                            getSubTotalColumns() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSubTotalColumns( const css::uno::Sequence<
                                css::sheet::SubTotalColumn >& aSubTotalColumns )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScConsolidationDescriptor : public cppu::WeakImplHelper<
                                        css::sheet::XConsolidationDescriptor,
                                        css::lang::XServiceInfo >
{
private:
    ScConsolidateParam      aParam;

public:
                            ScConsolidationDescriptor();
    virtual                 ~ScConsolidationDescriptor();

    void                    SetParam( const ScConsolidateParam& rNew );
    const ScConsolidateParam& GetParam() const  { return aParam; }

                            // XConsolidationDescriptor
    virtual css::sheet::GeneralFunction SAL_CALL getFunction()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFunction( css::sheet::GeneralFunction nFunction )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getSources(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSources( const css::uno::Sequence< css::table::CellRangeAddress >& aSources )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellAddress SAL_CALL getStartOutputPosition()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setStartOutputPosition(
                                const css::table::CellAddress& aStartOutputPosition )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getUseColumnHeaders() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setUseColumnHeaders( sal_Bool bUseColumnHeaders )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getUseRowHeaders() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setUseRowHeaders( sal_Bool bUseRowHeaders )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getInsertLinks() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setInsertLinks( sal_Bool bInsertLinks )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
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
    virtual                 ~ScFilterDescriptorBase();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // in the derived classes(?):
                            // (nField[] here within the area)
    virtual void            GetData( ScQueryParam& rParam ) const = 0;
    virtual void            PutData( const ScQueryParam& rParam ) = 0;

                            // XSheetFilterDescriptor
    virtual css::uno::Sequence< css::sheet::TableFilterField > SAL_CALL
                            getFilterFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFilterFields( const css::uno::Sequence<
                                css::sheet::TableFilterField >& aFilterFields )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetFilterDescriptor2
    virtual css::uno::Sequence< css::sheet::TableFilterField2 > SAL_CALL
                            getFilterFields2() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFilterFields2( const css::uno::Sequence<
                                css::sheet::TableFilterField2 >& aFilterFields )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetFilterDescriptor3
    virtual css::uno::Sequence< css::sheet::TableFilterField3 > SAL_CALL
                            getFilterFields3() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFilterFields3( const css::uno::Sequence<
                                css::sheet::TableFilterField3 >& aFilterFields )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//  ScFilterDescriptor - dummy container to use with XFilterable

class ScFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScQueryParam            aStoredParam;       // nField[] here within the area

public:
                            ScFilterDescriptor(ScDocShell* pDocSh);
    virtual                 ~ScFilterDescriptor();

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;

                            // external access:
    void                    SetParam( const ScQueryParam& rNew );
    const ScQueryParam&     GetParam() const    { return aStoredParam; }
};

//  ScRangeFilterDescriptor - FilterDescriptor of a data base area

class ScRangeFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScDatabaseRangeObj*     pParent;

public:
                            ScRangeFilterDescriptor(ScDocShell* pDocSh, ScDatabaseRangeObj* pPar);
    virtual                 ~ScRangeFilterDescriptor();

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;
};

//  ScDataPilotFilterDescriptor - FilterDescriptor of a DataPilotDescriptors

class ScDataPilotFilterDescriptor : public ScFilterDescriptorBase
{
private:
    ScDataPilotDescriptorBase*  pParent;

public:
                            ScDataPilotFilterDescriptor(ScDocShell* pDocSh, ScDataPilotDescriptorBase* pPar);
    virtual                 ~ScDataPilotFilterDescriptor();

                            // from ScFilterDescriptorBase:
    virtual void            GetData( ScQueryParam& rParam ) const override;
    virtual void            PutData( const ScQueryParam& rParam ) override;
};

class ScDatabaseRangeObj : public cppu::WeakImplHelper<
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
    XDBRefreshListenerArr_Impl aRefreshListeners;
    bool                    bIsUnnamed;
    SCTAB                   aTab;

private:
    ScDBData*               GetDBData_Impl() const;
    void                    Refreshed_Impl();

public:
    ScDatabaseRangeObj(ScDocShell* pDocSh, const OUString& rNm);
    SC_DLLPUBLIC ScDatabaseRangeObj(ScDocShell* pDocSh, const SCTAB nTab);
    virtual ~ScDatabaseRangeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // nField[] here within the area:
    void                    GetQueryParam(ScQueryParam& rQueryParam) const;
    void                    SetQueryParam(const ScQueryParam& rQueryParam);
    void                    GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
    void                    SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XDatabaseRange
    virtual css::table::CellRangeAddress SAL_CALL getDataArea()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setDataArea( const css::table::CellRangeAddress& aDataArea )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getSortDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sheet::XSubTotalDescriptor > SAL_CALL
                            getSubTotalDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getImportDescriptor() throw(css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRefreshListener( const css::uno::Reference<
                                    css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeRefreshListener( const css::uno::Reference<
                                    css::util::XRefreshListener >& l )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XCellRangeReferrer
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getReferredCells() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference<
                                        css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScDatabaseRangesObj : public cppu::WeakImplHelper<
                                css::sheet::XDatabaseRanges,
                                css::container::XEnumerationAccess,
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScDatabaseRangeObj*     GetObjectByIndex_Impl(size_t nIndex);
    ScDatabaseRangeObj*     GetObjectByName_Impl(const OUString& aName);

public:
                            ScDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScDatabaseRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XDatabaseRanges
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const css::table::CellRangeAddress& aRange )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScUnnamedDatabaseRangesObj : public cppu::WeakImplHelper<
                                css::sheet::XUnnamedDatabaseRanges>,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
                            ScUnnamedDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScUnnamedDatabaseRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XUnnamedDatabaseRanges
    virtual void SAL_CALL setByTable( const css::table::CellRangeAddress& aRange )
                                throw(css::uno::RuntimeException,
                                      css::lang::IndexOutOfBoundsException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getByTable( sal_Int32 nTab )
                                throw(css::uno::RuntimeException,
                                css::lang::IndexOutOfBoundsException,
                                css::container::NoSuchElementException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByTable( sal_Int32 nTab )
                                throw (css::uno::RuntimeException,
                                css::lang::IndexOutOfBoundsException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
