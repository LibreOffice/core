/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#include "precompiled_ucb.hxx"

#include "ucpext_datasupplier.hxx"
#include "ucpext_content.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
/** === end UNO includes === **/

#include <ucbhelper/contentidentifier.hxx>
#include <comphelper/componentcontext.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <tools/diagnose_ex.h>

#include <vector>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::ucb::IllegalIdentifierException;
    using ::com::sun::star::ucb::ResultSetException;
    using ::com::sun::star::deployment::XPackageInformationProvider;
    using ::com::sun::star::beans::Property;
    /** === end UNO using === **/

    //==================================================================================================================
    //= ResultListEntry
    //==================================================================================================================
    struct ResultListEntry
    {
        ::rtl::OUString                 sId;
        Reference< XContentIdentifier > xId;
        ContentProperties               aProperties;
        Reference< XContent >           xContent;
        Reference< XRow >               xRow;

        ResultListEntry( const ::rtl::OUString& i_rParentId, const ::rtl::OUString& i_rLocalId )
        {
            aProperties.aTitle = i_rLocalId;
            sId = i_rParentId + Content::escapeIdentifier( i_rLocalId );
        }
    };

    typedef ::std::vector< ResultListEntry >    ResultList;

    //==================================================================================================================
    //= DataSupplier_Impl
    //==================================================================================================================
    struct DataSupplier_Impl
    {
        ::osl::Mutex                                m_aMutex;
        ResultList                                  m_aResults;
        ::rtl::Reference< Content >                 m_xContent;
        Reference< XMultiServiceFactory >           m_xSMgr;
        sal_Int32                                   m_nOpenMode;

        DataSupplier_Impl( const Reference< XMultiServiceFactory >& i_rORB, const ::rtl::Reference< Content >& i_rContent,
                           const sal_Int32 i_nOpenMode )
            :m_xContent( i_rContent )
            ,m_xSMgr( i_rORB )
            ,m_nOpenMode( i_nOpenMode )
        {
        }
        ~DataSupplier_Impl();
    };

    //------------------------------------------------------------------------------------------------------------------
    DataSupplier_Impl::~DataSupplier_Impl()
    {
    }

    //==================================================================================================================
    //= DataSupplier
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DataSupplier::DataSupplier( const Reference< XMultiServiceFactory >& i_rORB,
                                const ::rtl::Reference< Content >& i_rContent,
                                const sal_Int32 i_nOpenMode )
        :m_pImpl( new DataSupplier_Impl( i_rORB, i_rContent, i_nOpenMode ) )
    {
        try
        {
            if ( Content::denotesRootContent( m_pImpl->m_xContent->getIdentifier() ) )
            {
                const ::rtl::OUString sContentId( m_pImpl->m_xContent->getIdentifier()->getContentIdentifier() );

                const ::comphelper::ComponentContext aContext( i_rORB );
                Reference< XPackageInformationProvider > xPackageInfo(
                    aContext.getSingleton( "com.sun.star.deployment.PackageInformationProvider" ), UNO_QUERY_THROW );

                Sequence< Sequence< ::rtl::OUString > > aExtensionInfo( xPackageInfo->getExtensionList() );
                for (   const Sequence< ::rtl::OUString >* pExtInfo = aExtensionInfo.getConstArray();
                        pExtInfo != aExtensionInfo.getConstArray() + aExtensionInfo.getLength();
                        ++pExtInfo
                    )
                {
                    ENSURE_OR_CONTINUE( pExtInfo->getLength() > 0, "illegal extension info" );

                    ResultListEntry aEntry( sContentId, (*pExtInfo)[0] );
                    aEntry.aProperties.aContentType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.oracle.ooo.extension-content" ) );
                    m_pImpl->m_aResults.push_back( ResultListEntry( aEntry ) );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    DataSupplier::~DataSupplier()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString DataSupplier::queryContentIdentifierString( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
        {
            const ::rtl::OUString sId = m_pImpl->m_aResults[ i_nIndex ].sId;
            if ( sId.getLength() )
                return sId;
        }

        OSL_ENSURE( false, "DataSupplier::queryContentIdentifierString: illegal index, or illegal result entry id!" );
        return ::rtl::OUString();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
        {
            Reference< XContentIdentifier > xId( m_pImpl->m_aResults[ i_nIndex ].xId );
            if ( xId.is() )
                return xId;
        }

        ::rtl::OUString sId = queryContentIdentifierString( i_nIndex );
        if ( sId.getLength() )
        {
            Reference< XContentIdentifier > xId = new ::ucbhelper::ContentIdentifier( sId );
            m_pImpl->m_aResults[ i_nIndex ].xId = xId;
            return xId;
        }

        return Reference< XContentIdentifier >();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XContent > DataSupplier::queryContent( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_pImpl->m_aResults.size(), "illegal index!", NULL );

        Reference< XContent > xContent( m_pImpl->m_aResults[ i_nIndex ].xContent );
        if ( xContent.is() )
            return xContent;

        Reference< XContentIdentifier > xId( queryContentIdentifier( i_nIndex ) );
        if ( xId.is() )
        {
            try
            {
                Reference< XContent > xContent( m_pImpl->m_xContent->getProvider()->queryContent( xId ) );
                m_pImpl->m_aResults[ i_nIndex ].xContent = xContent;
                return xContent;

            }
            catch ( const IllegalIdentifierException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        return Reference< XContent >();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool DataSupplier::getResult( sal_uInt32 i_nIndex )
    {
        ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( m_pImpl->m_aResults.size() > i_nIndex )
            // result already present.
            return sal_True;

        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_uInt32 DataSupplier::totalCount()
    {
        ::osl::ClearableGuard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );
        return m_pImpl->m_aResults.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_uInt32 DataSupplier::currentCount()
    {
        return m_pImpl->m_aResults.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool DataSupplier::isCountFinal()
    {
        return sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XRow > DataSupplier::queryPropertyValues( sal_uInt32 i_nIndex  )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );
        ENSURE_OR_RETURN( i_nIndex < m_pImpl->m_aResults.size(), "DataSupplier::queryPropertyValues: illegal index!", NULL );

        Reference< XRow > xRow = m_pImpl->m_aResults[ i_nIndex ].xRow;
        if ( xRow.is() )
            return xRow;

        xRow = Content::getPropertyValues( m_pImpl->m_xSMgr, getResultSet()->getProperties(),
            m_pImpl->m_aResults[ i_nIndex ].aProperties );
        return xRow;
    }

    //------------------------------------------------------------------------------------------------------------------
    void DataSupplier::releasePropertyValues( sal_uInt32 i_nIndex )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( i_nIndex < m_pImpl->m_aResults.size() )
            m_pImpl->m_aResults[ i_nIndex ].xRow.clear();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DataSupplier::close()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void DataSupplier::validate() throw( ResultSetException )
    {
    }

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................
