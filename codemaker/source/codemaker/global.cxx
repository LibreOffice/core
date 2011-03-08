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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"
#include "osl/process.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustring.hxx"
#include "osl/thread.h"
#include "osl/file.hxx"

#include <string.h>
#if defined(SAL_W32) || defined(SAL_OS2)
#include <io.h>

#include <direct.h>
#include <errno.h>
#endif

#ifdef UNX
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#endif

#include "codemaker/global.hxx"

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
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
                                const OString typeName,
                                const OString postfix,
                                sal_Bool bLowerCase,
                                const OString prefix )
{
    OString type(typeName);

    if (bLowerCase)
    {
        type = typeName.toAsciiLowerCase();
    }

    sal_uInt32 length = destination.getLength();

    sal_Bool withPoint = sal_False;
    if (length == 0)
    {
        length++;
        withPoint = sal_True;
    }

    length += prefix.getLength() + type.getLength() + postfix.getLength();

    sal_Bool withSeperator = sal_False;
    if (destination.getStr()[destination.getLength()] != '\\' &&
        destination.getStr()[destination.getLength()] != '/' &&
        type.getStr()[0] != '\\' &&
        type.getStr()[0] != '/')
    {
        length++;
        withSeperator = sal_True;
    }

    OStringBuffer nameBuffer(length);

    if (withPoint)
        nameBuffer.append('.');
    else
        nameBuffer.append(destination.getStr(), destination.getLength());

    if (withSeperator)
        nameBuffer.append("/", 1);

    OString tmpStr(type);
    if (prefix.getLength() > 0)
    {
        tmpStr = type.replaceAt(type.lastIndexOf('/')+1, 0, prefix);
    }

    nameBuffer.append(tmpStr.getStr(), tmpStr.getLength());
    nameBuffer.append(postfix.getStr(), postfix.getLength());

    OString fileName(nameBuffer);

    sal_Char token;
#ifdef SAL_UNX
    fileName = fileName.replace('\\', '/');
    token = '/';
#else
    fileName = fileName.replace('/', '\\');
    token = '\\';
#endif

    nameBuffer = OStringBuffer(length);

    sal_Int32 nIndex = 0;
    do
    {
        nameBuffer.append(fileName.getToken(0, token, nIndex).getStr());
        if( nIndex == -1 )
            break;

        if (nameBuffer.getLength() == 0 || OString(".") == nameBuffer.getStr())
        {
            nameBuffer.append(token);
            continue;
        }

#if defined(SAL_UNX) || defined(SAL_OS2)
        if (mkdir((char*)nameBuffer.getStr(), 0777) == -1)
#else
        if (mkdir((char*)nameBuffer.getStr()) == -1)
#endif
        {
            if ( errno == ENOENT )
                return OString();
        }

        nameBuffer.append(token);
    } while( nIndex != -1 );

    OUString uSysFileName;
    OSL_VERIFY( FileBase::getSystemPathFromFileURL(
        convertToFileUrl(fileName), uSysFileName) == FileBase::E_None );
    return OUStringToOString(uSysFileName, osl_getThreadTextEncoding());;
}

sal_Bool fileExists(const OString& fileName)
{
    FILE  *f= fopen(fileName.getStr(), "r");

    if (f != NULL)
    {
        fclose(f);
        return sal_True;
    }

    return sal_False;
}

sal_Bool checkFileContent(const OString& targetFileName, const OString& tmpFileName)
{
    FILE  *target = fopen(targetFileName.getStr(), "r");
    FILE  *tmp = fopen(tmpFileName.getStr(), "r");
    sal_Bool    bFindChanges = sal_False;

    if (target != NULL && tmp != NULL)
    {
        sal_Char    buffer1[1024+1];
        sal_Char    buffer2[1024+1];
        sal_Int32   n1 = 0;
        sal_Int32   n2 = 0;

        while ( !bFindChanges && !feof(target) && !feof(tmp))
        {
            n1 = fread(buffer1, sizeof(sal_Char), 1024, target);
            n2 = fread(buffer2, sizeof(sal_Char), 1024, tmp);

            if ( n1 != n2 )
                bFindChanges = sal_True;
            else
                if ( rtl_compareMemory(buffer1, buffer2, n2) != 0 )
                    bFindChanges =  sal_True;
        }
    }

    if (target) fclose(target);
    if (tmp) fclose(tmp);

    return bFindChanges;
}

sal_Bool makeValidTypeFile(const OString& targetFileName, const OString& tmpFileName,
                           sal_Bool bFileCheck)
{
    if (bFileCheck) {
        if (checkFileContent(targetFileName, tmpFileName)) {
            if ( !unlink(targetFileName.getStr()) )
                if ( !rename(tmpFileName.getStr(), targetFileName.getStr()) )
                    return sal_True;
        } else
            return removeTypeFile(tmpFileName);
    } else {
        if (fileExists(targetFileName))
            if (!removeTypeFile(targetFileName))
                return sal_False;

        if ( rename(tmpFileName.getStr(), targetFileName.getStr()) ) {
            if (errno == EEXIST)
                return sal_True;
        } else
            return sal_True;
    }
    return sal_False;
}

