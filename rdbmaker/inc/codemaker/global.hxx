/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

const ::rtl::OString inGlobalSet(const ::rtl::OUString & r);

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
    virtual ~FileStream();

    sal_Bool isValid();

    void open(const ::rtl::OString& name, FileAccessMode nMode = FAM_READWRITE);
    void close();

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

