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

#ifdef AIX
#    undef _THREAD_SAFE
#endif


#ifdef WNT
#include <tools/presys.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tools/postsys.h>
#else
// From MinGW
typedef unsigned short WORD;
#define PRIMARYLANGID(lgid) ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid) ((WORD)(lgid) >> 10)
#define LANG_SPANISH 0x0a
#define SUBLANG_NEUTRAL 0x00
#define SUBLANG_SPANISH 0x01
#endif

#include "cmdline.hxx"

#include <comphelper/string.hxx>
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/file.hxx"
#include "sal/main.h"

#include "tools/config.hxx"
#include "i18nlangtag/languagetag.hxx"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <string>

#ifndef WNT
#include <cstring>
#endif

namespace /* private */
{


void ShowUsage()
{
    std::cout << "Usage: -ulf ulf_file -rc rc_output_file -rct rc_template_file -rch rch_file -rcf rcf_file" << std::endl;
    std::cout << "-ulf Name of the ulf file" << std::endl;
    std::cout << "-rc  Name of the resulting resource file" << std::endl;
    std::cout << "-rct Name of the resource template file" << std::endl;
    std::cout << "-rch Name of the resource file header" << std::endl;
    std::cout << "-rcf Name of the resource file footer" << std::endl;
}

inline OUString OStringToOUString(const OString& str)
{ return OStringToOUString(str, osl_getThreadTextEncoding()); }

inline OString OUStringToOString(const OUString& str)
{ return OUStringToOString(str, osl_getThreadTextEncoding()); }

/** Get the directory where the module
    is located as system directory, the
    returned directory has a trailing '\'  */
OUString get_module_path()
{
    OUString cwd_url;
    OUString module_path;
    if (osl_Process_E_None == osl_getProcessWorkingDir(&cwd_url.pData))
        osl::FileBase::getSystemPathFromFileURL(cwd_url, module_path);

    return module_path;
}

/** Make the absolute directory of a base and
    a relative directory, if the relative
    directory is absolute the relative
    directory will be returned unchanged.
    Base and relative directory should be
    system paths the returned directory is
    a system path too */
OUString get_absolute_path(
    const OUString& BaseDir, const OUString& RelDir)
{
    OUString base_url;
    OUString rel_url;

    osl::FileBase::getFileURLFromSystemPath(BaseDir, base_url);
    osl::FileBase::getFileURLFromSystemPath(RelDir, rel_url);

    OUString abs_url;
    osl::FileBase::getAbsoluteFileURL(base_url, rel_url, abs_url);

    OUString abs_sys_path;
    osl::FileBase::getSystemPathFromFileURL(abs_url, abs_sys_path);

    return abs_sys_path;
}

OString get_absolute_file_path(const std::string& file_name)
{
    OUString fp = get_absolute_path(
        get_module_path(), OStringToOUString(file_name.c_str()));
    return OUStringToOString(fp);
}

/** A helper class, enables stream exceptions
    on construction, restors the old exception
    state on destruction */
class StreamExceptionsEnabler
{
public:
    explicit StreamExceptionsEnabler(
        std::ios& iostrm,
        std::ios::iostate NewIos = std::ios::failbit | std::ios::badbit) :
        m_IoStrm(iostrm),
        m_OldIos(m_IoStrm.exceptions())
    {
        m_IoStrm.exceptions(NewIos);
    }

    ~StreamExceptionsEnabler()
    {
        m_IoStrm.exceptions(m_OldIos);
    }
private:
    std::ios& m_IoStrm;
    std::ios::iostate m_OldIos;
};

typedef std::vector<std::string> string_container_t;

class iso_lang_identifier
{
public:
    iso_lang_identifier() {};

    iso_lang_identifier(const OString& str) :
        lang_(str)
    { init(); }

    iso_lang_identifier(const std::string& str) :
        lang_(str.c_str())
    { init(); }

    OString language() const
    { return lang_; }

    OString country() const
    { return country_; }

    OString make_OString() const
    { return lang_ + "-" + country_; }

