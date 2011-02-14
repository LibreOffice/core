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

using namespace com::sun::star;
using namespace rtl;
using namespace comphelper;
using namespace osl;

DECLARE_LIST( StringList_Impl, OUString* )

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

// SfxContentHelper ------------------------------------------------------

sal_Bool SfxContentHelper::Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData, sal_Int32 nNameClash )
{
    sal_Bool bRet = sal_True, bKillSource = sal_False;
    INetURLObject aSourceObj( rSource );
    DBG_ASSERT( aSourceObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    INetURLObject aDestObj( rDest );
    DBG_ASSERT( aDestObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    if ( bMoveData && aSourceObj.GetProtocol() != aDestObj.GetProtocol() )
    {
        bMoveData = sal_False;
        bKillSource = sal_True;
    }
    String aName = aDestObj.getName();
    aDestObj.removeSegment();
    aDestObj.setFinalSlash();

    try
    {
        ::ucbhelper::Content aDestPath( aDestObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        uno::Reference< ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        OUString aTransferName = OUString::createFromAscii( "transfer" );
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, uno::makeAny(
                ucb::TransferInfo( bMoveData, aSourceObj.GetMainURL( INetURLObject::NO_DECODE ), aName, nNameClash ) ) );
        }
        else
        {
            DBG_ERRORFILE( "transfer command not available" );
        }
    }
    catch( ucb::CommandAbortedException& )
    {
        bRet = sal_False;
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
        bRet = sal_False;
    }

    if ( bKillSource )
        SfxContentHelper::Kill( rSource );

    return bRet;
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
    catch( ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ucb::ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::IsFolder( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        bRet = aCnt.isFolder();
    }
    catch( ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ucb::ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::GetTitle( const String& rContent, String& rTitle )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        OUString aTemp;
        aCnt.getPropertyValue( OUString::createFromAscii( "Title" ) ) >>= aTemp;
        rTitle = String( aTemp );
        bRet = sal_True;
    }
    catch( ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::Kill( const String& rContent )
{
    sal_Bool bRet = sal_True;
    INetURLObject aDeleteObj( rContent );
    DBG_ASSERT( aDeleteObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        ::ucbhelper::Content aCnt( aDeleteObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), uno::makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}

// -----------------------------------------------------------------------

uno::Sequence < OUString > SfxContentHelper::GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted )
{
    StringList_Impl* pFiles = NULL;
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "IsFolder" );

        try
        {
            ::ucbhelper::ResultSetInclude eInclude = bFolder ? ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS : ::ucbhelper::INCLUDE_DOCUMENTS_ONLY;
            if ( !bSorted )
            {
                xResultSet = aCnt.createCursor( aProps, eInclude );
            }
            else
            {
                uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
                xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

                uno::Reference < ucb::XAnyCompareFactory > xFactory;
                uno::Reference < lang::XMultiServiceFactory > xMgr = getProcessServiceFactory();
                uno::Reference < ucb::XSortedDynamicResultSetFactory > xSRSFac(
                    xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), uno::UNO_QUERY );

                uno::Sequence< ucb::NumberedSortingInfo > aSortInfo( 2 );
                ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
                pInfo[ 0 ].ColumnIndex = 2;
                pInfo[ 0 ].Ascending   = sal_False;
                pInfo[ 1 ].ColumnIndex = 1;
                pInfo[ 1 ].Ascending   = sal_True;

                uno::Reference< ucb::XDynamicResultSet > xDynamicResultSet;
                xDynamicResultSet =
                    xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xFactory );
                if ( xDynamicResultSet.is() )
                {
                    xResultSet = xDynamicResultSet->getStaticResultSet();
                }
            }
        }
        catch( ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( uno::Exception& )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pFiles = new StringList_Impl;
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    OUString aId = xContentAccess->queryContentIdentifierString();
                    OUString* pFile = new OUString( aId );
                    pFiles->Insert( pFile, LIST_APPEND );
                }
            }
            catch( ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( pFiles )
    {
        sal_uIntPtr nCount = pFiles->Count();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( sal_uIntPtr i = 0; i < nCount; ++i )
        {
            OUString* pFile = pFiles->GetObject(i);
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return uno::Sequence < OUString > ();
}

// -----------------------------------------------------------------------

uno::Sequence < OUString > SfxContentHelper::GetFolderContentProperties( const String& rFolder, sal_Bool bIsFolder )
{
    StringList_Impl* pProperties = NULL;
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        uno::Reference< task::XInteractionHandler > xInteractionHandler = uno::Reference< task::XInteractionHandler > (
                    xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), uno::UNO_QUERY );

        ::ucbhelper::Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), new ::ucbhelper::CommandEnvironment( xInteractionHandler, uno::Reference< ucb::XProgressHandler >() ) );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Sequence< OUString > aProps(5);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "ContentType" );
        pProps[2] = OUString::createFromAscii( "Size" );
        pProps[3] = OUString::createFromAscii( "DateModified" );
        pProps[4] = OUString::createFromAscii( "IsFolder" );

        try
        {
            uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
            ::ucbhelper::ResultSetInclude eInclude = bIsFolder ? ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS : ::ucbhelper::INCLUDE_DOCUMENTS_ONLY;
            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            uno::Reference < ucb::XAnyCompareFactory > xCmpFactory;
            uno::Reference < lang::XMultiServiceFactory > xMgr = getProcessServiceFactory();
            uno::Reference < ucb::XSortedDynamicResultSetFactory > xSRSFac(
                xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), uno::UNO_QUERY );

            uno::Sequence< ucb::NumberedSortingInfo > aSortInfo( 2 );
            ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 5;
            pInfo[ 0 ].Ascending   = sal_False;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = sal_True;

            uno::Reference< ucb::XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xCmpFactory );
            if ( xDynamicResultSet.is() )
            {
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }

