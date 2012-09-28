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
#include <memory>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>

#include <uno/current_context.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

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
#include "bootstrapservices.hxx"


#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SERVICE_NAME "com.sun.star.security.AccessController"
#define IMPL_NAME "com.sun.star.security.comp.stoc.AccessController"
#define USER_CREDS "access-control.user-credentials"


using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;

extern ::rtl_StandardModuleCount g_moduleCount;

namespace stoc_sec
{
// static stuff initialized when loading lib
static OUString s_envType = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
const char s_acRestriction[] = "access-control.restriction";

//##################################################################################################

/** ac context intersects permissions of two ac contexts
*/
class acc_Intersection
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

    inline acc_Intersection(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW(());

public:
    virtual ~acc_Intersection()
        SAL_THROW(());

    static inline Reference< security::XAccessControlContext > create(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW(());

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_Intersection::acc_Intersection(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW(())
    : m_x1( x1 )
    , m_x2( x2 )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
acc_Intersection::~acc_Intersection()
    SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//--------------------------------------------------------------------------------------------------
inline Reference< security::XAccessControlContext > acc_Intersection::create(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW(())
{
    if (! x1.is())
        return x2;
    if (! x2.is())
        return x1;
    return new acc_Intersection( x1, x2 );
}
//__________________________________________________________________________________________________
void acc_Intersection::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    m_x1->checkPermission( perm );
    m_x2->checkPermission( perm );
}

/** ac context unifies permissions of two ac contexts
*/
class acc_Union
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

    inline acc_Union(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW(());

public:
    virtual ~acc_Union()
        SAL_THROW(());

    static inline Reference< security::XAccessControlContext > create(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW(());

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_Union::acc_Union(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW(())
    : m_x1( x1 )
    , m_x2( x2 )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
acc_Union::~acc_Union()
    SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//--------------------------------------------------------------------------------------------------
inline Reference< security::XAccessControlContext > acc_Union::create(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW(())
{
    if (! x1.is())
        return Reference< security::XAccessControlContext >(); // unrestricted
    if (! x2.is())
        return Reference< security::XAccessControlContext >(); // unrestricted
    return new acc_Union( x1, x2 );
}
//__________________________________________________________________________________________________
void acc_Union::checkPermission(
    Any const & perm )
    throw (RuntimeException)
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
    : public WeakImplHelper1< security::XAccessControlContext >
{
    PermissionCollection m_permissions;

public:
    inline acc_Policy(
        PermissionCollection const & permissions )
        SAL_THROW(());
    virtual ~acc_Policy()
        SAL_THROW(());

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_Policy::acc_Policy(
    PermissionCollection const & permissions )
    SAL_THROW(())
    : m_permissions( permissions )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
acc_Policy::~acc_Policy()
    SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
void acc_Policy::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    m_permissions.checkPermission( perm );
}

/** current context overriding dynamic ac restriction
*/
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
        SAL_THROW(());
    virtual ~acc_CurrentContext() SAL_THROW(());

    // XInterface impl
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();

    // XCurrentContext impl
    virtual Any SAL_CALL getValueByName( OUString const & name )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_CurrentContext::acc_CurrentContext(
    Reference< XCurrentContext > const & xDelegate,
    Reference< security::XAccessControlContext > const & xRestriction )
    SAL_THROW(())
    : m_refcount( 0 )
    , m_xDelegate( xDelegate )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

    if (xRestriction.is())
    {
        m_restriction = makeAny( xRestriction );
    }
    // return empty any otherwise on getValueByName(), not null interface
}
//__________________________________________________________________________________________________
acc_CurrentContext::~acc_CurrentContext()
    SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
void acc_CurrentContext::acquire()
    throw ()
{
    ::osl_atomic_increment( &m_refcount );
}
//__________________________________________________________________________________________________
void acc_CurrentContext::release()
    throw ()
{
    if (! ::osl_atomic_decrement( &m_refcount ))
    {
        delete this;
    }
}
//__________________________________________________________________________________________________
Any acc_CurrentContext::getValueByName( OUString const & name )
    throw (RuntimeException)
{
    if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(s_acRestriction)))
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

//##################################################################################################

//--------------------------------------------------------------------------------------------------
static inline void dispose( Reference< XInterface > const & x )
    SAL_THROW( (RuntimeException) )
{
    Reference< lang::XComponent > xComp( x, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}
//--------------------------------------------------------------------------------------------------
static inline Reference< security::XAccessControlContext > getDynamicRestriction(
    Reference< XCurrentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
{
    if (xContext.is())
    {
        Any acc(xContext->getValueByName(rtl::OUString(s_acRestriction)));
        if (typelib_TypeClass_INTERFACE == acc.pType->eTypeClass)
        {
            // avoid ref-counting
            OUString const & typeName =
                *reinterpret_cast< OUString const * >( &acc.pType->pTypeName );
            if ( typeName == "com.sun.star.security.XAccessControlContext" )
            {
                return Reference< security::XAccessControlContext >(
                    *reinterpret_cast< security::XAccessControlContext ** const >( acc.pData ) );
            }
            else // try to query
            {
                return Reference< security::XAccessControlContext >::query(
                    *reinterpret_cast< XInterface ** const >( acc.pData ) );
            }
        }
    }
    return Reference< security::XAccessControlContext >();
}
//==================================================================================================
class cc_reset
{
    void * m_cc;
public:
    inline cc_reset( void * cc ) SAL_THROW(())
        : m_cc( cc ) {}
    inline ~cc_reset() SAL_THROW(())
        { ::uno_setCurrentContext( m_cc, s_envType.pData, 0 ); }
};

//##################################################################################################

struct MutexHolder
{
    Mutex m_mutex;
};
typedef WeakComponentImplHelper3<
    security::XAccessController, lang::XServiceInfo, lang::XInitialization > t_helper;

//==================================================================================================
class AccessController
    : public MutexHolder
    , public t_helper
{
    Reference< XComponentContext > m_xComponentContext;

    Reference< security::XPolicy > m_xPolicy;
    Reference< security::XPolicy > const & getPolicy()
        SAL_THROW( (RuntimeException) );

    // mode
    enum Mode { OFF, ON, DYNAMIC_ONLY, SINGLE_USER, SINGLE_DEFAULT_USER } m_mode;

    PermissionCollection m_defaultPermissions;
    // for single-user mode
    PermissionCollection m_singleUserPermissions;
    OUString m_singleUserId;
    bool m_defaultPerm_init;
    bool m_singleUser_init;
    // for multi-user mode
    lru_cache< OUString, PermissionCollection, ::rtl::OUStringHash, equal_to< OUString > >
        m_user2permissions;

    ThreadData m_rec;
    typedef vector< pair< OUString, Any > > t_rec_vec;
    inline void clearPostPoned() SAL_THROW(());
    void checkAndClearPostPoned() SAL_THROW( (RuntimeException) );

    PermissionCollection getEffectivePermissions(
        Reference< XCurrentContext > const & xContext,
        Any const & demanded_perm )
        SAL_THROW( (RuntimeException) );

protected:
    virtual void SAL_CALL disposing();

public:
    AccessController( Reference< XComponentContext > const & xComponentContext )
        SAL_THROW( (RuntimeException) );
    virtual ~AccessController()
        SAL_THROW(());

    //  XInitialization impl
    virtual void SAL_CALL initialize(
        Sequence< Any > const & arguments )
        throw (Exception);

    // XAccessController impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
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

    // XServiceInfo impl
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
AccessController::AccessController( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (RuntimeException) )
    : t_helper( m_mutex )
    , m_xComponentContext( xComponentContext )
    , m_mode( ON ) // default
    , m_defaultPerm_init( false )
    , m_singleUser_init( false )
    , m_rec( 0 )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

    OUString mode;
    if (m_xComponentContext->getValueByName( OUSTR("/services/" SERVICE_NAME "/mode") ) >>= mode)
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
                OUSTR("/services/" SERVICE_NAME "/single-user-id") ) >>= m_singleUserId;
            if (m_singleUserId.isEmpty())
            {
                throw RuntimeException(
                    OUSTR("expected a user id in component context entry "
                          "\"/services/" SERVICE_NAME "/single-user-id\"!"),
                    (OWeakObject *)this );
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
            OUSTR("/services/" SERVICE_NAME "/user-cache-size") ) >>= cacheSize))
        {
            cacheSize = 128; // reasonable default?
        }
#ifdef __CACHE_DIAGNOSE
        cacheSize = 2;
#endif
        m_user2permissions.setSize( cacheSize );
    }
}
//__________________________________________________________________________________________________
AccessController::~AccessController()
    SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
void AccessController::disposing()
{
    m_mode = OFF; // avoid checks from now on xxx todo review/ better DYNAMIC_ONLY?
    m_xPolicy.clear();
    m_xComponentContext.clear();
}

// XInitialization impl
//__________________________________________________________________________________________________
void AccessController::initialize(
    Sequence< Any > const & arguments )
    throw (Exception)
{
    // xxx todo: review for forking
    // portal forking hack: re-initialize for another user-id
    if (SINGLE_USER != m_mode) // only if in single-user mode
    {
        throw RuntimeException(
            OUSTR("invalid call: ac must be in \"single-user\" mode!"), (OWeakObject *)this );
    }
    OUString userId;
    arguments[ 0 ] >>= userId;
    if ( userId.isEmpty() )
    {
        throw RuntimeException(
            OUSTR("expected a user-id as first argument!"), (OWeakObject *)this );
    }
    // assured that no sync is necessary: no check happens at this forking time
    m_singleUserId = userId;
    m_singleUser_init = false;
}

//__________________________________________________________________________________________________
Reference< security::XPolicy > const & AccessController::getPolicy()
    SAL_THROW( (RuntimeException) )
{
    // get policy singleton
    if (! m_xPolicy.is())
    {
        Reference< security::XPolicy > xPolicy;
        m_xComponentContext->getValueByName(
            OUSTR("/singletons/com.sun.star.security.thePolicy") ) >>= xPolicy;
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
                OUSTR("cannot get policy singleton!"), (OWeakObject *)this );
        }
    }
    return m_xPolicy;
}

#ifdef __DIAGNOSE
static void dumpPermissions(
    PermissionCollection const & collection, OUString const & userId = OUString() ) SAL_THROW(())
{
    OUStringBuffer buf( 48 );
    if (!userId.isEmpty())
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> dumping permissions of user \"") );
        buf.append( userId );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\":") );
    }
    else
    {
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("> dumping default permissions:") );
    }
    OString str( ::rtl::OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "%s", str.getStr() );
    Sequence< OUString > permissions( collection.toStrings() );
    OUString const * p = permissions.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < permissions.getLength(); ++nPos )
    {
        OString str( ::rtl::OUStringToOString( p[ nPos ], RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "%s", str.getStr() );
    }
    OSL_TRACE( "> permission dump done" );
}
#endif


