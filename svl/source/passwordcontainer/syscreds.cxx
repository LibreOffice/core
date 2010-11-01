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

#include "syscreds.hxx"
#include "com/sun/star/beans/PropertyValue.hpp"

using namespace com::sun::star;

SysCredentialsConfigItem::SysCredentialsConfigItem(
    SysCredentialsConfig * pOwner )
: utl::ConfigItem( rtl::OUString::createFromAscii( "Office.Common/Passwords" ),
                   CONFIG_MODE_IMMEDIATE_UPDATE ),
  m_bInited( false ),
  m_pOwner( pOwner )
{
    uno::Sequence< ::rtl::OUString > aNode( 1 );
    aNode[ 0 ] = rtl::OUString::createFromAscii(
        "Office.Common/Passwords/AuthenticateUsingSystemCredentials" );
    EnableNotification( aNode );
}

//virtual
void SysCredentialsConfigItem::Notify(
    const uno::Sequence< rtl::OUString > & /*seqPropertyNames*/ )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bInited = false;
        // rebuild m_seqURLs
        getSystemCredentialsURLs();
    }
    m_pOwner->persistentConfigChanged();
}

void SysCredentialsConfigItem::Commit()
{
    // does nothing
}

uno::Sequence< rtl::OUString >
SysCredentialsConfigItem::getSystemCredentialsURLs()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bInited )
    {
        // read config item
        uno::Sequence< ::rtl::OUString > aPropNames( 1 );
        aPropNames[ 0 ] = rtl::OUString::createFromAscii(
            "AuthenticateUsingSystemCredentials" );
        uno::Sequence< uno::Any > aAnyValues(
            utl::ConfigItem::GetProperties( aPropNames ) );

        OSL_ENSURE(
            aAnyValues.getLength() == 1,
            "SysCredentialsConfigItem::getSystemCredentialsURLs: "
            "Error reading config item!" );

        uno::Sequence< rtl::OUString > aValues;
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
    const uno::Sequence< rtl::OUString > & seqURLList )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // write config item.
    uno::Sequence< rtl::OUString > aPropNames( 1 );
    uno::Sequence< uno::Any > aPropValues( 1 );
    aPropNames[ 0 ]
        = ::rtl::OUString::createFromAscii(
            "AuthenticateUsingSystemCredentials" );
    aPropValues[ 0 ] <<= seqURLList;

    utl::ConfigItem::SetModified();
    utl::ConfigItem::PutProperties( aPropNames, aPropValues );

    m_seqURLs = seqURLList;
    m_bInited = true;
}

//============================================================================

namespace
{
    // TODO: This code is actually copied from svl/source/passwordcontainer.cxx
    bool removeLastSegment( ::rtl::OUString & aURL )
    {
        sal_Int32 aInd = aURL.lastIndexOf( sal_Unicode( '/' ) );

        if( aInd > 0  )
        {
            sal_Int32 aPrevInd = aURL.lastIndexOf( sal_Unicode( '/' ), aInd );
            if ( aURL.indexOf( ::rtl::OUString::createFromAscii( "://" ) )
                    != aPrevInd - 2 ||
                 aInd != aURL.getLength() - 1 )
            {
                aURL = aURL.copy( 0, aInd );
                return true;
            }
        }

        return false;
    }

