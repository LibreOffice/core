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
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <uno/environment.h>

#include <functional>

namespace comphelper {
namespace service_decl {

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
                 char const* pSupportedServiceNames, char cDelim = ';' )
        : m_createFunc(implClass.m_createFunc),
          m_pImplName(pImplName),
          m_pServiceNames(pSupportedServiceNames),
          m_cDelim(cDelim) {}

    /// @internal gets called by component_getFactoryHelper()
    void * getFactory( sal_Char const* pImplName ) const;

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
    char const m_cDelim;
};

/** To specify whether the implementation class expects arguments
    (uno::Sequence<uno::Any>).
*/
template <bool> struct with_args;

/// @internal
namespace detail {
template <typename ImplT>
class OwnServiceImpl
    : public ImplT
{
    typedef ImplT BaseT;

public:
    OwnServiceImpl( const OwnServiceImpl& ) = delete;
    OwnServiceImpl& operator=( const OwnServiceImpl& ) = delete;
    OwnServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Sequence<css::uno::Any> const& args,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        :BaseT(args, xContext), m_rServiceDecl(rServiceDecl) {}
    OwnServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : BaseT(xContext), m_rServiceDecl(rServiceDecl) {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException) override {
        return m_rServiceDecl.getImplementationName();
    }
    virtual sal_Bool SAL_CALL supportsService( OUString const& name )
        throw (css::uno::RuntimeException) override {
        return m_rServiceDecl.supportsService(name);
    }
    virtual css::uno::Sequence< OUString>
    SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException) override {
        return m_rServiceDecl.getSupportedServiceNames();
    }

private:
    ServiceDecl const& m_rServiceDecl;
};

template <typename ImplT>
class ServiceImpl : public OwnServiceImpl< ::cppu::ImplInheritanceHelper<ImplT,css::lang::XServiceInfo> >
{
typedef OwnServiceImpl< ::cppu::ImplInheritanceHelper<ImplT,css::lang::XServiceInfo> > ServiceImpl_BASE;
public:
    ServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Sequence<css::uno::Any> const& args,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : ServiceImpl_BASE(rServiceDecl, args, xContext) {}
    ServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : ServiceImpl_BASE(rServiceDecl, xContext) {}
};

template <typename ImplT>
class InheritingServiceImpl : public OwnServiceImpl< ImplT >
{
typedef OwnServiceImpl< ImplT > ServiceImpl_BASE;
public:
    InheritingServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Sequence<css::uno::Any> const& args,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : ServiceImpl_BASE(rServiceDecl, args, xContext) {}
    InheritingServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : ServiceImpl_BASE(rServiceDecl, xContext) {}
};

template <typename ServiceImplT>
struct PostProcessDefault {
    css::uno::Reference<css::uno::XInterface>
    operator()( ServiceImplT * p ) const {
        return static_cast<css::lang::XServiceInfo *>(p);
    }
};

template <typename ImplT, typename PostProcessFuncT, typename WithArgsT>
struct CreateFunc;

template <typename ImplT, typename PostProcessFuncT>
struct CreateFunc<ImplT, PostProcessFuncT, with_args<false> > {
    PostProcessFuncT const m_postProcessFunc;
    explicit CreateFunc( PostProcessFuncT const& postProcessFunc )
        : m_postProcessFunc(postProcessFunc) {}

    css::uno::Reference<css::uno::XInterface>
    operator()( ServiceDecl const& rServiceDecl,
                css::uno::Sequence<css::uno::Any> const&,
                css::uno::Reference<css::uno::XComponentContext>
                const& xContext ) const
    {
        return m_postProcessFunc(
            new ImplT( rServiceDecl, xContext ) );
    }
};

template <typename ImplT, typename PostProcessFuncT>
struct CreateFunc<ImplT, PostProcessFuncT, with_args<true> > {
    PostProcessFuncT const m_postProcessFunc;
    explicit CreateFunc( PostProcessFuncT const& postProcessFunc )
        : m_postProcessFunc(postProcessFunc) {}

    css::uno::Reference<css::uno::XInterface>
    operator()( ServiceDecl const& rServiceDecl,
                css::uno::Sequence<css::uno::Any> const& args,
                css::uno::Reference<css::uno::XComponentContext>
                const& xContext ) const
    {
        return m_postProcessFunc(
            new ImplT( rServiceDecl, args, xContext ) );
    }
};

} // namespace detail

