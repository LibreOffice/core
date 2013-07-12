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

#ifndef SC_DATAUNO_HXX
#define SC_DATAUNO_HXX

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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>
#include "svl/itemprop.hxx"
#include "svl/lstner.hxx"
#include <boost/ptr_container/ptr_vector.hpp>

class ScDBData;
class ScDocShell;

class ScSubTotalFieldObj;
class ScDatabaseRangeObj;
class ScDataPilotDescriptorBase;

struct ScSortParam;

typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XRefreshListener > XDBRefreshListenerRef;
typedef boost::ptr_vector<XDBRefreshListenerRef> XDBRefreshListenerArr_Impl;

class ScDataUnoConversion
{
public:
    static com::sun::star::sheet::GeneralFunction SubTotalToGeneral( ScSubTotalFunc eSubTotal );
};


//  ImportDescriptor is not available as Uno-Objekt any longer, only Property-Sequence

class ScImportDescriptor
{
public:
    static void FillImportParam(
                    ScImportParam& rParam,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq,
                    const ScImportParam& rParam );
    static long GetPropertyCount();
};

//  SortDescriptor is not available as Uno-Objekt any longer, only Property-Sequence

class ScSortDescriptor
{
public:
    static void FillSortParam(
                    ScSortParam& rParam,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq );
    static void FillProperties(
                    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rSeq,
                    const ScSortParam& rParam );
    static long GetPropertyCount();
};


//  ScSubTotalDescriptorBase - base class for SubTotalDescriptor stand alone and in DB area (context?)

//  to uno, both look the same

class ScSubTotalDescriptorBase : public cppu::WeakImplHelper6<
                                        com::sun::star::sheet::XSubTotalDescriptor,
                                        com::sun::star::container::XEnumerationAccess,
                                        com::sun::star::container::XIndexAccess,
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XUnoTunnel,
                                        com::sun::star::lang::XServiceInfo >
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
    virtual void SAL_CALL   addNew( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::SubTotalColumn >& aSubTotalColumns,
                                sal_Int32 nGroupColumn )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clear() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScSubTotalDescriptorBase* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XSubTotalDescriptor> xObj );

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
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
    virtual void            GetData( ScSubTotalParam& rParam ) const;
    virtual void            PutData( const ScSubTotalParam& rParam );

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
    virtual void            GetData( ScSubTotalParam& rParam ) const;
    virtual void            PutData( const ScSubTotalParam& rParam );
};


class ScSubTotalFieldObj : public cppu::WeakImplHelper2<
                                com::sun::star::sheet::XSubTotalField,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::sheet::XSubTotalDescriptor> xRef;
    ScSubTotalDescriptorBase&   rParent;
    sal_uInt16                      nPos;

