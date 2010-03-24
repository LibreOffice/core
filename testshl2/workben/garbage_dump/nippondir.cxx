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
#include "precompiled_testshl2.hxx"

#include <iostream>
#include<osl/file.hxx>

#include<osl/thread.hxx>
#include <tools/string.hxx>

// -----------------------------------------------------------------------------
// ---------------------------------- defines ----------------------------------
// -----------------------------------------------------------------------------
#ifndef ASCII
#define ASCII(x) OUString::createFromAscii(x)
#endif

using namespace osl;
using namespace rtl;
using namespace std;


ostream& operator << (ostream& out, rtl::OUString const& aStr)
{
    sal_Unicode const* const pStr = aStr.getStr();
    sal_Unicode const* const pEnd = pStr + aStr.getLength();
    for (sal_Unicode const* p = pStr; p < pEnd; ++p)
        if (0 < *p && *p < 127) // ASCII
            out << char(*p);
        else
            out << "[\\u" << hex << *p << "]";
    return out;
}


inline void operator <<= (::rtl::OUString& _rUnicodeString, const sal_Char* _pAsciiString)
{
    _rUnicodeString = ::rtl::OUString::createFromAscii(_pAsciiString);
}

inline void operator <<= (::rtl::OUString& _rUnicodeString, const ::rtl::OString& _rAsciiString)
{
    _rUnicodeString <<= _rAsciiString.getStr();
}

/*
inline void operator <<= (Any& _rUnoValue, const sal_Char* _pAsciiString)
{
    _rUnoValue <<= ::rtl::OUString::createFromAscii(_pAsciiString);
}

inline void operator <<= (Any& _rUnoValue, const ::rtl::OString& _rAsciiString)
{
    _rUnoValue <<= _rAsciiString.getStr();
}
*/

inline void operator <<= (::rtl::OString& _rAsciiString, ::rtl::OUString const& _rUnicodeString )
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString, RTL_TEXTENCODING_ASCII_US).getStr();
}
// -----------------------------------------------------------------------------

