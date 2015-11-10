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


#include <vector>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <uno/current_context.h>
#include <uno/lbnames.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/XPolicy.hpp>

#include "lru_cache.h"
#include "permissions.h"

#include <memory>

#define SERVICE_NAME "com.sun.star.security.AccessController"
#define USER_CREDS "access-control.user-credentials"


using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace css::uno;
using namespace stoc_sec;

namespace {

// static stuff initialized when loading lib
static OUString s_envType = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
const char s_acRestriction[] = "access-control.restriction";



/** ac context intersects permissions of two ac contexts
*/
class acc_Intersection
    : public WeakImplHelper< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

    inline acc_Intersection(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 );

public:
    virtual ~acc_Intersection();

    static inline Reference< security::XAccessControlContext > create(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 );

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException, std::exception) override;
};

inline acc_Intersection::acc_Intersection(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    : m_x1( x1 )
    , m_x2( x2 )
{}

acc_Intersection::~acc_Intersection()
{}

inline Reference< security::XAccessControlContext > acc_Intersection::create(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
{
    if (! x1.is())
        return x2;
    if (! x2.is())
        return x1;
    return new acc_Intersection( x1, x2 );
}

void acc_Intersection::checkPermission(
    Any const & perm )
    throw (RuntimeException, std::exception)
{
    m_x1->checkPermission( perm );
    m_x2->checkPermission( perm );
}

/** ac context unifies permissions of two ac contexts
*/
class acc_Union
    : public WeakImplHelper< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

    inline acc_Union(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 );

public:
    virtual ~acc_Union();

    static inline Reference< security::XAccessControlContext > create(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 );

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException, std::exception) override;
};

