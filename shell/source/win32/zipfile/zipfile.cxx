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

#include "zipexcptn.hxx"
#include "internal/zipfile.hxx"
#include "internal/global.hxx"
#include "internal/types.hxx"
#include "internal/stream_helper.hxx"

#include <malloc.h>
#include <algorithm>
#include <functional>

#include <string.h>

namespace
{

struct LocalFileHeader
{
    unsigned short min_version;
    unsigned short general_flag;
    unsigned short compression;
    unsigned short lastmod_time;
    unsigned short lastmod_date;
    unsigned crc32;
    unsigned compressed_size;
    unsigned uncompressed_size;
    unsigned short filename_size;
    unsigned short extra_field_size;
    std::string filename;
    std::string extra_field;
    LocalFileHeader()
        : min_version(0), general_flag(0), compression(0), lastmod_time(0), lastmod_date(0),
          crc32(0), compressed_size(0), uncompressed_size(0), filename_size(0), extra_field_size(0),
          filename(), extra_field() {}
    ~LocalFileHeader() {}
};

struct CentralDirectoryEntry
{
    unsigned short creator_version;
    unsigned short min_version;
    unsigned short general_flag;
    unsigned short compression;
    unsigned short lastmod_time;
    unsigned short lastmod_date;
    unsigned crc32;
    unsigned compressed_size;
    unsigned uncompressed_size;
    unsigned short filename_size;
    unsigned short extra_field_size;
    unsigned short file_comment_size;
    unsigned short disk_num;
    unsigned short internal_attr;
    unsigned external_attr;
    unsigned offset;
    std::string filename;
    std::string extra_field;
    std::string file_comment;
    CentralDirectoryEntry()
        : creator_version(0), min_version(0), general_flag(0), compression(0), lastmod_time(0),
          lastmod_date(0), crc32(0), compressed_size(0), uncompressed_size(0), filename_size(0),
          extra_field_size(0), file_comment_size(0), disk_num(0), internal_attr(0),
          external_attr(0), offset(0), filename(), extra_field(), file_comment() {}
    ~CentralDirectoryEntry() {}
};

struct CentralDirectoryEnd
{
    unsigned short disk_num;
    unsigned short cdir_disk;
    unsigned short disk_entries;
    unsigned short cdir_entries;
    unsigned cdir_size;
    unsigned cdir_offset;
    unsigned short comment_size;
    std::string comment;
    CentralDirectoryEnd()
        : disk_num(0), cdir_disk(0), disk_entries(0), cdir_entries(0),
          cdir_size(0), cdir_offset(0), comment_size(0), comment() {}
    ~CentralDirectoryEnd() {}
};

#define CDIR_ENTRY_SIG 0x02014b50
#define LOC_FILE_HEADER_SIG 0x04034b50
#define CDIR_END_SIG 0x06054b50

static unsigned char readByte(StreamInterface *stream)
{
    if (!stream || stream->stell() == -1)
        throw IOException(-1);
    unsigned char tmpBuf;
    unsigned long numBytesRead = stream->sread(&tmpBuf, 1);
    if (numBytesRead != 1)
        throw IOException(-1);
    return tmpBuf;
}

static unsigned short readShort(StreamInterface *stream)
{
    unsigned short p0 = (unsigned short)readByte(stream);
    unsigned short p1 = (unsigned short)readByte(stream);
    return (unsigned short)(p0|(p1<<8));
}

static unsigned readInt(StreamInterface *stream)
{
    unsigned p0 = (unsigned)readByte(stream);
    unsigned p1 = (unsigned)readByte(stream);
    unsigned p2 = (unsigned)readByte(stream);
    unsigned p3 = (unsigned)readByte(stream);
    return (unsigned)(p0|(p1<<8)|(p2<<16)|(p3<<24));
}

static bool readCentralDirectoryEnd(StreamInterface *stream, CentralDirectoryEnd &end)
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
        end.comment_size = readShort(stream);
        end.comment.clear();
        for (unsigned short i = 0; i < end.comment_size; i++)
            end.comment.append(1,(char)readByte(stream));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

static bool readCentralDirectoryEntry(StreamInterface *stream, CentralDirectoryEntry &entry)
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
        entry.filename_size = readShort(stream);
        entry.extra_field_size = readShort(stream);
        entry.file_comment_size = readShort(stream);
        entry.disk_num = readShort(stream);
        entry.internal_attr = readShort(stream);
        entry.external_attr = readInt(stream);
        entry.offset = readInt(stream);
        unsigned short i = 0;
        entry.filename.clear();
        for (i=0; i < entry.filename_size; i++)
            entry.filename.append(1,(char)readByte(stream));
        entry.extra_field.clear();
        for (i=0; i < entry.extra_field_size; i++)
            entry.extra_field.append(1,(char)readByte(stream));
        entry.file_comment.clear();
        for (i=0; i < entry.file_comment_size; i++)
            entry.file_comment.append(1,(char)readByte(stream));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

static bool readLocalFileHeader(StreamInterface *stream, LocalFileHeader &header)
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
        header.filename_size = readShort(stream);
        header.extra_field_size = readShort(stream);
        unsigned short i = 0;
        header.filename.clear();
        for (i=0; i < header.filename_size; i++)
            header.filename.append(1,(char)readByte(stream));
        header.extra_field.clear();
        for (i=0; i < header.extra_field_size; i++)
            header.extra_field.append(1,(char)readByte(stream));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

static bool areHeadersConsistent(const LocalFileHeader &header, const CentralDirectoryEntry &entry)
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

static bool findCentralDirectoryEnd(StreamInterface *stream)
{
    if (!stream)
        return false;
    stream->sseek(0, SEEK_SET);
    if (stream->sseek(-1024, SEEK_END)) stream->sseek(0, SEEK_SET);
    try
    {
        while (stream->stell() != -1)
        {
            unsigned signature = readInt(stream);
            if (signature == CDIR_END_SIG)
            {
                stream->sseek(-4, SEEK_CUR);
                return true;
            }
            else
                stream->sseek(-3, SEEK_CUR);
        }
    }
    catch (...)
    {
        return false;
    }
    return false;
}

static bool isZipStream(StreamInterface *stream)
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

} // anonymous namespace

