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
#include <package/packagedllapi.hxx>
#include <comphelper/threadpool.hxx>
#include <atomic>
#include <memory>

namespace ZipUtils
{
/// Parallel compression a stream using the libz deflate algorithm.
///
/// Almost a replacement for the Deflater class. Call startDeflate() with the data,
/// check with finished() or waitForTasks() and retrieve result with getOutput().
/// The class will internally split into multiple threads.
class ThreadedDeflater final
{
    class Task;
    // Note: All this should be lock-less. Each task writes only to its part
    // of the data, flags are atomic.
    css::uno::Sequence<sal_Int8> inBuffer;
    int zlibLevel;
    std::shared_ptr<comphelper::ThreadTaskTag> threadTaskTag;
    std::atomic<int> pendingTasksCount;
    std::vector<std::vector<sal_Int8>> outBuffers;

public:
    // Unlike with Deflater class, bNoWrap is always true.
    ThreadedDeflater(sal_Int32 nSetLevel);
    ~ThreadedDeflater() COVERITY_NOEXCEPT_FALSE;
    void startDeflate(const css::uno::Sequence<sal_Int8>& rBuffer);
    void waitForTasks();
    bool finished() const;
    css::uno::Sequence<sal_Int8> getOutput() const;
    void clear();
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
