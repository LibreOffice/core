/*************************************************************************
 *
 *  $RCSfile: DAVResourceAccess.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:39:02 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#include <ucbhelper/simpleauthenticationrequest.hxx>
#endif

#ifndef _DAVAUTHLISTENERIMPL_HXX_
#include "DAVAuthListenerImpl.hxx"
#endif
#ifndef _DAVRESOURCEACCESS_HXX_
#include "DAVResourceAccess.hxx"
#endif


using namespace webdav_ucp;
using namespace com::sun::star;


//=========================================================================
//=========================================================================
//
// DAVAuthListener_Impl Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
// virtual
int DAVAuthListener_Impl::authenticate(
    const ::rtl::OUString & inRealm,
    const ::rtl::OUString & inHostName,
    ::rtl::OUString & inoutUserName,
    ::rtl::OUString & outPassWord )
{
    if ( m_xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH
            = m_xEnv->getInteractionHandler();
        if ( xIH.is() )
        {
            // #102871# - Supply username and password from previous try.
            // Password container service depends on this!
            if ( inoutUserName.getLength() == 0 )
                inoutUserName = m_aPrevUsername;

            if ( outPassWord.getLength() == 0 )
                outPassWord = m_aPrevPassword;

            rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
                = new ucbhelper::SimpleAuthenticationRequest( inHostName,
                                                              inRealm,
                                                              inoutUserName,
                                                              outPassWord );
            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
            {
                // Handler handled the request.
                uno::Reference< task::XInteractionAbort > xAbort(
                    xSelection.get(), uno::UNO_QUERY );
                if ( !xAbort.is() )
                {
                    const rtl::Reference<
                        ucbhelper::InteractionSupplyAuthentication > & xSupp
                        = xRequest->getAuthenticationSupplier();

                    inoutUserName = xSupp->getUserName();
                    outPassWord   = xSupp->getPassword();

                    // #102871# - Remember username and password.
                    m_aPrevUsername = inoutUserName;
                    m_aPrevPassword = outPassWord;

                    // go on.
                    return 0;
                }
            }
        }
    }
    // Abort.
    return -1;
}

//=========================================================================
//=========================================================================
//
// DAVResourceAccess Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
DAVResourceAccess::DAVResourceAccess(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr,
                rtl::Reference< DAVSessionFactory > const & rSessionFactory,
                const rtl::OUString & rURL )
    throw( DAVException )
: m_aURL( rURL ),
  m_xSMgr( rSMgr ),
  m_xSessionFactory( rSessionFactory )
{
}

//=========================================================================
void DAVResourceAccess::OPTIONS( DAVCapabilities & rCapabilities,
                                   const uno::Reference<
                                      ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->OPTIONS( getRequestURI(),
                                 rCapabilities,
                                 DAVRequestEnvironment(
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PROPFIND( const Depth nDepth,
                          const std::vector< rtl::OUString > & rPropertyNames,
                          std::vector< DAVResource > & rResources,
                          const uno::Reference<
                                  ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->PROPFIND( getRequestURI(),
                                  nDepth,
                                  rPropertyNames,
                                  rResources,
                                  DAVRequestEnvironment(
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PROPFIND( const Depth nDepth,
                                    std::vector< DAVResourceInfo > & rResInfo,
                                    const uno::Reference<
                                      ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->PROPFIND( getRequestURI(),
                                  nDepth,
                                  rResInfo,
                                  DAVRequestEnvironment(
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv ) ) ) ;
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PROPPATCH( const std::vector< ProppatchValue >& rValues,
                                      const uno::Reference<
                                       ucb::XCommandEnvironment >& xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->PROPPATCH( getRequestURI(),
                                   rValues,
                                   DAVRequestEnvironment(
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::HEAD( const std::vector< rtl::OUString > & rHeaderNames,
                              DAVResource & rResource,
                              const uno::Reference<
                                ucb::XCommandEnvironment >& xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->HEAD( getRequestURI(),
                              rHeaderNames,
                              rResource,
                              DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::GET(
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    uno::Reference< io::XInputStream > xStream;
    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            xStream = m_xSession->GET( getRequestURI(),
                                       DAVRequestEnvironment(
                                        getRequestURI(),
                                        new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return xStream;
}

//=========================================================================
void DAVResourceAccess::GET( uno::Reference< io::XOutputStream > & rStream,
                               const uno::Reference<
                                      ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->GET( getRequestURI(),
                             rStream,
                             DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::GET(
                const std::vector< rtl::OUString > & rHeaderNames,
                DAVResource & rResource,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    uno::Reference< io::XInputStream > xStream;
    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            xStream = m_xSession->GET( getRequestURI(),
                                       rHeaderNames,
                                       rResource,
                                       DAVRequestEnvironment(
                                        getRequestURI(),
                                        new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return xStream;
}

//=========================================================================
void DAVResourceAccess::GET(
                    uno::Reference< io::XOutputStream > & rStream,
                    const std::vector< rtl::OUString > & rHeaderNames,
                    DAVResource & rResource,
                    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->GET( getRequestURI(),
                             rStream,
                             rHeaderNames,
                             rResource,
                             DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PUT( const uno::Reference< io::XInputStream > & rStream,
                               const uno::Reference<
                                     ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->PUT( getRequestURI(),
                             rStream,
                             DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::POST(
                const rtl::OUString & rContentType,
                    const rtl::OUString & rReferer,
                const uno::Reference< io::XInputStream > & rInputStream,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( DAVException )
{
    initialize();

    uno::Reference< io::XInputStream > xStream;
    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            xStream = m_xSession->POST( getRequestURI(),
                                        rContentType,
                                        rReferer,
                                        rInputStream,
                                        DAVRequestEnvironment(
                                            getRequestURI(),
                                            new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return xStream;
}

//=========================================================================

void DAVResourceAccess::POST(
                const rtl::OUString & rContentType,
                const rtl::OUString & rReferer,
                const uno::Reference< io::XInputStream > & rInputStream,
                uno::Reference< io::XOutputStream > & rOutputStream,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->POST( getRequestURI(),
                              rContentType,
                              rReferer,
                              rInputStream,
                              rOutputStream,
                              DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::MKCOL( const uno::Reference<
                                      ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->MKCOL( getRequestURI(),
                               DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::COPY( const ::rtl::OUString & rSourcePath,
                              const ::rtl::OUString & rDestinationURI,
                                sal_Bool bOverwrite,
                                const uno::Reference<
                                          ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->COPY( rSourcePath,
                              rDestinationURI,
                              DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ),
                              bOverwrite );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::MOVE( const ::rtl::OUString & rSourcePath,
                              const ::rtl::OUString & rDestinationURI,
                                sal_Bool bOverwrite,
                                const uno::Reference<
                                          ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->MOVE( rSourcePath,
                              rDestinationURI,
                              DAVRequestEnvironment(
                                getRequestURI(),
                                new DAVAuthListener_Impl( xEnv ) ),
                              bOverwrite );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::DESTROY( const uno::Reference<
                                       ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
    initialize();

    sal_Bool bRetry;
    do
    {
        bRetry = sal_False;
        try
        {
            m_xSession->DESTROY( getRequestURI(),
                                 DAVRequestEnvironment(
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv ) ) );
        }
        catch ( DAVException & e )
        {
            bRetry = handleException( e );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::LOCK ( const ucb::Lock & rLock,
                                 const uno::Reference<
                                      ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
//    initialize();
    OSL_ENSURE( sal_False, "DAVResourceAccess::LOCK - NYI" );
}

//=========================================================================
void DAVResourceAccess::UNLOCK ( const ucb::Lock & rLock,
                                   const uno::Reference<
                                       ucb::XCommandEnvironment > & xEnv )
    throw( DAVException )
{
//    initialize();
    OSL_ENSURE( sal_False, "DAVResourceAccess::UNLOCK - NYI" );
}

//=========================================================================
void DAVResourceAccess::setURL( const rtl::OUString & rNewURL )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_aURL  = rNewURL;
    m_aPath = rtl::OUString(); // Next initialize() will create new session.
}

//=========================================================================
// init dav session and path
void DAVResourceAccess::initialize()
    throw ( DAVException )
{
    if ( m_aPath.getLength() == 0 )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if ( m_aPath.getLength() == 0 )
        {
            NeonUri aURI( m_aURL );
            rtl::OUString aPath( aURI.GetPath() );
            if ( !aPath.getLength() )
                throw DAVException( DAVException::DAV_INVALID_ARG );

            if ( !m_xSession.is() || !m_xSession->CanUse( m_aURL ) )
            {
                m_xSession.clear();

                // create new webdav session
                m_xSession
                    = m_xSessionFactory->createDAVSession( m_aURL, m_xSMgr );

                if ( !m_xSession.is() )
                    return;
            }

            // Own URI is needed for redirect cycle detection.
            m_aRedirectURIs.push_back( aURI );

            // Success.
            m_aPath = aPath;
        }
    }
}

//=========================================================================
const rtl::OUString & DAVResourceAccess::getRequestURI() const
{
    OSL_ENSURE( m_xSession.is(),
                "DAVResourceAccess::getRequestURI - Not initialized!" );

    // In case a proxy is used we have to use the absolute URI for a request.
    if ( m_xSession->UsesProxy() )
        return m_aURL;

    return m_aPath;
}

//=========================================================================
sal_Bool DAVResourceAccess::detectRedirectCycle(
                                const rtl::OUString& rRedirectURL )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    NeonUri aUri( rRedirectURL );

    std::vector< NeonUri >::const_iterator it  = m_aRedirectURIs.begin();
    std::vector< NeonUri >::const_iterator end = m_aRedirectURIs.end();

    while ( it != end )
    {
        if ( aUri == (*it) )
            return sal_True;

        it++;
    }

    return sal_False;
}

//=========================================================================
sal_Bool DAVResourceAccess::handleException( DAVException & e )
{
    switch ( e.getError() )
    {
        case DAVException::DAV_HTTP_REDIRECT:
            try
            {
                if ( !detectRedirectCycle( e.getData() ) )
                {
                    // set new URL and path.
                    setURL( e.getData() );
                    initialize();
                    return sal_True;
                }
            }
            catch( DAVException const & )
            {
            }
            return sal_False;

        default:
            return sal_False; // Abort
    }
}