namespace internal
{
/* for case in-sensitive string comparison */
struct stricmp : public std::unary_function<std::string, bool>
{
    stricmp(const std::string &str) : str_(str)
    {}

    bool operator() (const std::string &other)
    {
        return (0 == _stricmp(str_.c_str(), other.c_str()));
    }

    std::string str_;
};
} // namespace internal

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
bool ZipFile::IsZipFile(const std::string& /*FileName*/)
{
    return true;
}

bool ZipFile::IsZipFile(void* /*stream*/)
{
    return true;
}


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
bool ZipFile::IsValidZipFileVersionNumber(const std::string& /*FileName*/)
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
ZipFile::ZipFile(const std::string &FileName) :
    m_pStream(0),
    m_bShouldFree(true)
{
    m_pStream = new FileStream(FileName.c_str());
    if (m_pStream && !isZipStream(m_pStream))
    {
        delete m_pStream;
        m_pStream = 0;
    }
}

ZipFile::ZipFile(StreamInterface *stream) :
    m_pStream(stream),
    m_bShouldFree(false)
{
    if (!isZipStream(stream))
        m_pStream = 0;
}


/** Destroys a zip file
*/
ZipFile::~ZipFile()
{
    if (m_pStream && m_bShouldFree)
        delete m_pStream;
}

