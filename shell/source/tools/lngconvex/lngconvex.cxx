// Test_Stl_Stream.cpp : Definiert den Einsprungpunkt für die Konsolenanwendung.
//

//#define WIN32_LEAN_AND_MEAN

#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>

#define VCL_NEED_BASETSD

#include <iostream>
#include <fstream>
#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>
#include <map>
#include <sstream>
#include <iterator>
#include <algorithm>

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _TOOLS_CONFIG_HXX_
#include <tools/config.hxx>
#endif

#ifndef _TOOLS_L2TXTENC_HXX_
#include <tools/l2txtenc.hxx>
#endif

#ifndef _CMDLINE_HXX_
#include "cmdline.hxx"
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

namespace /* private */
{

//-------------------------------
/** Various constants
*/
const std::string RC_PLACEHOLDER_LANGUAGE    = "%LANGUAGE%";
const std::string RC_PLACEHOLDER_SUBLANGUAGE = "%SUBLANGUAGE%";


//####################################


typedef int CountryIdentifier_t;

typedef std::vector<CountryIdentifier_t> CountryIdentifierContainer_t;

CountryIdentifierContainer_t g_country_lng_file_countries;
CountryIdentifierContainer_t g_lng_file_countries;

// we support only this languages that are
// in the ctrylng file as well as appearing
// in the lng file
CountryIdentifierContainer_t g_country_intersection;

//------------------------------------
// Helper function
//------------------------------------


void ShowUsage()
{
    std::cout << "Usage: -lng lng_file -rc rc_output_file -c country_language_file -rct rc_template_file" << std::endl;
    std::cout << "-lng Name of the lng file" << std::endl;
    std::cout << "-rc Name of the resulting resource file" << std::endl;
    std::cout << "-c Name of the country_language file" << std::endl;
    std::cout << "-rct Name of the resource template file" << std::endl;
    std::cout << "-rch Name of the resource file header" << std::endl;
    std::cout << "-rcf Name of the resource file footer" << std::endl;
}

//-------------------------------------------
/** Get the directory where the module
    is located as system directory,
    the returned directory has a trailing '\'
*/

rtl::OUString GetModuleDirectory()
{
    rtl::OUString cwd_url;
    rtl::OUString module_dir;

    if (osl_Process_E_None == osl_getProcessWorkingDir(&cwd_url.pData))
        osl::FileBase::getSystemPathFromFileURL(cwd_url, module_dir);

    return module_dir;
}

//-------------------------------------------
/** Make the absolute directory of a base and
    a relative directory,
    if the relative directory is absolute the
    the relative directory will be returned
    unchanged.
    Base and relative directory should be
    system paths the returned directory is
    a system path too
*/
rtl::OUString GetAbsoluteDirectory(
    const rtl::OUString& BaseDir, const rtl::OUString& RelDir)
{
    rtl::OUString base_url;
    rtl::OUString rel_url;

    osl::FileBase::getFileURLFromSystemPath(BaseDir, base_url);
    osl::FileBase::getFileURLFromSystemPath(RelDir, rel_url);

    rtl::OUString abs_url;
    osl::FileBase::getAbsoluteFileURL(base_url, rel_url, abs_url);

    rtl::OUString abs_sys_path;
    osl::FileBase::getSystemPathFromFileURL(abs_url, abs_sys_path);

    return abs_sys_path;
}

//--------------------------
/**
*/
rtl::OUString OStr2OUStr(const rtl::OString& aOStr)
{
    return rtl::OStringToOUString(aOStr, osl_getThreadTextEncoding());
}


//--------------------------
/**
*/
rtl::OString OUStr2OStr(const rtl::OUString& aOUStr)
{
    return rtl::OUStringToOString(aOUStr, osl_getThreadTextEncoding());
}

//####################################

//-------------------------------------------
/** A helper class, enables stream exceptions
    on construction, restors the old exception
    state on destruction
*/
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


//####################################


//-----------------------------
/** A line struct (in order to
    define line reading
    operators for file streams)
*/
struct Line
{
    std::string m_Line;
};

typedef std::vector<Line> LineBuffer_t;

//-----------------------------
/** A line input operator
*/
template<class charT, class traits>
std::basic_istream<charT,traits>& operator>>(
    std::basic_istream<charT,traits>& strm, Line& l)
{
    // reset line
    l.m_Line = std::string();

    char chr;
    while(strm.get(chr))
    {
        if (chr != '\n')
            l.m_Line.push_back(chr);
        else
            break;
    }

    return strm;
}

//-----------------------------
/** A line output operator
*/
template<class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(
    std::basic_ostream<charT, traits>& os, const Line& l)
{
    os << l.m_Line;
    return os;
}


//#####################################


//------------------------------------
// global variables
//------------------------------------

LineBuffer_t g_resource_template;


//#####################################


//-------------------------------------
/** A replacement table contains pairs
    of placeholders and the appropriate
    substitute
*/
class Substitutor
{
private:
    typedef std::map<std::string, std::string> ReplacementTable_t;
    typedef std::map<CountryIdentifier_t, ReplacementTable_t*> CountryReplacementTable_t;

public:
    typedef CountryReplacementTable_t::iterator iterator;
    typedef CountryReplacementTable_t::const_iterator const_iterator;

public:

