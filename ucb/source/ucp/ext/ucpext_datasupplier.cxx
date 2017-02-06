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
#include <ucbhelper/contentidentifier.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>

#include <memory>
#include <vector>


namespace ucb { namespace ucp { namespace ext
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
    using ::com::sun::star::ucb::ResultSetException;
    using ::com::sun::star::deployment::PackageInformationProvider;
    using ::com::sun::star::deployment::XPackageInformationProvider;
    using ::com::sun::star::sdbc::XResultSet;


    //= ResultListEntry

    struct ResultListEntry
    {
        OUString                 sId;
        Reference< XContentIdentifier > xId;
        ::rtl::Reference< Content >     pContent;
        Reference< XRow >               xRow;
    };

    typedef ::std::vector< ResultListEntry >    ResultList;


    //= DataSupplier_Impl

    struct DataSupplier_Impl
    {
        ::osl::Mutex                                m_aMutex;
        ResultList                                  m_aResults;
        ::rtl::Reference< Content >                 m_xContent;
        Reference< XComponentContext >              m_xContext;

        DataSupplier_Impl( const Reference< XComponentContext >& rxContext, const ::rtl::Reference< Content >& i_rContent )
            :m_xContent( i_rContent )
            ,m_xContext( rxContext )
        {
        }
    };


    //= helper

    namespace
    {
        OUString lcl_compose( const OUString& i_rBaseURL, const OUString& i_rRelativeURL )
        {
            ENSURE_OR_RETURN( !i_rBaseURL.isEmpty(), "illegal base URL", i_rRelativeURL );

            OUStringBuffer aComposer( i_rBaseURL );
            if ( !i_rBaseURL.endsWith("/") )
                aComposer.append( '/' );
            aComposer.append( i_rRelativeURL );
            return aComposer.makeStringAndClear();
        }
    }


    //= DataSupplier


    DataSupplier::DataSupplier( const Reference< XComponentContext >& rxContext,
                                const ::rtl::Reference< Content >& i_rContent )
        :m_pImpl( new DataSupplier_Impl( rxContext, i_rContent ) )
    {
    }


