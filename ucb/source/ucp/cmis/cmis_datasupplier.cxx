/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

using namespace com::sun::star;
using namespace std;

namespace cmis
{

    typedef std::vector< ResultListEntry* > ResultList;

    DataSupplier::DataSupplier( ChildrenProvider* pChildrenProvider, sal_Int32 nOpenMode )
        : m_pChildrenProvider( pChildrenProvider ), mnOpenMode(nOpenMode), mbCountFinal(false)
    {
    }

    bool DataSupplier::getData()
    {
        if ( mbCountFinal )
            return true;

        list< uno::Reference< ucb::XContent > > aChildren = m_pChildrenProvider->getChildren( );

        // Loop over the results and filter them
        for ( list< uno::Reference< ucb::XContent > >::iterator it = aChildren.begin();
                it != aChildren.end(); ++it )
        {
            OUString sContentType = ( *it )->getContentType( );
            bool bIsFolder = sContentType != CMIS_FILE_TYPE;
            if ( ( mnOpenMode == ucb::OpenMode::FOLDERS && bIsFolder ) ||
                 ( mnOpenMode == ucb::OpenMode::DOCUMENTS && !bIsFolder ) ||
                 ( mnOpenMode == ucb::OpenMode::ALL ) )
            {
                maResults.push_back( new ResultListEntry( *it ) );
            }
        }
        mbCountFinal = true;

        return true;
    }

    DataSupplier::~DataSupplier()
    {
        while ( maResults.size( ) > 0 )
        {
            ResultListEntry* back = maResults.back( );
            maResults.pop_back( );
            delete back;
        }
    }

    OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
    {
        auto const xTemp(queryContentIdentifier(nIndex));
        return (xTemp.is()) ? xTemp->getContentIdentifier() : OUString();
    }

    uno::Reference< ucb::XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
    {
        auto const xTemp(queryContent(nIndex));
        return (xTemp.is()) ? xTemp->getIdentifier() : uno::Reference<ucb::XContentIdentifier>();
    }

    uno::Reference< ucb::XContent > DataSupplier::queryContent( sal_uInt32 nIndex )
    {
        if (!getResult(nIndex))
            return uno::Reference<ucb::XContent>();

        return maResults[ nIndex ]->xContent;
    }

    bool DataSupplier::getResult( sal_uInt32 nIndex )
    {
        if ( maResults.size() > nIndex ) // Result already present.
            return true;

        if ( getData() && maResults.size() > nIndex )
            return true;

        return false;
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

    bool DataSupplier::isCountFinal()
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
                    aCmd.Name = "getPropertyValues";
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
            maResults[ nIndex ]->xRow.clear();
    }

    void DataSupplier::close()
    {
    }

    void DataSupplier::validate()
    {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
