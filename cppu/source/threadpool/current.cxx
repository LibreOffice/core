/*************************************************************************
 *
 *  $RCSfile: current.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:02:09 $
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

#include "rtl/uuid.h"
#include "osl/thread.h"
#include "osl/mutex.hxx"

#include "uno/environment.hxx"
#include "uno/mapping.h"
#include "uno/lbnames.h"
#include "typelib/typedescription.h"

#include "current.hxx"


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;

namespace cppu
{

//--------------------------------------------------------------------------------------------------
class SAL_NO_VTABLE XInterface
{
public:
    virtual void SAL_CALL slot_queryInterface() = 0;
    virtual void SAL_CALL acquire() throw () = 0;
    virtual void SAL_CALL release() throw () = 0;
};
//--------------------------------------------------------------------------------------------------
static typelib_InterfaceTypeDescription * get_type_XCurrentContext()
{
    static typelib_InterfaceTypeDescription * s_type_XCurrentContext = 0;
    if (0 == s_type_XCurrentContext)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (0 == s_type_XCurrentContext)
        {
            OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XCurrentContext") );
            typelib_InterfaceTypeDescription * pTD = 0;
            typelib_TypeDescriptionReference * pMembers[1] = { 0 };
            OUString sMethodName0(
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XCurrentContext::getValueByName") );
            typelib_typedescriptionreference_new(
                &pMembers[0],
                typelib_TypeClass_INTERFACE_METHOD,
                sMethodName0.pData );
            typelib_typedescription_newInterface(
                &pTD,
                sTypeName.pData, 0x00000000, 0x0000, 0x0000, 0x00000000, 0x00000000,
                * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ),
                1,
                pMembers );

            typelib_typedescription_register( (typelib_TypeDescription**)&pTD );
            typelib_typedescriptionreference_release( pMembers[0] );

            typelib_InterfaceMethodTypeDescription * pMethod = 0;
            typelib_Parameter_Init aParameters[1];
            OUString sParamName0( RTL_CONSTASCII_USTRINGPARAM("Name") );
            OUString sParamType0( RTL_CONSTASCII_USTRINGPARAM("string") );
            aParameters[0].pParamName = sParamName0.pData;
            aParameters[0].eTypeClass = typelib_TypeClass_STRING;
            aParameters[0].pTypeName = sParamType0.pData;
            aParameters[0].bIn = sal_True;
            aParameters[0].bOut = sal_False;
            rtl_uString * pExceptions[1];
            OUString sExceptionName0(
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.RuntimeException") );
            pExceptions[0] = sExceptionName0.pData;
            OUString sReturnType0( RTL_CONSTASCII_USTRINGPARAM("any") );
            typelib_typedescription_newInterfaceMethod(
                &pMethod,
                3, sal_False,
                sMethodName0.pData,
                typelib_TypeClass_ANY, sReturnType0.pData,
                1, aParameters, 1, pExceptions );
            typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );
            typelib_typedescription_release( (typelib_TypeDescription*)pMethod );
#if ! defined CPPU_LEAK_STATIC_DATA
            // another static ref
            ++reinterpret_cast< typelib_TypeDescription * >( pTD )->
                nStaticRefCount;
#endif
            s_type_XCurrentContext = pTD;
        }
    }
    return s_type_XCurrentContext;
}

//##################################################################################################

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
                reinterpret_cast< ::cppu::XInterface * >( pId->pCurrentContext )->release();
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
            reinterpret_cast< ::cppu::XInterface * >( pId->pCurrentContext )->release();
        }
        pId->pCurrentContext = 0;
    }

    if (pCurrentContext)
    {
        OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );
        if (rEnvTypeName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ))
        {
            reinterpret_cast< ::cppu::XInterface * >( pCurrentContext )->acquire();
            pId->pCurrentContext = pCurrentContext;
            pId->pCurrentContextEnv = 0; // special for this compiler env
        }
        else
        {
            uno_Environment * pEnv = 0;
            ::uno_getEnvironment( &pEnv, pEnvTypeName, pEnvContext );
            OSL_ASSERT( pEnv && pEnv->pExtEnv );
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
    void ** ppCurrentContext, rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    ::com::sun::star::uno::Environment target_env;
    OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );

    // release inout parameter
    if (*ppCurrentContext)
    {
        if (rEnvTypeName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ))
        {
            reinterpret_cast< ::cppu::XInterface * >( *ppCurrentContext )->release();
        }
        else
        {
            uno_getEnvironment( (uno_Environment **) &target_env, pEnvTypeName, pEnvContext );
            OSL_ASSERT( target_env.is() );
            if (! target_env.is())
                return sal_False;
            uno_ExtEnvironment * pEnv = target_env.get()->pExtEnv;
            OSL_ASSERT( 0 != pEnv );
            if (0 == pEnv)
                return sal_False;
            (*pEnv->releaseInterface)( pEnv, *ppCurrentContext );
        }
        *ppCurrentContext = 0;
    }

    // case: null-ref
    if (0 == pId->pCurrentContext)
        return sal_True;

    // case: same env (current compiler env)
    if ((0 == pId->pCurrentContextEnv) &&
        rEnvTypeName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) &&
        (0 == pEnvContext))
    {
        reinterpret_cast< ::cppu::XInterface * >( pId->pCurrentContext )->acquire();
        *ppCurrentContext = pId->pCurrentContext;
        return sal_True;
    }
    // case: same env (!= current compiler env)
    if ((0 != pId->pCurrentContextEnv) &&
        (0 == ::rtl_ustr_compare(
            ((uno_Environment *) pId->pCurrentContextEnv)->pTypeName->buffer,
            pEnvTypeName->buffer )) &&
        ((uno_Environment *) pId->pCurrentContextEnv)->pContext == pEnvContext)
    {
        // target env == current env
        (*pId->pCurrentContextEnv->acquireInterface)(
            pId->pCurrentContextEnv, pId->pCurrentContext );
        *ppCurrentContext = pId->pCurrentContext;
        return sal_True;
    }
    // else: mapping needed

    if (! target_env.is())
    {
        uno_getEnvironment( (uno_Environment **) &target_env, pEnvTypeName, pEnvContext );
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return sal_False;
    }

    ::com::sun::star::uno::Environment source_env;
    uno_Environment * p_source_env = (uno_Environment *) pId->pCurrentContextEnv;
    if (0 == p_source_env)
    {
        OUString current_env_name(
            RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
        uno_getEnvironment( (uno_Environment **) &source_env, current_env_name.pData, 0 );
        OSL_ASSERT( source_env.is() );
        if (! source_env.is())
            return sal_False;
        p_source_env = source_env.get();
    }

    uno_Mapping * mapping = 0;
    uno_getMapping( &mapping, p_source_env, target_env.get(), 0 );
    OSL_ASSERT( mapping != 0 );
    if (! mapping)
        return sal_False;
    (*mapping->mapInterface)(
        mapping,
        ppCurrentContext, pId->pCurrentContext, ::cppu::get_type_XCurrentContext() );
    (*mapping->release)( mapping );
    return sal_True;
}
