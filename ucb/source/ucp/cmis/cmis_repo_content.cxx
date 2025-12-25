/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <boost/make_shared.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>

#include <config_oauth2.h>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <ucbhelper/macros.hxx>

#include "auth_provider.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_repo_content.hxx"
#include "cmis_resultset.hxx"
#include <memory>

#define OUSTR_TO_STDSTR(s) std::string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ) )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;

namespace cmis
{
    RepoContent::RepoContent( const uno::Reference< uno::XComponentContext >& rxContext,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        std::vector< libcmis::RepositoryPtr > && aRepos )
        : ContentImplHelper( rxContext, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_aRepositories( std::move(aRepos) )
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
        return uno::Any( lang::IllegalArgumentException(
            u"Wrong argument type!"_ustr,
            getXWeak(), -1) );
    }

    uno::Reference< sdbc::XRow > RepoContent::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_xContext );

        for( const beans::Property& rProp : rProperties )
        {
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

        return xRow;
    }

    void RepoContent::getRepositories( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        // Set the proxy if needed. We are doing that all times as the proxy data shouldn't be cached.
        ucbhelper::InternetProxyDecider aProxyDecider( m_xContext );
        INetURLObject aBindingUrl( m_aURL.getBindingUrl( ) );
        const OUString sProxy = aProxyDecider.getProxy(
                INetURLObject::GetScheme( aBindingUrl.GetProtocol( ) ), aBindingUrl.GetHost(), aBindingUrl.GetPort() );
        libcmis::SessionFactory::setProxySettings( OUSTR_TO_STDSTR( sProxy ), std::string(), std::string(), std::string() );

        if ( !m_aRepositories.empty() )
            return;

        libcmis::Session* session = createSession(xEnv, *m_pProvider, m_aURL,
                                                  m_xIdentifier->getContentIdentifier(), this);
        assert(session);
        m_aRepositories = session->getRepositories();
    }

    libcmis::RepositoryPtr RepoContent::getRepository( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        // Ensure we have the repositories extracted
        getRepositories( xEnv );

        libcmis::RepositoryPtr repo;

        if ( !m_sRepositoryId.isEmpty() )
        {
            auto it = std::find_if(m_aRepositories.begin(), m_aRepositories.end(),
                [&](const libcmis::RepositoryPtr& rRepo) { return STD_TO_OUSTR(rRepo->getId()) == m_sRepositoryId; });
            if (it != m_aRepositories.end())
                repo = *it;
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
            beans::Property( u"IsDocument"_ustr,
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( u"IsFolder"_ustr,
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( u"Title"_ustr,
                -1, cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( u"IsReadOnly"_ustr,
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
            ( u"getCommandInfo"_ustr,
              -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo
            ( u"getPropertySetInfo"_ustr,
              -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo
            ( u"getPropertyValues"_ustr,
              -1, cppu::UnoType<uno::Sequence< beans::Property >>::get() ),
            ucb::CommandInfo
            ( u"setPropertyValues"_ustr,
              -1, cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get() ),

            // Optional standard commands
            ucb::CommandInfo
            ( u"open"_ustr,
              -1, cppu::UnoType<ucb::OpenCommandArgument2>::get() ),
        };

        const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, nProps );
    }

    OUString RepoContent::getParentURL( )
    {
        SAL_INFO( "ucb.ucp.cmis", "RepoContent::getParentURL()" );

        // TODO Implement me

        return OUString();
    }

    XTYPEPROVIDER_COMMON_IMPL( RepoContent );

    OUString SAL_CALL RepoContent::getImplementationName()
    {
       return u"com.sun.star.comp.CmisRepoContent"_ustr;
    }

    uno::Sequence< OUString > SAL_CALL RepoContent::getSupportedServiceNames()
    {
       return { u"com.sun.star.ucb.Content"_ustr };
    }

    OUString SAL_CALL RepoContent::getContentType()
    {
        return CMIS_REPO_TYPE;
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
        static cppu::OTypeCollection s_aFolderCollection
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
        return s_aFolderCollection.getTypes();
    }

    std::vector< uno::Reference< ucb::XContent > > RepoContent::getChildren( )
    {
        std::vector< uno::Reference< ucb::XContent > > result;

        // TODO Cache the results somehow
        SAL_INFO( "ucb.ucp.cmis", "RepoContent::getChildren" );

        if ( m_sRepositoryId.isEmpty( ) )
        {
            for ( const auto& rRepo : m_aRepositories )
            {
                URL aUrl( m_aURL );
                aUrl.setObjectPath( STD_TO_OUSTR( rRepo->getId( ) ) );

                uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aUrl.asString( ) );
                uno::Reference< ucb::XContent > xContent = new RepoContent( m_xContext, m_pProvider, xId, std::vector(m_aRepositories) );

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
