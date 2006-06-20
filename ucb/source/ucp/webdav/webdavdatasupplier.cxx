/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webdavdatasupplier.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:38:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif

#ifndef _WEBDAV_UCP_DATASUPPLIER_HXX
#include "webdavdatasupplier.hxx"
#endif
#ifndef _WEBDAV_UCP_CONTENT_HXX
#include "webdavcontent.hxx"
#endif
#ifndef _WEBDAV_UCP_CONTENTPROPERTIES_HXX
#include "ContentProperties.hxx"
#endif
#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif
#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif

using namespace com::sun::star;
using namespace webdav_ucp;

namespace webdav_ucp
{

//=========================================================================
//
// struct ResultListEntry.
//
//=========================================================================

struct ResultListEntry
{
    rtl::OUString                                             aId;
    uno::Reference< com::sun::star::ucb::XContentIdentifier > xId;
    uno::Reference< com::sun::star::ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >                              xRow;
    const ContentProperties*                                  pData;

    ResultListEntry( const ContentProperties* pEntry ) : pData( pEntry ) {};
     ~ResultListEntry() { delete pData; }
};

//=========================================================================
//
// ResultList.
//
//=========================================================================

typedef std::vector< ResultListEntry* > ResultList;

//=========================================================================
//
// struct DataSupplier_Impl.
//
//=========================================================================

struct DataSupplier_Impl
{
    osl::Mutex                                   m_aMutex;
    ResultList                                   m_aResults;
    rtl::Reference< Content >                    m_xContent;
    uno::Reference< lang::XMultiServiceFactory > m_xSMgr;
    sal_Int32                                    m_nOpenMode;
    sal_Bool                                     m_bCountFinal;
    sal_Bool                                     m_bThrowException;

    DataSupplier_Impl(
                const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                const rtl::Reference< Content >& rContent,
                sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ), m_nOpenMode( nOpenMode ),
      m_bCountFinal( sal_False ), m_bThrowException( sal_False ) {}
    ~DataSupplier_Impl();
};

//=========================================================================
DataSupplier_Impl::~DataSupplier_Impl()
{
    ResultList::const_iterator it  = m_aResults.begin();
    ResultList::const_iterator end = m_aResults.end();

    while ( it != end )
    {
        delete (*it);
        it++;
    }
}

}

//=========================================================================
//=========================================================================
//
// DataSupplier Implementation.
//
//=========================================================================
//=========================================================================

DataSupplier::DataSupplier(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent, nOpenMode ) )
{
}

//=========================================================================
// virtual
DataSupplier::~DataSupplier()
{
    delete m_pImpl;
}

