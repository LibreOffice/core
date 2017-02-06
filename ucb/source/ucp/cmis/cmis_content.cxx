/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdio>

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/document/CmisProperty.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument2.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#ifndef SYSTEM_CURL
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <config_oauth2.h>
#include <o3tl/runtimetooustring.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <sax/tools/converter.hxx>

#include "auth_provider.hxx"
#include "certvalidation_handler.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_resultset.hxx"
#include "cmis_strings.hxx"
#include <std_inputstream.hxx>
#include <std_outputstream.hxx>

#define OUSTR_TO_STDSTR(s) string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace
{
    util::DateTime lcl_boostToUnoTime(const boost::posix_time::ptime& boostTime)
    {
        util::DateTime unoTime;
        unoTime.Year = boostTime.date().year();
        unoTime.Month = boostTime.date().month();
        unoTime.Day = boostTime.date().day();
        unoTime.Hours = boostTime.time_of_day().hours();
        unoTime.Minutes = boostTime.time_of_day().minutes();
        unoTime.Seconds = boostTime.time_of_day().seconds();

        // TODO FIXME maybe we should compile with BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
        //            to actually get nanosecond precision in boostTime?
        // use this way rather than total_nanos to avoid overflows with 32-bit long
        const long ticks = boostTime.time_of_day().fractional_seconds();
        long nanoSeconds = ticks * ( 1000000000 / boost::posix_time::time_duration::ticks_per_second());

        unoTime.NanoSeconds = nanoSeconds;

        return unoTime;
    }

    uno::Any lcl_cmisPropertyToUno( const libcmis::PropertyPtr& pProperty )
    {
        uno::Any aValue;
        switch ( pProperty->getPropertyType( )->getType( ) )
        {
            default:
            case libcmis::PropertyType::String:
                {
                    vector< string > aCmisStrings = pProperty->getStrings( );
                    uno::Sequence< OUString > aStrings( aCmisStrings.size( ) );
                    OUString* aStringsArr = aStrings.getArray( );
                    sal_Int32 i = 0;
                    for ( vector< string >::iterator it = aCmisStrings.begin( );
                            it != aCmisStrings.end( ); ++it, ++i )
                    {
                        string str = *it;
                        aStringsArr[i] = STD_TO_OUSTR( str );
                    }
                    aValue <<= aStrings;
                }
                break;
            case libcmis::PropertyType::Integer:
                {
                    vector< long > aCmisLongs = pProperty->getLongs( );
                    uno::Sequence< sal_Int64 > aLongs( aCmisLongs.size( ) );
                    sal_Int64* aLongsArr = aLongs.getArray( );
                    sal_Int32 i = 0;
                    for ( vector< long >::iterator it = aCmisLongs.begin( );
                            it != aCmisLongs.end( ); ++it, ++i )
                    {
                        aLongsArr[i] = *it;
                    }
                    aValue <<= aLongs;
                }
                break;
            case libcmis::PropertyType::Decimal:
                {
                    vector< double > aCmisDoubles = pProperty->getDoubles( );
                    uno::Sequence< double > aDoubles( aCmisDoubles.size( ) );
                    double* aDoublesArr = aDoubles.getArray( );
                    sal_Int32 i = 0;
                    for ( vector< double >::iterator it = aCmisDoubles.begin( );
                            it != aCmisDoubles.end( ); ++it, ++i )
                    {
                        aDoublesArr[i] = *it;
                    }
                    aValue <<= aDoubles;
                }
                break;
            case libcmis::PropertyType::Bool:
                {
                    vector< bool > aCmisBools = pProperty->getBools( );
                    uno::Sequence< sal_Bool > aBools( aCmisBools.size( ) );
                    sal_Bool* aBoolsArr = aBools.getArray( );
                    sal_Int32 i = 0;
                    for ( vector< bool >::iterator it = aCmisBools.begin( );
                            it != aCmisBools.end( ); ++it, ++i )
                    {
                        aBoolsArr[i] = *it;
                    }
                    aValue <<= aBools;
                }
                break;
            case libcmis::PropertyType::DateTime:
                {
                    vector< boost::posix_time::ptime > aCmisTimes = pProperty->getDateTimes( );
                    uno::Sequence< util::DateTime > aTimes( aCmisTimes.size( ) );
                    util::DateTime* aTimesArr = aTimes.getArray( );
                    sal_Int32 i = 0;
                    for ( vector< boost::posix_time::ptime >::iterator it = aCmisTimes.begin( );
                            it != aCmisTimes.end( ); ++it, ++i )
                    {
                        aTimesArr[i] = lcl_boostToUnoTime( *it );
                    }
                    aValue <<= aTimes;
                }
                break;
        }
        return aValue;
    }

    libcmis::PropertyPtr lcl_unoToCmisProperty(const document::CmisProperty& prop )
    {
        libcmis::PropertyTypePtr propertyType( new libcmis::PropertyType( ) );

        OUString id = prop.Id;
        OUString name = prop.Name;
        bool bUpdatable = prop.Updatable;
        bool bRequired = prop.Required;
        bool bMultiValued = prop.MultiValued;
        bool bOpenChoice = prop.OpenChoice;
        uno::Any value = prop.Value;
        std::vector< std::string > values;

        libcmis::PropertyType::Type type = libcmis::PropertyType::String;
        if ( prop.Type == CMIS_TYPE_STRING )
        {
            uno::Sequence< OUString > seqValue;
            value >>= seqValue;
            sal_Int32 nNumValue = seqValue.getLength( );
            for ( sal_Int32 i = 0; i < nNumValue; ++i )
            {
                values.push_back( OUSTR_TO_STDSTR( seqValue[i] ) );
            }
            type = libcmis::PropertyType::String;
        }
        else if ( prop.Type == CMIS_TYPE_BOOL )
        {
            uno::Sequence< sal_Bool > seqValue;
            value >>= seqValue;
            sal_Int32 nNumValue = seqValue.getLength( );
            for ( sal_Int32 i = 0; i < nNumValue; ++i )
            {
                values.push_back( OUSTR_TO_STDSTR( OUString::boolean( seqValue[i] ) ) );
            }
            type = libcmis::PropertyType::Bool;
        }
        else if ( prop.Type == CMIS_TYPE_INTEGER )
        {
            uno::Sequence< sal_Int64 > seqValue;
            value >>= seqValue;
            sal_Int32 nNumValue = seqValue.getLength( );
            for ( sal_Int32 i = 0; i < nNumValue; ++i )
            {
                values.push_back( OUSTR_TO_STDSTR( OUString::number( seqValue[i] ) ) );
            }
            type = libcmis::PropertyType::Integer;
        }
        else if ( prop.Type == CMIS_TYPE_DECIMAL )
        {
            uno::Sequence< double > seqValue;
            value >>= seqValue;
            sal_Int32 nNumValue = seqValue.getLength( );
            for ( sal_Int32 i = 0; i < nNumValue; ++i )
            {
                values.push_back( OUSTR_TO_STDSTR( OUString::number( seqValue[i] ) ) );
            }
            type = libcmis::PropertyType::Decimal;
        }
        else if ( prop.Type == CMIS_TYPE_DATETIME )
        {
            uno::Sequence< util::DateTime > seqValue;
            value >>= seqValue;
            sal_Int32 nNumValue = seqValue.getLength( );
            for ( sal_Int32 i = 0; i < nNumValue; ++i )
            {
                OUStringBuffer aBuffer;
                ::sax::Converter::convertDateTime( aBuffer, seqValue[i], nullptr );
                values.push_back( OUSTR_TO_STDSTR( aBuffer.makeStringAndClear( )  ) );
            }
            type = libcmis::PropertyType::DateTime;
        }

        propertyType->setId( OUSTR_TO_STDSTR( id ));
        propertyType->setDisplayName( OUSTR_TO_STDSTR( name ) );
        propertyType->setUpdatable( bUpdatable );
        propertyType->setRequired( bRequired );
        propertyType->setMultiValued( bMultiValued );
        propertyType->setOpenChoice( bOpenChoice );
        propertyType->setType( type );

        libcmis::PropertyPtr property( new libcmis::Property( propertyType, values ) );

        return property;
    }

    uno::Sequence< uno::Any > generateErrorArguments( const cmis::URL & rURL )
    {
        uno::Sequence< uno::Any > aArguments(3);

        size_t i = 0;
        aArguments[i++] <<= beans::PropertyValue(
            "Binding URL",
            - 1,
            uno::makeAny( rURL.getBindingUrl() ),
            beans::PropertyState_DIRECT_VALUE );

        aArguments[i++] <<= beans::PropertyValue(
            "Username",
            -1,
            uno::makeAny( rURL.getUsername() ),
            beans::PropertyState_DIRECT_VALUE );

        aArguments[i++] <<= beans::PropertyValue(
            "Repository Id",
            -1,
            uno::makeAny( rURL.getRepositoryId() ),
            beans::PropertyState_DIRECT_VALUE );

        return aArguments;
    }
}

