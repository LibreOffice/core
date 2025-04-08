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
#ifndef INCLUDED_PACKAGE_INC_ZIPOUTPUTSTREAM_HXX
#define INCLUDED_PACKAGE_INC_ZIPOUTPUTSTREAM_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

#include "ByteChucker.hxx"
#include <comphelper/threadpool.hxx>

#include <cstddef>
#include <exception>
#include <vector>

struct ZipEntry;
class ZipOutputEntryInThread;

class ZipOutputStream
{
    css::uno::Reference< css::io::XOutputStream > m_xStream;
    std::vector<std::unique_ptr<ZipEntry>> m_aZipList;
    std::shared_ptr<comphelper::ThreadTaskTag> mpThreadTaskTag;

    ByteChucker         m_aChucker;
    ZipEntry            *m_pCurrentEntry;
    std::vector< ZipOutputEntryInThread* > m_aEntries;
    std::exception_ptr m_aDeflateException;

public:
    ZipOutputStream(
        const css::uno::Reference< css::io::XOutputStream > &xOStream );
    ~ZipOutputStream();

    void addDeflatingThreadTask( ZipOutputEntryInThread *pEntry, std::unique_ptr<comphelper::ThreadTask> pThreadTask );

    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void writeLOC(std::unique_ptr<ZipEntry>&& pEntry, bool bEncrypt = false);
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void rawWrite( const css::uno::Sequence< sal_Int8 >& rBuffer );
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void rawCloseEntry( bool bEncrypt = false );

    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void finish();
    const css::uno::Reference< css::io::XOutputStream >& getStream() const;

    static sal_uInt32 getCurrentDosTime();
    static void setEntry(ZipEntry& rEntry);

private:
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void writeEND(sal_uInt32 nOffset, sal_uInt32 nLength);
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void writeCEN( const ZipEntry &rEntry );
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void writeDataDescriptor( const ZipEntry &rEntry );
    void writeExtraFields( const ZipEntry& rEntry );

    // ScheduledThread handling helpers
    void consumeScheduledThreadTaskEntry(std::unique_ptr<ZipOutputEntryInThread> pCandidate);
    void consumeFinishedScheduledThreadTaskEntries();

public:
    void reduceScheduledThreadTasksToGivenNumberOrLess(
        std::size_t nThreadTasks);

    const std::shared_ptr<comphelper::ThreadTaskTag>& getThreadTaskTag() const { return mpThreadTaskTag; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
