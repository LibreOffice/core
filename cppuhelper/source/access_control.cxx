/*************************************************************************
 *
 *  $RCSfile: access_control.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-12-14 13:19:51 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <uno/current_context.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/security/XAccessController.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#define AC_RESTRICTION "ac-restriction"
#define AC_SERVICE "com.sun.star.security.AccessController"


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace cppu
{

static OUString str_envType = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
static OUString str_acRestriction = OUSTR(AC_RESTRICTION);

//==================================================================================================
class acc_Combiner
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

public:
    inline acc_Combiner(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW( () )
        : m_x1( x1 )
        , m_x2( x2 )
        {}

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        security::Permission const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void acc_Combiner::checkPermission(
    security::Permission const & perm )
    throw (RuntimeException)
{
    m_x1->checkPermission( perm );
    m_x2->checkPermission( perm );
}
//==================================================================================================
class acc_CurrentContext
    : public ImplHelper1< XCurrentContext >
{
    oslInterlockedCount m_refcount;

    Reference< XCurrentContext > m_xDelegate;
    Any m_restriction;

public:
    inline acc_CurrentContext(
        Reference< XCurrentContext > const & xDelegate,
        Reference< security::XAccessControlContext > const & xRestriction )
        SAL_THROW( () );

    // XInterface impl
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();

    // XCurrentContext
    virtual Any SAL_CALL getValueByName( OUString const & name )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_CurrentContext::acc_CurrentContext(
    Reference< XCurrentContext > const & xDelegate,
    Reference< security::XAccessControlContext > const & xRestriction )
    SAL_THROW( () )
    : m_refcount( 0 )
    , m_xDelegate( xDelegate )
{
    if (xRestriction.is())
    {
        m_restriction = makeAny( xRestriction );
    }
    // return empty any otherwise on getValueByName(), not null interface
}
//__________________________________________________________________________________________________
void acc_CurrentContext::acquire()
    throw ()
{
    ::osl_incrementInterlockedCount( &m_refcount );
}
//__________________________________________________________________________________________________
void acc_CurrentContext::release()
    throw ()
{
    if (! ::osl_decrementInterlockedCount( &m_refcount ))
    {
        delete this;
    }
}
//__________________________________________________________________________________________________
Any acc_CurrentContext::getValueByName( OUString const & name )
    throw (RuntimeException)
{
    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(AC_RESTRICTION) ))
    {
        return m_restriction;
    }
    else
    {
        return m_xDelegate->getValueByName( name );
    }
}

//--------------------------------------------------------------------------------------------------
static inline Reference< security::XAccessControlContext > getCurrentRestriction(
    Reference< XCurrentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
{
    if (xContext.is())
    {
        Any acc( xContext->getValueByName( str_acRestriction ) );
        if (typelib_TypeClass_INTERFACE == acc.pType->eTypeClass)
        {
            OUString const & typeName =
                * reinterpret_cast< OUString const * >( &acc.pType->pTypeName );
            if (typeName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.XAccessControlContext") ))
            {
                return Reference< security::XAccessControlContext >(
                    * reinterpret_cast< security::XAccessControlContext * * const >( acc.pData ) );
            }
            else // try to query
            {
                return Reference< security::XAccessControlContext >::query(
                    * reinterpret_cast< XInterface * * const >( acc.pData ) );
            }
        }
    }
    return Reference< security::XAccessControlContext >();
}
//==================================================================================================
Reference< security::XAccessControlContext > SAL_CALL ac_defimpl_getCurrentRestriction()
    SAL_THROW( () )
{
    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, str_envType.pData, 0 );
    return getCurrentRestriction( xContext );
}
//==================================================================================================
struct __cc_reset
{
    void * m_cc;
    inline __cc_reset( void * cc ) SAL_THROW( () )
        : m_cc( cc ) {}
    inline ~__cc_reset() SAL_THROW( () )
        { ::uno_setCurrentContext( m_cc, str_envType.pData, 0 ); }
};
//==================================================================================================
Any SAL_CALL ac_defimpl_doRestricted(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    SAL_THROW( (Exception) )
{
    if (xRestriction.is())
    {
        Reference< XCurrentContext > xOldContext;
        ::uno_getCurrentContext( (void **)&xOldContext, str_envType.pData, 0 );
        Reference< security::XAccessControlContext > xOldRestr(
            getCurrentRestriction( xOldContext ) );

        if (xOldRestr.is())
        {
            Reference< XCurrentContext > xNewContext( new acc_CurrentContext(
                xOldContext, new acc_Combiner( xRestriction, xOldRestr ) ) );
            ::uno_setCurrentContext( xNewContext.get(), str_envType.pData, 0 );
        }
        else
        {
            Reference< XCurrentContext > xNewContext( new acc_CurrentContext(
                xOldContext, xRestriction ) );
            ::uno_setCurrentContext( xNewContext.get(), str_envType.pData, 0 );
        }

        __cc_reset reset( xOldContext.get() );
        return xAction->run();
    }
    else
    {
        return xAction->run();
    }
}
//==================================================================================================
Any SAL_CALL ac_defimpl_doPrivileged(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    SAL_THROW( (Exception) )
{
    Reference< XCurrentContext > xOldContext;
    ::uno_getCurrentContext( (void **)&xOldContext, str_envType.pData, 0 );

    // override AC_RESTRICTION
    Reference< XCurrentContext > xContext( new acc_CurrentContext(
        xOldContext, xRestriction ) );
    ::uno_setCurrentContext( xContext.get(), str_envType.pData, 0 );

    __cc_reset reset( xOldContext.get() );
    return xAction->run();
}


//##################################################################################################
//### default service impl #########################################################################
//##################################################################################################

//==================================================================================================
class DefaultAccessController
    : public WeakImplHelper1< security::XAccessController >
{
public:
    // XAccessController impl
    virtual void SAL_CALL checkPermission(
        security::Permission const & perm )
        throw (RuntimeException);
    virtual Any SAL_CALL doRestricted(
        Reference< security::XAction > const & xAction,
        Reference< security::XAccessControlContext > const & xRestriction )
        throw (Exception);
    virtual Any SAL_CALL doPrivileged(
        Reference< security::XAction > const & xAction,
        Reference< security::XAccessControlContext > const & xRestriction )
        throw (Exception);
    virtual Reference< security::XAccessControlContext > SAL_CALL getContext()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void DefaultAccessController::checkPermission(
    security::Permission const & perm )
    throw (RuntimeException)
{
    // only dynamic checks of ac contexts, no static checks concerning credentials
    Reference< security::XAccessControlContext > xACC( ac_defimpl_getCurrentRestriction() );
    if (xACC.is())
    {
        xACC->checkPermission( perm );
    }
}
//__________________________________________________________________________________________________
Any DefaultAccessController::doRestricted(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception)
{
    return ac_defimpl_doRestricted( xAction, xRestriction );
}
//__________________________________________________________________________________________________
Any DefaultAccessController::doPrivileged(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception)
{
    return ac_defimpl_doPrivileged( xAction, xRestriction );
}
//__________________________________________________________________________________________________
Reference< security::XAccessControlContext > DefaultAccessController::getContext()
    throw (RuntimeException)
{
    return ac_defimpl_getCurrentRestriction();
}

//--------------------------------------------------------------------------------------------------
static Reference< XInterface > SAL_CALL create_default_ac(
    Reference< XComponentContext > const & )
    SAL_THROW( (Exception) )
{
    return (OWeakObject *)new DefaultAccessController();
}
//=== run on bootstrapping =========================================================================
Reference< lang::XSingleComponentFactory > createDefaultAccessController()
    SAL_THROW( () )
{
    OUString serviceName( RTL_CONSTASCII_USTRINGPARAM(AC_SERVICE) );
    return createSingleComponentFactory(
        create_default_ac,
        OUSTR("com.sun.star.comp.security.DummyAccessController"),
        Sequence< OUString >( &serviceName, 1 ) );
}

}

