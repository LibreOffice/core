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

#ifndef HELPCOMPILER_HXX
#define HELPCOMPILER_HXX

#include <string>
#include <hash_map>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#ifdef SYSTEM_DB
#include <db.h>
#else
#include <berkeleydb/db.h>
#endif

#include <boost/shared_ptr.hpp>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>

#include <rtl/ustring.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>

#include <compilehelp.hxx>

#define EMULATEORIGINAL 1

#ifdef CMCDEBUG
    #define HCDBG(foo) do { if (1) foo; } while(0)
#else
    #define HCDBG(foo) do { if (0) foo; } while(0)
#endif

namespace fs
{
    rtl_TextEncoding getThreadTextEncoding( void );

    enum convert { native };
    class path
    {
    public:
        ::rtl::OUString data;
    public:
        path() {}
        path(const path &rOther) : data(rOther.data) {}
        path(const std::string &in, convert)
        {
            rtl::OUString sWorkingDir;
            osl_getProcessWorkingDir(&sWorkingDir.pData);

            rtl::OString tmp(in.c_str());
            rtl::OUString ustrSystemPath(rtl::OStringToOUString(tmp, getThreadTextEncoding()));
            osl::File::getFileURLFromSystemPath(ustrSystemPath, data);
            osl::File::getAbsoluteFileURL(sWorkingDir, data, data);
        }
        path(const std::string &FileURL)
        {
            rtl::OString tmp(FileURL.c_str());
            data = rtl::OStringToOUString(tmp, getThreadTextEncoding());
        }
        std::string native_file_string() const
        {
            ::rtl::OUString ustrSystemPath;
            osl::File::getSystemPathFromFileURL(data, ustrSystemPath);
            rtl::OString tmp(rtl::OUStringToOString(ustrSystemPath, getThreadTextEncoding()));
            HCDBG(std::cerr << "native_file_string is " << tmp.getStr() << std::endl);
            return std::string(tmp.getStr());
        }
        wchar_t const * native_file_string_w() const
        {
            ::rtl::OUString ustrSystemPath;
            osl::File::getSystemPathFromFileURL(data, ustrSystemPath);
            return ustrSystemPath.getStr();
        }

        std::string native_directory_string() const { return native_file_string(); }
        std::string toUTF8() const
        {
            rtl::OString tmp(rtl::OUStringToOString(data, RTL_TEXTENCODING_UTF8));
            return std::string(tmp.getStr());
        }
        bool empty() const { return data.getLength() == 0; }
        path operator/(const std::string &in) const
        {
            path ret(*this);
            HCDBG(std::cerr << "orig was " <<
                rtl::OUStringToOString(ret.data, RTL_TEXTENCODING_UTF8).getStr() << std::endl);
            rtl::OString tmp(in.c_str());
            rtl::OUString ustrSystemPath(rtl::OStringToOUString(tmp, getThreadTextEncoding()));
            ret.data += rtl::OUString(sal_Unicode('/'));
            ret.data += ustrSystemPath;
            HCDBG(std::cerr << "final is " <<
                rtl::OUStringToOString(ret.data, RTL_TEXTENCODING_UTF8).getStr() << std::endl);
            return ret;
        }
        void append(const char *in)
        {
            rtl::OString tmp(in);
            rtl::OUString ustrSystemPath(rtl::OStringToOUString(tmp, getThreadTextEncoding()));
            data = data + ustrSystemPath;
        }
        void append(const std::string &in) { append(in.c_str()); }
    };

    void create_directory(const fs::path indexDirName);
    void rename(const fs::path &src, const fs::path &dest);
    void copy(const fs::path &src, const fs::path &dest);
    bool exists(const fs::path &in);
    void remove_all(const fs::path &in);
    void remove(const fs::path &in);
}