inline acc_Union::acc_Union(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    : m_x1( x1 )
    , m_x2( x2 )
{}

acc_Union::~acc_Union()
{}

inline Reference< security::XAccessControlContext > acc_Union::create(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
{
    if (! x1.is())
        return Reference< security::XAccessControlContext >(); // unrestricted
    if (! x2.is())
        return Reference< security::XAccessControlContext >(); // unrestricted
    return new acc_Union( x1, x2 );
}

void acc_Union::checkPermission(
    Any const & perm )
    throw (RuntimeException, std::exception)
{
    try
    {
        m_x1->checkPermission( perm );
    }
    catch (security::AccessControlException &)
    {
        m_x2->checkPermission( perm );
    }
}

/** ac context doing permission checks on static permissions
*/
class acc_Policy
    : public WeakImplHelper< security::XAccessControlContext >
{
    PermissionCollection m_permissions;

public:
    explicit acc_Policy(
        PermissionCollection const & permissions )
        : m_permissions( permissions )
    {}

    virtual ~acc_Policy();

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException, std::exception) override;
};

acc_Policy::~acc_Policy()
{}

void acc_Policy::checkPermission(
    Any const & perm )
    throw (RuntimeException, std::exception)
{
    m_permissions.checkPermission( perm );
}

/** current context overriding dynamic ac restriction
*/
class acc_CurrentContext
    : public WeakImplHelper< XCurrentContext >
{
    Reference< XCurrentContext > m_xDelegate;
    Any m_restriction;

public:
    inline acc_CurrentContext(
        Reference< XCurrentContext > const & xDelegate,
        Reference< security::XAccessControlContext > const & xRestriction );

    // XCurrentContext impl
    virtual Any SAL_CALL getValueByName( OUString const & name )
        throw (RuntimeException, std::exception) override;
};

inline acc_CurrentContext::acc_CurrentContext(
    Reference< XCurrentContext > const & xDelegate,
    Reference< security::XAccessControlContext > const & xRestriction )
    : m_xDelegate( xDelegate )
{
    if (xRestriction.is())
    {
        m_restriction = makeAny( xRestriction );
    }
    // return empty any otherwise on getValueByName(), not null interface
}

Any acc_CurrentContext::getValueByName( OUString const & name )
    throw (RuntimeException, std::exception)
{
    if (name == s_acRestriction)
    {
        return m_restriction;
    }
    else if (m_xDelegate.is())
    {
        return m_xDelegate->getValueByName( name );
    }
    else
    {
        return Any();
    }
}



static inline Reference< security::XAccessControlContext > getDynamicRestriction(
    Reference< XCurrentContext > const & xContext )
{
    if (xContext.is())
    {
        Any acc(xContext->getValueByName(s_acRestriction));
        if (typelib_TypeClass_INTERFACE == acc.pType->eTypeClass)
        {
            // avoid ref-counting
            OUString const & typeName =
                OUString::unacquired( &acc.pType->pTypeName );
            if ( typeName == "com.sun.star.security.XAccessControlContext" )
            {
                return Reference< security::XAccessControlContext >(
                    *static_cast< security::XAccessControlContext ** const >( acc.pData ) );
            }
            else // try to query
            {
                return Reference< security::XAccessControlContext >::query(
                    *static_cast< XInterface ** const >( acc.pData ) );
            }
        }
    }
    return Reference< security::XAccessControlContext >();
}

class cc_reset
{
    void * m_cc;
public:
    explicit cc_reset( void * cc )
        : m_cc( cc ) {}
    inline ~cc_reset()
        { ::uno_setCurrentContext( m_cc, s_envType.pData, nullptr ); }
};

struct MutexHolder
{
    Mutex m_mutex;
};
typedef WeakComponentImplHelper<
    security::XAccessController, lang::XServiceInfo, lang::XInitialization > t_helper;


class AccessController
    : public MutexHolder
    , public t_helper
{
    Reference< XComponentContext > m_xComponentContext;

    Reference< security::XPolicy > m_xPolicy;
    Reference< security::XPolicy > const & getPolicy();

    // mode
    enum Mode { OFF, ON, DYNAMIC_ONLY, SINGLE_USER, SINGLE_DEFAULT_USER } m_mode;

    PermissionCollection m_defaultPermissions;
    // for single-user mode
    PermissionCollection m_singleUserPermissions;
    OUString m_singleUserId;
    bool m_defaultPerm_init;
    bool m_singleUser_init;
    // for multi-user mode
    lru_cache< OUString, PermissionCollection, OUStringHash, equal_to< OUString > >
        m_user2permissions;

    ThreadData m_rec;
    typedef vector< pair< OUString, Any > > t_rec_vec;
    inline void clearPostPoned();
    void checkAndClearPostPoned();

    PermissionCollection getEffectivePermissions(
        Reference< XCurrentContext > const & xContext,
        Any const & demanded_perm );

protected:
    virtual void SAL_CALL disposing() override;

public:
    explicit AccessController( Reference< XComponentContext > const & xComponentContext );
    virtual ~AccessController();

    //  XInitialization impl
    virtual void SAL_CALL initialize(
        Sequence< Any > const & arguments )
        throw (Exception, std::exception) override;

    // XAccessController impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException, std::exception) override;
    virtual Any SAL_CALL doRestricted(
        Reference< security::XAction > const & xAction,
        Reference< security::XAccessControlContext > const & xRestriction )
        throw (Exception, std::exception) override;
    virtual Any SAL_CALL doPrivileged(
        Reference< security::XAction > const & xAction,
        Reference< security::XAccessControlContext > const & xRestriction )
        throw (Exception, std::exception) override;
    virtual Reference< security::XAccessControlContext > SAL_CALL getContext()
        throw (RuntimeException, std::exception) override;

    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException, std::exception) override;
};

AccessController::AccessController( Reference< XComponentContext > const & xComponentContext )
    : t_helper( m_mutex )
    , m_xComponentContext( xComponentContext )
    , m_mode( ON ) // default
    , m_defaultPerm_init( false )
    , m_singleUser_init( false )
    , m_rec( nullptr )
{
    // The .../mode value had originally been set in
    // cppu::add_access_control_entries (cppuhelper/source/servicefactory.cxx)
    // to something other than "off" depending on various UNO_AC* bootstrap
    // variables that are no longer supported, so this is mostly dead code now:
    OUString mode;
    if (m_xComponentContext->getValueByName( "/services/" SERVICE_NAME "/mode" ) >>= mode)
    {
        if ( mode == "off" )
        {
            m_mode = OFF;
        }
        else if ( mode == "on" )
        {
            m_mode = ON;
        }
        else if ( mode == "dynamic-only" )
        {
            m_mode = DYNAMIC_ONLY;
        }
        else if ( mode == "single-user" )
        {
            m_xComponentContext->getValueByName(
                "/services/" SERVICE_NAME "/single-user-id" ) >>= m_singleUserId;
            if (m_singleUserId.isEmpty())
            {
                throw RuntimeException(
                    "expected a user id in component context entry "
                    "\"/services/" SERVICE_NAME "/single-user-id\"!",
                    static_cast<OWeakObject *>(this) );
            }
            m_mode = SINGLE_USER;
        }
        else if ( mode == "single-default-user" )
        {
            m_mode = SINGLE_DEFAULT_USER;
        }
    }

    // switch on caching for DYNAMIC_ONLY and ON (sharable multi-user process)
    if (ON == m_mode || DYNAMIC_ONLY == m_mode)
    {
        sal_Int32 cacheSize = 0; // multi-user cache size
        if (! (m_xComponentContext->getValueByName(
            "/services/" SERVICE_NAME "/user-cache-size" ) >>= cacheSize))
        {
            cacheSize = 128; // reasonable default?
        }
#ifdef __CACHE_DIAGNOSE
        cacheSize = 2;
#endif
        m_user2permissions.setSize( cacheSize );
    }
}

