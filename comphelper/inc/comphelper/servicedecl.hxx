/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicedecl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:10:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#if ! defined(COMPHELPER_SERVICEDECL_HXX_INCLUDED)
#define COMPHELPER_SERVICEDECL_HXX_INCLUDED

#if ! defined(INCLUDED_COMPHELPERDLLAPI_H)
#include "comphelper/comphelperdllapi.h"
#endif
#if ! defined(_CPPUHELPER_IMPLBASE1_HXX_)
#include "cppuhelper/implbase1.hxx"
#endif
#if ! defined(_COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_)
#include "com/sun/star/uno/XComponentContext.hpp"
#endif
#if ! defined(_COM_SUN_STAR_LANG_XSERVICEINFO_HPP_)
#include "com/sun/star/lang/XServiceInfo.hpp"
#endif
#if ! defined(_COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_)
#include "com/sun/star/registry/XRegistryKey.hpp"
#endif
#include "boost/utility.hpp"
#include "boost/function.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/repetition.hpp"
#include "boost/preprocessor/seq/enum.hpp"

namespace comphelper {
namespace service_decl {

class ServiceDecl;

namespace detail {
namespace css = ::com::sun::star;
typedef ::boost::function3<
    css::uno::Reference<css::uno::XInterface> /* return */,
    ServiceDecl const&,
    css::uno::Sequence<css::uno::Any> const&,
    css::uno::Reference<css::uno::XComponentContext> const&> CreateFuncF;
}

/** Class to declare a service implementation.  There is no need to implement
    lang::XServiceInfo nor lang::XInitialization anymore.
    The declaration can be done in various ways, the (simplest) form is

    <pre>
    class MyClass : cppu::WeakImplHelper2<XInterface1, XInterface2> {
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
    class MyClass : cppu::WeakImplHelper2<XInterface1, XInterface2> {
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
class COMPHELPER_DLLPUBLIC ServiceDecl : private ::boost::noncopyable
{
public:
    /** Ctor for multiple supported service names.

        @param implClass implementation class description
        @param pImplName implementation name
        @param pSupportedServiceNames supported service names
        @param cDelim delimiter for supported service names
    */
    template <typename ImplClassT>
    ServiceDecl( ImplClassT const& implClass,
                 char const* pImplName,
                 char const* pSupportedServiceNames, char cDelim = ';' )
        : m_createFunc(implClass.m_createFunc),
          m_pImplName(pImplName),
          m_pServiceNames(pSupportedServiceNames),
          m_cDelim(cDelim) {}

    /// @internal gets called by component_writeInfoHelper()
    bool writeInfo( ::com::sun::star::registry::XRegistryKey * xKey ) const;
    /// @internal gets called by component_getFactoryHelper()
    void * getFactory( sal_Char const* pImplName ) const;

    /// @return supported service names
    ::com::sun::star::uno::Sequence< ::rtl::OUString>
    getSupportedServiceNames() const;

    /// @return whether name is in set of supported service names
    bool supportsService( ::rtl::OUString const& name ) const;

    /// @return implementation name
    ::rtl::OUString getImplementationName() const;

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
class ServiceImpl
    : public ::cppu::ImplInheritanceHelper1<ImplT, css::lang::XServiceInfo>,
      private ::boost::noncopyable
{
    typedef ::cppu::ImplInheritanceHelper1<ImplT,css::lang::XServiceInfo> BaseT;

public:
    ServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Sequence<css::uno::Any> const& args,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : BaseT(args, xContext), m_rServiceDecl(rServiceDecl) {}
    ServiceImpl(
        ServiceDecl const& rServiceDecl,
        css::uno::Reference<css::uno::XComponentContext> const& xContext )
        : BaseT(xContext), m_rServiceDecl(rServiceDecl) {}

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException) {
        return m_rServiceDecl.getImplementationName();
    }
    virtual sal_Bool SAL_CALL supportsService( ::rtl::OUString const& name )
        throw (css::uno::RuntimeException) {
        return m_rServiceDecl.supportsService(name);
    }
    virtual css::uno::Sequence< ::rtl::OUString>
    SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException) {
        return m_rServiceDecl.getSupportedServiceNames();
    }

private:
    ServiceDecl const& m_rServiceDecl;
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
            new ServiceImpl<ImplT>( rServiceDecl, xContext ) );
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
            new ServiceImpl<ImplT>( rServiceDecl, args, xContext ) );
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
struct class_ {
    typedef ImplT_ ImplT;
    typedef detail::ServiceImpl<ImplT_> ServiceImplT;

