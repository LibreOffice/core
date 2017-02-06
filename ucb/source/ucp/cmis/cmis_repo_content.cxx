/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#ifndef SYSTEM_CURL
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <config_oauth2.h>
#include <rtl/uri.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/proxydecider.hxx>

#include "auth_provider.hxx"
#include "certvalidation_handler.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_repo_content.hxx"
#include "cmis_resultset.hxx"
#include <memory>

#define OUSTR_TO_STDSTR(s) string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace cmis
{
    RepoContent::RepoContent( const uno::Reference< uno::XComponentContext >& rxContext,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        vector< libcmis::RepositoryPtr > const & aRepos )
        : ContentImplHelper( rxContext, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_sRepositoryId( ),
        m_aRepositories( aRepos )
    {
        // Split the URL into bits
        OUString sURL = m_xIdentifier->getContentIdentifier( );
        SAL_INFO( "ucb.ucp.cmis", "RepoContent::RepoContent() " << sURL );

        m_sRepositoryId = m_aURL.getObjectPath();
        if (!m_sRepositoryId.isEmpty() && m_sRepositoryId[0] == '/')
            m_sRepositoryId = m_sRepositoryId.copy(1);
    }

    RepoContent::~RepoContent()
    {
    }

    uno::Any RepoContent::getBadArgExcept()
    {
        return uno::makeAny( lang::IllegalArgumentException(
            "Wrong argument type!",
            static_cast< cppu::OWeakObject * >( this ), -1) );
    }

    uno::Reference< sdbc::XRow > RepoContent::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_xContext );

        sal_Int32 nProps;
        const beans::Property* pProps;

        nProps = rProperties.getLength();
        pProps = rProperties.getConstArray();

        for( sal_Int32 n = 0; n < nProps; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            try
            {
                if ( rProp.Name == "IsDocument" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsFolder" )
                {
                    xRow->appendBoolean( rProp, true );
                }
                else if ( rProp.Name == "Title" )
                {
                    xRow->appendString( rProp, STD_TO_OUSTR( getRepository( xEnv )->getName( ) ) );
                }
                else if ( rProp.Name == "IsReadOnly" )
                {
                    xRow->appendBoolean( rProp, true );
                }
                else
                {
                    xRow->appendVoid( rProp );
                    SAL_INFO( "ucb.ucp.cmis", "Looking for unsupported property " << rProp.Name );
                }
            }
            catch (const libcmis::Exception&)
            {
                xRow->appendVoid( rProp );
            }
        }

        return uno::Reference< sdbc::XRow >( xRow.get() );
    }

    void RepoContent::getRepositories( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
#ifndef SYSTEM_CURL
        // Initialize NSS library to make sure libcmis (and curl) can access CACERTs using NSS
        // when using internal libcurl.
        uno::Reference< css::xml::crypto::XNSSInitializer >
            xNSSInitializer = css::xml::crypto::NSSInitializer::create( m_xContext );

        uno::Reference< css::xml::crypto::XDigestContext > xDigestContext(
                xNSSInitializer->getDigestContext( css::xml::crypto::DigestID::SHA256,
                                                          uno::Sequence< beans::NamedValue >() ),
                                                          uno::UNO_SET_THROW );
#endif

        // Set the proxy if needed. We are doing that all times as the proxy data shouldn't be cached.
        ucbhelper::InternetProxyDecider aProxyDecider( m_xContext );
        INetURLObject aBindingUrl( m_aURL.getBindingUrl( ) );
        const ucbhelper::InternetProxyServer& rProxy = aProxyDecider.getProxy(
                INetURLObject::GetScheme( aBindingUrl.GetProtocol( ) ), aBindingUrl.GetHost(), aBindingUrl.GetPort() );
        OUString sProxy = rProxy.aName;
        if ( rProxy.nPort > 0 )
            sProxy += ":" + OUString::number( rProxy.nPort );
        libcmis::SessionFactory::setProxySettings( OUSTR_TO_STDSTR( sProxy ), string(), string(), string() );

        if ( m_aRepositories.empty() )
        {
            // Set the SSL Validation handler
            libcmis::CertValidationHandlerPtr certHandler(
                    new CertValidationHandler( xEnv, m_xContext, aBindingUrl.GetHost( ) ) );
            libcmis::SessionFactory::setCertificateValidationHandler( certHandler );

            // Get the auth credentials
            AuthProvider authProvider( xEnv, m_xIdentifier->getContentIdentifier( ), m_aURL.getBindingUrl( ) );
            AuthProvider::setXEnv( xEnv );

            string rUsername = OUSTR_TO_STDSTR( m_aURL.getUsername( ) );
            string rPassword = OUSTR_TO_STDSTR( m_aURL.getPassword( ) );

            bool bIsDone = false;

            while( !bIsDone )
            {
                if ( authProvider.authenticationQuery( rUsername, rPassword ) )
                {
                    try
                    {
                        // Create a session to get repositories
                        libcmis::OAuth2DataPtr oauth2Data;
                        if ( m_aURL.getBindingUrl( ) == GDRIVE_BASE_URL )
                        {
                            libcmis::SessionFactory::setOAuth2AuthCodeProvider( AuthProvider::gdriveAuthCodeFallback );
                            oauth2Data.reset( new libcmis::OAuth2Data(
                                GDRIVE_AUTH_URL, GDRIVE_TOKEN_URL,
                                GDRIVE_SCOPE, GDRIVE_REDIRECT_URI,
                                GDRIVE_CLIENT_ID, GDRIVE_CLIENT_SECRET ) );
                        }
                        if ( m_aURL.getBindingUrl().startsWith( ALFRESCO_CLOUD_BASE_URL ) )
                            oauth2Data.reset( new libcmis::OAuth2Data(
                                ALFRESCO_CLOUD_AUTH_URL, ALFRESCO_CLOUD_TOKEN_URL,
                                ALFRESCO_CLOUD_SCOPE, ALFRESCO_CLOUD_REDIRECT_URI,
                                ALFRESCO_CLOUD_CLIENT_ID, ALFRESCO_CLOUD_CLIENT_SECRET ) );
                        if ( m_aURL.getBindingUrl( ) == ONEDRIVE_BASE_URL )
                        {
                            libcmis::SessionFactory::setOAuth2AuthCodeProvider( AuthProvider::onedriveAuthCodeFallback );
                            oauth2Data.reset( new libcmis::OAuth2Data(
                                ONEDRIVE_AUTH_URL, ONEDRIVE_TOKEN_URL,
                                ONEDRIVE_SCOPE, ONEDRIVE_REDIRECT_URI,
                                ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET ) );
                        }

                        std::unique_ptr<libcmis::Session> session(libcmis::SessionFactory::createSession(
                                OUSTR_TO_STDSTR( m_aURL.getBindingUrl( ) ),
                                rUsername, rPassword, "", false, oauth2Data ));
                        if (!session)
                            ucbhelper::cancelCommandExecution(
                                                ucb::IOErrorCode_INVALID_DEVICE,
                                                uno::Sequence< uno::Any >( 0 ),
                                                xEnv );
                        m_aRepositories = session->getRepositories( );

                        bIsDone = true;
                    }
                    catch ( const libcmis::Exception& e )
                    {
                        SAL_INFO( "ucb.ucp.cmis", "Error getting repositories: " << e.what() );

                        if ( e.getType().compare( "permissionDenied" ) != 0 )
                        {
                            ucbhelper::cancelCommandExecution(
                                            ucb::IOErrorCode_INVALID_DEVICE,
                                            uno::Sequence< uno::Any >( 0 ),
                                            xEnv );
                        }
                    }
                }
                else
                {
                    // Throw user cancelled exception
                    ucbhelper::cancelCommandExecution(
                                        ucb::IOErrorCode_ABORT,
                                        uno::Sequence< uno::Any >( 0 ),
                                        xEnv,
                                        "Authentication cancelled" );
                }
            }
        }
    }

    libcmis::RepositoryPtr RepoContent::getRepository( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        // Ensure we have the repositories extracted
        getRepositories( xEnv );

        libcmis::RepositoryPtr repo;

        if ( !m_sRepositoryId.isEmpty() )
        {
            for ( vector< libcmis::RepositoryPtr >::iterator it = m_aRepositories.begin( );
                    it != m_aRepositories.end( ) && nullptr == repo.get( ); ++it )
            {
                if ( STD_TO_OUSTR( ( *it )->getId( ) ) == m_sRepositoryId )
                    repo = *it;
            }
        }
        else
            repo = m_aRepositories.front( );
        return repo;
    }

    uno::Sequence< beans::Property > RepoContent::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
    {
        static const beans::Property aGenericProperties[] =
        {
            beans::Property( "IsDocument",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "IsFolder",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "Title",
                -1, cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( "IsReadOnly",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        };

        const int nProps = SAL_N_ELEMENTS(aGenericProperties);
        return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
    }

    uno::Sequence< ucb::CommandInfo > RepoContent::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
    {
        static const ucb::CommandInfo aCommandInfoTable[] =
        {
            // Required commands
            ucb::CommandInfo
            ( "getCommandInfo",
              -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo
            ( "getPropertySetInfo",
              -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo
            ( "getPropertyValues",
              -1, cppu::UnoType<uno::Sequence< beans::Property >>::get() ),
            ucb::CommandInfo
            ( "setPropertyValues",
              -1, cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get() ),

            // Optional standard commands
            ucb::CommandInfo
            ( "open",
              -1, cppu::UnoType<ucb::OpenCommandArgument2>::get() ),
        };

        const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, nProps );
    }

    OUString RepoContent::getParentURL( )
    {
        OUString sRet;

        SAL_INFO( "ucb.ucp.cmis", "RepoContent::getParentURL()" );

        // TODO Implement me

        return sRet;
    }

    XTYPEPROVIDER_COMMON_IMPL( RepoContent );

    OUString SAL_CALL RepoContent::getImplementationName()
    {
       return OUString("com.sun.star.comp.CmisRepoContent");
    }

    uno::Sequence< OUString > SAL_CALL RepoContent::getSupportedServiceNames()
    {
       uno::Sequence<OUString> aSNS { "com.sun.star.ucb.Content" };
       return aSNS;
    }

    OUString SAL_CALL RepoContent::getContentType()
    {
        return OUString( CMIS_REPO_TYPE );
    }

    uno::Any SAL_CALL RepoContent::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        SAL_INFO( "ucb.ucp.cmis", "RepoContent::execute( ) - " << aCommand.Name );

        uno::Any aRet;

        if ( aCommand.Name == "getPropertyValues" )
        {
            uno::Sequence< beans::Property > Properties;
            if ( !( aCommand.Argument >>= Properties ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= getPropertyValues( Properties, xEnv );
        }
        else if ( aCommand.Name == "getPropertySetInfo" )
            aRet <<= getPropertySetInfo( xEnv, false );
        else if ( aCommand.Name == "getCommandInfo" )
            aRet <<= getCommandInfo( xEnv, false );
        else if ( aCommand.Name == "open" )
        {
            ucb::OpenCommandArgument2 aOpenCommand;
            if ( !( aCommand.Argument >>= aOpenCommand ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            const ucb::OpenCommandArgument2& rOpenCommand = aOpenCommand;

            getRepositories( xEnv );
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(m_xContext, this, rOpenCommand, xEnv );
            aRet <<= xSet;
        }
        else
        {
            SAL_INFO( "ucb.ucp.cmis", "Command not allowed" );
        }

        return aRet;
    }

    void SAL_CALL RepoContent::abort( sal_Int32 /*CommandId*/ )
    {
        SAL_INFO( "ucb.ucp.cmis", "TODO - RepoContent::abort()" );
        // TODO Implement me
    }

    uno::Sequence< uno::Type > SAL_CALL RepoContent::getTypes()
    {
        static cppu::OTypeCollection aFolderCollection
            (CPPU_TYPE_REF( lang::XTypeProvider ),
             CPPU_TYPE_REF( lang::XServiceInfo ),
             CPPU_TYPE_REF( lang::XComponent ),
             CPPU_TYPE_REF( ucb::XContent ),
             CPPU_TYPE_REF( ucb::XCommandProcessor ),
             CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
             CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
             CPPU_TYPE_REF( beans::XPropertyContainer ),
             CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
             CPPU_TYPE_REF( container::XChild ) );
        return aFolderCollection.getTypes();
    }

    list< uno::Reference< ucb::XContent > > RepoContent::getChildren( )
    {
        list< uno::Reference< ucb::XContent > > result;

        // TODO Cache the results somehow
        SAL_INFO( "ucb.ucp.cmis", "RepoContent::getChildren" );

        if ( m_sRepositoryId.isEmpty( ) )
        {
            for ( vector< libcmis::RepositoryPtr >::iterator it = m_aRepositories.begin( );
                    it != m_aRepositories.end(); ++it )
            {
                URL aUrl( m_aURL );
                aUrl.setObjectPath( STD_TO_OUSTR( ( *it )->getId( ) ) );

                uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aUrl.asString( ) );
                uno::Reference< ucb::XContent > xContent = new RepoContent( m_xContext, m_pProvider, xId, m_aRepositories );

                result.push_back( xContent );
            }
        }
        else
        {
            // Return the repository root as child
            OUString sUrl;
            OUString sEncodedBinding = rtl::Uri::encode(
                    m_aURL.getBindingUrl( ) + "#" + m_sRepositoryId,
                    rtl_UriCharClassRelSegment,
                    rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_UTF8 );
            sUrl = "vnd.libreoffice.cmis://" + sEncodedBinding;

            uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( sUrl );
            uno::Reference< ucb::XContent > xContent = new Content( m_xContext, m_pProvider, xId );

            result.push_back( xContent );
        }
        return result;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
