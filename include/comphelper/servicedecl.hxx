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
#ifndef INCLUDED_COMPHELPER_SERVICEDECL_HXX
#define INCLUDED_COMPHELPER_SERVICEDECL_HXX

#include <comphelper/comphelperdllapi.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <functional>
#include <initializer_list>

namespace comphelper::service_decl {

class ServiceDecl;

namespace detail {
typedef ::std::function<
    css::uno::Reference<css::uno::XInterface> /* return */
    (ServiceDecl const&,
     css::uno::Sequence<css::uno::Any> const&,
     css::uno::Reference<css::uno::XComponentContext> const&)> CreateFuncF;
}

/** Class to declare a service implementation.  There is no need to implement
    lang::XServiceInfo nor lang::XInitialization anymore.
    The declaration can be done in various ways, the (simplest) form is

    <pre>
    class MyClass : public cppu::WeakImplHelper<XInterface1, XInterface2> {
    public:
        MyClass( uno::Reference<uno::XComponentContext> const& xContext )
        [...]
    };
    [...]
    namespace sdecl = comphelper::service_decl;
    sdecl::ServiceDecl const myDecl(
        sdecl::class_<MyClass>(),
        "my.unique.implementation.name",
        "MyServiceSpec1;MyServiceSpec2" );
    </pre>

    If the service demands initialization by arguments, the implementation
    class has to define a constructor taking both arguments and component
    context:

    <pre>
    class MyClass : public cppu::WeakImplHelper<XInterface1, XInterface2> {
    public:
        MyClass( uno::Sequence<uno::Any> const& args,
                 uno::Reference<uno:XComponentContext> const& xContext )
        [...]
    };
    [...]
    namespace sdecl = comphelper::service_decl;
    sdecl::ServiceDecl const myDecl(
        sdecl::class_<MyClass, sdecl::with_args<true> >(),
        "my.unique.implementation.name",
        "MyServiceSpec1;MyServiceSpec2" );
    </pre>

    Additionally, there is the possibility to process some code after creation,
    e.g. to add the newly created object as a listener or perform aggregation
    (C++-UNO only):

    <pre>
    uno::Reference<uno::XInterface> somePostProcCode( MyClass * p );
    [...]
    namespace sdecl = comphelper::service_decl;
    sdecl::ServiceDecl const myDecl(
        sdecl::class_<MyClass, ... >(&somePostProcCode),
        "my.unique.implementation.name",
        "MyServiceSpec1;MyServiceSpec2" );
    </pre>

    In the latter case, somePostProcCode gets the yet unacquired "raw" pointer.
*/
class COMPHELPER_DLLPUBLIC ServiceDecl
{
public:
    /** Ctor for multiple supported service names.

        @param implClass implementation class description
        @param pImplName implementation name
        @param pSupportedServiceNames supported service names
        @param cDelim delimiter for supported service names
    */
    ServiceDecl( const ServiceDecl& ) = delete;
    ServiceDecl& operator=( const ServiceDecl& ) = delete;
    template <typename ImplClassT>
    ServiceDecl( ImplClassT const& implClass,
                 char const* pImplName,
                 char const* pSupportedServiceNames )
        : m_createFunc(implClass.m_createFunc),
          m_pImplName(pImplName),
          m_pServiceNames(pSupportedServiceNames) {}

    /// @internal gets called by component_getFactoryHelper()
    void * getFactory( char const* pImplName ) const;

    /// @return supported service names
    css::uno::Sequence< OUString> getSupportedServiceNames() const;

    /// @return whether name is in set of supported service names
    bool supportsService( OUString const& name ) const;

    /// @return implementation name
    OUString getImplementationName() const;

private:
    class Factory;
    friend class Factory;

    detail::CreateFuncF const m_createFunc;
    char const* const m_pImplName;
    char const* const m_pServiceNames;
};

/** To specify whether the implementation class expects arguments
    (uno::Sequence<uno::Any>).
*/
template <bool> struct with_args;

/// @internal
namespace detail {

} // namespace detail


COMPHELPER_DLLPUBLIC
void* component_getFactoryHelper( const char* pImplName,
                                  std::initializer_list<ServiceDecl const *> args );

} // namespace comphelper::service_decl


#endif //  ! defined( INCLUDED_COMPHELPER_SERVICEDECL_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