    std::string make_std_string() const
    {
        OString tmp(lang_ + "-" + country_);
        return tmp.getStr();
    }

private:
    void init()
    {
        sal_Int32 idx = lang_.indexOf('-');

        if (idx > -1)
        {
            country_ = lang_.copy(idx + 1);
            lang_ = lang_.copy(0, idx);
        }
    }

private:
    OString lang_;
    OString country_;
};

/** Convert a OUString to the MS resource
    file format string e.g.
    OUString -> L"\x1A00\x2200\x3400" */
std::string make_winrc_unicode_string(const OUString& str)
{
    std::ostringstream oss;
    oss << "L\"";

    size_t length = str.getLength();
    const sal_Unicode* pchr = str.getStr();

    for (size_t i = 0; i < length; i++)
        oss << "\\x" << std::hex << (int)*pchr++;

    oss << "\"";
    return oss.str();
}

std::string make_winrc_unicode_string(const std::string& str)
{
    return make_winrc_unicode_string(
        OUString::createFromAscii(str.c_str()));
}

/** A replacement table contains pairs of
    placeholders and the appropriate substitute */
class Substitutor
{
private:
    typedef std::map<std::string, std::string> replacement_table_t;
    typedef std::map<std::string, replacement_table_t*> iso_lang_replacement_table_t;

public:
    typedef iso_lang_replacement_table_t::iterator iterator;
    typedef iso_lang_replacement_table_t::const_iterator const_iterator;

    iterator begin()
    { return iso_lang_replacement_table_.begin(); }

    iterator end()
    { return iso_lang_replacement_table_.end(); }

public:

    Substitutor() {};

    ~Substitutor()
    {
        iso_lang_replacement_table_t::iterator iter_end = iso_lang_replacement_table_.end();
        iso_lang_replacement_table_t::iterator iter = iso_lang_replacement_table_.begin();

        for( /* no init */; iter != iter_end; ++iter)
            delete iter->second;

        iso_lang_replacement_table_.clear();
    }

    void set_language(const iso_lang_identifier& iso_lang)
    {
        active_iso_lang_ = iso_lang;
    }

    // If Text is a placeholder substitute it with
    //its substitute else leave it unchanged
    void substitute(std::string& Text)
    {
        replacement_table_t* prt = get_replacement_table(active_iso_lang_.make_std_string());
        OSL_ASSERT(prt);
        replacement_table_t::iterator iter = prt->find(Text);
        if (iter != prt->end())
            Text = iter->second;
    }

