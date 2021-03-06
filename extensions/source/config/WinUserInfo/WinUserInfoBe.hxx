/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <memory>

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class XComponentContext;
}
}
}
}

namespace extensions
{
namespace config
{
namespace WinUserInfo
{
class WinUserInfoBe_Impl;

typedef cppu::WeakComponentImplHelper<css::beans::XPropertySet, css::lang::XServiceInfo>
    BackendBase;

struct WinUserInfoMutexHolder
{
    osl::Mutex mMutex;
};
/**
  Implements the PlatformBackend service, a specialization of the
  XPropertySet service for retrieving Active Directory user profile
  configuration settings.
*/
class WinUserInfoBe : private WinUserInfoMutexHolder, public BackendBase
{
public:
    explicit WinUserInfoBe(const css::uno::Reference<css::uno::XComponentContext>& xContext);
    virtual ~WinUserInfoBe() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& aServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return css::uno::Reference<css::beans::XPropertySetInfo>();
    }

    virtual void SAL_CALL setPropertyValue(OUString const&, css::uno::Any const&) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(OUString const& PropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const&, css::uno::Reference<css::beans::XVetoableChangeListener> const&) override
    {
    }

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const&, css::uno::Reference<css::beans::XVetoableChangeListener> const&) override
    {
    }

private:
    std::unique_ptr<WinUserInfoBe_Impl> m_pImpl;
};
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