struct joaat_hash
{
    size_t operator()(const std::string &str) const
    {
        size_t hash = 0;
        const char *key = str.data();
        for (size_t i = 0; i < str.size(); i++)
        {
            hash += key[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        return hash;
    }
};

#define get16bits(d) ((((sal_uInt32)(((const sal_uInt8 *)(d))[1])) << 8)\
                       +(sal_uInt32)(((const sal_uInt8 *)(d))[0]) )

struct SuperFastHash
{
    size_t operator()(const std::string &str) const
    {
        const char * data = str.data();
        int len = str.size();
        size_t hash = len, tmp;
        if (len <= 0 || data == NULL) return 0;

        int rem = len & 3;
        len >>= 2;

        /* Main loop */
        for (;len > 0; len--)
        {
            hash  += get16bits (data);
            tmp    = (get16bits (data+2) << 11) ^ hash;
            hash   = (hash << 16) ^ tmp;
            data  += 2*sizeof (sal_uInt16);
            hash  += hash >> 11;
        }

        /* Handle end cases */
        switch (rem)
        {
            case 3: hash += get16bits (data);
                    hash ^= hash << 16;
                    hash ^= data[sizeof (sal_uInt16)] << 18;
                    hash += hash >> 11;
                    break;
            case 2: hash += get16bits (data);
                    hash ^= hash << 11;
                    hash += hash >> 17;
                    break;
            case 1: hash += *data;
                    hash ^= hash << 10;
                    hash += hash >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash;
    }
};

#define pref_hash joaat_hash

typedef std::hash_map<std::string, std::string, pref_hash> Stringtable;
typedef std::list<std::string> LinkedList;
typedef std::vector<std::string> HashSet;

typedef std::hash_map<std::string, LinkedList, pref_hash> Hashtable;

class StreamTable
{
public:
    std::string document_id;
    std::string document_path;
    std::string document_module;
    std::string document_title;

    HashSet *appl_hidlist;
    Hashtable *appl_keywords;
    Stringtable *appl_helptexts;
    xmlDocPtr appl_doc;

    HashSet *default_hidlist;
    Hashtable *default_keywords;
    Stringtable *default_helptexts;
    xmlDocPtr default_doc;

    StreamTable() :
        appl_hidlist(NULL), appl_keywords(NULL), appl_helptexts(NULL), appl_doc(NULL),
        default_hidlist(NULL), default_keywords(NULL), default_helptexts(NULL), default_doc(NULL)
    {}
    void dropdefault()
    {
        delete default_hidlist;
        delete default_keywords;
        delete default_helptexts;
        if (default_doc) xmlFreeDoc(default_doc);
    }
    void dropappl()
    {
        delete appl_hidlist;
        delete appl_keywords;
        delete appl_helptexts;
        if (appl_doc) xmlFreeDoc(appl_doc);
    }
    ~StreamTable()
    {
        dropappl();
        dropdefault();
    }
};

struct HelpProcessingException
{
    HelpProcessingErrorClass        m_eErrorClass;
    std::string                     m_aErrorMsg;
    std::string                     m_aXMLParsingFile;
    int                             m_nXMLParsingLine;

    HelpProcessingException( HelpProcessingErrorClass eErrorClass, const std::string& aErrorMsg )
        : m_eErrorClass( eErrorClass )
        , m_aErrorMsg( aErrorMsg )
    {}
    HelpProcessingException( const std::string& aErrorMsg, const std::string& aXMLParsingFile, int nXMLParsingLine )
        : m_eErrorClass( HELPPROCESSING_XMLPARSING_ERROR )
        , m_aErrorMsg( aErrorMsg )
        , m_aXMLParsingFile( aXMLParsingFile )
        , m_nXMLParsingLine( nXMLParsingLine )
    {}
};

class HelpCompiler
{
public:
    HelpCompiler(StreamTable &streamTable,
                const fs::path &in_inputFile,
                const fs::path &in_src,
                const fs::path &in_resEmbStylesheet,
                const std::string &in_module,
                const std::string &in_lang,
                bool in_bExtensionMode);
    bool compile( void ) throw (HelpProcessingException);
    void addEntryToJarFile(const std::string &prefix,
        const std::string &entryName, const std::string &bytesToAdd);
    void addEntryToJarFile(const std::string &prefix,
                const std::string &entryName, const HashSet &bytesToAdd);
    void addEntryToJarFile(const std::string &prefix,
                const std::string &entryName, const Stringtable &bytesToAdd);
    void addEntryToJarFile(const std::string &prefix,
                const std::string &entryName, const Hashtable &bytesToAdd);
private:
    xmlDocPtr getSourceDocument(const fs::path &filePath);
    HashSet switchFind(xmlDocPtr doc);
    xmlNodePtr clone(xmlNodePtr node, const std::string& appl);
    StreamTable &streamTable;
    const fs::path inputFile, src;
    const std::string module, lang;
    const fs::path resEmbStylesheet;
    bool bExtensionMode;
};

#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
