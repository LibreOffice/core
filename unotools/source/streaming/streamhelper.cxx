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


sal_Int32 SAL_CALL OInputStreamHelper::readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception)
{
    if (!m_xLockBytes.Is())
        throw stario::NotConnectedException(OUString(), static_cast<staruno::XWeak*>(this));

    if (nBytesToRead < 0)
        throw stario::BufferSizeExceededException(OUString(), static_cast<staruno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );
    aData.realloc(nBytesToRead);

    sal_Size nRead;
    ErrCode nError = m_xLockBytes->ReadAt(m_nActPos, (void*)aData.getArray(), nBytesToRead, &nRead);
    // FIXME  nRead could be truncated on 64-bit arches
    m_nActPos += (sal_uInt32)nRead;

    if (nError != ERRCODE_NONE)
        throw stario::IOException(OUString(), static_cast<staruno::XWeak*>(this));

    // adjust sequence if data read is lower than the desired data
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

void SAL_CALL OInputStreamHelper::seek( sal_Int64 location ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // cast is truncating, but position would be truncated as soon as
    // put into SvLockBytes anyway
    m_nActPos = sal::static_int_cast<sal_uInt32>(location);
}

sal_Int64 SAL_CALL OInputStreamHelper::getPosition(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_nActPos;
}

sal_Int64 SAL_CALL OInputStreamHelper::getLength(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    if (!m_xLockBytes.Is())
        return 0;

    ::osl::MutexGuard aGuard( m_aMutex );
    SvLockBytesStat aStat;
    m_xLockBytes->Stat( &aStat, SVSTATFLAG_DEFAULT );
    return aStat.nSize;
}


sal_Int32 SAL_CALL OInputStreamHelper::readSomeBytes(staruno::Sequence< sal_Int8 >& aData,
                                                     sal_Int32 nMaxBytesToRead)
    throw (stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception)
{
    // read all data desired
    return readBytes(aData, nMaxBytesToRead);
}


void SAL_CALL OInputStreamHelper::skipBytes(sal_Int32 nBytesToSkip)
    throw (stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw stario::NotConnectedException(OUString(), static_cast<staruno::XWeak*>(this));

    if (nBytesToSkip < 0)
        throw stario::BufferSizeExceededException(OUString(), static_cast<staruno::XWeak*>(this));

    m_nActPos += nBytesToSkip;
}


sal_Int32 SAL_CALL OInputStreamHelper::available()
    throw (stario::NotConnectedException, stario::IOException, staruno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw stario::NotConnectedException(OUString(), static_cast<staruno::XWeak*>(this));

    return m_nAvailable;
}


void SAL_CALL OInputStreamHelper::closeInput()
    throw (stario::NotConnectedException, stario::IOException, staruno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_xLockBytes.Is())
        throw stario::NotConnectedException(OUString(), static_cast<staruno::XWeak*>(this));

    m_xLockBytes = NULL;
}

} // namespace utl


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
