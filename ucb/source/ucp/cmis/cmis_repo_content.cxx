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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

#include <comphelper/processfactory.hxx>
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

#define OUSTR_TO_STDSTR(s) string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace cmis
{
    RepoContent::RepoContent( const uno::Reference< uno::XComponentContext >& rxContext,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        vector< libcmis::RepositoryPtr > aRepos )
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxContext, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_sRepositoryId( ),
        m_aRepositories( aRepos )
    {
        // Split the URL into bits
        OUString sURL = m_xIdentifier->getContentIdentifier( );
        SAL_INFO( "cmisucp", "RepoContent::RepoContent() " << sURL );

        m_sRepositoryId = m_aURL.getObjectPath( );
        if ( m_sRepositoryId[0] == '/' )
            m_sRepositoryId = m_sRepositoryId.copy( 1 );

    }

    RepoContent::~RepoContent()
    {
    }

    uno::Any RepoContent::getBadArgExcept()
    {
        return uno::makeAny( lang::IllegalArgumentException(
            OUString("Wrong argument type!"),
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
                    xRow->appendBoolean( rProp, sal_False );
                }
                else if ( rProp.Name == "IsFolder" )
                {
                    xRow->appendBoolean( rProp, sal_True );
                }
                else if ( rProp.Name == "Title" )
                {
                    xRow->appendString( rProp, STD_TO_OUSTR( getRepository( xEnv )->getName( ) ) );
                }
                else if ( rProp.Name == "IsReadOnly" )
                {
                    xRow->appendBoolean( rProp, sal_True );
                }
                else
                {
                    xRow->appendVoid( rProp );
                    SAL_INFO( "cmisucp", "Looking for unsupported property " << rProp.Name );
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

            string rUsername = OUSTR_TO_STDSTR( m_aURL.getUsername( ) );
            string rPassword = OUSTR_TO_STDSTR( m_aURL.getPassword( ) );
            if ( authProvider.authenticationQuery( rUsername, rPassword ) )
            {
                try
                {
                    // Create a session to get repositories
                    libcmis::OAuth2DataPtr oauth2Data = NULL;

                    libcmis::Session* session = libcmis::SessionFactory::createSession(
                            OUSTR_TO_STDSTR( m_aURL.getBindingUrl( ) ),
                            rUsername, rPassword, "", sal_False, oauth2Data );
                    if (session == NULL )
                        ucbhelper::cancelCommandExecution(
                                            ucb::IOErrorCode_INVALID_DEVICE,
                                            uno::Sequence< uno::Any >( 0 ),
                                            xEnv,
                                            OUString( ) );
                    m_aRepositories = session->getRepositories( );
                    delete session;
                }
                catch (const libcmis::Exception& e)
                {
                    SAL_INFO( "cmisucp", "Error getting repositories: " << e.what() );
                }
            }
            else
            {
                // Throw user cancelled exception
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_ABORT,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    OUString( "Authentication cancelled" ) );
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
                    it != m_aRepositories.end( ) && NULL == repo.get( ); ++it )
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
            beans::Property( OUString( "IsDocument" ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( OUString( "IsFolder" ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( OUString( "Title" ),
                -1, getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( OUString( "IsReadOnly" ),
                -1, getCppuBooleanType(),
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
            ( OUString( "getCommandInfo" ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( OUString( "getPropertySetInfo" ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( OUString( "getPropertyValues" ),
              -1, getCppuType( static_cast<uno::Sequence< beans::Property > * >( 0 ) ) ),
            ucb::CommandInfo
            ( OUString( "setPropertyValues" ),
              -1, getCppuType( static_cast<uno::Sequence< beans::PropertyValue > * >( 0 ) ) ),

            // Optional standard commands
            ucb::CommandInfo
            ( OUString( "open" ),
              -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),
        };

        const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, nProps );
    }

    OUString RepoContent::getParentURL( )
    {
        OUString sRet;

        SAL_INFO( "cmisucp", "RepoContent::getParentURL()" );

        // TODO Implement me

        return sRet;
    }

    XTYPEPROVIDER_COMMON_IMPL( RepoContent );

    void SAL_CALL RepoContent::acquire() throw()
    {
        ContentImplHelper::acquire();
    }

    void SAL_CALL RepoContent::release() throw()
    {
        ContentImplHelper::release();
    }

    uno::Any SAL_CALL RepoContent::queryInterface( const uno::Type & rType ) throw ( uno::RuntimeException )
    {
        return ContentImplHelper::queryInterface(rType);
    }

    OUString SAL_CALL RepoContent::getImplementationName() throw( uno::RuntimeException )
    {
       return OUString("com.sun.star.comp.CmisRepoContent");
    }

    uno::Sequence< OUString > SAL_CALL RepoContent::getSupportedServiceNames()
           throw( uno::RuntimeException )
    {
       uno::Sequence< OUString > aSNS( 1 );
       aSNS.getArray()[ 0 ] = "com.sun.star.ucb.Content";
       return aSNS;
    }

    OUString SAL_CALL RepoContent::getContentType() throw( uno::RuntimeException )
    {
        return OUString( CMIS_REPO_TYPE );
    }

    uno::Any SAL_CALL RepoContent::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
            throw( uno::Exception, ucb::CommandAbortedException, uno::RuntimeException )
    {
        SAL_INFO( "cmisucp", "RepoContent::execute( ) - " << aCommand.Name );

        uno::Any aRet;

        if ( aCommand.Name == "getPropertyValues" )
        {
            uno::Sequence< beans::Property > Properties;
            if ( !( aCommand.Argument >>= Properties ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= getPropertyValues( Properties, xEnv );
        }
        else if ( aCommand.Name == "getPropertySetInfo" )
            aRet <<= getPropertySetInfo( xEnv, sal_False );
        else if ( aCommand.Name == "getCommandInfo" )
            aRet <<= getCommandInfo( xEnv, sal_False );
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
            SAL_INFO( "cmisucp", "Command not allowed" );
        }

        return aRet;
    }

    void SAL_CALL RepoContent::abort( sal_Int32 /*CommandId*/ ) throw( uno::RuntimeException )
    {
        SAL_INFO( "cmisucp", "TODO - RepoContent::abort()" );
        // TODO Implement me
    }

    uno::Sequence< uno::Type > SAL_CALL RepoContent::getTypes() throw( uno::RuntimeException )
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
        SAL_INFO( "cmisucp", "RepoContent::getChildren" );

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
