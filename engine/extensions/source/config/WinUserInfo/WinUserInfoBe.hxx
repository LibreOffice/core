/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    explicit WinUserInfoBe();
    virtual ~WinUserInfoBe() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool supportsService(const OUString& aServiceName) override;

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override
    {
        return css::uno::Reference<css::beans::XPropertySetInfo>();
    }

    virtual void setPropertyValue(OUString const&, cpo::uno::Any const&) override;

    virtual cpo::uno::Any getPropertyValue(OUString const& PropertyName) override;

    virtual void addPropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void removePropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void addVetoableChangeListener(
        OUString const&, css::uno::Reference<css::beans::XVetoableChangeListener> const&) override
    {
    }

    virtual void removeVetoableChangeListener(
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