public:
                            ScSubTotalFieldObj( ScSubTotalDescriptorBase* pDesc, sal_uInt16 nP );
    virtual                 ~ScSubTotalFieldObj();

                            // XSubTotalField
    virtual sal_Int32 SAL_CALL getGroupColumn() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setGroupColumn( sal_Int32 nGroupColumn )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::SubTotalColumn > SAL_CALL
                            getSubTotalColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSubTotalColumns( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::SubTotalColumn >& aSubTotalColumns )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScConsolidationDescriptor : public cppu::WeakImplHelper2<
                                        com::sun::star::sheet::XConsolidationDescriptor,
                                        com::sun::star::lang::XServiceInfo >
{
private:
    ScConsolidateParam      aParam;

public:
                            ScConsolidationDescriptor();
    virtual                 ~ScConsolidationDescriptor();

    void                    SetParam( const ScConsolidateParam& rNew );
    const ScConsolidateParam& GetParam() const  { return aParam; }

                            // XConsolidationDescriptor
    virtual ::com::sun::star::sheet::GeneralFunction SAL_CALL getFunction()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFunction( ::com::sun::star::sheet::GeneralFunction nFunction )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getSources(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSources( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aSources )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellAddress SAL_CALL getStartOutputPosition()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setStartOutputPosition(
                                const ::com::sun::star::table::CellAddress& aStartOutputPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getUseColumnHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setUseColumnHeaders( sal_Bool bUseColumnHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getUseRowHeaders() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setUseRowHeaders( sal_Bool bUseRowHeaders )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getInsertLinks() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setInsertLinks( sal_Bool bInsertLinks )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScFilterDescriptorBase - base class for FilterDescriptor
//                           stand alone, in a DB area (or context?) and in the DataPilot

//  to uno, all three look the same

class ScFilterDescriptorBase : public cppu::WeakImplHelper5<
                                    com::sun::star::sheet::XSheetFilterDescriptor,
                                    com::sun::star::sheet::XSheetFilterDescriptor2,
                                    com::sun::star::sheet::XSheetFilterDescriptor3,
                                    com::sun::star::beans::XPropertySet,
                                    com::sun::star::lang::XServiceInfo >,
                               public SfxListener
{
private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocSh;

public:
                            ScFilterDescriptorBase(ScDocShell* pDocShell);
    virtual                 ~ScFilterDescriptorBase();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // in the derived classes(?):
                            // (nField[] here within the area)
    virtual void            GetData( ScQueryParam& rParam ) const = 0;
    virtual void            PutData( const ScQueryParam& rParam ) = 0;

                            // XSheetFilterDescriptor
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TableFilterField > SAL_CALL
                            getFilterFields() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFilterFields( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::TableFilterField >& aFilterFields )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetFilterDescriptor2
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TableFilterField2 > SAL_CALL
                            getFilterFields2() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFilterFields2( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::TableFilterField2 >& aFilterFields )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetFilterDescriptor3
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TableFilterField3 > SAL_CALL
                            getFilterFields3() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setFilterFields3( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::sheet::TableFilterField3 >& aFilterFields )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
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
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );

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
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );
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
    virtual void            GetData( ScQueryParam& rParam ) const;
    virtual void            PutData( const ScQueryParam& rParam );
};


class ScDatabaseRangeObj : public cppu::WeakImplHelper6<
                                com::sun::star::sheet::XDatabaseRange,
                                com::sun::star::util::XRefreshable,
                                com::sun::star::container::XNamed,
                                com::sun::star::sheet::XCellRangeReferrer,
                                com::sun::star::beans::XPropertySet,
                                com::sun::star::lang::XServiceInfo >,
                           public SfxListener
{
private:
    ScDocShell*             pDocShell;
    String                  aName;
    SfxItemPropertySet      aPropSet;
    XDBRefreshListenerArr_Impl aRefreshListeners;
    bool                    bIsUnnamed;
    SCTAB                   aTab;

private:
    ScDBData*               GetDBData_Impl() const;
    void                    Refreshed_Impl();

public:
    ScDatabaseRangeObj(ScDocShell* pDocSh, const String& rNm);
    SC_DLLPUBLIC ScDatabaseRangeObj(ScDocShell* pDocSh, const SCTAB nTab);
    virtual ~ScDatabaseRangeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // nField[] here within the area:
    void                    GetQueryParam(ScQueryParam& rQueryParam) const;
    void                    SetQueryParam(const ScQueryParam& rQueryParam);
    void                    GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
    void                    SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

                            // XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XDatabaseRange
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getDataArea()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setDataArea( const ::com::sun::star::table::CellRangeAddress& aDataArea )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getSortDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            getFilterDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSubTotalDescriptor > SAL_CALL
                            getSubTotalDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getImportDescriptor() throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL   refresh() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRefreshListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XRefreshListener >& l )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeReferrer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getReferredCells() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDatabaseRangesObj : public cppu::WeakImplHelper4<
                                com::sun::star::sheet::XDatabaseRanges,
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScDatabaseRangeObj*     GetObjectByIndex_Impl(size_t nIndex);
    ScDatabaseRangeObj*     GetObjectByName_Impl(const OUString& aName);

public:
                            ScDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScDatabaseRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDatabaseRanges
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                const ::com::sun::star::table::CellRangeAddress& aRange )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScUnnamedDatabaseRangesObj : public cppu::WeakImplHelper1<
                                com::sun::star::sheet::XUnnamedDatabaseRanges>,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
                            ScUnnamedDatabaseRangesObj(ScDocShell* pDocSh);
    virtual                 ~ScUnnamedDatabaseRangesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XUnnamedDatabaseRanges
    virtual void SAL_CALL setByTable( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException,
                                      ::com::sun::star::lang::IndexOutOfBoundsException );
    virtual com::sun::star::uno::Any SAL_CALL getByTable( sal_Int32 nTab )
                                throw(::com::sun::star::uno::RuntimeException,
                                ::com::sun::star::lang::IndexOutOfBoundsException,
                                ::com::sun::star::container::NoSuchElementException );
    virtual sal_Bool SAL_CALL hasByTable( sal_Int32 nTab )
                                throw (::com::sun::star::uno::RuntimeException,
                                ::com::sun::star::lang::IndexOutOfBoundsException);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