AccessController::~AccessController()
{}

void AccessController::disposing()
{
    m_mode = OFF; // avoid checks from now on xxx todo review/ better DYNAMIC_ONLY?
    m_xPolicy.clear();
    m_xComponentContext.clear();
}

// XInitialization impl

void AccessController::initialize(
    Sequence< Any > const & arguments )
    throw (Exception, std::exception)
{
    // xxx todo: review for forking
    // portal forking hack: re-initialize for another user-id
    if (SINGLE_USER != m_mode) // only if in single-user mode
    {
        throw RuntimeException(
            "invalid call: ac must be in \"single-user\" mode!", static_cast<OWeakObject *>(this) );
    }
    OUString userId;
    arguments[ 0 ] >>= userId;
    if ( userId.isEmpty() )
    {
        throw RuntimeException(
            "expected a user-id as first argument!", static_cast<OWeakObject *>(this) );
    }
    // assured that no sync is necessary: no check happens at this forking time
    m_singleUserId = userId;
    m_singleUser_init = false;
}


Reference< security::XPolicy > const & AccessController::getPolicy()
{
    // get policy singleton
    if (! m_xPolicy.is())
    {
        Reference< security::XPolicy > xPolicy;
        m_xComponentContext->getValueByName(
            "/singletons/com.sun.star.security.thePolicy" ) >>= xPolicy;
        if (xPolicy.is())
        {
            MutexGuard guard( m_mutex );
            if (! m_xPolicy.is())
            {
                m_xPolicy = xPolicy;
            }
        }
        else
        {
            throw SecurityException(
                "cannot get policy singleton!", static_cast<OWeakObject *>(this) );
        }
    }
    return m_xPolicy;
}