    detail::CreateFuncF const m_createFunc;

    typedef detail::PostProcessDefault<ServiceImplT> PostProcessDefaultT;

    /** Default ctor.  Implementation class without args, expecting
        component context as single argument.
    */
    class_() : m_createFunc(
        detail::CreateFunc<ImplT, PostProcessDefaultT, WithArgsT>(
            PostProcessDefaultT() ) ) {}

    /** Ctor to pass a post processing function/functor.

        @tpl PostProcessDefaultT let your compiler deduce this
        @param postProcessFunc function/functor that gets the yet unacquired
                               ImplT_ pointer returning a
                               uno::Reference<uno::XInterface>
    */
    template <typename PostProcessFuncT>
    explicit class_( PostProcessFuncT const& postProcessFunc )
        : m_createFunc( detail::CreateFunc<ImplT, PostProcessFuncT, WithArgsT>(
                            postProcessFunc ) ) {}
};


//
// component_... helpers with arbitrary service declarations:
//

#define COMPHELPER_SERVICEDECL_writeInfo(z_, n_, unused_) \
    bRet &= BOOST_PP_CAT(s, n_).writeInfo( xRegistryKey );

#define COMPHELPER_SERVICEDECL_getFactory(z_, n_, unused_) \
    if (pRet == 0) \
        pRet = BOOST_PP_CAT(s, n_).getFactory(pImplName);

/** The following preprocessor repetitions generate functions like

    <pre>
        inline sal_Bool component_writeInfoHelper(
            ::com::sun::star::lang::XMultiServiceFactory *,
            ::com::sun::star::registry::XRegistryKey * xRegistryKey,
            ServiceDecl const& s0, ServiceDecl const& s1, ... );

        inline void * component_getFactoryHelper(
            sal_Char const* pImplName,
            ::com::sun::star::lang::XMultiServiceFactory *,
            ::com::sun::star::registry::XRegistryKey * xRegistryKey,
            ServiceDecl const& s0, ServiceDecl const& s1, ... );
    </pre>

    which call on the passed service declarations.

    The maximum number of service declarations can be set by defining
    COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS; its default is 8.
*/
#define COMPHELPER_SERVICEDECL_make(z_, n_, unused_) \
inline sal_Bool component_writeInfoHelper( \
    ::com::sun::star::lang::XMultiServiceFactory *, \
    ::com::sun::star::registry::XRegistryKey * xRegistryKey, \
    BOOST_PP_ENUM_PARAMS(n_, ServiceDecl const& s) ) \
{ \
    bool bRet = true; \
    BOOST_PP_REPEAT(n_, COMPHELPER_SERVICEDECL_writeInfo, ~) \
    return bRet; \
} \
inline void * component_getFactoryHelper( \
    sal_Char const* pImplName, \
    ::com::sun::star::lang::XMultiServiceFactory *, \
    ::com::sun::star::registry::XRegistryKey *, \
    BOOST_PP_ENUM_PARAMS(n_, ServiceDecl const& s) ) \
{ \
    void * pRet = 0; \
    BOOST_PP_REPEAT(n_, COMPHELPER_SERVICEDECL_getFactory, ~) \
    return pRet; \
}

#if ! defined(COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS)
#define COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS 8
#endif

