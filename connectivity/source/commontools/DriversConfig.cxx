/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "connectivity/DriversConfig.hxx"
#include <tools/wldcrd.hxx>

using namespace connectivity;
using namespace utl;
using namespace ::com::sun::star;

namespace
{
    void lcl_convert(const uno::Sequence< ::rtl::OUString >& _aSource,uno::Any& _rDest)
    {
        uno::Sequence<uno::Any> aRet(_aSource.getLength());
        uno::Any* pAny = aRet.getArray();
        const ::rtl::OUString* pIter = _aSource.getConstArray();
        const ::rtl::OUString* pEnd  = pIter + _aSource.getLength();
        for (;pIter != pEnd ; ++pIter,++pAny)
        {
            *pAny <<= *pIter;
        }
        _rDest <<= aRet;
    }
    void lcl_fillValues(const ::utl::OConfigurationNode& _aURLPatternNode,const ::rtl::OUString& _sNode,::comphelper::NamedValueCollection& _rValues)
    {
        const ::utl::OConfigurationNode aPropertiesNode = _aURLPatternNode.openNode(_sNode);
        if ( aPropertiesNode.isValid() )
        {
            uno::Sequence< ::rtl::OUString > aStringSeq;
            static const ::rtl::OUString s_sValue(RTL_CONSTASCII_USTRINGPARAM("/Value"));
            const uno::Sequence< ::rtl::OUString > aProperties = aPropertiesNode.getNodeNames();
            const ::rtl::OUString* pPropertiesIter = aProperties.getConstArray();
            const ::rtl::OUString* pPropertiesEnd  = pPropertiesIter + aProperties.getLength();
            for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter)
            {
                uno::Any aValue = aPropertiesNode.getNodeValue(*pPropertiesIter + s_sValue);
                if ( aValue >>= aStringSeq )
                {
                    lcl_convert(aStringSeq,aValue);
                }
                _rValues.put(*pPropertiesIter,aValue);
            } // for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter,++pNamedIter)
        } // if ( aPropertiesNode.isValid() )
    }
    void lcl_readURLPatternNode(const ::utl::OConfigurationTreeRoot& _aInstalled,const ::rtl::OUString& _sEntry,TInstalledDriver& _rInstalledDriver)
    {
        const ::utl::OConfigurationNode aURLPatternNode = _aInstalled.openNode(_sEntry);
        if ( aURLPatternNode.isValid() )
        {
            static const ::rtl::OUString s_sParentURLPattern(RTL_CONSTASCII_USTRINGPARAM("ParentURLPattern"));
            static const ::rtl::OUString s_sDriver(RTL_CONSTASCII_USTRINGPARAM("Driver"));
            static const ::rtl::OUString s_sDriverTypeDisplayName(RTL_CONSTASCII_USTRINGPARAM("DriverTypeDisplayName"));
            static const ::rtl::OUString s_sProperties(RTL_CONSTASCII_USTRINGPARAM("Properties"));
            static const ::rtl::OUString s_sFeatures(RTL_CONSTASCII_USTRINGPARAM("Features"));
            static const ::rtl::OUString s_sMetaData(RTL_CONSTASCII_USTRINGPARAM("MetaData"));
            ::rtl::OUString sParentURLPattern;
            aURLPatternNode.getNodeValue(s_sParentURLPattern) >>= sParentURLPattern;
            if ( sParentURLPattern.getLength() )
                lcl_readURLPatternNode(_aInstalled,sParentURLPattern,_rInstalledDriver);

            ::rtl::OUString sDriverFactory;
            aURLPatternNode.getNodeValue(s_sDriver) >>= sDriverFactory;
            if ( sDriverFactory.getLength() )
                _rInstalledDriver.sDriverFactory = sDriverFactory;

            ::rtl::OUString sDriverTypeDisplayName;
            aURLPatternNode.getNodeValue(s_sDriverTypeDisplayName) >>= sDriverTypeDisplayName;
            OSL_ENSURE(sDriverTypeDisplayName.getLength(),"No valid DriverTypeDisplayName property!");
            if ( sDriverTypeDisplayName.getLength() )
                _rInstalledDriver.sDriverTypeDisplayName = sDriverTypeDisplayName;

            lcl_fillValues(aURLPatternNode,s_sProperties,_rInstalledDriver.aProperties);
            lcl_fillValues(aURLPatternNode,s_sFeatures,_rInstalledDriver.aFeatures);
            lcl_fillValues(aURLPatternNode,s_sMetaData,_rInstalledDriver.aMetaData);
        }
    }
}
// -----------------------------------------------------------------------------
DriversConfigImpl::DriversConfigImpl()
{
}
// -----------------------------------------------------------------------------
void DriversConfigImpl::Load(const uno::Reference< lang::XMultiServiceFactory >& _rxORB) const
{
    if ( m_aDrivers.empty() )
    {
        if ( !m_aInstalled.isValid() )
        {
            static const ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.DataAccess.Drivers/Installed")); ///Installed
            m_aInstalled = ::utl::OConfigurationTreeRoot::createWithServiceFactory(_rxORB, s_sNodeName, -1, ::utl::OConfigurationTreeRoot::CM_READONLY);
        }

        if ( m_aInstalled.isValid() )
        {
            const uno::Sequence< ::rtl::OUString > aURLPatterns = m_aInstalled.getNodeNames();
            const ::rtl::OUString* pPatternIter = aURLPatterns.getConstArray();
            const ::rtl::OUString* pPatternEnd  = pPatternIter + aURLPatterns.getLength();
            for (;pPatternIter != pPatternEnd ; ++pPatternIter)
            {
                TInstalledDriver aInstalledDriver;
                lcl_readURLPatternNode(m_aInstalled,*pPatternIter,aInstalledDriver);
                if ( aInstalledDriver.sDriverFactory.getLength() )
                    m_aDrivers.insert(TInstalledDrivers::value_type(*pPatternIter,aInstalledDriver));
            }
        } // if ( m_aInstalled.isValid() )
    }
}
// -----------------------------------------------------------------------------
DriversConfig::DriversConfig(const uno::Reference< lang::XMultiServiceFactory >& _rxORB)
:m_xORB(_rxORB)
{
}

