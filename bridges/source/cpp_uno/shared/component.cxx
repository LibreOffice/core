/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include "component.hxx"

#include "bridges/cpp_uno/shared/bridge.hxx"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "osl/time.h"
#include "rtl/process.h"
#include "rtl/unload.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"
#include "uno/mapping.h"
#include "cppu/EnvDcp.hxx"

namespace bridges { namespace cpp_uno { namespace shared {

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

} } }

namespace {

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) \
    || (defined(__GNUC__) && defined(__APPLE__))
static ::rtl::OUString * s_pStaticOidPart = 0;
#endif

const ::rtl::OUString & SAL_CALL cppu_cppenv_getStaticOIdPart() SAL_THROW( () )
{
#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) \
    || (defined(__GNUC__) && defined(__APPLE__)))
    static ::rtl::OUString * s_pStaticOidPart = 0;
#endif
    if (! s_pStaticOidPart)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            ::rtl::OUStringBuffer aRet( 64 );
            aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            for ( sal_Int32 i = 0; i < 16; ++i )
            {
                aRet.append( (sal_Int32)ar[i], 16 );
            }
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) \
    || (defined(__GNUC__) && defined(__APPLE__))
            s_pStaticOidPart = new ::rtl::OUString( aRet.makeStringAndClear() );
#else
            static ::rtl::OUString s_aStaticOidPart(
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
    SAL_THROW( () )
{
    uno_ExtEnvironment  * pEnv       = va_arg(*pParam, uno_ExtEnvironment *);
    rtl_uString        ** ppOId      = va_arg(*pParam, rtl_uString **);
    void                * pInterface = va_arg(*pParam, void *);


    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (pEnv && ppOId && pInterface)
    {
        if (*ppOId)
        {
            rtl_uString_release( *ppOId );
            *ppOId = 0;
        }

        try
        {
            ::com::sun::star::uno::Reference<
                  ::com::sun::star::uno::XInterface > xHome(
                      reinterpret_cast< ::com::sun::star::uno::XInterface * >(
                          pInterface ),
                      ::com::sun::star::uno::UNO_QUERY );
            OSL_ENSURE( xHome.is(), "### query to XInterface failed!" );
            if (xHome.is())
            {
                // interface
                ::rtl::OUStringBuffer oid( 64 );
                oid.append( reinterpret_cast< sal_Int64 >(xHome.get()), 16 );
                oid.append( (sal_Unicode)';' );
                // ;environment[context]
                oid.append(
                    *reinterpret_cast< ::rtl::OUString const * >(
                        &((uno_Environment *) pEnv)->pTypeName ) );
                oid.append( (sal_Unicode)'[' );
                oid.append(
                    reinterpret_cast< sal_Int64 >(
                        ((uno_Environment *)pEnv)->pContext),
                    16 );
                // ];good guid
                oid.append( cppu_cppenv_getStaticOIdPart() );
                ::rtl::OUString aRet( oid.makeStringAndClear() );
                ::rtl_uString_acquire( *ppOId = aRet.pData );
            }
        }
        catch (::com::sun::star::uno::RuntimeException &)
        {
            OSL_ENSURE(
                0, "### RuntimeException occurred udring queryInterface()!" );
        }
    }
}

static void SAL_CALL computeObjectIdentifier(
    uno_ExtEnvironment * pExtEnv, rtl_uString ** ppOId, void * pInterface )
    SAL_THROW( () )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_computeObjectIdentifier, pExtEnv, ppOId, pInterface);
}

static void s_stub_acquireInterface(va_list * pParam)
    SAL_THROW( () )
{
    /*uno_ExtEnvironment * pExtEnv = */va_arg(*pParam, uno_ExtEnvironment *);
    void               * pCppI   = va_arg(*pParam, void *);

    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->acquire();
}

static void SAL_CALL acquireInterface( uno_ExtEnvironment * pExtEnv, void * pCppI )
    SAL_THROW( () )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_acquireInterface, pExtEnv, pCppI);
}

static void s_stub_releaseInterface(va_list * pParam)
    SAL_THROW( () )
{
    /*uno_ExtEnvironment * pExtEnv = */va_arg(*pParam, uno_ExtEnvironment *);
    void               * pCppI   = va_arg(*pParam, void *);

    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->release();
}

static void SAL_CALL releaseInterface( uno_ExtEnvironment * pExtEnv, void * pCppI )
    SAL_THROW( () )
{
    uno_Environment_invoke(&pExtEnv->aBase, s_stub_releaseInterface, pExtEnv, pCppI);
}

static void SAL_CALL environmentDisposing( uno_Environment * ) SAL_THROW( () )
{
    bridges::cpp_uno::shared::g_moduleCount.modCnt.release(
        &bridges::cpp_uno::shared::g_moduleCount.modCnt );
}

sal_Bool SAL_CALL component_canUnload(TimeValue * pTime) SAL_THROW_EXTERN_C() {
    return bridges::cpp_uno::shared::g_moduleCount.canUnload(
        &bridges::cpp_uno::shared::g_moduleCount, pTime);
}

void SAL_CALL uno_initEnvironment(uno_Environment * pCppEnv)
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pCppEnv->pExtEnv, "### expected extended environment!" );
    OSL_ENSURE(
        ::rtl_ustr_ascii_compare_WithLength(
             pCppEnv->pTypeName->buffer, rtl_str_getLength(CPPU_CURRENT_LANGUAGE_BINDING_NAME), CPPU_CURRENT_LANGUAGE_BINDING_NAME )
        == 0,
        "### wrong environment type!" );
    bridges::cpp_uno::shared::g_moduleCount.modCnt.acquire(
        &bridges::cpp_uno::shared::g_moduleCount.modCnt );
    ((uno_ExtEnvironment *)pCppEnv)->computeObjectIdentifier
        = computeObjectIdentifier;
    ((uno_ExtEnvironment *)pCppEnv)->acquireInterface = acquireInterface;
    ((uno_ExtEnvironment *)pCppEnv)->releaseInterface = releaseInterface;
    pCppEnv->environmentDisposing = environmentDisposing;
}

void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo)
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( ppMapping && pFrom && pTo );
    if (ppMapping && pFrom && pTo && pFrom->pExtEnv && pTo->pExtEnv)
    {
        uno_Mapping * pMapping = 0;

        rtl::OUString from_envTypeName(cppu::EnvDcp::getTypeName(pFrom->pTypeName));
        rtl::OUString to_envTypeName(cppu::EnvDcp::getTypeName(pTo->pTypeName));

        if (0 == rtl_ustr_ascii_compare(
                from_envTypeName.pData->buffer,
                CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
            0 == rtl_ustr_ascii_compare(
                to_envTypeName.pData->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = bridges::cpp_uno::shared::Bridge::createMapping(
                pFrom->pExtEnv, pTo->pExtEnv, sal_True );
            ::uno_registerMapping(
                &pMapping, bridges::cpp_uno::shared::freeMapping,
                (uno_Environment *)pFrom->pExtEnv,
                (uno_Environment *)pTo->pExtEnv, 0 );
        }
        else if (0 == rtl_ustr_ascii_compare(
                     to_envTypeName.pData->buffer,
                     CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
                 0 == rtl_ustr_ascii_compare(
                     from_envTypeName.pData->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = bridges::cpp_uno::shared::Bridge::createMapping(
                pTo->pExtEnv, pFrom->pExtEnv, sal_False );
            ::uno_registerMapping(
                &pMapping, bridges::cpp_uno::shared::freeMapping,
                (uno_Environment *)pFrom->pExtEnv,
                (uno_Environment *)pTo->pExtEnv, 0 );
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
