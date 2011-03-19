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
#include "precompiled_unotools.hxx"

#include "unotools/ucbhelper.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include <tools/wldcrd.hxx>
#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <ucbhelper/contentbroker.hxx>

#include "unotools/localfilehelper.hxx"
#include <vector>

using namespace ucbhelper;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace comphelper;
using namespace osl;

using ::rtl::OUString;

typedef ::std::vector< OUString* > StringList_Impl;

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

namespace utl
{

sal_Bool UCBContentHelper::Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData, sal_Int32 nNameClash )
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
        Content aDestPath( aDestObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        uno::Reference< ::com::sun::star::ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        OUString aTransferName(RTL_CONSTASCII_USTRINGPARAM("transfer"));
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, makeAny(
                ::com::sun::star::ucb::TransferInfo( bMoveData, aSourceObj.GetMainURL( INetURLObject::NO_DECODE ), aName, nNameClash ) ) );
        }
        else
        {
            DBG_ERRORFILE( "transfer command not available" );
        }
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        bRet = sal_False;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bRet = sal_False;
    }

    if ( bKillSource )
        UCBContentHelper::Kill( rSource );

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::IsDocument( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isDocument();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_WARNING( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

Any UCBContentHelper::GetProperty( const String& rContent, const ::rtl::OUString& rName )
{
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        return aCnt.getPropertyValue( rName );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_WARNING( "Any other exception" );
    }

    return Any();
}

sal_Bool UCBContentHelper::IsFolder( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isFolder();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_WARNING( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::GetTitle( const String& rContent, String& rTitle )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        OUString aTemp;
        if ( aCnt.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Title")) ) >>= aTemp )
        {
            rTitle = String( aTemp );
            bRet = sal_True;
        }
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::Kill( const String& rContent )
{
    sal_Bool bRet = sal_True;
    INetURLObject aDeleteObj( rContent );
    DBG_ASSERT( aDeleteObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        Content aCnt( aDeleteObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString(RTL_CONSTASCII_USTRINGPARAM("delete")), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_WARNING( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}

// -----------------------------------------------------------------------

Sequence < OUString > UCBContentHelper::GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted )
{
    StringList_Impl* pFiles = NULL;
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps( bSorted ? 2 : 1 );
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        if ( bSorted )
            pProps[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder"));

        try
        {
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            if ( !bSorted )
            {
                xResultSet = aCnt.createCursor( aProps, eInclude );
            }
            else
            {
                uno::Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
                xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

                uno::Reference < com::sun::star::ucb::XAnyCompareFactory > xFactory;
                uno::Reference < XMultiServiceFactory > xMgr = getProcessServiceFactory();
                uno::Reference < com::sun::star::ucb::XSortedDynamicResultSetFactory > xSRSFac(
                    xMgr->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory")) ), UNO_QUERY );

                Sequence< com::sun::star::ucb::NumberedSortingInfo > aSortInfo( 2 );
                com::sun::star::ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
                pInfo[ 0 ].ColumnIndex = 2;
                pInfo[ 0 ].Ascending   = sal_False;
                pInfo[ 1 ].ColumnIndex = 1;
                pInfo[ 1 ].Ascending   = sal_True;

                uno::Reference< com::sun::star::ucb::XDynamicResultSet > xDynamicResultSet;
                xDynamicResultSet =
                    xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xFactory );
                if ( xDynamicResultSet.is() )
                {
                    xResultSet = xDynamicResultSet->getStaticResultSet();
                }
            }
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            // folder not exists?
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }

        if ( xResultSet.is() )
        {
            pFiles = new StringList_Impl;
            uno::Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    OUString aId = xContentAccess->queryContentIdentifierString();
                    OUString* pFile = new OUString( aId );
                    pFiles->push_back( pFile );
                }
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
            }
            catch( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    if ( pFiles )
    {
        size_t nCount = pFiles->size();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            OUString* pFile = (*pFiles)[ i ];
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

// -----------------------------------------------------------------------

Sequence < OUString > UCBContentHelper::GetResultSet( const String& rURL )
{
    StringList_Impl* pList = NULL;
    try
    {
        Content aCnt( rURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        uno::Reference< XResultSet > xResultSet;
        uno::Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
        Sequence< OUString > aProps(3);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        pProps[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("ContentType"));
        // TODO: can be optimized, property never used:
        pProps[2] = OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder"));

        try
        {
            xDynResultSet = aCnt.createDynamicCursor( aProps, INCLUDE_FOLDERS_AND_DOCUMENTS );
            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }

        if ( xResultSet.is() )
        {
            pList = new StringList_Impl;
            uno::Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            uno::Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

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
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
            }
            catch( ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    if ( pList )
    {
        size_t nCount = pList->size();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            OUString* pEntry = (*pList)[ i ];
            pRet[i] = *( pEntry );
            delete pEntry;
        }
        delete pList;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::CopyTo( const String& rSource, const String& rDest )
{
    return Transfer_Impl( rSource, rDest, sal_False, NameClash::ERROR );
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash )
{
    return Transfer_Impl( rSource, rDest, sal_True, nNameClash );
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::CanMakeFolder( const String& rFolder )
{
    try
    {
        Content aCnt( rFolder, uno::Reference< XCommandEnvironment > () );
        Sequence< ContentInfo > aInfo = aCnt.queryCreatableContentsInfo();
        sal_Int32 nCount = aInfo.getLength();
        if ( nCount == 0 )
            return sal_False;

        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            // Simply look for the first KIND_FOLDER...
            const ContentInfo & rCurr = aInfo[i];
            if ( rCurr.Attributes & ContentInfoAttribute::KIND_FOLDER )
                return sal_True;
        }
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& ) {}
    catch( RuntimeException& ) {}
    catch( Exception& ) {}

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::MakeFolder( const String& rFolder, sal_Bool bNewOnly )
{
    INetURLObject aURL( rFolder );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    String aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    aURL.removeSegment();
    Content aCnt;
    Content aNew;
    uno::Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    uno::Reference< XInteractionHandler > xInteractionHandler = uno::Reference< XInteractionHandler > (
               xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );
    if ( Content::create( aURL.GetMainURL( INetURLObject::NO_DECODE ), new CommandEnvironment( xInteractionHandler, uno::Reference< XProgressHandler >() ), aCnt ) )
        return MakeFolder( aCnt, aTitle, aNew, bNewOnly );
    else
        return sal_False;
}

sal_Bool UCBContentHelper::MakeFolder( Content& aCnt, const String& aTitle, Content& rNew, sal_Bool bNewOnly )
{
    sal_Bool bAlreadyExists = sal_False;

    try
    {
        Sequence< ContentInfo > aInfo = aCnt.queryCreatableContentsInfo();
        sal_Int32 nCount = aInfo.getLength();
        if ( nCount == 0 )
            return sal_False;

        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            // Simply look for the first KIND_FOLDER...
            const ContentInfo & rCurr = aInfo[i];
            if ( rCurr.Attributes & ContentInfoAttribute::KIND_FOLDER )
            {
                // Make sure the only required bootstrap property is "Title",
                const Sequence< Property > & rProps = rCurr.Properties;
                if ( rProps.getLength() != 1 )
                    continue;

                if ( !rProps[ 0 ].Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
                    continue;

                Sequence<OUString> aNames(1);
                OUString* pNames = aNames.getArray();
                pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
                Sequence<Any> aValues(1);
                Any* pValues = aValues.getArray();
                pValues[0] = makeAny( OUString( aTitle ) );

                if ( !aCnt.insertNewContent( rCurr.Type, aNames, aValues, rNew ) )
                    continue;

                return sal_True;
            }
        }
    }
    catch ( InteractiveIOException& r )
    {
        if ( r.Code == IOErrorCode_ALREADY_EXISTING )
        {
            bAlreadyExists = sal_True;
        }
    }
    catch ( NameClashException& )
    {
        bAlreadyExists = sal_True;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( RuntimeException& )
    {
    }
    catch( Exception& )
    {
    }

    if( bAlreadyExists && !bNewOnly )
    {
        INetURLObject aObj( aCnt.getURL() );
        aObj.Append( aTitle );
        rNew = Content( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference < XCommandEnvironment >() );
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::HasParentFolder( const String& rFolder )
{
    sal_Bool bRet = sal_False;
    try
    {
        Content aCnt( rFolder, uno::Reference< XCommandEnvironment > () );
        uno::Reference< XChild > xChild( aCnt.get(), UNO_QUERY );
        if ( xChild.is() )
        {
            uno::Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                String aParentURL = String( xParent->getIdentifier()->getContentIdentifier() );
                bRet = ( aParentURL.Len() > 0 && aParentURL != rFolder );
            }
        }
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_uLong UCBContentHelper::GetSize( const String& rContent )
{
    sal_uLong nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Size")) ) >>= nTemp;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }
    nSize = (sal_uInt32)nTemp;
    return nSize;
}

// -----------------------------------------------------------------------

sal_Bool UCBContentHelper::IsYounger( const String& rIsYoung, const String& rIsOlder )
{
    DateTime aYoungDate, aOlderDate;
    INetURLObject aYoungObj( rIsYoung );
    DBG_ASSERT( aYoungObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    INetURLObject aOlderObj( rIsOlder );
    DBG_ASSERT( aOlderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > aCmdEnv;
        Content aYoung( aYoungObj.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        ::com::sun::star::util::DateTime aTempYoungDate;
        aYoung.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DateModified")) ) >>= aTempYoungDate;
        CONVERT_DATETIME( aTempYoungDate, aYoungDate );
        Content aOlder( aOlderObj.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        ::com::sun::star::util::DateTime aTempOlderDate;
        aOlder.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DateModified")) ) >>= aTempOlderDate;
        CONVERT_DATETIME( aTempOlderDate, aOlderDate );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return ( aYoungDate > aOlderDate );
}

// -----------------------------------------------------------------------
sal_Bool UCBContentHelper::Find( const String& rFolder, const String& rName, String& rFile, sal_Bool bAllowWildCards )
{
    sal_Bool bWild = bAllowWildCards && ( rName.Search( '*' ) != STRING_NOTFOUND || rName.Search( '?' ) != STRING_NOTFOUND );

    sal_Bool bRet = sal_False;

    // get a list of URLs for all children of rFolder
    Sequence< ::rtl::OUString > aFiles = GetFolderContents( rFolder, sal_False );

    const ::rtl::OUString* pFiles  = aFiles.getConstArray();
    sal_uInt32 i, nCount = aFiles.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        // get the last name of the URLs and compare it with rName
        INetURLObject aFileObject( pFiles[i] );
        String aFile = aFileObject.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).toAsciiLowerCase();
        if ( (bWild && WildCard( rName ).Matches( aFile )) || aFile == rName )
        {
            // names match
            rFile = aFileObject.GetMainURL( INetURLObject::NO_DECODE );
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------
sal_Bool UCBContentHelper::Exists( const String& rURL )
{

    String sObjectPhysicalName;
    sal_Bool bIsLocalFile = ::utl::LocalFileHelper::ConvertURLToPhysicalName( rURL, sObjectPhysicalName );
    // try to create a directory entry for the URL given
    if ( bIsLocalFile )
    {
        ::rtl::OUString sIn( sObjectPhysicalName ), sOut;
        if ( osl_File_E_None == osl_getFileURLFromSystemPath( sIn.pData, &sOut.pData ) )
        {
            // #106526 osl_getDirectoryItem is an existence check
            // no further osl_getFileStatus call necessary
            DirectoryItem aItem;
            return (FileBase::E_None == DirectoryItem::get(sOut, aItem));
        }
        return sal_False;
    }

    // divide URL into folder and name part
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rURL );
    ::rtl::OUString aFileName = aObj.getName(
        INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).toAsciiLowerCase();
    aObj.removeSegment();
    aObj.removeFinalSlash();

    // get a list of URLs for all children of rFolder
    Sequence< ::rtl::OUString > aFiles = GetFolderContents( aObj.GetMainURL( INetURLObject::NO_DECODE ), sal_True, sal_False );

    const ::rtl::OUString* pFiles  = aFiles.getConstArray();
    sal_uInt32 i, nCount = aFiles.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        // get the last name of the URLs and compare it with rName
        INetURLObject aFileObject( pFiles[i] );
        ::rtl::OUString aFile = aFileObject.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).toAsciiLowerCase();
        if ( aFile == aFileName )
        {
            // names match
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------
sal_Bool UCBContentHelper::FindInPath( const String& rPath, const String& rName, String& rFile, char cDelim, sal_Bool bAllowWildCards )
{
    // extract the single folder names from the path variable and try to find the file in one of these folders
    sal_uInt16 nTokenCount = rPath.GetTokenCount( cDelim );
    for ( sal_uInt16 nToken = 0; nToken < nTokenCount; ++nToken )
    {
        String aPath = rPath.GetToken( nToken, cDelim );
        if ( Find( aPath, rName, rFile, bAllowWildCards ) )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------
sal_Bool UCBContentHelper::IsSubPath( const ::rtl::OUString& rPath, const ::rtl::OUString& rSubfolderCandidate, const uno::Reference< ::com::sun::star::ucb::XContentProvider >& xProv )
{
    sal_Bool bResult = sal_False;

    uno::Reference< ::com::sun::star::ucb::XContentProvider > xContentProvider = xProv;

    // the comparing is done in the following way:
    // - first compare in case sensitive way
    // - if name are different try a fallback comparing inf case insensitive way
    // - if the last comparing succeeded get casepreserving normalized names for the files and compare them
    // ( the second step is required because retrieving of the normalized names might be very expensive in some cases )

    INetURLObject aCandidate( rSubfolderCandidate );
    INetURLObject aCandidateLowCase( rSubfolderCandidate.toAsciiLowerCase() ); // will be used for case insensitive comparing
    INetURLObject aParentFolder( rPath );
    INetURLObject aParentFolderLowCase( rPath.toAsciiLowerCase() ); // will be used for case insensitive comparing

    if ( aCandidate.GetProtocol() == aParentFolder.GetProtocol() )
    {
        if ( !xContentProvider.is() )
        {
            ::ucbhelper::ContentBroker* pBroker = NULL;
            if ( aCandidate.GetProtocol() == INET_PROT_FILE )
            {
                pBroker = ::ucbhelper::ContentBroker::get();
                if ( pBroker )
                    xContentProvider = pBroker->getContentProviderInterface();
            }
        }

        INetURLObject aLastTmpObj;
        do
        {
            if ( aParentFolder == aCandidate )
            {
                // if case sensitive comparing succeeded there is no need for additional checks
                bResult = sal_True;
            }
            else if ( xContentProvider.is() && aParentFolderLowCase == aCandidateLowCase )
            {
                // the comparing was done using caseinsensitive way
                // the case sensitive comparing have failed already
                // the normalized urls must be retrieved
                try
                {
                    uno::Reference< ::com::sun::star::ucb::XContent > xSecCont =
                        xContentProvider->queryContent(
                            uno::Reference< ::com::sun::star::ucb::XContentIdentifierFactory >(
                                xContentProvider, ::com::sun::star::uno::UNO_QUERY_THROW )->createContentIdentifier(
                                    aParentFolder.GetMainURL( INetURLObject::NO_DECODE ) ) );

                    uno::Reference< ::com::sun::star::ucb::XContent > xLocCont =
                        xContentProvider->queryContent(
                            uno::Reference< ::com::sun::star::ucb::XContentIdentifierFactory >(
                                xContentProvider, ::com::sun::star::uno::UNO_QUERY_THROW )->createContentIdentifier(
                                    aCandidate.GetMainURL( INetURLObject::NO_DECODE ) ) );

                    if ( !xSecCont.is() || !xLocCont.is() )
                        throw ::com::sun::star::uno::RuntimeException();

                    ::rtl::OUString aSecNormStr;
                    ::rtl::OUString aLocNormStr;

                    bResult =
                    ( ( uno::Reference< ::com::sun::star::ucb::XCommandProcessor >(
                            xSecCont, ::com::sun::star::uno::UNO_QUERY_THROW )->execute(
                                ::com::sun::star::ucb::Command(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCasePreservingURL" ) ),
                                    -1,
                                    ::com::sun::star::uno::Any() ),
                                0,
                                uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() )
                        >>= aSecNormStr )
                    && ( uno::Reference< ::com::sun::star::ucb::XCommandProcessor >(
                            xLocCont, ::com::sun::star::uno::UNO_QUERY_THROW )->execute(
                                ::com::sun::star::ucb::Command(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCasePreservingURL" ) ),
                                    -1,
                                    ::com::sun::star::uno::Any() ),
                                0,
                                uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() )
                        >>= aLocNormStr )
                    && aLocNormStr.equals( aSecNormStr ) );
                }
                catch( ::com::sun::star::uno::Exception& )
                {}
            }

            // INetURLObject::removeSegment sometimes return true without exchanging URL,
            // for example in case of "file:///"
            aLastTmpObj = aCandidate;

        } while( aCandidate.removeSegment() && aCandidateLowCase.removeSegment() && aCandidate != aLastTmpObj && !bResult );
    }

    return bResult;
}

// -----------------------------------------------------------------------
sal_Bool UCBContentHelper::EqualURLs( const ::rtl::OUString& aFirstURL, const ::rtl::OUString& aSecondURL )
{
    sal_Bool bResult = sal_False;

    if ( aFirstURL.getLength() && aSecondURL.getLength() )
    {
        INetURLObject aFirst( aFirstURL );
        INetURLObject aSecond( aSecondURL );

        if ( aFirst.GetProtocol() != INET_PROT_NOT_VALID && aSecond.GetProtocol() != INET_PROT_NOT_VALID )
        {
            try
            {
                ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
                if ( !pBroker )
                    throw uno::RuntimeException();

                uno::Reference< ::com::sun::star::ucb::XContentIdentifierFactory > xIdFac
                    = pBroker->getContentIdentifierFactoryInterface();
                if ( !xIdFac.is() )
                    throw uno::RuntimeException();

                uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xIdFirst
                    = xIdFac->createContentIdentifier( aFirst.GetMainURL( INetURLObject::NO_DECODE ) );
                uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xIdSecond
                    = xIdFac->createContentIdentifier( aSecond.GetMainURL( INetURLObject::NO_DECODE ) );

                if ( xIdFirst.is() && xIdSecond.is() )
                {
                    uno::Reference< ::com::sun::star::ucb::XContentProvider > xProvider =
                                                            pBroker->getContentProviderInterface();
                    if ( !xProvider.is() )
                        throw uno::RuntimeException();
                    bResult = !xProvider->compareContentIds( xIdFirst, xIdSecond );
                }
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "Can't compare URL's, treat as different!\n" );
            }
        }
    }

    return bResult;
}



} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
