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

#include "internal/global.hxx"
#include "zipexcptn.hxx"

//------------------------------------------
/**
*/
RuntimeException::RuntimeException(int Error) :
    m_Error(Error)
{
}

//------------------------------------------
/**
*/
RuntimeException::~RuntimeException() throw()
{
}

//------------------------------------------
/**
*/
int RuntimeException::GetErrorCode() const
{
    return m_Error;
}

//------------------------------------------
/**
*/
ZipException::ZipException(int Error) :
    RuntimeException(Error)
{
}

//------------------------------------------
/**
*/
const char* ZipException::what() const throw()
{
    return 0;
}

//------------------------------------------
/**
*/
Win32Exception::Win32Exception(int Error) :
    RuntimeException(Error),
    m_MsgBuff(0)
{
}

//------------------------------------------
/**
*/
Win32Exception::~Win32Exception() throw()
{
    if (m_MsgBuff)
        LocalFree(m_MsgBuff);
}

//------------------------------------------
/**
*/
const char* Win32Exception::what() const throw()
{
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetErrorCode(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &m_MsgBuff,
        0,
        NULL);

    return reinterpret_cast<char*>(m_MsgBuff);
}

//------------------------------------------
/**
*/
ZipContentMissException::ZipContentMissException(int Error) :
    ZipException(Error)
{
}

//------------------------------------------
/**
*/
AccessViolationException::AccessViolationException(int Error) :
    Win32Exception(Error)
{
}

//------------------------------------------
/**
*/
IOException::IOException(int Error) :
    Win32Exception(Error)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
