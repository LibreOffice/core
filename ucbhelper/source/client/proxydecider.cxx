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

#include <sal/config.h>

#include <string_view>
#include <utility>
#include <vector>
#include <deque>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <osl/socket.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <cppuhelper/implbase.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <o3tl/string_view.hxx>

#ifdef _WIN32
#include <o3tl/char16_t2wchar_t.hxx>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winhttp.h>
#include <process.h>
#endif

using namespace com::sun::star;
using namespace ucbhelper;

constexpr OUString CONFIG_ROOT_KEY = u"org.openoffice.Inet/Settings"_ustr;
constexpr OUString PROXY_TYPE_KEY = u"ooInetProxyType"_ustr;
constexpr OUString NO_PROXY_LIST_KEY = u"ooInetNoProxy"_ustr;
constexpr OUString HTTP_PROXY_NAME_KEY = u"ooInetHTTPProxyName"_ustr;
constexpr OUString HTTP_PROXY_PORT_KEY = u"ooInetHTTPProxyPort"_ustr;
constexpr OUString HTTPS_PROXY_NAME_KEY = u"ooInetHTTPSProxyName"_ustr;
constexpr OUString HTTPS_PROXY_PORT_KEY = u"ooInetHTTPSProxyPort"_ustr;


namespace ucbhelper
{


namespace proxydecider_impl
{

namespace {

// A simple case ignoring wildcard matcher.
class WildCard
{
private:
    OString m_aWildString;

public:
    explicit WildCard( std::u16string_view rWildCard )
    : m_aWildString(
        OUStringToOString(
            rWildCard, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() ) {}

    bool Matches( std::u16string_view rStr ) const;
};

}

namespace {

class HostnameCache
{
    typedef std::pair< OUString, OUString > HostListEntry;

    std::deque< HostListEntry >    m_aHostList;

public:
    bool get( std::u16string_view rKey, OUString & rValue ) const
    {
        for (auto const& host : m_aHostList)
        {
            if ( host.first == rKey )
            {
                rValue = host.second;
                return true;
            }
        }
        return false;
    }

    void put( const OUString & rKey, const OUString & rValue )
    {
        static constexpr sal_uInt32 nCapacity = 256;

        if ( m_aHostList.size() == nCapacity )
            m_aHostList.resize( nCapacity / 2 );

        m_aHostList.push_front( HostListEntry( rKey, rValue ) );
    }
};

}

class InternetProxyDecider_Impl :
    public cppu::WeakImplHelper< util::XChangesListener >
{
    // see officecfg/registry/schema/org/openoffice/Inet.xcs for the definition of these values
    enum class ProxyType { NoProxy, Automatic, Manual };
    mutable osl::Mutex                       m_aMutex;
    InternetProxyServer                      m_aHttpProxy;
    InternetProxyServer                      m_aHttpsProxy;
    const InternetProxyServer                m_aEmptyProxy;
    ProxyType                                m_nProxyType;
    uno::Reference< util::XChangesNotifier > m_xNotifier;
    typedef std::pair< WildCard, WildCard > NoProxyListEntry;
    std::vector< NoProxyListEntry >          m_aNoProxyList;
    mutable HostnameCache                    m_aHostnames;

private:
    bool shouldUseProxy( std::u16string_view rHost,
                         sal_Int32 nPort,
                         bool bUseFullyQualified ) const;
public:
    explicit InternetProxyDecider_Impl(
        const uno::Reference< uno::XComponentContext >& rxContext );

    void dispose();

    InternetProxyServer getProxy(const OUString& rProtocol,
                                          const OUString & rHost,
                                          sal_Int32 nPort ) const;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& Event ) override;

