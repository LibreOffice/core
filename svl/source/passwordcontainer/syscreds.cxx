/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "syscreds.hxx"
#include "com/sun/star/beans/PropertyValue.hpp"

using namespace com::sun::star;

SysCredentialsConfigItem::SysCredentialsConfigItem(
    SysCredentialsConfig * pOwner )
: utl::ConfigItem( OUString("Office.Common/Passwords"),
                   CONFIG_MODE_IMMEDIATE_UPDATE ),
  m_bInited( false ),
  m_pOwner( pOwner )
{
    uno::Sequence< OUString > aNode( 1 );
    aNode[ 0 ] = "Office.Common/Passwords/AuthenticateUsingSystemCredentials";
    EnableNotification( aNode );
}


void SysCredentialsConfigItem::Notify(
    const uno::Sequence< OUString > & /*seqPropertyNames*/ )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bInited = false;
        
        getSystemCredentialsURLs();
    }
    m_pOwner->persistentConfigChanged();
}

void SysCredentialsConfigItem::Commit()
{
    
}

uno::Sequence< OUString >
SysCredentialsConfigItem::getSystemCredentialsURLs()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bInited )
    {
        
        uno::Sequence< OUString > aPropNames( 1 );
        aPropNames[ 0 ] = "AuthenticateUsingSystemCredentials";
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
    ::osl::MutexGuard aGuard( m_aMutex );

    
    uno::Sequence< OUString > aPropNames( 1 );
    uno::Sequence< uno::Any > aPropValues( 1 );
    aPropNames[ 0 ] = "AuthenticateUsingSystemCredentials";
    aPropValues[ 0 ] <<= seqURLList;

    utl::ConfigItem::SetModified();
    utl::ConfigItem::PutProperties( aPropNames, aPropValues );

    m_seqURLs = seqURLList;
    m_bInited = true;
}


namespace
{
    
    bool removeLastSegment( OUString & aURL )
    {
        sal_Int32 aInd = aURL.lastIndexOf( '/' );

        if( aInd > 0  )
        {
            sal_Int32 aPrevInd = aURL.lastIndexOf( '/', aInd );
            if ( aURL.indexOf( ":
                 aInd != aURL.getLength() - 1 )
            {
                aURL = aURL.copy( 0, aInd );
                return true;
            }
        }

        return false;
    }

    bool findURL( StringSet const & rContainer, OUString const & aURL, OUString & aResult )
    {
        
        if( !rContainer.empty() && !aURL.isEmpty() )
        {
            OUString aUrl( aURL );

            
            
            do
            {
                
                StringSet::const_iterator aIter = rContainer.find( aUrl );
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
        aResult = OUString();
        return false;
    }

} 

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
        uno::Sequence< OUString > aURLs(
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

    uno::Sequence< OUString > aURLs( m_aCfgContainer.size() );
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

OUString SysCredentialsConfig::find( OUString const & aURL )
{
    osl::MutexGuard aGuard( m_aMutex );
    OUString aResult;
    if ( findURL( m_aMemContainer, aURL, aResult ) )
        return aResult;

    initCfg();
    if ( findURL( m_aCfgContainer, aURL, aResult ) )
        return aResult;

    return OUString();
}

void SysCredentialsConfig::add( OUString const & rURL, bool bPersistent )
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

void SysCredentialsConfig::remove( OUString const & rURL )
{
    m_aMemContainer.erase( rURL );

    initCfg();
    if ( m_aCfgContainer.erase( rURL ) > 0 )
        writeCfg();
}

uno::Sequence< OUString > SysCredentialsConfig::list( bool bOnlyPersistent )
{
    initCfg();
    sal_Int32 nCount = m_aCfgContainer.size()
                     + ( bOnlyPersistent ? 0 : m_aMemContainer.size() );
    uno::Sequence< OUString > aResult( nCount );

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
    m_bCfgInited = false; 
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
