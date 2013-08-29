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


#include "helper.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <unotools/localedatawrapper.hxx>
#include <rtl/strbuf.hxx>

#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <vcl/svapp.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <vector>

using namespace com::sun::star;
using namespace comphelper;
using namespace osl;

using ::std::vector;


std::vector<OUString> SfxContentHelper::GetResultSet( const OUString& rURL )
{
    vector<OUString> aList;
    try
    {
        ::ucbhelper::Content aCnt( rURL, uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
        uno::Sequence< OUString > aProps(3);
        OUString* pProps = aProps.getArray();
        pProps[0] = "Title";
        pProps[1] = "ContentType";
        pProps[2] = "IsFolder";

        try
        {
            xDynResultSet = aCnt.createDynamicCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( const ucb::CommandAbortedException& )
        {
            SAL_WARN( "sfx.bastyp", "GetResultSet: CommandAbortedException" );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "sfx.bastyp", "GetResultSet: Any other exception" );
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
                    OUString aRow = aTitle;
                    aRow += "\t";
                    aRow += aType;
                    aRow += "\t";
                    aRow += xContentAccess->queryContentIdentifierString();
                    aList.push_back( OUString( aRow ) );
                }
            }
            catch( const ucb::CommandAbortedException& )
            {
                SAL_WARN( "sfx.bastyp", "XContentAccess::next(): CommandAbortedException" );
            }
            catch( const uno::Exception& )
            {
                SAL_WARN( "sfx.bastyp", "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN( "sfx.bastyp", "GetResultSet: Any other exception: " << e.Message );
    }

    return aList;
}

// -----------------------------------------------------------------------

std::vector< OUString > SfxContentHelper::GetHelpTreeViewContents( const OUString& rURL )
{
    vector< OUString > aProperties;
    try
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        uno::Reference< task::XInteractionHandler > xInteractionHandler(
            task::InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );

        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ), comphelper::getProcessComponentContext() );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = "Title";
        pProps[1] = "IsFolder";

        try
        {
            uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
            xDynResultSet = aCnt.createDynamicCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( const ucb::CommandAbortedException& )
        {
        }
        catch( const uno::Exception& )
        {
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
                    sal_Bool bFolder = xRow->getBoolean(2);
                    OUString aRow = aTitle;
                    aRow += "\t";
                    aRow += xContentAccess->queryContentIdentifierString();
                    aRow += "\t";
                    aRow += bFolder ? OUString("1") : OUString("0");
                    aProperties.push_back( aRow );
                }
            }
            catch( const ucb::CommandAbortedException& )
            {
            }
            catch( const uno::Exception& )
            {
            }
        }
    }
    catch( const uno::Exception& )
    {
    }

    return aProperties;
}

// -----------------------------------------------------------------------

OUString SfxContentHelper::GetActiveHelpString( const OUString& rURL )
{
    OUStringBuffer aRet;
    try
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        uno::Reference< task::XInteractionHandler > xInteractionHandler(
            task::InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );
        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ), comphelper::getProcessComponentContext() );
        // open the "active help" stream
        uno::Reference< io::XInputStream > xStream = aCnt.openStream();
        // and convert it to a String
        uno::Sequence< sal_Int8 > lData;
        sal_Int32 nRead = xStream->readBytes( lData, 1024 );
        while ( nRead > 0 )
        {
            OString sOldString( (sal_Char*)lData.getConstArray(), nRead );
            OUString sString = OStringToOUString( sOldString, RTL_TEXTENCODING_UTF8 );
            aRet.append( sString );

            nRead = xStream->readBytes( lData, 1024 );
        }
    }
    catch( const uno::Exception& )
    {
    }

    return aRet.makeStringAndClear();
}

// -----------------------------------------------------------------------

bool SfxContentHelper::IsHelpErrorDocument( const OUString& rURL )
{
    bool bRet = false;
    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( rURL ).GetMainURL( INetURLObject::NO_DECODE ),
                      uno::Reference< ucb::XCommandEnvironment >(),
                      comphelper::getProcessComponentContext() );
        if ( !( aCnt.getPropertyValue( "IsErrorDocument" ) >>= bRet ) )
        {
            SAL_WARN( "sfx.bastyp", "Property 'IsErrorDocument' is missing" );
        }
    }
    catch( const uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Int64 SfxContentHelper::GetSize( const OUString& rContent )
{
    sal_Int64 nSize = 0;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.getPropertyValue( "Size" ) >>= nSize;
    }
    catch( const ucb::CommandAbortedException& )
    {
        SAL_WARN( "sfx.bastyp", "CommandAbortedException" );
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "sfx.bastyp", "Any other exception" );
    }
    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