//__________________________________________________________________________________________________
inline void AccessController::clearPostPoned() SAL_THROW(())
{
    delete reinterpret_cast< t_rec_vec * >( m_rec.getData() );
    m_rec.setData( 0 );
}
//__________________________________________________________________________________________________
void AccessController::checkAndClearPostPoned() SAL_THROW( (RuntimeException) )
{
    // check postponed permissions
    auto_ptr< t_rec_vec > rec( reinterpret_cast< t_rec_vec * >( m_rec.getData() ) );
    m_rec.setData( 0 ); // takeover ownership
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
//__________________________________________________________________________________________________
/** this is the only function calling the policy singleton and thus has to take care
    of recurring calls!

    @param demanded_perm (if not empty) is the demanded permission of a checkPermission() call
                         which will be postponed for recurring calls
*/
PermissionCollection AccessController::getEffectivePermissions(
    Reference< XCurrentContext > const & xContext,
    Any const & demanded_perm )
    SAL_THROW( (RuntimeException) )
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
            xContext->getValueByName( OUSTR(USER_CREDS ".id") ) >>= userId;
        }
        if ( userId.isEmpty() )
        {
            throw SecurityException(
                OUSTR("cannot determine current user in multi-user ac!"), (OWeakObject *)this );
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
    t_rec_vec * rec = reinterpret_cast< t_rec_vec * >( m_rec.getData() );
    if (rec) // tls entry exists => this is recursive call
    {
        if (demanded_perm.hasValue())
        {
            // enqueue
            rec->push_back( pair< OUString, Any >( userId, demanded_perm ) );
        }
#ifdef __DIAGNOSE
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("> info: recurring call of user \"") );
        buf.append( userId );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"") );
        OString str(
            ::rtl::OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );
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
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("deployment error (AccessControlException occurred): ") );
        buf.append( exc.Message );
        throw DeploymentException( buf.makeStringAndClear(), exc.Context );
    }
    catch (RuntimeException &)
    {
        // dont check postponed, just cleanup
        clearPostPoned();
        delete reinterpret_cast< t_rec_vec * >( m_rec.getData() );
        m_rec.setData( 0 );
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
        // dont check postponed, just cleanup
        clearPostPoned();
        throw;
    }
}

