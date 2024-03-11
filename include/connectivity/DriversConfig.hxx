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
#ifndef INCLUDED_CONNECTIVITY_DRIVERSCONFIG_HXX
#define INCLUDED_CONNECTIVITY_DRIVERSCONFIG_HXX

#include <config_options.h>
#include <sal/config.h>

#include <map>
#include <string_view>

#include <com/sun/star/uno/Sequence.h>
#include <connectivity/dbtoolsdllapi.hxx>
#include <comphelper/singletonref.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <unotools/confignode.hxx>

namespace connectivity
{
    struct TInstalledDriver
    {
        ::comphelper::NamedValueCollection aProperties;
        ::comphelper::NamedValueCollection aFeatures;
        ::comphelper::NamedValueCollection aMetaData;
        OUString sDriverFactory;
        OUString sDriverTypeDisplayName;
    };
    typedef std::map<OUString, TInstalledDriver> TInstalledDrivers;

    class DriversConfigImpl
    {
        mutable ::utl::OConfigurationTreeRoot   m_aInstalled;
        mutable TInstalledDrivers               m_aDrivers;
    public:
        DriversConfigImpl();

        const TInstalledDrivers& getInstalledDrivers(const css::uno::Reference< css::uno::XComponentContext >& _rxORB) const;
    };

    // Allows to access all driver which are located in the configuration

    class UNLESS_MERGELIBS_MORE(OOO_DLLPUBLIC_DBTOOLS) DriversConfig
    {
        typedef comphelper::SingletonRef<DriversConfigImpl> OSharedConfigNode;

        const ::comphelper::NamedValueCollection& impl_get(std::u16string_view _sURL,sal_Int32 _nProps) const;
    public:
        DriversConfig(css::uno::Reference< css::uno::XComponentContext > _xORB);
        ~DriversConfig();

        DriversConfig( const DriversConfig& );
        DriversConfig& operator=( const DriversConfig& );

        OUString getDriverFactoryName(std::u16string_view _sUrl) const;
        OUString getDriverTypeDisplayName(std::u16string_view _sUrl) const;
        const ::comphelper::NamedValueCollection& getProperties(std::u16string_view _sURL) const;
        const ::comphelper::NamedValueCollection& getFeatures(std::u16string_view _sURL) const;
        const ::comphelper::NamedValueCollection& getMetaData(std::u16string_view _sURL) const;
        css::uno::Sequence< OUString > getURLs() const;
    private:
        OSharedConfigNode                                  m_aNode;
        css::uno::Reference< css::uno::XComponentContext > m_xORB;
    };
}
#endif // INCLUDED_CONNECTIVITY_DRIVERSCONFIG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