namespace FileHelper
{
    rtl::OUString createOSLErrorString(FileBase::RC eError)
    {
        rtl::OUString aRet;
        switch(eError)
        {
        case osl_File_E_None:
            break;

        case osl_File_E_PERM:
            aRet = ASCII("Operation not permitted");
            break;

        case osl_File_E_NOENT:
            aRet = ASCII("No such file or directory");
            break;

        case osl_File_E_SRCH:
            aRet = ASCII("unknown error: osl_File_E_SRCH");
            break;

        case osl_File_E_INTR:
            aRet = ASCII("function call was interrupted");
            break;

        case osl_File_E_IO:
            aRet = ASCII("I/O error");
            break;

        case osl_File_E_NXIO:
            aRet = ASCII("No such device or address");
            break;

        case osl_File_E_2BIG:
            aRet = ASCII("unknown error: osl_File_E_2BIG");
            break;

        case osl_File_E_NOEXEC:
            aRet = ASCII("unknown error: osl_File_E_NOEXEC");
            break;

        case osl_File_E_BADF:
            aRet = ASCII("Bad file");
            break;

        case osl_File_E_CHILD:
            aRet = ASCII("unknown error: osl_File_E_CHILD");
            break;

        case osl_File_E_AGAIN:
            aRet = ASCII("Operation would block");
            break;

        case osl_File_E_NOMEM:
            aRet = ASCII("not enough memory for allocating structures");
            break;

        case osl_File_E_ACCES:
            aRet = ASCII("Permission denied");
            break;

        case osl_File_E_FAULT:
            aRet = ASCII("Bad address");
            break;

        case osl_File_E_BUSY:
            aRet = ASCII("Text file busy");
            break;

        case osl_File_E_EXIST:
            aRet = ASCII("File exists");
            break;

        case osl_File_E_XDEV:
            aRet = ASCII("unknown error: osl_File_E_XDEV");
            break;

        case osl_File_E_NODEV:
            aRet = ASCII("No such device");
            break;

        case osl_File_E_NOTDIR:
            aRet = ASCII("Not a directory");
            break;

        case osl_File_E_ISDIR:
            aRet = ASCII("Is a director");
            break;

        case osl_File_E_INVAL:
            aRet = ASCII("the format of the parameters was not valid");
            break;

        case osl_File_E_NFILE:
            aRet = ASCII("too many open files in the system");
            break;

        case osl_File_E_MFILE:
            aRet = ASCII("too many open files used by the process");
            break;

        case osl_File_E_NOTTY:
            aRet = ASCII("unknown error: osl_File_E_NOTTY");
            break;

        case osl_File_E_FBIG:
            aRet = ASCII("File too large");
            break;

        case osl_File_E_NOSPC:
            aRet = ASCII("No space left on device");
            break;

        case osl_File_E_SPIPE:
            aRet = ASCII("unknown error: osl_File_E_SPIPE");
            break;

        case osl_File_E_ROFS:
            aRet = ASCII("Read-only file system");
            break;

        case osl_File_E_MLINK:
            aRet = ASCII("Too many links");
            break;

        case osl_File_E_PIPE:
            aRet = ASCII("unknown error: osl_File_E_PIPE");
            break;

        case osl_File_E_DOM:
            aRet = ASCII("unknown error: osl_File_E_DOM");
            break;

        case osl_File_E_RANGE:
            aRet = ASCII("unknown error: osl_File_E_RANGE");
            break;

        case osl_File_E_DEADLK:
            aRet = ASCII("unknown error: osl_File_E_DEADLK");
            break;

        case osl_File_E_NAMETOOLONG:
            aRet = ASCII("File name too long");
            break;

        case osl_File_E_NOLCK:
            aRet = ASCII("No record locks available");
            break;

        case osl_File_E_NOSYS:
            aRet = ASCII("Function not implemente");
            break;

        case osl_File_E_NOTEMPTY:
            aRet = ASCII("Directory not empt");
            break;

        case osl_File_E_LOOP:
            aRet = ASCII("Too many symbolic links encountered");
            break;

        case osl_File_E_ILSEQ:
            aRet = ASCII("unknown error: osl_File_E_ILSEQ");
            break;

        case osl_File_E_NOLINK:
            aRet = ASCII("Link has been severed");
            break;

        case osl_File_E_MULTIHOP:
            aRet = ASCII("Multihop attempted");
            break;

        case osl_File_E_USERS:
            aRet = ASCII("unknown error: osl_File_E_USERS");
            break;

        case osl_File_E_OVERFLOW:
            aRet = ASCII("Value too large for defined data type");
            break;

            /* unmapped error: always last entry in enum! */
        case osl_File_E_invalidError:
            aRet = ASCII("unmapped Error");
            break;
        }
        return aRet;
    }
} // namespace FileHelper


#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif
{

    OUString aDirectory(OUString::createFromAscii("file:///C:/%E6%96%B0%E3%81%97%E3%81%84%E3%83%95%E3%82%A9%E3%83%AB%E3%83%80"));
//  OUString aDirex(OUString::createFromAscii("file:///C:/%C2%90V%E2%80%9A%C2%B5%E2%80%9A%C2%A2%C6%92t%C6%92H%C6%92%E2%80%B9%C6%92_"));

    osl::FileBase::RC eError = osl::Directory::create(aDirectory);
    if (eError != osl::FileBase::E_None)
    {
        OUString aUStr = FileHelper::createOSLErrorString(eError);
        OString aStr;
        aStr <<= aUStr;
        OSL_ENSURE(false, aStr.getStr());
    }

//        OUString suSystemPath, aTmp;
//        FileBase::RC aRC  = FileBase::getSystemPathFromFileURL( aDirectory, suSystemPath );
//
//        String suSystemPathAsString(suSystemPath);
//        ByteString sSystemPath(suSystemPathAsString, osl_getThreadTextEncoding());
//
//
//        ByteString sSystemPathNew = sSystemPath;
//        UniString suSystemPathNew(sSystemPathNew, osl_getThreadTextEncoding());
//        FileBase::RC aRC2 = FileBase::getFileURLFromSystemPath( suSystemPathNew, aTmp );

//
//     aRC  = FileBase::getSystemPathFromFileURL( aDirex, aTmp );
//     aRC2 = FileBase::getFileURLFromSystemPath( aTmp, aTmp2 );
}

