/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#include <vector>

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>

#include <com/sun/star/ucb/OpenMode.hpp>

#include "gio_datasupplier.hxx"
#include "gio_content.hxx"
#include "gio_provider.hxx"

#include <stdio.h>

using namespace com::sun::star;

using namespace gio;

namespace gio
{

typedef std::vector< ResultListEntry* > ResultList;

DataSupplier::DataSupplier( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
    const uno::Reference< ::gio::Content >& rContent, sal_Int32 nOpenMode )
    : mxContent(rContent), m_xSMgr(rxSMgr), mnOpenMode(nOpenMode), mbCountFinal(false)
{
}

bool DataSupplier::getData()
{
    if (mbCountFinal)
        return true;

    GFile *pFile = mxContent->getGFile();

    GFileEnumerator* pEnumerator = g_file_enumerate_children(pFile, "*",
        G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (!pEnumerator)
        return sal_False;

    GFileInfo *pInfo = NULL;
    while ((pInfo = g_file_enumerator_next_file (pEnumerator, NULL, NULL)))
    {
        switch ( mnOpenMode )
        {
            case ucb::OpenMode::FOLDERS:
                if (g_file_info_get_file_type(pInfo) != G_FILE_TYPE_DIRECTORY)
                    continue;
                break;
            case ucb::OpenMode::DOCUMENTS:
                if (g_file_info_get_file_type(pInfo) != G_FILE_TYPE_REGULAR)
                    continue;
                break;
            case ucb::OpenMode::ALL:
            default:
                break;
        }

        maResults.push_back( new ResultListEntry( pInfo ) );
        g_object_unref(pInfo);
    }

    mbCountFinal = sal_True;

    g_file_enumerator_close(pEnumerator, NULL, NULL);
    return true;
}

DataSupplier::~DataSupplier()
{
    ResultList::const_iterator it  = maResults.begin();
    ResultList::const_iterator end = maResults.end();

    while ( it != end )
    {
        delete (*it);
        ++it;
    }
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
        GFile *pFile = mxContent->getGFile();
        char* parent = g_file_get_uri(pFile);
        rtl::OUString aId = rtl::OUString::createFromAscii( parent );
        g_free(parent);

        char *escaped_name =
            g_uri_escape_string( g_file_info_get_name(maResults[ nIndex ]->pInfo) , NULL, false);

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
                aId += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

        aId += rtl::OUString::createFromAscii( escaped_name );

        g_free( escaped_name );

        maResults[ nIndex ]->aId = aId;
        return aId;

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