BOOST_PP_REPEAT_FROM_TO(1, COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS,
                        COMPHELPER_SERVICEDECL_make, ~)

#undef COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS
#undef COMPHELPER_SERVICEDECL_make
#undef COMPHELPER_SERVICEDECL_getFactory
#undef COMPHELPER_SERVICEDECL_writeInfo

} // namespace service_decl
} // namespace comphelper

/** The following preprocessor macro generates the C access functions,
    that are used to initialize and register the components of a
    shared library object.

    If you have, say, written a lib that contains three distinct
    components, each with its own ServiceDecl object, you might want
    to employ the following code:

    <pre>
        // must reside outside _any_ namespace
        COMPHELPER_SERVICEDECL_EXPORTS3(yourServiceDecl1,
                                       yourServiceDecl2,
                                       yourServiceDecl3);
    </pre>

    For your convenience, the COMPHELPER_SERVICEDECL_EXPORTS<N> macro
    comes pre-defined up to N=8, if you should need more arguments,
    call COMPHELPER_SERVICEDECL_make_exports directly, like this:

    <pre>
        // must reside outside _any_ namespace
        COMPHELPER_SERVICEDECL_make_exports((yourServiceDecl1)(yourServiceDecl2)...(yourServiceDeclN));
    </pre>

    Note the missing colons between the bracketed arguments.
 */
#define COMPHELPER_SERVICEDECL_make_exports(varargs_ )  \
extern "C" \
{ \
    void SAL_CALL component_getImplementationEnvironment( const sal_Char**  ppEnvTypeName, \
                                                          uno_Environment** /*ppEnv*/ ) \
    { \
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME; \
    } \
 \
    sal_Bool SAL_CALL component_writeInfo( lang::XMultiServiceFactory*  pServiceManager, \
                                           registry::XRegistryKey*      pRegistryKey ) \
    { \
        return component_writeInfoHelper( pServiceManager, pRegistryKey, \
                                          BOOST_PP_SEQ_ENUM(varargs_) ); \
    } \
 \
    void* SAL_CALL component_getFactory( sal_Char const*                pImplName, \
                                         lang::XMultiServiceFactory*    pServiceManager, \
                                         registry::XRegistryKey*        pRegistryKey ) \
    { \
        return component_getFactoryHelper( pImplName, pServiceManager, \
                                           pRegistryKey, \
                                           BOOST_PP_SEQ_ENUM(varargs_) ); \
    } \
}

#define COMPHELPER_SERVICEDECL_EXPORTS1(comp0_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_))
#define COMPHELPER_SERVICEDECL_EXPORTS2(comp0_,comp1_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_))
#define COMPHELPER_SERVICEDECL_EXPORTS3(comp0_,comp1_,comp2_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_))
#define COMPHELPER_SERVICEDECL_EXPORTS4(comp0_,comp1_,comp2_,comp3_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_)(comp3_))
#define COMPHELPER_SERVICEDECL_EXPORTS5(comp0_,comp1_,comp2_,comp3_,comp4_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_)(comp3_)(comp4_))
#define COMPHELPER_SERVICEDECL_EXPORTS6(comp0_,comp1_,comp2_,comp3_,comp4_,comp5_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_)(comp3_)(comp4_)(comp5_))
#define COMPHELPER_SERVICEDECL_EXPORTS7(comp0_,comp1_,comp2_,comp3_,comp4_,comp5_,comp6_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_)(comp3_)(comp4_)(comp5_)(comp6_))
#define COMPHELPER_SERVICEDECL_EXPORTS8(comp0_,comp1_,comp2_,comp3_,comp4_,comp5_,comp6_,comp7_) \
    COMPHELPER_SERVICEDECL_make_exports((comp0_)(comp1_)(comp2_)(comp3_)(comp4_)(comp5_)(comp6_)(comp7_))

#endif //  ! defined(COMPHELPER_SERVICEDECL_HXX_INCLUDED)

