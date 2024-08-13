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

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <utility>

#include "gio_datasupplier.hxx"
#include "gio_content.hxx"

using namespace gio;

namespace gio
{

DataSupplier::DataSupplier( rtl::Reference< ::gio::Content > xContent, sal_Int32 nOpenMode )
    : mxContent(std::move(xContent)), mnOpenMode(nOpenMode), mbCountFinal(false)
{
}

bool DataSupplier::getData()
{
    if (mbCountFinal)
        return true;

    GFile *pFile = mxContent->getGFile();

    GFileEnumerator* pEnumerator = g_file_enumerate_children(pFile, "*",
        G_FILE_QUERY_INFO_NONE, nullptr, nullptr);

    if (!pEnumerator)
        return false;

    GFileInfo *pInfo = nullptr;
    while ((pInfo = g_file_enumerator_next_file (pEnumerator, nullptr, nullptr)))
    {
        switch ( mnOpenMode )
        {
            case css::ucb::OpenMode::FOLDERS:
                if (g_file_info_get_file_type(pInfo) != G_FILE_TYPE_DIRECTORY)
                    continue;
                break;
            case css::ucb::OpenMode::DOCUMENTS:
                if (g_file_info_get_file_type(pInfo) != G_FILE_TYPE_REGULAR)
                    continue;
                break;
            case css::ucb::OpenMode::ALL:
            default:
                break;
        }

        maResults.emplace_back( new ResultListEntry( pInfo ) );
        g_object_unref(pInfo);
    }

    mbCountFinal = true;

    g_file_enumerator_close(pEnumerator, nullptr, nullptr);
    return true;
}

DataSupplier::~DataSupplier()
{
}

OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    if ( nIndex < maResults.size() )
    {
        OUString aId = maResults[ nIndex ]->aId;
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
        OUString aId = OUString::createFromAscii( parent );
        g_free(parent);

        char *escaped_name =
            g_uri_escape_string( g_file_info_get_name(maResults[ nIndex ]->pInfo) , nullptr, false);

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
                aId += "/";

        aId += OUString::createFromAscii( escaped_name );

        g_free( escaped_name );

        maResults[ nIndex ]->aId = aId;
        return aId;
    }

    return OUString();
}

css::uno::Reference< css::ucb::XContentIdentifier > DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    if ( nIndex < maResults.size() )
    {
        css::uno::Reference< css::ucb::XContentIdentifier > xId = maResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        css::uno::Reference< css::ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aId );
        maResults[ nIndex ]->xId = xId;
        return xId;
    }

    return css::uno::Reference< css::ucb::XContentIdentifier >();
}

css::uno::Reference< css::ucb::XContent > DataSupplier::queryContent( sal_uInt32 nIndex )
{
    if ( nIndex < maResults.size() )
    {
        css::uno::Reference< css::ucb::XContent > xContent = maResults[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    css::uno::Reference< css::ucb::XContentIdentifier > xId = queryContentIdentifier( nIndex );
    if ( xId.is() )
    {
        try
        {
            css::uno::Reference< css::ucb::XContent > xContent = mxContent->getProvider()->queryContent( xId );
            maResults[ nIndex ]->xContent = xContent;
            return xContent;
        }
        catch ( css::ucb::IllegalIdentifierException& )
        {
        }
    }
    return css::uno::Reference< css::ucb::XContent >();
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

css::uno::Reference< css::sdbc::XRow > DataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    if ( nIndex < maResults.size() )
    {
        css::uno::Reference< css::sdbc::XRow > xRow = maResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( !getResult( nIndex ) )
        return {};

    css::uno::Reference< css::ucb::XContent > xContent( queryContent( nIndex ) );
    if ( !xContent )
        return {};

    try
    {
        css::uno::Reference< css::ucb::XCommandProcessor > xCmdProc( xContent, css::uno::UNO_QUERY );
        if ( !xCmdProc )
            return {};
        sal_Int32 nCmdId( xCmdProc->createCommandIdentifier() );
        css::ucb::Command aCmd;
        aCmd.Name = "getPropertyValues";
        aCmd.Handle = -1;
        aCmd.Argument <<= getResultSet()->getProperties();
        css::uno::Any aResult( xCmdProc->execute(
            aCmd, nCmdId, getResultSet()->getEnvironment() ) );
        css::uno::Reference< css::sdbc::XRow > xRow;
        if ( aResult >>= xRow )
        {
            maResults[ nIndex ]->xRow = xRow;
            return xRow;
        }
    }
    catch ( css::uno::Exception const & )
    {
    }
    return css::uno::Reference< css::sdbc::XRow >();
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
