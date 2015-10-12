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

#ifndef INCLUDED_UCB_SOURCE_CORE_UCBPROPS_HXX
#define INCLUDED_UCB_SOURCE_CORE_UCBPROPS_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/implbase.hxx>



#define PROPERTIES_MANAGER_SERVICE_NAME "com.sun.star.ucb.PropertiesManager"



// class UcbPropertiesManager.



class UcbPropertiesManager : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::beans::XPropertySetInfo >
{
    css::uno::Sequence< css::beans::Property > m_pProps;

private:
    bool queryProperty( const OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    explicit UcbPropertiesManager( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory >&
                                rxSMgr );
    virtual ~UcbPropertiesManager();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif // INCLUDED_UCB_SOURCE_CORE_UCBPROPS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
