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

#include <vector>
#include <string_view>

#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

typedef ::std::vector< ::rtl::OString >             StringVector;


// FileStream

class FileStream final
{
public:
    FileStream();
    ~FileStream();

    bool isValid() const;

    void createTempFile(const ::rtl::OString& sPath);
    void close();

    const ::rtl::OString& getName() const { return m_name; }

    bool write(void const * buffer, sal_uInt64 size);

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i);
    friend FileStream &operator<<(FileStream& o, char const * s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OString const * s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OString& s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OStringBuffer const * s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OStringBuffer& s);
    friend FileStream & operator <<(FileStream & out, std::u16string_view s);

private:
    oslFileHandle m_file;
    ::rtl::OString  m_name;
};


// Helper functions

::rtl::OString getTempDir(const ::rtl::OString& sFileName);

::rtl::OString createFileNameFromType(const ::rtl::OString& destination,
                                      const ::rtl::OString& type,
                                      const ::rtl::OString& postfix);

bool fileExists(const ::rtl::OString& fileName);
bool makeValidTypeFile(const ::rtl::OString& targetFileName,
                           const ::rtl::OString& tmpFileName,
                           bool bFileCheck);
bool removeTypeFile(const ::rtl::OString& fileName);

::rtl::OUString convertToFileUrl(const ::rtl::OString& fileName);

class CannotDumpException final {
public:
    CannotDumpException(OUString const & message): message_(message) {}

    ~CannotDumpException() noexcept;

    const OUString& getMessage() const { return message_; }

private:
    OUString message_;
};

#endif // INCLUDED_CODEMAKER_GLOBAL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
