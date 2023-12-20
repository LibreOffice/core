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

#ifndef INCLUDED_PACKAGE_THREADEDDEFLATER_HXX
#define INCLUDED_PACKAGE_THREADEDDEFLATER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/threadpool.hxx>
#include <memory>
#include <vector>
#include <functional>

namespace ZipUtils
{
/// Parallel compression a stream using the libz deflate algorithm.
///
/// Call deflateWrite() with the input stream and input/output processing functions.
/// This will use multiple threads for compression on each batch of data from the stream.
class ThreadedDeflater final
{
    class Task;
    // Note: All this should be lock-less. Each task writes only to its part
    // of the data.
    std::vector<std::vector<sal_Int8>> outBuffers;
    std::shared_ptr<comphelper::ThreadTaskTag> threadTaskTag;
    css::uno::Sequence<sal_Int8> inBuffer;
    css::uno::Sequence<sal_Int8> prevDataBlock;
    std::function<void(const css::uno::Sequence<sal_Int8>&, sal_Int32)> maProcessOutputFunc;
    sal_Int64 totalIn;
    sal_Int64 totalOut;
    int zlibLevel;

public:
    // Unlike with Deflater class, bNoWrap is always true.
    ThreadedDeflater(sal_Int32 nSetLevel);
    ~ThreadedDeflater() COVERITY_NOEXCEPT_FALSE;
    void deflateWrite(const css::uno::Reference<css::io::XInputStream>& xInStream,
                      const std::function<void(const css::uno::Sequence<sal_Int8>&, sal_Int32)>&
                          rProcessInputFunc,
                      const std::function<void(const css::uno::Sequence<sal_Int8>&, sal_Int32)>&
                          rProcessOutputFunc);
    sal_Int64 getTotalIn() const { return totalIn; }
    sal_Int64 getTotalOut() const { return totalOut; }

private:
    void processDeflatedBuffers();
    void clear();
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
