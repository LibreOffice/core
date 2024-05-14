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


#include "ucpext_datasupplier.hxx"
#include "ucpext_content.hxx"
#include "ucpext_provider.hxx"

#include <com/sun/star/deployment/PackageInformationProvider.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <o3tl/string_view.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <memory>
#include <string_view>
#include <utility>


namespace ucb::ucp::ext
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::ucb::IllegalIdentifierException;
    using ::com::sun::star::deployment::PackageInformationProvider;
    using ::com::sun::star::deployment::XPackageInformationProvider;
    using ::com::sun::star::sdbc::XResultSet;


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
    }


    //= DataSupplier


    DataSupplier::DataSupplier( const Reference< XComponentContext >& rxContext,
                                ::rtl::Reference< Content > i_xContent )
        :m_xContent(std::move( i_xContent ))
        ,m_xContext( rxContext )
    {
    }


    void DataSupplier::fetchData()
    {
        try
        {
            const Reference< XPackageInformationProvider > xPackageInfo = PackageInformationProvider::get( m_xContext );

            const OUString sContentIdentifier( m_xContent->getIdentifier()->getContentIdentifier() );

            switch ( m_xContent->getExtensionContentType() )
            {
            case E_ROOT:
            {
                const Sequence< Sequence< OUString > > aExtensionInfo( xPackageInfo->getExtensionList() );
                for ( auto const & extInfo : aExtensionInfo )
                {
                    if ( !extInfo.hasElements() )
                    {
                        SAL_WARN( "ucb.ucp.ext", "illegal extension info" );
                        continue;
                    }

                    const OUString& rLocalId = extInfo[0];
                    ResultListEntry aEntry;
                    aEntry.sId = ContentProvider::getRootURL() + Content::encodeIdentifier( rLocalId ) + "/";
                    m_aResults.push_back( aEntry );
                }
            }
            break;
            case E_EXTENSION_ROOT:
            case E_EXTENSION_CONTENT:
            {
                const OUString sPackageLocation( m_xContent->getPhysicalURL() );
                ::ucbhelper::Content aWrappedContent( sPackageLocation, getResultSet()->getEnvironment(), m_xContext );

                // obtain the properties which our result set is set up for from the wrapped content
                Sequence< OUString > aPropertyNames { u"Title"_ustr };

                const Reference< XResultSet > xFolderContent( aWrappedContent.createCursor( aPropertyNames ), UNO_SET_THROW );
                const Reference< XRow > xContentRow( xFolderContent, UNO_QUERY_THROW );
                while ( xFolderContent->next() )
                {
                    ResultListEntry aEntry;
                    aEntry.sId = lcl_compose( sContentIdentifier, xContentRow->getString( 1 ) );
                    m_aResults.push_back( aEntry );
                }
            }
            break;
            default:
                OSL_FAIL( "DataSupplier::fetchData: unimplemented content type!" );
                break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("ucb.ucp.ext");
        }
    }


    DataSupplier::~DataSupplier()
    {
    }


    OUString DataSupplier::queryContentIdentifierString( sal_uInt32 i_nIndex )
    {
        std::unique_lock aGuard( m_aMutex );

        if ( i_nIndex < m_aResults.size() )
        {
            const OUString sId = m_aResults[ i_nIndex ].sId;
            if ( !sId.isEmpty() )
                return sId;
        }

        OSL_FAIL( "DataSupplier::queryContentIdentifierString: illegal index, or illegal result entry id!" );
        return OUString();
    }


    Reference< XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 i_nIndex )
    {
        std::unique_lock aGuard( m_aMutex );

        if ( i_nIndex < m_aResults.size() )
        {
            Reference< XContentIdentifier > xId( m_aResults[ i_nIndex ].xId );
            if ( xId.is() )
                return xId;
        }

        OUString sId = queryContentIdentifierString( i_nIndex );
        if ( !sId.isEmpty() )
        {
            Reference< XContentIdentifier > xId = new ::ucbhelper::ContentIdentifier( sId );
            m_aResults[ i_nIndex ].xId = xId;
            return xId;
        }

        return Reference< XContentIdentifier >();
    }


    Reference< XContent > DataSupplier::queryContent( sal_uInt32 i_nIndex )
    {
        std::unique_lock aGuard( m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_aResults.size(), "illegal index!", nullptr );


        ::rtl::Reference< Content > pContent( m_aResults[ i_nIndex ].pContent );
        if ( pContent.is() )
            return pContent;

        Reference< XContentIdentifier > xId( queryContentIdentifier( i_nIndex ) );
        if ( xId.is() )
        {
            try
            {
                Reference< XContent > xContent( m_xContent->getProvider()->queryContent( xId ) );
                pContent.set( dynamic_cast< Content* >( xContent.get() ) );
                OSL_ENSURE( pContent.is() || !xContent.is(), "DataSupplier::queryContent: invalid content implementation!" );
                m_aResults[ i_nIndex ].pContent = pContent;
                return pContent;

            }
            catch ( const IllegalIdentifierException& )
            {
                DBG_UNHANDLED_EXCEPTION("ucb.ucp.ext");
            }
        }

        return Reference< XContent >();
    }


    bool DataSupplier::getResult( sal_uInt32 i_nIndex )
    {
        std::unique_lock aGuard( m_aMutex );

        // true if result already present.
        return m_aResults.size() > i_nIndex;
    }


    sal_uInt32 DataSupplier::totalCount()
    {
        std::unique_lock aGuard( m_aMutex );
        return m_aResults.size();
    }


    sal_uInt32 DataSupplier::currentCount()
    {
        return m_aResults.size();
    }


    bool DataSupplier::isCountFinal()
    {
        return true;
    }


    Reference< XRow > DataSupplier::queryPropertyValues( sal_uInt32 i_nIndex  )
    {
        std::unique_lock aGuard( m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_aResults.size(), "DataSupplier::queryPropertyValues: illegal index!", nullptr );

        Reference< XRow > xRow = m_aResults[ i_nIndex ].xRow;
        if ( xRow.is() )
            return xRow;

        ENSURE_OR_RETURN( queryContent( i_nIndex ).is(), "could not retrieve the content", nullptr );

        switch ( m_xContent->getExtensionContentType() )
        {
        case E_ROOT:
        {
            const OUString& rId( m_aResults[ i_nIndex ].sId );
            const OUString sRootURL( ContentProvider::getRootURL() );
            OUString sTitle = Content::decodeIdentifier( rId.copy( sRootURL.getLength() ) );
            if ( sTitle.endsWith("/") )
                sTitle = sTitle.copy( 0, sTitle.getLength() - 1 );
            xRow = Content::getArtificialNodePropertyValues( m_xContext, getResultSet()->getProperties(), sTitle );
        }
        break;

        case E_EXTENSION_ROOT:
        case E_EXTENSION_CONTENT:
        {
            xRow = m_aResults[ i_nIndex ].pContent->getPropertyValues(
                getResultSet()->getProperties(), getResultSet()->getEnvironment() );
        }
        break;
        default:
            OSL_FAIL( "DataSupplier::queryPropertyValues: unhandled case!" );
            break;
        }

        m_aResults[ i_nIndex ].xRow = xRow;
        return xRow;
    }


    void DataSupplier::releasePropertyValues( sal_uInt32 i_nIndex )
    {
        std::unique_lock aGuard( m_aMutex );

        if ( i_nIndex < m_aResults.size() )
            m_aResults[ i_nIndex ].xRow.clear();
    }


    void DataSupplier::close()
    {
    }


    void DataSupplier::validate()
    {
    }


} // namespace ucb::ucp::ext


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
