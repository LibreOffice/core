/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

#include "auth_provider.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_repo_content.hxx"
#include "cmis_resultset.hxx"

#define OUSTR_TO_STDSTR(s) string( rtl::OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) rtl::OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace cmis
{
    RepoContent::RepoContent( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        list< libcmis::RepositoryPtr > aRepos )
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxSMgr, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_sRepositoryId( ),
        m_aRepositories( aRepos )
    {
        // Split the URL into bits
        rtl::OUString sURL = m_xIdentifier->getContentIdentifier( );
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
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wrong argument type!")),
            static_cast< cppu::OWeakObject * >( this ), -1) );
    }

    uno::Reference< sdbc::XRow > RepoContent::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_xSMgr );

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
        if ( m_aRepositories.size( ) == 0 )
        {
            // Get the auth credentials
            AuthProvider authProvider( xEnv, m_xIdentifier->getContentIdentifier( ), m_aURL.getBindingUrl( ) );

            string rUsername = OUSTR_TO_STDSTR( m_aURL.getUsername( ) );
            string rPassword = OUSTR_TO_STDSTR( m_aURL.getPassword( ) );
            if ( authProvider.authenticationQuery( rUsername, rPassword ) )
            {
                try
                {
                    m_aRepositories = libcmis::SessionFactory::getRepositories(
                           OUSTR_TO_STDSTR( m_aURL.getBindingUrl( ) ), rUsername, rPassword );
                }
                catch (const libcmis::Exception&)
                {
                }
            }
            else
            {
                // Throw user cancelled exception
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_ABORT,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    rtl::OUString::createFromAscii( "Authentication cancelled" ) );
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
            for ( list< libcmis::RepositoryPtr >::iterator it = m_aRepositories.begin( );
                    it != m_aRepositories.end( ) && NULL == repo.get( ); ++it )
            {
                if ( STD_TO_OUSTR( ( *it )->getId( ) ) == m_sRepositoryId )
                    repo = *it;
            }
        }
        return repo;
    }

    uno::Sequence< beans::Property > RepoContent::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
    {
        static const beans::Property aGenericProperties[] =
        {
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        };

        const int nProps = SAL_N_ELEMENTS(aGenericProperties);
        return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
    }

    uno::Sequence< ucb::CommandInfo > RepoContent::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
    {
        static ucb::CommandInfo aCommandInfoTable[] =
        {
            // Required commands
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
              -1, getCppuType( static_cast<uno::Sequence< beans::Property > * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
              -1, getCppuType( static_cast<uno::Sequence< beans::PropertyValue > * >( 0 ) ) ),

            // Optional standard commands
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
              -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),
        };

        const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, nProps );
    }

    ::rtl::OUString RepoContent::getParentURL( )
    {
        rtl::OUString sRet;

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

    rtl::OUString SAL_CALL RepoContent::getImplementationName() throw( uno::RuntimeException )
    {
       return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.CmisRepoContent"));
    }

    uno::Sequence< rtl::OUString > SAL_CALL RepoContent::getSupportedServiceNames()
           throw( uno::RuntimeException )
    {
       uno::Sequence< rtl::OUString > aSNS( 1 );
       aSNS.getArray()[ 0 ] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.Content"));
       return aSNS;
    }

    rtl::OUString SAL_CALL RepoContent::getContentType() throw( uno::RuntimeException )
    {
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CMIS_REPO_TYPE ) );
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
                = new DynamicResultSet(m_xSMgr, this, rOpenCommand, xEnv );
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
            for ( list< libcmis::RepositoryPtr >::iterator it = m_aRepositories.begin( );
                    it != m_aRepositories.end(); ++it )
            {
                URL aUrl( m_aURL );
                aUrl.setObjectPath( STD_TO_OUSTR( ( *it )->getId( ) ) );

                uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aUrl.asString( ) );
                uno::Reference< ucb::XContent > xContent = new RepoContent( m_xSMgr, m_pProvider, xId, m_aRepositories );

                result.push_back( xContent );
            }
        }
        else
        {
            // Return the repository root as child
            rtl::OUString sUrl;
            rtl::OUString sEncodedBinding = rtl::Uri::encode(
                    m_aURL.getBindingUrl( ) + "#" + m_sRepositoryId,
                    rtl_UriCharClassRelSegment,
                    rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_UTF8 );
            sUrl = "vnd.libreoffice.cmis+atom://" + sEncodedBinding;

            uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( sUrl );
            uno::Reference< ucb::XContent > xContent = new Content( m_xSMgr, m_pProvider, xId );

            result.push_back( xContent );
        }
        return result;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
