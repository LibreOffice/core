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

#include <rtl/ustring.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>

#include <ucbhelper/content.hxx>
#include <unotools/streamwrap.hxx>
#include <ucblockbytes.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace utl
{

static SvStream* lcl_CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                   Reference < XInteractionHandler > xInteractionHandler,
                                   UcbLockBytesHandler* pHandler, bool bEnsureFileExists )
{
    SvStream* pStream = nullptr;
    Reference< XUniversalContentBroker > ucb(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    UcbLockBytesRef xLockBytes;
    if ( eOpenMode & StreamMode::WRITE )
    {
        bool bTruncate = bool( eOpenMode & StreamMode::TRUNC );
        if ( bTruncate )
        {
            try
            {
                // truncate is implemented with deleting the original file
                ::ucbhelper::Content aCnt(
                    rFileName, Reference < XCommandEnvironment >(),
                    comphelper::getProcessComponentContext() );
                aCnt.executeCommand( "delete", makeAny( true ) );
            }

            catch ( const CommandAbortedException& )
            {
                // couldn't truncate/delete
            }
            catch ( const ContentCreationException& )
            {
            }
            catch ( const Exception& )
            {
            }
        }

        if ( bEnsureFileExists || bTruncate )
        {
            try
            {
                // make sure that the desired file exists before trying to open
                SvMemoryStream aStream(0,0);
                ::utl::OInputStreamWrapper* pInput = new ::utl::OInputStreamWrapper( aStream );
                Reference< XInputStream > xInput( pInput );

                ::ucbhelper::Content aContent(
                    rFileName, Reference < XCommandEnvironment >(),
                    comphelper::getProcessComponentContext() );
                InsertCommandArgument aInsertArg;
                aInsertArg.Data = xInput;

                aInsertArg.ReplaceExisting = sal_False;
                Any aCmdArg;
                aCmdArg <<= aInsertArg;
                aContent.executeCommand( "insert", aCmdArg );
            }

            // it is NOT an error when the stream already exists and no truncation was desired
            catch ( const CommandAbortedException& )
            {
                // currently never an error is detected !
            }
            catch ( const ContentCreationException& )
            {
            }
            catch ( const Exception& )
            {
            }
        }
    }

    try
    {
        // create LockBytes using UCB
        ::ucbhelper::Content aContent(
            rFileName, Reference < XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        xLockBytes = UcbLockBytes::CreateLockBytes( aContent.get(), Sequence < PropertyValue >(),
                                                    eOpenMode, xInteractionHandler, pHandler );
        if ( xLockBytes.Is() )
        {
            pStream = new SvStream( xLockBytes );
            pStream->SetBufferSize( 4096 );
            pStream->SetError( xLockBytes->GetError() );
        }
    }
    catch ( const CommandAbortedException& )
    {
    }
    catch ( const ContentCreationException& )
    {
    }
    catch ( const Exception& )
    {
    }

    return pStream;
}

SvStream* UcbStreamHelper::CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                         UcbLockBytesHandler* pHandler )
{
    return lcl_CreateStream( rFileName, eOpenMode, Reference < XInteractionHandler >(), pHandler, true /* bEnsureFileExists */ );
}

SvStream* UcbStreamHelper::CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                         Reference < XInteractionHandler > xInteractionHandler,
                                         UcbLockBytesHandler* pHandler )
{
    return lcl_CreateStream( rFileName, eOpenMode, xInteractionHandler, pHandler, true /* bEnsureFileExists */ );
}

SvStream* UcbStreamHelper::CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                         bool bFileExists,
                                         UcbLockBytesHandler* pHandler )
{
    return lcl_CreateStream( rFileName, eOpenMode, Reference < XInteractionHandler >(), pHandler, !bFileExists );
}

SvStream* UcbStreamHelper::CreateStream( Reference < XInputStream > xStream )
{
    SvStream* pStream = nullptr;
    UcbLockBytesRef xLockBytes = UcbLockBytes::CreateInputLockBytes( xStream );
    if ( xLockBytes.Is() )
    {
        pStream = new SvStream( xLockBytes );
        pStream->SetBufferSize( 4096 );
        pStream->SetError( xLockBytes->GetError() );
    }

    return pStream;
}

SvStream* UcbStreamHelper::CreateStream( Reference < XStream > xStream )
{
    SvStream* pStream = nullptr;
    if ( xStream->getOutputStream().is() )
    {
        UcbLockBytesRef xLockBytes = UcbLockBytes::CreateLockBytes( xStream );
        if ( xLockBytes.Is() )
        {
            pStream = new SvStream( xLockBytes );
            pStream->SetBufferSize( 4096 );
            pStream->SetError( xLockBytes->GetError() );
        }
    }
    else
        return CreateStream( xStream->getInputStream() );

    return pStream;
}

SvStream* UcbStreamHelper::CreateStream( Reference < XInputStream > xStream, bool bCloseStream )
{
    SvStream* pStream = nullptr;
    UcbLockBytesRef xLockBytes = UcbLockBytes::CreateInputLockBytes( xStream );
    if ( xLockBytes.Is() )
    {
        if ( !bCloseStream )
            xLockBytes->setDontClose_Impl();

        pStream = new SvStream( xLockBytes );
        pStream->SetBufferSize( 4096 );
        pStream->SetError( xLockBytes->GetError() );
    }

    return pStream;
};

SvStream* UcbStreamHelper::CreateStream( Reference < XStream > xStream, bool bCloseStream )
{
    SvStream* pStream = nullptr;
    if ( xStream->getOutputStream().is() )
    {
        UcbLockBytesRef xLockBytes = UcbLockBytes::CreateLockBytes( xStream );
        if ( xLockBytes.Is() )
        {
            if ( !bCloseStream )
                xLockBytes->setDontClose_Impl();

            pStream = new SvStream( xLockBytes );
            pStream->SetBufferSize( 4096 );
            pStream->SetError( xLockBytes->GetError() );
        }
    }
    else
        return CreateStream( xStream->getInputStream(), bCloseStream );

    return pStream;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