// XAccessController impl
//__________________________________________________________________________________________________
void AccessController::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("checkPermission() call on disposed AccessController!"), (OWeakObject *)this );
    }

    if (OFF == m_mode)
        return;

    // first dynamic check of ac contexts
    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );
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
//__________________________________________________________________________________________________
Any AccessController::doRestricted(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("doRestricted() call on disposed AccessController!"), (OWeakObject *)this );
    }

    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
        return xAction->run();

    if (xRestriction.is())
    {
        Reference< XCurrentContext > xContext;
        ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

        // override restriction
        Reference< XCurrentContext > xNewContext(
            new acc_CurrentContext( xContext, acc_Intersection::create(
                                        xRestriction, getDynamicRestriction( xContext ) ) ) );
        ::uno_setCurrentContext( xNewContext.get(), s_envType.pData, 0 );
        cc_reset reset( xContext.get() );
        return xAction->run();
    }
    else
    {
        return xAction->run();
    }
}
//__________________________________________________________________________________________________
Any AccessController::doPrivileged(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("doPrivileged() call on disposed AccessController!"), (OWeakObject *)this );
    }

    if (OFF == m_mode) // no dynamic check will be performed
    {
        return xAction->run();
    }

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

    Reference< security::XAccessControlContext > xOldRestr(
        getDynamicRestriction( xContext ) );

    if (xOldRestr.is()) // previous restriction
    {
        // override restriction
        Reference< XCurrentContext > xNewContext(
            new acc_CurrentContext( xContext, acc_Union::create( xRestriction, xOldRestr ) ) );
        ::uno_setCurrentContext( xNewContext.get(), s_envType.pData, 0 );
        cc_reset reset( xContext.get() );
        return xAction->run();
    }
    else // no previous restriction => never current restriction
    {
        return xAction->run();
    }
}
//__________________________________________________________________________________________________
Reference< security::XAccessControlContext > AccessController::getContext()
    throw (RuntimeException)
{
    if (rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("getContext() call on disposed AccessController!"), (OWeakObject *)this );
    }

    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
    {
        return new acc_Policy( PermissionCollection( new AllPermission() ) );
    }

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

    return acc_Intersection::create(
        getDynamicRestriction( xContext ),
        new acc_Policy( getEffectivePermissions( xContext, Any() ) ) );
}

