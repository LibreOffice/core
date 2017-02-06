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

#include <sal/config.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <comphelper/oslfile2streamwrap.hxx>

#include <algorithm>

namespace comphelper
{
    using namespace osl;


OSLInputStreamWrapper::OSLInputStreamWrapper( File& _rFile )
    : m_pFile(&_rFile)
{
}


OSLInputStreamWrapper::~OSLInputStreamWrapper()
{
}


sal_Int32 SAL_CALL OSLInputStreamWrapper::readBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
{
    if (!m_pFile)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt64 nRead = 0;
    FileBase::RC eError = m_pFile->read(static_cast<void*>(aData.getArray()), nBytesToRead, nRead);
    if (eError != FileBase::E_None)
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));

    // Wenn gelesene Zeichen < MaxLength, css::uno::Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( sal::static_int_cast< sal_Int32 >(nRead) );

    return sal::static_int_cast< sal_Int32 >(nRead);
}


sal_Int32 SAL_CALL OSLInputStreamWrapper::readSomeBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
{
    if (!m_pFile)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    if (nMaxBytesToRead < 0)
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));

    return readBytes(aData, nMaxBytesToRead);
}


void SAL_CALL OSLInputStreamWrapper::skipBytes(sal_Int32 nBytesToSkip)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    sal_uInt64 nCurrentPos;
    m_pFile->getPos(nCurrentPos);

    sal_uInt64 nNewPos = nCurrentPos + nBytesToSkip;
    FileBase::RC eError = m_pFile->setPos(osl_Pos_Absolut, nNewPos);
    if (eError != FileBase::E_None)
    {
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));
    }
}


sal_Int32 SAL_CALL OSLInputStreamWrapper::available()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    sal_uInt64 nPos;
    FileBase::RC eError = m_pFile->getPos(nPos);
    if (eError != FileBase::E_None)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    sal_uInt64 nDummy = 0;
    eError = m_pFile->setPos(osl_Pos_End, nDummy);
    if (eError != FileBase::E_None)
       throw css::io::NotConnectedException(OUString(),static_cast<css::uno::XWeak*>(this));

    sal_uInt64 nAvailable;
    eError = m_pFile->getPos(nAvailable);
    if (eError != FileBase::E_None)
       throw css::io::NotConnectedException(OUString(),static_cast<css::uno::XWeak*>(this));

    nAvailable = nAvailable - nPos;
    eError = m_pFile->setPos(osl_Pos_Absolut, nPos);
    if (eError != FileBase::E_None)
       throw css::io::NotConnectedException(OUString(),static_cast<css::uno::XWeak*>(this));
    return sal::static_int_cast< sal_Int32 >(
        std::max(nAvailable, sal::static_int_cast< sal_uInt64 >(SAL_MAX_INT32)));
}


void SAL_CALL OSLInputStreamWrapper::closeInput()
{
    if (!m_pFile)
        throw css::io::NotConnectedException(OUString(), static_cast<css::uno::XWeak*>(this));

    m_pFile->close();

    m_pFile = nullptr;
}

/*************************************************************************/
// css::io::XOutputStream


OSLOutputStreamWrapper::OSLOutputStreamWrapper(osl::File & _rFile):
    rFile(_rFile)
{}

OSLOutputStreamWrapper::~OSLOutputStreamWrapper() {}

void SAL_CALL OSLOutputStreamWrapper::writeBytes(const css::uno::Sequence< sal_Int8 >& aData)
{
    sal_uInt64 nWritten;
    FileBase::RC eError = rFile.write(aData.getConstArray(),aData.getLength(), nWritten);
    if (eError != FileBase::E_None
        || nWritten != sal::static_int_cast< sal_uInt32 >(aData.getLength()))
    {
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));
    }
}


void SAL_CALL OSLOutputStreamWrapper::flush()
{
}


void SAL_CALL OSLOutputStreamWrapper::closeOutput()
{
    rFile.close();
}

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
