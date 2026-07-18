/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <sal/config.h>

#include <memory>

#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>


namespace connectivity::mozab
{
    typedef ::cppu::WeakComponentImplHelper<   css::mozilla::XMozillaBootstrap,
                                               css::lang::XServiceInfo > OMozillaBootstrap_BASE;
    class ProfileAccess;
    class MozillaBootstrap : public OMozillaBootstrap_BASE
    {
    private:
        ::osl::Mutex                m_aMutex;       // mutex is need to control member access
        virtual ~MozillaBootstrap() override;
        std::unique_ptr<ProfileAccess> m_ProfileAccess;
    public:

        MozillaBootstrap();

        // OComponentHelper
        virtual void disposing() override;

        // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

        // XMozillaBootstrap

        // XProfileDiscover
        virtual ::sal_Int32 getProfileCount( css::mozilla::MozillaProductType product) override;
        virtual ::sal_Int32 getProfileList( css::mozilla::MozillaProductType product, cpo::uno::Sequence< OUString >& list ) override;
        virtual OUString getDefaultProfile( css::mozilla::MozillaProductType product ) override;
        virtual OUString getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName ) override;
        virtual bool isProfileLocked( css::mozilla::MozillaProductType product, const OUString& profileName ) override;
        virtual bool getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName ) override;

        // XProfileManager
        virtual ::sal_Int32 bootupProfile( css::mozilla::MozillaProductType product, const OUString& profileName ) override;
        virtual ::sal_Int32 shutdownProfile(  ) override;
        virtual css::mozilla::MozillaProductType getCurrentProduct(  ) override;
        virtual OUString getCurrentProfile(  ) override;
        virtual bool isCurrentProfileLocked(  ) override;
        virtual OUString setCurrentProfile( css::mozilla::MozillaProductType product, const OUString& profileName ) override;

        // XProxyRunner
        virtual ::sal_Int32 Run( const css::uno::Reference< css::mozilla::XCodeProxy >& aCode ) override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
