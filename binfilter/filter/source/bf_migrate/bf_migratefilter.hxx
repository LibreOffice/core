/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BF_MIGRATEFILTER_HXX
#define _BF_MIGRATEFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>

#include <com/sun/star/document/XExporter.hpp>

#include <com/sun/star/document/XImporter.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase5.hxx>
namespace binfilter {

enum FilterType 
{
    FILTER_IMPORT,
    FILTER_EXPORT
};

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */

class bf_MigrateFilter : public cppu::WeakImplHelper5 
< 
    com::sun::star::document::XFilter,
    com::sun::star::document::XExporter,
    com::sun::star::document::XImporter,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxLegServFact;
    ::rtl::OUString msFilterName;
    FilterType meType;

    sal_Bool exportImpl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor) 
        throw (::com::sun::star::uno::RuntimeException);

    sal_Bool importImpl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor) 
        throw (::com::sun::star::uno::RuntimeException);

    sal_Bool getContactToLegacyProcessServiceFactory()
        throw (::com::sun::star::uno::RuntimeException);

public:
    bf_MigrateFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    :	mxMSF(rxMSF) 
    {
    }

    virtual ~bf_MigrateFilter() 
    {
    }

    // XFilter
    virtual sal_Bool SAL_CALL filter(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor) 
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL cancel() 
        throw (::com::sun::star::uno::RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc) 
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc) 
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments) 
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() 
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName) 
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() 
        throw (::com::sun::star::uno::RuntimeException);
};

::rtl::OUString bf_MigrateFilter_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL bf_MigrateFilter_supportsService(const ::rtl::OUString& ServiceName) 
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL bf_MigrateFilter_getSupportedServiceNames() 
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL bf_MigrateFilter_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( ::com::sun::star::uno::Exception );

}//end of namespace binfilter
#endif // _BF_MIGRATEFILTER_HXX
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
