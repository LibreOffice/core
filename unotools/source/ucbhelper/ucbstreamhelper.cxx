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

#include <unotools/ucblockbytes.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>

#include <com/sun/star/ucb/XCommandEnvironment.hdl>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace utl
{

static SvStream* lcl_CreateStream( const String& rFileName, StreamMode eOpenMode,
        Reference < XInteractionHandler > xInteractionHandler,
        UcbLockBytesHandler* pHandler, sal_Bool /*bForceSynchron*/, sal_Bool bEnsureFileExists )
{
    SvStream* pStream = NULL;
    ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
    if ( pBroker )
    {
        UcbLockBytesRef xLockBytes;
        if ( eOpenMode & STREAM_WRITE )
        {
            sal_Bool bTruncate = ( eOpenMode & STREAM_TRUNC ) != 0;
            if ( bTruncate )
            {
                try
                {
                    // truncate is implemented with deleting the original file
                    ::ucbhelper::Content aCnt( rFileName, Reference < XCommandEnvironment >() );
                    aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
                }

                catch ( CommandAbortedException& )
                {
                    // couldn't truncate/delete
                }
                catch ( ContentCreationException& )
                {
                }
                catch ( Exception& )
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

                    ::ucbhelper::Content aContent( rFileName, Reference < XCommandEnvironment >() );
                    InsertCommandArgument aInsertArg;
                    aInsertArg.Data = xInput;

                    aInsertArg.ReplaceExisting = sal_False;
                    Any aCmdArg;
                    aCmdArg <<= aInsertArg;
                    aContent.executeCommand( ::rtl::OUString::createFromAscii( "insert" ), aCmdArg );
                }

                // it is NOT an error when the stream already exists and no truncation was desired
                catch ( CommandAbortedException& )
                {
                    // currently never an error is detected !
                }
                catch ( ContentCreationException& )
                {
                }
                catch ( Exception& )
                {
                }
            }
        }

        try
        {
            // create LockBytes using UCB
            ::ucbhelper::Content aContent( rFileName, Reference < XCommandEnvironment >() );
            xLockBytes = UcbLockBytes::CreateLockBytes( aContent.get(), Sequence < PropertyValue >(),
                                                eOpenMode, xInteractionHandler, pHandler );
            if ( xLockBytes.Is() )
            {
                pStream = new SvStream( xLockBytes );
                pStream->SetBufferSize( 4096 );
                pStream->SetError( xLockBytes->GetError() );
            }
        }
        catch ( CommandAbortedException& )
        {
        }
        catch ( ContentCreationException& )
        {
        }
        catch ( Exception& )
        {
        }
    }
    else
        // if no UCB is present at least conventional file io is supported
        pStream = new SvFileStream( rFileName, eOpenMode );

    return pStream;
}

//============================================================================

SvStream* UcbStreamHelper::CreateStream( const String& rFileName, StreamMode eOpenMode,
        UcbLockBytesHandler* pHandler, sal_Bool bForceSynchron )
{
    return lcl_CreateStream( rFileName, eOpenMode, Reference < XInteractionHandler >(), pHandler, bForceSynchron, sal_True /* bEnsureFileExists */ );
}

SvStream* UcbStreamHelper::CreateStream( const String& rFileName, StreamMode eOpenMode,
        Reference < XInteractionHandler > xInteractionHandler,
        UcbLockBytesHandler* pHandler, sal_Bool bForceSynchron )
{
    return lcl_CreateStream( rFileName, eOpenMode, xInteractionHandler, pHandler, bForceSynchron, sal_True /* bEnsureFileExists */ );
}

SvStream* UcbStreamHelper::CreateStream( const String& rFileName, StreamMode eOpenMode,
        sal_Bool bFileExists,
        UcbLockBytesHandler* pHandler, sal_Bool bForceSynchron )
{
    return lcl_CreateStream( rFileName, eOpenMode, Reference < XInteractionHandler >(), pHandler, bForceSynchron, !bFileExists );
}

SvStream* UcbStreamHelper::CreateStream( Reference < XInputStream > xStream )
{
    SvStream* pStream = NULL;
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
    SvStream* pStream = NULL;
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

SvStream* UcbStreamHelper::CreateStream( Reference < XInputStream > xStream, sal_Bool bCloseStream )
{
    SvStream* pStream = NULL;
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

SvStream* UcbStreamHelper::CreateStream( Reference < XStream > xStream, sal_Bool bCloseStream )
{
    SvStream* pStream = NULL;
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
