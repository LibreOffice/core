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

#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#   include <io.h>
#   include <direct.h>
#endif

#ifdef UNX
#   include <sys/stat.h>
#   include <unistd.h>
#endif

#include <codemaker/global.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::osl;


OString getTempDir(const OString& sFileName)
{
    sal_Int32 index = 0;
#ifdef SAL_UNX
    if ((index=sFileName.lastIndexOf('/')) > 0)
        return sFileName.copy(0, index);
#else
    if ((index=sFileName.lastIndexOf('\\')) > 0)
        return sFileName.copy(0, index);
#endif
    return ".";
}

OString createFileNameFromType( const OString& destination,
                                const OString& typeName,
                                const OString& postfix )
{
    OString type(typeName.replace('.', '/'));

    sal_uInt32 length = destination.getLength();

    bool bWithPoint = false;
    if (length == 0)
    {
        length++;
        bWithPoint = true;
    }

    length += type.getLength() + postfix.getLength();

    bool bWithSeparator = false;
    if (!(destination.endsWith("\\") || destination.endsWith("/"))
        && !(type.startsWith("\\") || type.startsWith("/")))
    {
        length++;
        bWithSeparator = true;
    }

    OStringBuffer fileNameBuf(length);

    if (bWithPoint)
        fileNameBuf.append('.');
    else
        fileNameBuf.append(destination);

    if (bWithSeparator)
        fileNameBuf.append("/");

    fileNameBuf.append(type);
    fileNameBuf.append(postfix);

    OString fileName(fileNameBuf.makeStringAndClear());

    char token;
#ifdef SAL_UNX
    fileName = fileName.replace('\\', '/');
    token = '/';
#else
    fileName = fileName.replace('/', '\\');
    token = '\\';
#endif

    OStringBuffer buffer(length);

    sal_Int32 nIndex = 0;
    do
    {
        buffer.append(fileName.getToken(0, token, nIndex));
        if( nIndex == -1 )
            break;

        if (buffer.isEmpty() || OString(".") == buffer.getStr())
        {
            buffer.append(token);
            continue;
        }

#if defined(SAL_UNX)
        if (mkdir(buffer.getStr(), 0777) == -1)
#else
        if (mkdir(buffer.getStr()) == -1)
#endif
        {
            if ( errno == ENOENT )
                return OString();
        }

        buffer.append(token);
    } while(true);

    OUString uSysFileName;
    OSL_VERIFY( FileBase::getSystemPathFromFileURL(
        convertToFileUrl(fileName), uSysFileName) == FileBase::E_None );
    return OUStringToOString(uSysFileName, osl_getThreadTextEncoding());
}

bool fileExists(const OString& fileName)
{
    FILE  *f= fopen(fileName.getStr(), "r");

    if (f != nullptr)
    {
        fclose(f);
        return true;
    }

    return false;
}

static bool checkFileContent(const OString& targetFileName, const OString& tmpFileName)
{
    FILE  *target = fopen(targetFileName.getStr(), "r");
    FILE  *tmp = fopen(tmpFileName.getStr(), "r");
    bool    bFindChanges = false;

    if (target != nullptr && tmp != nullptr)
    {
        char        buffer1[1024+1];
        char        buffer2[1024+1];
        sal_Int32   n1 = 0;
        sal_Int32   n2 = 0;

        while ( !bFindChanges && !feof(target) && !feof(tmp))
        {
            n1 = fread(buffer1, sizeof(char), 1024, target);
            n2 = fread(buffer2, sizeof(char), 1024, tmp);

            if ( n1 != n2 )
                bFindChanges = true;
            else
                if ( memcmp(buffer1, buffer2, n2) != 0 )
                    bFindChanges =  true;
        }
    }

    if (target) fclose(target);
    if (tmp) fclose(tmp);

    return bFindChanges;
}

bool makeValidTypeFile(const OString& targetFileName, const OString& tmpFileName,
                           bool bFileCheck)
{
    if (bFileCheck) {
        if (checkFileContent(targetFileName, tmpFileName)) {
            if ( !unlink(targetFileName.getStr()) )
                if ( !rename(tmpFileName.getStr(), targetFileName.getStr()) )
                    return true;
        } else
            return removeTypeFile(tmpFileName);
    } else {
        if (fileExists(targetFileName))
            if (!removeTypeFile(targetFileName))
                return false;

        if ( rename(tmpFileName.getStr(), targetFileName.getStr()) ) {
            if (errno == EEXIST)
                return true;
        } else
            return true;
    }
    return false;
}

bool removeTypeFile(const OString& fileName)
{
    return unlink(fileName.getStr()) == 0;
}

OUString convertToFileUrl(const OString& fileName)
{
    if ( fileName.startsWith("file://") )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.startsWith(".") || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}


// FileStream

FileStream::FileStream()
    : m_file(nullptr)
{
}

FileStream::~FileStream()
{
    if ( isValid() )
        osl_closeFile(m_file);
}

bool FileStream::isValid() const
{
    return m_file != nullptr;
}

void FileStream::createTempFile(const OString& sPath)
{
    OString sTmp(".");
    OUString sTmpPath;
    OUString sTmpName;

    if (!sPath.isEmpty())
        sTmp = sPath;

    sTmpPath = convertToFileUrl(sTmp);

    if (osl_createTempFile(sTmpPath.pData, &m_file, &sTmpName.pData) == osl_File_E_None) {
#ifdef SAL_UNX
        sal_uInt64 const uAttr = osl_File_Attribute_OwnWrite |
                           osl_File_Attribute_OwnRead |
                           osl_File_Attribute_GrpWrite |
                           osl_File_Attribute_GrpRead |
                           osl_File_Attribute_OthRead;
        if (osl_setFileAttributes(sTmpName.pData, uAttr) != osl_File_E_None) {
            m_file = nullptr;
            return;
        }
#endif
        OUString sSysTmpName;
        FileBase::getSystemPathFromFileURL(sTmpName, sSysTmpName);
        m_name = OUStringToOString(sSysTmpName, osl_getThreadTextEncoding());
    } else
        m_file = nullptr;
}

void FileStream::close()
{
    if ( isValid() )
    {
        osl_closeFile(m_file);
        m_file = nullptr;
        m_name.clear();
    }
}

bool FileStream::write(void const * buffer, sal_uInt64 size) {
    while (size > 0) {
        sal_uInt64 written;
        if (osl_writeFile(m_file, buffer, size, &written) != osl_File_E_None) {
            return false;
        }
        OSL_ASSERT(written <= size);
        size -= written;
        buffer = static_cast< char const * >(buffer) + written;
    }
    return true;
}

FileStream &operator<<(FileStream& o, sal_uInt32 i) {
    sal_uInt64 writtenBytes;
    OString s = OString::number(static_cast<sal_Int32>(i));
    osl_writeFile(o.m_file, s.getStr(), s.getLength() * sizeof(char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, char const * s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s, strlen(s), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, OString const * s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s->getStr(), s->getLength() * sizeof(char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, const OString& s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s.getStr(), s.getLength() * sizeof(char), &writtenBytes);
    return o;

}
FileStream &operator<<(FileStream& o, OStringBuffer const * s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s->getStr(), s->getLength() * sizeof(char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, const OStringBuffer& s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(
        o.m_file, s.getStr(), s.getLength() * sizeof(char), &writtenBytes);
    return o;
}

FileStream & operator <<(FileStream & out, std::u16string_view s) {
    return out << OUStringToOString(s, RTL_TEXTENCODING_UTF8);
}

CannotDumpException::~CannotDumpException() noexcept {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
