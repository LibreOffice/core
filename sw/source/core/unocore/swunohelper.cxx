/*************************************************************************
 *
 *  $RCSfile: swunohelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:41:21 $
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

#pragma hdrstop

#define _SVSTDARR_STRINGS

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HDL_
#include <com/sun/star/ucb/NameClash.hdl>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#include <svtools/svstdarr.hxx>

#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

namespace SWUnoHelper {

sal_Int32 GetEnumAsInt32( const UNO_NMSPC::Any& rVal )
{
    sal_Int32 eVal;
    try
    {
        eVal = comphelper::getEnumAsINT32( rVal );
    }
    catch( UNO_NMSPC::Exception & )
    {
        eVal = 0;
        ASSERT( FALSE, "can't get EnumAsInt32" );
    }
    return eVal;
}


// methods for UCB actions
BOOL UCB_DeleteFile( const String& rURL )
{
    BOOL bRemoved;
    try
    {
        ucb::Content aTempContent( rURL,
                                STAR_REFERENCE( ucb::XCommandEnvironment )());
        aTempContent.executeCommand(
                        rtl::OUString::createFromAscii( "delete" ),
                        UNO_NMSPC::makeAny( sal_Bool( sal_True ) ) );
        bRemoved = TRUE;
    }
    catch( UNO_NMSPC::Exception& )
    {
        bRemoved = FALSE;
        ASSERT( FALSE, "Exeception from executeCommand( delete )" );
    }
    return bRemoved;
}

BOOL UCB_CopyFile( const String& rURL, const String& rNewURL, BOOL bCopyIsMove )
{
    BOOL bCopyCompleted = TRUE;
    try
    {
        INetURLObject aURL( rNewURL );
        String sName( aURL.GetName() );
        aURL.removeSegment();
        String sMainURL( aURL.GetMainURL(INetURLObject::NO_DECODE) );

        ucb::Content aTempContent( sMainURL,
                                STAR_REFERENCE( ucb::XCommandEnvironment )());

        UNO_NMSPC::Any aAny;
        STAR_NMSPC::ucb::TransferInfo aInfo;
        aInfo.NameClash = STAR_NMSPC::ucb::NameClash::ERROR;
        aInfo.NewTitle = sName;
        aInfo.SourceURL = rURL;
        aInfo.MoveData = bCopyIsMove;
        aAny <<= aInfo;
        aTempContent.executeCommand(
                            rtl::OUString::createFromAscii( "transfer" ),
                            aAny );
    }
    catch( UNO_NMSPC::Exception& )
    {
        ASSERT( FALSE, "Exeception from executeCommand( transfer )" );
        bCopyCompleted = FALSE;
    }
    return bCopyCompleted;
}

BOOL UCB_IsCaseSensitiveFileName( const String& rURL )
{
    BOOL bCaseSensitive;
    try
    {
        STAR_REFERENCE( lang::XMultiServiceFactory ) xMSF =
                                    comphelper::getProcessServiceFactory();

        INetURLObject aTempObj( rURL );
        aTempObj.SetBase( aTempObj.GetBase().ToLowerAscii() );
        STAR_REFERENCE( ucb::XContentIdentifier ) xRef1 = new
                ucb::ContentIdentifier( xMSF,
                            aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

        aTempObj.SetBase(aTempObj.GetBase().ToUpperAscii());
        STAR_REFERENCE( ucb::XContentIdentifier ) xRef2 = new
                ucb::ContentIdentifier( xMSF,
                            aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

        STAR_REFERENCE( ucb::XContentProvider ) xProv =
                ucb::ContentBroker::get()->getContentProviderInterface();

        sal_Int32 nCompare = xProv->compareContentIds( xRef1, xRef2 );
        bCaseSensitive = 0 != nCompare;
    }
    catch( UNO_NMSPC::Exception& )
    {
        bCaseSensitive = FALSE;
        ASSERT( FALSE, "Exeception from compareContentIds()" );
    }
    return bCaseSensitive;
}

BOOL UCB_IsReadOnlyFileName( const String& rURL )
{
    BOOL bIsReadOnly = FALSE;
    try
    {
        ucb::Content aCnt( rURL, STAR_REFERENCE( ucb::XCommandEnvironment )());
        UNO_NMSPC::Any aAny = aCnt.getPropertyValue(
                            rtl::OUString::createFromAscii( "IsReadOnly" ));
        if(aAny.hasValue())
            bIsReadOnly = *(sal_Bool*)aAny.getValue();
    }
    catch( UNO_NMSPC::Exception& )
    {
        bIsReadOnly = FALSE;
    }
    return bIsReadOnly;
}

BOOL UCB_IsFile( const String& rURL )
{
    BOOL bExists = FALSE;
    try
    {
        ::ucb::Content aContent( rURL, STAR_REFERENCE( ucb::XCommandEnvironment )() );
        bExists = aContent.isDocument();
    }
    catch (UNO_NMSPC::Exception &)
    {
    }
    return bExists;
}

BOOL UCB_IsDirectory( const String& rURL )
{
    BOOL bExists = FALSE;
    try
    {
        ::ucb::Content aContent( rURL, STAR_REFERENCE( ucb::XCommandEnvironment )() );
        bExists = aContent.isFolder();
    }
    catch (UNO_NMSPC::Exception &)
    {
    }
    return bExists;
}

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a SvPtrarr -->
    //                       !! objects must be deleted from the caller!!
BOOL UCB_GetFileListOfFolder( const String& rURL, SvStrings& rList,
                                const String* pExtension,
                                SvPtrarr* pDateTimeList )
{
    BOOL bOk = FALSE;
    try
    {
        ucb::Content aCnt( rURL, STAR_REFERENCE( ucb::XCommandEnvironment )());
        STAR_REFERENCE( sdbc::XResultSet ) xResultSet;

        USHORT nSeqSize = pDateTimeList ? 2 : 1;
        UNO_NMSPC::Sequence < rtl::OUString > aProps( nSeqSize );
        rtl::OUString* pProps = aProps.getArray();
        pProps[ 0 ] = rtl::OUString::createFromAscii( "Title" );
        if( pDateTimeList )
            pProps[ 1 ] = rtl::OUString::createFromAscii( "DateModified" );

        try
        {
            xResultSet = aCnt.createCursor( aProps, ::ucb::INCLUDE_DOCUMENTS_ONLY );
        }
        catch( UNO_NMSPC::Exception& )
        {
            DBG_ERRORFILE( "create cursor failed!" );
        }

        if( xResultSet.is() )
        {
            STAR_REFERENCE( sdbc::XRow ) xRow( xResultSet, UNO_NMSPC::UNO_QUERY );
            xub_StrLen nExtLen = pExtension ? pExtension->Len() : 0;
            try
            {
                if( xResultSet->first() )
                {
                    do {
                        String sTitle( xRow->getString( 1 ) );
                        if( !nExtLen ||
                            ( sTitle.Len() > nExtLen &&
                              sTitle.Equals( *pExtension,
                                          sTitle.Len() - nExtLen, nExtLen )) )
                        {
                            String* pStr = new String( sTitle );
                            rList.Insert( pStr, rList.Count() );

                            if( pDateTimeList )
                            {
                                STAR_NMSPC::util::DateTime aStamp = xRow->getTimestamp(2);
                                ::DateTime* pDateTime = new ::DateTime(
                                        ::Date( aStamp.Day,
                                                aStamp.Month,
                                                aStamp.Year ),
                                        ::Time( aStamp.Hours,
                                                aStamp.Minutes,
                                                aStamp.Seconds,
                                                aStamp.HundredthSeconds ));
                                void* p = pDateTime;
                                pDateTimeList->Insert( p,
                                                    pDateTimeList->Count() );
                            }
                        }

                    } while( xResultSet->next() );
                }
                bOk = TRUE;
            }
            catch( UNO_NMSPC::Exception& )
            {
                DBG_ERRORFILE( "Exception caught!" );
            }
        }
    }
    catch( UNO_NMSPC::Exception& )
    {
        DBG_ERRORFILE( "Exception caught!" );
        bOk = FALSE;
    }
    return bOk;
}

}
