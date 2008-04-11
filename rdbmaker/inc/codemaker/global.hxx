/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: global.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CODEMAKER_GLOBAL_HXX_
#define _CODEMAKER_GLOBAL_HXX_

#include <list>
#include <vector>
#include <set>

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

struct EqualString
{
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 == str2);
    }
};

struct HashString
{
    size_t operator()(const ::rtl::OString& str) const
    {
        return str.hashCode();
    }
};

struct LessString
{
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 < str2);
    }
};

#if defined(_MSC_VER) &&  _MSC_VER < 1200
typedef ::std::new_alloc NewAlloc;
#endif


typedef ::std::list< ::rtl::OString >               StringList;
typedef ::std::vector< ::rtl::OString >             StringVector;
typedef ::std::set< ::rtl::OString, LessString >    StringSet;

::rtl::OString makeTempName();

::rtl::OString createFileNameFromType(const ::rtl::OString& destination,
                                      const ::rtl::OString type,
                                      const ::rtl::OString postfix,
                                      sal_Bool bLowerCase=sal_False,
                                      const ::rtl::OString prefix="");

sal_Bool fileExists(const ::rtl::OString& fileName);
sal_Bool checkFileContent(const ::rtl::OString& targetFileName, const ::rtl::OString& tmpFileName);

const ::rtl::OString inGlobalSet(const ::rtl::OUString & r);
inline const ::rtl::OString inGlobalSet(sal_Char* p)
{
    return inGlobalSet( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(p) ) );
}

::rtl::OUString convertToFileUrl(const ::rtl::OString& fileName);

//*************************************************************************
// FileStream
//*************************************************************************
enum FileAccessMode
{
    FAM_READ,                   // "r"
    FAM_WRITE,                  // "w"
    FAM_APPEND,                 // "a"
    FAM_READWRITE_EXIST,        // "r+"
    FAM_READWRITE,              // "w+"
    FAM_READAPPEND              // "a+"
};

class FileStream //: public ofstream
{
public:
    FileStream();
    FileStream(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    virtual ~FileStream();

    sal_Bool isValid();

    void open(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    void close();

    sal_Int32       getSize();
    ::rtl::OString  getName() { return m_name; }

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i)
        {   fprintf(o.m_pFile, "%lu", sal::static_int_cast< unsigned long >(i));
            return o;
        }
    friend FileStream &operator<<(FileStream& o, char const * s)
        {   fprintf(o.m_pFile, "%s", s);
            return o;
        }
    friend FileStream &operator<<(FileStream& o, ::rtl::OString* s)
        {   fprintf(o.m_pFile, "%s", s->getStr());
            return o;
        }
    friend FileStream &operator<<(FileStream& o, const ::rtl::OString& s)
        {   fprintf(o.m_pFile, "%s", s.getStr());
            return o;
        }
    friend FileStream &operator<<(FileStream& o, ::rtl::OStringBuffer* s)
        {   fprintf(o.m_pFile, "%s", s->getStr());
            return o;
        }
    friend FileStream &operator<<(FileStream& o, const ::rtl::OStringBuffer& s)
        {   fprintf(o.m_pFile, "%s", s.getStr());
            return o;
        }

protected:
    const sal_Char* checkAccessMode(FileAccessMode mode);

    FILE*               m_pFile;
    ::rtl::OString      m_name;
};

#endif // _CODEMAKER_GLOBAL_HXX_

