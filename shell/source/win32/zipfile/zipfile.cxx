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

#include "zipexcptn.hxx"
#include <zipfile.hxx>
#include <global.hxx>
#include <types.hxx>
#include <stream_helper.hxx>

#include <malloc.h>
#include <algorithm>
#include <memory>

#include <string.h>

#include <o3tl/safeint.hxx>

#include <zlib.h>

namespace
{

struct LocalFileHeader
{
    unsigned short min_version = 0;
    unsigned short general_flag = 0;
    unsigned short compression = 0;
    unsigned short lastmod_time = 0;
    unsigned short lastmod_date = 0;
    unsigned crc32 = 0;
    unsigned compressed_size = 0;
    unsigned uncompressed_size = 0;
    std::string filename;
    std::string extra_field;
};

struct CentralDirectoryEntry
{
    unsigned short creator_version = 0;
    unsigned short min_version = 0;
    unsigned short general_flag = 0;
    unsigned short compression = 0;
    unsigned short lastmod_time = 0;
    unsigned short lastmod_date = 0;
    unsigned crc32 = 0;
    unsigned compressed_size = 0;
    unsigned uncompressed_size = 0;
    unsigned short disk_num = 0;
    unsigned short internal_attr = 0;
    unsigned external_attr = 0;
    unsigned offset = 0;
    std::string filename;
    std::string extra_field;
    std::string file_comment;
};

struct CentralDirectoryEnd
{
    unsigned short disk_num = 0;
    unsigned short cdir_disk = 0;
    unsigned short disk_entries = 0;
    unsigned short cdir_entries = 0;
    unsigned cdir_size = 0;
    unsigned cdir_offset = 0;
    std::string comment;
};

#define CDIR_ENTRY_SIG 0x02014b50
#define LOC_FILE_HEADER_SIG 0x04034b50
#define CDIR_END_SIG 0x06054b50

// This little lot performs in a truly appalling way without
// buffering eg. on an IStream.

unsigned short readShort(StreamInterface *stream)
{
    if (!stream || stream->stell() == -1)
        throw IOException(-1);
    unsigned short tmpBuf;
    unsigned long numBytesRead = stream->sread(
        reinterpret_cast<unsigned char *>( &tmpBuf ), 2);
    if (numBytesRead != 2)
        throw IOException(-1);
    return tmpBuf;
}

unsigned readInt(StreamInterface *stream)
{
    if (!stream || stream->stell() == -1)
        throw IOException(-1);
    unsigned tmpBuf;
    unsigned long numBytesRead = stream->sread(
        reinterpret_cast<unsigned char *>( &tmpBuf ), 4);
    if (numBytesRead != 4)
        throw IOException(-1);
    return tmpBuf;
}

std::string readString(StreamInterface *stream, unsigned long size)
{
    if (!stream || stream->stell() == -1)
        throw IOException(-1);
    auto tmp = std::make_unique<unsigned char[]>(size);
    unsigned long numBytesRead = stream->sread(tmp.get(), size);
    if (numBytesRead != size)
    {
        throw IOException(-1);
    }

    std::string aStr(reinterpret_cast<char *>(tmp.get()), size);
    return aStr;
}

bool readCentralDirectoryEnd(StreamInterface *stream, CentralDirectoryEnd &end)
{
    try
    {
        unsigned signature = readInt(stream);
        if (signature != CDIR_END_SIG)
            return false;

        end.disk_num = readShort(stream);
        end.cdir_disk = readShort(stream);
        end.disk_entries = readShort(stream);
        end.cdir_entries = readShort(stream);
        end.cdir_size = readInt(stream);
        end.cdir_offset = readInt(stream);
        unsigned short comment_size = readShort(stream);
        end.comment.assign(readString(stream, comment_size));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool readCentralDirectoryEntry(StreamInterface *stream, CentralDirectoryEntry &entry)
{
    try
    {
        unsigned signature = readInt(stream);
        if (signature != CDIR_ENTRY_SIG)
            return false;

        entry.creator_version = readShort(stream);
        entry.min_version = readShort(stream);
        entry.general_flag = readShort(stream);
        entry.compression = readShort(stream);
        entry.lastmod_time = readShort(stream);
        entry.lastmod_date = readShort(stream);
        entry.crc32 = readInt(stream);
        entry.compressed_size = readInt(stream);
        entry.uncompressed_size = readInt(stream);
        unsigned short filename_size = readShort(stream);
        unsigned short extra_field_size = readShort(stream);
        unsigned short file_comment_size = readShort(stream);
        entry.disk_num = readShort(stream);
        entry.internal_attr = readShort(stream);
        entry.external_attr = readInt(stream);
        entry.offset = readInt(stream);
        entry.filename.assign(readString(stream, filename_size));
        entry.extra_field.assign(readString(stream, extra_field_size));
        entry.file_comment.assign(readString(stream, file_comment_size));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool readLocalFileHeader(StreamInterface *stream, LocalFileHeader &header)
{
    try
    {
        unsigned signature = readInt(stream);
        if (signature != LOC_FILE_HEADER_SIG)
            return false;

        header.min_version = readShort(stream);
        header.general_flag = readShort(stream);
        header.compression = readShort(stream);
        header.lastmod_time = readShort(stream);
        header.lastmod_date = readShort(stream);
        header.crc32 = readInt(stream);
        header.compressed_size = readInt(stream);
        header.uncompressed_size = readInt(stream);
        unsigned short filename_size = readShort(stream);
        unsigned short extra_field_size = readShort(stream);
        header.filename.assign(readString(stream, filename_size));
        header.extra_field.assign(readString(stream, extra_field_size));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool areHeadersConsistent(const LocalFileHeader &header, const CentralDirectoryEntry &entry)
{
    if (header.min_version != entry.min_version)
        return false;
    if (header.general_flag != entry.general_flag)
        return false;
    if (header.compression != entry.compression)
        return false;
    if (!(header.general_flag & 0x08))
    {
        if (header.crc32 != entry.crc32)
            return false;
        if (header.compressed_size != entry.compressed_size)
            return false;
        if (header.uncompressed_size != entry.uncompressed_size)
            return false;
    }
    return true;
}

#define BLOCK_SIZE 0x800

bool findSignatureAtOffset(StreamInterface *stream, unsigned long nOffset)
{
    // read in reasonably sized chunk, and read more, to get overlapping sigs
    unsigned char aBuffer[ BLOCK_SIZE + 4 ];

    stream->sseek(nOffset, SEEK_SET);

    unsigned long nBytesRead = stream->sread(aBuffer, sizeof(aBuffer));

    for (long n = nBytesRead - 4; n >= 0; n--)
    {
        if (aBuffer[n  ] == 0x50 && aBuffer[n+1] == 0x4b &&
            aBuffer[n+2] == 0x05 && aBuffer[n+3] == 0x06)
        { // a palpable hit ...
            stream->sseek(nOffset + n, SEEK_SET);
            return true;
        }
    }

    return false;
}

bool findCentralDirectoryEnd(StreamInterface *stream)
{
    if (!stream)
        return false;

    stream->sseek(0,SEEK_END);

    long nLength = stream->stell();
    if (nLength == -1)
        return false;

    try
    {
        for (long nOffset = nLength - BLOCK_SIZE - 4;
             nOffset > 0; nOffset -= BLOCK_SIZE)
        {
            if (findSignatureAtOffset(stream, nOffset))
                return true;
        }
        return findSignatureAtOffset(stream, 0);
    }
    catch (...)
    {
        return false;
    }
}

bool isZipStream(StreamInterface *stream)
{
    if (!findCentralDirectoryEnd(stream))
        return false;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(stream, end))
        return false;
    stream->sseek(end.cdir_offset, SEEK_SET);
    CentralDirectoryEntry entry;
    if (!readCentralDirectoryEntry(stream, entry))
        return false;
    stream->sseek(entry.offset, SEEK_SET);
    LocalFileHeader header;
    if (!readLocalFileHeader(stream, header))
        return false;
    if (!areHeadersConsistent(header, entry))
        return false;
    return true;
}

// Iteration ends when Func returns true
template <typename Func>
    requires std::is_invocable_r_v<bool, Func, const CentralDirectoryEntry&>
bool IterateEntries(StreamInterface* stream, Func func)
{
    if (!findCentralDirectoryEnd(stream))
        return false;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(stream, end))
        return false;
    stream->sseek(end.cdir_offset, SEEK_SET);
    CentralDirectoryEntry entry;
    while (stream->stell() != -1
           && o3tl::make_unsigned(stream->stell()) < end.cdir_offset + end.cdir_size)
    {
        if (!readCentralDirectoryEntry(stream, entry))
            return false;
        if (func(entry))
            return true;
    }
    return false;
}

bool FindEntry(StreamInterface* stream, const std::string& name,
               /* out, opt */ CentralDirectoryEntry* entry = nullptr)
{
    return IterateEntries(
        stream,
        [&name, entry](const CentralDirectoryEntry& candidate)
        {
            //#i34314# we need to compare package content names
            //case in-sensitive as it is not defined that such
            //names must be handled case sensitive
            if (name.length() == candidate.filename.length()
                && _strnicmp(name.c_str(), candidate.filename.c_str(), name.length()) == 0)
            {
                if (entry)
                    *entry = candidate;
                return true;
            }
            return false;
        });
}

} // anonymous namespace

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
bool ZipFile::IsZipFile(const Filepath_t& /*FileName*/)
{
    return true;
}

bool ZipFile::IsZipFile(void* /*stream*/)
{
    return true;
}


/** Returns whether the version of the specified zip file may be uncompressed with the
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
bool ZipFile::IsValidZipFileVersionNumber(const Filepath_t& /*FileName*/)
{
    return true;
}

bool ZipFile::IsValidZipFileVersionNumber(void* /* stream*/)
{
    return true;
}


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
ZipFile::ZipFile(const Filepath_t &FileName) :
    m_pStream(nullptr),
    m_bShouldFree(true)
{
    m_pStream = new FileStream(FileName.c_str());
    if (!isZipStream(m_pStream))
    {
        delete m_pStream;
        m_pStream = nullptr;
    }
}

ZipFile::ZipFile(StreamInterface *stream) :
    m_pStream(stream),
    m_bShouldFree(false)
{
    if (!isZipStream(stream))
        m_pStream = nullptr;
}


/** Destroys a zip file
*/
ZipFile::~ZipFile()
{
    if (m_pStream && m_bShouldFree)
        delete m_pStream;
}

/** Provides an interface to read the uncompressed data of a content of the zip file
*/
void ZipFile::GetUncompressedContent(
    const std::string &ContentName, /*inout*/ ZipContentBuffer_t &ContentBuffer)
{
    CentralDirectoryEntry entry;
    if (!FindEntry(m_pStream, ContentName, &entry))
        return;
    m_pStream->sseek(entry.offset, SEEK_SET);
    LocalFileHeader header;
    if (!readLocalFileHeader(m_pStream, header))
        return;
    if (!areHeadersConsistent(header, entry))
        return;
    ContentBuffer.clear();
    if (!entry.compression)
    {
        ContentBuffer.resize(entry.uncompressed_size);
        m_pStream->sread(reinterpret_cast<unsigned char *>(ContentBuffer.data()), entry.uncompressed_size);
    }
    else
    {
        z_stream strm{
            .next_in = Z_NULL, .avail_in = 0, .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL
        };

        /* allocate inflate state */
        int ret = inflateInit2(&strm,-MAX_WBITS);
        if (ret != Z_OK)
            return;

        std::vector<unsigned char> tmpBuffer(entry.compressed_size);
        if (entry.compressed_size != m_pStream->sread(tmpBuffer.data(), entry.compressed_size))
            return;

        strm.avail_in = entry.compressed_size;
        strm.next_in = reinterpret_cast<Bytef *>(tmpBuffer.data());

        ContentBuffer.resize(entry.uncompressed_size);
        strm.avail_out = entry.uncompressed_size;
        strm.next_out = reinterpret_cast<Bytef *>(ContentBuffer.data());
        ret = inflate(&strm, Z_FINISH);
        (void)inflateEnd(&strm);
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            ContentBuffer.clear();
            return;
        }
    }
}

/** Returns a list with the content names contained within this file

*/
std::vector<std::string> ZipFile::GetDirectory() const
{
    std::vector<std::string> dir;
    IterateEntries(m_pStream,
                   [&dir](const CentralDirectoryEntry& entry)
                   {
                       if (!entry.filename.empty())
                           dir.push_back(entry.filename);
                       return false;
                   });
    return dir;
}

/** Convenience query function */
bool ZipFile::HasContent(const std::string &ContentName) const
{
    return FindEntry(m_pStream, ContentName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