sal_Bool removeTypeFile(const OString& fileName)
{
    if ( !unlink(fileName.getStr()) )
        return sal_True;

    return sal_False;
}

static sal_Bool isFileUrl(const OString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString convertToFileUrl(const OString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
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


//*************************************************************************
// FileStream
//*************************************************************************
FileStream::FileStream()
    : m_file(NULL)
{
}

FileStream::FileStream(const OString& name, FileAccessMode mode)
    : m_file(NULL)
{
    if ( name.getLength() > 0 )
    {
        OUString sUrl(convertToFileUrl(name));
#ifdef SAL_UNX
        sal_uInt64 uAttr = osl_File_Attribute_OwnWrite |
                           osl_File_Attribute_OwnRead |
                           osl_File_Attribute_GrpWrite |
                           osl_File_Attribute_GrpRead |
                           osl_File_Attribute_OthRead;
        if (osl_openFile(sUrl.pData, &m_file, checkAccessMode(mode)) == osl_File_E_None &&
            osl_setFileAttributes(sUrl.pData, uAttr) == osl_File_E_None)
#else
        if (osl_openFile(sUrl.pData, &m_file, checkAccessMode(mode)) == osl_File_E_None)
#endif
            m_name = name;
        else
            m_file = NULL;
    }
}

FileStream::~FileStream()
{
    if ( isValid() )
        osl_closeFile(m_file);
}

sal_Bool FileStream::isValid()
{
    if ( m_file )
        return sal_True;

    return sal_False;
}

void FileStream::createTempFile(const OString& sPath)
{
    OString sTmp(".");
    OUString sTmpPath;
    OUString sTmpName;

    if (sPath.getLength() > 0)
        sTmp = sPath;

    sTmpPath = convertToFileUrl(sTmp);

    if (osl_createTempFile(sTmpPath.pData, &m_file, &sTmpName.pData) == osl_File_E_None) {
#ifdef SAL_UNX
        sal_uInt64 uAttr = osl_File_Attribute_OwnWrite |
                           osl_File_Attribute_OwnRead |
                           osl_File_Attribute_GrpWrite |
                           osl_File_Attribute_GrpRead |
                           osl_File_Attribute_OthRead;
        if (osl_setFileAttributes(sTmpName.pData, uAttr) != osl_File_E_None) {
            m_file = NULL;
            return;
        }
#endif
        OUString sSysTmpName;
        FileBase::getSystemPathFromFileURL(sTmpName, sSysTmpName);
        m_name = OUStringToOString(sSysTmpName, osl_getThreadTextEncoding());
    } else
        m_file = NULL;
}

void FileStream::open(const OString& name, FileAccessMode mode)
{
    if ( name.getLength() > 0 )
    {
        oslFileError ret =  osl_File_E_None;
        if ((ret = osl_openFile(convertToFileUrl(name).pData, &m_file, checkAccessMode(mode))) == osl_File_E_None)
            m_name = name;
        else
            m_file = NULL;
    }
}

void FileStream::close()
{
    if ( isValid() )
    {
        osl_closeFile(m_file);
        m_file = NULL;
        m_name = OString();
    }
}

sal_uInt32 FileStream::checkAccessMode(FileAccessMode mode)
{
    switch( mode )
    {
    case FAM_READ:
        return osl_File_OpenFlag_Read;
    case FAM_WRITE:
        return osl_File_OpenFlag_Write;
    case FAM_READWRITE_EXIST:
        return osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;
    case FAM_READWRITE:
        return osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create;
    }
    return osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create;
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
    OString s = OString::valueOf((sal_Int32)i);
    osl_writeFile(o.m_file, s.getStr(), s.getLength() * sizeof(sal_Char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, char const * s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s, strlen(s), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, ::rtl::OString* s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s->getStr(), s->getLength() * sizeof(sal_Char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, const ::rtl::OString& s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s.getStr(), s.getLength() * sizeof(sal_Char), &writtenBytes);
    return o;

}
FileStream &operator<<(FileStream& o, ::rtl::OStringBuffer* s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(o.m_file, s->getStr(), s->getLength() * sizeof(sal_Char), &writtenBytes);
    return o;
}
FileStream &operator<<(FileStream& o, const ::rtl::OStringBuffer& s) {
    sal_uInt64 writtenBytes;
    osl_writeFile(
        o.m_file, s.getStr(), s.getLength() * sizeof(sal_Char), &writtenBytes);
    return o;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
