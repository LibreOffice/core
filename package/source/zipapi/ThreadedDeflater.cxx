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

#include <ThreadedDeflater.hxx>
#include <zlib.h>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <sal/log.hxx>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star;

namespace ZipUtils
{
const sal_Int64 MaxBlockSize = 128 * 1024;

// Parallel ZLIB compression using threads. The class internally splits the data into
// blocks and spawns ThreadPool tasks to process them independently. This is achieved
// in a similar way how pigz works, see comments from Mark Adler at
// https://stackoverflow.com/questions/30294766/how-to-use-multiple-threads-for-zlib-compression
// and
// https://stackoverflow.com/questions/30794053/how-to-use-multiple-threads-for-zlib-compression-same-input-source

// Everything here should be either read-only, or writing to distinct data, or atomic.

class ThreadedDeflater::Task : public comphelper::ThreadTask
{
    z_stream stream;
    ThreadedDeflater* deflater;
    int sequence;
    int blockSize;
    bool firstTask : 1;
    bool lastTask : 1;

public:
    Task(ThreadedDeflater* deflater_, int sequence_, int blockSize_, bool firstTask_,
         bool lastTask_)
        : comphelper::ThreadTask(deflater_->threadTaskTag)
        , stream()
        , deflater(deflater_)
        , sequence(sequence_)
        , blockSize(blockSize_)
        , firstTask(firstTask_)
        , lastTask(lastTask_)
    {
    }

private:
    virtual void doWork() override;
};

ThreadedDeflater::ThreadedDeflater(sal_Int32 nSetLevel)
    : threadTaskTag(comphelper::ThreadPool::createThreadTaskTag())
    , totalIn(0)
    , totalOut(0)
    , zlibLevel(nSetLevel)
{
}

ThreadedDeflater::~ThreadedDeflater() COVERITY_NOEXCEPT_FALSE { clear(); }

void ThreadedDeflater::deflateWrite(
    const css::uno::Reference<css::io::XInputStream>& xInStream,
    std::function<void(const css::uno::Sequence<sal_Int8>&, sal_Int32)> aProcessInputFunc,
    std::function<void(const css::uno::Sequence<sal_Int8>&, sal_Int32)> aProcessOutputFunc)
{
    sal_Int64 nThreadCount = comphelper::ThreadPool::getSharedOptimalPool().getWorkerCount();
    sal_Int64 batchSize = MaxBlockSize * nThreadCount;
    inBuffer.realloc(batchSize);
    prevDataBlock.realloc(MaxBlockSize);
    outBuffers.resize(nThreadCount);
    maProcessOutputFunc = aProcessOutputFunc;
    bool firstTask = true;

    while (xInStream->available() > 0)
    {
        sal_Int64 inputBytes = xInStream->readBytes(inBuffer, batchSize);
        aProcessInputFunc(inBuffer, inputBytes);
        totalIn += inputBytes;
        int sequence = 0;
        bool lastBatch = xInStream->available() <= 0;
        sal_Int64 bytesPending = inputBytes;
        while (bytesPending > 0)
        {
            sal_Int64 taskSize = std::min(MaxBlockSize, bytesPending);
            bytesPending -= taskSize;
            bool lastTask = lastBatch && !bytesPending;
            comphelper::ThreadPool::getSharedOptimalPool().pushTask(
                std::make_unique<Task>(this, sequence++, taskSize, firstTask, lastTask));

            if (firstTask)
                firstTask = false;
        }

        assert(bytesPending == 0);

        comphelper::ThreadPool::getSharedOptimalPool().waitUntilDone(threadTaskTag);

        if (!lastBatch)
        {
            assert(inputBytes == batchSize);
            std::copy_n(std::cbegin(inBuffer) + (batchSize - MaxBlockSize), MaxBlockSize,
                        prevDataBlock.getArray());
        }

        processDeflatedBuffers();
    }
}

void ThreadedDeflater::processDeflatedBuffers()
{
    sal_Int64 batchOutputSize = 0;
    for (const auto& buffer : outBuffers)
        batchOutputSize += buffer.size();

    css::uno::Sequence<sal_Int8> outBuffer(batchOutputSize);

    auto pos = outBuffer.getArray();
    for (auto& buffer : outBuffers)
    {
        pos = std::copy(buffer.begin(), buffer.end(), pos);
        buffer.clear();
    }

    maProcessOutputFunc(outBuffer, batchOutputSize);
    totalOut += batchOutputSize;
}

void ThreadedDeflater::clear()
{
    inBuffer = uno::Sequence<sal_Int8>();
    outBuffers.clear();
}

#if defined Z_PREFIX
#define deflateInit2 z_deflateInit2
#define deflateBound z_deflateBound
#define deflateSetDictionary z_deflateSetDictionary
#define deflate z_deflate
#define deflateEnd z_deflateEnd
#endif

void ThreadedDeflater::Task::doWork()
{
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    // -MAX_WBITS means 32k window size and raw stream
    if (deflateInit2(&stream, deflater->zlibLevel, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL,
                     Z_DEFAULT_STRATEGY)
        != Z_OK)
    {
        SAL_WARN("package.threadeddeflate", "deflateInit2() failed");
        abort();
    }
    // Find out size for our output buffer to be large enough for deflate() needing to be called just once.
    sal_Int64 outputMaxSize = deflateBound(&stream, blockSize);
    // add extra size for Z_SYNC_FLUSH
    outputMaxSize += 20;
    deflater->outBuffers[sequence].resize(outputMaxSize);
    sal_Int64 myInBufferStart = sequence * MaxBlockSize;
    // zlib doesn't handle const properly
    unsigned char* inBufferPtr = reinterpret_cast<unsigned char*>(
        const_cast<signed char*>(deflater->inBuffer.getConstArray()));
    if (!firstTask)
    {
        // the window size is 32k, so set last 32k of previous data as the dictionary
        assert(MAX_WBITS == 15);
        assert(MaxBlockSize >= 32768);
        if (sequence > 0)
        {
            deflateSetDictionary(&stream, inBufferPtr + myInBufferStart - 32768, 32768);
        }
        else
        {
            unsigned char* prevBufferPtr = reinterpret_cast<unsigned char*>(
                const_cast<signed char*>(deflater->prevDataBlock.getConstArray()));
            deflateSetDictionary(&stream, prevBufferPtr + MaxBlockSize - 32768, 32768);
        }
    }
    stream.next_in = inBufferPtr + myInBufferStart;
    stream.avail_in = blockSize;
    stream.next_out = reinterpret_cast<unsigned char*>(deflater->outBuffers[sequence].data());
    stream.avail_out = outputMaxSize;

    // The trick is in using Z_SYNC_FLUSH instead of Z_NO_FLUSH. It will align the data at a byte boundary,
    // and since we use a raw stream, the data blocks then can be simply concatenated.
    int res = deflate(&stream, lastTask ? Z_FINISH : Z_SYNC_FLUSH);
    assert(stream.avail_in == 0); // Check that everything has been deflated.
    if (lastTask ? res == Z_STREAM_END : res == Z_OK)
    { // ok
        sal_Int64 outSize = outputMaxSize - stream.avail_out;
        deflater->outBuffers[sequence].resize(outSize);
    }
    else
    {
        SAL_WARN("package.threadeddeflate", "deflate() failed");
        abort();
    }
    deflateEnd(&stream);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
