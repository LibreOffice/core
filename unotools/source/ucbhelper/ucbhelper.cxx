/*************************************************************************
 *
 *  $RCSfile: ucbhelper.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:39:53 $
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

#include "unotools/ucbhelper.hxx"

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
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
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
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
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
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIODEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <tools/wldcrd.hxx>
#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>

#include "unotools/localfilehelper.hxx"

using namespace ::ucb;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;
using namespace comphelper;
using namespace osl;

DECLARE_LIST( StringList_Impl, OUString* );

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
        Content aDestPath( aDestObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        Reference< ::com::sun::star::ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        OUString aTransferName = OUString::createFromAscii( "transfer" );
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
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
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
    sal_Bool bRet = sal_False;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
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
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
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
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        OUString aTemp;
        if ( aCnt.getPropertyValue( OUString::createFromAscii( "Title" ) ) >>= aTemp )
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
        Content aCnt( aDeleteObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
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
        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps( bSorted ? 2 : 1 );
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        if ( bSorted )
            pProps[1] = OUString::createFromAscii( "IsFolder" );

        try
        {
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            if ( !bSorted )
            {
                xResultSet = aCnt.createCursor( aProps, eInclude );
            }
            else
            {
                Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
                xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

                Reference < com::sun::star::ucb::XAnyCompareFactory > xFactory;
                Reference < XMultiServiceFactory > xMgr = getProcessServiceFactory();
                Reference < com::sun::star::ucb::XSortedDynamicResultSetFactory > xSRSFac(
                    xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), UNO_QUERY );

                Sequence< com::sun::star::ucb::NumberedSortingInfo > aSortInfo( 2 );
                com::sun::star::ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
                pInfo[ 0 ].ColumnIndex = 2;
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
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    OUString aId = xContentAccess->queryContentIdentifierString();
                    OUString* pFile = new OUString( aId );
                    pFiles->Insert( pFile, LIST_APPEND );
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
        ULONG nCount = pFiles->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( ULONG i = 0; i < nCount; ++i )
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

Sequence < OUString > UCBContentHelper::GetResultSet( const String& rURL )
{
    StringList_Impl* pList = NULL;
    try
    {
        Content aCnt( rURL, Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        Reference< XResultSet > xResultSet;
        Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
        Sequence< OUString > aProps(3);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "ContentType" );
        pProps[2] = OUString::createFromAscii( "IsFolder" );

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
            Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    String aType( xRow->getString(2) );
                    sal_Bool bFolder = xRow->getBoolean(3);
                    String aRow = aTitle;
                    aRow += '\t';
                    aRow += aType;
                    aRow += '\t';
                    aRow += String( xContentAccess->queryContentIdentifierString() );
                    OUString* pRow = new OUString( aRow );
                    pList->Insert( pRow, LIST_APPEND );
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
        ULONG nCount = pList->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( ULONG i = 0; i < nCount; ++i )
        {
            OUString* pEntry = pList->GetObject(i);
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
        Content aCnt( rFolder, Reference< XCommandEnvironment > () );
        Reference< XContentCreator > xCreator = Reference< XContentCreator >( aCnt.get(), UNO_QUERY );
        if ( !xCreator.is() )
            return sal_False;

        Sequence< ContentInfo > aInfo = xCreator->queryCreatableContentsInfo();
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
    Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
               xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );
    if ( Content::create( aURL.GetMainURL( INetURLObject::NO_DECODE ), new CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() ), aCnt ) )
        return MakeFolder( aCnt, aTitle, aNew, bNewOnly );
    else
        return sal_False;
}

sal_Bool UCBContentHelper::MakeFolder( Content& aCnt, const String& aTitle, Content& rNew, sal_Bool bNewOnly )
{
    sal_Bool bAlreadyExists = sal_False;

    try
    {
        Reference< XContentCreator > xCreator = Reference< XContentCreator >( aCnt.get(), UNO_QUERY );
        if ( !xCreator.is() )
            return sal_False;

        Sequence< ContentInfo > aInfo = xCreator->queryCreatableContentsInfo();
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
        rNew = Content( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference < XCommandEnvironment >() );
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
        Content aCnt( rFolder, Reference< XCommandEnvironment > () );
        Reference< XChild > xChild( aCnt.get(), UNO_QUERY );
        if ( xChild.is() )
        {
            Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
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

ULONG UCBContentHelper::GetSize( const String& rContent )
{
    ULONG nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( rContent );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= nTemp;
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }
    nSize = (UINT32)nTemp;
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
        Reference< ::com::sun::star::ucb::XCommandEnvironment > aCmdEnv;
        Content aYoung( aYoungObj.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        ::com::sun::star::util::DateTime aTempYoungDate;
        aYoung.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aTempYoungDate;
        CONVERT_DATETIME( aTempYoungDate, aYoungDate );
        Content aOlder( aOlderObj.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        ::com::sun::star::util::DateTime aTempOlderDate;
        aOlder.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aTempOlderDate;
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
sal_Bool UCBContentHelper::Find( const String& rFolder, const String& rName, String& rFile, BOOL bAllowWildCards )
{
    BOOL bWild = bAllowWildCards && rName.Search( '*' ) != STRING_NOTFOUND || rName.Search( '?' ) != STRING_NOTFOUND;

    sal_Bool bRet = sal_False;

    // get a list of URLs for all children of rFolder
    Sequence< ::rtl::OUString > aFiles = GetFolderContents( rFolder, sal_False );

    const ::rtl::OUString* pFiles  = aFiles.getConstArray();
    UINT32 i, nCount = aFiles.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        // get the last name of the URLs and compare it with rName
        INetURLObject aFileObject( pFiles[i] );
        String aFile = aFileObject.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).ToLowerAscii();
        if ( bWild && WildCard( rName ).Matches( aFile ) || aFile == rName )
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
        INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).ToLowerAscii();
    aObj.removeSegment();
    aObj.removeFinalSlash();

    // get a list of URLs for all children of rFolder
    Sequence< ::rtl::OUString > aFiles = GetFolderContents( aObj.GetMainURL( INetURLObject::NO_DECODE ), sal_True, sal_False );

    const ::rtl::OUString* pFiles  = aFiles.getConstArray();
    UINT32 i, nCount = aFiles.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        // get the last name of the URLs and compare it with rName
        INetURLObject aFileObject( pFiles[i] );
        ::rtl::OUString aFile = aFileObject.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).ToLowerAscii();
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
sal_Bool UCBContentHelper::FindInPath( const String& rPath, const String& rName, String& rFile, char cDelim, BOOL bAllowWildCards )
{
    // extract the single folder names from the path variable and try to find the file in one of these folders
    USHORT nTokenCount = rPath.GetTokenCount( cDelim );
    USHORT nIndex = 0;
    for ( USHORT nToken = 0; nToken < nTokenCount; ++nToken )
    {
        String aPath = rPath.GetToken( nToken, cDelim );
        if ( Find( aPath, rName, rFile, bAllowWildCards ) )
            return TRUE;
    }

    return FALSE;
}

};

