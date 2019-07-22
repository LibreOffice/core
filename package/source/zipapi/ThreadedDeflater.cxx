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

public:
    Task(ThreadedDeflater* deflater_, int sequence_, int blockSize_)
        : comphelper::ThreadTask(deflater_->threadTaskTag)
        , stream()
        , deflater(deflater_)
        , sequence(sequence_)
        , blockSize(blockSize_)
    {
    }

private:
    virtual void doWork() override;
};

ThreadedDeflater::ThreadedDeflater(sal_Int32 nSetLevel)
    : threadTaskTag(comphelper::ThreadPool::createThreadTaskTag())
    , zlibLevel(nSetLevel)
    , pendingTasksCount(0)
{
}

ThreadedDeflater::~ThreadedDeflater() COVERITY_NOEXCEPT_FALSE
{
    waitForTasks();
    clear();
}

void ThreadedDeflater::startDeflate(const uno::Sequence<sal_Int8>& rBuffer)
{
    inBuffer = rBuffer;
    sal_Int64 size = inBuffer.getLength();
    int tasksCount = (size + MaxBlockSize - 1) / MaxBlockSize;
    tasksCount = std::max(tasksCount, 1);
    pendingTasksCount = tasksCount;
    outBuffers.resize(pendingTasksCount);
    for (int sequence = 0; sequence < tasksCount; ++sequence)
    {
        sal_Int64 thisSize = std::min(MaxBlockSize, size);
        size -= thisSize;
        comphelper::ThreadPool::getSharedOptimalPool().pushTask(
            std::make_unique<Task>(this, sequence, thisSize));
    }
    assert(size == 0);
}

bool ThreadedDeflater::finished() const { return pendingTasksCount == 0; }

css::uno::Sequence<sal_Int8> ThreadedDeflater::getOutput() const
{
    assert(finished());
    sal_Int64 totalSize = 0;
    for (const auto& buffer : outBuffers)
        totalSize += buffer.size();
    uno::Sequence<sal_Int8> outBuffer(totalSize);
    auto pos = outBuffer.begin();
    for (const auto& buffer : outBuffers)
        pos = std::copy(buffer.begin(), buffer.end(), pos);
    return outBuffer;
}

void ThreadedDeflater::waitForTasks()
{
    comphelper::ThreadPool::getSharedOptimalPool().waitUntilDone(threadTaskTag);
}

void ThreadedDeflater::clear()
{
    assert(finished());
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
    if (sequence != 0)
    {
        // the window size is 32k, so set last 32k of previous data as the dictionary
        assert(MAX_WBITS == 15);
        assert(MaxBlockSize >= 32768);
        deflateSetDictionary(&stream, inBufferPtr + myInBufferStart - 32768, 32768);
    }
    stream.next_in = inBufferPtr + myInBufferStart;
    stream.avail_in = blockSize;
    stream.next_out = reinterpret_cast<unsigned char*>(deflater->outBuffers[sequence].data());
    stream.avail_out = outputMaxSize;
    bool last = sequence == int(deflater->outBuffers.size() - 1); // Last block?
    // The trick is in using Z_SYNC_FLUSH instead of Z_NO_FLUSH. It will align the data at a byte boundary,
    // and since we use a raw stream, the data blocks then can be simply concatenated.
    int res = deflate(&stream, last ? Z_FINISH : Z_SYNC_FLUSH);
    assert(stream.avail_in == 0); // Check that everything has been deflated.
    if (last ? res == Z_STREAM_END : res == Z_OK)
    { // ok
        sal_Int64 outSize = outputMaxSize - stream.avail_out;
        deflater->outBuffers[sequence].resize(outSize);
        --deflater->pendingTasksCount;
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
