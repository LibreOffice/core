/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>

#include <com/sun/star/ucb/OpenMode.hpp>

#include "cmis_datasupplier.hxx"
#include "cmis_content.hxx"
#include "cmis_provider.hxx"

#define STD_TO_OUSTR( str ) rtl::OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace com::sun::star;
using namespace std;

namespace cmis
{

    typedef std::vector< ResultListEntry* > ResultList;

    DataSupplier::DataSupplier( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        const uno::Reference< ::cmis::Content >& rContent, sal_Int32 nOpenMode )
        : mxContent(rContent), m_xSMgr(rxSMgr), mnOpenMode(nOpenMode), mbCountFinal(false)
    {
    }

    bool DataSupplier::getData()
    {
        if ( mbCountFinal )
            return true;

        libcmis::ObjectPtr pObject = mxContent->getObject();
        libcmis::Folder* pFolder = dynamic_cast< libcmis::Folder* >( pObject.get( ) );
        if ( NULL != pFolder )
        {
            // Get the children from pObject
            try
            {
                vector< libcmis::ObjectPtr > children = pFolder->getChildren( );

                // Loop over the results and filter them
                for ( vector< libcmis::ObjectPtr >::iterator it = children.begin();
                        it != children.end(); ++it )
                {
                    bool bIsFolder = ( *it )->getBaseType( ) == "cmis:folder";
                    if ( ( mnOpenMode == ucb::OpenMode::FOLDERS && bIsFolder ) ||
                         ( mnOpenMode == ucb::OpenMode::DOCUMENTS && !bIsFolder ) ||
                         ( mnOpenMode == ucb::OpenMode::ALL ) )
                    {
                        maResults.push_back( new ResultListEntry( *it ) );
                    }
                }
                mbCountFinal = sal_True;

                return true;
            }
            catch ( const libcmis::Exception& e )
            {
                SAL_INFO( "cmisucp", "Exception thrown: " << e.what() );
                return false;
            }
        }

        return false;
    }

    DataSupplier::~DataSupplier()
    {
    }

    ::rtl::OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
    {
        if ( nIndex < maResults.size() )
        {
            ::rtl::OUString aId = maResults[ nIndex ]->aId;
            if ( aId.getLength() )
            {
                // Already cached.
                return aId;
            }
        }

        if ( getResult( nIndex ) )
        {
            string sObjectPath;
            vector< string > paths = maResults[nIndex]->pObject->getPaths( );
            if ( paths.size( ) > 0 )
                sObjectPath = paths.front( );
            else
            {
                // Handle the unfiled objects with their id...
                // They manage to sneak here if we don't have the permission to get the object
                // parents (and then the path)
                sObjectPath += "#" + maResults[nIndex]->pObject->getId( );
            }

            // Get the URL from the Path
            URL aUrl( mxContent->getIdentifier( )->getContentIdentifier( ) );
            aUrl.setObjectPath( STD_TO_OUSTR( sObjectPath ) );
            rtl::OUString aId = aUrl.asString( );

            maResults[ nIndex ]->aId = aId;
            return aId;
        }

        return ::rtl::OUString();
    }

    uno::Reference< ucb::XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
    {
        if ( nIndex < maResults.size() )
        {
            uno::Reference< ucb::XContentIdentifier > xId = maResults[ nIndex ]->xId;
            if ( xId.is() )
            {
                // Already cached.
                return xId;
            }
        }

        ::rtl::OUString aId = queryContentIdentifierString( nIndex );
        if ( aId.getLength() )
        {
            uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aId );
            maResults[ nIndex ]->xId = xId;
            return xId;
        }

        return uno::Reference< ucb::XContentIdentifier >();
    }

    uno::Reference< ucb::XContent > DataSupplier::queryContent( sal_uInt32 nIndex )
    {
        if ( nIndex < maResults.size() )
        {
            uno::Reference< ucb::XContent > xContent = maResults[ nIndex ]->xContent;
            if ( xContent.is() )
            {
                // Already cached.
                return xContent;
            }
        }

        uno::Reference< ucb::XContentIdentifier > xId = queryContentIdentifier( nIndex );
        if ( xId.is() )
        {
            try
            {
                uno::Reference< ucb::XContent > xContent = mxContent->getProvider()->queryContent( xId );
                maResults[ nIndex ]->xContent = xContent;
                return xContent;
            }
            catch ( ucb::IllegalIdentifierException& )
            {
            }
        }
        return uno::Reference< ucb::XContent >();
    }

    sal_Bool DataSupplier::getResult( sal_uInt32 nIndex )
    {
        if ( maResults.size() > nIndex ) // Result already present.
            return sal_True;

        if ( getData() && maResults.size() > nIndex )
            return sal_True;

        return sal_False;
    }

    sal_uInt32 DataSupplier::totalCount()
    {
        getData();
        return maResults.size();
    }

    sal_uInt32 DataSupplier::currentCount()
    {
        return maResults.size();
    }

    sal_Bool DataSupplier::isCountFinal()
    {
        return mbCountFinal;
    }

    uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
    {
        if ( nIndex < maResults.size() )
        {
            uno::Reference< sdbc::XRow > xRow = maResults[ nIndex ]->xRow;
            if ( xRow.is() )
            {
                // Already cached.
                return xRow;
            }
        }

        if ( getResult( nIndex ) )
        {
            uno::Reference< ucb::XContent > xContent( queryContent( nIndex ) );
            if ( xContent.is() )
            {
                try
                {
                    uno::Reference< ucb::XCommandProcessor > xCmdProc(
                        xContent, uno::UNO_QUERY_THROW );
                    sal_Int32 nCmdId( xCmdProc->createCommandIdentifier() );
                    ucb::Command aCmd;
                    aCmd.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getPropertyValues"));
                    aCmd.Handle = -1;
                    aCmd.Argument <<= getResultSet()->getProperties();
                    uno::Any aResult( xCmdProc->execute(
                        aCmd, nCmdId, getResultSet()->getEnvironment() ) );
                    uno::Reference< sdbc::XRow > xRow;
                    if ( aResult >>= xRow )
                    {
                        maResults[ nIndex ]->xRow = xRow;
                        return xRow;
                    }
                }
                catch ( uno::Exception const & )
                {
                }
            }
        }
        return uno::Reference< sdbc::XRow >();
    }

    void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
    {
        if ( nIndex < maResults.size() )
            maResults[ nIndex ]->xRow = uno::Reference< sdbc::XRow >();
    }

    void DataSupplier::close()
    {
    }

    void DataSupplier::validate() throw( ucb::ResultSetException )
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
