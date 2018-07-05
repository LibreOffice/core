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

#ifndef INCLUDED_HELPCOMPILER_INC_HELPCOMPILER_HXX
#define INCLUDED_HELPCOMPILER_INC_HELPCOMPILER_HXX

#include <sal/config.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>

#include <rtl/ustring.hxx>
#include <rtl/character.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include "BasCodeTagger.hxx"
#include <helpcompiler/compilehelp.hxx>

#if OSL_DEBUG_LEVEL > 2
    #include <iostream>
    #define HCDBG(foo) do { if (true) foo; } while(false)
#else
    #define HCDBG(foo) do { } while(false)
#endif

namespace fs
{
    rtl_TextEncoding getThreadTextEncoding();

    enum convert { native };
    class path
    {
    public:
        OUString data;
    public:
        path() {}
        path(const std::string &in, convert)
        {
            OUString sWorkingDir;
            osl_getProcessWorkingDir(&sWorkingDir.pData);
            OString tmp(in.c_str());
            OUString ustrSystemPath(OStringToOUString(tmp, getThreadTextEncoding()));
            osl::File::getFileURLFromSystemPath(ustrSystemPath, data);
            osl::File::getAbsoluteFileURL(sWorkingDir, data, data);
        }
        path(const std::string &FileURL)
        {
            OString tmp(FileURL.c_str());
            data = OStringToOUString(tmp, getThreadTextEncoding());
        }
        std::string native_file_string() const
        {
            OUString ustrSystemPath;
            osl::File::getSystemPathFromFileURL(data, ustrSystemPath);
            OString tmp(OUStringToOString(ustrSystemPath, getThreadTextEncoding()));
            HCDBG(std::cerr << "native_file_string is " << tmp.getStr() << std::endl);
            return std::string(tmp.getStr());
        }
#ifdef _WIN32
        std::wstring native_file_string_w() const
        {
            OUString ustrSystemPath;
            osl::File::getSystemPathFromFileURL(data, ustrSystemPath);
            return std::wstring(o3tl::toW(ustrSystemPath.getStr()));
        }
#endif
        std::string toUTF8() const
        {
            OString tmp(OUStringToOString(data, RTL_TEXTENCODING_UTF8));
            return std::string(tmp.getStr());
        }
        bool empty() const { return data.isEmpty(); }
        path operator/(const std::string &in) const
        {
            path ret(*this);
            HCDBG(std::cerr << "orig was " <<
                OUStringToOString(ret.data, RTL_TEXTENCODING_UTF8).getStr() << std::endl);
            OString tmp(in.c_str());
            OUString ustrSystemPath(OStringToOUString(tmp, getThreadTextEncoding()));
            ret.data += "/" + ustrSystemPath;
            HCDBG(std::cerr << "final is " <<
                OUStringToOString(ret.data, RTL_TEXTENCODING_UTF8).getStr() << std::endl);
            return ret;
        }
        void append(const char *in)
        {
            OString tmp(in);
            OUString ustrSystemPath(OStringToOUString(tmp, getThreadTextEncoding()));
            data = data + ustrSystemPath;
        }
        void append(const std::string &in) { append(in.c_str()); }
    };

    void create_directory(const fs::path& indexDirName);
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

typedef std::unordered_map<std::string, std::string, pref_hash> Stringtable;
typedef std::deque<std::string> LinkedList;

typedef std::unordered_map<std::string, LinkedList, pref_hash> Hashtable;

class StreamTable
{
public:
    std::string document_id;
    std::string document_path;
    std::string document_module;
    std::string document_title;

    std::unique_ptr< std::vector<std::string> > appl_hidlist;
    std::unique_ptr<Hashtable> appl_keywords;
    std::unique_ptr<Stringtable> appl_helptexts;
    xmlDocPtr appl_doc;

    StreamTable() :
        appl_doc(nullptr)
    {}
    void dropappl()
    {
        appl_hidlist.reset();
        appl_keywords.reset();
        appl_helptexts.reset();
        if (appl_doc) xmlFreeDoc(appl_doc);
    }
    ~StreamTable()
    {
        dropappl();
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
        : m_eErrorClass( HelpProcessingErrorClass::XmlParsing )
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
                const fs::path &in_zipdir,
                const fs::path &in_resCompactStylesheet,
                const fs::path &in_resEmbStylesheet,
                const std::string &in_module,
                const std::string &in_lang,
                bool in_bExtensionMode);
    /// @throws HelpProcessingException
    /// @throws BasicCodeTagger::TaggerException
    bool compile();
private:
    xmlDocPtr getSourceDocument(const fs::path &filePath);
    static void tagBasicCodeExamples(xmlDocPtr doc);
    xmlDocPtr compactXhpForJar(xmlDocPtr doc);
    void saveXhpForJar(xmlDocPtr doc, const fs::path &filePath);
    xmlNodePtr clone(xmlNodePtr node, const std::string& appl);
    StreamTable &streamTable;
    const fs::path inputFile, src, zipdir;
    const std::string module, lang;
    const fs::path resCompactStylesheet;
    const fs::path resEmbStylesheet;
    bool bExtensionMode;
    std::string gui;
};

inline char tocharlower(char c)
{
    return static_cast<char>(
        rtl::toAsciiLowerCase(static_cast<unsigned char>(c)));
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
