/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_PACKAGE_SOURCE_ZIPAPI_XBUFFEREDTHREADEDSTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPAPI_XBUFFEREDTHREADEDSTREAM_HXX

#include <salhelper/thread.hxx>
#include <XUnbufferedStream.hxx>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

typedef css::uno::Sequence< sal_Int8 > Buffer;

class XBufferedThreadedStream : public cppu::WeakImplHelper< css::io::XInputStream >
{
private:
    const css::uno::Reference<XInputStream> mxSrcStream;
    sal_Int64 mnPos;                                           /// position in stream
    sal_Int64 mnStreamSize;                                    /// available size of stream

    Buffer maInUseBuffer;                                   /// Buffer block in use
    int mnOffset;                                           /// position in maInUseBuffer
    std::queue < Buffer > maPendingBuffers;                 /// Buffers that are available for use
    std::queue < Buffer > maUsedBuffers;

    rtl::Reference< salhelper::Thread > mxUnzippingThread;
    std::mutex maBufferProtector;                           /// mutex protecting Buffer queues.
    std::condition_variable maBufferConsumeResume;
    std::condition_variable maBufferProduceResume;
    bool mbTerminateThread;                                 /// indicates the failure of one of the threads

    css::uno::Exception *maSavedException;                  /// exception caught during unzipping is saved to be thrown during reading

    static const size_t nBufferLowWater = 2;
    static const size_t nBufferHighWater = 4;
    static const size_t nBufferSize = 32 * 1024;

    const Buffer& getNextBlock();
    size_t remainingSize() const { return mnStreamSize - mnPos; }
    bool hasBytes() const { return mnPos < mnStreamSize; }

    bool canProduce() const
    {
        return( mbTerminateThread || maPendingBuffers.size() < nBufferHighWater );
    }

    bool canConsume() const
    {
        return( mbTerminateThread || !maPendingBuffers.empty() );
    }

public:
    XBufferedThreadedStream(
                  const css::uno::Reference<XInputStream>& xSrcStream,
                  sal_Int64 nStreamSize /* cf. sal_Int32 available(); */ );

    virtual ~XBufferedThreadedStream() override;

    void produce();
    void setTerminateThread();
    void saveException( css::uno::Exception *e ) { maSavedException = e; }

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
