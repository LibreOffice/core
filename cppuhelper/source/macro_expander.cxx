/*************************************************************************
 *
 *  $RCSfile: macro_expander.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:52:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <rtl/bootstrap.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

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

namespace
{

// static stuff initialized when loading lib
static OUString s_impl_name = OUSTR(IMPL_NAME);
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

typedef ::cppu::WeakComponentImplHelper3<
    util::XMacroExpander, lang::XServiceInfo, lang::XInitialization > t_uno_impl;

#if defined( MACOSX ) && ( __GNUC__ < 3 )
// Initialize static template data here to break circular reference to libstatic

cppu::ClassData3 cppu::WeakComponentImplHelper3< com::sun::star::util::XMacroExpander, com::sun::star::lang::XServiceInfo, com::sun::star::lang::XInitialization >::s_aCD(1);
#endif

struct mutex_holder
{
    Mutex m_mutex;
};
class Bootstrap_MacroExpander : public mutex_holder, public t_uno_impl
{
    rtlBootstrapHandle m_bstrap;
    OUString m_rc_path;

protected:
    virtual void SAL_CALL disposing();

public:
    inline Bootstrap_MacroExpander( Reference< XComponentContext > const & ) SAL_THROW( () )
        : t_uno_impl( m_mutex ),
          m_bstrap( 0 )
        {}
    virtual ~Bootstrap_MacroExpander()
        SAL_THROW( () );

    // XMacroExpander impl
    virtual OUString SAL_CALL expandMacros( OUString const & exp )
        throw (lang::IllegalArgumentException);
    //  XInitialization impl
    virtual void SAL_CALL initialize(
        Sequence< Any > const & arguments )
        throw (Exception);
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
{
    if (m_bstrap)
    {
        rtl_bootstrap_args_close( m_bstrap );
        m_bstrap = 0;
    }
}
//__________________________________________________________________________________________________
Bootstrap_MacroExpander::~Bootstrap_MacroExpander() SAL_THROW( () )
{
    if (m_bstrap)
    {
        rtl_bootstrap_args_close( m_bstrap );
        m_bstrap = 0;
    }
}

// XServiceInfo impl
//__________________________________________________________________________________________________
OUString Bootstrap_MacroExpander::getImplementationName()
    throw (RuntimeException)
{
    return s_impl_name;
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
//  XInitialization impl
//__________________________________________________________________________________________________
void SAL_CALL Bootstrap_MacroExpander::initialize(
    Sequence< Any > const & arguments )
    throw (Exception)
{
    if (m_bstrap)
    {
        throw RuntimeException(
            OUSTR("already initialized!"),
            Reference< XInterface >() );
    }
    if (1 != arguments.getLength())
    {
        throw lang::IllegalArgumentException(
            OUSTR("invalid number of args given!  give single file url!"),
            Reference< XInterface >(),
            0 );
    }
    if (! (arguments[ 0 ] >>= m_rc_path))
    {
        throw lang::IllegalArgumentException(
            OUSTR("give file url!"),
            Reference< XInterface >(),
            0 );
    }
}

// XMacroExpander impl
//__________________________________________________________________________________________________
OUString Bootstrap_MacroExpander::expandMacros( OUString const & exp )
    throw (lang::IllegalArgumentException)
{
    // determine bootstrap handle
    rtlBootstrapHandle bstrap;
    if (m_rc_path.getLength())
    {
        // late init
        if (! m_bstrap)
        {
            rtlBootstrapHandle bstrap = rtl_bootstrap_args_open( m_rc_path.pData );
            ClearableMutexGuard guard( Mutex::getGlobalMutex() );
            if (m_bstrap)
            {
                guard.clear();
                rtl_bootstrap_args_close( bstrap );
            }
            else
            {
                m_bstrap = bstrap;
            }
        }
        bstrap = m_bstrap;
    }
    else
    {
        bstrap = ::cppu::get_unorc().getHandle();
    }

    // expand
    OUString ret( exp );
    rtl_bootstrap_expandMacros_from_handle( bstrap, &ret.pData );
    return ret;
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
    return ::cppu::createSingleComponentFactory(
        service_create,
        s_impl_name,
        s_get_service_names() );
}

}
