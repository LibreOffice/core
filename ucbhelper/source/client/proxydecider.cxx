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

//=========================================================================
namespace ucbhelper
{

//=========================================================================
namespace proxydecider_impl
{

// A simple case ignoring wildcard matcher.
class WildCard
{
private:
    rtl::OString m_aWildString;

public:
    WildCard( const rtl::OUString& rWildCard )
    : m_aWildString(
        rtl::OUStringToOString(
            rWildCard, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() ) {}

    bool Matches( const rtl::OUString & rStr ) const;
};

//=========================================================================
typedef std::pair< WildCard, WildCard > NoProxyListEntry;

//=========================================================================

class HostnameCache
{
    typedef std::pair< rtl::OUString, rtl::OUString > HostListEntry;

    std::list< HostListEntry >     m_aHostList;
    sal_uInt32                     m_nCapacity;

public:
    explicit HostnameCache( sal_uInt32 nCapacity )
        : m_nCapacity( nCapacity ) {}

    bool get( const rtl::OUString & rKey, rtl::OUString & rValue ) const
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

    void put( const rtl::OUString & rKey, const rtl::OUString & rValue )
    {
        if ( m_aHostList.size() == m_nCapacity )
            m_aHostList.resize( m_nCapacity / 2 );

        m_aHostList.push_front( HostListEntry( rKey, rValue ) );
    }
};

//=========================================================================
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
    bool shouldUseProxy( const rtl::OUString & rHost,
                         sal_Int32 nPort,
                         bool bUseFullyQualified ) const;
public:
    InternetProxyDecider_Impl(
        const uno::Reference< uno::XComponentContext >& rxContext );
    virtual ~InternetProxyDecider_Impl();

    void dispose();

    const InternetProxyServer & getProxy( const rtl::OUString & rProtocol,
                                          const rtl::OUString & rHost,
                                          sal_Int32 nPort ) const;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& Event )
        throw( uno::RuntimeException );