    /** ctor
    */
    Substitutor() :
        m_ActiveCountry(0)
    {};

    explicit Substitutor(CountryIdentifier_t ActiveCountry) :
        m_ActiveCountry(ActiveCountry)
    {};

    /** dtor
    */
    ~Substitutor()
    {
        CountryReplacementTable_t::iterator iter_end = m_CountryReplacementTbl.end();
        CountryReplacementTable_t::iterator iter = m_CountryReplacementTbl.begin();

        for( /* no init */; iter != iter_end; ++iter)
            delete iter->second;

        m_CountryReplacementTbl.clear();
    }

    /**
    */
    void SetActiveCountry(CountryIdentifier_t CtryId)
    {
        m_ActiveCountry = CtryId;
    }

    /**
    */
    CountryIdentifier_t GetActiveCountry() const
    {
        return m_ActiveCountry;
    }

    /** If Text is a placeholder substitute it with
        its substitute else leave it unchanged
    */
    void Substitute(std::string& Text)
    {
        ReplacementTable_t* prt = GetReplacementTable(m_ActiveCountry);
        assert(prt);
        ReplacementTable_t::iterator iter = prt->find(Text);

        if (iter != prt->end())
            Text = iter->second;
    }

    /** Add a new substitution
    */
    void AddSubstitution(
        const std::string& Placeholder, const std::string& Substitute)
    {
        ReplacementTable_t* prt = GetReplacementTable(m_ActiveCountry);
        assert(prt);
        prt->insert(std::make_pair(Placeholder, Substitute));
    }

private:

    /** Return the replacement table for the country id
        create a new one if not already present
    */
    ReplacementTable_t* GetReplacementTable(const CountryIdentifier_t& CountryId)
    {
        CountryReplacementTable_t::iterator iter = m_CountryReplacementTbl.find(CountryId);

        ReplacementTable_t* prt = 0;

        if (m_CountryReplacementTbl.end() == iter)
        {
            prt = new ReplacementTable_t();
            m_CountryReplacementTbl.insert(std::make_pair(CountryId, prt));
        }
        else
        {
            prt = iter->second;
        }

        return prt;
    }

private:
    CountryReplacementTable_t m_CountryReplacementTbl;
    CountryIdentifier_t m_ActiveCountry;
};


//-----------------------------------------------------
/*  LangSubLangPair_t contains the Languages and the
    SubLanguage defines as string (see MS Platform SDK
    winnt.h), e.g LANG_GERMAN, SUBLANG_GERMAN

    CountryLanguageMap_t associates country identifiers
    (the country phone preselection) and the appropriate
    LangSubLangPair_t's
*/
typedef std::pair<std::string, std::string> LangSubLangPair_t;
typedef std::pair<CountryIdentifier_t, LangSubLangPair_t> CountryLanguagePair_t;

//-------------------------------
/** A stream input operator for
    country language file lines
*/
template<class charT, class traits>
std::basic_istream<charT,traits>& operator>>(
    std::basic_istream<charT,traits>& strm, CountryLanguagePair_t& cl_pair)
{
    strm >> cl_pair.first;
    strm >> cl_pair.second.first;
    strm >> cl_pair.second.second;

    return strm;
}

//--------------------------------
/** Read the country language file
    and add the entries to the
    substitutor

    The format of the country
    language file is:

    Country Identifier LANG_??? SUBLANG_???
    ...

    e.g.

    01 LANG_ENGLISH SUBLANG_ENGLISH_US
    49 LANG_GERMAN SUBLANG_GERMAN
    ...

    Will not detect if the entries of the
    file are valid.


    @param  is
    a file stream

    @param substitutor
    the substitutor

    @precond the file stream is open
    and the file is in a proper format

    @throws std::ios::failure
    if the file could not be read at all
    or the format of the file is invalid
*/

void ReadCtryLngFile(
    std::ifstream& is, Substitutor& substitutor, CountryIdentifierContainer_t& CtryContainer)
{
    CountryLanguagePair_t cl_pair;

    StreamExceptionsEnabler sexc(is);

    try
    {
        while (is >> cl_pair)
        {
            CtryContainer.push_back(cl_pair.first);

            substitutor.SetActiveCountry(cl_pair.first);

            substitutor.AddSubstitution(
                RC_PLACEHOLDER_LANGUAGE,
                cl_pair.second.first);

            substitutor.AddSubstitution(
                RC_PLACEHOLDER_SUBLANGUAGE,
                cl_pair.second.second);
        }
    }
    catch(const std::ios::failure&)
    {
        if (!is.eof())
            throw;
    }

    // sort the country container
    std::sort(CtryContainer.begin(), CtryContainer.end());
}


//####################################


//-----------------------------------
/** Convert a OUString to the MS
    resource file format string
    e.g.
    OUString -> L"\x1A00\x2200\x3400"
*/
std::string OUStringToWinResourceString(const rtl::OUString& aString)
{
    std::ostringstream oss;

    oss << "L\"";

    size_t length = aString.getLength();
    const sal_Unicode* pchr = aString.getStr();

    for (size_t i = 0; i < length; i++)
        oss << "\\x" << std::hex << (int)*pchr++;

    oss << "\"";

    return oss.str();
}

//-------------------------------
/**
*/
void AddGroupEntriesToSubstitutor(
    Config& aConfig, const ByteString& GroupName, Substitutor& Substitutor, CountryIdentifierContainer_t& CtryContainer)
{
    // precondition
    assert(aConfig.HasGroup(GroupName));

    aConfig.SetGroup(GroupName);

    size_t key_count = aConfig.GetKeyCount();

    for (size_t i = 0; i < key_count; i++)
    {
        ByteString key = aConfig.GetKeyName(i);

        bool is_numeric_key = key.IsNumericAscii();

        assert(is_numeric_key);

        if (is_numeric_key)
        {
            CountryIdentifier_t country_id = key.ToInt32();

            CtryContainer.push_back(country_id);

            Substitutor.SetActiveCountry(country_id);

            ByteString key_value = aConfig.ReadKey(i);

            key_value.EraseLeadingAndTrailingChars('\"');

            rtl::OUString key_value_unicode =
                rtl::OStringToOUString(key_value, Langcode2TextEncoding(country_id));

            assert(key_value_unicode.getLength());

            Substitutor.AddSubstitution(
                GroupName.GetBuffer(),
                OUStringToWinResourceString(key_value_unicode));
        }
    }
}

//-------------------------------
/** Read the lng file ...
*/
void ReadLngFile(
    const std::string& FileName, Substitutor& Substitutor, CountryIdentifierContainer_t& CtryContainer)
{
    Config config(
        rtl::OStringToOUString(FileName.c_str(), osl_getThreadTextEncoding()).getStr());

    size_t group_count = config.GetGroupCount();

    for (size_t i = 0; i < group_count; i++)
    {
        AddGroupEntriesToSubstitutor(
            config, config.GetGroupName(i), Substitutor, CtryContainer);
    }

    // sort the country container
    std::sort(CtryContainer.begin(), CtryContainer.end());
}

//-------------------------------
/** Read the content of a file
    into a line buffer
*/
void ReadFile(const std::string& FileName, LineBuffer_t& LineBuffer)
{
    std::ifstream file(FileName.c_str());
    StreamExceptionsEnabler sexc(file);

    try
    {
        Line line;
        while (file >> line)
            LineBuffer.push_back(line);
    }
    catch(const std::ios::failure&)
    {
        if (!file.eof())
            throw;
    }
}

//-------------------------------
/** A simple helper function that
    appens the content of one
    file to another one

    @throws std::ios::failure
    if a read or write error
    occurs
*/
void FileAppendFile(std::ostream& os, std::istream& is)
{
    StreamExceptionsEnabler os_sexc(os);
    StreamExceptionsEnabler is_sexc(is);

    try
    {
        std::istream_iterator<Line> ii(is);
        std::istream_iterator<Line> eos;

        std::ostream_iterator<Line> oi(os, "\n");

        std::copy(ii, eos, oi);
    }
    catch(const std::ios::failure&)
    {
        if (!is.eof())
            throw;
    }
}

//----------------------------
/** Helper functions for docu
    purposes
*/

inline void ReadResourceTemplate(const std::string& FileName, LineBuffer_t& LineBuffer)
{
    ReadFile(FileName, LineBuffer);
}

inline void FileAppendRcHeader(std::ostream& os, std::istream& is)
{
    FileAppendFile(os, is);
}

inline void FileAppendRcFooter(std::ostream& os, std::istream& is)
{
    FileAppendFile(os, is);
}

//-------------------------------------------
/** Iterate all languages in the substitutor,
    replace the all placeholder and append the
    result to the output file

    @throws std::ios::failure
    on write errors
*/
void InflateRcTemplateAndAppendToFile(
    std::ostream& os, const LineBuffer_t& RcTemplate, Substitutor& substitutor, const CountryIdentifierContainer_t& Countries)
{
    StreamExceptionsEnabler sexc(os);

    CountryIdentifierContainer_t::const_iterator iter = Countries.begin();
    CountryIdentifierContainer_t::const_iterator iter_end = Countries.end();

    std::ostream_iterator<std::string> oi(os, "\n");

    for (/**/ ;iter != iter_end; ++iter)
    {
        substitutor.SetActiveCountry(*iter);

        LineBuffer_t::const_iterator rct_iter    = RcTemplate.begin();
        LineBuffer_t::const_iterator rct_iter_end = RcTemplate.end();

        for (/**/ ;rct_iter != rct_iter_end; ++rct_iter)
        {
            std::istringstream iss = rct_iter->m_Line;
            std::string line;

            while (iss)
            {
                std::string token;
                iss >> token;
                substitutor.Substitute(token);
                line += token;
                line += " ";
            }

            oi = line;
        }
    }
}

//--------------------------------
/** It is only usefull to generate
    resources for languages that
    are in the lng file and in the
    country-language file, that's
    why we make the intersection
    of the both country lists
*/
void MakeUniqueIntersection(
    const CountryIdentifierContainer_t& CtryContainer1,
    const CountryIdentifierContainer_t& CtryContainer2,
    CountryIdentifierContainer_t& CtryContainer)
{
    std::back_insert_iterator<CountryIdentifierContainer_t> bii(CtryContainer);

    std::set_intersection(
        CtryContainer1.begin(),
        CtryContainer1.end(),
        CtryContainer2.begin(),
        CtryContainer2.end(),
        bii);

    std::unique(CtryContainer.begin(), CtryContainer.end());
}

} // namespace /* private */