//=========================================================================
// virtual
rtl::OUString DataSupplier::queryContentIdentifierString( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
    {
        rtl::OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        rtl::OUString aId = m_pImpl->m_xContent->getResourceAccess().getURL();

        const ContentProperties& props
                            = *( m_pImpl->m_aResults[ nIndex ]->pData );

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += rtl::OUString::createFromAscii( "/" );

        aId += props.getEscapedTitle();

        if ( props.isTrailingSlash() )
            aId += rtl::OUString::createFromAscii( "/" );

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return rtl::OUString();
}

//=========================================================================
// virtual
uno::Reference< com::sun::star::ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
    {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
            = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    rtl::OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
            = new ::ucb::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< com::sun::star::ucb::XContentIdentifier >();
}

//=========================================================================
// virtual
uno::Reference< com::sun::star::ucb::XContent >
DataSupplier::queryContent( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
    {
        uno::Reference< com::sun::star::ucb::XContent > xContent
            = m_pImpl->m_aResults[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
        = queryContentIdentifier( nIndex );
    if ( xId.is() )
    {
        try
        {
            uno::Reference< com::sun::star::ucb::XContent > xContent
                = m_pImpl->m_xContent->getProvider()->queryContent( xId );
            m_pImpl->m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( com::sun::star::ucb::IllegalIdentifierException& )
        {
        }
    }
    return uno::Reference< com::sun::star::ucb::XContent >();
}

//=========================================================================
// virtual
sal_Bool DataSupplier::getResult( sal_Int32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) > nIndex )
    {
        // Result already present.
        return sal_True;
    }

    // Obtain values...
    if ( getData() )
    {
        if ( sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) > nIndex )
        {
            // Result already present.
            return sal_True;
        }
    }

    return sal_False;
}

//=========================================================================
// virtual
sal_Int32 DataSupplier::totalCount()
{
  // Obtain values...
  getData();

  return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_Int32 DataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_Bool DataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}

//=========================================================================
// virtual
uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues(
                                                    sal_Int32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
    {
        uno::Reference< sdbc::XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( nIndex ) )
    {
        uno::Reference< sdbc::XRow > xRow
            = Content::getPropertyValues(
                        m_pImpl->m_xSMgr,
                        getResultSet()->getProperties(),
                        *(m_pImpl->m_aResults[ nIndex ]->pData),
                        rtl::Reference< ::ucb::ContentProviderImplHelper >(
                            m_pImpl->m_xContent->getProvider().getBodyPtr() ),
                        queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}

//=========================================================================
// virtual
void DataSupplier::releasePropertyValues( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
        m_pImpl->m_aResults[ nIndex ]->xRow = uno::Reference< sdbc::XRow >();
}

//=========================================================================
// virtual
void DataSupplier::close()
{
}

//=========================================================================
// virtual
void DataSupplier::validate()
    throw( com::sun::star::ucb::ResultSetException )
{
    if ( m_pImpl->m_bThrowException )
        throw com::sun::star::ucb::ResultSetException();
}

//=========================================================================
sal_Bool DataSupplier::getData()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_bCountFinal )
    {
        std::vector< rtl::OUString > propertyNames;
        ContentProperties::UCBNamesToDAVNames(
                        getResultSet()->getProperties(), propertyNames );

        // Append "resourcetype", if not already present. It's value is
        // needed to get a valid ContentProperties::pIsFolder value, which
        // is needed for OpenMode handling.

        std::vector< rtl::OUString >::const_iterator it
            = propertyNames.begin();
        std::vector< rtl::OUString >::const_iterator end
            = propertyNames.end();

        while ( it != end )
        {
            if ( (*it).equals( DAVProperties::RESOURCETYPE ) )
                break;

            it++;
        }

        if ( it == end )
            propertyNames.push_back( DAVProperties::RESOURCETYPE );

        std::vector< DAVResource > resources;
        try
        {
            // propfind depth 1, get property values for parent AND for each
            // child
            m_pImpl->m_xContent->getResourceAccess()
                .PROPFIND( DAVONE,
                           propertyNames,
                           resources,
                           getResultSet()->getEnvironment() );
          }
          catch ( DAVException & )
        {
//          OSL_ENSURE( sal_False, "PROPFIND : DAVException" );
            m_pImpl->m_bThrowException = sal_True;
          }

        if ( !m_pImpl->m_bThrowException )
        {
            NeonUri aURI( m_pImpl->m_xContent->getResourceAccess().getURL() );
            rtl::OUString aPath = aURI.GetPath();
            if ( aPath.getStr()[ aPath.getLength() - 1 ] == sal_Unicode( '/' ) )
                aPath = aPath.copy( 0, aPath.getLength() - 1 );

            aPath = NeonUri::unescape( aPath );
            bool bFoundParent = false;

            for ( sal_uInt32 n = 0; n < resources.size(); ++n )
            {
                const DAVResource & rRes = resources[ n ];

                // Filter parent, which is contained somewhere(!) in the vector.
                if ( !bFoundParent )
                {
                    NeonUri aCurrURI( rRes.uri );
                    rtl::OUString aCurrPath = aCurrURI.GetPath();
                    if ( aCurrPath.getStr()[
                            aCurrPath.getLength() - 1 ] == sal_Unicode( '/' ) )
                        aCurrPath
                            = aCurrPath.copy( 0, aCurrPath.getLength() - 1 );

                    aCurrPath = NeonUri::unescape( aCurrPath );
                    if ( aPath == aCurrPath )
                    {
                        bFoundParent = true;
                        continue;
                    }
                }

                ContentProperties* pContentProperties
                    = new ContentProperties( rRes );

                // Check resource against open mode.
                switch ( m_pImpl->m_nOpenMode )
                {
                    case com::sun::star::ucb::OpenMode::FOLDERS:
                    {
                        sal_Bool bFolder = sal_False;

                        const uno::Any & rValue
                            = pContentProperties->getValue(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "IsFolder" ) ) );
                        rValue >>= bFolder;

                        if ( !bFolder )
                            continue;

                        break;
                    }

                    case com::sun::star::ucb::OpenMode::DOCUMENTS:
                    {
                        sal_Bool bDocument = sal_False;

                        const uno::Any & rValue
                            = pContentProperties->getValue(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "IsDocument" ) ) );
                        rValue >>= bDocument;

                        if ( !bDocument )
                            continue;

                        break;
                    }

                    case com::sun::star::ucb::OpenMode::ALL:
                    default:
                        break;
                }

                m_pImpl->m_aResults.push_back(
                                    new ResultListEntry( pContentProperties ) );
            }
        }

          m_pImpl->m_bCountFinal = sal_True;

        // Callback possible, because listeners may be informed!
        aGuard.clear();
        getResultSet()->rowCountFinal();
    }
    return !m_pImpl->m_bThrowException;
}

