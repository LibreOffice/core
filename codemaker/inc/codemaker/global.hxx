/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: global.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:22:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_CODEMAKER_GLOBAL_HXX
#define INCLUDED_CODEMAKER_GLOBAL_HXX

#include <list>
#include <vector>
#include <set>

#include <stdio.h>

#ifndef _OSL_FILE_HXX_
#include "osl/file.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _RTL_STRBUF_HXX_
#include "rtl/strbuf.hxx"
#endif

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

//*************************************************************************
// FileStream
//*************************************************************************
enum FileAccessMode
{
    FAM_READ,                   // "r"
    FAM_WRITE,                  // "w"
    FAM_READWRITE_EXIST,        // "r+"
    FAM_READWRITE               // "w+"
};

class FileStream
{
public:
    FileStream();
    FileStream(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    virtual ~FileStream();

    sal_Bool isValid();

    void open(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    void createTempFile(const ::rtl::OString& sPath);
    void close();

    ::rtl::OString  getName() { return m_name; }

    bool write(void const * buffer, sal_uInt64 size);

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i);
    friend FileStream &operator<<(FileStream& o, char const * s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OString* s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OString& s);
    friend FileStream &operator<<(FileStream& o, ::rtl::OStringBuffer* s);
    friend FileStream &operator<<(FileStream& o, const ::rtl::OStringBuffer& s);

private:
    sal_uInt32 checkAccessMode(FileAccessMode mode);

    oslFileHandle m_file;
    ::rtl::OString  m_name;
};


//*************************************************************************
// Helper functions
//*************************************************************************
::rtl::OString getTempDir(const ::rtl::OString& sFileName);

::rtl::OString createFileNameFromType(const ::rtl::OString& destination,
                                      const ::rtl::OString type,
                                      const ::rtl::OString postfix,
                                      sal_Bool bLowerCase=sal_False,
                                      const ::rtl::OString prefix="");

sal_Bool fileExists(const ::rtl::OString& fileName);
sal_Bool makeValidTypeFile(const ::rtl::OString& targetFileName,
                           const ::rtl::OString& tmpFileName,
                           sal_Bool bFileCheck);
sal_Bool removeTypeFile(const ::rtl::OString& fileName);

::rtl::OUString convertToFileUrl(const ::rtl::OString& fileName);

//*************************************************************************
// Global exception to signal problems when a type cannot be dumped
//*************************************************************************
class CannotDumpException
{
public:
    CannotDumpException(const ::rtl::OString& msg)
        : m_message(msg) {}

    ::rtl::OString  m_message;
};


#endif // INCLUDED_CODEMAKER_GLOBAL_HXX