#ifdef __DIAGNOSE
static void dumpPermissions(
    PermissionCollection const & collection, OUString const & userId = OUString() )
{
    OUStringBuffer buf( 48 );
    if (!userId.isEmpty())
    {
        buf.append( "> dumping permissions of user \"" );
        buf.append( userId );
        buf.append( "\":" );
    }
    else
    {
        buf.append( "> dumping default permissions:" );
    }
    OString str( OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", str.getStr() );
    Sequence< OUString > permissions( collection.toStrings() );
    OUString const * p = permissions.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < permissions.getLength(); ++nPos )
    {
        OString str( OUStringToOString( p[ nPos ], RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "%s", str.getStr() );
    }
    OSL_TRACE( "> permission dump done" );
}
#endif



inline void AccessController::clearPostPoned()
{
    delete static_cast< t_rec_vec * >( m_rec.getData() );
    m_rec.setData( nullptr );
}

void AccessController::checkAndClearPostPoned()
{
    // check postponed permissions
    std::unique_ptr< t_rec_vec > rec( static_cast< t_rec_vec * >( m_rec.getData() ) );
    m_rec.setData( nullptr ); // takeover ownership
    OSL_ASSERT( rec.get() );
    if (rec.get())
    {
        t_rec_vec const & vec = *rec.get();
        switch (m_mode)
        {
        case SINGLE_USER:
        {
            OSL_ASSERT( m_singleUser_init );
            for ( size_t nPos = 0; nPos < vec.size(); ++nPos )
            {
                pair< OUString, Any > const & p = vec[ nPos ];
                OSL_ASSERT( m_singleUserId.equals( p.first ) );
                m_singleUserPermissions.checkPermission( p.second );
            }
            break;
        }
        case SINGLE_DEFAULT_USER:
        {
            OSL_ASSERT( m_defaultPerm_init );
            for ( size_t nPos = 0; nPos < vec.size(); ++nPos )
            {
                pair< OUString, Any > const & p = vec[ nPos ];
                OSL_ASSERT( p.first.isEmpty() ); // default-user
                m_defaultPermissions.checkPermission( p.second );
            }
            break;
        }
        case ON:
        {
            for ( size_t nPos = 0; nPos < vec.size(); ++nPos )
            {
                pair< OUString, Any > const & p = vec[ nPos ];
                PermissionCollection const * pPermissions;
                // lookup policy for user
                {
                    MutexGuard guard( m_mutex );
                    pPermissions = m_user2permissions.lookup( p.first );
                }
                OSL_ASSERT( pPermissions );
                if (pPermissions)
                {
                    pPermissions->checkPermission( p.second );
                }
            }
            break;
        }
        default:
            OSL_FAIL( "### this should never be called in this ac mode!" );
            break;
        }
    }
}

/** this is the only function calling the policy singleton and thus has to take care
    of recurring calls!

    @param demanded_perm (if not empty) is the demanded permission of a checkPermission() call
                         which will be postponed for recurring calls
*/
PermissionCollection AccessController::getEffectivePermissions(
    Reference< XCurrentContext > const & xContext,
    Any const & demanded_perm )
{
    OUString userId;

    switch (m_mode)
    {
    case SINGLE_USER:
    {
        if (m_singleUser_init)
            return m_singleUserPermissions;
        userId = m_singleUserId;
        break;
    }
    case SINGLE_DEFAULT_USER:
    {
        if (m_defaultPerm_init)
            return m_defaultPermissions;
        break;
    }
    case ON:
    {
        if (xContext.is())
        {
            xContext->getValueByName( USER_CREDS ".id" ) >>= userId;
        }
        if ( userId.isEmpty() )
        {
            throw SecurityException(
                "cannot determine current user in multi-user ac!", static_cast<OWeakObject *>(this) );
        }

        // lookup policy for user
        MutexGuard guard( m_mutex );
        PermissionCollection const * pPermissions = m_user2permissions.lookup( userId );
        if (pPermissions)
            return *pPermissions;
        break;
    }
    default:
        OSL_FAIL( "### this should never be called in this ac mode!" );
        return PermissionCollection();
    }

    // call on policy
    // iff this is a recurring call for the default user, then grant all permissions
    t_rec_vec * rec = static_cast< t_rec_vec * >( m_rec.getData() );
    if (rec) // tls entry exists => this is recursive call
    {
        if (demanded_perm.hasValue())
        {
            // enqueue
            rec->push_back( pair< OUString, Any >( userId, demanded_perm ) );
        }
#ifdef __DIAGNOSE
        OUStringBuffer buf( 48 );
        buf.append( "> info: recurring call of user \"" );
        buf.append( userId );
        buf.append( "\"" );
        OString str(
            OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "%s", str.getStr() );
#endif
        return PermissionCollection( new AllPermission() );
    }
    else // no tls
    {
        rec = new t_rec_vec;
        m_rec.setData( rec );
    }

    try // calls on API
    {
        // init default permissions
        if (! m_defaultPerm_init)
        {
            PermissionCollection defaultPermissions(
                getPolicy()->getDefaultPermissions() );
            // assign
            MutexGuard guard( m_mutex );
            if (! m_defaultPerm_init)
            {
                m_defaultPermissions = defaultPermissions;
                m_defaultPerm_init = true;
            }
#ifdef __DIAGNOSE
            dumpPermissions( m_defaultPermissions );
#endif
        }

        PermissionCollection ret;

        // init user permissions
        switch (m_mode)
        {
        case SINGLE_USER:
        {
            ret = PermissionCollection(
                getPolicy()->getPermissions( userId ), m_defaultPermissions );
            {
            // assign
            MutexGuard guard( m_mutex );
            if (m_singleUser_init)
            {
                ret = m_singleUserPermissions;
            }
            else
            {
                m_singleUserPermissions = ret;
                m_singleUser_init = true;
            }
            }
#ifdef __DIAGNOSE
            dumpPermissions( ret, userId );
#endif
            break;
        }
        case SINGLE_DEFAULT_USER:
        {
            ret = m_defaultPermissions;
            break;
        }
        case ON:
        {
            ret = PermissionCollection(
                getPolicy()->getPermissions( userId ), m_defaultPermissions );
            {
            // cache
            MutexGuard guard( m_mutex );
            m_user2permissions.set( userId, ret );
            }
#ifdef __DIAGNOSE
            dumpPermissions( ret, userId );
#endif
            break;
        }
        default:
            break;
        }

        // check postponed
        checkAndClearPostPoned();
        return ret;
    }
    catch (const security::AccessControlException & exc) // wrapped into DeploymentException
    {
        clearPostPoned(); // safety: exception could have happened before checking postponed?
        OUStringBuffer buf( 64 );
        buf.append( "deployment error (AccessControlException occurred): " );
        buf.append( exc.Message );
        throw DeploymentException( buf.makeStringAndClear(), exc.Context );
    }
    catch (RuntimeException &)
    {
        // don't check postponed, just cleanup
        clearPostPoned();
        delete static_cast< t_rec_vec * >( m_rec.getData() );
        m_rec.setData( nullptr );
        throw;
    }
    catch (Exception &)
    {
        // check postponed permissions first
        // => AccessControlExceptions are errors, user exceptions not!
        checkAndClearPostPoned();
        throw;
    }
    catch (...)
    {
        // don't check postponed, just cleanup
        clearPostPoned();
        throw;
    }
}

// XAccessController impl

void AccessController::checkPermission(
    Any const & perm )
    throw (RuntimeException, std::exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            "checkPermission() call on disposed AccessController!", static_cast<OWeakObject *>(this) );
    }

    if (OFF == m_mode)
        return;

    // first dynamic check of ac contexts
    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( reinterpret_cast<void **>(&xContext), s_envType.pData, nullptr );
    Reference< security::XAccessControlContext > xACC( getDynamicRestriction( xContext ) );
    if (xACC.is())
    {
        xACC->checkPermission( perm );
    }

    if (DYNAMIC_ONLY == m_mode)
        return;

    // then static check
    getEffectivePermissions( xContext, perm ).checkPermission( perm );
}

