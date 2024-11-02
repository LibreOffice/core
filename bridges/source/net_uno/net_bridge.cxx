/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "net_bridge.hxx"

#include <bridges/net_uno/net_context.hxx>
#include <sal/log.hxx>

namespace net_uno
{
namespace
{
void SAL_CALL Mapping_acquire(uno_Mapping* mapping) noexcept
{
    Mapping* that = static_cast<Mapping*>(mapping);
    that->m_bridge->acquire();
}

void SAL_CALL Mapping_release(uno_Mapping* mapping) noexcept
{
    Mapping* that = static_cast<Mapping*>(mapping);
    that->m_bridge->release();
}

void SAL_CALL Mapping_free(uno_Mapping* mapping) noexcept
{
    Mapping* that = static_cast<Mapping*>(mapping);
    delete that->m_bridge;
}

void SAL_CALL Mapping_net2uno(uno_Mapping* mapping, void** ppOut, void* pIn,
                              typelib_InterfaceTypeDescription* pTD) noexcept
{
    assert(ppOut && pTD && "### null ptr!");

    Mapping* that = static_cast<Mapping*>(mapping);
    Bridge* bridge = that->m_bridge;

    if (pIn)
    {
        Value interfaceValue;
        interfaceValue.interfaceData = pIn;
        bridge->map_net_value_to_uno(*ppOut, &interfaceValue, pTD->aBase.pWeakRef, true, true);
    }
}

void SAL_CALL Mapping_uno2net(uno_Mapping* mapping, void** ppOut, void* pIn,
                              typelib_InterfaceTypeDescription* pTD) noexcept
{
    assert(ppOut && pTD && "### null ptr!");

    Mapping* that = static_cast<Mapping*>(mapping);
    Bridge* bridge = that->m_bridge;

    if (pIn)
    {
        Value interfaceValue;
        bridge->map_uno_to_net_value(&pIn, &interfaceValue, pTD->aBase.pWeakRef, false);
        *ppOut = interfaceValue.interfaceData;
    }
}
}

Bridge::Bridge(uno_Environment* uno_net_env, uno_ExtEnvironment* uno_env, bool registered_net2uno)
    : m_ref(1)
    , m_uno_env(uno_env)
    , m_net_env(uno_net_env)
    , m_registered_net2uno(registered_net2uno)
{
    assert(m_net_env && m_uno_env);

    (*m_uno_env->aBase.acquire)(&m_uno_env->aBase);
    (*m_net_env->acquire)(m_net_env);

    // net2uno
    m_net2uno.acquire = Mapping_acquire;
    m_net2uno.release = Mapping_release;
    m_net2uno.mapInterface = Mapping_net2uno;
    m_net2uno.m_bridge = this;

    // uno2net
    m_uno2net.acquire = Mapping_acquire;
    m_uno2net.release = Mapping_release;
    m_uno2net.mapInterface = Mapping_uno2net;
    m_uno2net.m_bridge = this;
}

Bridge::~Bridge()
{
    (*m_net_env->release)(m_net_env);
    (*m_uno_env->aBase.release)(&m_uno_env->aBase);
}

void Bridge::acquire()
{
    if (osl_atomic_increment(&m_ref) == 1)
    {
        if (m_registered_net2uno)
        {
            uno_Mapping* mapping = &m_net2uno;
            uno_registerMapping(&mapping, Mapping_free, m_net_env, &m_uno_env->aBase, nullptr);
        }
        else
        {
            uno_Mapping* mapping = &m_uno2net;
            uno_registerMapping(&mapping, Mapping_free, &m_uno_env->aBase, m_net_env, nullptr);
        }
    }
}

void Bridge::release()
{
    if (osl_atomic_decrement(&m_ref) == 0)
    {
        uno_revokeMapping(m_registered_net2uno ? &m_net2uno : &m_uno2net);
    }
}

extern "C" {
static void net_env_disposing(uno_Environment* env)
{
    // TODO: more rigorous lifetime control
    // TODO: invoke some dispose routine on .NET side
    delete static_cast<Context*>(env->pContext);
}

SAL_DLLPUBLIC_EXPORT void uno_initEnvironment(uno_Environment* net_env) noexcept
{
    // The code creating the uno_Environment needs to initialize
    // pContext with a Context object, complete with all callbacks.
    assert(net_env->pContext);

    net_env->pExtEnv = nullptr;
    net_env->environmentDisposing = net_env_disposing;
}

SAL_DLLPUBLIC_EXPORT void uno_ext_getMapping(uno_Mapping** ppMapping, uno_Environment* pFrom,
                                             uno_Environment* pTo) noexcept
{
    assert(ppMapping && pFrom && pTo);

    if (*ppMapping)
    {
        (*(*ppMapping)->release)(*ppMapping);
        *ppMapping = nullptr;
    }

    const OUString& from_env_typename = OUString::unacquired(&pFrom->pTypeName);
    const OUString& to_env_typename = OUString::unacquired(&pTo->pTypeName);

    uno_Mapping* mapping = nullptr;
    if (from_env_typename == UNO_LB_NET && to_env_typename == UNO_LB_UNO)
    {
        Bridge* bridge = new Bridge(pFrom, pTo->pExtEnv, true);
        mapping = &bridge->m_net2uno;
        uno_registerMapping(&mapping, Mapping_free, pFrom, &pTo->pExtEnv->aBase, nullptr);
    }
    else if (from_env_typename == UNO_LB_UNO && to_env_typename == UNO_LB_NET)
    {
        Bridge* bridge = new Bridge(pTo, pFrom->pExtEnv, false);
        mapping = &bridge->m_uno2net;
        uno_registerMapping(&mapping, Mapping_free, &pFrom->pExtEnv->aBase, pTo, nullptr);
    }
    *ppMapping = mapping;
}

SAL_DLLPUBLIC_EXPORT IntPtr allocateMemory(int nBytes) { return std::malloc(nBytes); }

SAL_DLLPUBLIC_EXPORT void freeMemory(IntPtr pMemory) { std::free(pMemory); }

SAL_DLLPUBLIC_EXPORT void releaseProxy(IntPtr pBridge, IntPtr pUnoInterface, IntPtr pTypeDesc)
{
    Bridge* bridge = static_cast<Bridge*>(pBridge);
    uno_Interface* pUnoI = static_cast<uno_Interface*>(pUnoInterface);
    typelib_TypeDescription* pTD = static_cast<typelib_TypeDescription*>(pTypeDesc);

    // Revoke from UNO; already revoked from .NET
    (*bridge->m_uno_env->revokeInterface)(bridge->m_uno_env, pUnoI);

    (*pUnoI->release)(pUnoI);
    typelib_typedescription_release(pTD);
    bridge->release();
}

SAL_DLLPUBLIC_EXPORT sal_Bool dispatchCall(IntPtr pBridge, IntPtr pUnoInterface, IntPtr pTypeDesc,
                                           String sFunctionName, Value* pArgs, Value* pRet,
                                           Value* pExc)
{
    Bridge* bridge = static_cast<Bridge*>(pBridge);
    Context* context = static_cast<Context*>(bridge->m_net_env->pContext);

    OUString sMethodName(sFunctionName);

    try
    {
        uno_Interface* pUnoI = static_cast<uno_Interface*>(pUnoInterface);
        typelib_InterfaceTypeDescription* pTD
            = static_cast<typelib_InterfaceTypeDescription*>(pTypeDesc);

        for (sal_Int32 i = 0; i < pTD->nAllMembers; ++i)
        {
            // Try to avoid getting typedescription as long as possible because
            // of Mutex.acquire() in typelib_typedescriptionreference_getDescription()
            typelib_TypeDescriptionReference* memberType = pTD->ppAllMembers[i];

            // Check method_name against fully qualified typeName of memberType
            // typeName is of the form <name> "::" <method_name> *(":@" <idx> "," <idx> ":" <name>)
            const OUString& typeName = OUString::unacquired(&memberType->pTypeName);
            sal_Int32 offset = typeName.indexOf(':') + 2;
            sal_Int32 remainder = typeName.getLength() - offset;
            assert(offset >= 2 && offset < typeName.getLength() && typeName[offset - 1] == ':');

            switch (memberType->eTypeClass)
            {
                case typelib_TypeClass_INTERFACE_METHOD:
                    if ((sMethodName.getLength() == remainder
                         || (sMethodName.getLength() < remainder
                             && typeName[offset + sMethodName.getLength()] == ':'))
                        && typeName.match(sMethodName, offset))
                    {
                        TypeDescHolder memberTD(memberType);
                        typelib_InterfaceMethodTypeDescription* pMethodTD
                            = reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(
                                memberTD.get());
                        return bridge->call_uno_func(pUnoI, memberTD.get(),
                                                     pMethodTD->pReturnTypeRef, pMethodTD->nParams,
                                                     pMethodTD->pParams, pArgs, pRet, pExc);
                    }
                    break;

                case typelib_TypeClass_INTERFACE_ATTRIBUTE:
                    if (sMethodName.getLength() > 4
                        && (sMethodName.getLength() - 4 == remainder
                            || (sMethodName.getLength() - 4 < remainder
                                && typeName[offset + (sMethodName.getLength() - 4)] == ':'))
                        && sMethodName[1] == 'e' && sMethodName[2] == 't'
                        && rtl_ustr_compare_WithLength(
                               typeName.getStr() + offset, sMethodName.getLength() - 4,
                               sMethodName.getStr() + 4, sMethodName.getLength() - 4)
                               == 0)
                    {
                        TypeDescHolder memberTD(memberType);
                        typelib_InterfaceAttributeTypeDescription* pAttribTD
                            = reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(
                                memberTD.get());
                        if (sMethodName[0] == 'g')
                        {
                            return bridge->call_uno_func(pUnoI, memberTD.get(),
                                                         pAttribTD->pAttributeTypeRef, 0, nullptr,
                                                         pArgs, pRet, pExc);
                        }
                        else if (sMethodName[0] == 's' && !pAttribTD->bReadOnly)
                        {
                            typelib_MethodParameter param;
                            param.pTypeRef = pAttribTD->pAttributeTypeRef;
                            param.bIn = true;
                            param.bOut = false;

                            return bridge->call_uno_func(pUnoI, memberTD.get(),
                                                         pAttribTD->pAttributeTypeRef, 1, &param,
                                                         pArgs, pRet, pExc);
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        // No matching method info found
        throw BridgeRuntimeError(SAL_WHERE, "could not find function " + sMethodName
                                                + " to call from type "
                                                + OUString::unacquired(&pTD->aBase.pTypeName));
    }
    catch (const BridgeRuntimeError& err)
    {
        SAL_WARN("bridges", ".NET bridge error: " << err.m_message);
        context->throwError(err.m_location.getStr(), err.m_message.getStr());
        return false;
    }
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
