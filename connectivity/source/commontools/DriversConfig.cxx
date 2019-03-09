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
#include <connectivity/DriversConfig.hxx>
#include <tools/wldcrd.hxx>
#include <comphelper/sequence.hxx>

using namespace connectivity;
using namespace utl;
using namespace ::com::sun::star;

namespace
{
    void lcl_convert(const uno::Sequence< OUString >& _aSource,uno::Any& _rDest)
    {
        uno::Sequence<uno::Any> aRet(_aSource.getLength());
        uno::Any* pAny = aRet.getArray();
        const OUString* pIter = _aSource.getConstArray();
        const OUString* pEnd  = pIter + _aSource.getLength();
        for (;pIter != pEnd ; ++pIter,++pAny)
        {
            *pAny <<= *pIter;
        }
        _rDest <<= aRet;
    }
    void lcl_fillValues(const ::utl::OConfigurationNode& _aURLPatternNode,const OUString& _sNode,::comphelper::NamedValueCollection& _rValues)
    {
        const ::utl::OConfigurationNode aPropertiesNode = _aURLPatternNode.openNode(_sNode);
        if ( aPropertiesNode.isValid() )
        {
            uno::Sequence< OUString > aStringSeq;
            const uno::Sequence< OUString > aProperties = aPropertiesNode.getNodeNames();
            const OUString* pPropertiesIter = aProperties.getConstArray();
            const OUString* pPropertiesEnd  = pPropertiesIter + aProperties.getLength();
            for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter)
            {
                uno::Any aValue = aPropertiesNode.getNodeValue(*pPropertiesIter + "/Value");
                if ( aValue >>= aStringSeq )
                {
                    lcl_convert(aStringSeq,aValue);
                }
                _rValues.put(*pPropertiesIter,aValue);
            } // for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter,++pNamedIter)
        } // if ( aPropertiesNode.isValid() )
    }
    void lcl_readURLPatternNode(const ::utl::OConfigurationTreeRoot& _aInstalled,const OUString& _sEntry,TInstalledDriver& _rInstalledDriver)
    {
        const ::utl::OConfigurationNode aURLPatternNode = _aInstalled.openNode(_sEntry);
        if ( aURLPatternNode.isValid() )
        {
            OUString sParentURLPattern;
            aURLPatternNode.getNodeValue("ParentURLPattern") >>= sParentURLPattern;
            if ( !sParentURLPattern.isEmpty() )
                lcl_readURLPatternNode(_aInstalled,sParentURLPattern,_rInstalledDriver);

            OUString sDriverFactory;
            aURLPatternNode.getNodeValue("Driver") >>= sDriverFactory;
            if ( !sDriverFactory.isEmpty() )
                _rInstalledDriver.sDriverFactory = sDriverFactory;

            OUString sDriverTypeDisplayName;
            aURLPatternNode.getNodeValue("DriverTypeDisplayName") >>= sDriverTypeDisplayName;
            OSL_ENSURE(!sDriverTypeDisplayName.isEmpty(),"No valid DriverTypeDisplayName property!");
            if ( !sDriverTypeDisplayName.isEmpty() )
                _rInstalledDriver.sDriverTypeDisplayName = sDriverTypeDisplayName;

            lcl_fillValues(aURLPatternNode,"Properties",_rInstalledDriver.aProperties);
            lcl_fillValues(aURLPatternNode,"Features",_rInstalledDriver.aFeatures);
            lcl_fillValues(aURLPatternNode,"MetaData",_rInstalledDriver.aMetaData);
        }
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
                             "org.openoffice.Office.DataAccess.Drivers/Installed", -1, ::utl::OConfigurationTreeRoot::CM_READONLY);
        }

        if ( m_aInstalled.isValid() )
        {
            const uno::Sequence< OUString > aURLPatterns = m_aInstalled.getNodeNames();
            const OUString* pPatternIter = aURLPatterns.getConstArray();
            const OUString* pPatternEnd  = pPatternIter + aURLPatterns.getLength();
            for (;pPatternIter != pPatternEnd ; ++pPatternIter)
            {
                TInstalledDriver aInstalledDriver;
                lcl_readURLPatternNode(m_aInstalled,*pPatternIter,aInstalledDriver);
                if ( !aInstalledDriver.sDriverFactory.isEmpty() )
                    m_aDrivers.emplace(*pPatternIter,aInstalledDriver);
            }
        } // if ( m_aInstalled.isValid() )
    }
    return m_aDrivers;
}

DriversConfig::DriversConfig(const uno::Reference< uno::XComponentContext >& _rxORB)
:m_xORB(_rxORB)
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


OUString DriversConfig::getDriverFactoryName(const OUString& _sURL) const
{
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

OUString DriversConfig::getDriverTypeDisplayName(const OUString& _sURL) const
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

const ::comphelper::NamedValueCollection& DriversConfig::getProperties(const OUString& _sURL) const
{
    return impl_get(_sURL,1);
}

const ::comphelper::NamedValueCollection& DriversConfig::getFeatures(const OUString& _sURL) const
{
    return impl_get(_sURL,0);
}

const ::comphelper::NamedValueCollection& DriversConfig::getMetaData(const OUString& _sURL) const
{
    return impl_get(_sURL,2);
}

const ::comphelper::NamedValueCollection& DriversConfig::impl_get(const OUString& _sURL,sal_Int32 _nProps) const
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
