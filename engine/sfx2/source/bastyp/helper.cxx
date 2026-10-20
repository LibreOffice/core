/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <string_view>

#include <helper.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace ::cpo;
using namespace comphelper;

using ::std::vector;


std::vector<OUString> SfxContentHelper::GetResultSet( const OUString& rURL )
{
    vector<OUString> aList;
    try
    {
        ::ucbhelper::Content aCnt( rURL, uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Reference< ucb::XDynamicResultSet > xDynResultSet;

        try
        {
            xDynResultSet = aCnt.createDynamicCursor( { u"Title"_ustr, u"ContentType"_ustr, u"IsFolder"_ustr } );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( const cpo::uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.bastyp", "GetResultSet" );
        }


        if ( xResultSet.is() )
        {
            uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            try
            {
                while ( xResultSet->next() )
                {
                    OUString aTitle( xRow->getString(1) );
                    OUString aType( xRow->getString(2) );
                    OUString aRow = aTitle +
                        "\t" +
                        aType +
                        "\t" +
                        xContentAccess->queryContentIdentifierString();
                    aList.push_back( aRow );
                }
            }
            catch( const cpo::uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION( "sfx.bastyp", "XContentAccess::next()" );
            }
        }
    }
    catch( const cpo::uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.bastyp", "GetResultSet");
    }

    return aList;
}


OUString SfxContentHelper::GetActiveHelpString( const OUString& rURL )
{
    OUStringBuffer aRet;
    try
    {
        const uno::Reference< cpo::uno::XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
        uno::Reference< task::XInteractionHandler > xInteractionHandler(
            task::InteractionHandler::createWithParent(xContext, nullptr), uno::UNO_QUERY_THROW );
        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ), comphelper::getProcessComponentContext() );
        // open the "active help" stream
        uno::Reference< io::XInputStream > xStream = aCnt.openStream();
        // and convert it to a String
        cpo::uno::Sequence< sal_Int8 > lData;
        sal_Int32 nRead = xStream->readBytes( lData, 1024 );
        while ( nRead > 0 )
        {
            std::string_view sOldString( reinterpret_cast<char const *>(lData.getConstArray()), nRead );
            OUString sString = OStringToOUString( sOldString, RTL_TEXTENCODING_UTF8 );
            aRet.append( sString );

            nRead = xStream->readBytes( lData, 1024 );
        }
    }
    catch( const cpo::uno::Exception& )
    {
    }

    return aRet.makeStringAndClear();
}


sal_Int64 SfxContentHelper::GetSize( std::u16string_view rContent )
{
    sal_Int64 nSize = 0;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.getPropertyValue( u"Size"_ustr ) >>= nSize;
    }
    catch( const cpo::uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.bastyp", "" );
    }
    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