    void DataSupplier::fetchData()
    {
        try
        {
            const Reference< XPackageInformationProvider > xPackageInfo = PackageInformationProvider::get( m_pImpl->m_xContext );

            const OUString sContentIdentifier( m_pImpl->m_xContent->getIdentifier()->getContentIdentifier() );

            switch ( m_pImpl->m_xContent->getExtensionContentType() )
            {
            case E_ROOT:
            {
                Sequence< Sequence< OUString > > aExtensionInfo( xPackageInfo->getExtensionList() );
                for (   const Sequence< OUString >* pExtInfo = aExtensionInfo.getConstArray();
                        pExtInfo != aExtensionInfo.getConstArray() + aExtensionInfo.getLength();
                        ++pExtInfo
                    )
                {
                    if ( pExtInfo->getLength() <= 0 )
                    {
                        SAL_WARN( "ucb.ucp.ext", "illegal extension info" );
                        continue;
                    }

                    const OUString& rLocalId = (*pExtInfo)[0];
                    ResultListEntry aEntry;
                    aEntry.sId = ContentProvider::getRootURL() + Content::encodeIdentifier( rLocalId ) + "/";
                    m_pImpl->m_aResults.push_back( aEntry );
                }
            }
            break;
            case E_EXTENSION_ROOT:
            case E_EXTENSION_CONTENT:
            {
                const OUString sPackageLocation( m_pImpl->m_xContent->getPhysicalURL() );
                ::ucbhelper::Content aWrappedContent( sPackageLocation, getResultSet()->getEnvironment(), m_pImpl->m_xContext );

                // obtain the properties which our result set is set up for from the wrapped content
                Sequence< OUString > aPropertyNames { "Title" };

                const Reference< XResultSet > xFolderContent( aWrappedContent.createCursor( aPropertyNames ), UNO_SET_THROW );
                const Reference< XRow > xContentRow( xFolderContent, UNO_QUERY_THROW );
                while ( xFolderContent->next() )
                {
                    ResultListEntry aEntry;
                    aEntry.sId = lcl_compose( sContentIdentifier, xContentRow->getString( 1 ) );
                    m_pImpl->m_aResults.push_back( aEntry );
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    DataSupplier::~DataSupplier()
    {
    }


    OUString DataSupplier::queryContentIdentifierString( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
        {
            const OUString sId = m_pImpl->m_aResults[ i_nIndex ].sId;
            if ( !sId.isEmpty() )
                return sId;
        }

        OSL_FAIL( "DataSupplier::queryContentIdentifierString: illegal index, or illegal result entry id!" );
        return OUString();
    }


    Reference< XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
        {
            Reference< XContentIdentifier > xId( m_pImpl->m_aResults[ i_nIndex ].xId );
            if ( xId.is() )
                return xId;
        }

        OUString sId = queryContentIdentifierString( i_nIndex );
        if ( !sId.isEmpty() )
        {
            Reference< XContentIdentifier > xId = new ::ucbhelper::ContentIdentifier( sId );
            m_pImpl->m_aResults[ i_nIndex ].xId = xId;
            return xId;
        }

        return Reference< XContentIdentifier >();
    }


    Reference< XContent > DataSupplier::queryContent( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_pImpl->m_aResults.size(), "illegal index!", nullptr );


        ::rtl::Reference< Content > pContent( m_pImpl->m_aResults[ i_nIndex ].pContent );
        if ( pContent.is() )
            return pContent.get();

        Reference< XContentIdentifier > xId( queryContentIdentifier( i_nIndex ) );
        if ( xId.is() )
        {
            try
            {
                Reference< XContent > xContent( m_pImpl->m_xContent->getProvider()->queryContent( xId ) );
                pContent.set( dynamic_cast< Content* >( xContent.get() ) );
                OSL_ENSURE( pContent.is() || !xContent.is(), "DataSupplier::queryContent: invalid content implementation!" );
                m_pImpl->m_aResults[ i_nIndex ].pContent = pContent;
                return pContent.get();

            }
            catch ( const IllegalIdentifierException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        return Reference< XContent >();
    }


    bool DataSupplier::getResult( sal_uInt32 i_nIndex )
    {
        ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( m_pImpl->m_aResults.size() > i_nIndex )
            // result already present.
            return true;

        return false;
    }


    sal_uInt32 DataSupplier::totalCount()
    {
        ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );
        return m_pImpl->m_aResults.size();
    }


    sal_uInt32 DataSupplier::currentCount()
    {
        return m_pImpl->m_aResults.size();
    }


    bool DataSupplier::isCountFinal()
    {
        return true;
    }


    Reference< XRow > DataSupplier::queryPropertyValues( sal_uInt32 i_nIndex  )
    {
        ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_pImpl->m_aResults.size(), "DataSupplier::queryPropertyValues: illegal index!", nullptr );

        Reference< XRow > xRow = m_pImpl->m_aResults[ i_nIndex ].xRow;
        if ( xRow.is() )
            return xRow;

        ENSURE_OR_RETURN( queryContent( i_nIndex ).is(), "could not retrieve the content", nullptr );

        switch ( m_pImpl->m_xContent->getExtensionContentType() )
        {
        case E_ROOT:
        {
            const OUString& rId( m_pImpl->m_aResults[ i_nIndex ].sId );
            const OUString sRootURL( ContentProvider::getRootURL() );
            OUString sTitle = Content::decodeIdentifier( rId.copy( sRootURL.getLength() ) );
            if ( sTitle.endsWith("/") )
                sTitle = sTitle.copy( 0, sTitle.getLength() - 1 );
            xRow = Content::getArtificialNodePropertyValues( m_pImpl->m_xContext, getResultSet()->getProperties(), sTitle );
        }
        break;

        case E_EXTENSION_ROOT:
        case E_EXTENSION_CONTENT:
        {
            xRow = m_pImpl->m_aResults[ i_nIndex ].pContent->getPropertyValues(
                getResultSet()->getProperties(), getResultSet()->getEnvironment() );
        }
        break;
        default:
            OSL_FAIL( "DataSupplier::queryPropertyValues: unhandled case!" );
            break;
        }

        m_pImpl->m_aResults[ i_nIndex ].xRow = xRow;
        return xRow;
    }


    void DataSupplier::releasePropertyValues( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
            m_pImpl->m_aResults[ i_nIndex ].xRow.clear();
    }


    void DataSupplier::close()
    {
    }


    void DataSupplier::validate()
    {
    }


} } }   // namespace ucp::ext


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
