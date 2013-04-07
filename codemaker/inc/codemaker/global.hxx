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

#ifndef INCLUDED_CODEMAKER_GLOBAL_HXX
#define INCLUDED_CODEMAKER_GLOBAL_HXX

#include <list>
#include <vector>
#include <set>

#include <stdio.h>

#include "osl/file.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

struct EqualString
{
    sal_Bool operator()(const OString& str1, const OString& str2) const
    {
        return (str1 == str2);
    }
};

struct HashString
{
    size_t operator()(const OString& str) const
    {
        return str.hashCode();
    }
};

struct LessString
{
    sal_Bool operator()(const OString& str1, const OString& str2) const
    {
        return (str1 < str2);
    }
};

typedef ::std::list< OString >               StringList;
typedef ::std::vector< OString >             StringVector;
typedef ::std::set< OString, LessString >    StringSet;

//*************************************************************************
// FileStream
//*************************************************************************
enum FileAccessMode
{
    FAM_READ,                   // "r"
    FAM_WRITE,                  // "w"
    FAM_READWRITE_EXIST,        // "r+"
    FAM_READWRITE               // "w+"
};

class FileStream
{
public:
    FileStream();
    virtual ~FileStream();

    sal_Bool isValid();

    void createTempFile(const OString& sPath);
    void close();

    OString  getName() { return m_name; }

    bool write(void const * buffer, sal_uInt64 size);

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i);
    friend FileStream &operator<<(FileStream& o, char const * s);
    friend FileStream &operator<<(FileStream& o, OString* s);
    friend FileStream &operator<<(FileStream& o, const OString& s);
    friend FileStream &operator<<(FileStream& o, OStringBuffer* s);
    friend FileStream &operator<<(FileStream& o, const OStringBuffer& s);

private:
    oslFileHandle m_file;
    OString  m_name;
};


//*************************************************************************
// Helper functions
//*************************************************************************
OString getTempDir(const OString& sFileName);

OString createFileNameFromType(const OString& destination,
                                      const OString type,
                                      const OString postfix,
                                      sal_Bool bLowerCase=sal_False,
                                      const OString prefix="");

sal_Bool fileExists(const OString& fileName);
sal_Bool makeValidTypeFile(const OString& targetFileName,
                           const OString& tmpFileName,
                           sal_Bool bFileCheck);
sal_Bool removeTypeFile(const OString& fileName);

OUString convertToFileUrl(const OString& fileName);

//*************************************************************************
// Global exception to signal problems when a type cannot be dumped
//*************************************************************************
class CannotDumpException
{
public:
    CannotDumpException(const OString& msg)
        : m_message(msg) {}

    OString  m_message;
};


#endif // INCLUDED_CODEMAKER_GLOBAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
