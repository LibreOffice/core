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

#include "ucpext_content.hxx"
#include "ucpext_provider.hxx"
#include "ucpext_resultset.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/deployment/PackageInformationProvider.hpp>

#include <o3tl/string_view.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>

#include <algorithm>
#include <string_view>


namespace ucb::ucp::ext
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::ucb::OpenCommandArgument2;
    using ::com::sun::star::ucb::XDynamicResultSet;
    using ::com::sun::star::ucb::UnsupportedCommandException;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::IllegalAccessException;
    using ::com::sun::star::ucb::CommandInfo;
    using ::com::sun::star::deployment::PackageInformationProvider;
    using ::com::sun::star::deployment::XPackageInformationProvider;

    namespace OpenMode = ::com::sun::star::ucb::OpenMode;
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;


    //= helper

    namespace
    {

        OUString lcl_compose( std::u16string_view i_rBaseURL, const OUString& i_rRelativeURL )
        {
            ENSURE_OR_RETURN( !i_rBaseURL.empty(), "illegal base URL", i_rRelativeURL );

            OUStringBuffer aComposer( i_rBaseURL );
            if ( !o3tl::ends_with(i_rBaseURL, u"/") )
                aComposer.append( '/' );
            aComposer.append( i_rRelativeURL );
            return aComposer.makeStringAndClear();
        }


        struct SelectPropertyName
        {
            const OUString& operator()( const Property& i_rProperty ) const
            {
                return i_rProperty.Name;
            }
        };
    }


    //= Content


    Content::Content( const Reference< XComponentContext >& rxContext, ::ucbhelper::ContentProviderImplHelper* i_pProvider,
                      const Reference< XContentIdentifier >& i_rIdentifier )
        :Content_Base( rxContext, i_pProvider, i_rIdentifier )
        ,m_eExtContentType( E_UNKNOWN )
    {
        const OUString sURL( getIdentifier()->getContentIdentifier() );
        if ( denotesRootContent( sURL ) )
        {
            m_eExtContentType = E_ROOT;
        }
        else
        {
            const std::u16string_view sRelativeURL( sURL.subView( ContentProvider::getRootURL().getLength() ) );
            const size_t nSepPos = sRelativeURL.find( '/' );
            if ( ( nSepPos == std::u16string_view::npos ) || ( nSepPos == sRelativeURL.size() - 1 ) )
            {
                m_eExtContentType = E_EXTENSION_ROOT;
            }
            else
            {
                m_eExtContentType = E_EXTENSION_CONTENT;
            }
        }

        if ( m_eExtContentType == E_ROOT )
            return;

        const OUString sRootURL = ContentProvider::getRootURL();
        m_sExtensionId = sURL.copy( sRootURL.getLength() );

        const sal_Int32 nNextSep = m_sExtensionId.indexOf( '/' );
        if ( nNextSep > -1 )
        {
            m_sPathIntoExtension = m_sExtensionId.copy( nNextSep + 1 );
            m_sExtensionId = m_sExtensionId.copy( 0, nNextSep );
        }
        m_sExtensionId = Content::decodeIdentifier( m_sExtensionId );
    }


    Content::~Content()
    {
    }


    OUString SAL_CALL Content::getImplementationName()
    {
        return u"org.openoffice.comp.ucp.ext.Content"_ustr;
    }


    Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    {
        return { u"com.sun.star.ucb.Content"_ustr, u"com.sun.star.ucb.ExtensionContent"_ustr };
    }


    OUString SAL_CALL Content::getContentType()
    {
        impl_determineContentType();
        return *m_aContentType;
    }


    Any SAL_CALL Content::execute( const Command& aCommand, sal_Int32 /* CommandId */, const Reference< XCommandEnvironment >& i_rEnvironment )
    {
        Any aRet;

        if ( aCommand.Name == "getPropertyValues" )
        {
            Sequence< Property > Properties;
            if ( !( aCommand.Argument >>= Properties ) )
            {
                ::ucbhelper::cancelCommandExecution( Any( IllegalArgumentException(
                    OUString(), *this, -1 ) ),
                    i_rEnvironment );
                // unreachable
            }

            aRet <<= getPropertyValues( Properties, i_rEnvironment );
        }
        else if ( aCommand.Name == "setPropertyValues" )
        {
            Sequence< PropertyValue > aProperties;
            if ( !( aCommand.Argument >>= aProperties ) )
            {
                ::ucbhelper::cancelCommandExecution( Any( IllegalArgumentException(
                    OUString(), *this, -1 ) ),
                    i_rEnvironment );
                // unreachable
            }

            if ( !aProperties.hasElements() )
            {
                ::ucbhelper::cancelCommandExecution( Any( IllegalArgumentException(
                    OUString(), *this, -1 ) ),
                    i_rEnvironment );
                // unreachable
            }

            aRet <<= setPropertyValues( aProperties );
        }
        else if ( aCommand.Name == "getPropertySetInfo" )
        {
            // implemented by base class.
            aRet <<= getPropertySetInfo( i_rEnvironment );
        }
        else if ( aCommand.Name == "getCommandInfo" )
        {
            // implemented by base class.
            aRet <<= getCommandInfo( i_rEnvironment );
        }
        else if ( aCommand.Name == "open" )
        {
            OpenCommandArgument2 aOpenCommand;
            if ( !( aCommand.Argument >>= aOpenCommand ) )
            {
                ::ucbhelper::cancelCommandExecution( Any( IllegalArgumentException(
                    OUString(), *this, -1 ) ),
                    i_rEnvironment );
                // unreachable
            }

            bool bOpenFolder =
                ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
                  ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
                  ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );


            if ( bOpenFolder && impl_isFolder() )
            {
                Reference< XDynamicResultSet > xSet = new ResultSet( m_xContext, this, aOpenCommand, i_rEnvironment );
                aRet <<= xSet;
            }

            if ( aOpenCommand.Sink.is() )
            {
                const OUString sPhysicalContentURL( getPhysicalURL() );
                ::ucbhelper::Content aRequestedContent( sPhysicalContentURL, i_rEnvironment, m_xContext );
                aRet = aRequestedContent.executeCommand( u"open"_ustr, Any( aOpenCommand ) );
            }
        }

        else
        {
            ::ucbhelper::cancelCommandExecution( Any( UnsupportedCommandException(
                OUString(), *this ) ),
                i_rEnvironment );
            // unreachable
        }

        return aRet;
    }


    void SAL_CALL Content::abort( sal_Int32 )
    {
    }


    OUString Content::encodeIdentifier( const OUString& i_rIdentifier )
    {
        return ::rtl::Uri::encode( i_rIdentifier, rtl_UriCharClassRegName, rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8 );
    }


    OUString Content::decodeIdentifier( const OUString& i_rIdentifier )
    {
        return ::rtl::Uri::decode( i_rIdentifier, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
    }


    bool Content::denotesRootContent( std::u16string_view i_rContentIdentifier )
    {
        const OUString sRootURL( ContentProvider::getRootURL() );
        if ( i_rContentIdentifier == sRootURL )
            return true;

        // the root URL contains only two trailing /, but we also recognize 3 of them as denoting the root URL
        if  (   o3tl::starts_with(i_rContentIdentifier,  sRootURL )
            &&  ( sal_Int32(i_rContentIdentifier.size()) == sRootURL.getLength() + 1 )
            &&  ( i_rContentIdentifier[ i_rContentIdentifier.size() - 1 ] == '/' )
            )
            return true;

        return false;
    }


    OUString Content::getParentURL()
    {
        const OUString sRootURL( ContentProvider::getRootURL() );

        switch ( m_eExtContentType )
        {
        case E_ROOT:
            // don't have a parent
            return sRootURL;

        case E_EXTENSION_ROOT:
            // our parent is the root itself
            return sRootURL;

        case E_EXTENSION_CONTENT:
        {
            const OUString sURL = m_xIdentifier->getContentIdentifier();

            // cut the root URL
            if ( !sURL.match( sRootURL ) )
            {
                SAL_INFO( "ucb.ucp.ext", "illegal URL structure - no root" );
                break;
            }

            OUString sRelativeURL( sURL.copy( sRootURL.getLength() ) );

            // cut the extension ID
            const OUString sSeparatedExtensionId( encodeIdentifier( m_sExtensionId ) + "/" );
            if ( !sRelativeURL.match( sSeparatedExtensionId ) )
            {
                SAL_INFO( "ucb.ucp.ext", "illegal URL structure - no extension ID" );
                break;
            }

            sRelativeURL = sRelativeURL.copy( sSeparatedExtensionId.getLength() );

            // cut the final slash (if any)
            if ( sRelativeURL.isEmpty() )
            {
                SAL_INFO( "ucb.ucp.ext", "illegal URL structure - ExtensionContent should have a level below the extension ID" );
                break;
            }

            if ( sRelativeURL.endsWith("/") )
                sRelativeURL = sRelativeURL.copy( 0, sRelativeURL.getLength() - 1 );

            // remove the last segment
            const sal_Int32 nLastSep = sRelativeURL.lastIndexOf( '/' );
            sRelativeURL = sRelativeURL.copy( 0, nLastSep != -1 ? nLastSep : 0 );

            return sRootURL + sSeparatedExtensionId + sRelativeURL;
        }

        default:
            OSL_FAIL( "Content::getParentURL: unhandled case!" );
            break;
        }
        return OUString();
    }


    Reference< XRow > Content::getArtificialNodePropertyValues( const Reference< XComponentContext >& rxContext,
        const Sequence< Property >& i_rProperties, const OUString& i_rTitle )
    {
        // note: empty sequence means "get values of all supported properties".
        ::rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( rxContext );

        if ( i_rProperties.hasElements() )
        {
            for ( const Property& rProp : i_rProperties )
            {
                // Process Core properties.
                if ( rProp.Name == "ContentType" )
                {
                    xRow->appendString ( rProp, ContentProvider::getArtificialNodeContentType() );
                }
                else if ( rProp.Name == "Title" )
                {
                    xRow->appendString ( rProp, i_rTitle );
                }
                else if ( rProp.Name == "IsDocument" )
                {
                    xRow->appendBoolean( rProp, false );
                }
                else if ( rProp.Name == "IsFolder" )
                {
                    xRow->appendBoolean( rProp, true );
                }
                else
                {
                    // append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
        else
        {
            // Append all Core Properties.
            xRow->appendString ( Property( u"ContentType"_ustr,
                          -1,
                          cppu::UnoType<OUString>::get(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                ContentProvider::getArtificialNodeContentType() );
            xRow->appendString ( Property( u"Title"_ustr,
                          -1,
                          cppu::UnoType<OUString>::get(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                i_rTitle );
            xRow->appendBoolean( Property( u"IsDocument"_ustr,
                          -1,
                          cppu::UnoType<bool>::get(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                false );
            xRow->appendBoolean( Property( u"IsFolder"_ustr,
                          -1,
                          cppu::UnoType<bool>::get(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                true );
        }

        return xRow;
    }


    OUString Content::getPhysicalURL() const
    {
        ENSURE_OR_RETURN( m_eExtContentType != E_ROOT, "illegal call", OUString() );

        // create a ucb::XContent for the physical file within the deployed extension
        const Reference< XPackageInformationProvider > xPackageInfo = PackageInformationProvider::get(m_xContext);
        const OUString sPackageLocation( xPackageInfo->getPackageLocation( m_sExtensionId ) );

        if ( m_sPathIntoExtension.isEmpty() )
            return sPackageLocation;
        return lcl_compose( sPackageLocation, m_sPathIntoExtension );
    }


    Reference< XRow > Content::getPropertyValues( const Sequence< Property >& i_rProperties, const Reference< XCommandEnvironment >& i_rEnv )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

        switch ( m_eExtContentType )
        {
        case E_ROOT:
            return getArtificialNodePropertyValues( m_xContext, i_rProperties, ContentProvider::getRootURL() );
        case E_EXTENSION_ROOT:
            return getArtificialNodePropertyValues( m_xContext, i_rProperties, m_sExtensionId );
        case E_EXTENSION_CONTENT:
        {
            const OUString sPhysicalContentURL( getPhysicalURL() );
            ::ucbhelper::Content aRequestedContent( sPhysicalContentURL, i_rEnv, m_xContext );

            // translate the property request
            Sequence< OUString > aPropertyNames( i_rProperties.getLength() );
            ::std::transform(
                i_rProperties.begin(),
                i_rProperties.end(),
                aPropertyNames.getArray(),
                SelectPropertyName()
            );
            const Sequence< Any > aPropertyValues = aRequestedContent.getPropertyValues( aPropertyNames );
            const ::rtl::Reference< ::ucbhelper::PropertyValueSet > xValueRow = new ::ucbhelper::PropertyValueSet( m_xContext );
            sal_Int32 i=0;
            for (   const Any* value = aPropertyValues.getConstArray();
                    value != aPropertyValues.getConstArray() + aPropertyValues.getLength();
                    ++value, ++i
                )
            {
                xValueRow->appendObject( aPropertyNames[i], *value );
            }
            return xValueRow;
        }

        default:
            OSL_FAIL( "Content::getPropertyValues: unhandled case!" );
            break;
        }

        OSL_FAIL( "Content::getPropertyValues: unreachable!" );
        return nullptr;
    }


    Sequence< Any > Content::setPropertyValues( const Sequence< PropertyValue >& i_rValues)
    {
        ::osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

        Sequence< Any > aRet( i_rValues.getLength() );

        PropertyChangeEvent aEvent;
        aEvent.Source         = getXWeak();
        aEvent.Further        = false;
        aEvent.PropertyHandle = -1;

        for ( auto& rRet : asNonConstRange(aRet) )
        {
            // all our properties are read-only ...
            rRet <<= IllegalAccessException(u"property is read-only."_ustr, *this );
        }

        return aRet;
    }


    Sequence< CommandInfo > Content::getCommands( const Reference< XCommandEnvironment > & /*xEnv*/ )
    {
        static const CommandInfo aCommandInfoTable[] =
        {
            // Mandatory commands

            CommandInfo(
                u"getCommandInfo"_ustr,
                -1,
                cppu::UnoType<void>::get()
            ),
            CommandInfo(
                u"getPropertySetInfo"_ustr,
                -1,
                cppu::UnoType<void>::get()
            ),
            CommandInfo(
                u"getPropertyValues"_ustr,
                -1,
                cppu::UnoType<Sequence< Property >>::get()
            ),
            CommandInfo(
                u"setPropertyValues"_ustr,
                -1,
                cppu::UnoType<Sequence< PropertyValue >>::get()
            )

            // Optional standard commands

            , CommandInfo(
                u"open"_ustr,
                -1,
                cppu::UnoType<OpenCommandArgument2>::get()
            )
        };

        return Sequence< CommandInfo >( aCommandInfoTable, SAL_N_ELEMENTS(aCommandInfoTable) );
    }


    Sequence< Property > Content::getProperties( const Reference< XCommandEnvironment > & /*xEnv*/ )
    {
        static const Property aProperties[] =
        {
            Property(
                u"ContentType"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                u"IsDocument"_ustr,
                -1,
                cppu::UnoType<bool>::get(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                u"IsFolder"_ustr,
                -1,
                cppu::UnoType<bool>::get(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                u"Title"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            )
        };
        return Sequence< Property >( aProperties, SAL_N_ELEMENTS( aProperties ) );
    }


    bool Content::impl_isFolder()
    {
        if ( m_aIsFolder.has_value() )
            return *m_aIsFolder;

        bool bIsFolder = false;
        try
        {
            Sequence< Property > aProps{ { /*Name*/ u"IsFolder"_ustr, {}, {}, {} } };
            Reference< XRow > xRow( getPropertyValues( aProps, nullptr ), UNO_SET_THROW );
            bIsFolder = xRow->getBoolean(1);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("ucb.ucp.ext");
        }
        m_aIsFolder = bIsFolder;
        return *m_aIsFolder;
    }


    void Content::impl_determineContentType()
    {
        if ( !!m_aContentType )
            return;

        m_aContentType = ContentProvider::getArtificialNodeContentType();
        if ( m_eExtContentType != E_EXTENSION_CONTENT )
            return;

        try
        {
            Sequence< Property > aProps{ { /*Name*/ u"ContentType"_ustr, {}, {}, {} } };
            Reference< XRow > xRow( getPropertyValues( aProps, nullptr ), UNO_SET_THROW );
            m_aContentType = xRow->getString(1);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("ucb.ucp.ext");
        }
    }


}   // namespace ucp::ext


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
