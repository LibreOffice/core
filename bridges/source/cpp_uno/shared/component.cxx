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

#include "bridges/cpp_uno/shared/bridge.hxx"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/mutex.hxx"
#include "osl/time.h"
#include "rtl/process.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include <sal/log.hxx>
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"
#include "uno/mapping.h"
#include "cppu/EnvDcp.hxx"

namespace bridges { namespace cpp_uno { namespace shared {

} } }

namespace {

#if (defined(__GNUC__) && defined(__APPLE__))
static OUString * s_pStaticOidPart = 0;
#endif

const OUString & SAL_CALL cppu_cppenv_getStaticOIdPart()
{
#if ! (defined(__GNUC__) && defined(__APPLE__))
    static OUString * s_pStaticOidPart = nullptr;
#endif
    if (! s_pStaticOidPart)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            OUStringBuffer aRet( 64 );
            aRet.append( "];" );
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            for ( sal_Int32 i = 0; i < 16; ++i )
            {
                aRet.append( (sal_Int32)ar[i], 16 );
            }
#if (defined(__GNUC__) && defined(__APPLE__))
            s_pStaticOidPart = new OUString( aRet.makeStringAndClear() );
#else
            static OUString s_aStaticOidPart(
                aRet.makeStringAndClear() );
            s_pStaticOidPart = &s_aStaticOidPart;
#endif
        }
    }
    return *s_pStaticOidPart;
}

}

extern "C" {

static void s_stub_computeObjectIdentifier(va_list * pParam)
{
    uno_ExtEnvironment  * pEnv       = va_arg(*pParam, uno_ExtEnvironment *);
    rtl_uString        ** ppOId      = va_arg(*pParam, rtl_uString **);
    void                * pInterface = va_arg(*pParam, void *);


    assert(pEnv && ppOId && pInterface);
    if (pEnv && ppOId && pInterface)
    {
        if (*ppOId)
        {
            rtl_uString_release( *ppOId );
            *ppOId = nullptr;
        }

        try
        {
            ::com::sun::star::uno::Reference<
                  ::com::sun::star::uno::XInterface > xHome(
                      static_cast< ::com::sun::star::uno::XInterface * >(
                          pInterface ),
                      ::com::sun::star::uno::UNO_QUERY );
            assert(xHome.is() && "### query to XInterface failed!");
            if (xHome.is())
            {
                // interface
                OUStringBuffer oid( 64 );
                oid.append( reinterpret_cast< sal_Int64 >(xHome.get()), 16 );
                oid.append( ';' );
                // ;environment[context]
                oid.append( OUString::unacquired(&pEnv->aBase.pTypeName) );
                oid.append( '[' );
                oid.append(
                    reinterpret_cast< sal_Int64 >(pEnv->aBase.pContext),
                    16 );
                // ];good guid
                oid.append( cppu_cppenv_getStaticOIdPart() );
                OUString aRet( oid.makeStringAndClear() );
                ::rtl_uString_acquire( *ppOId = aRet.pData );
            }
        }
        catch (const ::com::sun::star::uno::RuntimeException & e)
        {
            SAL_WARN("bridges",
                "### RuntimeException occurred during queryInterface(): "
                << e.Message);
        }
    }
}

static void SAL_CALL computeObjectIdentifier(
    uno_ExtEnvironment * pExtEnv, rtl_uString ** ppOId, void * pInterface )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_computeObjectIdentifier, pExtEnv, ppOId, pInterface);
}

static void s_stub_acquireInterface(va_list * pParam)
{
    /*uno_ExtEnvironment * pExtEnv = */va_arg(*pParam, uno_ExtEnvironment *);
    void               * pCppI   = va_arg(*pParam, void *);

    static_cast< ::com::sun::star::uno::XInterface * >( pCppI )->acquire();
}

static void SAL_CALL acquireInterface( uno_ExtEnvironment * pExtEnv, void * pCppI )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_acquireInterface, pExtEnv, pCppI);
}

static void s_stub_releaseInterface(va_list * pParam)
{
    /*uno_ExtEnvironment * pExtEnv = */va_arg(*pParam, uno_ExtEnvironment *);
    void               * pCppI   = va_arg(*pParam, void *);

    static_cast< ::com::sun::star::uno::XInterface * >( pCppI )->release();
}

static void SAL_CALL releaseInterface( uno_ExtEnvironment * pExtEnv, void * pCppI )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_releaseInterface, pExtEnv, pCppI);
}

static void SAL_CALL environmentDisposing(
    SAL_UNUSED_PARAMETER uno_Environment * )
{
}

#ifdef DISABLE_DYNLOADING
#define uno_initEnvironment CPPU_ENV_uno_initEnvironment
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_initEnvironment(uno_Environment * pCppEnv)
    SAL_THROW_EXTERN_C()
{
    assert(pCppEnv->pExtEnv);
    assert(
        ::rtl_ustr_ascii_compare_WithLength(
             pCppEnv->pTypeName->buffer, rtl_str_getLength(CPPU_CURRENT_LANGUAGE_BINDING_NAME), CPPU_CURRENT_LANGUAGE_BINDING_NAME )
        == 0
        && "### wrong environment type!");
    reinterpret_cast<uno_ExtEnvironment *>(pCppEnv)->computeObjectIdentifier
        = computeObjectIdentifier;
    reinterpret_cast<uno_ExtEnvironment *>(pCppEnv)->acquireInterface = acquireInterface;
    reinterpret_cast<uno_ExtEnvironment *>(pCppEnv)->releaseInterface = releaseInterface;
    pCppEnv->environmentDisposing = environmentDisposing;
}

#ifdef DISABLE_DYNLOADING
#define uno_ext_getMapping CPPU_ENV_uno_ext_getMapping
#endif

SAL_DLLPUBLIC_EXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo)
    SAL_THROW_EXTERN_C()
{
    assert(ppMapping && pFrom && pTo);
    if (ppMapping && pFrom && pTo && pFrom->pExtEnv && pTo->pExtEnv)
    {
        uno_Mapping * pMapping = nullptr;

        OUString from_envTypeName(cppu::EnvDcp::getTypeName(pFrom->pTypeName));
        OUString to_envTypeName(cppu::EnvDcp::getTypeName(pTo->pTypeName));

        if (0 == rtl_ustr_ascii_compare(
                from_envTypeName.pData->buffer,
                CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
            0 == rtl_ustr_ascii_compare(
                to_envTypeName.pData->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = bridges::cpp_uno::shared::Bridge::createMapping(
                pFrom->pExtEnv, pTo->pExtEnv, true );
            ::uno_registerMapping(
                &pMapping, bridges::cpp_uno::shared::freeMapping,
                &pFrom->pExtEnv->aBase,
                &pTo->pExtEnv->aBase, nullptr );
        }
        else if (0 == rtl_ustr_ascii_compare(
                     to_envTypeName.pData->buffer,
                     CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
                 0 == rtl_ustr_ascii_compare(
                     from_envTypeName.pData->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = bridges::cpp_uno::shared::Bridge::createMapping(
                pTo->pExtEnv, pFrom->pExtEnv, false );
            ::uno_registerMapping(
                &pMapping, bridges::cpp_uno::shared::freeMapping,
                &pFrom->pExtEnv->aBase,
                &pTo->pExtEnv->aBase, nullptr );
        }

        if (*ppMapping)
        {
            (*(*ppMapping)->release)( *ppMapping );
        }
        if (pMapping)
        *ppMapping = pMapping;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
