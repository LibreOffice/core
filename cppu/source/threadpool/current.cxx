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

static typelib_InterfaceTypeDescription * get_type_XCurrentContext()
{
    static typelib_InterfaceTypeDescription * s_type_XCurrentContext = 0;
    if (0 == s_type_XCurrentContext)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (0 == s_type_XCurrentContext)
        {
            OUString sTypeName("com.sun.star.uno.XCurrentContext");
            typelib_InterfaceTypeDescription * pTD = 0;
            typelib_TypeDescriptionReference * pMembers[1] = { 0 };
            OUString sMethodName0("com.sun.star.uno.XCurrentContext::getValueByName");
            typelib_typedescriptionreference_new(
                &pMembers[0],
                typelib_TypeClass_INTERFACE_METHOD,
                sMethodName0.pData );
            typelib_typedescription_newInterface(
                &pTD,
                sTypeName.pData, 0, 0, 0, 0, 0,
                * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ),
                1,
                pMembers );

            typelib_typedescription_register( (typelib_TypeDescription**)&pTD );
            typelib_typedescriptionreference_release( pMembers[0] );

            typelib_InterfaceMethodTypeDescription * pMethod = 0;
            typelib_Parameter_Init aParameters[1];
            OUString sParamName0("Name");
            OUString sParamType0("string");
            aParameters[0].pParamName = sParamName0.pData;
            aParameters[0].eTypeClass = typelib_TypeClass_STRING;
            aParameters[0].pTypeName = sParamType0.pData;
            aParameters[0].bIn = sal_True;
            aParameters[0].bOut = sal_False;
            rtl_uString * pExceptions[1];
            OUString sExceptionName0("com.sun.star.uno.RuntimeException");
            pExceptions[0] = sExceptionName0.pData;
            OUString sReturnType0("any");
            typelib_typedescription_newInterfaceMethod(
                &pMethod,
                3, sal_False,
                sMethodName0.pData,
                typelib_TypeClass_ANY, sReturnType0.pData,
                1, aParameters, 1, pExceptions );
            typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );
            typelib_typedescription_release( (typelib_TypeDescription*)pMethod );
            // another static ref:
            ++reinterpret_cast< typelib_TypeDescription * >( pTD )->
                nStaticRefCount;
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
    inline oslThreadKey getThreadKey() SAL_THROW(());

    inline ThreadKey( oslThreadKeyCallbackFunction pCallback ) SAL_THROW(());
    inline ~ThreadKey() SAL_THROW(());
};
//__________________________________________________________________________________________________
inline ThreadKey::ThreadKey( oslThreadKeyCallbackFunction pCallback ) SAL_THROW(())
    : _bInit( sal_False )
    , _pCallback( pCallback )
{
}
//__________________________________________________________________________________________________
inline ThreadKey::~ThreadKey() SAL_THROW(())
{
    if (_bInit)
    {
        ::osl_destroyThreadKey( _hThreadKey );
    }
}
//__________________________________________________________________________________________________
inline oslThreadKey ThreadKey::getThreadKey() SAL_THROW(())
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
IdContainer * getIdContainer() SAL_THROW(())
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
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_setCurrentContext(
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
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_getCurrentContext(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