    bool findURL( StringSet const & rContainer, rtl::OUString const & aURL, rtl::OUString & aResult )
    {
        // TODO: This code is actually copied from svl/source/passwordcontainer.cxx
        if( !rContainer.empty() && aURL.getLength() )
        {
            ::rtl::OUString aUrl( aURL );

            // each iteration remove last '/...' section from the aUrl
            // while it's possible, up to the most left '://'
            do
            {
                // first look for <url>/somename and then look for <url>/somename/...
                StringSet::const_iterator aIter = rContainer.find( aUrl );
                if( aIter != rContainer.end() )
                {
                    aResult = *aIter;
                    return true;
                }
                else
                {
                    ::rtl::OUString tmpUrl( aUrl );
                    if ( tmpUrl.getStr()[tmpUrl.getLength() - 1] != (sal_Unicode)'/' )
                      tmpUrl += ::rtl::OUString::createFromAscii( "/" );

                    aIter = rContainer.lower_bound( tmpUrl );
                    if( aIter != rContainer.end() && aIter->match( tmpUrl ) )
                    {
                        aResult = *aIter;
                        return true;
                    }
                }
            }
            while( removeLastSegment( aUrl ) && aUrl.getLength() );
        }
        aResult = rtl::OUString();
        return false;
    }

} // namespace

SysCredentialsConfig::SysCredentialsConfig()
: m_aConfigItem( this ),
  m_bCfgInited( false )
{
}

void SysCredentialsConfig::initCfg()
{
    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bCfgInited )
    {
        uno::Sequence< rtl::OUString > aURLs(
            m_aConfigItem.getSystemCredentialsURLs() );
        for ( sal_Int32 n = 0; n < aURLs.getLength(); ++n )
            m_aCfgContainer.insert( aURLs[ n ] );

        m_bCfgInited = true;
    }
}

void SysCredentialsConfig::writeCfg()
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( m_bCfgInited, "SysCredentialsConfig::writeCfg : not initialized!" );

    uno::Sequence< rtl::OUString > aURLs( m_aCfgContainer.size() );
    StringSet::const_iterator it = m_aCfgContainer.begin();
    const StringSet::const_iterator end = m_aCfgContainer.end();
    sal_Int32 n = 0;

    while ( it != end )
    {
        aURLs[ n ] = *it;
        ++it;
        ++n;
    }

    m_aConfigItem.setSystemCredentialsURLs( aURLs );
}

rtl::OUString SysCredentialsConfig::find( rtl::OUString const & aURL )
{
    osl::MutexGuard aGuard( m_aMutex );
    rtl::OUString aResult;
    if ( findURL( m_aMemContainer, aURL, aResult ) )
        return aResult;

    initCfg();
    if ( findURL( m_aCfgContainer, aURL, aResult ) )
        return aResult;

    return rtl::OUString();
}

void SysCredentialsConfig::add( rtl::OUString const & rURL, bool bPersistent )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( bPersistent )
    {
        m_aMemContainer.erase( rURL );

        initCfg();
        m_aCfgContainer.insert( rURL );
        writeCfg();
    }
    else
    {
        initCfg();
        if ( m_aCfgContainer.erase( rURL ) > 0 )
            writeCfg();

        m_aMemContainer.insert( rURL );
    }
}

void SysCredentialsConfig::remove( rtl::OUString const & rURL )
{
    m_aMemContainer.erase( rURL );

    initCfg();
    if ( m_aCfgContainer.erase( rURL ) > 0 )
        writeCfg();
}

uno::Sequence< rtl::OUString > SysCredentialsConfig::list( bool bOnlyPersistent )
{
    initCfg();
    sal_Int32 nCount = m_aCfgContainer.size()
                     + ( bOnlyPersistent ? 0 : m_aMemContainer.size() );
    uno::Sequence< rtl::OUString > aResult( nCount );

    StringSet::const_iterator it = m_aCfgContainer.begin();
    StringSet::const_iterator end = m_aCfgContainer.end();
    sal_Int32 n = 0;

    while ( it != end )
    {
        aResult[ n ] = *it;
        ++it;
        ++n;
    }

    if ( !bOnlyPersistent )
    {
        it = m_aMemContainer.begin();
        end = m_aMemContainer.end();

        while ( it != end )
        {
            aResult[ n ] = *it;
            ++it;
            ++n;
        }
    }
    return aResult;
}

void SysCredentialsConfig::persistentConfigChanged()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_bCfgInited = false; // re-init on demand.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