Any AccessController::doRestricted(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception, std::exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            "doRestricted() call on disposed AccessController!", static_cast<OWeakObject *>(this) );
    }

    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
        return xAction->run();

    if (xRestriction.is())
    {
        Reference< XCurrentContext > xContext;
        ::uno_getCurrentContext( reinterpret_cast<void **>(&xContext), s_envType.pData, nullptr );

        // override restriction
        Reference< XCurrentContext > xNewContext(
            new acc_CurrentContext( xContext, acc_Intersection::create(
                                        xRestriction, getDynamicRestriction( xContext ) ) ) );
        ::uno_setCurrentContext( xNewContext.get(), s_envType.pData, nullptr );
        cc_reset reset( xContext.get() );
        return xAction->run();
    }
    else
    {
        return xAction->run();
    }
}

Any AccessController::doPrivileged(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception, std::exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            "doPrivileged() call on disposed AccessController!", static_cast<OWeakObject *>(this) );
    }

    if (OFF == m_mode) // no dynamic check will be performed
    {
        return xAction->run();
    }

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( reinterpret_cast<void **>(&xContext), s_envType.pData, nullptr );

    Reference< security::XAccessControlContext > xOldRestr(
        getDynamicRestriction( xContext ) );

    if (xOldRestr.is()) // previous restriction
    {
        // override restriction
        Reference< XCurrentContext > xNewContext(
            new acc_CurrentContext( xContext, acc_Union::create( xRestriction, xOldRestr ) ) );
        ::uno_setCurrentContext( xNewContext.get(), s_envType.pData, nullptr );
        cc_reset reset( xContext.get() );
        return xAction->run();
    }
    else // no previous restriction => never current restriction
    {
        return xAction->run();
    }
}

Reference< security::XAccessControlContext > AccessController::getContext()
    throw (RuntimeException, std::exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            "getContext() call on disposed AccessController!", static_cast<OWeakObject *>(this) );
    }

    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
    {
        return new acc_Policy( PermissionCollection( new AllPermission() ) );
    }

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( reinterpret_cast<void **>(&xContext), s_envType.pData, nullptr );

    return acc_Intersection::create(
        getDynamicRestriction( xContext ),
        new acc_Policy( getEffectivePermissions( xContext, Any() ) ) );
}

// XServiceInfo impl

OUString AccessController::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.security.comp.stoc.AccessController");
}

sal_Bool AccessController::supportsService( OUString const & serviceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > AccessController::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = SERVICE_NAME;
    return aSNS;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_security_comp_stoc_AccessController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new AccessController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
