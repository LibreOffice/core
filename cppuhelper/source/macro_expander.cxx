/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <rtl/bootstrap.hxx>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

#include "macro_expander.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SERVICE_NAME_A "com.sun.star.lang.MacroExpander"
#define SERVICE_NAME_B "com.sun.star.lang.BootstrapMacroExpander"
#define IMPL_NAME "com.sun.star.lang.comp.cppuhelper.BootstrapMacroExpander"


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{
//---- private forward -----------------------------------------------------------------------------
Bootstrap const & get_unorc() SAL_THROW( () );
}

namespace cppuhelper { namespace detail {

rtl::OUString expandMacros(rtl::OUString const & text) {
    rtl::OUString t(text);
    rtl_bootstrap_expandMacros_from_handle(
        cppu::get_unorc().getHandle(), &t.pData);
    return t;
}

} }

namespace
{
inline OUString s_impl_name() { return OUSTR(IMPL_NAME); }
static Sequence< OUString > const & s_get_service_names()
{
    static Sequence< OUString > const * s_pnames = 0;
    if (! s_pnames)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! s_pnames)
        {
            static Sequence< OUString > s_names( 2 );
            s_names[ 0 ] = OUSTR(SERVICE_NAME_A);
            s_names[ 1 ] = OUSTR(SERVICE_NAME_B);
            s_pnames = &s_names;
        }
    }
    return *s_pnames;
}

typedef ::cppu::WeakComponentImplHelper2<
    util::XMacroExpander, lang::XServiceInfo > t_uno_impl;

struct mutex_holder
{
    Mutex m_mutex;
};
class Bootstrap_MacroExpander : public mutex_holder, public t_uno_impl
{
protected:
    virtual void SAL_CALL disposing();

public:
    inline Bootstrap_MacroExpander( Reference< XComponentContext > const & ) SAL_THROW( () )
        : t_uno_impl( m_mutex )
        {}
    virtual ~Bootstrap_MacroExpander()
        SAL_THROW( () );

    // XMacroExpander impl
    virtual OUString SAL_CALL expandMacros( OUString const & exp )
        throw (lang::IllegalArgumentException);
    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};

//__________________________________________________________________________________________________
void Bootstrap_MacroExpander::disposing()
{}
//__________________________________________________________________________________________________
Bootstrap_MacroExpander::~Bootstrap_MacroExpander() SAL_THROW( () )
{}

// XServiceInfo impl
//__________________________________________________________________________________________________
OUString Bootstrap_MacroExpander::getImplementationName()
    throw (RuntimeException)
{
    return s_impl_name();
}
//__________________________________________________________________________________________________
sal_Bool Bootstrap_MacroExpander::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    Sequence< OUString > const & service_names = s_get_service_names();
    OUString const * p = service_names.getConstArray();
    for ( sal_Int32 nPos = service_names.getLength(); nPos--; )
    {
        if (p[ nPos ].equals( serviceName ))
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > Bootstrap_MacroExpander::getSupportedServiceNames()
    throw (RuntimeException)
{
    return s_get_service_names();
}

// XMacroExpander impl
//__________________________________________________________________________________________________
OUString Bootstrap_MacroExpander::expandMacros( OUString const & exp )
    throw (lang::IllegalArgumentException)
{
    return cppuhelper::detail::expandMacros( exp );
}

//==================================================================================================
Reference< XInterface > SAL_CALL service_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (RuntimeException) )
{
    return static_cast< ::cppu::OWeakObject * >( new Bootstrap_MacroExpander( xComponentContext ) );
}

}

namespace cppu
{

//##################################################################################################
Reference< lang::XSingleComponentFactory > create_boostrap_macro_expander_factory() SAL_THROW( () )
{
    Reference< lang::XSingleComponentFactory > free(::cppu::createSingleComponentFactory(
                                                        service_create,
                                                        s_impl_name(),
                                                        s_get_service_names() ));

    uno::Environment curr_env(Environment::getCurrent());
    uno::Environment target_env(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))));

    uno::Mapping target2curr(target_env, curr_env);

    return Reference<lang::XSingleComponentFactory>(
        reinterpret_cast<lang::XSingleComponentFactory *>(
            target2curr.mapInterface(free.get(), ::getCppuType(&free))),
        SAL_NO_ACQUIRE);
}

}
