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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <utility>
#include <vector>
#include <list>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <osl/socket.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cppuhelper/implbase1.hxx>
#include "ucbhelper/proxydecider.hxx"

using namespace com::sun::star;
using namespace ucbhelper;

#define CONFIG_ROOT_KEY      "org.openoffice.Inet/Settings"
#define PROXY_TYPE_KEY       "ooInetProxyType"
#define NO_PROXY_LIST_KEY    "ooInetNoProxy"
#define HTTP_PROXY_NAME_KEY  "ooInetHTTPProxyName"
#define HTTP_PROXY_PORT_KEY  "ooInetHTTPProxyPort"
#define HTTPS_PROXY_NAME_KEY "ooInetHTTPSProxyName"
#define HTTPS_PROXY_PORT_KEY "ooInetHTTPSProxyPort"
#define FTP_PROXY_NAME_KEY   "ooInetFTPProxyName"
#define FTP_PROXY_PORT_KEY   "ooInetFTPProxyPort"


namespace ucbhelper
{


namespace proxydecider_impl
{


class WildCard
{
private:
    OString m_aWildString;

public:
    WildCard( const OUString& rWildCard )
    : m_aWildString(
        OUStringToOString(
            rWildCard, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() ) {}

    bool Matches( const OUString & rStr ) const;
};


typedef std::pair< WildCard, WildCard > NoProxyListEntry;



class HostnameCache
{
    typedef std::pair< OUString, OUString > HostListEntry;

    std::list< HostListEntry >     m_aHostList;
    sal_uInt32                     m_nCapacity;

public:
    explicit HostnameCache( sal_uInt32 nCapacity )
        : m_nCapacity( nCapacity ) {}

    bool get( const OUString & rKey, OUString & rValue ) const
    {
        std::list< HostListEntry >::const_iterator it
            = m_aHostList.begin();
        const std::list< HostListEntry >::const_iterator end
            = m_aHostList.end();

        while ( it != end )
        {
            if ( (*it).first == rKey )
            {
                rValue = (*it).second;
                return true;
            }
            ++it;
        }
        return false;
    }

