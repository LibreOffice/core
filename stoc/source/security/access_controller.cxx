/*************************************************************************
 *
 *  $RCSfile: access_controller.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-01-25 09:29:35 $
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

#include <hash_set>
#include <hash_map>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/process.h>
#include <osl/thread.hxx>
#include <rtl/ustrbuf.hxx>
#include <uno/current_context.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/XPolicy.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/security/AllPermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SERVICE_NAME "com.sun.star.security.AccessController"
#define IMPL_NAME "com.sun.star.security.comp.stoc.AccessController"
#define USER_CREDS "access-control.user-credentials"

// xxx todo deliver dll, ingo

using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace stoc_security
{

// static stuff initialized when loading lib
static OUString s_envType = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
static OUString s_implName = OUSTR(IMPL_NAME);
static OUString s_serviceName = OUSTR(SERVICE_NAME);
static OUString s_acRestriction = OUSTR("access-control.restriction");

static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );
rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

static OUString s_filePermission = OUSTR("com.sun.star.io.FilePermission");
static OUString s_socketPermission = OUSTR("com.sun.star.connection,SocketPermission");
static OUString s_runtimePermission = OUSTR("com.sun.star.security.RuntimePermission");
static OUString s_allPermission = OUSTR("com.sun.star.security.AllPermission");
static OUString s_allFiles = OUSTR("<<ALL FILES>>");

//##################################################################################################

//--------------------------------------------------------------------------------------------------
static inline bool implies_actions(
    OUString const & granted_actions, OUString const & demanded_actions )
    SAL_THROW( () )
{
    // on average it makes no sense to build up a hash_set, because I think commonly there
    // only one demanded action, so tokenize every granted actions for every demanded action

    sal_Int32 n = 0;
    do
    {
        OUString demanded( demanded_actions.getToken( 0, ',', n ) );
        bool found = false;
        sal_Int32 n2 = 0;
        do
        {
            OUString granted( granted_actions.getToken( 0, ',', n2 ) );
            if (demanded.equals( granted ))
            {
                found = true;
                break;
            }
        }
        while (n2 >= 0); // all granted
        if (! found)
            return false;
    }
    while (n >= 0); // all demanded
    // all actions in
    return true;
}
//--------------------------------------------------------------------------------------------------
static inline bool endsWith( OUString const & s, char const * p, sal_Int32 n )
{
    sal_Int32 nPos = s.getLength();
    if (nPos < n)
        return false;
    sal_Unicode const * b = s.pData->buffer;
    while (n)
    {
        if (b[ --nPos ] != p[ --n ])
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
static OUString const & getWorkingDir() SAL_THROW( () )
{
    static OUString * s_workingDir = 0;
    if (! s_workingDir)
    {
        OUString workingDir;
        ::osl_getProcessWorkingDir( &workingDir.pData );

        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! s_workingDir)
        {
            static OUString s_dir( workingDir );
            s_workingDir = &s_dir;
        }
    }
    return *s_workingDir;
}
//--------------------------------------------------------------------------------------------------
static inline OUString makeAbsolutePath( OUString const & url ) SAL_THROW( () )
{
    if (url.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("*") ))
    {
        OUStringBuffer buf( 64 );
        buf.append( getWorkingDir() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/*") );
        return buf.makeStringAndClear();
    }
    if (url.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("-") ))
    {
        OUStringBuffer buf( 64 );
        buf.append( getWorkingDir() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/-") );
        return buf.makeStringAndClear();
    }
    if (0 == url.compareToAscii( RTL_CONSTASCII_STRINGPARAM("file:///") )) // file url
        return url;
    // else relative path
    OUString out;
    oslFileError rc = ::osl_getAbsoluteFileURL(
        getWorkingDir().pData, url.pData, &out.pData );
    return (osl_File_E_None == rc ? out : url);
}
// FilePermission
//--------------------------------------------------------------------------------------------------
static inline bool implies(
    Any const & granted, io::FilePermission const & demanded )
    SAL_THROW( () )
{
    // avoid string ref-counting
    OUString const & granted_typeName =
        *reinterpret_cast< OUString const * >( &granted.pType->pTypeName );
    if (granted_typeName.equals( s_allPermission ))
        return true;
    if (! granted_typeName.equals( s_filePermission ))
        return false;

    io::FilePermission const & granted_perm =
        *reinterpret_cast< io::FilePermission const * >( granted.pData );

    // check actions
    if (! implies_actions( granted_perm.Actions, demanded.Actions ))
        return false;

    // check url
    if (granted_perm.URL.equals( s_allFiles ))
        return true;
    if (demanded.URL.equals( s_allFiles ))
        return false;
    if (granted_perm.URL.equals( demanded.URL ))
        return true;
    // make absolute pathes
    OUString granted_url( makeAbsolutePath( granted_perm.URL ) );
    OUString demanded_url( makeAbsolutePath( demanded.URL ) );
    if (granted_url.equals( demanded_url ))
        return true;
    if (granted_url.getLength() > demanded_url.getLength())
        return false;
    // check /- wildcard: all files and recursive in that path
    if (endsWith( granted_url, RTL_CONSTASCII_STRINGPARAM("/-") ))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = granted_url.getLength() -1;
        return (0 == demanded_url.compareTo( granted_url, len ));
    }
    // check /* wildcard: all files in that path (not recursive!)
    if (endsWith( granted_url, RTL_CONSTASCII_STRINGPARAM("/*") ))
    {
        // demanded url must start with granted path (including path trailing path sep)
        sal_Int32 len = granted_url.getLength() -1;
        return ((0 == demanded_url.compareTo( granted_url, len )) &&
                (0 > demanded_url.indexOf( '/', len ))); // in addition, no deeper pathes
    }

    return false;
}
// SocketPermission
//--------------------------------------------------------------------------------------------------
static inline bool implies(
    Any const & granted, connection::SocketPermission const & demanded )
    SAL_THROW( () )
{
    // avoid string ref-counting
    OUString const & granted_typeName =
        *reinterpret_cast< OUString const * >( &granted.pType->pTypeName );
    if (granted_typeName.equals( s_allPermission ))
        return true;
    if (! granted_typeName.equals( s_socketPermission ))
        return false;

    connection::SocketPermission const & granted_perm =
        *reinterpret_cast< connection::SocketPermission const * >( granted.pData );
    // check actions
    if (! implies_actions( granted_perm.Actions, demanded.Actions ))
        return false;
    // check host
    // xxx todo
    if (granted_perm.Host == demanded.Host)
        return true;

    return false;
}
// RuntimePermission
//--------------------------------------------------------------------------------------------------
static inline bool implies(
    Any const & granted, security::RuntimePermission const & demanded )
    SAL_THROW( () )
{
    // avoid string ref-counting
    OUString const & granted_typeName =
        *reinterpret_cast< OUString const * >( &granted.pType->pTypeName );
    if (granted_typeName.equals( s_allPermission ))
        return true;
    if (! granted_typeName.equals( s_runtimePermission ))
        return false;
    security::RuntimePermission const & granted_perm =
        *reinterpret_cast< security::RuntimePermission const * >( granted.pData );
    return (sal_False != (granted_perm.Name == demanded.Name));
}
// AllPermission
//--------------------------------------------------------------------------------------------------
static inline bool implies(
    Any const & granted, security::AllPermission const & demanded )
    SAL_THROW( () )
{
    // avoid string ref-counting
    OUString const & granted_typeName =
        *reinterpret_cast< OUString const * >( &granted.pType->pTypeName );
    return (sal_False != granted_typeName.equals( s_allPermission ));
}

//--------------------------------------------------------------------------------------------------
template< typename permission_type >
static inline bool implies(
    Sequence< Any > const & granted_permissions, permission_type const & demanded )
    SAL_THROW( () )
{
    Any const * granted = granted_permissions.getConstArray();
    sal_Int32 nCount = granted_permissions.getLength();
    for ( sal_Int32 nPos = 0; nPos < nCount; ++nPos )
    {
        if (implies( granted[ nPos ], demanded ))
            return true;
    }
    return false;
}
//--------------------------------------------------------------------------------------------------
static void checkStaticPermissions(
    Sequence< Any > const & granted_permissions, Any const & demanded_permission )
    SAL_THROW( (RuntimeException) )
{
    Type const & demanded_type = demanded_permission.getValueType();

    // supported permission types
    if (demanded_type.equals( ::getCppuType( (io::FilePermission const *)0 ) ))
    {
        io::FilePermission const & demanded =
            *reinterpret_cast< io::FilePermission const * >( demanded_permission.pData );
        if (implies( granted_permissions, demanded ))
            return;
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("access denied: ") );
        buf.append( demanded_type.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (url=\"") );
        buf.append( demanded.URL );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\", actions=\"") );
        buf.append( demanded.Actions );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
        throw security::AccessControlException(
            buf.makeStringAndClear(), Reference< XInterface >(), demanded_permission );
    }
    else if (demanded_type.equals( ::getCppuType( (connection::SocketPermission const *)0 ) ))
    {
        connection::SocketPermission const & demanded =
            *reinterpret_cast< connection::SocketPermission const * >( demanded_permission.pData );
        if (implies( granted_permissions, demanded ))
            return;
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("access denied: ") );
        buf.append( demanded_type.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (host=\"") );
        buf.append( demanded.Host );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\", actions=\"") );
        buf.append( demanded.Actions );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
        throw security::AccessControlException(
            buf.makeStringAndClear(), Reference< XInterface >(), demanded_permission );
    }
    else if (demanded_type.equals( ::getCppuType( (security::RuntimePermission const *)0 ) ))
    {
        security::RuntimePermission const & demanded =
            *reinterpret_cast< security::RuntimePermission const * >( demanded_permission.pData );
        if (implies( granted_permissions, demanded ))
            return;
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("access denied: ") );
        buf.append( demanded_type.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (name=\"") );
        buf.append( demanded.Name );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
        throw security::AccessControlException(
            buf.makeStringAndClear(), Reference< XInterface >(), demanded_permission );
    }
    else if (demanded_type.equals( ::getCppuType( (security::AllPermission const *)0 ) ))
    {
        security::AllPermission const & demanded =
            *reinterpret_cast< security::AllPermission const * >( demanded_permission.pData );
        if (implies( granted_permissions, demanded ))
            return;
        throw security::AccessControlException(
            OUSTR("access denied: com.sun.star.security.AllPermission"),
            Reference< XInterface >(), demanded_permission );
    }
    else
    {
        OUStringBuffer buf( 48 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("checking for unsupported permission type: ") );
        buf.append( demanded_type.getTypeName() );
        throw RuntimeException(
            buf.makeStringAndClear(), Reference< XInterface >() );
    }
}

/** ac context granting all permissions
*/
class acc_allPermissions
    : public WeakImplHelper1< security::XAccessControlContext >
{
public:
    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void acc_allPermissions::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    // no restriction; allow all
}
/** ac context combiner combining two ac contexts
*/
class acc_Combiner
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Reference< security::XAccessControlContext > m_x1, m_x2;

    inline acc_Combiner(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW( () );

public:
    virtual ~acc_Combiner()
        SAL_THROW( () );

    static inline Reference< security::XAccessControlContext > create(
        Reference< security::XAccessControlContext > const & x1,
        Reference< security::XAccessControlContext > const & x2 )
        SAL_THROW( () );

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline acc_Combiner::acc_Combiner(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW( () )
    : m_x1( x1 )
    , m_x2( x2 )
{
     s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
acc_Combiner::~acc_Combiner()
    SAL_THROW( () )
{
     s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}
//--------------------------------------------------------------------------------------------------
inline Reference< security::XAccessControlContext > acc_Combiner::create(
    Reference< security::XAccessControlContext > const & x1,
    Reference< security::XAccessControlContext > const & x2 )
    SAL_THROW( () )
{
    if (! x1.is())
        return x2;
    if (! x2.is())
        return x1;
    return new acc_Combiner( x1, x2 );
}
//__________________________________________________________________________________________________
void acc_Combiner::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    m_x1->checkPermission( perm );
    m_x2->checkPermission( perm );
}

/** ac context doing permission checks on static user permissions
*/
class acc_UserPolicy
    : public WeakImplHelper1< security::XAccessControlContext >
{
    Sequence< Any > m_permissions;

public:
    inline acc_UserPolicy(
        Sequence< Any > permissions )
        SAL_THROW( () )
        : m_permissions( permissions )
        {}

    // XAccessControlContext impl
    virtual void SAL_CALL checkPermission(
        Any const & perm )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void acc_UserPolicy::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    checkStaticPermissions( m_permissions, perm );
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
        SAL_THROW( () );
    virtual ~acc_CurrentContext() SAL_THROW( () );

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
    SAL_THROW( () )
    : m_refcount( 0 )
    , m_xDelegate( xDelegate )
{
     s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );

    if (xRestriction.is())
    {
        m_restriction = makeAny( xRestriction );
    }
    // return empty any otherwise on getValueByName(), not null interface
}
//__________________________________________________________________________________________________
acc_CurrentContext::~acc_CurrentContext()
    SAL_THROW( () )
{
     s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
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
    if (name.equals( s_acRestriction ))
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
    Any acc( xContext->getValueByName( s_acRestriction ) );
    if (typelib_TypeClass_INTERFACE == acc.pType->eTypeClass)
    {
        // avoid ref-counting
        OUString const & typeName =
            *reinterpret_cast< OUString const * >( &acc.pType->pTypeName );
        if (typeName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.XAccessControlContext") ))
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
    return Reference< security::XAccessControlContext >();
}
//==================================================================================================
class cc_reset
{
    void * m_cc;
public:
    inline cc_reset( void * cc ) SAL_THROW( () )
        : m_cc( cc ) {}
    inline ~cc_reset() SAL_THROW( () )
        { ::uno_setCurrentContext( m_cc, s_envType.pData, 0 ); }
};
//==================================================================================================
class RecursionCheck : public ThreadData
{
    typedef hash_set< OUString, OUStringHash > t_set;
    static void SAL_CALL destroyKeyData( void * );

    inline t_set * getData() SAL_THROW( () )
        { return reinterpret_cast< t_set * >( ThreadData::getData() ); }

public:
    inline RecursionCheck() SAL_THROW( () )
        : ThreadData( (oslThreadKeyCallbackFunction)destroyKeyData )
        {}

    bool isRecurring( OUString const & userId ) SAL_THROW( () );
    void mark( OUString const & userId ) SAL_THROW( () );
    void unmark( OUString const & userId ) SAL_THROW( () );
};
//__________________________________________________________________________________________________
void RecursionCheck::destroyKeyData( void * p )
{
    delete reinterpret_cast< t_set * >( p );
}
//__________________________________________________________________________________________________
bool RecursionCheck::isRecurring( OUString const & userId ) SAL_THROW( () )
{
    t_set const * s = getData();
    if (s)
    {
        t_set::const_iterator const iFind( s->find( userId ) );
        return (iFind != s->end());
    }
    return false;
}
//__________________________________________________________________________________________________
inline void RecursionCheck::mark( OUString const & userId ) SAL_THROW( () )
{
    t_set * s = getData();
    if (s)
    {
        s->insert( userId );
    }
    else
    {
        t_set * s = new t_set();
        s->insert( userId );
        setData( s );
    }
}
//__________________________________________________________________________________________________
inline void RecursionCheck::unmark( OUString const & userId ) SAL_THROW( () )
{
    t_set * s = getData();
    if (s)
    {
        s->erase( userId );
    }
}
//==================================================================================================
class RecursionMarkGuard
{
    RecursionCheck & m_rec_check;
    OUString const & m_userId;

public:
    inline RecursionMarkGuard( RecursionCheck & rec_check, OUString const & userId ) SAL_THROW( () )
        : m_rec_check( rec_check )
        , m_userId( userId )
        { m_rec_check.mark( m_userId ); }
    inline ~RecursionMarkGuard() SAL_THROW( () )
        { m_rec_check.unmark( m_userId ); }
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

    RecursionCheck m_rec_check;

    Reference< security::XPolicy > m_xPolicy;
    Reference< security::XPolicy > const & getPolicy()
        SAL_THROW( (RuntimeException) );

    // mode
    enum { OFF, ON, DYNAMIC_ONLY, SINGLE_USER, SINGLE_DEFAULT_USER } m_mode;

    // for single-user mode
    Sequence< Any > m_singleUserPermissions;
    OUString m_singleUserId;
    bool m_singleUser_init;

    typedef hash_map< OUString, Sequence< Any >, OUStringHash > t_user2permissions;
    t_user2permissions m_user2permissions;

    Sequence< Any > getUserPermissions(
        Reference< XCurrentContext > const & xContext )
        SAL_THROW( (RuntimeException) );

protected:
    virtual void SAL_CALL disposing();

public:
    AccessController( Reference< XComponentContext > const & xComponentContext )
        SAL_THROW( () );
    virtual ~AccessController()
        SAL_THROW( () );

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
    SAL_THROW( () )
    : t_helper( m_mutex )
    , m_xComponentContext( xComponentContext )
    , m_mode( ON ) // default
    , m_singleUser_init( false )
{
     s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );

    OUString mode;
    if (m_xComponentContext->getValueByName( OUSTR("/services/" SERVICE_NAME "/mode") ) >>= mode)
    {
        if (mode.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("off") ))
        {
            m_mode = OFF;
        }
        else if (mode.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("on") ))
        {
            m_mode = ON;
        }
        else if (mode.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dynamic-only") ))
        {
            m_mode = DYNAMIC_ONLY;
        }
        else if (mode.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("single-user") ))
        {
            m_xComponentContext->getValueByName(
                OUSTR("/services/" SERVICE_NAME "/single-user-id") ) >>= m_singleUserId;
            if (! m_singleUserId.getLength())
            {
                throw RuntimeException(
                    OUSTR("expected a user id in component context entry "
                          "\"/services/" SERVICE_NAME "/single-user-id\"!"),
                    (OWeakObject *)this );
            }
            m_mode = SINGLE_USER;
        }
        else if (mode.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("single-default-user") ))
        {
            m_mode = SINGLE_DEFAULT_USER;
        }
    }
}
//__________________________________________________________________________________________________
AccessController::~AccessController()
    SAL_THROW( () )
{
     s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
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
    if (! userId.getLength())
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
//__________________________________________________________________________________________________
Sequence< Any > AccessController::getUserPermissions(
    Reference< XCurrentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
{
    // this is the only place calling the policy singleton taking care of recurring calls

    switch (m_mode)
    {
    case SINGLE_USER:
    case SINGLE_DEFAULT_USER:
    {
        if (! m_singleUser_init)
        {
            // call on policy
            // iff this is a recurring call for the same user, then grant all permissions
            if (m_rec_check.isRecurring( m_singleUserId ))
            {
                Any allPerm( makeAny( security::AllPermission() ) );
                return Sequence< Any >( &allPerm, 1 );
            }
            Sequence< Any > singleUserPermissions;
            {
            RecursionMarkGuard rec_guard( m_rec_check, m_singleUserId ); // mark possible recursion
            if (SINGLE_USER == m_mode)
                singleUserPermissions = getPolicy()->getPermissions( m_singleUserId );
            else
                singleUserPermissions = getPolicy()->getDefaultPermissions();
            }

            // assign
            MutexGuard guard( m_mutex );
            if (! m_singleUser_init)
            {
                m_singleUserPermissions = singleUserPermissions;
                m_singleUser_init = true;
            }
        }
        return m_singleUserPermissions;
    }
    case ON:
    {
        OUString userId;
        if (xContext.is())
        {
            xContext->getValueByName( OUSTR(USER_CREDS ".id") ) >>= userId;
        }
        if (! userId.getLength())
        {
            throw SecurityException(
                OUSTR("cannot determine current user in multi-user ac!"), (OWeakObject *)this );
        }

        // lookup policy for user
        ClearableMutexGuard guard( m_mutex );
        t_user2permissions::const_iterator iFind( m_user2permissions.find( userId ) );
        if (m_user2permissions.end() == iFind)
        {
            guard.clear();

            // call on policy
            // iff this is a recurring call for the same user, then grant all permissions
            if (m_rec_check.isRecurring( userId ))
            {
                Any allPerm( makeAny( security::AllPermission() ) );
                return Sequence< Any >( &allPerm, 1 );
            }
            Sequence< Any > permissions;
            {
            RecursionMarkGuard rec_guard( m_rec_check, userId ); // mark possible recursion
            permissions = getPolicy()->getPermissions( userId );
            }

            {
            // assign
            MutexGuard guard( m_mutex );
            t_user2permissions::const_iterator iFind( m_user2permissions.find( userId ) );
            if (m_user2permissions.end() != iFind) // inserted in the meantime
            {
                return iFind->second;
            }
            else // insert new
            {
                pair< t_user2permissions::iterator, bool > insertion( m_user2permissions.insert(
                    t_user2permissions::value_type( userId, permissions ) ) );
                OSL_ASSERT( insertion.second );
                return permissions;
            }
            }
        }
        else
        {
            return iFind->second;
        }
    }
    default:
        OSL_ENSURE( 0, "### this should never be called in this mode!" );
        return Sequence< Any >();
    }
}

// XAccessController impl
//__________________________________________________________________________________________________
void AccessController::checkPermission(
    Any const & perm )
    throw (RuntimeException)
{
    if (OFF == m_mode)
        return;

    // first dynamic check of ac contexts
    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );
    if (xContext.is())
    {
        Reference< security::XAccessControlContext > xACC( getDynamicRestriction( xContext ) );
        if (xACC.is())
        {
            xACC->checkPermission( perm );
        }
    }

    if (DYNAMIC_ONLY == m_mode)
        return;

    // then static check
    checkStaticPermissions( getUserPermissions( xContext ), perm );
}
//__________________________________________________________________________________________________
Any AccessController::doRestricted(
    Reference< security::XAction > const & xAction,
    Reference< security::XAccessControlContext > const & xRestriction )
    throw (Exception)
{
    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
        return xAction->run();

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

    if (xRestriction.is())
    {
        // override restriction
        Reference< security::XAccessControlContext > xOldRestr;
        if (xContext.is())
        {
            xOldRestr = getDynamicRestriction( xContext );
        }

        Reference< XCurrentContext > xNewContext( new acc_CurrentContext(
            xContext, acc_Combiner::create( xRestriction, xOldRestr ) ) );
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
    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
        return xAction->run();

    // override restriction
    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

    Reference< XCurrentContext > xNewContext( new acc_CurrentContext(
        xContext, xRestriction ) );
    ::uno_setCurrentContext( xNewContext.get(), s_envType.pData, 0 );

    cc_reset reset( xContext.get() );
    return xAction->run();
}
//__________________________________________________________________________________________________
Reference< security::XAccessControlContext > AccessController::getContext()
    throw (RuntimeException)
{
    if (OFF == m_mode) // optimize this way, because no dynamic check will be performed
        return new acc_allPermissions(); // dummy granting all permissions

    Reference< XCurrentContext > xContext;
    ::uno_getCurrentContext( (void **)&xContext, s_envType.pData, 0 );

    Reference< security::XAccessControlContext > xDynamic;
    if (xContext.is())
    {
        xDynamic = getDynamicRestriction( xContext );
    }

    return acc_Combiner::create( xDynamic, new acc_UserPolicy( getUserPermissions( xContext ) ) );
}

// XServiceInfo impl
//__________________________________________________________________________________________________
OUString AccessController::getImplementationName()
    throw (RuntimeException)
{
    return s_implName;
}
//__________________________________________________________________________________________________
sal_Bool AccessController::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    OUString const * pNames = s_serviceNames.getConstArray();
    for ( sal_Int32 nPos = s_serviceNames.getLength(); nPos--; )
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
    return s_serviceNames;
}

//##################################################################################################

//--------------------------------------------------------------------------------------------------
static Reference< XInterface > SAL_CALL ac_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return (OWeakObject *)new AccessController( xComponentContext );
}
//--------------------------------------------------------------------------------------------------
static Sequence< OUString > ac_getSupportedServiceNames() SAL_THROW( () )
{
    return s_serviceNames;
}
//--------------------------------------------------------------------------------------------------
static OUString ac_getImplementationName() SAL_THROW( () )
{
    return s_implName;
}
//--------------------------------------------------------------------------------------------------
Reference< XInterface > SAL_CALL filepolicy_create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) );
//--------------------------------------------------------------------------------------------------
Sequence< OUString > filepolicy_getSupportedServiceNames() SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
OUString filepolicy_getImplementationName() SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
static struct ImplementationEntry s_entries [] =
{
    {
        ac_create, ac_getImplementationName,
        ac_getSupportedServiceNames, createSingleComponentFactory,
        &s_moduleCount.modCnt, 0
    },
    {
        filepolicy_create, filepolicy_getImplementationName,
        filepolicy_getSupportedServiceNames, createSingleComponentFactory,
        &s_moduleCount.modCnt, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

}

using namespace stoc_security;

extern "C"
{
//==================================================================================================
sal_Bool SAL_CALL component_canUnload( TimeValue * pTime )
{
    return s_moduleCount.canUnload( &s_moduleCount, pTime );
}
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    sal_Char const ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, s_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    sal_Char const * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, s_entries );
}
}
