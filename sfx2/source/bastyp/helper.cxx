/*************************************************************************
 *
 *  $RCSfile: helper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-31 10:09:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "helper.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_ILLEGALIDENTIFIEREXCEPTION_HPP_
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <vcl/svapp.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

using namespace ::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;
using namespace comphelper;
using namespace osl;

DECLARE_LIST( StringList_Impl, OUString* );

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

void AppendDateTime_Impl( const ::com::sun::star::util::DateTime rDT, String& rRow )
{
    DateTime aDT;
    CONVERT_DATETIME( rDT, aDT );
    const International& rInter = Application::GetAppInternational();
    String aDateStr = rInter.GetDate( aDT );
    aDateStr += String::CreateFromAscii( ", " );
    aDateStr += rInter.GetTime( aDT );
    rRow += aDateStr;
}

// SfxContentHelper ------------------------------------------------------

sal_Bool SfxContentHelper::Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData, sal_Int32 nNameClash )
{
    sal_Bool bRet = sal_True, bKillSource = sal_False;
    INetURLObject aSourceObj( rSource, INET_PROT_FILE );
    INetURLObject aDestObj( rDest, INET_PROT_FILE );
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
        Content aDestPath( aDestObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        Reference< ::com::sun::star::ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        OUString aTransferName = OUString::createFromAscii( "transfer" );
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, makeAny(
                ::com::sun::star::ucb::TransferInfo( bMoveData, aSourceObj.GetMainURL(), aName, nNameClash ) ) );
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
    catch( ... )
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
    INetURLObject aObj( rContent, INET_PROT_FILE );
    try
    {
        Content aCnt( aObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isDocument();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::IsFolder( const String& rContent )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent, INET_PROT_FILE );
    try
    {
        Content aCnt( aObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        bRet = aCnt.isFolder();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::IllegalIdentifierException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ContentCreationException& )
    {
        DBG_WARNING( "IllegalIdentifierException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::GetTitle( const String& rContent, String& rTitle )
{
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent, INET_PROT_FILE );
    try
    {
        Content aCnt( aObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        OUString aTemp;
        aCnt.getPropertyValue( OUString::createFromAscii( "Title" ) ) >>= aTemp;
        rTitle = String( aTemp );
        bRet = sal_True;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::Kill( const String& rContent )
{
    sal_Bool bRet = sal_True;
    INetURLObject aDeleteObj( rContent, INET_PROT_FILE );
    try
    {
        Content aCnt( aDeleteObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}

// -----------------------------------------------------------------------

Sequence < OUString > SfxContentHelper::GetFolderContents( const String& rFolder, sal_Bool bFolder )
{
    StringList_Impl* pFiles = NULL;
    INetURLObject aFolderObj( rFolder, INET_PROT_FILE );
    try
    {
        Content aCnt( aFolderObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(1);
        OUString* pProps = aProps.getArray();
        pProps[0] == OUString::createFromAscii( "Url" );

        try
        {
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            xResultSet = aCnt.createCursor( aProps, eInclude );
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pFiles = new StringList_Impl;
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
#if SUPD>611
                    OUString aId = xContentAccess->queryContentIdentifierString();
#else
                    OUString aId = xContentAccess->queryContentIdentfierString();
#endif
                    OUString* pFile = new OUString( aId );
                    pFiles->Insert( pFile, LIST_APPEND );
                }
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ... )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( ... )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( pFiles )
    {
        USHORT nCount = pFiles->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            OUString* pFile = pFiles->GetObject(i);
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

Sequence < OUString > SfxContentHelper::GetFolderContentProperties( const String& rFolder, sal_Bool bFolder )
{
    StringList_Impl* pProperties = NULL;
    INetURLObject aFolderObj( rFolder, INET_PROT_FILE );
    try
    {
        Content aCnt( aFolderObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        Reference< XResultSet > xResultSet;
        Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
        Sequence< OUString > aProps(5);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "ContentType" );
        pProps[2] = OUString::createFromAscii( "Size" );
        pProps[3] = OUString::createFromAscii( "DateModified" );
        pProps[4] = OUString::createFromAscii( "IsFolder" );

        try
        {
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            Reference < com::sun::star::ucb::XAnyCompareFactory > xFactory;
            Reference < XMultiServiceFactory > xMgr = getProcessServiceFactory();
            Reference < com::sun::star::ucb::XSortedDynamicResultSetFactory > xSRSFac(
                xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), UNO_QUERY );

            Sequence< com::sun::star::ucb::NumberedSortingInfo > aSortInfo( 2 );
            com::sun::star::ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 5;
            pInfo[ 0 ].Ascending   = sal_False;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = sal_True;

            Reference< com::sun::star::ucb::XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xFactory );
            if ( xDynamicResultSet.is() )
            {
                sal_Int16 nCaps = xDynamicResultSet->getCapabilities();
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }

//          if ( xDynResultSet.is() )
//              xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pProperties = new StringList_Impl;
            Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            ULONG nFolderPos = LIST_APPEND;

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    String aType( xRow->getString(2) );
                    sal_Int64 nSize = xRow->getLong(3);
                    ::com::sun::star::util::DateTime aDT = xRow->getTimestamp(4);
                    sal_Bool bFolder = xRow->getBoolean(5);

                    String aRow = aTitle;
                    aRow += '\t';
//!                 aRow += aType;
//!                 aRow += '\t';
                    aRow += String::CreateFromInt64( nSize );
                    aRow += '\t';
                    AppendDateTime_Impl( aDT, aRow );
                    aRow += '\t';
#if SUPD>611
                    aRow += String( xContentAccess->queryContentIdentifierString() );
#else
                    aRow += String( xContentAccess->queryContentIdentfierString() );
#endif
                    aRow += '\t';
                    aRow += bFolder ? '1' : '0';
                    OUString* pRow = new OUString( aRow );
                    ULONG nPos = LIST_APPEND;
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
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ... )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( ... )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( pProperties )
    {
        USHORT nCount = pProperties->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            OUString* pProperty = pProperties->GetObject(i);
            pRet[i] = *( pProperty );
            delete pProperty;
        }
        delete pProperties;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::CopyTo( const String& rSource, const String& rDest )
{
    return Transfer_Impl( rSource, rDest, sal_False, NameClash::ERROR );
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash )
{
    return Transfer_Impl( rSource, rDest, sal_True, nNameClash );
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::MakeFolder( const String& rFolder )
{
    INetURLObject aURL( rFolder, INET_PROT_FILE );
    String aNewFolderURL = aURL.GetMainURL();
    String aTitle = aURL.getName();
    aURL.removeSegment();
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) );
    Sequence<Any> aValues(2);
    Any* pValues = aValues.getArray();
    pValues[0] = makeAny( OUString( aTitle ) );
    pValues[1] = makeAny( sal_Bool( sal_True ) );
    Reference< ::com::sun::star::ucb::XCommandEnvironment > aCmdEnv;
    sal_Bool bRet = sal_False;
    try
    {
        Content aCnt( aURL.GetMainURL(), aCmdEnv );
        Content aNewFolder( aNewFolderURL, aCmdEnv );
        OUString aType( RTL_CONSTASCII_USTRINGPARAM( "FSysFolder" ) );
        bRet = aCnt.insertNewContent( aType, aNames, aValues, aNewFolder );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
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
    INetURLObject aObj( rPath, INET_PROT_FILE );
    try
    {
        Content aCnt( aObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString::createFromAscii( "FreeSpace" ) ) >>= rFreeBytes;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
        nErr = ERRCODE_IO_GENERAL;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
        nErr = ERRCODE_IO_GENERAL;
    }
    return nErr;
}

// -----------------------------------------------------------------------

ULONG SfxContentHelper::GetSize( const String& rContent )
{
    ULONG nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( rContent, INET_PROT_FILE );
    try
    {
        Content aCnt( aObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= nTemp;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
    nSize = (UINT32)nTemp;
    return nSize;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::IsYounger( const String& rIsYoung, const String& rIsOlder )
{
    DateTime aYoungDate, aOlderDate;
    INetURLObject aYoungObj( rIsYoung, INET_PROT_FILE );
    INetURLObject aOlderObj( rIsOlder, INET_PROT_FILE );
    try
    {
        Reference< ::com::sun::star::ucb::XCommandEnvironment > aCmdEnv;
        Content aYoung( aYoungObj.GetMainURL(), aCmdEnv );
        ::com::sun::star::util::DateTime aTempYoungDate;
        aYoung.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aTempYoungDate;
        CONVERT_DATETIME( aTempYoungDate, aYoungDate );
        Content aOlder( aOlderObj.GetMainURL(), aCmdEnv );
        ::com::sun::star::util::DateTime aTempOlderDate;
        aOlder.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aTempOlderDate;
        CONVERT_DATETIME( aTempOlderDate, aOlderDate );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }

    return ( aYoungDate > aOlderDate );
}

// -----------------------------------------------------------------------
// please don't use it (only used in appbas.cxx and appcfg.cxx)
sal_Bool SfxContentHelper::Exists( const String& rContent )
{
    INetURLObject aObj( rContent, INET_PROT_FILE );
    rtl::OUString aTmp( aObj.GetMainURL() );
    rtl::OUString aResult;
    if ( FileBase::getNormalizedPathFromFileURL( aTmp, aResult )  == FileBase::E_None )
    {
        FileBase::RC err = Directory::create( aResult );
        if ( err == FileBase::E_EXIST )
            return sal_True;
        else
            Directory::remove( aResult );
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SfxContentHelper::Find( const String& rFolder, const String& rName, String& rFile )
{
    sal_Bool bRet = sal_False;
    rtl::OUString aFile;
    if ( FileBase::searchNormalizedPath( rName, rFolder, aFile ) == FileBase::E_None )
    {
        rtl::OUString aResult;
        FileBase::getFileURLFromNormalizedPath( aFile, aResult );
        rFile = aResult;
        bRet = sal_True;
    }

    return bRet;
}


