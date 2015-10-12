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

#ifndef INCLUDED_SC_INC_CHARTUNO_HXX
#define INCLUDED_SC_INC_CHARTUNO_HXX

#include "address.hxx"
#include "rangelst.hxx"
#include <svl/lstner.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>

#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>

class ScDocShell;
class ScChartObj;

class ScChartsObj : public cppu::WeakImplHelper<
                            com::sun::star::table::XTableCharts,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;           // Charts are per sheet

    ScChartObj*             GetObjectByIndex_Impl(long nIndex) const;
    ScChartObj*             GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScChartsObj(ScDocShell* pDocSh, SCTAB nT);
    virtual                 ~ScChartsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XTableCharts
    virtual void SAL_CALL   addNewByName( const OUString& aName,
                                    const ::com::sun::star::awt::Rectangle& aRect,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& aRanges,
                                    sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
                                        throw(::com::sun::star::uno::RuntimeException,
                                              std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& aName )
                                        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

typedef ::cppu::WeakComponentImplHelper<
    ::com::sun::star::table::XTableChart,
    ::com::sun::star::document::XEmbeddedObjectSupplier,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XServiceInfo > ScChartObj_Base;

typedef ::comphelper::OPropertyContainer ScChartObj_PBase;
typedef ::comphelper::OPropertyArrayUsageHelper< ScChartObj > ScChartObj_PABase;

class ScChartObj : public ::comphelper::OBaseMutex
                  ,public ScChartObj_Base
                  ,public ScChartObj_PBase
                  ,public ScChartObj_PABase
                  ,public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;           // Charts are per sheet
    OUString                aChartName;

    void    Update_Impl( const ScRangeListRef& rRanges, bool bColHeaders, bool bRowHeaders );
    void    GetData_Impl( ScRangeListRef& rRanges, bool& rColHeaders, bool& rRowHeaders ) const;

protected:
    // ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception,
               std::exception) override;
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

public:
                            ScChartObj(ScDocShell* pDocSh, SCTAB nT, const OUString& rN);
    virtual                 ~ScChartObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    virtual void SAL_CALL disposing() override;

                            // XTableChart
    virtual sal_Bool SAL_CALL getHasColumnHeaders() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setHasColumnHeaders( sal_Bool bHasColumnHeaders )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;
    virtual sal_Bool SAL_CALL getHasRowHeaders() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setHasRowHeaders( sal_Bool bHasRowHeaders )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRanges(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setRanges( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::table::CellRangeAddress >& aRanges )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;

                            // XEmbeddedObjectSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL
                            getEmbeddedObject() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
