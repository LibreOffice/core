/*************************************************************************
 *
 *  $RCSfile: global.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-20 13:09:02 $
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

#ifndef _CODEMAKER_GLOBAL_HXX_
#define _CODEMAKER_GLOBAL_HXX_

#include <list>
#include <vector>
#include <set>

#include <stdio.h>

#include "osl/file.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

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
    FAM_READWRITE,              // "w+"
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

    // friend functions
    friend FileStream &operator<<(FileStream& o, sal_uInt32 i);
    friend FileStream &operator<<(FileStream& o, sal_Char* s);
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

const ::rtl::OString inGlobalSet(const ::rtl::OUString & r);
inline const ::rtl::OString inGlobalSet(sal_Char* p)
{
    return inGlobalSet( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(p) ) );
}

::rtl::OUString convertToFileUrl(const ::rtl::OString& fileName);


#endif // _CODEMAKER_GLOBAL_HXX_

