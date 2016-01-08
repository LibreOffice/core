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

// ZipException.h: interface for the ZipException class.

#ifndef INCLUDED_SHELL_SOURCE_WIN32_ZIPFILE_ZIPEXCPTN_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_ZIPFILE_ZIPEXCPTN_HXX

#include <stdexcept>


/**
*/
class RuntimeException : public std::exception
{
public:
    explicit RuntimeException(int Error);
    virtual ~RuntimeException() throw();

    int GetErrorCode() const;

private:
    int m_Error;
};


/**
*/
class ZipException : public RuntimeException
{
public:
    explicit ZipException(int Error);

    virtual const char* what() const throw();
};


/**
*/
class Win32Exception : public RuntimeException
{
public:
    explicit Win32Exception(int Error);
    virtual ~Win32Exception() throw();

    virtual const char* what() const throw();

private:
    void* m_MsgBuff;
};


/**
*/
class ZipContentMissException : public ZipException
{
public:
    explicit ZipContentMissException(int Error);
};


/**
*/
class AccessViolationException : public Win32Exception
{
public:
    explicit AccessViolationException(int Error);
};


/**
*/
class IOException : public Win32Exception
{
public:
    explicit IOException(int Error);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