    // XEventListener ( base of XChangesLisetenr )
    virtual void SAL_CALL disposing( const lang::EventObject& Source )
        throw( uno::RuntimeException );

private:
    void setNoProxyList( const rtl::OUString & rNoProxyList );
};

//=========================================================================
//=========================================================================
//
// WildCard Implementation.
//
//=========================================================================
//=========================================================================

bool WildCard::Matches( const rtl::OUString& rString ) const
{
    rtl::OString aString
        = rtl::OUStringToOString(
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
                    return 0;
                break;

            default:
                if ( ( *pWild == '\\' ) && ( ( *( pWild + 1 ) == '?' )
                                             || ( *( pWild + 1 ) == '*') ) )
                    pWild++;
                if ( *pWild != *pStr )
                    if ( !pos )
                        return 0;
                    else
                        pWild += pos;
                else
                    break;

                // Note: fall-thru's are intended!

            case '*':
                while ( *pWild == '*' )
                    pWild++;
                if ( *pWild == '\0' )
                    return 1;
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

//=========================================================================
bool getConfigStringValue(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const char * key,
    rtl::OUString & value )
{
    try
    {
        if ( !( xNameAccess->getByName( rtl::OUString::createFromAscii( key ) )
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

//=========================================================================
bool getConfigInt32Value(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const char * key,
    sal_Int32 & value )
{
    try
    {
        uno::Any aValue = xNameAccess->getByName(
            rtl::OUString::createFromAscii( key ) );
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

//=========================================================================
//=========================================================================
//
// InternetProxyDecider_Impl Implementation.
//
//=========================================================================
//=========================================================================

InternetProxyDecider_Impl::InternetProxyDecider_Impl(
    const uno::Reference< uno::XComponentContext >& rxContext )
    : m_nProxyType( 0 ),
      m_aHostnames( 256 ) // cache size
{
    try
    {
        //////////////////////////////////////////////////////////////
        // Read proxy configuration from config db.
        //////////////////////////////////////////////////////////////

        uno::Reference< lang::XMultiServiceFactory > xConfigProv =
                configuration::theDefaultProvider::get( rxContext );

        uno::Sequence< uno::Any > aArguments( 1 );
        aArguments[ 0 ] <<= rtl::OUString( CONFIG_ROOT_KEY );

        uno::Reference< uno::XInterface > xInterface(
                    xConfigProv->createInstanceWithArguments(
                        rtl::OUString(
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
                // *** Proxy type ***
                getConfigInt32Value(
                    xNameAccess, PROXY_TYPE_KEY, m_nProxyType );

                // *** No proxy list ***
                rtl::OUString aNoProxyList;
                getConfigStringValue(
                    xNameAccess, NO_PROXY_LIST_KEY, aNoProxyList );
                setNoProxyList( aNoProxyList );

                // *** HTTP ***
                getConfigStringValue(
                    xNameAccess, HTTP_PROXY_NAME_KEY, m_aHttpProxy.aName );

                m_aHttpProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, HTTP_PROXY_PORT_KEY, m_aHttpProxy.nPort );
                if ( m_aHttpProxy.nPort == -1 )
                    m_aHttpProxy.nPort = 80; // standard HTTP port.

                // *** HTTPS ***
                getConfigStringValue(
                    xNameAccess, HTTPS_PROXY_NAME_KEY, m_aHttpsProxy.aName );

                m_aHttpsProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, HTTPS_PROXY_PORT_KEY, m_aHttpsProxy.nPort );
                if ( m_aHttpsProxy.nPort == -1 )
                    m_aHttpsProxy.nPort = 443; // standard HTTPS port.

                // *** FTP ***
                getConfigStringValue(
                    xNameAccess, FTP_PROXY_NAME_KEY, m_aFtpProxy.aName );

                m_aFtpProxy.nPort = -1;
                getConfigInt32Value(
                    xNameAccess, FTP_PROXY_PORT_KEY, m_aFtpProxy.nPort );
            }

            // Register as listener for config changes.

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
        // createInstance, createInstanceWithArguments
        OSL_FAIL( "InternetProxyDecider - Exception!" );
    }
}

//=========================================================================
// virtual
InternetProxyDecider_Impl::~InternetProxyDecider_Impl()
{
}

//=========================================================================
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

    // Do this unguarded!
    if ( xNotifier.is() )
        xNotifier->removeChangesListener( this );
}

//=========================================================================
bool InternetProxyDecider_Impl::shouldUseProxy( const rtl::OUString & rHost,
                                                sal_Int32 nPort,
                                                bool bUseFullyQualified ) const
{
    rtl::OUStringBuffer aBuffer;

    if ( ( rHost.indexOf( ':' ) != -1 ) &&
         ( rHost[ 0 ] != sal_Unicode( '[' ) ) )
    {
        // host is given as numeric IPv6 address
        aBuffer.appendAscii( "[" );
        aBuffer.append( rHost );
        aBuffer.appendAscii( "]" );
    }
    else
    {
        // host is given either as numeric IPv4 address or non-numeric hostname
        aBuffer.append( rHost );
    }

    aBuffer.append( sal_Unicode( ':' ) );
    aBuffer.append( rtl::OUString::valueOf( nPort ) );
    const rtl::OUString aHostAndPort( aBuffer.makeStringAndClear() );

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

//=========================================================================
const InternetProxyServer & InternetProxyDecider_Impl::getProxy(
                                            const rtl::OUString & rProtocol,
                                            const rtl::OUString & rHost,
                                            sal_Int32 nPort ) const
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_nProxyType == 0 )
    {
        // Never use proxy.
        return m_aEmptyProxy;
    }

    if ( !rHost.isEmpty() && !m_aNoProxyList.empty() )
    {
        //////////////////////////////////////////////////////////////////
        // First, try direct hostname match - #110515#
        //////////////////////////////////////////////////////////////////

        if ( !shouldUseProxy( rHost, nPort, false ) )
            return m_aEmptyProxy;

        //////////////////////////////////////////////////////////////////
        // Second, try match against full qualified hostname - #104401#
        //////////////////////////////////////////////////////////////////

        rtl::OUString aHost;

        if ( ( rHost[ 0 ] == sal_Unicode( '[' ) ) &&
             ( rHost.getLength() > 1 ) )
        {
            // host is given as numeric IPv6 address. name resolution
            // functions need hostname without square brackets.
            aHost = rHost.copy( 1, rHost.getLength() - 2 );
        }
        else
        {
            aHost = rHost;
        }

        rtl::OUString aFullyQualifiedHost;
        if ( !m_aHostnames.get( aHost, aFullyQualifiedHost ) )
        {
            // This might be quite expensive (DNS lookup).
            const osl::SocketAddr aAddr( aHost, nPort );
            aFullyQualifiedHost = aAddr.getHostname().toAsciiLowerCase();
            m_aHostnames.put( aHost, aFullyQualifiedHost );
        }

        // Error resolving name? -> fallback.
        if ( aFullyQualifiedHost.isEmpty() )
            aFullyQualifiedHost = aHost;

        if ( aFullyQualifiedHost != aHost )
        {
            if ( !shouldUseProxy( aFullyQualifiedHost, nPort, false ) )
                return m_aEmptyProxy;
        }

        //////////////////////////////////////////////////////////////////
        // Third, try match of fully qualified entries in no-proxy list
        // against full qualified hostname
        //
        // Example:
        // list: staroffice-doc -> full: xyz.germany.sun.com
        // in:   staroffice-doc.germany.sun.com -> full: xyz.germany.sun.com
        //
        //////////////////////////////////////////////////////////////////

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
        // All other protocols use the HTTP proxy.
        return m_aHttpProxy;
    }
    return m_aEmptyProxy;
}

//=========================================================================
// virtual
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
            rtl::OUString aKey;
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
                    rtl::OUString aNoProxyList;
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
                        m_aHttpProxy.nPort = 80; // standard HTTP port.
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
                        m_aHttpsProxy.nPort = 443; // standard HTTPS port.
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

//=========================================================================
// virtual
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

//=========================================================================
void InternetProxyDecider_Impl::setNoProxyList(
                                        const rtl::OUString & rNoProxyList )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_aNoProxyList.clear();

    if ( !rNoProxyList.isEmpty() )
    {
        // List of connection endpoints hostname[:port],
        // separated by semicolon. Wilcards allowed.

        sal_Int32 nPos = 0;
        sal_Int32 nEnd = rNoProxyList.indexOf( ';' );
        sal_Int32 nLen = rNoProxyList.getLength();

        do
        {
            if ( nEnd == -1 )
                nEnd = nLen;

            rtl::OUString aToken = rNoProxyList.copy( nPos, nEnd - nPos );

            if ( !aToken.isEmpty() )
            {
                rtl::OUString aServer;
                rtl::OUString aPort;

                // numerical IPv6 address?
                bool bIPv6Address = false;
                sal_Int32 nClosedBracketPos = aToken.indexOf( ']' );
                if ( nClosedBracketPos == -1 )
                    nClosedBracketPos = 0;
                else
                    bIPv6Address = true;

                sal_Int32 nColonPos = aToken.indexOf( ':', nClosedBracketPos );
                if ( nColonPos == -1 )
                {
                    // No port given, server pattern equals current token
                    aPort = rtl::OUString("*");
                    if ( aToken.indexOf( '*' ) == -1 )
                    {
                        // pattern describes exactly one server
                        aServer = aToken;
                    }

                    aToken += rtl::OUString(":*");
                }
                else
                {
                    // Port given, extract server pattern
                    sal_Int32 nAsterixPos = aToken.indexOf( '*' );
                    aPort = aToken.copy( nColonPos + 1 );
                    if ( nAsterixPos < nColonPos )
                    {
                        // pattern describes exactly one server
                        aServer = aToken.copy( 0, nColonPos );
                    }
                }

                rtl::OUStringBuffer aFullyQualifiedHost;
                if ( !aServer.isEmpty() )
                {
                    // Remember fully qualified server name if current list
                    // entry specifies exactly one non-fully qualified server
                    // name.

                    // remove square brackets from host name in case it's
                    // a numerical IPv6 address.
                    if ( bIPv6Address )
                        aServer = aServer.copy( 1, aServer.getLength() - 2 );

                    // This might be quite expensive (DNS lookup).
                    const osl::SocketAddr aAddr( aServer, 0 );
                    rtl::OUString aTmp = aAddr.getHostname().toAsciiLowerCase();
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

} // namespace proxydecider_impl

//=========================================================================
//=========================================================================
//
// InternetProxyDecider Implementation.
//
//=========================================================================
//=========================================================================

InternetProxyDecider::InternetProxyDecider(
    const uno::Reference< uno::XComponentContext>& rxContext )
: m_pImpl( new proxydecider_impl::InternetProxyDecider_Impl( rxContext ) )
{
    m_pImpl->acquire();
}

//=========================================================================
InternetProxyDecider::~InternetProxyDecider()
{
    // Break circular reference between config listener and notifier.
    m_pImpl->dispose();

    // Let him go...
    m_pImpl->release();
}

//=========================================================================
bool InternetProxyDecider::shouldUseProxy( const rtl::OUString & rProtocol,
                                           const rtl::OUString & rHost,
                                           sal_Int32 nPort ) const
{
    const InternetProxyServer & rData = m_pImpl->getProxy( rProtocol,
                                                           rHost,
                                                           nPort );
    return !rData.aName.isEmpty();
}

//=========================================================================
const InternetProxyServer & InternetProxyDecider::getProxy(
                                            const rtl::OUString & rProtocol,
                                            const rtl::OUString & rHost,
                                            sal_Int32 nPort ) const
{
    return m_pImpl->getProxy( rProtocol, rHost, nPort );
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
