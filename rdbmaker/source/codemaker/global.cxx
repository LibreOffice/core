/*************************************************************************
 *
 *  $RCSfile: global.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 14:28:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_OSTRINGBUFFER_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef     _OSL_THREAD_H_
#include    <osl/thread.h>
#endif
#ifndef     _OSL_FILE_HXX_
#include    <osl/file.hxx>
#endif

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

#ifndef _CODEMAKER_GLOBAL_HXX_
#include    <codemaker/global.hxx>
#endif

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
using namespace ::osl;

OString makeTempName(sal_Char* prefix)
{
    static OUString uTMP( RTL_CONSTASCII_USTRINGPARAM("TMP") );
    static OUString uTEMP( RTL_CONSTASCII_USTRINGPARAM("TEMP") );
    OUString        uPrefix( RTL_CONSTASCII_USTRINGPARAM("cmk_") );
    OUString        uPattern;

    sal_Char*   pPrefix = "cmk_";
    sal_Char    tmpPattern[512];
    sal_Char    *pTmpName = NULL;

    if (prefix)
        pPrefix = prefix;

    if ( osl_getEnvironment(uTMP.pData, &uPattern.pData) != osl_Process_E_None )
    {
        if ( osl_getEnvironment(uTEMP.pData, &uPattern.pData) != osl_Process_E_None )
        {
#if defined(SAL_W32) || defined(SAL_OS2)
            OSL_ASSERT( sizeof(tmpPattern) > RTL_CONSTASCII_LENGTH( "." ) );
            strncpy(tmpPattern, ".", sizeof(tmpPattern)-1);
#else
            OSL_ASSERT( sizeof(tmpPattern) > RTL_CONSTASCII_LENGTH( "/tmp" ) );
            strncpy(tmpPattern, "/tmp", sizeof(tmpPattern)-1);
#endif
        }
    }

    if (uPattern.getLength())
    {
        OString aOStr( OUStringToOString(uPattern, RTL_TEXTENCODING_UTF8) );
        OSL_ASSERT( sizeof(tmpPattern) > aOStr.getLength() );
        strncpy(tmpPattern, aOStr.getStr(), sizeof(tmpPattern)-1);
    }

#ifdef SAL_W32
    OSL_ASSERT( sizeof(tmpPattern) > ( strlen(tmpPattern)
                                       + RTL_CONSTASCII_LENGTH("\\")
                                       + strlen(pPrefix)
                                       + RTL_CONSTASCII_LENGTH("XXXXXX") ) );
    strncat(tmpPattern, "\\", sizeof(tmpPattern)-1-strlen(tmpPattern));
    strncat(tmpPattern, pPrefix, sizeof(tmpPattern)-1-strlen(tmpPattern));
    strncat(tmpPattern, "XXXXXX", sizeof(tmpPattern)-1-strlen(tmpPattern));
    pTmpName = mktemp(tmpPattern);
#endif

#ifdef SAL_OS2
    char* tmpname = tempnam(NULL, prefix);
    OSL_ASSERT( sizeof(tmpPattern) > strlen(tmpname) );
    strncpy(tmpPattern, tmpname, sizeof(tmpPattern)-1);
    pTmpName = tmpPattern;
#endif

#ifdef SAL_UNX
    OSL_ASSERT( sizeof(tmpPattern) > ( strlen(tmpPattern)
                                       + RTL_CONSTASCII_LENGTH("/")
                                       + strlen(pPrefix)
                                       + RTL_CONSTASCII_LENGTH("XXXXXX") ) );
    strncat(tmpPattern, "/", sizeof(tmpPattern)-1-strlen(tmpPattern));
    strncat(tmpPattern, pPrefix, sizeof(tmpPattern)-1-strlen(tmpPattern));
    strncat(tmpPattern, "XXXXXX", sizeof(tmpPattern)-1-strlen(tmpPattern));
    pTmpName = mktemp(tmpPattern);
#endif

    return OString(pTmpName);
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
        nameBuffer.append(fileName.getToken( 0, token, nIndex ).getStr());

        if (nameBuffer.getLength() == 0 || OString(".") == nameBuffer.getStr())
        {
            nameBuffer.append(token);
            continue;
        }

#ifdef SAL_UNX
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
        OSL_VERIFY( osl_getProcessWorkingDir(&uWorkingDir.pData) == osl_Process_E_None );
        OSL_VERIFY( FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName) == FileBase::E_None );
    } else
    {
        OSL_VERIFY( FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName) == FileBase::E_None );
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

