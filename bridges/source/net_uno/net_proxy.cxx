/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "net_proxy.hxx"
#include "net_base.hxx"

#include <bridges/net_uno/net_context.hxx>

namespace net_uno
{
namespace
{
void SAL_CALL NetProxy_acquire(uno_Interface* pUnoI) noexcept
{
    NetProxy* that = static_cast<NetProxy*>(pUnoI);
    that->acquire();
}

void SAL_CALL NetProxy_release(uno_Interface* pUnoI) noexcept
{
    NetProxy* that = static_cast<NetProxy*>(pUnoI);
    that->release();
}

void SAL_CALL NetProxy_free([[maybe_unused]] uno_ExtEnvironment* pEnv, void* pUnoI) noexcept
{
    NetProxy* that = static_cast<NetProxy*>(pUnoI);
    delete that;
}

void SAL_CALL NetProxy_dispatch(uno_Interface* pUnoI, const typelib_TypeDescription* pMemberTD,
                                void* pUnoRet, void** pUnoArgs, uno_Any** pUnoExc) noexcept
{
    NetProxy* proxy = static_cast<NetProxy*>(pUnoI);

    try
    {
        switch (pMemberTD->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                const typelib_InterfaceAttributeTypeDescription* pAttribTD
                    = reinterpret_cast<const typelib_InterfaceAttributeTypeDescription*>(pMemberTD);

                if (pUnoRet) // getter
                {
                    proxy->m_bridge.call_net_func(proxy->m_netI, pMemberTD,
                                                  pAttribTD->pAttributeTypeRef, 0, nullptr, nullptr,
                                                  pUnoRet, pUnoExc);
                }
                else // setter
                {
                    typelib_MethodParameter param;
                    param.pTypeRef = pAttribTD->pAttributeTypeRef;
                    param.bIn = true;
                    param.bOut = false;

                    proxy->m_bridge.call_net_func(proxy->m_netI, pMemberTD, nullptr, 1, &param,
                                                  pUnoArgs, nullptr, pUnoExc);
                }
                break;
            }
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                const typelib_InterfaceMethodTypeDescription* pMethodTD
                    = reinterpret_cast<const typelib_InterfaceMethodTypeDescription*>(pMemberTD);

                sal_Int32 nMemberPos = pMethodTD->aBase.nPosition;
                assert(nMemberPos < proxy->m_TD->nAllMembers);

                sal_Int32 nFunctionPos = proxy->m_TD->pMapMemberIndexToFunctionIndex[nMemberPos];
                assert(nFunctionPos < proxy->m_TD->nMapFunctionIndexToMemberIndex);

                switch (nFunctionPos)
                {
                    case 1: // acquire()
                        NetProxy_acquire(proxy);
                        *pUnoExc = nullptr;
                        break;
                    case 2: // release()
                        NetProxy_release(proxy);
                        *pUnoExc = nullptr;
                        break;
                    default: // arbitrary method call
                        proxy->m_bridge.call_net_func(
                            proxy->m_netI, pMemberTD, pMethodTD->pReturnTypeRef, pMethodTD->nParams,
                            pMethodTD->pParams, pUnoArgs, pUnoRet, pUnoExc);
                        break;
                }
                break;
            }
            default:
            {
                throw BridgeRuntimeError(SAL_WHERE,
                                         "could not find function "
                                             + OUString::unacquired(&pMemberTD->pTypeName));
            }
        }
    }
    catch (const BridgeRuntimeError& err)
    {
        SAL_WARN("bridges", ".NET bridge error: " << err.m_message);

        css::uno::RuntimeException exc("[net_uno bridge error] " + err.m_message,
                                       css::uno::Reference<css::uno::XInterface>());
        uno_type_any_construct(*pUnoExc, &exc,
                               cppu::UnoType<css::uno::RuntimeException>::get().getTypeLibType(),
                               nullptr);
    }
}
}

NetProxy::NetProxy(Bridge& rBridge, IntPtr pNetI, typelib_InterfaceTypeDescription* pTD,
                   const OUString& sOid)
    : m_ref(1)
    , m_bridge(rBridge)
    , m_netI(nullptr)
    , m_oid(sOid)
    , m_TD(pTD)
{
    void* that = this;
    (*m_bridge.m_uno_env->registerProxyInterface)(m_bridge.m_uno_env, &that, NetProxy_free,
                                                  m_oid.pData, m_TD);

    OUString sInterfaceName = map_uno_type_to_net(pTD->aBase.pWeakRef);

    Context* pCtx = static_cast<Context*>(m_bridge.m_net_env->pContext);
    m_netI = pCtx->registerInterface(pNetI, m_oid.getStr(), sInterfaceName.getStr());

    m_bridge.acquire();

    uno_Interface::acquire = NetProxy_acquire;
    uno_Interface::release = NetProxy_release;
    uno_Interface::pDispatcher = NetProxy_dispatch;
}

NetProxy::~NetProxy()
{
    OUString sInterfaceName = map_uno_type_to_net(m_TD->aBase.pWeakRef);
    static_cast<Context*>(m_bridge.m_net_env->pContext)
        ->revokeInterface(m_oid.getStr(), sInterfaceName.getStr());

    m_bridge.release();
}

void NetProxy::acquire()
{
    // rebirth of proxy zombie
    if (osl_atomic_increment(&m_ref) == 1)
    {
        void* that = this;
        (*m_bridge.m_uno_env->registerProxyInterface)(m_bridge.m_uno_env, &that, NetProxy_free,
                                                      m_oid.pData, m_TD);
    }
}

void NetProxy::release()
{
    // revoke from uno env on last release
    if (osl_atomic_decrement(&m_ref) == 0)
    {
        (*m_bridge.m_uno_env->revokeInterface)(m_bridge.m_uno_env, this);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
