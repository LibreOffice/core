/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/asyncfunc.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/uno/Reference.hxx>

namespace
{
class theAsyncFuncUnoTunnelId : public rtl::Static<UnoTunnelIdInit, theAsyncFuncUnoTunnelId>
{
};
}

AsyncFunc::AsyncFunc(const std::function<void()>& rAsyncFunc)
    : m_pAsyncFunc(rAsyncFunc)
{
}

AsyncFunc::~AsyncFunc() {}

void AsyncFunc::Execute()
{
    if (m_pAsyncFunc)
        m_pAsyncFunc();
}

const css::uno::Sequence<sal_Int8>& AsyncFunc::getUnoTunnelId()
{
    return theAsyncFuncUnoTunnelId::get().getSeq();
}

AsyncFunc* AsyncFunc::getImplementation(const css::uno::Reference<css::uno::XInterface>& xInterface)
{
    css::uno::Reference<css::lang::XUnoTunnel> xUnoTunnel(xInterface, css::uno::UNO_QUERY);
    if (xUnoTunnel.is())
    {
        return reinterpret_cast<AsyncFunc*>(xUnoTunnel->getSomething(AsyncFunc::getUnoTunnelId()));
    }

    return nullptr;
}

//XUnoTunnel
sal_Int64 SAL_CALL AsyncFunc::getSomething(const css::uno::Sequence<sal_Int8>& rId)
{
    if (rId.getLength() == 16
        && 0 == memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
