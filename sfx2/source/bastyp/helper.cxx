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
#include <rtl/oustringostreaminserter.hxx>
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

using ::rtl::OUString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;

typedef vector< OUString* > StringList_Impl;

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

void AppendDateTime_Impl( const util::DateTime rDT,
                          String& rRow, const LocaleDataWrapper& rWrapper )
{
    DateTime aDT( DateTime::EMPTY );
    CONVERT_DATETIME( rDT, aDT );
    String aDateStr = rWrapper.getDate( aDT );
    aDateStr += rtl::OUString(", ");
    aDateStr += rWrapper.getTime( aDT );
    rRow += aDateStr;
}

// -----------------------------------------------------------------------

uno::Sequence < OUString > SfxContentHelper::GetResultSet( const String& rURL )
{
    StringList_Impl* pList = NULL;
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
            SAL_WARN( "sfx2.bastyp", "GetResultSet: CommandAbortedException" );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "sfx2.bastyp", "GetResultSet: Any other exception" );
        }


        if ( xResultSet.is() )
        {
            pList = new StringList_Impl();
            uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    String aType( xRow->getString(2) );
                    String aRow = aTitle;
                    aRow += '\t';
                    aRow += aType;
                    aRow += '\t';
                    aRow += String( xContentAccess->queryContentIdentifierString() );
                    OUString* pRow = new OUString( aRow );
                    pList->push_back( pRow );
                }
            }
            catch( const ucb::CommandAbortedException& )
            {
                SAL_WARN( "sfx2.bastyp", "XContentAccess::next(): CommandAbortedException" );
            }
            catch( const uno::Exception& )
            {
                SAL_WARN( "sfx2.bastyp", "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN( "sfx2.bastyp", "GetResultSet: Any other exception: " << e.Message );
    }

    if ( pList )
    {
        size_t nCount = pList->size();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            OUString* pEntry = pList->at(i);
            pRet[i] = *( pEntry );
            delete pEntry;
        }
        pList->clear();
        delete pList;
        return aRet;
    }
    else
        return uno::Sequence < OUString > ();
}

// -----------------------------------------------------------------------

uno::Sequence< OUString > SfxContentHelper::GetHelpTreeViewContents( const String& rURL )
{
    StringList_Impl* pProperties = NULL;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        uno::Reference< task::XInteractionHandler > xInteractionHandler = uno::Reference< task::XInteractionHandler > (
                    xFactory->createInstance( "com.sun.star.task.InteractionHandler" ), uno::UNO_QUERY );

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
            pProperties = new StringList_Impl();
            uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    sal_Bool bFolder = xRow->getBoolean(2);
                    String aRow = aTitle;
                    aRow += '\t';
                    aRow += String( xContentAccess->queryContentIdentifierString() );
                    aRow += '\t';
                    aRow += bFolder ? '1' : '0';
                    OUString* pRow = new OUString( aRow );
                    pProperties->push_back( pRow );
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

    if ( pProperties )
    {
        size_t nCount = pProperties->size();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            OUString* pProperty = pProperties->at(i);
            pRet[i] = *( pProperty );
            delete pProperty;
        }
        pProperties->clear();
        delete pProperties;
        return aRet;
    }
    else
        return uno::Sequence < OUString > ();
}

// -----------------------------------------------------------------------

String SfxContentHelper::GetActiveHelpString( const String& rURL )
{
    String aRet;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        uno::Reference< task::XInteractionHandler > xInteractionHandler = uno::Reference< task::XInteractionHandler > (
                    xFactory->createInstance( "com.sun.star.task.InteractionHandler" ), uno::UNO_QUERY );
        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ), comphelper::getProcessComponentContext() );
        // open the "active help" stream
        uno::Reference< io::XInputStream > xStream = aCnt.openStream();
        // and convert it to a String
        uno::Sequence< sal_Int8 > lData;
        sal_Int32 nRead = xStream->readBytes( lData, 1024 );
        while ( nRead > 0 )
        {
            OStringBuffer sBuffer( nRead );
            for( sal_Int32 i = 0; i < nRead; ++i )
                sBuffer.append( (sal_Char)lData[i] );
            OUString sString = OStringToOUString( sBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
            aRet += String( sString );

            nRead = xStream->readBytes( lData, 1024 );
        }
    }
    catch( const uno::Exception& )
    {
    }

    return aRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::IsHelpErrorDocument( const String& rURL )
{
    sal_Bool bRet = sal_False;
    try
    {
        ::ucbhelper::Content aCnt( INetURLObject( rURL ).GetMainURL( INetURLObject::NO_DECODE ),
                      uno::Reference< ucb::XCommandEnvironment >(),
                      comphelper::getProcessComponentContext() );
        if ( !( aCnt.getPropertyValue( "IsErrorDocument" ) >>= bRet ) )
        {
            SAL_WARN( "sfx2.bastyp", "Property 'IsErrorDocument' is missing" );
        }
    }
    catch( const uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_uIntPtr SfxContentHelper::GetSize( const String& rContent )
{
    sal_uIntPtr nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.getPropertyValue( "Size" ) >>= nTemp;
    }
    catch( const ucb::CommandAbortedException& )
    {
        SAL_WARN( "sfx2.bastyp", "CommandAbortedException" );
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "sfx2.bastyp", "Any other exception" );
    }
    nSize = (sal_uInt32)nTemp;
    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
