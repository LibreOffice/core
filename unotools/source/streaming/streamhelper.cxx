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

#include <unotools/streamhelper.hxx>

namespace utl
{

void SAL_CALL OInputStreamHelper::acquire() throw ()
{
    InputStreamHelper_Base::acquire();
}

void SAL_CALL OInputStreamHelper::release() throw ()
{
    InputStreamHelper_Base::release();
}

sal_Int32 SAL_CALL OInputStreamHelper::readBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    if (!m_xLockBytes.Is())
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException(OUString(), static_cast<css::uno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );
    aData.realloc(nBytesToRead);

    sal_Size nRead(0);
    ErrCode nError = m_xLockBytes->ReadAt(m_nActPos, static_cast<void*>(aData.getArray()), nBytesToRead, &nRead);
    m_nActPos += nRead;

    if (nError != ERRCODE_NONE)
        throw css::io::IOException(OUString(), static_cast<css::uno::XWeak*>(this));

    // adjust sequence if data read is lower than the desired data
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

void SAL_CALL OInputStreamHelper::seek( sal_Int64 location ) throw(css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_nActPos = location;
}

sal_Int64 SAL_CALL OInputStreamHelper::getPosition(  ) throw(css::io::IOException, css::uno::RuntimeException, std::exception)
{
    return m_nActPos;
}

sal_Int64 SAL_CALL OInputStreamHelper::getLength(  ) throw(css::io::IOException, css::uno::RuntimeException, std::exception)
{
    if (!m_xLockBytes.Is())
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    SvLockBytesStat aStat;
    m_xLockBytes->Stat( &aStat, SVSTATFLAG_DEFAULT );
    return aStat.nSize;
}

sal_Int32 SAL_CALL OInputStreamHelper::readSomeBytes(css::uno::Sequence< sal_Int8 >& aData,
                                                     sal_Int32 nMaxBytesToRead)
    throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    // read all data desired
    return readBytes(aData, nMaxBytesToRead);
}

void SAL_CALL OInputStreamHelper::skipBytes(sal_Int32 nBytesToSkip)
    throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    if (nBytesToSkip < 0)
        throw css::io::BufferSizeExceededException(OUString(), static_cast<css::uno::XWeak*>(this));

    m_nActPos += nBytesToSkip;
}

sal_Int32 SAL_CALL OInputStreamHelper::available()
    throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    return m_nAvailable;
}

void SAL_CALL OInputStreamHelper::closeInput()
    throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    m_xLockBytes = nullptr;
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