/** Defines a service implementation class.

    @tpl ImplT_ service implementation class
    @WithArgsT whether the implementation class ctor expects arguments
               (uno::Sequence<uno::Any>, uno::Reference<uno::XComponentContext>)
               or just (uno::Reference<uno::XComponentContext>)
*/
template <typename ImplT_, typename WithArgsT = with_args<false> >
struct serviceimpl_base {
    typedef ImplT_ ImplT;

    detail::CreateFuncF const m_createFunc;

    typedef detail::PostProcessDefault<ImplT> PostProcessDefaultT;

    /** Default ctor.  Implementation class without args, expecting
        component context as single argument.
    */
    serviceimpl_base() : m_createFunc(
        detail::CreateFunc<ImplT, PostProcessDefaultT, WithArgsT>(
            PostProcessDefaultT() ) ) {}

    /** Ctor to pass a post processing function/functor.

        @tpl PostProcessDefaultT let your compiler deduce this
        @param postProcessFunc function/functor that gets the yet unacquired
                               ImplT_ pointer returning a
                               uno::Reference<uno::XInterface>
    */
    template <typename PostProcessFuncT>
    explicit serviceimpl_base( PostProcessFuncT const& postProcessFunc )
        : m_createFunc( detail::CreateFunc<ImplT, PostProcessFuncT, WithArgsT>(
                            postProcessFunc ) ) {}
};

template <typename ImplT_, typename WithArgsT = with_args<false> >
struct class_ : public serviceimpl_base< detail::ServiceImpl<ImplT_>, WithArgsT >
{
    typedef serviceimpl_base< detail::ServiceImpl<ImplT_>, WithArgsT > baseT;
    /** Default ctor.  Implementation class without args, expecting
        component context as single argument.
    */
    class_() : baseT() {}
    template <typename PostProcessFuncT>
    /** Ctor to pass a post processing function/functor.

        @tpl PostProcessDefaultT let your compiler deduce this
        @param postProcessFunc function/functor that gets the yet unacquired
                               ImplT_ pointer returning a
                               uno::Reference<uno::XInterface>
    */
    explicit class_( PostProcessFuncT const& postProcessFunc ) : baseT( postProcessFunc ) {}
};

template <typename ImplT_, typename WithArgsT = with_args<false> >
struct inheritingClass_ : public serviceimpl_base< detail::InheritingServiceImpl<ImplT_>, WithArgsT >
{
    typedef serviceimpl_base< detail::InheritingServiceImpl<ImplT_>, WithArgsT > baseT;
    /** Default ctor.  Implementation class without args, expecting
        component context as single argument.
    */
    inheritingClass_() : baseT() {}
    template <typename PostProcessFuncT>
    /** Ctor to pass a post processing function/functor.

        @tpl PostProcessDefaultT let your compiler deduce this
        @param postProcessFunc function/functor that gets the yet unacquired
                               ImplT_ pointer returning a
                               uno::Reference<uno::XInterface>
    */
    explicit inheritingClass_( PostProcessFuncT const& postProcessFunc ) : baseT( postProcessFunc ) {}
};

// component_... helpers with arbitrary service declarations:

template< typename T >
inline void* component_getFactoryHelper( const sal_Char* pImplName, void* pRet,
                                         const T& s )
{
    if( pRet == nullptr )
        return s.getFactory( pImplName );
    return pRet;
}

template< typename T, typename... Args >
inline void* component_getFactoryHelper( const sal_Char* pImplName, void* pRet,
                                         const T& s, const Args&... args )
{
    if( pRet == nullptr )
        return component_getFactoryHelper( pImplName, s.getFactory( pImplName ), args... );
    return pRet;
}

template< typename... Args >
inline void* component_getFactoryHelper( const sal_Char* pImplName,
                                         const Args&... args )
{
    void* pRet = nullptr;
    return component_getFactoryHelper( pImplName, pRet, args... );
}

} // namespace service_decl
} // namespace comphelper


#endif //  ! defined( INCLUDED_COMPHELPER_SERVICEDECL_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