// -----------------------------------------------------------------------------
DriversConfig::~DriversConfig()
{
}

// -----------------------------------------------------------------------------
DriversConfig::DriversConfig( const DriversConfig& _rhs )
{
    *this = _rhs;
}

// -----------------------------------------------------------------------------
DriversConfig& DriversConfig::operator=( const DriversConfig& _rhs )
{
    if ( this != &_rhs )
    {
        m_aNode = _rhs.m_aNode;
    }
    return *this;
}

// -----------------------------------------------------------------------------
::rtl::OUString DriversConfig::getDriverFactoryName(const ::rtl::OUString& _sURL) const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    ::rtl::OUString sRet;
    ::rtl::OUString sOldPattern;
    TInstalledDrivers::const_iterator aIter = rDrivers.begin();
    TInstalledDrivers::const_iterator aEnd = rDrivers.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(aIter->first);
        if ( sOldPattern.getLength() < aIter->first.getLength() && aWildCard.Matches(_sURL) )
        {
            sRet = aIter->second.sDriverFactory;
            sOldPattern = aIter->first;
        }
    }

    return sRet;
}
// -----------------------------------------------------------------------------
::rtl::OUString DriversConfig::getDriverTypeDisplayName(const ::rtl::OUString& _sURL) const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    ::rtl::OUString sRet;
    ::rtl::OUString sOldPattern;
    TInstalledDrivers::const_iterator aIter = rDrivers.begin();
    TInstalledDrivers::const_iterator aEnd = rDrivers.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(aIter->first);
        if ( sOldPattern.getLength() < aIter->first.getLength() && aWildCard.Matches(_sURL) )
        {
            sRet = aIter->second.sDriverTypeDisplayName;
            sOldPattern = aIter->first;
        }
    }

    return sRet;
}
// -----------------------------------------------------------------------------
const ::comphelper::NamedValueCollection& DriversConfig::getProperties(const ::rtl::OUString& _sURL) const
{
    return impl_get(_sURL,1);
}
// -----------------------------------------------------------------------------
const ::comphelper::NamedValueCollection& DriversConfig::getFeatures(const ::rtl::OUString& _sURL) const
{
    return impl_get(_sURL,0);
}
// -----------------------------------------------------------------------------
const ::comphelper::NamedValueCollection& DriversConfig::getMetaData(const ::rtl::OUString& _sURL) const
{
    return impl_get(_sURL,2);
}
// -----------------------------------------------------------------------------
const ::comphelper::NamedValueCollection& DriversConfig::impl_get(const ::rtl::OUString& _sURL,sal_Int32 _nProps) const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    const ::comphelper::NamedValueCollection* pRet = NULL;
    ::rtl::OUString sOldPattern;
    TInstalledDrivers::const_iterator aIter = rDrivers.begin();
    TInstalledDrivers::const_iterator aEnd = rDrivers.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(aIter->first);
        if ( sOldPattern.getLength() < aIter->first.getLength() && aWildCard.Matches(_sURL) )
        {
            switch(_nProps)
            {
                case 0:
                    pRet = &aIter->second.aFeatures;
                    break;
                case 1:
                    pRet = &aIter->second.aProperties;
                    break;
                case 2:
                    pRet = &aIter->second.aMetaData;
                    break;
            }
            sOldPattern = aIter->first;
        }
    } // for(;aIter != aEnd;++aIter)
    if ( pRet == NULL )
    {
        static const ::comphelper::NamedValueCollection s_sEmpty;
        pRet = &s_sEmpty;
    }
    return *pRet;
}
// -----------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DriversConfig::getURLs() const
{
    const TInstalledDrivers& rDrivers = m_aNode->getInstalledDrivers(m_xORB);
    uno::Sequence< ::rtl::OUString > aRet(rDrivers.size());
    ::rtl::OUString* pIter = aRet.getArray();
    TInstalledDrivers::const_iterator aIter = rDrivers.begin();
    TInstalledDrivers::const_iterator aEnd = rDrivers.end();
    for(;aIter != aEnd;++aIter,++pIter)
    {
        *pIter = aIter->first;
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
