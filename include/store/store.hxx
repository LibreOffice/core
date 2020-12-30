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

#pragma once

#include <store/store.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <store/types.h>

namespace store
{

class OStoreStream
{
public:
    /** Construction.
     */
    OStoreStream()
        : m_hImpl (nullptr)
    {}

    /** Destruction.
     */
    ~OStoreStream()
    {
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
    }

    /** Copy construction.
     */
    OStoreStream (OStoreStream const & rhs)
        : m_hImpl (rhs.m_hImpl)
    {
        if (m_hImpl)
            (void) store_acquireHandle (m_hImpl);
    }

    /** Assignment.
     */
    OStoreStream & operator= (OStoreStream const & rhs)
    {
        if (rhs.m_hImpl)
            (void) store_acquireHandle (rhs.m_hImpl);
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
        m_hImpl = rhs.m_hImpl;
        return *this;
    }

    /** Open the stream.
        @see store_openStream()
     */
    storeError create (
        storeFileHandle       hFile,
        OUString const & rPath,
        OUString const & rName,
        storeAccessMode       eMode)
    {
        if (m_hImpl)
        {
            (void) store_releaseHandle (m_hImpl);
            m_hImpl = nullptr;
        }
        return store_openStream (hFile, rPath.pData, rName.pData, eMode, &m_hImpl);
    }

    /** Read from the stream.
        @see store_readStream()
     */
    storeError readAt (
        sal_uInt32   nOffset,
        void *       pBuffer,
        sal_uInt32   nBytes,
        sal_uInt32 & rnDone)
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_readStream (m_hImpl, nOffset, pBuffer, nBytes, &rnDone);
    }

    /** Write to the stream.
        @see store_writeStream()
     */
    storeError writeAt (
        sal_uInt32   nOffset,
        void const * pBuffer,
        sal_uInt32   nBytes,
        sal_uInt32 & rnDone)
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_writeStream (m_hImpl, nOffset, pBuffer, nBytes, &rnDone);
    }

private:
    /** Representation.
     */
    storeStreamHandle m_hImpl;
};

class OStoreDirectory
{
public:
    /** Construction.
     */
    OStoreDirectory()
        : m_hImpl (nullptr)
    {}

    /** Destruction.
     */
    ~OStoreDirectory()
    {
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
    }

    /** Copy construction.
     */
    OStoreDirectory (OStoreDirectory const & rhs)
        : m_hImpl (rhs.m_hImpl)
    {
        if (m_hImpl)
            (void) store_acquireHandle (m_hImpl);
    }

    /** Move construction.
     */
    OStoreDirectory (OStoreDirectory && rhs) noexcept
        : m_hImpl (rhs.m_hImpl)
    {
        rhs.m_hImpl = nullptr;
    }

    /** Assignment.
     */
    OStoreDirectory & operator= (OStoreDirectory const & rhs)
    {
        if (rhs.m_hImpl)
            (void) store_acquireHandle (rhs.m_hImpl);
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
        m_hImpl = rhs.m_hImpl;
        return *this;
    }

    /** Move assignment.
     */
    OStoreDirectory & operator= (OStoreDirectory && rhs) noexcept
    {
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
        m_hImpl = rhs.m_hImpl;
        rhs.m_hImpl = nullptr;
        return *this;
    }

    /** Open the directory.
        @see store_openDirectory()
     */
    storeError create (
        storeFileHandle       hFile,
        OUString const & rPath,
        OUString const & rName,
        storeAccessMode       eMode)
    {
        if (m_hImpl)
        {
            (void) store_releaseHandle (m_hImpl);
            m_hImpl = nullptr;
        }
        return store_openDirectory (hFile, rPath.pData, rName.pData, eMode, &m_hImpl);
    }

    /** Directory iterator type.
        @see first()
        @see next()
     */
    typedef storeFindData iterator;

    /** Find first directory entry.
        @see store_findFirst()
     */
    storeError first (iterator& it)
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_findFirst (m_hImpl, &it);
    }

    /** Find next directory entry.
        @see store_findNext()
     */
    storeError next (iterator& it)
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_findNext (m_hImpl, &it);
    }

private:
    /** Representation.
     */
    storeDirectoryHandle m_hImpl;
};

class OStoreFile
{
public:
    /** Construction.
     */
    OStoreFile()
        : m_hImpl (nullptr)
    {}

    /** Destruction.
     */
    ~OStoreFile()
    {
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
    }

    /** Copy construction.
     */
    OStoreFile (OStoreFile const & rhs)
        : m_hImpl (rhs.m_hImpl)
    {
        if (m_hImpl)
            (void) store_acquireHandle (m_hImpl);
    }

    /** Assignment.
     */
    OStoreFile & operator= (OStoreFile const & rhs)
    {
        if (rhs.m_hImpl)
            (void) store_acquireHandle (rhs.m_hImpl);
        if (m_hImpl)
            (void) store_releaseHandle (m_hImpl);
        m_hImpl = rhs.m_hImpl;
        return *this;
    }

    /** Conversion into File Handle.
     */
    operator storeFileHandle() const
    {
        return m_hImpl;
    }

    /** Check for a valid File Handle.
        @return sal_True if valid, sal_False otherwise.
     */
    bool isValid() const
    {
        return (m_hImpl != nullptr);
    }

    /** Open the file.
        @see store_openFile()
     */
    storeError create(
        OUString const & rFilename,
        storeAccessMode       eAccessMode )
    {
        if (m_hImpl)
        {
            (void) store_releaseHandle (m_hImpl);
            m_hImpl = nullptr;
        }
        return store_openFile (rFilename.pData, eAccessMode, STORE_DEFAULT_PAGESIZE, &m_hImpl);
    }

    /** Open the temporary file in memory.
        @see store_createMemoryFile()
     */
    storeError createInMemory ()
    {
        if (m_hImpl)
        {
            (void) store_releaseHandle (m_hImpl);
            m_hImpl = nullptr;
        }
        return store_createMemoryFile (STORE_DEFAULT_PAGESIZE, &m_hImpl);
    }

    /** Close the file.
        @see store_closeFile()
     */
    void close()
    {
        if (m_hImpl)
        {
            (void) store_closeFile (m_hImpl);
            m_hImpl = nullptr;
        }
    }

    /** Flush the file.
        @see store_flushFile()
     */
    storeError flush() const
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_flushFile (m_hImpl);
    }

    /** Remove a file entry.
        @see store_remove()
     */
    storeError remove (
        OUString const & rPath, OUString const & rName)
    {
        if (!m_hImpl)
            return store_E_InvalidHandle;

        return store_remove (m_hImpl, rPath.pData, rName.pData);
    }

private:
    /** Representation.
     */
    storeFileHandle m_hImpl;
};

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
