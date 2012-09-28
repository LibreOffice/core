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

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/content.hxx>
#include <swunohelper.hxx>

namespace SWUnoHelper {

sal_Int32 GetEnumAsInt32( const ::com::sun::star::uno::Any& rVal )
{
    sal_Int32 eVal;
    try
    {
        eVal = comphelper::getEnumAsINT32( rVal );
    }
    catch( ::com::sun::star::uno::Exception & )
    {
        eVal = 0;
        OSL_FAIL( "can't get EnumAsInt32" );
    }
    return eVal;
}


// methods for UCB actions
sal_Bool UCB_DeleteFile( const String& rURL )
{
    sal_Bool bRemoved;
    try
    {
        ucbhelper::Content aTempContent( rURL,
                                ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                comphelper::getProcessComponentContext() );
        aTempContent.executeCommand(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("delete")),
                        ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
        bRemoved = sal_True;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bRemoved = sal_False;
        OSL_FAIL( "Exeception from executeCommand( delete )" );
    }
    return bRemoved;
}

sal_Bool UCB_CopyFile( const String& rURL, const String& rNewURL, sal_Bool bCopyIsMove )
{
    sal_Bool bCopyCompleted = sal_True;
    try
    {
        INetURLObject aURL( rNewURL );
        String sName( aURL.GetName() );
        aURL.removeSegment();
        String sMainURL( aURL.GetMainURL(INetURLObject::NO_DECODE) );

        ucbhelper::Content aTempContent( sMainURL,
                                ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                comphelper::getProcessComponentContext() );

        ::com::sun::star::uno::Any aAny;
        ::com::sun::star::ucb::TransferInfo aInfo;
        aInfo.NameClash = ::com::sun::star::ucb::NameClash::ERROR;
        aInfo.NewTitle = sName;
        aInfo.SourceURL = rURL;
        aInfo.MoveData = bCopyIsMove;
        aAny <<= aInfo;
        aTempContent.executeCommand(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("transfer")),
                            aAny );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL( "Exeception from executeCommand( transfer )" );
        bCopyCompleted = sal_False;
    }
    return bCopyCompleted;
}

sal_Bool UCB_IsCaseSensitiveFileName( const String& rURL )
{
    sal_Bool bCaseSensitive;
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    comphelper::getProcessServiceFactory();

        INetURLObject aTempObj( rURL );
        aTempObj.SetBase( aTempObj.GetBase().toAsciiLowerCase() );
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xRef1 = new
                ucbhelper::ContentIdentifier( xMSF,
                            aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

        aTempObj.SetBase(aTempObj.GetBase().toAsciiUpperCase());
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xRef2 = new
                ucbhelper::ContentIdentifier( xMSF,
                            aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XUniversalContentBroker > xUcb =
              com::sun::star::ucb::UniversalContentBroker::create(comphelper::getProcessComponentContext());

        sal_Int32 nCompare = xUcb->compareContentIds( xRef1, xRef2 );
        bCaseSensitive = 0 != nCompare;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bCaseSensitive = sal_False;
        OSL_FAIL( "Exeception from compareContentIds()" );
    }
    return bCaseSensitive;
}

sal_Bool UCB_IsReadOnlyFileName( const String& rURL )
{
    sal_Bool bIsReadOnly = sal_False;
    try
    {
        ucbhelper::Content aCnt( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        ::com::sun::star::uno::Any aAny = aCnt.getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsReadOnly")));
        if(aAny.hasValue())
            bIsReadOnly = *(sal_Bool*)aAny.getValue();
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bIsReadOnly = sal_False;
    }
    return bIsReadOnly;
}

sal_Bool UCB_IsFile( const String& rURL )
{
    sal_Bool bExists = sal_False;
    try
    {
        ::ucbhelper::Content aContent( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        bExists = aContent.isDocument();
    }
    catch (::com::sun::star::uno::Exception &)
    {
    }
    return bExists;
}

sal_Bool UCB_IsDirectory( const String& rURL )
{
    sal_Bool bExists = sal_False;
    try
    {
        ::ucbhelper::Content aContent( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        bExists = aContent.isFolder();
    }
    catch (::com::sun::star::uno::Exception &)
    {
    }
    return bExists;
}

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a std::vector<String*> -->
    //                       !! objects must be deleted from the caller!!
bool UCB_GetFileListOfFolder( const String& rURL,
                                std::vector<String*>& rList,
                                const String* pExtension,
                                std::vector< ::DateTime* >* pDateTimeList )
{
    sal_Bool bOk = sal_False;
    try
    {
        ucbhelper::Content aCnt( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;

        sal_uInt16 nSeqSize = pDateTimeList ? 2 : 1;
        ::com::sun::star::uno::Sequence < rtl::OUString > aProps( nSeqSize );
        rtl::OUString* pProps = aProps.getArray();
        pProps[ 0 ] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        if( pDateTimeList )
            pProps[ 1 ] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DateModified"));

        try
        {
            xResultSet = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL( "create cursor failed!" );
        }

        if( xResultSet.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow > xRow( xResultSet, ::com::sun::star::uno::UNO_QUERY );
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
                            rList.push_back( new String(sTitle) );

                            if( pDateTimeList )
                            {
                                ::com::sun::star::util::DateTime aStamp = xRow->getTimestamp(2);
                                ::DateTime* pDateTime = new ::DateTime(
                                        ::Date( aStamp.Day,
                                                aStamp.Month,
                                                aStamp.Year ),
                                        ::Time( aStamp.Hours,
                                                aStamp.Minutes,
                                                aStamp.Seconds,
                                                aStamp.HundredthSeconds ));
                                pDateTimeList->push_back( pDateTime );
                            }
                        }

                    } while( xResultSet->next() );
                }
                bOk = sal_True;
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                OSL_FAIL( "Exception caught!" );
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL( "Exception caught!" );
        bOk = sal_False;
    }
    return bOk;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
