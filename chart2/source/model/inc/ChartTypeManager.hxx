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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_INC_CHARTTYPEMANAGER_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_INC_CHARTTYPEMANAGER_HXX

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase3.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/chart2/XChartTypeManager.hpp>

namespace chart
{

class ChartTypeManager :
        public ::cppu::WeakImplHelper3<
        css::lang::XServiceInfo,
        ::com::sun::star::lang::XMultiServiceFactory,
        ::com::sun::star::chart2::XChartTypeManager >
{
public:
    explicit ChartTypeManager(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ChartTypeManager();

    virtual OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException, std::exception )
        SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
            throw( ::com::sun::star::uno::RuntimeException, std::exception )
        SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException, std::exception )
        SAL_OVERRIDE;
    static OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString >
        getSupportedServiceNames_Static();

    /// establish methods for factory instatiation
    static css::uno::Reference< css::uno::XInterface > SAL_CALL create( css::uno::Reference< css::uno::XComponentContext > const & xContext)
        throw(css::uno::Exception)
    {
        return (::cppu::OWeakObject *)new ChartTypeManager( xContext );
    }

protected:
    // ____ XMultiServiceFactory ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(
            const OUString& ServiceSpecifier,
            const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any >& Arguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence<
        OUString > SAL_CALL getAvailableServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XChartTypeManager ____
    // currently empty

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
};

}

// INCLUDED_CHART2_SOURCE_MODEL_INC_CHARTTYPEMANAGER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