    void add_substitution(
        const std::string& Placeholder, const std::string& Substitute)
    {
        replacement_table_t* prt = get_replacement_table(active_iso_lang_.make_std_string());
        OSL_ASSERT(prt);
        prt->insert(std::make_pair(Placeholder, Substitute));
    }


private:
    // Return the replacement table for the iso lang id
    // create a new one if not already present
    replacement_table_t* get_replacement_table(const std::string& iso_lang)
    {
        iso_lang_replacement_table_t::iterator iter =
            iso_lang_replacement_table_.find(iso_lang);

        replacement_table_t* prt = NULL;

        if (iso_lang_replacement_table_.end() == iter)
        {
            prt = new replacement_table_t();
            iso_lang_replacement_table_.insert(std::make_pair(iso_lang, prt));
        }
        else
        {
            prt = iter->second;
        }
        return prt;
    }

private:
    iso_lang_replacement_table_t iso_lang_replacement_table_;
    iso_lang_identifier active_iso_lang_;
};

typedef std::map< unsigned short , std::string , std::less< unsigned short > > shortmap;

void add_group_entries(
    Config& aConfig,
    const OString& GroupName,
    Substitutor& Substitutor)
{
    OSL_ASSERT(aConfig.HasGroup(GroupName));

    aConfig.SetGroup(GroupName);
    size_t key_count = aConfig.GetKeyCount();
    shortmap map;

    for (size_t i = 0; i < key_count; i++)
    {
        OString iso_lang = aConfig.GetKeyName(sal::static_int_cast<sal_uInt16>(i));
        OString key_value_utf8 = aConfig.ReadKey(sal::static_int_cast<sal_uInt16>(i));
        iso_lang_identifier myiso_lang( iso_lang );
        LanguageType ltype = LanguageTag(
                OStringToOUString( myiso_lang.language(), RTL_TEXTENCODING_UTF8),
                OStringToOUString( myiso_lang.country(), RTL_TEXTENCODING_UTF8)).makeFallback().getLanguageType();
        if(  ( ltype & 0x0200 ) == 0 && map[ ltype ].empty()  )
        {
            Substitutor.set_language(iso_lang_identifier(iso_lang));

            key_value_utf8 = comphelper::string::strip(key_value_utf8, '\"');

            OUString key_value_utf16 =
                OStringToOUString(key_value_utf8, RTL_TEXTENCODING_UTF8);

            Substitutor.add_substitution(
                GroupName.getStr(), make_winrc_unicode_string(key_value_utf16));
            map[ static_cast<unsigned short>(ltype) ] = std::string( iso_lang.getStr() );
        }
        else
        {
            if( !map[ ltype ].empty() )
            {
                printf("ERROR: Duplicated ms id %d found for the languages %s and %s !!!! This does not work in microsoft resources\nPlease remove one!\n", ltype , map[ ltype ].c_str() , iso_lang.getStr());
                exit( -1 );
            }
        }
    }
}

void read_ulf_file(const std::string& FileName, Substitutor& Substitutor)
{
    // work-around for #i32420#

    // as the Config class is currently not able to deal correctly with
    // UTF8 files starting with a byte-order-mark we create a copy of the
    // original file without the byte-order-mark
    OUString tmpfile_url;
    osl_createTempFile(NULL, NULL, &tmpfile_url.pData);

    OUString tmpfile_sys;
    osl::FileBase::getSystemPathFromFileURL(tmpfile_url, tmpfile_sys);

    std::ifstream in(FileName.c_str());
    std::ofstream out(OUStringToOString(tmpfile_sys).getStr());

    try
    {
        StreamExceptionsEnabler sexc_out(out);
        StreamExceptionsEnabler sexc_in(in);

        //skip the byte-order-mark 0xEF 0xBB 0xBF, identifying UTF8 files
        unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
        char buff[3];
        in.read(&buff[0], 3);

        if (memcmp(buff, BOM, 3) != 0)
            in.seekg(0);

        std::string line;
        while (std::getline(in, line))
            out << line << std::endl;
    }
    catch (const std::ios::failure&)
    {
        if (!in.eof())
            throw;
    }


    // end work-around for #i32420#

    Config config(tmpfile_url.getStr());
    size_t grpcnt = config.GetGroupCount();
    for (size_t i = 0; i < grpcnt; i++)
        add_group_entries(config, config.GetGroupName(sal::static_int_cast<sal_uInt16>(i)), Substitutor);
}

void read_file(
    const std::string& fname,
    string_container_t& string_container)
{
    std::ifstream file(fname.c_str());
    StreamExceptionsEnabler sexc(file);

    try
    {
        std::string line;
        while (std::getline(file, line))
            string_container.push_back(line);
    }
    catch(const std::ios::failure&)
    {
        if (!file.eof())
            throw;
    }
}

/** A simple helper function that appens the
    content of one file to another one  */
void concatenate_files(std::ostream& os, std::istream& is)
{
    StreamExceptionsEnabler os_sexc(os);
    StreamExceptionsEnabler is_sexc(is);

    try
    {
        std::string line;
        while (std::getline(is, line))
            os << line << std::endl;
    }
    catch(const std::ios::failure&)
    {
        if (!is.eof())
            throw;
    }
}

bool is_placeholder(const std::string& str)
{
    return ((str.length() > 1) &&
            ('%' == str[0]) &&
            ('%' == str[str.length() - 1]));
}

void start_language_section(
    std::ostream_iterator<std::string>& ostream_iter, const iso_lang_identifier& iso_lang)
{
    ostream_iter = std::string();

    std::string lang_section("LANGUAGE ");

    LanguageType ltype = LanguageTag(
            OStringToOUString( iso_lang.language(), RTL_TEXTENCODING_UTF8),
            OStringToOUString( iso_lang.country(), RTL_TEXTENCODING_UTF8)).makeFallback().getLanguageType();

    char buff[10];
    int primLangID = PRIMARYLANGID(ltype);
    int subLangID = SUBLANGID(ltype);
    // Our resources are normaly not sub language dependant.
    // Esp. for spanish we don't want to distinguish between trad.
    // and internatinal sorting ( which leads to two different sub languages )
    // Setting the sub language to neutral allows us to use one
    // stringlist for all spanish variants
    if ( ( primLangID == LANG_SPANISH ) &&
         ( subLangID == SUBLANG_SPANISH ) )
        subLangID = SUBLANG_NEUTRAL;

#ifdef WNT
    _itoa(primLangID, buff, 16);
#else
    sprintf(buff, "%x", primLangID);
#endif
    lang_section += std::string("0x") + std::string(buff);

    lang_section += std::string(" , ");

#ifdef WNT
    _itoa(subLangID, buff, 16);
#else
    sprintf(buff, "%x", subLangID);
#endif
    lang_section += std::string("0x") + std::string(buff);
    ostream_iter = lang_section;
}

/** Iterate all languages in the substitutor,
    replace the all placeholder and append the
    result to the output file */
void inflate_rc_template_to_file(
    std::ostream& os, const string_container_t& rctmpl, Substitutor& substitutor)
{
    StreamExceptionsEnabler sexc(os);

    Substitutor::const_iterator iter = substitutor.begin();
    Substitutor::const_iterator iter_end = substitutor.end();

    std::ostream_iterator<std::string> oi(os, "\n");

    for ( /**/ ;iter != iter_end; ++iter)
    {
        substitutor.set_language(iso_lang_identifier(iter->first));

        string_container_t::const_iterator rct_iter = rctmpl.begin();
        string_container_t::const_iterator rct_iter_end = rctmpl.end();

        if (!rctmpl.empty())
            start_language_section(oi, iter->first);

        for ( /**/ ;rct_iter != rct_iter_end; ++rct_iter)
        {
            std::istringstream iss(*rct_iter);
            std::string line;

            while (iss)
            {
                std::string token;
                iss >> token;
                substitutor.substitute(token);

                // HACK for partially merged
                // *.lng files where some strings have
                // a particular language that others
                // don't have in order to keep the
                // build
                if (is_placeholder(token))
                    token = make_winrc_unicode_string(token);

                line += token;
                line += " ";
            }
            oi = line;
        }
    }
}

} // namespace /* private */

