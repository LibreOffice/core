/*************************************************************************
 *
 *  $RCSfile: webdavdatasupplier.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:55:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

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

#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace rtl;
using namespace ucb;

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
    OUString            aId;
    Reference< XContentIdentifier > xId;
    Reference< XContent >       xContent;
    Reference< XRow >       xRow;
    const ContentProperties*    pData;

    ResultListEntry( const ContentProperties* pEntry ) : pData( pEntry ) {};
     ~ResultListEntry() {delete pData;}
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
    osl::Mutex                        m_aMutex;
    ResultList                        m_aResults;
    vos::ORef< Content >              m_xContent;
    Reference< XMultiServiceFactory > m_xSMgr;
      sal_Bool                        m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                       const vos::ORef< Content >& rContent )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
      m_bCountFinal( sal_False ) {}
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

DataSupplier::DataSupplier( const Reference< XMultiServiceFactory >& rxSMgr,
                const vos::ORef< Content >& rContent,
                sal_Int32 nOpenMode,
                std::vector< DAVResource >& resources)
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent ) )
{
    for (sal_Int32 i=0; i<resources.size()-1; i++)
    {
      ContentProperties* pContentProperty = new ContentProperties;
      pContentProperty->setValues(resources[i]);

      // Check resource against open mode.
      sal_Bool bMatchesOpenMode = sal_True;

      switch ( nOpenMode )
      {
        case OpenMode::FOLDERS:
            if ( !pContentProperty->bIsFolder )
            {
                // Entry is a document.
                bMatchesOpenMode = sal_False;
            }
            break;

        case OpenMode::DOCUMENTS:
            if ( !pContentProperty->bIsDocument )
            {
                // Entry is a folder.
                bMatchesOpenMode = sal_False;
            }
            break;

        case OpenMode::ALL:
        default:
            break;
      }

      if ( bMatchesOpenMode )
          m_pImpl->m_aResults.push_back(new ResultListEntry( pContentProperty ) );
    }


    m_pImpl->m_bCountFinal = sal_True;
    vos::ORef< ResultSet > xResultSet = getResultSet();
    if ( xResultSet.isValid() )
    {
      xResultSet->rowCountFinal();
    }
}

//=========================================================================
// virtual
DataSupplier::~DataSupplier()
{
    delete m_pImpl;
}

//=========================================================================
// virtual
OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        OUString aId
            = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();

        const ContentProperties& props = *(m_pImpl->m_aResults[ nIndex ]->pData);

//      fprintf(stderr,OUStringToOString( props.aTitle, RTL_TEXTENCODING_ASCII_US ));
//      fprintf(stderr, "\n");

//      fprintf(stderr,OUStringToOString(aId, RTL_TEXTENCODING_ASCII_US ));
//      fprintf(stderr, "\n");

//      fprintf(stderr, "size : %d\n", m_pImpl->m_aResults.size());

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += OUString::createFromAscii( "/" );

        aId += props.aTitle;

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}

//=========================================================================
// virtual
Reference< XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContentIdentifier > xId
                                = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        Reference< XContentIdentifier > xId = new ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return Reference< XContentIdentifier >();
}

//=========================================================================
// virtual
Reference< XContent > DataSupplier::queryContent( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContent > xContent = m_pImpl->m_aResults[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    Reference< XContentIdentifier > xId = queryContentIdentifier( nIndex );
    if ( xId.is() )
    {
        try
        {
            Reference< XContent > xContent
                = m_pImpl->m_xContent->getProvider()->queryContent( xId );
            m_pImpl->m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( IllegalIdentifierException& )
        {
        }
    }
    return Reference< XContent >();
}

//=========================================================================
// virtual
sal_Bool DataSupplier::getResult( sal_uInt32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_aResults.size() > nIndex )
    {
        // Result already present.
        return sal_True;
    }

    // Result not (yet) present.

    if ( m_pImpl->m_bCountFinal )
        return sal_False;

}

//=========================================================================
// virtual
sal_uInt32 DataSupplier::totalCount()
{
  return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_uInt32 DataSupplier::currentCount()
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
Reference< XRow > DataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( nIndex ) )
    {
        Reference< XRow > xRow = Content::getPropertyValues(
                                    m_pImpl->m_xSMgr,
                                    getResultSet()->getProperties(),
                                    *(m_pImpl->m_aResults[ nIndex ]->pData),
                                    m_pImpl->m_xContent->getProvider(),
                                    queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return Reference< XRow >();
}

//=========================================================================
// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = Reference< XRow >();
}

//=========================================================================
// virtual
void DataSupplier::close()
{
}

//=========================================================================
// virtual
void DataSupplier::validate()
    throw( ResultSetException )
{
}

