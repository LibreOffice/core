/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: global.cxx,v $
 * $Revision: 1.14 $
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
#include <osl/process.h>
#ifndef _RTL_OSTRINGBUFFER_HXX_
#include <rtl/strbuf.hxx>
#endif
#include <rtl/ustring.hxx>
#include    <osl/thread.h>
#include    <osl/file.hxx>

#include <stdlib.h>
#include <stdio.h>
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
#include    <codemaker/global.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
using namespace ::osl;

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
        nameBuffer.append(fileName.getToken( 0, token, nIndex ).getStr());

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
    }
    while ( nIndex >= 0 );

    return fileName;
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
    sal_Bool ret = sal_False;

    if (target != NULL && tmp != NULL)
    {
        sal_Bool    bFindChanges = sal_False;
        sal_Char    buffer1[1024+1];
        sal_Char    buffer2[1024+1];
        sal_Int32   n1 = 0;
        sal_Int32   n2 = 0;

        while ( !bFindChanges && !feof(target) && !feof(tmp))
        {
            n1 = fread(buffer1, sizeof(sal_Char), 1024, target);
            n2 = fread(buffer2, sizeof(sal_Char), 1024, tmp);

            if ( n1 != n2 )
            {
                bFindChanges = sal_True;
            }
            else
            {
                if ( rtl_compareMemory(buffer1, buffer2, n2) != 0 )
                    bFindChanges =  sal_True;
            }
        }

        fclose(target);
        fclose(tmp);

        if ( bFindChanges )
        {
            if ( !unlink(targetFileName.getStr()) )
                if ( !rename(tmpFileName.getStr(), targetFileName.getStr()) )
                    ret = sal_True;
        }
        else
        {
            if ( !unlink(tmpFileName.getStr()) )
                ret = sal_True;
        }
    }

    return ret;
}

const OString inGlobalSet(const OUString & rValue)
{
    OString sValue( OUStringToOString(rValue, RTL_TEXTENCODING_UTF8) );
    static StringSet aGlobalMap;
    StringSet::iterator iter = aGlobalMap.find( sValue );
    if( iter != aGlobalMap.end() )
        return *iter;
    return *(aGlobalMap.insert( sValue ).first);
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
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None) {
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
{
}

FileStream::FileStream(const OString& name, FileAccessMode mode)
    : m_pFile(NULL)
{
    if ( name.getLength() > 0 )
    {
        m_name = name;
        m_pFile = fopen(m_name, checkAccessMode(mode));
    }
}

FileStream::~FileStream()
{
    if ( isValid() )
    {
        fflush(m_pFile);
        fclose(m_pFile);
    }
}

sal_Bool FileStream::isValid()
{
    if ( m_pFile )
        return sal_True;

    return sal_False;
}

void FileStream::open(const OString& name, FileAccessMode mode)
{
    if ( name.getLength() > 0 )
    {
        m_name = name;
        m_pFile = fopen(m_name, checkAccessMode(mode));
    }
}

void FileStream::close()
{
    if ( isValid() )
    {
        fflush(m_pFile);
        fclose(m_pFile);
        m_pFile = NULL;
        m_name = OString();
    }
}

sal_Int32 FileStream::getSize()
{
    sal_Int32 pos = 0;
    sal_Int32 size = 0;
    if ( isValid() )
    {
        fflush(m_pFile);
        pos = ftell(m_pFile);
        if (!fseek(m_pFile, 0, SEEK_END))
            size = ftell(m_pFile);
        fseek(m_pFile, pos, SEEK_SET);
    }
    return size;
}

const sal_Char* FileStream::checkAccessMode(FileAccessMode mode)
{
    switch( mode )
    {
    case FAM_READ:
        return "r";
    case FAM_WRITE:
        return "w";
    case FAM_APPEND:
        return "a";
    case FAM_READWRITE_EXIST:
        return "r+";
    case FAM_READWRITE:
        return "w+";
    case FAM_READAPPEND:
        return "a+";
    }
    return "w+";
}

