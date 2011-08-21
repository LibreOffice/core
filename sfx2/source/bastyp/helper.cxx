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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

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
#include <rtl/strbuf.hxx>

#include <tools/ref.hxx>
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
    DateTime aDT;
    CONVERT_DATETIME( rDT, aDT );
    String aDateStr = rWrapper.getDate( aDT );
    aDateStr += String::CreateFromAscii( ", " );
    aDateStr += rWrapper.getTime( aDT );
    rRow += aDateStr;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::IsDocument( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        bRet = aCnt.isDocument();
    }
    catch( const ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( const ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( const ucb::ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( const uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

uno::Sequence < OUString > SfxContentHelper::GetResultSet( const String& rURL )
{
    StringList_Impl* pList = NULL;
    try
    {
        ::ucbhelper::Content aCnt( rURL, uno::Reference< ucb::XCommandEnvironment >() );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
        uno::Sequence< OUString > aProps(3);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        pProps[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("ContentType"));
        pProps[2] = OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder"));

        try
        {
            xDynResultSet = aCnt.createDynamicCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( const ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( const uno::Exception& )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
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
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( const uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( const uno::Exception& e )
    {
        (void) e;
        DBG_ERRORFILE(
            rtl::OUStringToOString(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "GetResultSet: Any other exception: ")) +
                 e.Message),
                RTL_TEXTENCODING_UTF8).
            getStr());
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
                    xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), uno::UNO_QUERY );

        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ) );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        pProps[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder"));

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
                    xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), uno::UNO_QUERY );
        ::ucbhelper::Content aCnt( rURL, new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ) );
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
                      uno::Reference< ucb::XCommandEnvironment > () );
        if ( !( aCnt.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsErrorDocument")) ) >>= bRet ) )
        {
            DBG_ERRORFILE( "Property 'IsErrorDocument' is missing" );
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
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Size")) ) >>= nTemp;
    }
    catch( const ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( const uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
    nSize = (sal_uInt32)nTemp;
    return nSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
