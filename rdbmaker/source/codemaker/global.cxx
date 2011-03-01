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
#include <osl/process.h>
#include <rtl/strbuf.hxx>
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
        m_pFile = fopen(m_name.getStr(), checkAccessMode(mode));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
