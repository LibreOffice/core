/*************************************************************************
 *
 *  $RCSfile: proxydecider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-01 14:57:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <utility>
#include <vector>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _OSL_SOCKET_HXX_
#include <osl/socket.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _UCBHELPER_PROXYDECIDER_HXX
#include "ucbhelper/proxydecider.hxx"
#endif

using namespace com::sun::star;
using namespace ucbhelper;

#define CONFIG_ROOT_KEY     "org.openoffice.Inet/Settings"
#define PROXY_TYPE_KEY      "ooInetProxyType"
#define NO_PROXY_LIST_KEY   "ooInetNoProxy"
#define HTTP_PROXY_NAME_KEY "ooInetHTTPProxyName"
#define HTTP_PROXY_PORT_KEY "ooInetHTTPProxyPort"
#define FTP_PROXY_NAME_KEY  "ooInetFTPProxyName"
#define FTP_PROXY_PORT_KEY  "ooInetFTPProxyPort"

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
class InternetProxyDecider_Impl :
    public cppu::WeakImplHelper1< util::XChangesListener >
{
    mutable osl::Mutex                       m_aMutex;
    InternetProxyServer                      m_aHttpProxy;
    InternetProxyServer                      m_aFtpProxy;
    const InternetProxyServer                m_aEmptyProxy;
    sal_Int32                                m_nProxyType;
    uno::Reference< util::XChangesNotifier > m_xNotifier;
    std::vector< NoProxyListEntry >          m_aNoProxyList;

private:
    bool shouldUseProxy( const rtl::OUString & rHost,
                         sal_Int32 nPort,
                         bool bUseFullyQualified ) const;
public:
    InternetProxyDecider_Impl(
        const uno::Reference< lang::XMultiServiceFactory >& rxSMgr );
    virtual ~InternetProxyDecider_Impl();

    static rtl::Reference< InternetProxyDecider_Impl > createInstance(
        const uno::Reference< lang::XMultiServiceFactory >& rxSMgr );

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
//=========================================================================
//
// InternetProxyDecider_Impl Implementation.
//
//=========================================================================
//=========================================================================

InternetProxyDecider_Impl::InternetProxyDecider_Impl(
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr )
: m_nProxyType( 0 )
{
    try
    {
        //////////////////////////////////////////////////////////////
        // Read proxy configuration from config db.
        //////////////////////////////////////////////////////////////

        uno::Reference< lang::XMultiServiceFactory > xConfigProv(
                rxSMgr->createInstance(
                    rtl::OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                uno::UNO_QUERY );

        uno::Sequence< uno::Any > aArguments( 1 );
        aArguments[ 0 ] <<= rtl::OUString::createFromAscii( CONFIG_ROOT_KEY );

        uno::Reference< uno::XInterface > xInterface(
                    xConfigProv->createInstanceWithArguments(
                        rtl::OUString::createFromAscii(
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
                try
                {
                    if ( !( xNameAccess->getByName(
                                rtl::OUString::createFromAscii(
                                    PROXY_TYPE_KEY ) ) >>= m_nProxyType ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }

                rtl::OUString aNoProxyList;
                try
                {
                    if ( !( xNameAccess->getByName(
                                rtl::OUString::createFromAscii(
                                    NO_PROXY_LIST_KEY ) ) >>= aNoProxyList ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }

                setNoProxyList( aNoProxyList );

                try
                {
                    if ( !( xNameAccess->getByName(
                                rtl::OUString::createFromAscii(
                                    HTTP_PROXY_NAME_KEY ) )
                            >>= m_aHttpProxy.aName ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }

                m_aHttpProxy.nPort = -1;
                try
                {
                    uno::Any aValue = xNameAccess->getByName(
                            rtl::OUString::createFromAscii(
                                HTTP_PROXY_PORT_KEY ) );
                    if ( aValue.hasValue() &&
                         !( aValue >>= m_aHttpProxy.nPort ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }

                if ( m_aHttpProxy.nPort == -1 )
                    m_aHttpProxy.nPort = 80; // standard HTTP port.

                try
                {
                    if ( !( xNameAccess->getByName(
                                rtl::OUString::createFromAscii(
                                    FTP_PROXY_NAME_KEY ) )
                            >>= m_aFtpProxy.aName ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }

                m_aFtpProxy.nPort = -1;
                try
                {
                    uno::Any aValue = xNameAccess->getByName(
                            rtl::OUString::createFromAscii(
                                FTP_PROXY_PORT_KEY ) );
                    if ( aValue.hasValue() &&
                         !( aValue >>= m_aFtpProxy.nPort ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - "
                                    "Error getting config item value!" );
                    }
                }
                catch ( lang::WrappedTargetException const & )
                {
                }
                catch ( container::NoSuchElementException const & )
                {
                }
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
        OSL_ENSURE( sal_False, "InternetProxyDecider - Exception!" );
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
    rtl::OUStringBuffer aBuffer( rHost );
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
        it++;
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

    if ( rHost.getLength() && m_aNoProxyList.size() )
    {
        //////////////////////////////////////////////////////////////////
        // First, try direct hostname match - #110515#
        //////////////////////////////////////////////////////////////////

        if ( !shouldUseProxy( rHost, nPort, false ) )
            return m_aEmptyProxy;

        //////////////////////////////////////////////////////////////////
        // Second, try match against full qualified hostname - #104401#
        //////////////////////////////////////////////////////////////////

        // This might be quite expensive (DNS lookup).
        const osl::SocketAddr aAddr( rHost, nPort );
        rtl::OUString aFullyQualifiedHost(
            aAddr.getHostname().toAsciiLowerCase() );

        // Error resolving name? -> fallback.
        if ( !aFullyQualifiedHost.getLength() )
            aFullyQualifiedHost = rHost;

        if ( aFullyQualifiedHost != rHost )
        {
            if ( !shouldUseProxy( aFullyQualifiedHost, nPort, false ) )
                return m_aEmptyProxy;
        }

        //////////////////////////////////////////////////////////////////
        // Third, try match of fully qualified entries in no-proxy lit
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

    if ( rProtocol.toAsciiLowerCase()
            .equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ftp" ) ) )
    {
        if ( m_aFtpProxy.aName.getLength() > 0 && m_aFtpProxy.nPort >= 0 )
            return m_aFtpProxy;
    }
    else if ( m_aHttpProxy.aName.getLength() )
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
            if ( ( rElem.Accessor >>= aKey ) && aKey.getLength() )
            {
                if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    PROXY_TYPE_KEY ) ) )
                {
                    if ( !( rElem.Element >>= m_nProxyType ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    NO_PROXY_LIST_KEY ) ) )
                {
                    rtl::OUString aNoProxyList;
                    if ( !( rElem.Element >>= aNoProxyList ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }

                    setNoProxyList( aNoProxyList );
                }
                else if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    HTTP_PROXY_NAME_KEY ) ) )
                {
                    if ( !( rElem.Element >>= m_aHttpProxy.aName ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    HTTP_PROXY_PORT_KEY ) ) )
                {
                    if ( !( rElem.Element >>= m_aHttpProxy.nPort ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }

                    if ( m_aHttpProxy.nPort == -1 )
                        m_aHttpProxy.nPort = 80; // standard HTTP port.
                }
                else if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    FTP_PROXY_NAME_KEY ) ) )
                {
                    if ( !( rElem.Element >>= m_aFtpProxy.aName ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
                else if ( aKey.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                                    FTP_PROXY_PORT_KEY ) ) )
                {
                    if ( !( rElem.Element >>= m_aFtpProxy.nPort ) )
                    {
                        OSL_ENSURE( sal_False,
                                    "InternetProxyDecider - changesOccurred - "
                                    "Error getting config item value!" );
                    }
                }
            }
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL InternetProxyDecider_Impl::disposing(
                                        const lang::EventObject& Source )
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

    if ( rNoProxyList.getLength() )
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

            if ( aToken.getLength() )
            {
                rtl::OUString aServer;
                rtl::OUString aPort;
                sal_Int32 nColonPos = aToken.indexOf( ':' );
                if ( nColonPos == -1 )
                {
                    // No port given, server pattern equals current token
                    aPort = rtl::OUString::createFromAscii( "*" );
                    if ( aToken.indexOf( '*' ) == -1 )
                    {
                        // pattern describes exactly one server
                        aServer = aToken;
                    }

                    aToken += rtl::OUString::createFromAscii( ":*" );
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
                if ( aServer.getLength() )
                {
                    // Remember fully qualified server name if current list
                    // entry specifies exactly one non-fully qualified server
                    // name.

                    // This might be quite expensive (DNS lookup).
                    const osl::SocketAddr aAddr( aServer, 0 );
                    rtl::OUString aTmp = aAddr.getHostname().toAsciiLowerCase();
                    if ( aTmp != aServer.toAsciiLowerCase() )
                    {
                        aFullyQualifiedHost.append( aTmp );
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
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr )
: m_pImpl( new proxydecider_impl::InternetProxyDecider_Impl( rxSMgr ) )
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
    return ( rData.aName.getLength() > 0 );
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
