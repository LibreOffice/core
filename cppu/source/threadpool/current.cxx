/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: current.cxx,v $
 * $Revision: 1.16 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "rtl/uuid.h"
#include "osl/thread.h"
#include "osl/mutex.hxx"

#include "uno/environment.hxx"
#include "uno/mapping.hxx"
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
            (*pId->pCurrentContextEnv->releaseInterface)(
                pId->pCurrentContextEnv, pId->pCurrentContext );
            (*((uno_Environment *)pId->pCurrentContextEnv)->release)(
                (uno_Environment *)pId->pCurrentContextEnv );
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
        (*pId->pCurrentContextEnv->releaseInterface)(
            pId->pCurrentContextEnv, pId->pCurrentContext );
        (*((uno_Environment *)pId->pCurrentContextEnv)->release)(
            (uno_Environment *)pId->pCurrentContextEnv );
        pId->pCurrentContextEnv = 0;

        pId->pCurrentContext = 0;
    }

    if (pCurrentContext)
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
    return sal_True;
}
//##################################################################################################
extern "C" sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext, rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    Environment target_env;

    // release inout parameter
    if (*ppCurrentContext)
    {
        target_env = Environment(rtl::OUString(pEnvTypeName), pEnvContext);
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return sal_False;
        uno_ExtEnvironment * pEnv = target_env.get()->pExtEnv;
        OSL_ASSERT( 0 != pEnv );
        if (0 == pEnv)
            return sal_False;
        (*pEnv->releaseInterface)( pEnv, *ppCurrentContext );

        *ppCurrentContext = 0;
    }

    // case: null-ref
    if (0 == pId->pCurrentContext)
        return sal_True;

    if (! target_env.is())
    {
        target_env = Environment(rtl::OUString(pEnvTypeName), pEnvContext);
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return sal_False;
    }

    Mapping mapping((uno_Environment *) pId->pCurrentContextEnv, target_env.get());
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
        return sal_False;

    mapping.mapInterface(ppCurrentContext, pId->pCurrentContext, ::cppu::get_type_XCurrentContext() );

    return sal_True;
}
