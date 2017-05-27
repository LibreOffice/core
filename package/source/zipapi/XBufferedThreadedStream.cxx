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

#include <XBufferedThreadedStream.hxx>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

using namespace css::uno;
using com::sun::star::packages::zip::ZipIOException;

namespace {

class UnzippingThread: public salhelper::Thread
{
    XBufferedThreadedStream *mxStream;
public:
    explicit UnzippingThread(XBufferedThreadedStream *xStream): Thread("Unzipping"), mxStream(xStream) {}
private:
    virtual void execute() override
    {
        try
        {
            mxStream->produce();
        }
        catch( const RuntimeException &e )
        {
            SAL_WARN("XBufferedThreadedStream", "RuntimeException from unbuffered Stream " << e.Message );
            mxStream->saveException( new RuntimeException( e ) );
        }
        catch( const ZipIOException &e )
        {
            SAL_WARN("XBufferedThreadedStream", "ZipIOException from unbuffered Stream " << e.Message );
            mxStream->saveException( new ZipIOException( e ) );
        }
        catch( const Exception &e )
        {
            SAL_WARN("XBufferedThreadedStream", "Unexpected exception " << e.Message );
            mxStream->saveException( new Exception( e ) );
        }

        mxStream->setTerminateThread();
    }
};

}

XBufferedThreadedStream::XBufferedThreadedStream(
                    const Reference<XInputStream>& xSrcStream )
: mxSrcStream( xSrcStream )
, mnPos(0)
, mnStreamSize( xSrcStream->available() )
, mnOffset( 0 )
, mxUnzippingThread( new UnzippingThread(this) )
, mbTerminateThread( false )
, maSavedException( nullptr )
{
    mxUnzippingThread->launch();
}

XBufferedThreadedStream::~XBufferedThreadedStream()
{
    setTerminateThread();
    mxUnzippingThread->join();
}

void XBufferedThreadedStream::produce()
{
    Buffer pProducedBuffer;
    std::unique_lock<std::mutex> aGuard( maBufferProtector );
    do
    {
        if( !maUsedBuffers.empty() )
        {
            pProducedBuffer = maUsedBuffers.front();
            maUsedBuffers.pop();
        }

        aGuard.unlock();
        mxSrcStream->readBytes( pProducedBuffer, nBufferSize );

        aGuard.lock();
        maPendingBuffers.push( pProducedBuffer );
        maBufferConsumeResume.notify_one();
        maBufferProduceResume.wait( aGuard, [&]{return canProduce(); } );

        if( mbTerminateThread )
            break;

    } while( hasBytes() );
}

const Buffer& XBufferedThreadedStream::getNextBlock()
{
    const sal_Int32 nBufSize = maInUseBuffer.getLength();
    if( nBufSize <= 0 || mnOffset >= nBufSize )
    {
        std::unique_lock<std::mutex> aGuard( maBufferProtector );
        if( mnOffset >= nBufSize )
            maUsedBuffers.push( maInUseBuffer );

        maBufferConsumeResume.wait( aGuard, [&]{return canConsume(); } );

        if( maPendingBuffers.empty() )
        {
            maInUseBuffer = Buffer();
            if( maSavedException )
                throw *maSavedException;
        }
        else
        {
            maInUseBuffer = maPendingBuffers.front();
            maPendingBuffers.pop();
            mnOffset = 0;

            if( maPendingBuffers.size() <= nBufferLowWater )
                maBufferProduceResume.notify_one();
        }
    }

    return maInUseBuffer;
}

void XBufferedThreadedStream::setTerminateThread()
{
    mbTerminateThread = true;
    maBufferProduceResume.notify_one();
    maBufferConsumeResume.notify_one();
}

sal_Int32 SAL_CALL XBufferedThreadedStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
{
    if( !hasBytes() )
        return 0;

    const sal_Int32 nAvailableSize = std::min<sal_Int32>( nBytesToRead, remainingSize() );
    rData.realloc( nAvailableSize );
    sal_Int32 i = 0, nPendingBytes = nAvailableSize;

    while( nPendingBytes )
    {
        const Buffer &pBuffer = getNextBlock();
        if( pBuffer.getLength() <= 0 )
        {
            rData.realloc( nAvailableSize - nPendingBytes );
            return nAvailableSize - nPendingBytes;
        }
        const sal_Int32 limit = std::min<sal_Int32>( nPendingBytes, pBuffer.getLength() - mnOffset );

        memcpy( &rData[i], &pBuffer[mnOffset], limit );

        nPendingBytes -= limit;
        mnOffset += limit;
        mnPos += limit;
        i += limit;
    }

    return nAvailableSize;
}

sal_Int32 SAL_CALL XBufferedThreadedStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes( aData, nMaxBytesToRead );
}
void SAL_CALL XBufferedThreadedStream::skipBytes( sal_Int32 nBytesToSkip )
{
    if( nBytesToSkip )
    {
        Sequence < sal_Int8 > aSequence( nBytesToSkip );
        readBytes( aSequence, nBytesToSkip );
    }
}

sal_Int32 SAL_CALL XBufferedThreadedStream::available()
{
    if( !hasBytes() )
        return 0;

    return remainingSize();
}

void SAL_CALL XBufferedThreadedStream::closeInput()
{
    setTerminateThread();
    mxUnzippingThread->join();
    mxSrcStream->closeInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