namespace cmis
{
    Content::Content( const uno::Reference< uno::XComponentContext >& rxContext,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        libcmis::ObjectPtr const & pObject )
        : ContentImplHelper( rxContext, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_pSession( nullptr ),
        m_pObject( pObject ),
        m_sURL( Identifier->getContentIdentifier( ) ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_bTransient( false ),
        m_bIsFolder( false )
    {
        SAL_INFO( "ucb.ucp.cmis", "Content::Content() " << m_sURL );

        m_sObjectPath = m_aURL.getObjectPath( );
        m_sObjectId = m_aURL.getObjectId( );
    }

    Content::Content( const uno::Reference< uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const uno::Reference< ucb::XContentIdentifier >& Identifier,
        bool bIsFolder )
        : ContentImplHelper( rxContext, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_pSession( nullptr ),
        m_sURL( Identifier->getContentIdentifier( ) ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_bTransient( true ),
        m_bIsFolder( bIsFolder )
    {
        SAL_INFO( "ucb.ucp.cmis", "Content::Content() " << m_sURL );

        m_sObjectPath = m_aURL.getObjectPath( );
        m_sObjectId = m_aURL.getObjectId( );
    }

    Content::~Content()
    {
    }

    libcmis::Session* Content::getSession( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
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

        // Look for a cached session, key is binding url + repo id
        OUString sSessionId = m_aURL.getBindingUrl( ) + m_aURL.getRepositoryId( );
        if ( nullptr == m_pSession )
            m_pSession = m_pProvider->getSession( sSessionId, m_aURL.getUsername( ) );

        if ( nullptr == m_pSession )
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

            // Set the SSL Validation handler
            libcmis::CertValidationHandlerPtr certHandler(
                    new CertValidationHandler( xEnv, m_xContext, aBindingUrl.GetHost( ) ) );
            libcmis::SessionFactory::setCertificateValidationHandler( certHandler );

            // Get the auth credentials
            AuthProvider aAuthProvider(xEnv, m_xIdentifier->getContentIdentifier(), m_aURL.getBindingUrl());
            AuthProvider::setXEnv( xEnv );

            string rUsername = OUSTR_TO_STDSTR( m_aURL.getUsername( ) );
            string rPassword = OUSTR_TO_STDSTR( m_aURL.getPassword( ) );

            bool bIsDone = false;

            while ( !bIsDone )
            {
                if (aAuthProvider.authenticationQuery(rUsername, rPassword))
                {
                    // Initiate a CMIS session and register it as we found nothing
                    libcmis::OAuth2DataPtr oauth2Data;
                    if ( m_aURL.getBindingUrl( ) == GDRIVE_BASE_URL )
                    {
                        libcmis::SessionFactory::setOAuth2AuthCodeProvider(AuthProvider::gdriveAuthCodeFallback);
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
                        libcmis::SessionFactory::setOAuth2AuthCodeProvider(AuthProvider::onedriveAuthCodeFallback);
                        oauth2Data.reset( new libcmis::OAuth2Data(
                            ONEDRIVE_AUTH_URL, ONEDRIVE_TOKEN_URL,
                            ONEDRIVE_SCOPE, ONEDRIVE_REDIRECT_URI,
                            ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET ) );
                    }
                    try
                    {
                        m_pSession = libcmis::SessionFactory::createSession(
                            OUSTR_TO_STDSTR( m_aURL.getBindingUrl( ) ),
                            rUsername, rPassword, OUSTR_TO_STDSTR( m_aURL.getRepositoryId( ) ), false, oauth2Data );

                        if ( m_pSession == nullptr )
                        {
                            // Fail: session was not created
                            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_INVALID_DEVICE,
                                generateErrorArguments(m_aURL),
                                xEnv);
                        }
                        else if ( m_pSession->getRepository() == nullptr )
                        {
                            // Fail: no repository or repository is invalid
                            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_INVALID_DEVICE,
                                generateErrorArguments(m_aURL),
                                xEnv,
                                "error accessing a repository");
                        }
                        else
                        {
                            m_pProvider->registerSession(sSessionId, m_aURL.getUsername( ), m_pSession);
                        }

                        bIsDone = true;
                    }
                    catch( const libcmis::Exception & e )
                    {
                        if ( e.getType().compare( "permissionDenied" ) != 0 )
                            throw;
                    }
                }
                else
                {
                    // Silently fail as the user cancelled the authentication
                    ucbhelper::cancelCommandExecution(
                                        ucb::IOErrorCode_ABORT,
                                        uno::Sequence< uno::Any >( 0 ),
                                        xEnv );
                    throw uno::RuntimeException( );
                }
            }
        }
        return m_pSession;
    }

    libcmis::ObjectTypePtr const & Content::getObjectType( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( nullptr == m_pObjectType.get( ) && m_bTransient )
        {
            string typeId = m_bIsFolder ? "cmis:folder" : "cmis:document";
            // The type to create needs to be fetched from the possible children types
            // defined in the parent folder. Then, we'll pick up the first one we find matching
            // cmis:folder or cmis:document (depending what we need to create).
            // The easy case will work in most cases, but not on some servers (like Lotus Live)
            libcmis::Folder* pParent = nullptr;
            bool bTypeRestricted = false;
            try
            {
                pParent = dynamic_cast< libcmis::Folder* >( getObject( xEnv ).get( ) );
            }
            catch ( const libcmis::Exception& )
            {
            }

            if ( pParent )
            {
                map< string, libcmis::PropertyPtr >& aProperties = pParent->getProperties( );
                map< string, libcmis::PropertyPtr >::iterator it = aProperties.find( "cmis:allowedChildObjectTypeIds" );
                if ( it != aProperties.end( ) )
                {
                    libcmis::PropertyPtr pProperty = it->second;
                    if ( pProperty )
                    {
                        vector< string > typesIds = pProperty->getStrings( );
                        for ( vector< string >::iterator typeIt = typesIds.begin();
                                typeIt != typesIds.end() && !m_pObjectType; ++typeIt )
                        {
                            bTypeRestricted = true;
                            libcmis::ObjectTypePtr type = getSession( xEnv )->getType( *typeIt );

                            // FIXME Improve performances by adding getBaseTypeId( ) method to libcmis
                            if ( type->getBaseType( )->getId( ) == typeId )
                                m_pObjectType = type;
                        }
                    }
                }
            }

            if ( !bTypeRestricted )
                m_pObjectType = getSession( xEnv )->getType( typeId );
        }
        return m_pObjectType;
    }


    libcmis::ObjectPtr const & Content::getObject( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        // can't get the session for some reason
        // the recent file opening at start up is an example.
        try
        {
            if ( !getSession( xEnv ) )
                return m_pObject;
        }
        catch ( uno::RuntimeException& )
        {
            return m_pObject;
        }
        if ( !m_pObject.get() )
        {
            if ( !m_sObjectId.isEmpty( ) )
            {
                try
                {
                    m_pObject = getSession( xEnv )->getObject( OUSTR_TO_STDSTR( m_sObjectId ) );
                }
                catch ( const libcmis::Exception& )
                {
                    throw libcmis::Exception( "Object not found" );
                }
            }
            else if (!(m_sObjectPath.isEmpty() || m_sObjectPath == "/"))
            {
                try
                {
                    m_pObject = getSession( xEnv )->getObjectByPath( OUSTR_TO_STDSTR( m_sObjectPath ) );
                }
                catch ( const libcmis::Exception& )
                {
                    // In some cases, getting the object from the path doesn't work,
                    // but getting the parent from its path and the get the child in the list is OK.
                    // It's weird, but needed to handle case where the path isn't the folders/files
                    // names separated by '/' (as in Lotus Live)
                    INetURLObject aParentUrl( m_sURL );
                    string sName = OUSTR_TO_STDSTR( aParentUrl.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset ) );
                    aParentUrl.removeSegment( );
                    OUString sParentUrl = aParentUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    // Avoid infinite recursion if sParentUrl == m_sURL
                    if (sParentUrl != m_sURL)
                    {
                        rtl::Reference<Content> xParent(new Content(m_xContext, m_pProvider, new ucbhelper::ContentIdentifier(sParentUrl)));
                        libcmis::FolderPtr pParentFolder = boost::dynamic_pointer_cast< libcmis::Folder >(xParent->getObject(xEnv));
                        if (pParentFolder)
                        {
                            vector< libcmis::ObjectPtr > children = pParentFolder->getChildren();
                            for (vector< libcmis::ObjectPtr >::iterator it = children.begin();
                                it != children.end() && !m_pObject; ++it)
                            {
                                if ((*it)->getName() == sName)
                                    m_pObject = *it;
                            }
                        }
                    }

                    if ( !m_pObject )
                        throw libcmis::Exception( "Object not found" );
                }
            }
            else
            {
                m_pObject = getSession( xEnv )->getRootFolder( );
                m_sObjectPath = "/";
                m_sObjectId = OUString( );
            }
        }

        return m_pObject;
    }

