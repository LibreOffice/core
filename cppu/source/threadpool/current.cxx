/*************************************************************************
 *
 *  $RCSfile: current.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-28 10:46:08 $
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

#include <hash_map>

#include <rtl/uuid.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>

//  #include <uno/current_context.hxx>
#include <uno/environment.h>
#include <uno/mapping.hxx>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include "current.hxx"


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
//  using namespace ::com::sun::star::security;
//  using namespace ::com::sun::star::security::auth::login;


namespace cppu
{

//==================================================================================================
class ThreadKey
{
    sal_Bool     _bInit;
    oslThreadKey _hThreadKey;
    oslThreadKeyCallbackFunction _pCallback;

public:
    inline oslThreadKey getThreadKey() SAL_THROW( () );

    inline ThreadKey( oslThreadKeyCallbackFunction pCallback ) SAL_THROW( () );
    inline ~ThreadKey() SAL_THROW( () );
};
//__________________________________________________________________________________________________
inline ThreadKey::ThreadKey( oslThreadKeyCallbackFunction pCallback ) SAL_THROW( () )
    : _bInit( sal_False )
    , _pCallback( pCallback )
{
}
//__________________________________________________________________________________________________
inline ThreadKey::~ThreadKey() SAL_THROW( () )
{
    if (_bInit)
    {
        ::osl_destroyThreadKey( _hThreadKey );
    }
}
//__________________________________________________________________________________________________
inline oslThreadKey ThreadKey::getThreadKey() SAL_THROW( () )
{
    if (! _bInit)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! _bInit)
        {
            _hThreadKey = ::osl_createThreadKey( _pCallback );
            _bInit = sal_True;
        }
    }
    return _hThreadKey;
}
/*
//==================================================================================================
struct CurrentContext
    : public XTypeProvider
    , public XCurrentContext
    , XNameContainer
{
    oslInterlockedCount _nRef;

    uno_ExtEnvironment *    _pCachedEnv;
    void *                  _pCachedInterface;
    inline void setCachedInterface( void * pInterface, uno_ExtEnvironment * pEnv ) SAL_THROW( () );

    Reference< XMultiServiceFactory >   _xMgr;
    Reference< XAccessController >      _xAccessController;
    Reference< XLoginContext >          _xLoginContext;

    typedef ::std::hash_map< OUString, Any, OUStringHash, ::std::equal_to< OUString > >
        t_String2Any;

    t_String2Any _env;

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType )
        throw (RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes()
        throw (RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw (RuntimeException);
    // XCurrentContext
    virtual Reference< XMultiServiceFactory > SAL_CALL getServiceManager()
        throw (RuntimeException);
    virtual Reference< XAccessController > SAL_CALL getAccessController()
        throw (RuntimeException);
    virtual Reference< XLoginContext > SAL_CALL getLoginContext()
        throw (RuntimeException);
    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString & rName, const Any & rElement )
        throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const OUString & rName )
        throw (NoSuchElementException, WrappedTargetException, RuntimeException);
    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString & rName, const Any & rElement )
        throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);
    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString & rName )
        throw (NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString & rName )
        throw (RuntimeException);
    // XElementAccess
    virtual Type SAL_CALL getElementType()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (RuntimeException);

    inline CurrentContext() SAL_THROW( () );
    inline ~CurrentContext() SAL_THROW( () );
};
//__________________________________________________________________________________________________
inline CurrentContext::~CurrentContext() SAL_THROW( () )
{
#ifdef CPPU_ASSERTIONS
    OSL_TRACE( "\n> destructing current context..." );
#endif
    setCachedInterface( 0, 0 );
}
//__________________________________________________________________________________________________
inline CurrentContext::CurrentContext() SAL_THROW( () )
    : _nRef( 0 )
    , _pCachedEnv( 0 )
    , _pCachedInterface( 0 )
{
}
//__________________________________________________________________________________________________
inline void CurrentContext::setCachedInterface(
    void * pInterface, uno_ExtEnvironment * pEnv ) SAL_THROW( () )
{
    // acquire new one
    if (pInterface)
    {
        (*pEnv->acquireInterface)( pEnv, pInterface );
    }
    if (pEnv)
    {
        (((uno_Environment *)pEnv)->acquire)( (uno_Environment *)pEnv );
    }

    if (_pCachedInterface)
    {
        (*_pCachedEnv->releaseInterface)( _pCachedEnv, _pCachedInterface );
        (((uno_Environment *)_pCachedEnv)->release)( (uno_Environment *)_pCachedEnv );
    }

    _pCachedEnv = pEnv;
    _pCachedInterface = pInterface;
}

// XInterface
//__________________________________________________________________________________________________
Any CurrentContext::queryInterface( const Type & rType )
    throw (RuntimeException)
{
    if (rType == ::getCppuType( (const Reference< XCurrentContext > *)0 ))
    {
        return makeAny( Reference< XCurrentContext >( static_cast< XCurrentContext * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XInterface > *)0 ))
    {
        return makeAny( Reference< XInterface >( static_cast< XCurrentContext * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XNameContainer > *)0 ))
    {
        return makeAny( Reference< XNameContainer >( static_cast< XNameContainer * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XNameReplace > *)0 ))
    {
        return makeAny( Reference< XNameReplace >( static_cast< XNameReplace * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XNameAccess > *)0 ))
    {
        return makeAny( Reference< XNameAccess >( static_cast< XNameAccess * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XElementAccess > *)0 ))
    {
        return makeAny( Reference< XElementAccess >( static_cast< XElementAccess * >( this ) ) );
    }
    else if (rType == ::getCppuType( (const Reference< XTypeProvider > *)0 ))
    {
        return makeAny( Reference< XTypeProvider >( static_cast< XTypeProvider * >( this ) ) );
    }
    else
    {
        return Any();
    }
}
//__________________________________________________________________________________________________
void CurrentContext::acquire()
    throw ()
{
    ::osl_incrementInterlockedCount( & _nRef );
}
//__________________________________________________________________________________________________
void CurrentContext::release()
    throw ()
{
    if (! ::osl_decrementInterlockedCount( & _nRef ))
    {
        delete this;
    }
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > CurrentContext::getTypes()
    throw (RuntimeException)
{
    Type ar[2] = { ::getCppuType( (const Reference< XCurrentContext > *)0 ),
                   ::getCppuType( (const Reference< XNameContainer > *)0 ) };
    return Sequence< Type >( ar, 2 );
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > CurrentContext::getImplementationId()
    throw (RuntimeException)
{
    static Sequence< sal_Int8 > * s_pSeq = 0;
    if (! s_pSeq)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pSeq)
        {
            static Sequence< sal_Int8 > s_aSeq( 16 );
            ::rtl_createUuid( (sal_uInt8 *)s_aSeq.getArray(), 0, sal_True );
            s_pSeq = & s_aSeq;
        }
    }
    return * s_pSeq;
}

// XCurrentContext
//__________________________________________________________________________________________________
Reference< XMultiServiceFactory > CurrentContext::getServiceManager()
    throw (RuntimeException)
{
    return _xMgr;
}
//__________________________________________________________________________________________________
Reference< XLoginContext > CurrentContext::getLoginContext()
    throw (RuntimeException)
{
    return _xLoginContext;
}
//__________________________________________________________________________________________________
Reference< XAccessController > CurrentContext::getAccessController()
    throw (RuntimeException)
{
    return _xAccessController;
}

// XNameContainer
//__________________________________________________________________________________________________
void CurrentContext::insertByName( const OUString & rName, const Any & rElement )
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    const t_String2Any::const_iterator iFind( _env.find( rName ) );
    if (iFind != _env.end())
    {
        throw ElementExistException(
            rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": element exists!") ),
            static_cast< XCurrentContext * >( this ) );
    }
    _env[ rName ] = rElement;

    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.lang.ServiceManager") ))
    {
        if (!(rElement >>= _xMgr) || !_xMgr.is())
        {
            _env.erase( rName );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XMultiServiceFactory given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.AccessController") ))
    {
        if (!(rElement >>= _xAccessController) || !_xAccessController.is())
        {
            _env.erase( rName );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XAccessController given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.auth.login.LoginContext") ))
    {
        if (!(rElement >>= _xLoginContext) || !_xLoginContext.is())
        {
            _env.erase( rName );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XLoginContext given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
}
//__________________________________________________________________________________________________
void CurrentContext::removeByName( const OUString & rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    const t_String2Any::iterator iFind( _env.find( rName ) );
    if (iFind == _env.end())
    {
        throw NoSuchElementException(
            rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no such element!") ),
            static_cast< XCurrentContext * >( this ) );
    }
    _env.erase( iFind );

    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.lang.ServiceManager") ))
    {
        _xMgr.clear();
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.AccessController") ))
    {
        _xAccessController.clear();
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.auth.login.LoginContext") ))
    {
        _xLoginContext.clear();
    }
}
// XNameReplace
//__________________________________________________________________________________________________
void CurrentContext::replaceByName( const OUString & rName, const Any & rElement )
    throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    const t_String2Any::iterator iFind( _env.find( rName ) );
    if (iFind == _env.end())
    {
        throw NoSuchElementException(
            rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no such element!") ),
            static_cast< XCurrentContext * >( this ) );
    }
    iFind->second = rElement;

    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.lang.ServiceManager") ))
    {
        if (!(rElement >>= _xMgr) || !_xMgr.is())
        {
            _env.erase( iFind );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XMultiServiceFactory given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.AccessController") ))
    {
        if (!(rElement >>= _xAccessController) || !_xAccessController.is())
        {
            _env.erase( iFind );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XAccessController given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.security.auth.login.LoginContext") ))
    {
        if (!(rElement >>= _xLoginContext) || !_xLoginContext.is())
        {
            _env.erase( iFind );
            throw IllegalArgumentException(
                rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no XLoginContext given!") ),
                static_cast< XCurrentContext * >( this ), 1 );
        }
    }
}
// XNameAccess
//__________________________________________________________________________________________________
Any CurrentContext::getByName( const OUString & rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    const t_String2Any::const_iterator iFind( _env.find( rName ) );
    if (iFind != _env.end())
    {
        return iFind->second;
    }

    throw NoSuchElementException(
        rName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no such element!") ),
        static_cast< XCurrentContext * >( this ) );
}
//__________________________________________________________________________________________________
Sequence< OUString > CurrentContext::getElementNames()
    throw (RuntimeException)
{
    Sequence< OUString > keys( _env.size() );
    OUString * pKeys = keys.getArray();
    sal_uInt32 nPos = 0;
    for ( t_String2Any::const_iterator iPos( _env.begin() ); iPos != _env.end(); ++iPos )
    {
        pKeys[ nPos++ ] = iPos->first;
    }
    OSL_ASSERT( nPos == _env.size() );
    return keys;
}
//__________________________________________________________________________________________________
sal_Bool CurrentContext::hasByName( const OUString & rName )
    throw (RuntimeException)
{
    const t_String2Any::const_iterator iFind( _env.find( rName ) );
    return (iFind != _env.end());
}
// XElementAccess
//__________________________________________________________________________________________________
Type CurrentContext::getElementType()
    throw (RuntimeException)
{
    return ::getCppuVoidType();
}
//__________________________________________________________________________________________________
sal_Bool CurrentContext::hasElements()
    throw (RuntimeException)
{
    return (! _env.empty());
}
*/
//==================================================================================================
extern "C" void SAL_CALL delete_IdContainer( void * p )
{
    if (p)
    {
        IdContainer * pId = reinterpret_cast< IdContainer * >( p );
//          if (pId->pCurrent)
//          {
//              pId->pCurrent->release();
//          }
        if (pId->bInit)
        {
            rtl_byte_sequence_release( pId->pLocalThreadId );
            rtl_byte_sequence_release( pId->pCurrentId );
        }
        delete pId;
    }
}
//==================================================================================================
IdContainer * getIdContainer() SAL_THROW( () )
{
    static ThreadKey s_key( delete_IdContainer );
    oslThreadKey aKey = s_key.getThreadKey();

    IdContainer * pContainer = reinterpret_cast< IdContainer * >( ::osl_getThreadKeyData( aKey ) );
    if (! pContainer)
    {
        pContainer = new IdContainer();
        pContainer->pCurrent = 0;
        pContainer->bInit = sal_False;
        ::osl_setThreadKeyData( aKey, pContainer );
    }
    return pContainer;
}

}

