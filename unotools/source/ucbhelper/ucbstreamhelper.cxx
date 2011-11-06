/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/ucblockbytes.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HDL_
#include <com/sun/star/ucb/XCommandEnvironment.hdl>
#endif
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