    void put( const OUString & rKey, const OUString & rValue )
    {
        if ( m_aHostList.size() == m_nCapacity )
            m_aHostList.resize( m_nCapacity / 2 );

        m_aHostList.push_front( HostListEntry( rKey, rValue ) );
    }
};


class InternetProxyDecider_Impl :
    public cppu::WeakImplHelper1< util::XChangesListener >
{
    mutable osl::Mutex                       m_aMutex;
    InternetProxyServer                      m_aHttpProxy;
    InternetProxyServer                      m_aHttpsProxy;
    InternetProxyServer                      m_aFtpProxy;
    const InternetProxyServer                m_aEmptyProxy;
    sal_Int32                                m_nProxyType;
    uno::Reference< util::XChangesNotifier > m_xNotifier;
    std::vector< NoProxyListEntry >          m_aNoProxyList;
    mutable HostnameCache                    m_aHostnames;

private:
    bool shouldUseProxy( const OUString & rHost,
                         sal_Int32 nPort,
                         bool bUseFullyQualified ) const;
public:
    InternetProxyDecider_Impl(
        const uno::Reference< uno::XComponentContext >& rxContext );
    virtual ~InternetProxyDecider_Impl();

    void dispose();

    const InternetProxyServer & getProxy( const OUString & rProtocol,
                                          const OUString & rHost,
                                          sal_Int32 nPort ) const;

    
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& Event )
        throw( uno::RuntimeException );

    
    virtual void SAL_CALL disposing( const lang::EventObject& Source )
        throw( uno::RuntimeException );

private:
    void setNoProxyList( const OUString & rNoProxyList );
};



//

//



bool WildCard::Matches( const OUString& rString ) const
{
    OString aString
        = OUStringToOString(
                    rString, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase();
    const char * pStr  = aString.getStr();
    const char * pWild = m_aWildString.getStr();

    int pos  = 0;
    int flag = 0;

    while ( *pWild || flag )
    {
        switch ( *pWild )
        {
            case '?':
                if ( *pStr == '\0' )
                    return false;
                break;

            default:
                if ( ( *pWild == '\\' ) && ( ( *( pWild + 1 ) == '?' )
                                             || ( *( pWild + 1 ) == '*') ) )
                    pWild++;
                if ( *pWild != *pStr )
                    if ( !pos )
                        return false;
                    else
                        pWild += pos;
                else
                    break;

                

            case '*':
                while ( *pWild == '*' )
                    pWild++;
                if ( *pWild == '\0' )
                    return true;
                flag = 1;
                pos  = 0;
                if ( *pStr == '\0' )
                    return ( *pWild == '\0' );
                while ( *pStr && *pStr != *pWild )
                {
                    if ( *pWild == '?' ) {
                        pWild++;
                        while ( *pWild == '*' )
                            pWild++;
                    }
                    pStr++;
                    if ( *pStr == '\0' )
                        return ( *pWild == '\0' );
                }
                break;
        }
        if ( *pWild != '\0' )
            pWild++;
        if ( *pStr != '\0' )
            pStr++;
        else
            flag = 0;
        if ( flag )
            pos--;
    }
    return ( *pStr == '\0' ) && ( *pWild == '\0' );
}


bool getConfigStringValue(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const char * key,
    OUString & value )
{
    try
    {
        if ( !( xNameAccess->getByName( OUString::createFromAscii( key ) )
                >>= value ) )
        {
            OSL_FAIL( "InternetProxyDecider - "
                        "Error getting config item value!" );
            return false;
        }
    }
    catch ( lang::WrappedTargetException const & )
    {
        return false;
    }
    catch ( container::NoSuchElementException const & )
    {
        return false;
    }
    return true;
}


bool getConfigInt32Value(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const char * key,
    sal_Int32 & value )
{
    try
    {
        uno::Any aValue = xNameAccess->getByName(
            OUString::createFromAscii( key ) );
        if ( aValue.hasValue() && !( aValue >>= value ) )
        {
            OSL_FAIL( "InternetProxyDecider - "
                        "Error getting config item value!" );
            return false;
        }
    }
    catch ( lang::WrappedTargetException const & )
    {
        return false;
    }
    catch ( container::NoSuchElementException const & )
    {
        return false;
    }
    return true;
}



//

//



InternetProxyDecider_Impl::InternetProxyDecider_Impl(
    const uno::Reference< uno::XComponentContext >& rxContext )
    : m_nProxyType( 0 ),
      m_aHostnames( 256 ) 
{
    try
    {
        
        
        

        uno::Reference< lang::XMultiServiceFactory > xConfigProv =
                configuration::theDefaultProvider::get( rxContext );

        uno::Sequence< uno::Any > aArguments( 1 );
        aArguments[ 0 ] <<= OUString( CONFIG_ROOT_KEY );

        uno::Reference< uno::XInterface > xInterface(
                    xConfigProv->createInstanceWithArguments(
                        OUString(
                            "com.sun.star.configuration.ConfigurationAccess" ),
                    aArguments ) );

        OSL_ENSURE( xInterface.is(),
                    "InternetProxyDecider - No config access!" );

        if ( xInterface.is() )
        {
            uno::Reference< container::XNameAccess > xNameAccess(
                                            xInterface, uno::UNO_QUERY );
            OSL_ENSURE( xNameAccess.is(),
                        "InternetProxyDecider - No name access!" );

            if ( xNameAccess.is() )
            {
                
                getConfigInt32Value(
                    xNameAccess, PROXY_TYPE_KEY, m_nProxyType );

                
                OUString aNoProxyList;
                getConfigStringValue(
                    xNameAccess, NO_PROXY_LIST_KEY, aNoProxyList );
                setNoProxyList( aNoProxyList );

                
                getConfigStringValue(
                    xNameAccess, HTTP_PROXY_NAME_KEY, m_aHttpProxy.aName );

                m_aHttpProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, HTTP_PROXY_PORT_KEY, m_aHttpProxy.nPort );
                if ( m_aHttpProxy.nPort == -1 )
                    m_aHttpProxy.nPort = 80; 

                
                getConfigStringValue(
                    xNameAccess, HTTPS_PROXY_NAME_KEY, m_aHttpsProxy.aName );

                m_aHttpsProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, HTTPS_PROXY_PORT_KEY, m_aHttpsProxy.nPort );
                if ( m_aHttpsProxy.nPort == -1 )
                    m_aHttpsProxy.nPort = 443; 

                
                getConfigStringValue(
                    xNameAccess, FTP_PROXY_NAME_KEY, m_aFtpProxy.aName );

                m_aFtpProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, FTP_PROXY_PORT_KEY, m_aFtpProxy.nPort );
            }

            

            m_xNotifier = uno::Reference< util::XChangesNotifier >(
                                                xInterface, uno::UNO_QUERY );

            OSL_ENSURE( m_xNotifier.is(),
                        "InternetProxyDecider - No notifier!" );

            if ( m_xNotifier.is() )
                m_xNotifier->addChangesListener( this );
        }
    }
    catch ( uno::Exception const & )
    {
        
        OSL_FAIL( "InternetProxyDecider - Exception!" );
    }
}



InternetProxyDecider_Impl::~InternetProxyDecider_Impl()
{
}


void InternetProxyDecider_Impl::dispose()
{
    uno::Reference< util::XChangesNotifier > xNotifier;

    if ( m_xNotifier.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( m_xNotifier.is() )
        {
            xNotifier = m_xNotifier;
            m_xNotifier.clear();
        }
    }

    
    if ( xNotifier.is() )
        xNotifier->removeChangesListener( this );
}


