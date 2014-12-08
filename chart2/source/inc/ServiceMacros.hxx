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
#ifndef INCLUDED_CHART2_SOURCE_INC_SERVICEMACROS_HXX
#define INCLUDED_CHART2_SOURCE_INC_SERVICEMACROS_HXX

#include <cppuhelper/supportsservice.hxx>

/*
to use these macros the supported services and the implementation name needs to be static
especially you need to implement (declaration is contained in macro already):

static com::sun::star::uno::Sequence< OUString >
    Class::getSupportedServiceNames_Static();
*/

// XServiceInfo decl

namespace apphelper
{

#define APPHELPER_XSERVICEINFO_DECL()                                         \
    virtual OUString SAL_CALL getImplementationName()                         \
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE; \
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )  \
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE; \
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()\
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE; \
                                                                              \
    static OUString getImplementationName_Static();                           \
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

// XServiceInfo impl

#define APPHELPER_XSERVICEINFO_IMPL( Class, ImplName )                      \
OUString SAL_CALL Class::getImplementationName()                            \
    throw( css::uno::RuntimeException, std::exception )                     \
{                                                                           \
    return getImplementationName_Static();                                  \
}                                                                           \
                                                                            \
OUString Class::getImplementationName_Static()                              \
{                                                                           \
    return ImplName;                                                        \
}                                                                           \
                                                                            \
sal_Bool SAL_CALL Class::supportsService( const OUString& rServiceName )    \
    throw( css::uno::RuntimeException, std::exception )                     \
{                                                                           \
    return cppu::supportsService(this, rServiceName);                       \
}                                                                           \
                                                                            \
css::uno::Sequence< OUString > SAL_CALL Class::getSupportedServiceNames()   \
    throw( css::uno::RuntimeException, std::exception )                     \
{                                                                           \
    return getSupportedServiceNames_Static();                               \
}

// Service factory helper decl+impl

//to use this macro you need to provide a constructor:
//class( Reference< XComponentContext > const & xContext )
//and implement OWeakObject

#define APPHELPER_SERVICE_FACTORY_HELPER(Class)                             \
static css::uno::Reference< css::uno::XInterface > SAL_CALL create( css::uno::Reference< css::uno::XComponentContext > const & xContext) \
    throw(css::uno::Exception)                                              \
{                                                                           \
    return (::cppu::OWeakObject *)new Class( xContext );                    \
}

}//end namespace apphelper
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
