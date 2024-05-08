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
#include <config_fuzzers.h>

#include <connectivity/DriversConfig.hxx>
#include <o3tl/string_view.hxx>
#include <tools/wldcrd.hxx>
#include <comphelper/sequence.hxx>
#include <utility>

using namespace connectivity;
using namespace utl;
using namespace ::com::sun::star;

namespace
{
    void lcl_convert(const uno::Sequence< OUString >& _aSource,uno::Any& _rDest)
    {
        uno::Sequence<uno::Any> aRet(_aSource.getLength());
        std::transform(_aSource.begin(), _aSource.end(), aRet.getArray(),
                       [](auto& str) { return uno::Any(str); });
        _rDest <<= aRet;
    }
    void lcl_fillValues(const ::utl::OConfigurationNode& _aURLPatternNode,const OUString& _sNode,::comphelper::NamedValueCollection& _rValues)
    {
        const ::utl::OConfigurationNode aPropertiesNode = _aURLPatternNode.openNode(_sNode);
        if ( !aPropertiesNode.isValid() )
            return;

        uno::Sequence< OUString > aStringSeq;
        for (auto& prop : aPropertiesNode.getNodeNames())
        {
            uno::Any aValue = aPropertiesNode.getNodeValue(prop + "/Value");
            if ( aValue >>= aStringSeq )
            {
                lcl_convert(aStringSeq,aValue);
            }
            _rValues.put(prop, aValue);
        } // for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter,++pNamedIter)
    }
    void lcl_readURLPatternNode(const ::utl::OConfigurationTreeRoot& _aInstalled,const OUString& _sEntry,TInstalledDriver& _rInstalledDriver)
    {
        const ::utl::OConfigurationNode aURLPatternNode = _aInstalled.openNode(_sEntry);
        if ( !aURLPatternNode.isValid() )
            return;

        OUString sParentURLPattern;
        aURLPatternNode.getNodeValue(u"ParentURLPattern"_ustr) >>= sParentURLPattern;
        if ( !sParentURLPattern.isEmpty() )
            lcl_readURLPatternNode(_aInstalled,sParentURLPattern,_rInstalledDriver);

        OUString sDriverFactory;
        aURLPatternNode.getNodeValue(u"Driver"_ustr) >>= sDriverFactory;
        if ( !sDriverFactory.isEmpty() )
            _rInstalledDriver.sDriverFactory = sDriverFactory;

        OUString sDriverTypeDisplayName;
        aURLPatternNode.getNodeValue(u"DriverTypeDisplayName"_ustr) >>= sDriverTypeDisplayName;
        OSL_ENSURE(!sDriverTypeDisplayName.isEmpty(),"No valid DriverTypeDisplayName property!");
        if ( !sDriverTypeDisplayName.isEmpty() )
            _rInstalledDriver.sDriverTypeDisplayName = sDriverTypeDisplayName;

        lcl_fillValues(aURLPatternNode,u"Properties"_ustr,_rInstalledDriver.aProperties);
        lcl_fillValues(aURLPatternNode,u"Features"_ustr,_rInstalledDriver.aFeatures);
        lcl_fillValues(aURLPatternNode,u"MetaData"_ustr,_rInstalledDriver.aMetaData);
    }
}

DriversConfigImpl::DriversConfigImpl()
{
}

const TInstalledDrivers& DriversConfigImpl::getInstalledDrivers(const uno::Reference< uno::XComponentContext >& _rxORB) const
{
    if ( m_aDrivers.empty() )
    {
        if ( !m_aInstalled.isValid() )
        {
            m_aInstalled = ::utl::OConfigurationTreeRoot::createWithComponentContext(_rxORB,
                             u"org.openoffice.Office.DataAccess.Drivers/Installed"_ustr, -1, ::utl::OConfigurationTreeRoot::CM_READONLY);
        }

        if ( m_aInstalled.isValid() )
        {
            for (auto& pattern : m_aInstalled.getNodeNames())
            {
                TInstalledDriver aInstalledDriver;
                lcl_readURLPatternNode(m_aInstalled, pattern, aInstalledDriver);
                if ( !aInstalledDriver.sDriverFactory.isEmpty() )
                    m_aDrivers.emplace(pattern, aInstalledDriver);
            }
        } // if ( m_aInstalled.isValid() )
    }
    return m_aDrivers;
}

DriversConfig::DriversConfig(uno::Reference< uno::XComponentContext > _xORB)
:m_xORB(std::move(_xORB))
{
}


DriversConfig::~DriversConfig()
{
}


DriversConfig::DriversConfig( const DriversConfig& _rhs )
{
    *this = _rhs;
}


DriversConfig& DriversConfig::operator=( const DriversConfig& _rhs )
{
    if ( this != &_rhs )
    {
        m_aNode = _rhs.m_aNode;
    }
    return *this;
}


OUString DriversConfig::getDriverFactoryName(std::u16string_view _sURL) const
{
#if ENABLE_FUZZERS
    if (o3tl::starts_with(_sURL, u"sdbc:dbase:"))
        return "com.sun.star.comp.sdbc.dbase.ODriver";
#endif

    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    OUString sRet;
    OUString sOldPattern;
    for(const auto& [rPattern, rDriver] : rDrivers)
    {
        WildCard aWildCard(rPattern);
        if ( sOldPattern.getLength() < rPattern.getLength() && aWildCard.Matches(_sURL) )
        {
            sRet = rDriver.sDriverFactory;
            sOldPattern = rPattern;
        }
    }

    return sRet;
}

OUString DriversConfig::getDriverTypeDisplayName(std::u16string_view _sURL) const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    OUString sRet;
    OUString sOldPattern;
    for(const auto& [rPattern, rDriver] : rDrivers)
    {
        WildCard aWildCard(rPattern);
        if ( sOldPattern.getLength() < rPattern.getLength() && aWildCard.Matches(_sURL) )
        {
            sRet = rDriver.sDriverTypeDisplayName;
            sOldPattern = rPattern;
        }
    }

    return sRet;
}

const ::comphelper::NamedValueCollection& DriversConfig::getProperties(std::u16string_view _sURL)
    const
{
    return impl_get(_sURL,1);
}

const ::comphelper::NamedValueCollection& DriversConfig::getFeatures(std::u16string_view _sURL)
    const
{
    return impl_get(_sURL,0);
}

const ::comphelper::NamedValueCollection& DriversConfig::getMetaData(std::u16string_view _sURL)
    const
{
    return impl_get(_sURL,2);
}

const ::comphelper::NamedValueCollection& DriversConfig::impl_get(std::u16string_view _sURL,sal_Int32 _nProps) const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    const ::comphelper::NamedValueCollection* pRet = nullptr;
    OUString sOldPattern;
    for(const auto& [rPattern, rDriver] : rDrivers)
    {
        WildCard aWildCard(rPattern);
        if ( sOldPattern.getLength() < rPattern.getLength() && aWildCard.Matches(_sURL) )
        {
            switch(_nProps)
            {
                case 0:
                    pRet = &rDriver.aFeatures;
                    break;
                case 1:
                    pRet = &rDriver.aProperties;
                    break;
                case 2:
                    pRet = &rDriver.aMetaData;
                    break;
            }
            sOldPattern = rPattern;
        }
    } // for(;aIter != aEnd;++aIter)
    if ( pRet == nullptr )
    {
        static const ::comphelper::NamedValueCollection s_sEmpty;
        pRet = &s_sEmpty;
    }
    return *pRet;
}

uno::Sequence< OUString > DriversConfig::getURLs() const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    return comphelper::mapKeysToSequence(rDrivers);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