bool InternetProxyDecider_Impl::shouldUseProxy( const OUString & rHost,
                                                sal_Int32 nPort,
                                                bool bUseFullyQualified ) const
{
    OUStringBuffer aBuffer;

    if ( ( rHost.indexOf( ':' ) != -1 ) &&
         ( rHost[ 0 ] != '[' ) )
    {
        
        aBuffer.appendAscii( "[" );
        aBuffer.append( rHost );
        aBuffer.appendAscii( "]" );
    }
    else
    {
        
        aBuffer.append( rHost );
    }

    aBuffer.append( ':' );
    aBuffer.append( OUString::number( nPort ) );
    const OUString aHostAndPort( aBuffer.makeStringAndClear() );

    std::vector< NoProxyListEntry >::const_iterator it
        = m_aNoProxyList.begin();
    const std::vector< NoProxyListEntry >::const_iterator end
        = m_aNoProxyList.end();

    while ( it != end )
    {
        if ( bUseFullyQualified )
        {
            if ( (*it).second.Matches( aHostAndPort ) )
                return false;
        }
        else
        {
            if ( (*it).first.Matches( aHostAndPort ) )
                return false;
        }
        ++it;
    }

    return true;
}


const InternetProxyServer & InternetProxyDecider_Impl::getProxy(
                                            const OUString & rProtocol,
                                            const OUString & rHost,
                                            sal_Int32 nPort ) const
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_nProxyType == 0 )
    {
        
        return m_aEmptyProxy;
    }

    if ( !rHost.isEmpty() && !m_aNoProxyList.empty() )
    {
        
        
        

        if ( !shouldUseProxy( rHost, nPort, false ) )
            return m_aEmptyProxy;

        
        
        

        OUString aHost;

        if ( ( rHost[ 0 ] == '[' ) &&
             ( rHost.getLength() > 1 ) )
        {
            
            
            aHost = rHost.copy( 1, rHost.getLength() - 2 );
        }
        else
        {
            aHost = rHost;
        }

        OUString aFullyQualifiedHost;
        if ( !m_aHostnames.get( aHost, aFullyQualifiedHost ) )
        {
            
            const osl::SocketAddr aAddr( aHost, nPort );
            aFullyQualifiedHost = aAddr.getHostname().toAsciiLowerCase();
            m_aHostnames.put( aHost, aFullyQualifiedHost );
        }

        
        if ( aFullyQualifiedHost.isEmpty() )
            aFullyQualifiedHost = aHost;

        if ( aFullyQualifiedHost != aHost )
        {
            if ( !shouldUseProxy( aFullyQualifiedHost, nPort, false ) )
                return m_aEmptyProxy;
        }

        
        
        
        //
        
        
        
        //
        

        if ( !shouldUseProxy( aFullyQualifiedHost, nPort, true ) )
            return m_aEmptyProxy;
    }

    if ( rProtocol.toAsciiLowerCase() == "ftp" )
    {
        if ( !m_aFtpProxy.aName.isEmpty() && m_aFtpProxy.nPort >= 0 )
            return m_aFtpProxy;
    }
    else if ( rProtocol.toAsciiLowerCase() == "https" )
    {
        if ( !m_aHttpsProxy.aName.isEmpty() )
            return m_aHttpsProxy;
    }
    else if ( !m_aHttpProxy.aName.isEmpty() )
    {
        
        return m_aHttpProxy;
    }
    return m_aEmptyProxy;
}



