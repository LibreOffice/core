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
#include "rtl/ustring.hxx"
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
        aTempContent.executeCommand(OUString("delete"),
                        ::com::sun::star::uno::makeAny( sal_True ) );
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
        aTempContent.executeCommand( OUString("transfer"),
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
        INetURLObject aTempObj( rURL );
        aTempObj.SetBase( aTempObj.GetBase().toAsciiLowerCase() );
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xRef1 = new
                ucbhelper::ContentIdentifier( aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

        aTempObj.SetBase(aTempObj.GetBase().toAsciiUpperCase());
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > xRef2 = new
                ucbhelper::ContentIdentifier( aTempObj.GetMainURL( INetURLObject::NO_DECODE ));

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
            OUString("IsReadOnly"));
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
bool UCB_GetFileListOfFolder( const OUString& rURL,
                                std::vector<OUString*>& rList,
                                const OUString* pExtension,
                                std::vector< ::DateTime* >* pDateTimeList )
{
    bool bOk = false;
    try
    {
        ucbhelper::Content aCnt( rURL, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;

        sal_uInt16 nSeqSize = pDateTimeList ? 2 : 1;
        ::com::sun::star::uno::Sequence < OUString > aProps( nSeqSize );
        OUString* pProps = aProps.getArray();
        pProps[ 0 ] = OUString("Title");
        if( pDateTimeList )
            pProps[ 1 ] = OUString("DateModified");

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
            const sal_Int32 nExtLen = pExtension ? pExtension->getLength() : 0;
            try
            {
                if( xResultSet->first() )
                {
                    do {
                        const OUString sTitle( xRow->getString( 1 ) );
                        if( !nExtLen ||
                            ( sTitle.getLength() > nExtLen &&
                              sTitle.endsWith( *pExtension )) )
                        {
                            rList.push_back( new OUString(sTitle) );

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
                                                aStamp.NanoSeconds ));
                                pDateTimeList->push_back( pDateTime );
                            }
                        }

                    } while( xResultSet->next() );
                }
                bOk = true;
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
        bOk = false;
    }
    return bOk;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