/* MAIN
   The file names provided via command line should be
   absolute or relative to the directory of this module.

   Algo:
   1. read the ulf file and initialize the substitutor
   2. read the resource template file
   3. create the output file and append the header
   4. inflate the resource template to the output file
      for every language using the substitutor
   5. append the footer
*/
#define MAKE_ABSOLUTE(s) (get_absolute_file_path((s)).getStr())
#define ULF_FILE(c)    MAKE_ABSOLUTE((c).get_arg("-ulf"))
#define RC_TEMPLATE(c) MAKE_ABSOLUTE((c).get_arg("-rct"))
#define RC_FILE(c)     MAKE_ABSOLUTE((c).get_arg("-rc"))
#define RC_HEADER(c)   MAKE_ABSOLUTE((c).get_arg("-rch"))
#define RC_FOOTER(c)   MAKE_ABSOLUTE((c).get_arg("-rcf"))

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    try
    {
        CommandLine cmdline(argc, argv);

        Substitutor substitutor;
        read_ulf_file(ULF_FILE(cmdline), substitutor);

        string_container_t rc_tmpl;
        read_file(RC_TEMPLATE(cmdline), rc_tmpl);

        std::ofstream rc_file(RC_FILE(cmdline));
        std::ifstream in_header(RC_HEADER(cmdline));
        concatenate_files(rc_file, in_header);

        inflate_rc_template_to_file(rc_file, rc_tmpl, substitutor);

        std::ifstream in_footer(RC_FOOTER(cmdline));
        concatenate_files(rc_file, in_footer);
    }
    catch(const std::ios::failure& ex)
    {
        std::cout << ex.what() << std::endl;
        return 1;
    }
    catch(const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        ShowUsage();
        return 1;
    }
    catch(...)
    {
        std::cout << "Unexpected error..." << std::endl;
        return 1;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