    // XEventListener ( base of XChangesLisetenr )
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

private:
    void setNoProxyList( std::u16string_view rNoProxyList );
};


// WildCard Implementation.


bool WildCard::Matches( std::u16string_view rString ) const
{
    OString aString
        = OUStringToOString( rString, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase();
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

                [[fallthrough]];

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


static bool getConfigStringValue(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const OUString& key,
    OUString & value )
{
    try
    {
        if ( !( xNameAccess->getByName( key ) >>= value ) )
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


static bool getConfigInt32Value(
    const uno::Reference< container::XNameAccess > & xNameAccess,
    const OUString& key,
    sal_Int32 & value )
{
    try
    {
        uno::Any aValue = xNameAccess->getByName( key );
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


// InternetProxyDecider_Impl Implementation.


InternetProxyDecider_Impl::InternetProxyDecider_Impl(
    const uno::Reference< uno::XComponentContext >& rxContext )
    : m_nProxyType( ProxyType::NoProxy ),
      m_aHostnames()
{
    try
    {

        // Read proxy configuration from config db.


        uno::Reference< lang::XMultiServiceFactory > xConfigProv =
                configuration::theDefaultProvider::get( rxContext );

        uno::Sequence< uno::Any > aArguments{ uno::Any(CONFIG_ROOT_KEY) };
        uno::Reference< uno::XInterface > xInterface(
                    xConfigProv->createInstanceWithArguments(
                        u"com.sun.star.configuration.ConfigurationAccess"_ustr,
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
                sal_Int32 tmp = 0;
                getConfigInt32Value(
                    xNameAccess, PROXY_TYPE_KEY, tmp );
                m_nProxyType = static_cast<ProxyType>(tmp);

                // *** No proxy list ***
                OUString aNoProxyList;
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
            }

            // Register as listener for config changes.

            m_xNotifier.set( xInterface, uno::UNO_QUERY );

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


bool InternetProxyDecider_Impl::shouldUseProxy( std::u16string_view rHost,
                                                sal_Int32 nPort,
                                                bool bUseFullyQualified ) const
{
    OUStringBuffer aBuffer;

    if ( ( rHost.find( ':' ) != std::u16string_view::npos ) &&
         ( rHost[ 0 ] != '[' ) )
    {
        // host is given as numeric IPv6 address
        aBuffer.append( OUString::Concat("[") + rHost + "]" );
    }
    else
    {
        // host is given either as numeric IPv4 address or non-numeric hostname
        aBuffer.append( rHost );
    }

    aBuffer.append( ":" + OUString::number( nPort ) );

    for (auto const& noProxy : m_aNoProxyList)
    {
        if ( bUseFullyQualified )
        {
            if ( noProxy.second.Matches( aBuffer ) )
                return false;
        }
        else
        {
            if ( noProxy.first.Matches( aBuffer ) )
                return false;
        }
    }

    return true;
}

namespace
{
#ifdef _WIN32
struct GetPACProxyData
{
    const OUString& m_rProtocol;
    const OUString& m_rHost;
    sal_Int32 m_nPort;
    bool m_bAutoDetect = false;
    OUString m_sAutoConfigUrl;
    InternetProxyServer m_ProxyServer;

    GetPACProxyData(const OUString& rProtocol, const OUString& rHost, sal_Int32 nPort)
        : m_rProtocol(rProtocol)
        , m_rHost(rHost)
        , m_nPort(nPort)
    {
    }
};

// Tries to get proxy configuration using WinHttpGetProxyForUrl, which supports Web Proxy Auto-Discovery
// (WPAD) protocol and manually configured address to get Proxy Auto-Configuration (PAC) file.
// The WinINet/WinHTTP functions cannot correctly run in a STA COM thread, so use a dedicated thread
unsigned __stdcall GetPACProxyThread(void* lpParameter)
{
    assert(lpParameter);
    GetPACProxyData* pData = static_cast<GetPACProxyData*>(lpParameter);

    OUString url(pData->m_rProtocol + "://" + pData->m_rHost + ":"
                 + OUString::number(pData->m_nPort));

    HINTERNET hInternet = WinHttpOpen(L"Mozilla 5.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
                                      WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    DWORD nError = GetLastError();
    if (!hInternet)
        return nError;

    WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions{};
    if (pData->m_bAutoDetect)
    {
        AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
        AutoProxyOptions.dwAutoDetectFlags
            = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    }
    if (!pData->m_sAutoConfigUrl.isEmpty())
    {
        AutoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
        AutoProxyOptions.lpszAutoConfigUrl = o3tl::toW(pData->m_sAutoConfigUrl.getStr());
    }
    // First, try without autologon. According to
    // https://github.com/Microsoft/Windows-classic-samples/blob/master/Samples/Win7Samples/web/winhttp/WinhttpProxySample/GetProxy.cpp
    // autologon prevents caching, and so causes repetitive network traffic.
    AutoProxyOptions.fAutoLogonIfChallenged = FALSE;
    WINHTTP_PROXY_INFO ProxyInfo{};
    bool bResult
        = WinHttpGetProxyForUrl(hInternet, o3tl::toW(url.getStr()), &AutoProxyOptions, &ProxyInfo);
    nError = GetLastError();
    if (!bResult && nError == ERROR_WINHTTP_LOGIN_FAILURE)
    {
        AutoProxyOptions.fAutoLogonIfChallenged = TRUE;
        bResult = WinHttpGetProxyForUrl(hInternet, o3tl::toW(url.getStr()),
                                        &AutoProxyOptions, &ProxyInfo);
        nError = GetLastError();
    }
    WinHttpCloseHandle(hInternet);
    if (bResult)
    {
        if (ProxyInfo.lpszProxyBypass)
            GlobalFree(ProxyInfo.lpszProxyBypass);
        if (ProxyInfo.lpszProxy)
        {
            OUString sProxyResult(o3tl::toU(ProxyInfo.lpszProxy));
            GlobalFree(ProxyInfo.lpszProxy);
            // Get the first of possibly multiple results
            sProxyResult = sProxyResult.getToken(0, ';');
            sal_Int32 nPortSepPos = sProxyResult.indexOf(':');
            if (nPortSepPos != -1)
            {
                pData->m_ProxyServer.nPort = o3tl::toInt32(sProxyResult.subView(nPortSepPos + 1));
                sProxyResult = sProxyResult.copy(0, nPortSepPos);
            }
            else
            {
                pData->m_ProxyServer.nPort = 0;
            }
            pData->m_ProxyServer.aName = sProxyResult;
        }
    }

    return nError;
}

InternetProxyServer GetPACProxy(const OUString& rProtocol, const OUString& rHost, sal_Int32 nPort)
{
    GetPACProxyData aData(rProtocol, rHost, nPort);

    // WinHTTP only supports http(s), so don't try for other protocols
    if (!(rProtocol.equalsIgnoreAsciiCase("http") || rProtocol.equalsIgnoreAsciiCase("https")))
        return aData.m_ProxyServer;

    // Only try to get configuration from PAC (with all the overhead, including new thread)
    // if configured to do so
    {
        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG aProxyConfig{};
        bool bResult = WinHttpGetIEProxyConfigForCurrentUser(&aProxyConfig);
        if (aProxyConfig.lpszProxy)
            GlobalFree(aProxyConfig.lpszProxy);
        if (aProxyConfig.lpszProxyBypass)
            GlobalFree(aProxyConfig.lpszProxyBypass);
        // Don't try WPAD if AutoDetection or AutoConfig script URL are not configured
        if (!bResult || !(aProxyConfig.fAutoDetect || aProxyConfig.lpszAutoConfigUrl))
            return aData.m_ProxyServer;
        aData.m_bAutoDetect = aProxyConfig.fAutoDetect;
        if (aProxyConfig.lpszAutoConfigUrl)
        {
            aData.m_sAutoConfigUrl = o3tl::toU(aProxyConfig.lpszAutoConfigUrl);
            GlobalFree(aProxyConfig.lpszAutoConfigUrl);
        }
    }

    HANDLE hThread = reinterpret_cast<HANDLE>(
        _beginthreadex(nullptr, 0, GetPACProxyThread, &aData, 0, nullptr));
    if (hThread)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    return aData.m_ProxyServer;
}

#else // .. _WIN32

// Read the settings from the OS which are stored in env vars
//
InternetProxyServer GetUnixSystemProxy(const OUString & rProtocol)
{
    // TODO this could be improved to read the "no_proxy" env variable
    InternetProxyServer aProxy;
    OUString protocolLower = rProtocol.toAsciiLowerCase() + "_proxy";
    OString protocolLowerStr = OUStringToOString( protocolLower, RTL_TEXTENCODING_ASCII_US );
    const char* pEnvProxy = getenv(protocolLowerStr.getStr());
    if (!pEnvProxy)
        return aProxy;
    // expecting something like "https://example.ct:80"
    OUString tmp = OUString::createFromAscii(pEnvProxy);
    if (tmp.getLength() < (rProtocol.getLength() + 3))
        return aProxy;
    sal_Int32 x = tmp.indexOf("://");
    sal_Int32 at = tmp.indexOf('@', x == -1 ? 0 : x + 3);
    x = tmp.indexOf(':', at == -1 ? x == -1 ? 0 : x + 3 : at + 1);
    if (x == -1)
        return aProxy;
    int nPort = o3tl::toInt32(tmp.subView(x + 1));
    if (nPort == 0)
        return aProxy;
    aProxy.aName = tmp.copy(0, x);
    aProxy.nPort = nPort;
    return aProxy;
}

#endif // else .. _WIN32
}

InternetProxyServer InternetProxyDecider_Impl::getProxy(
                                            const OUString & rProtocol,
                                            const OUString & rHost,
                                            sal_Int32 nPort ) const
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_nProxyType == ProxyType::NoProxy )
    {
        // Never use proxy.
        return m_aEmptyProxy;
    }

    // If get from system
    if (m_nProxyType == ProxyType::Automatic && !rHost.isEmpty())
    {
#ifdef _WIN32
        InternetProxyServer aProxy(GetPACProxy(rProtocol, rHost, nPort));
#else
        InternetProxyServer aProxy(GetUnixSystemProxy(rProtocol));
#endif // _WIN32
        if (!aProxy.aName.isEmpty())
            return aProxy;
    }

    if ( !rHost.isEmpty() && !m_aNoProxyList.empty() )
    {

        // First, try direct hostname match - #110515#


        if ( !shouldUseProxy( rHost, nPort, false ) )
            return m_aEmptyProxy;


        // Second, try match against full qualified hostname - #104401#


        OUString aHost;

        if ( ( rHost.getLength() > 1 ) &&
             ( rHost[ 0 ] == '[' ))
        {
            // host is given as numeric IPv6 address. name resolution
            // functions need hostname without square brackets.
            aHost = rHost.copy( 1, rHost.getLength() - 2 );
        }
        else
        {
            aHost = rHost;
        }

        OUString aFullyQualifiedHost;
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


        // Third, try match of fully qualified entries in no-proxy list
        // against full qualified hostname

        // Example:
        // list: staroffice-doc -> full: xyz.germany.sun.com
        // in:   staroffice-doc.germany.sun.com -> full: xyz.germany.sun.com


        if ( !shouldUseProxy( aFullyQualifiedHost, nPort, true ) )
            return m_aEmptyProxy;
    }

    if (rProtocol.toAsciiLowerCase() == "https")
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

// virtual
void SAL_CALL InternetProxyDecider_Impl::changesOccurred(
                                        const util::ChangesEvent& Event )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    for ( const util::ElementChange& rElem : Event.Changes )
    {
        OUString aKey;
        if ( ( rElem.Accessor >>= aKey ) && !aKey.isEmpty() )
        {
            if ( aKey == PROXY_TYPE_KEY )
            {
                sal_Int32 tmp;
                if ( !( rElem.Element >>= tmp ) )
                {
                    OSL_FAIL( "InternetProxyDecider - changesOccurred - "
                                "Error getting config item value!" );
                }
                else
                    m_nProxyType = static_cast<ProxyType>(tmp);
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
        }
    }
}


// virtual
void SAL_CALL InternetProxyDecider_Impl::disposing(const lang::EventObject&)
{
    if ( m_xNotifier.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( m_xNotifier.is() )
            m_xNotifier.clear();
    }
}


void InternetProxyDecider_Impl::setNoProxyList(
                                        std::u16string_view rNoProxyList )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_aNoProxyList.clear();

    if ( rNoProxyList.empty() )
        return;

    // List of connection endpoints hostname[:port],
    // separated by semicolon. Wildcards allowed.

    size_t nPos = 0;
    size_t nEnd = rNoProxyList.find( ';' );
    size_t nLen = rNoProxyList.size();

    do
    {
        if ( nEnd == std::u16string_view::npos )
            nEnd = nLen;

        OUString aToken( rNoProxyList.substr( nPos, nEnd - nPos ) );

        if ( !aToken.isEmpty() )
        {
            OUString aServer;
            OUString aPort;

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
                aPort = "*";
                if ( aToken.indexOf( '*' ) == -1 )
                {
                    // pattern describes exactly one server
                    aServer = aToken;
                }

                aToken += ":*";
            }
            else
            {
                // Port given, extract server pattern
                sal_Int32 nAsteriskPos = aToken.indexOf( '*' );
                aPort = aToken.copy( nColonPos + 1 );
                if ( nAsteriskPos < nColonPos )
                {
                    // pattern describes exactly one server
                    aServer = aToken.copy( 0, nColonPos );
                }
            }

            OUStringBuffer aFullyQualifiedHost;
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
                OUString aTmp = aAddr.getHostname().toAsciiLowerCase();
                if ( aTmp != aServer.toAsciiLowerCase() )
                {
                    if ( bIPv6Address )
                        aFullyQualifiedHost.append( "[" + aTmp + "]" );
                    else
                        aFullyQualifiedHost.append( aTmp );
                    aFullyQualifiedHost.append( ":" + aPort );
                }
            }

            m_aNoProxyList.emplace_back( WildCard( aToken ),
                                  WildCard( aFullyQualifiedHost ) );
        }

        if ( nEnd != nLen )
        {
            nPos = nEnd + 1;
            nEnd = rNoProxyList.find( ';', nPos );
        }
    }
    while ( nEnd != nLen );
}

} // namespace proxydecider_impl


// InternetProxyDecider Implementation.


InternetProxyDecider::InternetProxyDecider(
    const uno::Reference< uno::XComponentContext>& rxContext )
: m_xImpl( new proxydecider_impl::InternetProxyDecider_Impl( rxContext ) )
{
}


InternetProxyDecider::~InternetProxyDecider()
{
    // Break circular reference between config listener and notifier.
    m_xImpl->dispose();
}


OUString InternetProxyDecider::getProxy(
                                            const OUString & rProtocol,
                                            const OUString & rHost,
                                            sal_Int32 nPort ) const
{
    InternetProxyServer ret(m_xImpl->getProxy(rProtocol, rHost, nPort));

    if (ret.aName.isEmpty() || ret.nPort == -1)
    {
        return ret.aName;
    }

    return ret.aName + ":" + OUString::number(ret.nPort);
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
