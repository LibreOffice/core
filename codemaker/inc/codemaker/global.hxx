/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 == str2);
    }
};

struct HashString
{
    size_t operator()(const ::rtl::OString& str) const
    {
        return str.hashCode();
    }
};

struct LessString
{
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 < str2);
    }
};

#if defined(_MSC_VER) &&  _MSC_VER < 1200
typedef ::std::new_alloc NewAlloc;
#endif


typedef ::std::list< ::rtl::OString >               StringList;
typedef ::std::vector< ::rtl::OString >             StringVector;
typedef ::std::set< ::rtl::OString, LessString >    StringSet;

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
    FileStream(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    virtual ~FileStream();

    sal_Bool isValid();

    void open(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    void createTempFile(const ::rtl::OString& sPath);
    void close();

    ::rtl::OString  getName() { return m_name; }

    bool write(void const * buffer, sal_uInt64 size);

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i);
    friend FileStream &operator<<(FileStream& o, char const * s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OString* s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OString& s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OStringBuffer* s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OStringBuffer& s);

private:
    sal_uInt32 checkAccessMode(FileAccessMode mode);

    oslFileHandle m_file;
    ::rtl::OString  m_name;
};


//*************************************************************************
// Helper functions
//*************************************************************************
::rtl::OString getTempDir(const ::rtl::OString& sFileName);

::rtl::OString createFileNameFromType(const ::rtl::OString& destination,
                                      const ::rtl::OString type,
                                      const ::rtl::OString postfix,
                                      sal_Bool bLowerCase=sal_False,
                                      const ::rtl::OString prefix="");

sal_Bool fileExists(const ::rtl::OString& fileName);
sal_Bool makeValidTypeFile(const ::rtl::OString& targetFileName,
                           const ::rtl::OString& tmpFileName,
                           sal_Bool bFileCheck);
sal_Bool removeTypeFile(const ::rtl::OString& fileName);

::rtl::OUString convertToFileUrl(const ::rtl::OString& fileName);

//*************************************************************************
// Global exception to signal problems when a type cannot be dumped
//*************************************************************************
class CannotDumpException
{
public:
    CannotDumpException(const ::rtl::OString& msg)
        : m_message(msg) {}

    ::rtl::OString  m_message;
};


#endif // INCLUDED_CODEMAKER_GLOBAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
