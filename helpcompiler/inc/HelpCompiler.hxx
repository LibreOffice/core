/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef HELPCOMPILER_HXX
#define HELPCOMPILER_HXX

#include <string>
#ifndef HAVE_CXX0X
#define BOOST_NO_0X_HDR_TYPEINDEX
#endif
#include <boost/unordered_map.hpp>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctype.h>

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

#include <helpcompiler/compilehelp.hxx>

#if OSL_DEBUG_LEVEL > 2
    #include <iostream>
    #define HCDBG(foo) do { if (1) foo; } while(0)
#else
    #define HCDBG(foo) do { } while(0)
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
#ifdef WNT
        wchar_t const * native_file_string_w() const
        {
            ::rtl::OUString ustrSystemPath;
            osl::File::getSystemPathFromFileURL(data, ustrSystemPath);
            return (wchar_t const *) ustrSystemPath.getStr();
        }
#endif
        std::string native_directory_string() const { return native_file_string(); }
        std::string toUTF8() const
        {
            rtl::OString tmp(rtl::OUStringToOString(data, RTL_TEXTENCODING_UTF8));
            return std::string(tmp.getStr());
        }
        bool empty() const { return data.isEmpty(); }
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
    void copy(const fs::path &src, const fs::path &dest);
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

#define pref_hash joaat_hash

typedef boost::unordered_map<std::string, std::string, pref_hash> Stringtable;
typedef std::list<std::string> LinkedList;
typedef std::vector<std::string> HashSet;

typedef boost::unordered_map<std::string, LinkedList, pref_hash> Hashtable;

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
        , m_nXMLParsingLine( 0 )
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
    xmlNodePtr clone(xmlNodePtr node, const std::string& appl);
    StreamTable &streamTable;
    const fs::path inputFile, src;
    const std::string module, lang;
    const fs::path resEmbStylesheet;
    bool bExtensionMode;
    std::string gui;
};

inline char tocharlower(char c)
{
    return static_cast<char>(tolower(c));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
