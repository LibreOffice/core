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

#include "syscreds.hxx"
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

SysCredentialsConfigItem::SysCredentialsConfigItem(
    SysCredentialsConfig * pOwner )
: utl::ConfigItem( u"Office.Common/Passwords"_ustr, ConfigItemMode::NONE ),
  m_bInited( false ),
  m_pOwner( pOwner )
{
    uno::Sequence<OUString> aNode { u"Office.Common/Passwords/AuthenticateUsingSystemCredentials"_ustr };
    EnableNotification( aNode );
}

//virtual
void SysCredentialsConfigItem::Notify(
    const uno::Sequence< OUString > & /*seqPropertyNames*/ )
{
    {
        std::unique_lock aGuard( m_aMutex );
        m_bInited = false;
        // rebuild m_seqURLs
        getSystemCredentialsURLs(aGuard);
    }
    m_pOwner->persistentConfigChanged();
}

void SysCredentialsConfigItem::ImplCommit()
{
    // does nothing
}

uno::Sequence< OUString >
SysCredentialsConfigItem::getSystemCredentialsURLs()
{
    std::unique_lock aGuard(m_aMutex);
    return getSystemCredentialsURLs(aGuard);
}

uno::Sequence< OUString >
SysCredentialsConfigItem::getSystemCredentialsURLs(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if ( !m_bInited )
    {
        // read config item
        uno::Sequence<OUString> aPropNames { u"AuthenticateUsingSystemCredentials"_ustr };
        uno::Sequence< uno::Any > aAnyValues(
            utl::ConfigItem::GetProperties( aPropNames ) );

        OSL_ENSURE(
            aAnyValues.getLength() == 1,
            "SysCredentialsConfigItem::getSystemCredentialsURLs: "
            "Error reading config item!" );

        uno::Sequence< OUString > aValues;
        if ( ( aAnyValues[ 0 ] >>= aValues ) ||
             ( !aAnyValues[ 0 ].hasValue() ) )
        {
            m_seqURLs = aValues;
            m_bInited = true;
        }
    }
    return m_seqURLs;
}

void SysCredentialsConfigItem::setSystemCredentialsURLs(
    const uno::Sequence< OUString > & seqURLList )
{
    // write config item.
    uno::Sequence< OUString > aPropNames{ u"AuthenticateUsingSystemCredentials"_ustr };
    uno::Sequence< uno::Any > aPropValues{ uno::Any(seqURLList) };

    utl::ConfigItem::SetModified();
    utl::ConfigItem::PutProperties( aPropNames, aPropValues );

    std::unique_lock aGuard( m_aMutex );

    m_seqURLs = seqURLList;
    m_bInited = true;
}


namespace
{
    // TODO: This code is actually copied from svl/source/passwordcontainer.cxx
    bool removeLastSegment( OUString & aURL )
    {
        sal_Int32 aInd = aURL.lastIndexOf( '/' );

        if( aInd > 0  )
        {
            sal_Int32 aPrevInd = aURL.lastIndexOf( '/', aInd );
            if ( aURL.indexOf( "://" ) != aPrevInd - 2 ||
                 aInd != aURL.getLength() - 1 )
            {
                aURL = aURL.copy( 0, aInd );
                return true;
            }
        }

        return false;
    }

    bool findURL( std::set<OUString> const & rContainer, OUString const & aURL, OUString & aResult )
    {
        // TODO: This code is actually copied from svl/source/passwordcontainer.cxx
        if( !rContainer.empty() && !aURL.isEmpty() )
        {
            OUString aUrl( aURL );

            // each iteration remove last '/...' section from the aUrl
            // while it's possible, up to the most left '://'
            do
            {
                // first look for <url>/somename and then look for <url>/somename/...
                auto aIter = rContainer.find( aUrl );
                if( aIter != rContainer.end() )
                {
                    aResult = *aIter;
                    return true;
                }
                else
                {
                    OUString tmpUrl( aUrl );
                    if ( !tmpUrl.endsWith("/") )
                      tmpUrl += "/";

                    aIter = rContainer.lower_bound( tmpUrl );
                    if( aIter != rContainer.end() && aIter->match( tmpUrl ) )
                    {
                        aResult = *aIter;
                        return true;
                    }
                }
            }
            while( removeLastSegment( aUrl ) && !aUrl.isEmpty() );
        }
        aResult.clear();
        return false;
    }

} // namespace

SysCredentialsConfig::SysCredentialsConfig()
: m_aConfigItem( this ),
  m_bCfgInited( false )
{
}

void SysCredentialsConfig::initCfg(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if ( !m_bCfgInited )
    {
        const uno::Sequence< OUString > aURLs(
            m_aConfigItem.getSystemCredentialsURLs() );
        m_aCfgContainer.insert( aURLs.begin(), aURLs.end() );
        m_bCfgInited = true;
    }
}

void SysCredentialsConfig::writeCfg(std::unique_lock<std::mutex>& /*rGuard*/)
{
    OSL_ENSURE( m_bCfgInited, "SysCredentialsConfig::writeCfg : not initialized!" );

    m_aConfigItem.setSystemCredentialsURLs( comphelper::containerToSequence(m_aCfgContainer) );
}

OUString SysCredentialsConfig::find( OUString const & aURL )
{
    std::unique_lock aGuard( m_aMutex );
    OUString aResult;
    if ( findURL( m_aMemContainer, aURL, aResult ) )
        return aResult;

    initCfg(aGuard);
    if ( findURL( m_aCfgContainer, aURL, aResult ) )
        return aResult;

    return OUString();
}

void SysCredentialsConfig::add( OUString const & rURL, bool bPersistent )
{
    std::unique_lock aGuard( m_aMutex );

    if ( bPersistent )
    {
        m_aMemContainer.erase( rURL );

        initCfg(aGuard);
        m_aCfgContainer.insert( rURL );
        writeCfg(aGuard);
    }
    else
    {
        initCfg(aGuard);
        if ( m_aCfgContainer.erase( rURL ) > 0 )
            writeCfg(aGuard);

        m_aMemContainer.insert( rURL );
    }
}

void SysCredentialsConfig::remove( OUString const & rURL )
{
    std::unique_lock aGuard(m_aMutex);

    m_aMemContainer.erase( rURL );

    initCfg(aGuard);
    if ( m_aCfgContainer.erase( rURL ) > 0 )
        writeCfg(aGuard);
}

uno::Sequence< OUString > SysCredentialsConfig::list( bool bOnlyPersistent )
{
    std::unique_lock aGuard(m_aMutex);
    initCfg(aGuard);
    sal_Int32 nCount = m_aCfgContainer.size()
                     + ( bOnlyPersistent ? 0 : m_aMemContainer.size() );
    uno::Sequence< OUString > aResult( nCount );
    auto aResultRange = asNonConstRange(aResult);
    sal_Int32 n = 0;

    for ( const auto& rItem : m_aCfgContainer )
    {
        aResultRange[ n ] = rItem;
        ++n;
    }

    if ( !bOnlyPersistent )
    {
        for ( const auto& rItem : m_aMemContainer )
        {
            aResultRange[ n ] = rItem;
            ++n;
        }
    }
    return aResult;
}

void SysCredentialsConfig::persistentConfigChanged()
{
    std::unique_lock aGuard( m_aMutex );
    m_bCfgInited = false; // re-init on demand.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