//####################################


//-------------------------------
/** Main

    The file names provided via
    command line should be absolute
    or relative to the directory
    of this module
*/
int main(int argc, char* argv[])
{
    try
    {
        CommandLine cmdline(argc, argv);
        Substitutor substitutor;

        rtl::OUString module_dir = GetModuleDirectory();

        rtl::OUString oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-c").c_str()));

        ReadCtryLngFile(
            std::ifstream(OUStr2OStr(oustr_abs_name).getStr()),
            substitutor,
            g_country_lng_file_countries);

        oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-lng").c_str()));

        ReadLngFile(
            OUStr2OStr(oustr_abs_name).getStr(),
            substitutor,
            g_lng_file_countries);

        MakeUniqueIntersection(
            g_country_lng_file_countries,
            g_lng_file_countries,
            g_country_intersection);

        oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-rct").c_str()));

        ReadResourceTemplate(
            OUStr2OStr(oustr_abs_name).getStr(),
            g_resource_template);

        oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-rc").c_str()));

        std::ofstream rc_output_file(OUStr2OStr(oustr_abs_name).getStr());

        oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-rch").c_str()));

        FileAppendRcHeader(
            rc_output_file,
            std::ifstream(OUStr2OStr(oustr_abs_name).getStr()));

        InflateRcTemplateAndAppendToFile(
            rc_output_file,
            g_resource_template,
            substitutor,
            g_country_intersection);

        oustr_abs_name = GetAbsoluteDirectory(
            module_dir, OStr2OUStr(cmdline.GetArgument("-rcf").c_str()));

        FileAppendRcFooter(
            rc_output_file,
            std::ifstream(OUStr2OStr(oustr_abs_name).getStr()));

    }
    catch(const std::ios::failure& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        ShowUsage();
    }
    catch(...)
    {
        std::cout << "Unexpected error..." << std::endl;
    }

    return 0;
}