void SAL_CALL InternetProxyDecider_Impl::changesOccurred(
                                        const util::ChangesEvent& Event )
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    sal_Int32 nCount = Event.Changes.getLength();
    if ( nCount )
    {
        const util::ElementChange* pElementChanges
            = Event.Changes.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const util::ElementChange& rElem = pElementChanges[ n ];
            OUString aKey;
            if ( ( rElem.Accessor >>= aKey ) && !aKey.isEmpty() )
            {
                if ( aKey == PROXY_TYPE_KEY )
                {
                    if ( !( rElem.Element >>= m_nProxyType ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey == NO_PROXY_LIST_KEY )
                {
                    OUString aNoProxyList;
                    if ( !( rElem.Element >>= aNoProxyList ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }

                    setNoProxyList( aNoProxyList );
                }
                else if ( aKey == HTTP_PROXY_NAME_KEY )
                {
                    if ( !( rElem.Element >>= m_aHttpProxy.aName ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey == HTTP_PROXY_PORT_KEY )
                {
                    if ( !( rElem.Element >>= m_aHttpProxy.nPort ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }

                    if ( m_aHttpProxy.nPort == -1 )
                        m_aHttpProxy.nPort = 80; 
                }
                else if ( aKey == HTTPS_PROXY_NAME_KEY )
                {
                    if ( !( rElem.Element >>= m_aHttpsProxy.aName ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey == HTTPS_PROXY_PORT_KEY )
                {
                    if ( !( rElem.Element >>= m_aHttpsProxy.nPort ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }

                    if ( m_aHttpsProxy.nPort == -1 )
                        m_aHttpsProxy.nPort = 443; 
                }
                else if ( aKey == FTP_PROXY_NAME_KEY )
                {
                    if ( !( rElem.Element >>= m_aFtpProxy.aName ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey == FTP_PROXY_PORT_KEY )
                {
                    if ( !( rElem.Element >>= m_aFtpProxy.nPort ) )
                    {
                        OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
            }
        }
    }
}



void SAL_CALL InternetProxyDecider_Impl::disposing(const lang::EventObject&)
    throw( uno::RuntimeException )
{
    if ( m_xNotifier.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( m_xNotifier.is() )
            m_xNotifier.clear();
    }
}


void InternetProxyDecider_Impl::setNoProxyList(
                                        const OUString & rNoProxyList )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_aNoProxyList.clear();

    if ( !rNoProxyList.isEmpty() )
    {
        
        

        sal_Int32 nPos = 0;
        sal_Int32 nEnd = rNoProxyList.indexOf( ';' );
        sal_Int32 nLen = rNoProxyList.getLength();

        do
        {
            if ( nEnd == -1 )
                nEnd = nLen;

            OUString aToken = rNoProxyList.copy( nPos, nEnd - nPos );

            if ( !aToken.isEmpty() )
            {
                OUString aServer;
                OUString aPort;

                
                bool bIPv6Address = false;
                sal_Int32 nClosedBracketPos = aToken.indexOf( ']' );
                if ( nClosedBracketPos == -1 )
                    nClosedBracketPos = 0;
                else
                    bIPv6Address = true;

                sal_Int32 nColonPos = aToken.indexOf( ':', nClosedBracketPos );
                if ( nColonPos == -1 )
                {
                    
                    aPort = "*";
                    if ( aToken.indexOf( '*' ) == -1 )
                    {
                        
                        aServer = aToken;
                    }

                    aToken += ":*";
                }
                else
                {
                    
                    sal_Int32 nAsteriskPos = aToken.indexOf( '*' );
                    aPort = aToken.copy( nColonPos + 1 );
                    if ( nAsteriskPos < nColonPos )
                    {
                        
                        aServer = aToken.copy( 0, nColonPos );
                    }
                }

                OUStringBuffer aFullyQualifiedHost;
                if ( !aServer.isEmpty() )
                {
                    
                    
                    

                    
                    
                    if ( bIPv6Address )
                        aServer = aServer.copy( 1, aServer.getLength() - 2 );

                    
                    const osl::SocketAddr aAddr( aServer, 0 );
                    OUString aTmp = aAddr.getHostname().toAsciiLowerCase();
                    if ( aTmp != aServer.toAsciiLowerCase() )
                    {
                        if ( bIPv6Address )
                        {
                            aFullyQualifiedHost.appendAscii( "[" );
                            aFullyQualifiedHost.append( aTmp );
                            aFullyQualifiedHost.appendAscii( "]" );
                        }
                        else
                        {
                            aFullyQualifiedHost.append( aTmp );
                        }
                        aFullyQualifiedHost.appendAscii( ":" );
                        aFullyQualifiedHost.append( aPort );
                    }
                }

                m_aNoProxyList.push_back(
                    NoProxyListEntry( WildCard( aToken ),
                                      WildCard(
                                        aFullyQualifiedHost
                                            .makeStringAndClear() ) ) );
            }

            if ( nEnd != nLen )
            {
                nPos = nEnd + 1;
                nEnd = rNoProxyList.indexOf( ';', nPos );
            }
        }
        while ( nEnd != nLen );
    }
}

} 



//

//



InternetProxyDecider::InternetProxyDecider(
    const uno::Reference< uno::XComponentContext>& rxContext )
: m_pImpl( new proxydecider_impl::InternetProxyDecider_Impl( rxContext ) )
{
    m_pImpl->acquire();
}


InternetProxyDecider::~InternetProxyDecider()
{
    
    m_pImpl->dispose();

    
    m_pImpl->release();
}


bool InternetProxyDecider::shouldUseProxy( const OUString & rProtocol,
                                           const OUString & rHost,
                                           sal_Int32 nPort ) const
{
    const InternetProxyServer & rData = m_pImpl->getProxy( rProtocol,
                                                           rHost,
                                                           nPort );
    return !rData.aName.isEmpty();
}


const InternetProxyServer & InternetProxyDecider::getProxy(
                                            const OUString & rProtocol,
                                            const OUString & rHost,
                                            sal_Int32 nPort ) const
{
    return m_pImpl->getProxy( rProtocol, rHost, nPort );
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
