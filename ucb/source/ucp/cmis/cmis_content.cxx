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

#include <cstdio>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
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
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>

#include <libcmis/document.hxx>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/std_inputstream.hxx>
#include <ucbhelper/std_outputstream.hxx>
#include <ucbhelper/propertyvalueset.hxx>

#include "auth_provider.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_resultset.hxx"

#define OUSTR_TO_STDSTR(s) string( rtl::OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) rtl::OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace
{
    util::DateTime lcl_boostToUnoTime( boost::posix_time::ptime boostTime )
    {
        util::DateTime unoTime;
        unoTime.Year = boostTime.date().year();
        unoTime.Month = boostTime.date().month();
        unoTime.Day = boostTime.date().day();
        unoTime.Hours = boostTime.time_of_day().hours();
        unoTime.Minutes = boostTime.time_of_day().minutes();
        unoTime.Seconds = boostTime.time_of_day().seconds();

        long total_milli = boostTime.time_of_day().total_milliseconds( );
        long milli = total_milli - boostTime.time_of_day().total_seconds( );
        long hundredthSeconds = milli / 10;

        unoTime.HundredthSeconds = hundredthSeconds;

        return unoTime;
    }

    uno::Any lcl_cmisPropertyToUno( libcmis::PropertyPtr pProperty )
    {
        uno::Any aValue;
        bool bMultiValued = pProperty->getPropertyType( )->isMultiValued( );
        switch ( pProperty->getPropertyType( )->getType( ) )
        {
            default:
            case libcmis::PropertyType::String:
                {
                    vector< string > aCmisStrings = pProperty->getStrings( );
                    if ( bMultiValued )
                    {
                        uno::Sequence< rtl::OUString > aStrings( aCmisStrings.size( ) );
                        rtl::OUString* aStringsArr = aStrings.getArray( );
                        sal_Int32 i = 0;
                        for ( vector< string >::iterator it = aCmisStrings.begin( );
                                it != aCmisStrings.end( ); ++it, ++i )
                        {
                            string str = *it;
                            aStringsArr[i] = STD_TO_OUSTR( str );
                        }
                        aValue <<= aStrings;
                    }
                    else if ( !aCmisStrings.empty( ) )
                    {
                        aValue <<= STD_TO_OUSTR( aCmisStrings.front( ) );
                    }
                }
                break;
            case libcmis::PropertyType::Integer:
                {
                    vector< long > aCmisLongs = pProperty->getLongs( );
                    if ( bMultiValued )
                    {
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
                    else if ( !aCmisLongs.empty( ) )
                    {
                        aValue <<= aCmisLongs.front( );
                    }
                }
                break;
            case libcmis::PropertyType::Decimal:
                {
                    vector< double > aCmisDoubles = pProperty->getDoubles( );
                    if ( bMultiValued )
                    {
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
                    else if ( !aCmisDoubles.empty( ) )
                    {
                        aValue <<= aCmisDoubles.front( );
                    }
                }
                break;
            case libcmis::PropertyType::Bool:
                {
                    vector< bool > aCmisBools = pProperty->getBools( );
                    if ( bMultiValued )
                    {
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
                    else if ( !aCmisBools.empty( ) )
                    {
                        aValue <<= sal_Bool( aCmisBools.front( ) );
                    }
                }
                break;
            case libcmis::PropertyType::DateTime:
                {
                    vector< boost::posix_time::ptime > aCmisTimes = pProperty->getDateTimes( );
                    if ( bMultiValued )
                    {
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
                    else if ( !aCmisTimes.empty( ) )
                    {
                        aValue <<= lcl_boostToUnoTime( aCmisTimes.front( ) );
                    }
                }
                break;
        }
        return aValue;
    }
}

namespace cmis
{
    Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier,
        libcmis::ObjectPtr pObject )
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxSMgr, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_pSession( NULL ),
        m_pObject( pObject ),
        m_sURL( Identifier->getContentIdentifier( ) ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_bTransient( false ),
        m_bIsFolder( false )
    {
        SAL_INFO( "cmisucp", "Content::Content() " << m_sURL );

        m_sObjectPath = m_aURL.getObjectPath( );
        m_sObjectId = m_aURL.getObjectId( );
    }

    Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr, ContentProvider *pProvider,
        const uno::Reference< ucb::XContentIdentifier >& Identifier,
        sal_Bool bIsFolder )
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxSMgr, pProvider, Identifier ),
        m_pProvider( pProvider ),
        m_pSession( NULL ),
        m_sURL( Identifier->getContentIdentifier( ) ),
        m_aURL( Identifier->getContentIdentifier( ) ),
        m_bTransient( true ),
        m_bIsFolder( bIsFolder )
    {
        SAL_INFO( "cmisucp", "Content::Content() " << m_sURL );

        m_sObjectPath = m_aURL.getObjectPath( );
        m_sObjectId = m_aURL.getObjectId( );
    }

    Content::~Content()
    {
    }

    libcmis::Session* Content::getSession( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        // Look for a cached session, key is binding url + repo id
        rtl::OUString sSessionId = m_aURL.getBindingUrl( ) + m_aURL.getRepositoryId( );
        if ( NULL == m_pSession )
            m_pSession = m_pProvider->getSession( sSessionId );

        if ( NULL == m_pSession )
        {
            // Get the auth credentials
            AuthProvider authProvider( xEnv, m_xIdentifier->getContentIdentifier( ), m_aURL.getBindingUrl( ) );

            string rUsername = OUSTR_TO_STDSTR( m_aURL.getUsername( ) );
            string rPassword = OUSTR_TO_STDSTR( m_aURL.getPassword( ) );
            if ( authProvider.authenticationQuery( rUsername, rPassword ) )
            {
                // Initiate a CMIS session and register it as we found nothing
                m_pSession = libcmis::SessionFactory::createSession(
                        OUSTR_TO_STDSTR( m_aURL.getBindingUrl( ) ),
                        rUsername, rPassword, OUSTR_TO_STDSTR( m_aURL.getRepositoryId( ) ) );
                if ( m_pSession == NULL )
                    ucbhelper::cancelCommandExecution(
                                        ucb::IOErrorCode_INVALID_DEVICE,
                                        uno::Sequence< uno::Any >( 0 ),
                                        xEnv,
                                        rtl::OUString( ) );
                m_pProvider->registerSession( sSessionId, m_pSession );
            }
            else
            {
                // Silently fail as the user cancelled the authentication
                throw uno::RuntimeException( );
            }
        }
        return m_pSession;
    }

    libcmis::ObjectTypePtr Content::getObjectType( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( NULL == m_pObjectType.get( ) && m_bTransient )
        {
            string typeId = m_bIsFolder ? "cmis:folder" : "cmis:document";
            m_pObjectType = getSession( xEnv )->getType( typeId );
        }
        return m_pObjectType;
    }


    libcmis::ObjectPtr Content::getObject( const uno::Reference< ucb::XCommandEnvironment >& xEnv ) throw ( libcmis::Exception )
    {
        if ( !m_pObject.get() )
        {
            if ( !m_sObjectPath.isEmpty( ) )
                m_pObject = getSession( xEnv )->getObjectByPath( OUSTR_TO_STDSTR( m_sObjectPath ) );
            else if (!m_sObjectId.isEmpty( ) )
                m_pObject = getSession( xEnv )->getObject( OUSTR_TO_STDSTR( m_sObjectId ) );
            else
            {
                m_pObject = getSession( xEnv )->getRootFolder( );
                m_sObjectPath = "/";
                m_sObjectId = rtl::OUString( );
            }
        }

        return m_pObject;
    }

    bool Content::isFolder(const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        bool bIsFolder = false;
        try
        {
            bIsFolder = getObject( xEnv )->getBaseType( ) == "cmis:folder";
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what( ) ) );
        }
        return bIsFolder;
    }

    uno::Any Content::getBadArgExcept()
    {
        return uno::makeAny( lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wrong argument type!")),
            static_cast< cppu::OWeakObject * >( this ), -1) );
    }

    uno::Reference< sdbc::XRow > Content::getPropertyValues(
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
                    try
                    {
                        xRow->appendBoolean( rProp, getObject( xEnv )->getBaseType( ) == "cmis:document" );
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
                        xRow->appendBoolean( rProp, getObject( xEnv )->getBaseType( ) == "cmis:folder" );
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
                    rtl::OUString sTitle;
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
                        rtl::OUString sPath = m_sObjectPath;

                        // Get rid of the trailing slash problem
                        if ( sPath[ sPath.getLength( ) - 1 ] == '/' )
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
                else if ( rProp.Name == "TitleOnServer" )
                {
                    string path;
                    try
                    {
                        vector< string > paths = getObject( xEnv )->getPaths( );
                        if ( !paths.empty( ) )
                            path = paths.front( );
                        else
                            path = getObject( xEnv )->getName( );

                        xRow->appendString( rProp, STD_TO_OUSTR( path ) );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "IsReadOnly" )
                {
                    boost::shared_ptr< libcmis::AllowableActions > allowableActions = getObject( xEnv )->getAllowableActions( );
                    sal_Bool bReadOnly = sal_False;
                    if ( !allowableActions->isAllowed( libcmis::ObjectAction::SetContentStream ) &&
                         !allowableActions->isAllowed( libcmis::ObjectAction::CheckIn ) )
                        bReadOnly = sal_True;

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
                        if ( NULL != document )
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
                        if ( NULL != document )
                            xRow->appendString( rProp, STD_TO_OUSTR( document->getContentType() ) );
                        else
                            xRow->appendVoid( rProp );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CmisPropertiesValues" )
                {
                    try
                    {
                        libcmis::ObjectPtr object = getObject( xEnv );
                        map< string, libcmis::PropertyPtr >& aProperties = object->getProperties( );
                        beans::PropertyValues aCmisProperties( aProperties.size( ) );
                        beans::PropertyValue* pCmisProps = aCmisProperties.getArray( );
                        sal_Int32 i = 0;
                        for ( map< string, libcmis::PropertyPtr >::iterator it = aProperties.begin();
                                it != aProperties.end( ); ++it, ++i )
                        {
                            string name = it->first;
                            pCmisProps[i].Name = STD_TO_OUSTR( name );
                            pCmisProps[i].Value = lcl_cmisPropertyToUno( it->second );
                        }
                        xRow->appendObject( rProp.Name, uno::makeAny( aCmisProperties ) );
                    }
                    catch ( const libcmis::Exception& )
                    {
                        xRow->appendVoid( rProp );
                    }
                }
                else if ( rProp.Name == "CmisPropertiesDisplayNames" )
                {
                    try
                    {
                        libcmis::ObjectPtr object = getObject( xEnv );
                        map< string, libcmis::PropertyPtr >& aProperties = object->getProperties( );
                        beans::PropertyValues aCmisProperties( aProperties.size( ) );
                        beans::PropertyValue* pCmisProps = aCmisProperties.getArray( );
                        sal_Int32 i = 0;
                        for ( map< string, libcmis::PropertyPtr >::iterator it = aProperties.begin();
                                it != aProperties.end( ); ++it, ++i )
                        {
                            string name = it->first;
                            string displayName = it->second->getPropertyType()->getDisplayName( );
                            pCmisProps[i].Name = STD_TO_OUSTR( name );
                            pCmisProps[i].Value = uno::makeAny( STD_TO_OUSTR( displayName ) );
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
                        sal_Bool bIsVersionable = object->getTypeDescription( )->isVersionable( );
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
                    SAL_INFO( "cmisucp", "Looking for unsupported property " << rProp.Name );
            }
            catch (const libcmis::Exception&)
            {
                xRow->appendVoid( rProp );
            }
        }

        return uno::Reference< sdbc::XRow >( xRow.get() );
    }

    bool Content::exists( const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        bool bExists = true;
        try
        {
            if ( !m_sObjectPath.isEmpty( ) )
                m_pSession->getObjectByPath( OUSTR_TO_STDSTR( m_sObjectPath ) );
            else if ( !m_sObjectId.isEmpty( ) )
                getSession( xEnv )->getObject( OUSTR_TO_STDSTR( m_sObjectId ) );
            // No need to handle the root folder case... how can it not exists?
        }
        catch ( const libcmis::Exception& )
        {
            bExists = false;
        }

        return bExists;
    }

    uno::Any Content::open(const ucb::OpenCommandArgument2 & rOpenCommand,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
            throw( uno::Exception )
    {
        bool bIsFolder = isFolder( xEnv );

        // Handle the case of the non-existing file
        if ( !exists( xEnv ) )
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= m_xIdentifier->getContentIdentifier();
            uno::Any aErr = uno::makeAny(
                ucb::InteractiveAugmentedIOException(rtl::OUString(), static_cast< cppu::OWeakObject * >( this ),
                    task::InteractionClassification_ERROR,
                    bIsFolder ? ucb::IOErrorCode_NOT_EXISTING_PATH : ucb::IOErrorCode_NOT_EXISTING, aArgs)
            );

            ucbhelper::cancelCommandExecution(aErr, xEnv);
        }

        uno::Any aRet;

        sal_Bool bOpenFolder = (
            ( rOpenCommand.Mode == ucb::OpenMode::ALL ) ||
            ( rOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
            ( rOpenCommand.Mode == ucb::OpenMode::DOCUMENTS )
         );

        if ( bOpenFolder && bIsFolder )
        {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(m_xSMgr, this, rOpenCommand, xEnv );
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
                        ( rtl::OUString(), static_cast< cppu::OWeakObject * >( this ),
                          sal_Int16( rOpenCommand.Mode ) ) ),
                        xEnv );
            }

            if ( !feedSink( rOpenCommand.Sink, xEnv ) )
            {
                // Note: rOpenCommand.Sink may contain an XStream
                //       implementation. Support for this type of
                //       sink is optional...
                SAL_INFO( "cmisucp", "Failed to copy data to sink" );

                ucbhelper::cancelCommandExecution(
                    uno::makeAny (ucb::UnsupportedDataSinkException
                        ( rtl::OUString(), static_cast< cppu::OWeakObject * >( this ),
                          rOpenCommand.Sink ) ),
                        xEnv );
            }
        }
        else
            SAL_INFO( "cmisucp", "Open falling through ..." );

        return aRet;
    }

    rtl::OUString Content::checkIn( const ucb::CheckinArgument& rArg,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
            throw( uno::Exception )
    {
        ucbhelper::Content aSourceContent( rArg.SourceURL, xEnv, comphelper::getProcessComponentContext( ) );
        uno::Reference< io::XInputStream > xIn = aSourceContent.openStream( );

        libcmis::ObjectPtr object;
        try
        {
            object = getObject( xEnv );
        }
        catch ( const libcmis::Exception& )
        {
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
        uno::Reference < io::XOutputStream > xOutput = new ucbhelper::StdOutputStream( pOut );
        copyData( xIn, xOutput );

        map< string, libcmis::PropertyPtr > newProperties;
        libcmis::DocumentPtr pDoc = pPwc->checkIn( rArg.MajorVersion, OUSTR_TO_STDSTR( rArg.VersionComment ), newProperties,
                           pOut, OUSTR_TO_STDSTR( rArg.MimeType ), OUSTR_TO_STDSTR( rArg.NewTitle ) );

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

    rtl::OUString Content::checkOut( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
            throw( uno::Exception )
    {
        rtl::OUString aRet;
        try
        {
            // Checkout the document if possible
            libcmis::DocumentPtr pDoc = boost::dynamic_pointer_cast< libcmis::Document >( getObject( xEnv ) );
            if ( pDoc.get( ) == NULL )
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
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what() ) );
        }
        return aRet;
    }

    rtl::OUString Content::cancelCheckOut( const uno::Reference< ucb::XCommandEnvironment > & xEnv )
            throw( uno::Exception )
    {
        rtl::OUString aRet;
        try
        {
            libcmis::DocumentPtr pPwc = boost::dynamic_pointer_cast< libcmis::Document >( getObject( xEnv ) );
            if ( pPwc.get( ) == NULL )
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
            libcmis::DocumentPtr pDoc;
            for ( vector< libcmis::DocumentPtr >::iterator it = aVersions.begin();
                    it != aVersions.end( ) && pDoc != NULL; ++it )
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
                    pDoc.reset( pVersion.get( ) );
            }

            // Compute the URL of the Document
            URL aCmisUrl( m_sURL );
            vector< string > aPaths = pDoc->getPaths( );
            if ( !aPaths.empty() )
            {
                string sPath = aPaths.front( );
                aCmisUrl.setObjectPath( STD_TO_OUSTR( sPath ) );
            }
            else
            {
                // We may have unfiled doc depending on the server, those
                // won't have any path, use their ID instead
                string sId = pDoc->getId( );
                aCmisUrl.setObjectId( STD_TO_OUSTR( sId ) );
            }
            aRet = aCmisUrl.asString( );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what() ) );
        }
        return aRet;
    }

    void Content::transfer( const ucb::TransferInfo& rTransferInfo,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv )
            throw( uno::Exception )
    {
        // If the source isn't on the same CMIS repository, then simply copy
        INetURLObject aSourceUrl( rTransferInfo.SourceURL );
        if ( aSourceUrl.GetProtocol() != INET_PROT_CMIS )
        {
            rtl::OUString sSrcBindingUrl = URL( rTransferInfo.SourceURL ).getBindingUrl( );
            if ( sSrcBindingUrl != m_aURL.getBindingUrl( ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::InteractiveBadTransferURLException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Unsupported URL scheme!" )),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                    xEnv );
            }
        }

        SAL_INFO( "cmisucp", "TODO - Content::transfer()" );
    }

    void Content::insert( const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting, const rtl::OUString& rMimeType,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
            throw( uno::Exception )
    {
        if ( !xInputStream.is() )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny
                ( ucb::MissingInputStreamException
                  ( rtl::OUString(), static_cast< cppu::OWeakObject * >( this ) ) ),
                xEnv );
        }

        // For transient content, the URL is the one of the parent
        if ( m_bTransient )
        {
            rtl::OUString sNewPath;

            // Try to get the object from the server if there is any
            libcmis::Folder* pFolder = NULL;
            try
            {
                pFolder = dynamic_cast< libcmis::Folder* >( getObject( xEnv ).get( ) );
            }
            catch ( const libcmis::Exception& )
            {
            }

            if ( pFolder != NULL )
            {
                map< string, libcmis::PropertyPtr >::iterator it = m_pObjectProps.find( "cmis:name" );
                if ( it == m_pObjectProps.end( ) )
                {
                    ucbhelper::cancelCommandExecution( uno::makeAny
                        ( uno::RuntimeException( "Missing name property",
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                        xEnv );
                }
                string newName = it->second->getStrings( ).front( );
                string newPath = pFolder->getPath( );
                if ( newPath[ newPath.size( ) - 1 ] != '/' )
                    newPath += "/";
                newPath += newName;

                libcmis::ObjectPtr object;
                try
                {
                    object = getSession( xEnv )->getObjectByPath( newPath );
                    sNewPath = STD_TO_OUSTR( newPath );
                }
                catch ( const libcmis::Exception& )
                {
                    // Nothing matched the path
                }

                if ( NULL != object.get( ) )
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
                    if ( NULL != document )
                    {
                        boost::shared_ptr< ostream > pOut( new ostringstream ( ios_base::binary | ios_base::in | ios_base::out ) );
                        uno::Reference < io::XOutputStream > xOutput = new ucbhelper::StdOutputStream( pOut );
                        copyData( xInputStream, xOutput );
                        document->setContentStream( pOut, OUSTR_TO_STDSTR( rMimeType ), string( ), bReplaceExisting );
                    }
                }
                else
                {
                    // We need to create a brand new object... either folder or document
                    bool bIsFolder = getObjectType( xEnv )->getBaseType( )->getId( ) == "cmis:folder";
                    setCmisProperty( "cmis:objectTypeId", getObjectType( xEnv )->getId( ), xEnv );

                    if ( bIsFolder )
                    {
                        libcmis::FolderPtr pNew = pFolder->createFolder( m_pObjectProps );
                        sNewPath = STD_TO_OUSTR( newPath );
                    }
                    else
                    {
                        boost::shared_ptr< ostream > pOut( new ostringstream ( ios_base::binary | ios_base::in | ios_base::out ) );
                        uno::Reference < io::XOutputStream > xOutput = new ucbhelper::StdOutputStream( pOut );
                        copyData( xInputStream, xOutput );
                        pFolder->createDocument( m_pObjectProps, pOut, OUSTR_TO_STDSTR( rMimeType ), string() );
                        sNewPath = STD_TO_OUSTR( newPath );
                    }
                }

                if ( !sNewPath.isEmpty( ) )
                {
                    // Update the current content: it's no longer transient
                    m_sObjectPath = sNewPath;
                    URL aUrl( m_sURL );
                    aUrl.setObjectPath( m_sObjectPath );
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
        uno::Reference< io::XInputStream > xIn,
        uno::Reference< io::XOutputStream > xOut )
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
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what() ) );
        }

        sal_Int32 nCount = rValues.getLength();
        uno::Sequence< uno::Any > aRet( nCount );

        bool bChanged = false;
        const beans::PropertyValue* pValues = rValues.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::PropertyValue& rValue = pValues[ n ];
            if ( rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) ||
                 rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) ||
                 rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) ||
                 rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) ||
                 rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Size" ) ) ||
                 rValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CreatableContentsInfo" ) ) )
            {
                lang::IllegalAccessException e ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only!")),
                       static_cast< cppu::OWeakObject* >( this ) );
                aRet[ n ] <<= e;
            }
            else if ( rValue.Name == "Title" )
            {
                rtl::OUString aNewTitle;
                if (!( rValue.Value >>= aNewTitle ))
                {
                    aRet[ n ] <<= beans::IllegalTypeException
                        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property value has wrong type!")),
                          static_cast< cppu::OWeakObject * >( this ) );
                    continue;
                }

                if ( aNewTitle.getLength() <= 0 )
                {
                    aRet[ n ] <<= lang::IllegalArgumentException
                        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Empty title not allowed!")),
                          static_cast< cppu::OWeakObject * >( this ), -1 );
                    continue;

                }

                setCmisProperty( "cmis:name", OUSTR_TO_STDSTR( aNewTitle ), xEnv );
                bChanged = true;
            }
            else
            {
                SAL_INFO( "cmisucp", "Couln't set property: " << rValue.Name );
                lang::IllegalAccessException e ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Property is read-only!")),
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
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what() ) );
        }

        return aRet;
    }

    sal_Bool Content::feedSink( uno::Reference< uno::XInterface> xSink,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( !xSink.is() )
            return sal_False;

        uno::Reference< io::XOutputStream > xOut = uno::Reference< io::XOutputStream >(xSink, uno::UNO_QUERY );
        uno::Reference< io::XActiveDataSink > xDataSink = uno::Reference< io::XActiveDataSink >(xSink, uno::UNO_QUERY );
        uno::Reference< io::XActiveDataStreamer > xDataStreamer = uno::Reference< io::XActiveDataStreamer >( xSink, uno::UNO_QUERY );

        if ( !xOut.is() && !xDataSink.is() && ( !xDataStreamer.is() || !xDataStreamer->getStream().is() ) )
            return sal_False;

        if ( xDataStreamer.is() && !xOut.is() )
            xOut = xDataStreamer->getStream()->getOutputStream();

        try
        {
            libcmis::Document* document = dynamic_cast< libcmis::Document* >( getObject( xEnv ).get() );
            boost::shared_ptr< istream > aIn = document->getContentStream( );

            uno::Reference< io::XInputStream > xIn = new ucbhelper::StdInputStream( aIn );
            if( !xIn.is( ) )
                return sal_False;

            if ( xDataSink.is() )
                xDataSink->setInputStream( xIn );
            else if ( xOut.is() )
                copyData( xIn, xOut );
        }
        catch ( const libcmis::Exception& e )
        {
            SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
            ucbhelper::cancelCommandExecution(
                                ucb::IOErrorCode_GENERAL,
                                uno::Sequence< uno::Any >( 0 ),
                                xEnv,
                                rtl::OUString::createFromAscii( e.what() ) );
        }

        return sal_True;
    }

    uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & )
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
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TitleOnServer" ) ),
                -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
                -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
                -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
                -1, getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CreatableContentsInfo" ) ),
                -1, getCppuType( static_cast< const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CmisPropertiesValues" ) ),
                -1, getCppuType( static_cast< const beans::PropertyValues * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CmisPropertiesDisplayNames" ) ),
                -1, getCppuType( static_cast< const beans::PropertyValues * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsVersionable" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CanCheckOut" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CanCancelCheckOut" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CanCheckIn" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        };

        const int nProps = SAL_N_ELEMENTS(aGenericProperties);
        return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
    }

    uno::Sequence< ucb::CommandInfo > Content::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
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
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
              -1, getCppuBooleanType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
              -1, getCppuType( static_cast<ucb::InsertCommandArgument2 * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
              -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),

            // Mandatory CMIS-only commands
            ucb::CommandInfo ( rtl::OUString( "checkout" ), -1, getCppuVoidType() ),
            ucb::CommandInfo ( rtl::OUString( "cancelCheckout" ), -1, getCppuVoidType() ),
            ucb::CommandInfo ( rtl::OUString( "checkIn" ), -1,
                    getCppuType( static_cast<ucb::TransferInfo * >( 0 ) ) ),

            // Folder Only, omitted if not a folder
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
              -1, getCppuType( static_cast<ucb::TransferInfo * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "createNewContent" ) ),
              -1, getCppuType( static_cast<ucb::ContentInfo * >( 0 ) ) )
        };

        const int nProps = SAL_N_ELEMENTS( aCommandInfoTable );
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, isFolder( xEnv ) ? nProps : nProps - 2);
    }

    ::rtl::OUString Content::getParentURL( )
    {
        rtl::OUString sRet;

        SAL_INFO( "cmisucp", "Content::getParentURL()" );

        string parentPath;
        try
        {
            libcmis::ObjectPtr pObj = getObject( uno::Reference< ucb::XCommandEnvironment >() );
            libcmis::Document* document = dynamic_cast< libcmis::Document* >( pObj.get( ) );
            if ( NULL != document )
            {
                vector< boost::shared_ptr< libcmis::Folder > > parents = document->getParents( );
                if ( !parents.empty( ) )
                    parentPath = parents.front( )->getPath( );
            }
            else
            {
                libcmis::Folder* folder = dynamic_cast< libcmis::Folder* >( pObj.get( ) );
                if ( NULL != folder )
                    parentPath = folder->getFolderParent( )->getPath( );
            }
        }
        catch ( const libcmis::Exception & )
        {
            // We may have an exception if we don't have the rights to
            // get the parents
        }

        if ( !parentPath.empty() )
        {
            URL aUrl( m_sURL );
            aUrl.setObjectPath( STD_TO_OUSTR( parentPath ) );
            sRet = aUrl.asString( );
        }
        else
        {
            INetURLObject aUrl( m_sURL );
            if ( aUrl.getSegmentCount( ) > 0 )
            {
                URL aCmisUrl( m_sURL );
                aUrl.removeSegment( );
                aCmisUrl.setObjectPath( aUrl.GetURLPath( INetURLObject::NO_DECODE ) );
                sRet = aCmisUrl.asString( );
            }
        }

        return sRet;
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

    uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType ) throw ( uno::RuntimeException )
    {
        uno::Any aRet = cppu::queryInterface( rType, static_cast< ucb::XContentCreator * >( this ) );
        return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface(rType);
    }

    rtl::OUString SAL_CALL Content::getImplementationName() throw( uno::RuntimeException )
    {
       return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.CmisContent"));
    }

    uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
           throw( uno::RuntimeException )
    {
           uno::Sequence< rtl::OUString > aSNS( 1 );
           aSNS.getArray()[ 0 ] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.CmisContent"));
           return aSNS;
    }

    rtl::OUString SAL_CALL Content::getContentType() throw( uno::RuntimeException )
    {
        return isFolder( uno::Reference< ucb::XCommandEnvironment >() )
            ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FOLDER_TYPE ))
            : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FILE_TYPE ));
    }

    uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
            throw( uno::Exception, ucb::CommandAbortedException, uno::RuntimeException )
    {
        SAL_INFO( "cmisucp", "Content::execute( ) - " << aCommand.Name );
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
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "createNewContent" ) )
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
                    folder->removeTree( );
                }
            }
            catch ( const libcmis::Exception& e )
            {
                SAL_INFO( "cmisucp", "Unexpected libcmis exception: " << e.what( ) );
                ucbhelper::cancelCommandExecution(
                                    ucb::IOErrorCode_GENERAL,
                                    uno::Sequence< uno::Any >( 0 ),
                                    xEnv,
                                    rtl::OUString::createFromAscii( e.what() ) );
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
        else
        {
            SAL_INFO( "cmisucp", "Unknown command to execute" );

            ucbhelper::cancelCommandExecution
                ( uno::makeAny( ucb::UnsupportedCommandException
                  ( rtl::OUString(),
                    static_cast< cppu::OWeakObject * >( this ) ) ),
                  xEnv );
        }

        return aRet;
    }

    void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ ) throw( uno::RuntimeException )
    {
        SAL_INFO( "cmisucp", "TODO - Content::abort()" );
        // TODO Implement me
    }

    uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
            throw( uno::RuntimeException )
    {
        return queryCreatableContentsInfo( uno::Reference< ucb::XCommandEnvironment >() );
    }

    uno::Reference< ucb::XContent > SAL_CALL Content::createNewContent(
            const ucb::ContentInfo& Info ) throw( uno::RuntimeException )
    {
        bool create_document;

        if ( Info.Type == CMIS_FILE_TYPE )
            create_document = true;
        else if ( Info.Type == CMIS_FOLDER_TYPE )
            create_document = false;
        else
        {
            SAL_INFO( "cmisucp", "Unknown type of content to create" );
            return uno::Reference< ucb::XContent >();
        }

        rtl::OUString sParentURL = m_xIdentifier->getContentIdentifier();
        URL aParentURL( sParentURL );

        // Set the parent URL for the transient objects
        uno::Reference< ucb::XContentIdentifier > xId(new ::ucbhelper::ContentIdentifier(m_xSMgr, sParentURL));

        try
        {
            return new ::cmis::Content( m_xSMgr, m_pProvider, xId, !create_document );
        }
        catch ( ucb::ContentCreationException & )
        {
            return uno::Reference< ucb::XContent >();
        }
    }

    uno::Sequence< uno::Type > SAL_CALL Content::getTypes() throw( uno::RuntimeException )
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
        else
        {
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
    }


    uno::Sequence< ucb::ContentInfo > Content::queryCreatableContentsInfo(
        const uno::Reference< ucb::XCommandEnvironment >& xEnv)
            throw( uno::RuntimeException )
    {
        if ( isFolder( xEnv ) )
        {
            uno::Sequence< ucb::ContentInfo > seq(2);

            // Minimum set of props we really need
            uno::Sequence< beans::Property > props( 1 );
            props[0] = beans::Property(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                -1,
                getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::BOUND );

            // file
            seq[0].Type       = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FILE_TYPE ));
            seq[0].Attributes = ( ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                                  ucb::ContentInfoAttribute::KIND_DOCUMENT );
            seq[0].Properties = props;

            // folder
            seq[1].Type       = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FOLDER_TYPE ));
            seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
            seq[1].Properties = props;

            return seq;
        }
        else
        {
            return uno::Sequence< ucb::ContentInfo >();
        }
    }

    list< uno::Reference< ucb::XContent > > Content::getChildren( )
    {
        list< uno::Reference< ucb::XContent > > results;
        SAL_INFO( "cmisucp", "Content::getChildren() " << m_sURL );

        libcmis::Folder* pFolder = dynamic_cast< libcmis::Folder* >( getObject( uno::Reference< ucb::XCommandEnvironment >() ).get( ) );
        if ( NULL != pFolder )
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

                    URL aUrl( m_sURL );
                    rtl::OUString sPath( m_sObjectPath );
                    if ( sPath[sPath.getLength( ) - 1] != '/' )
                        sPath += "/";
                    sPath += STD_TO_OUSTR( ( *it )->getName( ) );
                    aUrl.setObjectPath( sPath );
                    uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aUrl.asString( ) );
                    uno::Reference< ucb::XContent > xContent = new Content( m_xSMgr, m_pProvider, xId, *it );

                    results.push_back( xContent );
                }
            }
            catch ( const libcmis::Exception& e )
            {
                SAL_INFO( "cmisucp", "Exception thrown: " << e.what() );
            }
        }

        return results;
    }

    void Content::setCmisProperty( std::string sName, std::string sValue, const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    {
        if ( getObjectType( xEnv ).get( ) )
        {
            map< string, libcmis::PropertyPtr >::iterator propIt = m_pObjectProps.find( sName );
            vector< string > values;
            values.push_back( sValue );

            if ( propIt == m_pObjectProps.end( ) && getObjectType( xEnv ).get( ) )
            {
                map< string, libcmis::PropertyTypePtr > propsTypes = getObjectType( xEnv )->getPropertiesTypes( );
                map< string, libcmis::PropertyTypePtr >::iterator typeIt = propsTypes.find( sName );

                if ( typeIt != propsTypes.end( ) )
                {
                    libcmis::PropertyTypePtr propType = typeIt->second;
                    libcmis::PropertyPtr property( new libcmis::Property( propType, values ) );
                    m_pObjectProps.insert( pair< string, libcmis::PropertyPtr >( sName, property ) );
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