//          if ( xDynResultSet.is() )
//              xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( uno::Exception& )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
            pProperties = new StringList_Impl;
            uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
            sal_uIntPtr nFolderPos = LIST_APPEND;

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    String aType( xRow->getString(2) );
                    sal_Int64 nSize = xRow->getLong(3);
                    util::DateTime aDT = xRow->getTimestamp(4);
                    sal_Bool bFolder = xRow->getBoolean(5);

                    String aRow = aTitle;
                    aRow += '\t';
//!                 aRow += aType;
//!                 aRow += '\t';
                    aRow += String::CreateFromInt64( nSize );
                    aRow += '\t';
                    AppendDateTime_Impl( aDT, aRow, aLocaleWrapper );
                    aRow += '\t';
                    aRow += String( xContentAccess->queryContentIdentifierString() );
                    aRow += '\t';
                    aRow += bFolder ? '1' : '0';
                    OUString* pRow = new OUString( aRow );
                    sal_uIntPtr nPos = LIST_APPEND;
                    if ( bFolder )
                    {
                        if ( LIST_APPEND == nFolderPos )
                            nFolderPos = 0;
                        else
                            nFolderPos++;
                        nPos = nFolderPos;
                    }
                    pProperties->Insert( pRow, nPos );
                }
            }
            catch( ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( pProperties )
    {
        sal_uIntPtr nCount = pProperties->Count();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( sal_uIntPtr i = 0; i < nCount; ++i )
        {
            OUString* pProperty = pProperties->GetObject(i);
            pRet[i] = *( pProperty );
            delete pProperty;
        }
        delete pProperties;
        return aRet;
    }
    else
        return uno::Sequence < OUString > ();
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
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "ContentType" );
        pProps[2] = OUString::createFromAscii( "IsFolder" );

        try
        {
            xDynResultSet = aCnt.createDynamicCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( uno::Exception& )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pList = new StringList_Impl;
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
                    pList->Insert( pRow, LIST_APPEND );
                }
            }
            catch( ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( uno::Exception& e )
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
        sal_uIntPtr nCount = pList->Count();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( sal_uIntPtr i = 0; i < nCount; ++i )
        {
            OUString* pEntry = pList->GetObject(i);
            pRet[i] = *( pEntry );
            delete pEntry;
        }
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
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "IsFolder" );

        try
        {
            uno::Reference< ucb::XDynamicResultSet > xDynResultSet;
            xDynResultSet = aCnt.createDynamicCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ucb::CommandAbortedException& )
        {
        }
        catch( uno::Exception& )
        {
        }

        if ( xResultSet.is() )
        {
            pProperties = new StringList_Impl;
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
                    pProperties->Insert( pRow, LIST_APPEND );
                }
            }
            catch( ucb::CommandAbortedException& )
            {
            }
            catch( uno::Exception& )
            {
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    if ( pProperties )
    {
        sal_uIntPtr nCount = pProperties->Count();
        uno::Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( sal_uIntPtr i = 0; i < nCount; ++i )
        {
            OUString* pProperty = pProperties->GetObject(i);
            pRet[i] = *( pProperty );
            delete pProperty;
        }
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
    catch( uno::Exception& )
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
        if ( !( aCnt.getPropertyValue( OUString::createFromAscii( "IsErrorDocument" ) ) >>= bRet ) )
        {
            DBG_ERRORFILE( "Property 'IsErrorDocument' is missing" );
        }
    }
    catch( uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::CopyTo( const String& rSource, const String& rDest )
{
    return Transfer_Impl( rSource, rDest, sal_False, ucb::NameClash::ERROR );
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash )
{
    return Transfer_Impl( rSource, rDest, sal_True, nNameClash );
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::MakeFolder( const String& rFolder )
{
    INetURLObject aURL( rFolder );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    String aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    aURL.removeSegment();
    uno::Sequence < OUString > aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) );
    uno::Sequence<uno::Any> aValues(2);
    uno::Any* pValues = aValues.getArray();
    pValues[0] = uno::makeAny( OUString( aTitle ) );
    pValues[1] = uno::makeAny( sal_Bool( sal_True ) );
    uno::Reference< ucb::XCommandEnvironment > aCmdEnv;
    sal_Bool bRet = sal_False;
    try
    {
        ::ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        ::ucbhelper::Content aNewFolder;
        OUString aType( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.staroffice.fsys-folder" ) );
        bRet = aCnt.insertNewContent( aType, aNames, aValues, aNewFolder );
    }
    catch( ucb::CommandAbortedException& )
    {
        // double name?
    }
    catch( ucb::IllegalIdentifierException& )
    {
        DBG_ERRORFILE( "Illegal identifier" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

ErrCode SfxContentHelper::QueryDiskSpace( const String& rPath, sal_Int64& rFreeBytes )
{
    ErrCode nErr = 0;
    rFreeBytes = 0;
    INetURLObject aObj( rPath );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString::createFromAscii( "FreeSpace" ) ) >>= rFreeBytes;
    }
    catch( ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
        nErr = ERRCODE_IO_GENERAL;
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
        nErr = ERRCODE_IO_GENERAL;
    }
    return nErr;
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
        aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= nTemp;
    }
    catch( ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
    nSize = (sal_uInt32)nTemp;
    return nSize;
}

// -----------------------------------------------------------------------
// please don't use it (only used in appbas.cxx and appcfg.cxx)
sal_Bool SfxContentHelper::Exists( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        // just try to get the property; if no exception is thrown, the content exists!
        aCnt.isDocument();
        bRet = sal_True;
    }
    catch( ucb::CommandAbortedException& )
    {
            DBG_WARNING( "CommandAbortedException" );
    }
    catch( ucb::IllegalIdentifierException& )
    {
            DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ucb::ContentCreationException& )
    {
            DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( uno::Exception& )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;

}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::Find( const String& rFolder, const String& rName, String& rFile )
{
    sal_Bool bRet = sal_False;
    rtl::OUString aFile;

    if ( FileBase::searchFileURL( rName, rFolder, aFile ) == FileBase::E_None )
    {
        rFile = aFile;
        bRet = sal_True;
    }

    return bRet;
}


