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

#ifndef ZIPFILE_HXX_INCLUDED
#define ZIPFILE_HXX_INCLUDED

#ifndef _WINDOWS
#define _WINDOWS
#endif

#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif

#include <string>
#include <vector>
#include <memory>

class StreamInterface;

/** A simple zip content provider based on the zlib
*/

class ZipFile
{
public:

    typedef std::vector<std::string>   Directory_t;
    typedef std::auto_ptr<Directory_t> DirectoryPtr_t;
    typedef std::vector<char>          ZipContentBuffer_t;

public:

    /** Checks whether a file is a zip file or not

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user

    @returns    true if the file is a zip file
            false if the file is not a zip file

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist
            AccessViolationException if read access to the file is denied
    */
    static bool IsZipFile(const std::string &FileName);

    static bool IsZipFile(void *stream);


    /** Returns wheter the version of the specified zip file may be uncompressed with the
          currently used zlib version or not

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user
            The file must be a valid zip file

    @returns    true if the file may be uncompressed with the currently used zlib
            false if the file may not be uncompressed with the currently used zlib

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist or is no zip file
            AccessViolationException if read access to the file is denied
    */
    static bool IsValidZipFileVersionNumber(const std::string &FileName);

    static bool IsValidZipFileVersionNumber(void *stream);

public:

    /** Constructs a zip file from a zip file

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist or is no valid zip file
            AccessViolationException if read access to the file is denied
            WrongZipVersionException if the zip file cannot be uncompressed
            with the used zlib version
    */
    ZipFile(const std::string &FileName);

    ZipFile(StreamInterface *stream);


    /** Destroys a zip file
    */
    ~ZipFile();

    /** Provides an interface to read the uncompressed data of a content of the zip file

    @param      ContentName
                The name of the content in the zip file

    @param      ppstm
                Pointer to pointer, will receive an interface pointer
                to IUnknown on success

    @precond    The specified content must exist in this file
                ppstm must not be NULL

    @throws     std::bad_alloc if the necessary buffer could not be
                allocated
                ZipException if an zip error occurs
                ZipContentMissException if the specified zip content
                does not exist in this zip file
    */
    void GetUncompressedContent(const std::string &ContentName, /*inout*/ ZipContentBuffer_t &ContentBuffer);

    /** Returns a list with the content names contained within this file

        @throws ZipException if an error in the zlib happens
    */
    DirectoryPtr_t GetDirectory() const;

    /** Convinience query function may even realized with
        iterating over a ZipFileDirectory returned by
        GetDirectory
    */
    bool HasContent(const std::string &ContentName) const;

private:

    /** Returns the length of the longest file name
        in the current zip file

        @throws ZipException if an zip error occurs
    */
    long GetFileLongestFileNameLength() const;

private:
    StreamInterface *m_pStream;
    bool m_bShouldFree;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