//##################################################################################################
extern "C" void SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    *ppCurrentContext = 0;
    /*
    IdContainer * pContainer = getIdContainer();
    CurrentContext * that = pContainer->pCurrent;
    if (! that)
    {
        that = new CurrentContext();
        that->acquire();
        pContainer->pCurrent = that;
    }

    const OUString & rEnvTypeName = * reinterpret_cast< const OUString * >( & pEnvTypeName );

    // current env matches?
    if (rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) &&
        !pEnvContext)
    {
        that->acquire();
        *ppCurrentContext = static_cast< XCurrentContext * >( that );
    }
    // cached interface matches?
    else if (that->_pCachedInterface && that->_pCachedEnv &&
             rEnvTypeName.equals( ((uno_Environment *)that->_pCachedEnv)->pTypeName ) &&
             pEnvContext == ((uno_Environment *)that->_pCachedEnv)->pContext)
    {
        (*that->_pCachedEnv->acquireInterface)( that->_pCachedEnv, that->_pCachedInterface );
        *ppCurrentContext = that->_pCachedInterface;
    }
    // map and set as cached
    else
    {
        *ppCurrentContext = 0;

        uno_Environment * pTargetEnv = 0;
        ::uno_getEnvironment( &pTargetEnv, pEnvTypeName, pEnvContext );
        OSL_ASSERT( pTargetEnv );
        if (pTargetEnv)
        {
            uno_Environment * pThisEnv = 0;
            OUString aCurrentName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
            ::uno_getEnvironment( &pThisEnv, aCurrentName.pData, 0 );
            OSL_ASSERT( pThisEnv );
            if (pThisEnv)
            {
                Mapping aMapping( pThisEnv, pTargetEnv );
                if (aMapping.is())
                {
                    aMapping.mapInterface(
                        ppCurrentContext, static_cast< XCurrentContext * >( that ),
                        ::getCppuType( (const Reference< XCurrentContext > *)0 ) );
                    // set as cached
                    if (*ppCurrentContext && pTargetEnv->pExtEnv)
                    {
                        that->setCachedInterface(
                            *ppCurrentContext, pTargetEnv->pExtEnv );
                    }
                }
                (*pThisEnv->release)( pThisEnv );
            }
            (*pTargetEnv->release)( pTargetEnv );
        }
    }
    */
}
