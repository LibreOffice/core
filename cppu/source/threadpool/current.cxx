/*************************************************************************
 *
 *  $RCSfile: current.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2001-08-22 09:33:00 $
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

#include <uno/current_context.hxx>
#include <uno/environment.h>
#include <uno/mapping.hxx>

#include "current.hxx"


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;

using namespace ::com::sun::star::uno;


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

//==================================================================================================
extern "C" void SAL_CALL delete_IdContainer( void * p )
{
    if (p)
    {
        IdContainer * pId = reinterpret_cast< IdContainer * >( p );
        if (pId->pCurrentContext)
        {
            if (pId->pCurrentContextEnv)
            {
                (*pId->pCurrentContextEnv->releaseInterface)(
                    pId->pCurrentContextEnv, pId->pCurrentContext );
                (*((uno_Environment *)pId->pCurrentContextEnv)->release)(
                    (uno_Environment *)pId->pCurrentContextEnv );
            }
            else // current compiler used for context interface implementation
            {
                reinterpret_cast< XInterface * >( pId->pCurrentContext )->release();
            }
        }
        if (pId->bInit)
        {
            ::rtl_byte_sequence_release( pId->pLocalThreadId );
            ::rtl_byte_sequence_release( pId->pCurrentId );
        }
        delete pId;
    }
}
//==================================================================================================
IdContainer * getIdContainer() SAL_THROW( () )
{
    static ThreadKey s_key( delete_IdContainer );
    oslThreadKey aKey = s_key.getThreadKey();

    IdContainer * pId = reinterpret_cast< IdContainer * >( ::osl_getThreadKeyData( aKey ) );
    if (! pId)
    {
        pId = new IdContainer();
        pId->pCurrentContext = 0;
        pId->pCurrentContextEnv = 0;
        pId->bInit = sal_False;
        ::osl_setThreadKeyData( aKey, pId );
    }
    return pId;
}

}

//##################################################################################################
extern "C" sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    // free old one
    if (pId->pCurrentContext)
    {
        if (pId->pCurrentContextEnv)
        {
            (*pId->pCurrentContextEnv->releaseInterface)(
                pId->pCurrentContextEnv, pId->pCurrentContext );
            (*((uno_Environment *)pId->pCurrentContextEnv)->release)(
                (uno_Environment *)pId->pCurrentContextEnv );
            pId->pCurrentContextEnv = 0;
        }
        else // current compiler used for context interface implementation
        {
            reinterpret_cast< XInterface * >( pId->pCurrentContext )->release();
        }
        pId->pCurrentContext = 0;
    }

    if (pCurrentContext)
    {
        OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );
        if (rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ))
        {
            reinterpret_cast< XInterface * >( pCurrentContext )->acquire();
            pId->pCurrentContext = pCurrentContext;
            pId->pCurrentContextEnv = 0;
        }
        else
        {
            uno_Environment * pEnv = 0;
            ::uno_getEnvironment( &pEnv, pEnvTypeName, pEnvContext );
            OSL_ENSURE( pEnv && pEnv->pExtEnv, "### cannot get env of current context!" );
            if (pEnv)
            {
                if (pEnv->pExtEnv)
                {
                    pId->pCurrentContextEnv = pEnv->pExtEnv;
                    (*pId->pCurrentContextEnv->acquireInterface)(
                        pId->pCurrentContextEnv, pCurrentContext );
                    pId->pCurrentContext = pCurrentContext;
                }
                else
                {
                    (*pEnv->release)( pEnv );
                    return sal_False;
                }
            }
            else
            {
                return sal_False;
            }
        }
    }

    return sal_True;
}
//##################################################################################################
extern "C" sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    uno_Environment * pTargetEnv = 0;
    OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );

    // release inout parameter
    if (*ppCurrentContext)
    {
        if (rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ))
        {
            reinterpret_cast< XInterface * >( *ppCurrentContext )->release();
        }
        else
        {
            ::uno_getEnvironment( &pTargetEnv, pEnvTypeName, pEnvContext );
            OSL_ENSURE( pTargetEnv, "### cannot get env of target current context!" );
            if (pTargetEnv)
            {
                uno_ExtEnvironment * pEnv = pTargetEnv->pExtEnv;
                OSL_ENSURE( pEnv, "### cannot release given interface, because of incomplete env!" );
                if (pEnv)
                {
                    (*pEnv->releaseInterface)( pEnv, *ppCurrentContext );
                }
                else
                {
                    (*pTargetEnv->release)( pTargetEnv );
                    return sal_False;
                }
            }
            else
            {
                return sal_False;
            }
        }
        *ppCurrentContext = 0;
    }

    sal_Bool bRet = sal_False;

    if (pId->pCurrentContext)
    {
        if (pId->pCurrentContextEnv)
        {
            if (::rtl_ustr_compare(
                ((uno_Environment *)pId->pCurrentContextEnv)->pTypeName->buffer, pEnvTypeName->buffer ) &&
                ((uno_Environment *)pId->pCurrentContextEnv)->pContext == pEnvContext) // same env
            {
                (*pId->pCurrentContextEnv->acquireInterface)(
                    pId->pCurrentContextEnv, pId->pCurrentContext );
                *ppCurrentContext = pId->pCurrentContext;
                bRet = sal_True;
            }
            else // map from set context to target
            {
                if (! pTargetEnv)
                {
                    ::uno_getEnvironment( &pTargetEnv, pEnvTypeName, pEnvContext );
                }
                if (pTargetEnv)
                {
                    Mapping aMapping( (uno_Environment *)pId->pCurrentContextEnv, pTargetEnv );
                    if (aMapping.is())
                    {
                        aMapping.mapInterface(
                            ppCurrentContext, pId->pCurrentContext,
                            ::getCppuType( (Reference< XCurrentContext > const *)0 ) );
                        bRet = sal_True;
                    }
                }
            }
        }
        else if (pTargetEnv ||
                 !rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) ||
                 pEnvContext)
            // mapping needed from current to target
        {
            if (! pTargetEnv)
            {
                ::uno_getEnvironment( &pTargetEnv, pEnvTypeName, pEnvContext );
            }
            if (pTargetEnv)
            {
                OUString aCurrentEnv( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
                uno_Environment * pCurrentEnv = 0;
                ::uno_getEnvironment( &pCurrentEnv, aCurrentEnv.pData, 0 );
                OSL_ASSERT( pCurrentEnv );
                if (pCurrentEnv)
                {
                    Mapping aMapping( pCurrentEnv, pTargetEnv );
                    (*pCurrentEnv->release)( pCurrentEnv );
                    if (aMapping.is())
                    {
                        aMapping.mapInterface(
                            ppCurrentContext, pId->pCurrentContext,
                            ::getCppuType( (Reference< XCurrentContext > const *)0 ) );
                        bRet = sal_True;
                    }
                }
            }
        }
        else // set env is current env is target env => no mapping needed
        {
            reinterpret_cast< XInterface * >( pId->pCurrentContext )->acquire();
            *ppCurrentContext = pId->pCurrentContext;
            bRet = sal_True;
        }

        if (pTargetEnv)
        {
            (*pTargetEnv->release)( pTargetEnv );
        }
    }
    else // get null-ref is ok
    {
        bRet = sal_True;
    }

    return bRet;
}