// XServiceInfo impl
//__________________________________________________________________________________________________
OUString AccessController::getImplementationName()
    throw (RuntimeException)
{
    return stoc_bootstrap::ac_getImplementationName();
}
//__________________________________________________________________________________________________
sal_Bool AccessController::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pNames = aSNL.getConstArray();
    for ( sal_Int32 nPos = aSNL.getLength(); --nPos; )
    {
        if (serviceName.equals( pNames[ nPos ] ))
        {
            return sal_True;
        }
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > AccessController::getSupportedServiceNames()
    throw (RuntimeException)
{
    return stoc_bootstrap::ac_getSupportedServiceNames();
}
}
//##################################################################################################
namespace stoc_bootstrap {
//--------------------------------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ac_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return (OWeakObject *)new stoc_sec::AccessController( xComponentContext );
}
//--------------------------------------------------------------------------------------------------
Sequence< OUString > ac_getSupportedServiceNames() SAL_THROW(())
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME));
    return aSNS;
}
//--------------------------------------------------------------------------------------------------
OUString ac_getImplementationName() SAL_THROW(())
{
    return OUSTR(IMPL_NAME);
}
//--------------------------------------------------------------------------------------------------
Reference< XInterface > SAL_CALL filepolicy_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) );
//--------------------------------------------------------------------------------------------------
Sequence< OUString > filepolicy_getSupportedServiceNames() SAL_THROW(());
//--------------------------------------------------------------------------------------------------
OUString filepolicy_getImplementationName() SAL_THROW(());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