    bool Content::isFolder(const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        bool bIsFolder = false;
        try
        {
            libcmis::ObjectPtr obj = getObject( xEnv );
            if ( obj )
                bIsFolder = obj->getBaseType( ) == "cmis:folder";
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );

            ucbhelper::cancelCommandExecution(
                            ucb::IOErrorCode_GENERAL,
                            uno::Sequence< uno::Any >( 0 ),
                            xEnv,
                            OUString::createFromAscii( e.what( ) ) );

        }
        return bIsFolder;
    }

    uno::Any Content::getBadArgExcept()
    {
        return uno::makeAny( lang::IllegalArgumentException(
            "Wrong argument type!",
            static_cast< cppu::OWeakObject * >( this ), -1) );
    }

    libcmis::ObjectPtr Content::updateProperties(
         const uno::Any& iCmisProps,
         const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        // Convert iCmisProps to Cmis Properties;
        uno::Sequence< document::CmisProperty > aPropsSeq;
        iCmisProps >>= aPropsSeq;
        map< string, libcmis::PropertyPtr > aProperties;

        sal_Int32 propsLen = aPropsSeq.getLength( );
        for ( sal_Int32 i = 0; i< propsLen; i++ )
        {
            std::string id = OUSTR_TO_STDSTR( aPropsSeq[i].Id );
            libcmis::PropertyPtr prop = lcl_unoToCmisProperty( aPropsSeq[i] );
            aProperties.insert( std::pair<string, libcmis::PropertyPtr>( id, prop ) );
        }
        libcmis::ObjectPtr updateObj;
        try
        {
            updateObj = getObject( xEnv )->updateProperties( aProperties );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: "<< e.what( ) );
        }

        return updateObj;
    }

    uno::Reference< sdbc::XRow > Content::getPropertyValues(
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
                    try
                    {
                        libcmis::ObjectPtr obj = getObject( xEnv );
                        if ( obj )
                            xRow->appendBoolean( rProp, obj->getBaseType( ) == "cmis:document" );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        if ( m_pObjectType.get( ) )
                            xRow->appendBoolean( rProp, getObjectType( xEnv )->getBaseType()->getId( ) == "cmis:document" );
                        else
                            xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "IsFolder" )
                {
                    try
                    {
                        libcmis::ObjectPtr obj = getObject( xEnv );
                        if ( obj )
                            xRow->appendBoolean( rProp, obj->getBaseType( ) == "cmis:folder" );
                        else
                            xRow->appendBoolean( rProp, false );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        if ( m_pObjectType.get( ) )
                            xRow->appendBoolean( rProp, getObjectType( xEnv )->getBaseType()->getId( ) == "cmis:folder" );
                        else
                            xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "Title" )
                {
                    OUString sTitle;
                    try
                    {
                        sTitle = STD_TO_OUSTR( getObject( xEnv )->getName() );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        if ( !m_pObjectProps.empty() )
                        {
                            map< string, libcmis::PropertyPtr >::iterator it = m_pObjectProps.find( "cmis:name" );
                            if ( it != m_pObjectProps.end( ) )
                            {
                                vector< string > values = it->second->getStrings( );
                                if ( !values.empty() )
                                    sTitle = STD_TO_OUSTR( values.front( ) );
                            }
                        }
                    }

                    // Nothing worked... get it from the path
                    if ( sTitle.isEmpty( ) )
                    {
                        OUString sPath = m_sObjectPath;

                        // Get rid of the trailing slash problem
                        if ( sPath.endsWith("/") )
                            sPath = sPath.copy( 0, sPath.getLength() - 1 );

                        // Get the last segment
                        sal_Int32 nPos = sPath.lastIndexOf( '/' );
                        if ( nPos >= 0 )
                            sTitle = sPath.copy( nPos + 1 );
                    }

                    if ( !sTitle.isEmpty( ) )
                        xRow->appendString( rProp, sTitle );
                    else
                        xRow->appendVoid( rProp );
                }
                else if ( rProp.Name == "ObjectId" )
                {
                    OUString sId;
                    try
                    {
                        sId = STD_TO_OUSTR( getObject( xEnv )->getId() );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        if ( !m_pObjectProps.empty() )
                        {
                            map< string, libcmis::PropertyPtr >::iterator it = m_pObjectProps.find( "cmis:objectId" );
                            if ( it != m_pObjectProps.end( ) )
                            {
                                vector< string > values = it->second->getStrings( );
                                if ( !values.empty() )
                                    sId = STD_TO_OUSTR( values.front( ) );
                            }
                        }
                    }

                    if ( !sId.isEmpty( ) )
                        xRow->appendString( rProp, sId );
                    else
                        xRow->appendVoid( rProp );
                }
                else if ( rProp.Name == "TitleOnServer" )
                {
                    xRow->appendString( rProp, m_sObjectPath);
                }
                else if ( rProp.Name == "IsReadOnly" )
                {
                    boost::shared_ptr< libcmis::AllowableActions > allowableActions = getObject( xEnv )->getAllowableActions( );
                    bool bReadOnly = false;
                    if ( !allowableActions->isAllowed( libcmis::ObjectAction::SetContentStream ) &&
                         !allowableActions->isAllowed( libcmis::ObjectAction::CheckIn ) )
                        bReadOnly = true;

                    xRow->appendBoolean( rProp, bReadOnly );
                }
                else if ( rProp.Name == "DateCreated" )
                {
                    util::DateTime aTime = lcl_boostToUnoTime( getObject( xEnv )->getCreationDate( ) );
                    xRow->appendTimestamp( rProp, aTime );
                }
                else if ( rProp.Name == "DateModified" )
                {
                    util::DateTime aTime = lcl_boostToUnoTime( getObject( xEnv )->getLastModificationDate( ) );
                    xRow->appendTimestamp( rProp, aTime );
                }
                else if ( rProp.Name == "Size" )
                {
                    try
                    {
                        libcmis::Document* document = dynamic_cast< libcmis::Document* >( getObject( xEnv ).get( ) );
                        if ( nullptr != document )
                            xRow->appendLong( rProp, document->getContentLength() );
                        else
                            xRow->appendVoid( rProp );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CreatableContentsInfo" )
                {
                    xRow->appendObject( rProp, uno::makeAny( queryCreatableContentsInfo( xEnv ) ) );
                }
                else if ( rProp.Name == "MediaType" )
                {
                    try
                    {
                        libcmis::Document* document = dynamic_cast< libcmis::Document* >( getObject( xEnv ).get( ) );
                        if ( nullptr != document )
                            xRow->appendString( rProp, STD_TO_OUSTR( document->getContentType() ) );
                        else
                            xRow->appendVoid( rProp );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "IsVolume" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsRemote" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsRemoveable" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsFloppy" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsCompactDisc" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsHidden" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "TargetURL" )
                {
                    xRow->appendString( rProp, "" );
                }
                else if ( rProp.Name == "BaseURI" )
                {
                    xRow->appendString( rProp, m_aURL.getBindingUrl( ) );
                }
                else if ( rProp.Name == "CmisProperties" )
                {
                    try
                    {
                        libcmis::ObjectPtr object = getObject( xEnv );
                        map< string, libcmis::PropertyPtr >& aProperties = object->getProperties( );
                        uno::Sequence< document::CmisProperty > aCmisProperties( aProperties.size( ) );
                        document::CmisProperty* pCmisProps = aCmisProperties.getArray( );
                        sal_Int32 i = 0;
                        for ( map< string, libcmis::PropertyPtr >::iterator it = aProperties.begin();
                                it != aProperties.end( ); ++it, ++i )
                        {
                            string sId = it->first;
                            string sDisplayName =  it->second->getPropertyType()->getDisplayName( );
                            bool bUpdatable = it->second->getPropertyType()->isUpdatable( );
                            bool bRequired = it->second->getPropertyType()->isRequired( );
                            bool bMultiValued = it->second->getPropertyType()->isMultiValued();
                            bool bOpenChoice = it->second->getPropertyType()->isOpenChoice();

                            pCmisProps[i].Id = STD_TO_OUSTR( sId );
                            pCmisProps[i].Name = STD_TO_OUSTR( sDisplayName );
                            pCmisProps[i].Updatable = bUpdatable;
                            pCmisProps[i].Required = bRequired;
                            pCmisProps[i].MultiValued = bMultiValued;
                            pCmisProps[i].OpenChoice = bOpenChoice;
                            pCmisProps[i].Value = lcl_cmisPropertyToUno( it->second );
                            switch ( it->second->getPropertyType( )->getType( ) )
                            {
                                default:
                                case libcmis::PropertyType::String:
                                    pCmisProps[i].Type = CMIS_TYPE_STRING;
                                break;
                                case libcmis::PropertyType::Integer:
                                    pCmisProps[i].Type = CMIS_TYPE_INTEGER;
                                break;
                                case libcmis::PropertyType::Decimal:
                                    pCmisProps[i].Type = CMIS_TYPE_DECIMAL;
                                break;
                                case libcmis::PropertyType::Bool:
                                    pCmisProps[i].Type = CMIS_TYPE_BOOL;
                                break;
                                case libcmis::PropertyType::DateTime:
                                    pCmisProps[i].Type = CMIS_TYPE_DATETIME;
                                break;
                            }

                        }
                        xRow->appendObject( rProp.Name, uno::makeAny( aCmisProperties ) );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "IsVersionable" )
                {
                    try
                    {
                        libcmis::ObjectPtr object = getObject( xEnv );
                        bool bIsVersionable = object->getTypeDescription( )->isVersionable( );
                        xRow->appendBoolean( rProp, bIsVersionable );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CanCheckOut" )
                {
                    try
                    {
                        libcmis::ObjectPtr pObject = getObject( xEnv );
                        libcmis::AllowableActionsPtr aAllowables = pObject->getAllowableActions( );
                        bool bAllowed = false;
                        if ( aAllowables )
                        {
                            bAllowed = aAllowables->isAllowed( libcmis::ObjectAction::CheckOut );
                        }
                        xRow->appendBoolean( rProp, bAllowed );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CanCancelCheckOut" )
                {
                    try
                    {
                        libcmis::ObjectPtr pObject = getObject( xEnv );
                        libcmis::AllowableActionsPtr aAllowables = pObject->getAllowableActions( );
                        bool bAllowed = false;
                        if ( aAllowables )
                        {
                            bAllowed = aAllowables->isAllowed( libcmis::ObjectAction::CancelCheckOut );
                        }
                        xRow->appendBoolean( rProp, bAllowed );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CanCheckIn" )
                {
                    try
                    {
                        libcmis::ObjectPtr pObject = getObject( xEnv );
                        libcmis::AllowableActionsPtr aAllowables = pObject->getAllowableActions( );
                        bool bAllowed = false;
                        if ( aAllowables )
                        {
                            bAllowed = aAllowables->isAllowed( libcmis::ObjectAction::CheckIn );
                        }
                        xRow->appendBoolean( rProp, bAllowed );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else
                    SAL_INFO( "ucb.ucp.cmis", "Looking for unsupported property " << rProp.Name );
            }
            catch (const libcmis::Exception&)
            {
                xRow->appendVoid( rProp );
            }
        }

        return uno::Reference< sdbc::XRow >( xRow.get() );
    }

    uno::Any Content::open(const ucb::OpenCommandArgument2 & rOpenCommand,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        bool bIsFolder = isFolder( xEnv );

        // Handle the case of the non-existing file
        if ( !getObject( xEnv ) )
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= m_xIdentifier->getContentIdentifier();
            uno::Any aErr = uno::makeAny(
                ucb::InteractiveAugmentedIOException(OUString(), static_cast< cppu::OWeakObject * >( this ),
                    task::InteractionClassification_ERROR,
                    bIsFolder ? ucb::IOErrorCode_NOT_EXISTING_PATH : ucb::IOErrorCode_NOT_EXISTING, aArgs)
            );

            ucbhelper::cancelCommandExecution(aErr, xEnv);
        }

        uno::Any aRet;

        bool bOpenFolder = (
            ( rOpenCommand.Mode == ucb::OpenMode::ALL ) ||
            ( rOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
            ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENTS )
         );

        if ( bOpenFolder && bIsFolder )
        {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(m_xContext, this, rOpenCommand, xEnv );
            aRet <<= xSet;
        }
        else if ( rOpenCommand.Sink.is() )
        {
            if (
                ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE )
               )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny ( ucb::UnsupportedOpenModeException
                        ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                          sal_Int16( rOpenCommand.Mode ) ) ),
                        xEnv );
            }

            if ( !feedSink( rOpenCommand.Sink, xEnv ) )
            {
                // Note: rOpenCommand.Sink may contain an XStream
                //       implementation. Support for this type of
                //       sink is optional...
                SAL_INFO( "ucb.ucp.cmis", "Failed to copy data to sink" );

                ucbhelper::cancelCommandExecution(
                    uno::makeAny (ucb::UnsupportedDataSinkException
                        ( OUString(), static_cast< cppu::OWeakObject * >( this ),
                          rOpenCommand.Sink ) ),
                        xEnv );
            }
        }
        else
            SAL_INFO( "ucb.ucp.cmis", "Open falling through ..." );

        return aRet;
    }

    OUString Content::checkIn( const ucb::CheckinArgument& rArg,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        ucbhelper::Content aSourceContent( rArg.SourceURL, xEnv, comphelper::getProcessComponentContext( ) );
        uno::Reference< io::XInputStream > xIn = aSourceContent.openStream( );

        libcmis::ObjectPtr object;
        try
        {
            object = getObject( xEnv );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                OUString::createFromAscii( e.what() ) );
        }

        libcmis::Document* pPwc = dynamic_cast< libcmis::Document* >( object.get( ) );
        if ( !pPwc )
        {
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                "Checkin only supported by documents" );
        }

        boost::shared_ptr< ostream > pOut( new ostringstream ( ios_base::binary | ios_base::in | ios_base::out ) );
        uno::Reference < io::XOutputStream > xOutput = new StdOutputStream( pOut );
        copyData( xIn, xOutput );

        map< string, libcmis::PropertyPtr > newProperties;
        libcmis::DocumentPtr pDoc;

        try
        {
            pDoc = pPwc->checkIn( rArg.MajorVersion, OUSTR_TO_STDSTR( rArg.VersionComment ), newProperties,
                                  pOut, OUSTR_TO_STDSTR( rArg.MimeType ), OUSTR_TO_STDSTR( rArg.NewTitle ) );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                OUString::createFromAscii( e.what() ) );
        }

        // Get the URL and send it back as a result
        URL aCmisUrl( m_sURL );
        vector< string > aPaths = pDoc->getPaths( );
        if ( !aPaths.empty() )
        {
            string sPath = aPaths.front( );
            aCmisUrl.setObjectPath( STD_TO_OUSTR( sPath ) );
        }
        else
        {
            // We may have unfiled document depending on the server, those
            // won't have any path, use their ID instead
            string sId = pDoc->getId( );
            aCmisUrl.setObjectId( STD_TO_OUSTR( sId ) );
        }
        return aCmisUrl.asString( );
    }

    OUString Content::checkOut( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        OUString aRet;
        try
        {
            // Checkout the document if possible
            libcmis::DocumentPtr pDoc = boost::dynamic_pointer_cast< libcmis::Document >( getObject( xEnv ) );
            if ( pDoc.get( ) == nullptr )
            {
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_GENERAL,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    "Checkout only supported by documents" );
            }
            libcmis::DocumentPtr pPwc = pDoc->checkOut( );

            // Compute the URL of the Private Working Copy (PWC)
            URL aCmisUrl( m_sURL );
            vector< string > aPaths = pPwc->getPaths( );
            if ( !aPaths.empty() )
            {
                string sPath = aPaths.front( );
                aCmisUrl.setObjectPath( STD_TO_OUSTR( sPath ) );
            }
            else
            {
                // We may have unfiled PWC depending on the server, those
                // won't have any path, use their ID instead
                string sId = pPwc->getId( );
                aCmisUrl.setObjectId( STD_TO_OUSTR( sId ) );
            }
            aRet = aCmisUrl.asString( );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                o3tl::runtimeToOUString(e.what()));
        }
        return aRet;
    }

    OUString Content::cancelCheckOut( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        OUString aRet;
        try
        {
            libcmis::DocumentPtr pPwc = boost::dynamic_pointer_cast< libcmis::Document >( getObject( xEnv ) );
            if ( pPwc.get( ) == nullptr )
            {
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_GENERAL,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    "CancelCheckout only supported by documents" );
            }
            pPwc->cancelCheckout( );

            // Get the Original document (latest version)
            vector< libcmis::DocumentPtr > aVersions = pPwc->getAllVersions( );
            bool bFound = false;
            for ( vector< libcmis::DocumentPtr >::iterator it = aVersions.begin();
                    it != aVersions.end( ) && !bFound; ++it )
            {
                libcmis::DocumentPtr pVersion = *it;
                map< string, libcmis::PropertyPtr > aProps = pVersion->getProperties( );
                bool bIsLatestVersion = false;
                map< string, libcmis::PropertyPtr >::iterator propIt = aProps.find( string( "cmis:isLatestVersion" ) );
                if ( propIt != aProps.end( ) && !propIt->second->getBools( ).empty( ) )
                {
                    bIsLatestVersion = propIt->second->getBools( ).front( );
                }

                if ( bIsLatestVersion )
                {
                    bFound = true;
                    // Compute the URL of the Document
                    URL aCmisUrl( m_sURL );
                    vector< string > aPaths = pVersion->getPaths( );
                    if ( !aPaths.empty() )
                    {
                        string sPath = aPaths.front( );
                        aCmisUrl.setObjectPath( STD_TO_OUSTR( sPath ) );
                    }
                    else
                    {
                        // We may have unfiled doc depending on the server, those
                        // won't have any path, use their ID instead
                        string sId = pVersion->getId( );
                        aCmisUrl.setObjectId( STD_TO_OUSTR( sId ) );
                    }
                    aRet = aCmisUrl.asString( );
                }
            }
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                o3tl::runtimeToOUString(e.what()));
        }
        return aRet;
    }

    uno::Sequence< document::CmisVersion> Content::getAllVersions( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        try
        {
            // get the document
            libcmis::DocumentPtr pDoc = boost::dynamic_pointer_cast< libcmis::Document >( getObject( xEnv ) );
            if ( pDoc.get( ) == nullptr )
            {
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_GENERAL,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    "Can not get the document" );
            }
            vector< libcmis::DocumentPtr > aCmisVersions = pDoc->getAllVersions( );
            uno::Sequence< document::CmisVersion > aVersions( aCmisVersions.size( ) );
            int i = 0;
            for ( vector< libcmis::DocumentPtr >::iterator it = aCmisVersions.begin();
                    it != aCmisVersions.end( ); ++it, ++i )
            {
                libcmis::DocumentPtr pVersion = *it;
                aVersions[i].Id = STD_TO_OUSTR( pVersion->getId( ) );
                aVersions[i].Author = STD_TO_OUSTR( pVersion->getCreatedBy( ) );
                aVersions[i].TimeStamp = lcl_boostToUnoTime( pVersion->getLastModificationDate( ) );
                aVersions[i].Comment = STD_TO_OUSTR( pVersion->getStringProperty("cmis:checkinComment") );
            }
            return aVersions;
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_GENERAL,
                    uno::Sequence< uno::Any >( 0 ),
                    xEnv,
                    o3tl::runtimeToOUString(e.what()));
        }
        return uno::Sequence< document::CmisVersion > ( );
    }

    void Content::transfer( const ucb::TransferInfo& rTransferInfo,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        // If the source isn't on the same CMIS repository, then simply copy
        INetURLObject aSourceUrl( rTransferInfo.SourceURL );
        if ( aSourceUrl.GetProtocol() != INetProtocol::Cmis )
        {
            OUString sSrcBindingUrl = URL( rTransferInfo.SourceURL ).getBindingUrl( );
            if ( sSrcBindingUrl != m_aURL.getBindingUrl( ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::InteractiveBadTransferURLException(
                            "Unsupported URL scheme!",
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                    xEnv );
            }
        }

        SAL_INFO( "ucb.ucp.cmis", "TODO - Content::transfer()" );
    }

    void Content::insert( const uno::Reference< io::XInputStream > & xInputStream,
        bool bReplaceExisting, const OUString& rMimeType,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( !xInputStream.is() )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny
                ( ucb::MissingInputStreamException
                  ( OUString(), static_cast< cppu::OWeakObject * >( this ) ) ),
                xEnv );
        }

        // For transient content, the URL is the one of the parent
        if ( m_bTransient )
        {
            OUString sNewPath;

            // Try to get the object from the server if there is any
            libcmis::FolderPtr pFolder;
            try
            {
                pFolder = boost::dynamic_pointer_cast< libcmis::Folder >( getObject( xEnv ) );
            }
            catch ( const libcmis::Exception& )
            {
            }

            if ( pFolder != nullptr )
            {
                libcmis::ObjectPtr object;
                map< string, libcmis::PropertyPtr >::iterator it = m_pObjectProps.find( "cmis:name" );
                if ( it == m_pObjectProps.end( ) )
                {
                    ucbhelper::cancelCommandExecution( uno::makeAny
                        ( uno::RuntimeException( "Missing name property",
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                        xEnv );
                }
                string newName = it->second->getStrings( ).front( );
                string newPath = OUSTR_TO_STDSTR( m_sObjectPath );
                if ( !newPath.empty( ) && newPath[ newPath.size( ) - 1 ] != '/' )
                    newPath += "/";
                newPath += newName;
                try
                {
                    if ( !m_sObjectId.isEmpty( ) )
                        object = getSession( xEnv )->getObject( OUSTR_TO_STDSTR( m_sObjectId) );
                    else
                        object = getSession( xEnv )->getObjectByPath( newPath );
                    sNewPath = STD_TO_OUSTR( newPath );
                }
                catch ( const libcmis::Exception& )
                {
                    // Nothing matched the path
                }

                if ( nullptr != object.get( ) )
                {
                    // Are the base type matching?
                    if ( object->getBaseType( ) != m_pObjectType->getBaseType( )->getId() )
                    {
                        ucbhelper::cancelCommandExecution( uno::makeAny
                            ( uno::RuntimeException( "Can't change a folder into a document and vice-versa.",
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                            xEnv );
                    }

                    // Update the existing object if it's a document
                    libcmis::Document* document = dynamic_cast< libcmis::Document* >( object.get( ) );
                    if ( nullptr != document )
                    {
                        boost::shared_ptr< ostream > pOut( new ostringstream ( ios_base::binary | ios_base::in | ios_base::out ) );
                        uno::Reference < io::XOutputStream > xOutput = new StdOutputStream( pOut );
                        copyData( xInputStream, xOutput );
                        try
                        {
                            document->setContentStream( pOut, OUSTR_TO_STDSTR( rMimeType ), string( ), bReplaceExisting );
                        }
                        catch ( const libcmis::Exception& )
                        {
                            ucbhelper::cancelCommandExecution( uno::makeAny
                                ( uno::RuntimeException( "Error when setting document content",
                                    static_cast< cppu::OWeakObject * >( this ) ) ),
                                xEnv );
                        }
                    }
                }
                else
                {
                    // We need to create a brand new object... either folder or document
                    bool bIsFolder = getObjectType( xEnv )->getBaseType( )->getId( ) == "cmis:folder";
                    setCmisProperty( "cmis:objectTypeId", getObjectType( xEnv )->getId( ), xEnv );

                    if ( bIsFolder )
                    {
                        try
                        {
                            libcmis::FolderPtr pNew = pFolder->createFolder( m_pObjectProps );
                            sNewPath = STD_TO_OUSTR( newPath );
                        }
                        catch ( const libcmis::Exception& )
                        {
                            ucbhelper::cancelCommandExecution( uno::makeAny
                                ( uno::RuntimeException( "Error when creating folder",
                                    static_cast< cppu::OWeakObject * >( this ) ) ),
                                xEnv );
                        }
                    }
                    else
                    {
                        boost::shared_ptr< ostream > pOut( new ostringstream ( ios_base::binary | ios_base::in | ios_base::out ) );
                        uno::Reference < io::XOutputStream > xOutput = new StdOutputStream( pOut );
                        copyData( xInputStream, xOutput );
                        try
                        {
                            pFolder->createDocument( m_pObjectProps, pOut, OUSTR_TO_STDSTR( rMimeType ), string() );
                            sNewPath = STD_TO_OUSTR( newPath );
                        }
                        catch ( const libcmis::Exception& )
                        {
                            ucbhelper::cancelCommandExecution( uno::makeAny
                                ( uno::RuntimeException( "Error when creating document",
                                    static_cast< cppu::OWeakObject * >( this ) ) ),
                                xEnv );
                        }
                    }
                }

                if ( !sNewPath.isEmpty( ) || !m_sObjectId.isEmpty( ) )
                {
                    // Update the current content: it's no longer transient
                    m_sObjectPath = sNewPath;
                    URL aUrl( m_sURL );
                    aUrl.setObjectPath( m_sObjectPath );
                    aUrl.setObjectId( m_sObjectId );
                    m_sURL = aUrl.asString( );
                    m_pObject.reset( );
                    m_pObjectType.reset( );
                    m_pObjectProps.clear( );
                    m_bTransient = false;
                    inserted();
                }
            }
        }
    }

    const int TRANSFER_BUFFER_SIZE = 65536;

    void Content::copyData(
        const uno::Reference< io::XInputStream >& xIn,
        const uno::Reference< io::XOutputStream >& xOut )
    {
        uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

        while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
            xOut->writeBytes( theData );

        xOut->closeOutput();
    }

    uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        try
        {
            // Get the already set properties if possible
            if ( !m_bTransient && getObject( xEnv ).get( ) )
            {
                m_pObjectProps.clear( );
                m_pObjectType = getObject( xEnv )->getTypeDescription();
            }
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                o3tl::runtimeToOUString(e.what()));
        }

        sal_Int32 nCount = rValues.getLength();
        uno::Sequence< uno::Any > aRet( nCount );

        bool bChanged = false;
        const beans::PropertyValue* pValues = rValues.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::PropertyValue& rValue = pValues[ n ];
            if ( rValue.Name == "ContentType" ||
                 rValue.Name == "MediaType" ||
                 rValue.Name == "IsDocument" ||
                 rValue.Name == "IsFolder" ||
                 rValue.Name == "Size" ||
                 rValue.Name == "CreatableContentsInfo" )
            {
                lang::IllegalAccessException e ( "Property is read-only!",
                       static_cast< cppu::OWeakObject* >( this ) );
                aRet[ n ] <<= e;
            }
            else if ( rValue.Name == "Title" )
            {
                OUString aNewTitle;
                if (!( rValue.Value >>= aNewTitle ))
                {
                    aRet[ n ] <<= beans::IllegalTypeException
                        ( "Property value has wrong type!",
                          static_cast< cppu::OWeakObject * >( this ) );
                    continue;
                }

                if ( aNewTitle.getLength() <= 0 )
                {
                    aRet[ n ] <<= lang::IllegalArgumentException
                        ( "Empty title not allowed!",
                          static_cast< cppu::OWeakObject * >( this ), -1 );
                    continue;

                }

                setCmisProperty( "cmis:name", OUSTR_TO_STDSTR( aNewTitle ), xEnv );
                bChanged = true;
            }
            else
            {
                SAL_INFO( "ucb.ucp.cmis", "Couldn't set property: " << rValue.Name );
                lang::IllegalAccessException e ( "Property is read-only!",
                       static_cast< cppu::OWeakObject* >( this ) );
                aRet[ n ] <<= e;
            }
        }

        try
        {
            if ( !m_bTransient && bChanged )
            {
                getObject( xEnv )->updateProperties( m_pObjectProps );
            }
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                o3tl::runtimeToOUString(e.what()));
        }

        return aRet;
    }

    bool Content::feedSink( const uno::Reference< uno::XInterface>& xSink,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( !xSink.is() )
            return false;

        uno::Reference< io::XOutputStream > xOut(xSink, uno::UNO_QUERY );
        uno::Reference< io::XActiveDataSink > xDataSink(xSink, uno::UNO_QUERY );
        uno::Reference< io::XActiveDataStreamer > xDataStreamer( xSink, uno::UNO_QUERY );

        if ( !xOut.is() && !xDataSink.is() && ( !xDataStreamer.is() || !xDataStreamer->getStream().is() ) )
            return false;

        if ( xDataStreamer.is() && !xOut.is() )
            xOut = xDataStreamer->getStream()->getOutputStream();

        try
        {
            libcmis::Document* document = dynamic_cast< libcmis::Document* >( getObject( xEnv ).get() );

            if (!document)
                return false;

            boost::shared_ptr< istream > aIn = document->getContentStream( );

            uno::Reference< io::XInputStream > xIn = new StdInputStream( aIn );
            if( !xIn.is( ) )
                return false;

            if ( xDataSink.is() )
                xDataSink->setInputStream( xIn );
            else if ( xOut.is() )
                copyData( xIn, xOut );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                o3tl::runtimeToOUString(e.what()));
        }

        return true;
    }

    uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & )
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
            beans::Property( "ObjectId",
                -1, cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( "TitleOnServer",
                -1, cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( "IsReadOnly",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "DateCreated",
                -1, cppu::UnoType<util::DateTime>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "DateModified",
                -1, cppu::UnoType<util::DateTime>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "Size",
                -1, cppu::UnoType<sal_Int64>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "CreatableContentsInfo",
                -1, cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "MediaType",
                -1, cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( "CmisProperties",
                -1, cppu::UnoType<uno::Sequence< document::CmisProperty>>::get(),
                beans::PropertyAttribute::BOUND ),
            beans::Property( "IsVersionable",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "CanCheckOut",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "CanCancelCheckOut",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( "CanCheckIn",
                -1, cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        };

        const int nProps = SAL_N_ELEMENTS(aGenericProperties);
        return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
    }

    uno::Sequence< ucb::CommandInfo > Content::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
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
            ( "delete",
              -1, cppu::UnoType<bool>::get() ),
            ucb::CommandInfo
            ( "insert",
              -1, cppu::UnoType<ucb::InsertCommandArgument2>::get() ),
            ucb::CommandInfo
            ( "open",
              -1, cppu::UnoType<ucb::OpenCommandArgument2>::get() ),

            // Mandatory CMIS-only commands
            ucb::CommandInfo ( "checkout", -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo ( "cancelCheckout", -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo ( "checkIn", -1,
                    cppu::UnoType<ucb::TransferInfo>::get() ),
            ucb::CommandInfo ( "updateProperties", -1, cppu::UnoType<void>::get() ),
            ucb::CommandInfo
            ( "getAllVersions",
              -1, cppu::UnoType<uno::Sequence< document::CmisVersion >>::get() ),


            // Folder Only, omitted if not a folder
            ucb::CommandInfo
            ( "transfer",
              -1, cppu::UnoType<ucb::TransferInfo>::get() ),
            ucb::CommandInfo
            ( "createNewContent",
              -1, cppu::UnoType<ucb::ContentInfo>::get() )
        };

        const int nProps = SAL_N_ELEMENTS( aCommandInfoTable );
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, isFolder( xEnv ) ? nProps : nProps - 2);
    }

    OUString Content::getParentURL( )
    {
        SAL_INFO( "ucb.ucp.cmis", "Content::getParentURL()" );
        OUString parentUrl = "/";
        if ( m_sObjectPath == "/" )
            return parentUrl;
        else
        {
            INetURLObject aUrl( m_sURL );
            if ( aUrl.getSegmentCount( ) > 0 )
            {
                URL aCmisUrl( m_sURL );
                aUrl.removeSegment( );
                aCmisUrl.setObjectPath( aUrl.GetURLPath( INetURLObject::DecodeMechanism::WithCharset ) );
                parentUrl = aCmisUrl.asString( );
            }
        }
        return parentUrl;
    }

    XTYPEPROVIDER_COMMON_IMPL( Content );

    void SAL_CALL Content::acquire() throw()
    {
        ContentImplHelper::acquire();
    }

    void SAL_CALL Content::release() throw()
    {
        ContentImplHelper::release();
    }

    uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    {
        uno::Any aRet = cppu::queryInterface( rType, static_cast< ucb::XContentCreator * >( this ) );
        return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface(rType);
    }

    OUString SAL_CALL Content::getImplementationName()
    {
       return OUString("com.sun.star.comp.CmisContent");
    }

    uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    {
           uno::Sequence<OUString> aSNS { "com.sun.star.ucb.CmisContent" };
           return aSNS;
    }

    OUString SAL_CALL Content::getContentType()
    {
        OUString sRet;
        try
        {
            sRet = isFolder( uno::Reference< ucb::XCommandEnvironment >() )
                ? OUString(CMIS_FOLDER_TYPE)
                : OUString(CMIS_FILE_TYPE);
        }
        catch (const uno::RuntimeException&)
        {
            throw;
        }
        catch (const uno::Exception& e)
        {
            uno::Any a(cppu::getCaughtException());
            throw lang::WrappedTargetRuntimeException(
                "wrapped Exception " + e.Message,
                uno::Reference<uno::XInterface>(), a);
        }
        return sRet;
    }

    uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        SAL_INFO( "ucb.ucp.cmis", "Content::execute( ) - " << aCommand.Name );
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
            aRet = open( aOpenCommand, xEnv );
        }
        else if ( aCommand.Name == "transfer" )
        {
            ucb::TransferInfo transferArgs;
            if ( !( aCommand.Argument >>= transferArgs ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            transfer( transferArgs, xEnv );
        }
        else if ( aCommand.Name == "setPropertyValues" )
        {
            uno::Sequence< beans::PropertyValue > aProperties;
            if ( !( aCommand.Argument >>= aProperties ) || !aProperties.getLength() )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= setPropertyValues( aProperties, xEnv );
        }
        else if (aCommand.Name == "createNewContent"
                 && isFolder( xEnv ) )
        {
            ucb::ContentInfo arg;
            if ( !( aCommand.Argument >>= arg ) )
                    ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= createNewContent( arg );
        }
        else if ( aCommand.Name == "insert" )
        {
            ucb::InsertCommandArgument2 arg;
            if ( !( aCommand.Argument >>= arg ) )
            {
                ucb::InsertCommandArgument insertArg;
                if ( !( aCommand.Argument >>= insertArg ) )
                    ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );

                arg.Data = insertArg.Data;
                arg.ReplaceExisting = insertArg.ReplaceExisting;
            }
            // store the document id
            m_sObjectId = arg.DocumentId;
            insert( arg.Data, arg.ReplaceExisting, arg.MimeType, xEnv );
        }
        else if ( aCommand.Name == "delete" )
        {
            try
            {
                if ( !isFolder( xEnv ) )
                {
                    getObject( xEnv )->remove( );
                }
                else
                {
                    libcmis::Folder* folder = dynamic_cast< libcmis::Folder* >( getObject( xEnv ).get() );
                    if (folder)
                        folder->removeTree( );
                }
            }
            catch ( const libcmis::Exception& e )
            {
                SAL_INFO( "ucb.ucp.cmis", "Unexpected libcmis exception: " << e.what( ) );
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_GENERAL,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    o3tl::runtimeToOUString(e.what()));
            }
        }
        else if ( aCommand.Name == "checkout" )
        {
            aRet <<= checkOut( xEnv );
        }
        else if ( aCommand.Name == "cancelCheckout" )
        {
            aRet <<= cancelCheckOut( xEnv );
        }
        else if ( aCommand.Name == "checkin" )
        {
            ucb::CheckinArgument aArg;
            if ( !( aCommand.Argument >>= aArg ) )
            {
                ucbhelper::cancelCommandExecution ( getBadArgExcept(), xEnv );
            }
            aRet <<= checkIn( aArg, xEnv );
        }
        else if ( aCommand.Name == "getAllVersions" )
        {
            aRet <<= getAllVersions( xEnv );
        }
        else if ( aCommand.Name == "updateProperties" )
        {
            updateProperties( aCommand.Argument, xEnv );
        }
        else
        {
            SAL_INFO( "ucb.ucp.cmis", "Unknown command to execute" );

            ucbhelper::cancelCommandExecution
                ( uno::makeAny( ucb::UnsupportedCommandException
                  ( OUString(),
                    static_cast< cppu::OWeakObject * >( this ) ) ),
                  xEnv );
        }

        return aRet;
    }

    void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    {
        SAL_INFO( "ucb.ucp.cmis", "TODO - Content::abort()" );
        // TODO Implement me
    }

    uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
    {
        return queryCreatableContentsInfo( uno::Reference< ucb::XCommandEnvironment >() );
    }

    uno::Reference< ucb::XContent > SAL_CALL Content::createNewContent(
            const ucb::ContentInfo& Info )
    {
        bool create_document;

        if ( Info.Type == CMIS_FILE_TYPE )
            create_document = true;
        else if ( Info.Type == CMIS_FOLDER_TYPE )
            create_document = false;
        else
        {
            SAL_INFO( "ucb.ucp.cmis", "Unknown type of content to create" );
            return uno::Reference< ucb::XContent >();
        }

        OUString sParentURL = m_xIdentifier->getContentIdentifier();
        URL aParentURL( sParentURL );

        // Set the parent URL for the transient objects
        uno::Reference< ucb::XContentIdentifier > xId(new ::ucbhelper::ContentIdentifier(sParentURL));

        try
        {
            return new ::cmis::Content( m_xContext, m_pProvider, xId, !create_document );
        }
        catch ( ucb::ContentCreationException & )
        {
            return uno::Reference< ucb::XContent >();
        }
    }

    uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    {
        try
        {
            if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
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
                     CPPU_TYPE_REF( container::XChild ),
                     CPPU_TYPE_REF( ucb::XContentCreator ) );
                return aFolderCollection.getTypes();
            }
        }
        catch (const uno::RuntimeException&)
        {
            throw;
        }
        catch (const uno::Exception& e)
        {
            uno::Any a(cppu::getCaughtException());
            throw lang::WrappedTargetRuntimeException(
                "wrapped Exception " + e.Message,
                uno::Reference<uno::XInterface>(), a);
        }

        static cppu::OTypeCollection aFileCollection
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

        return aFileCollection.getTypes();
    }

    uno::Sequence< ucb::ContentInfo > Content::queryCreatableContentsInfo(
        const uno::Reference< ucb::XCommandEnvironment >& xEnv)
    {
        try
        {
            if ( isFolder( xEnv ) )
            {
                uno::Sequence< ucb::ContentInfo > seq(2);

                // Minimum set of props we really need
                uno::Sequence< beans::Property > props( 1 );
                props[0] = beans::Property(
                    "Title",
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::BOUND );

                // file
                seq[0].Type       =  CMIS_FILE_TYPE;
                seq[0].Attributes = ( ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                                      ucb::ContentInfoAttribute::KIND_DOCUMENT );
                seq[0].Properties = props;

                // folder
                seq[1].Type       = CMIS_FOLDER_TYPE;
                seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
                seq[1].Properties = props;

                return seq;
            }
        }
        catch (const uno::RuntimeException&)
        {
            throw;
        }
        catch (const uno::Exception& e)
        {
            uno::Any a(cppu::getCaughtException());
            throw lang::WrappedTargetRuntimeException(
                "wrapped Exception " + e.Message,
                uno::Reference<uno::XInterface>(), a);
        }
        return uno::Sequence< ucb::ContentInfo >();
    }

    list< uno::Reference< ucb::XContent > > Content::getChildren( )
    {
        list< uno::Reference< ucb::XContent > > results;
        SAL_INFO( "ucb.ucp.cmis", "Content::getChildren() " << m_sURL );

        libcmis::FolderPtr pFolder = boost::dynamic_pointer_cast< libcmis::Folder >( getObject( uno::Reference< ucb::XCommandEnvironment >() ) );
        if ( nullptr != pFolder )
        {
            // Get the children from pObject
            try
            {
                vector< libcmis::ObjectPtr > children = pFolder->getChildren( );

                // Loop over the results
                for ( vector< libcmis::ObjectPtr >::iterator it = children.begin();
                        it != children.end(); ++it )
                {
                    // TODO Cache the objects

                    INetURLObject aURL( m_sURL );
                    OUString sUser = aURL.GetUser( INetURLObject::DecodeMechanism::WithCharset );

                    URL aUrl( m_sURL );
                    OUString sPath( m_sObjectPath );
                    if ( !sPath.endsWith("/") )
                        sPath += "/";
                    sPath += STD_TO_OUSTR( ( *it )->getName( ) );
                    OUString sId = STD_TO_OUSTR( ( *it )->getId( ) );

                    aUrl.setObjectId( sId );
                    aUrl.setObjectPath( sPath );
                    aUrl.setUsername( sUser );

                    uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aUrl.asString( ) );
                    uno::Reference< ucb::XContent > xContent = new Content( m_xContext, m_pProvider, xId, *it );

                    results.push_back( xContent );
                }
            }
            catch ( const libcmis::Exception& e )
            {
                SAL_INFO( "ucb.ucp.cmis", "Exception thrown: " << e.what() );
            }
        }

        return results;
    }

    void Content::setCmisProperty(const std::string& rName, const std::string& rValue, const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( getObjectType( xEnv ).get( ) )
        {
            map< string, libcmis::PropertyPtr >::iterator propIt = m_pObjectProps.find(rName);
            vector< string > values;
            values.push_back(rValue);

            if ( propIt == m_pObjectProps.end( ) && getObjectType( xEnv ).get( ) )
            {
                map< string, libcmis::PropertyTypePtr > propsTypes = getObjectType( xEnv )->getPropertiesTypes( );
                map< string, libcmis::PropertyTypePtr >::iterator typeIt = propsTypes.find(rName);

                if ( typeIt != propsTypes.end( ) )
                {
                    libcmis::PropertyTypePtr propType = typeIt->second;
                    libcmis::PropertyPtr property( new libcmis::Property( propType, values ) );
                    m_pObjectProps.insert(pair< string, libcmis::PropertyPtr >(rName, property));
                }
            }
            else if ( propIt != m_pObjectProps.end( ) )
            {
                propIt->second->setValues( values );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