/** Provides an interface to read the uncompressed data of a content of the zip file

    @precond    The specified content must exist in this file
            ppstm must not be NULL
*/
void ZipFile::GetUncompressedContent(
    const std::string &ContentName, /*inout*/ ZipContentBuffer_t &ContentBuffer)
{
    if (!findCentralDirectoryEnd(m_pStream))
        return;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(m_pStream, end))
        return;
    m_pStream->sseek(end.cdir_offset, SEEK_SET);
    CentralDirectoryEntry entry;
    while (m_pStream->stell() != -1 && (unsigned long)m_pStream->stell() < end.cdir_offset + end.cdir_size)
    {
        if (!readCentralDirectoryEntry(m_pStream, entry))
            return;
        if (ContentName.length() == entry.filename_size && !_stricmp(entry.filename.c_str(), ContentName.c_str()))
            break;
    }
    if (ContentName.length() != entry.filename_size)
        return;
    if (_stricmp(entry.filename.c_str(), ContentName.c_str()))
        return;
    m_pStream->sseek(entry.offset, SEEK_SET);
    LocalFileHeader header;
    if (!readLocalFileHeader(m_pStream, header))
        return;
    if (!areHeadersConsistent(header, entry))
        return;
    ContentBuffer.clear();
    ContentBuffer = ZipContentBuffer_t(entry.uncompressed_size);
    if (!entry.compression)
        m_pStream->sread((unsigned char *)&ContentBuffer[0], entry.uncompressed_size);
    else
    {
        int ret;
        z_stream strm;

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit2(&strm,-MAX_WBITS);
        if (ret != Z_OK)
            return;

        std::vector<unsigned char> tmpBuffer(entry.compressed_size);
        if (entry.compressed_size != m_pStream->sread(&tmpBuffer[0], entry.compressed_size))
            return;

        strm.avail_in = entry.compressed_size;
        strm.next_in = reinterpret_cast<Bytef *>(&tmpBuffer[0]);

        strm.avail_out = entry.uncompressed_size;
        strm.next_out = reinterpret_cast<Bytef *>(&ContentBuffer[0]);
        ret = inflate(&strm, Z_FINISH);
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            ContentBuffer.clear();
            return;
        }
        (void)inflateEnd(&strm);
    }
}

/** Returns a list with the content names contained within this file

*/
ZipFile::DirectoryPtr_t ZipFile::GetDirectory() const
{
    DirectoryPtr_t dir(new Directory_t());
    if (!findCentralDirectoryEnd(m_pStream))
        return dir;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(m_pStream, end))
        return dir;
    m_pStream->sseek(end.cdir_offset, SEEK_SET);
    CentralDirectoryEntry entry;
    while (m_pStream->stell() != -1 && (unsigned long)m_pStream->stell() < end.cdir_offset + end.cdir_size)
    {
        if (!readCentralDirectoryEntry(m_pStream, entry))
            return dir;
        if (entry.filename_size)
            dir->push_back(entry.filename);
    }
    return dir;
}

/** Convinience query function may even realized with
    iterating over a ZipFileDirectory returned by
    GetDirectory */
bool ZipFile::HasContent(const std::string &ContentName) const
{
    //#i34314# we need to compare package content names
    //case in-sensitive as it is not defined that such
    //names must be handled case sensitive
    DirectoryPtr_t dir = GetDirectory();
    Directory_t::iterator iter =
        std::find_if(dir->begin(), dir->end(), internal::stricmp(ContentName));

    return (iter != dir->end());
}


/** Returns the length of the longest file name
        in the current zip file
*/
long ZipFile::GetFileLongestFileNameLength() const
{
    long lmax = 0;
    if (!findCentralDirectoryEnd(m_pStream))
        return lmax;
    CentralDirectoryEnd end;
    if (!readCentralDirectoryEnd(m_pStream, end))
        return lmax;
    m_pStream->sseek(end.cdir_offset, SEEK_SET);
    CentralDirectoryEntry entry;
    while (m_pStream->stell() != -1 && (unsigned long)m_pStream->stell() < end.cdir_offset + end.cdir_size)
    {
        if (!readCentralDirectoryEntry(m_pStream, entry))
            return lmax;
        if (entry.filename_size > lmax)
            lmax = entry.filename_size;
    }
    return lmax;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
