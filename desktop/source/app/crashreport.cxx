/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <desktop/crashreport.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <unotools/bootstrap.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <desktop/minidump.hxx>
#include <rtl/ustrbuf.hxx>

#include <config_version.h>
#include <config_folders.h>

#include <string>
#include <regex>


#if HAVE_FEATURE_BREAKPAD

#include <fstream>
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
#include <client/linux/handler/exception_handler.h>
#elif defined _WIN32
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#endif
#include <client/windows/handler/exception_handler.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif
#include <locale>
#include <codecvt>
#endif

osl::Mutex CrashReporter::maMutex;
std::unique_ptr<google_breakpad::ExceptionHandler> CrashReporter::mpExceptionHandler;
bool CrashReporter::mbInit = false;
CrashReporter::vmaKeyValues CrashReporter::maKeyValues;


#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* /*context*/, bool succeeded)
{
    CrashReporter::addKeyValue("DumpFile", OStringToOUString(descriptor.path(), RTL_TEXTENCODING_UTF8), CrashReporter::Write);
    SAL_WARN("desktop", "minidump generated: " << descriptor.path());

    return succeeded;
}
#elif defined _WIN32
static bool dumpCallback(const wchar_t* path, const wchar_t* id,
    void* /*context*/, EXCEPTION_POINTERS* /*exinfo*/,
    MDRawAssertionInfo* /*assertion*/,
    bool succeeded)
{
    // TODO: moggi: can we avoid this conversion
#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    std::string aPath = conv1.to_bytes(std::wstring(path)) + conv1.to_bytes(std::wstring(id)) + ".dmp";
    CrashReporter::addKeyValue("DumpFile", OStringToOUString(aPath.c_str(), RTL_TEXTENCODING_UTF8), CrashReporter::AddItem);
    CrashReporter::addKeyValue("GDIHandles", OUString::number(::GetGuiResources(::GetCurrentProcess(), GR_GDIOBJECTS)), CrashReporter::Write);
    SAL_WARN("desktop", "minidump generated: " << aPath);
    return succeeded;
}
#endif


void CrashReporter::writeToFile(std::ios_base::openmode Openmode)
{
    std::ofstream ini_file(getIniFileName(), Openmode);

    for (auto& keyValue : maKeyValues)
    {
        ini_file << OUStringToOString(keyValue.first, RTL_TEXTENCODING_UTF8) << "=";
        ini_file << OUStringToOString(keyValue.second, RTL_TEXTENCODING_UTF8) << "\n";
    }

    maKeyValues.clear();
    ini_file.close();
}

void CrashReporter::addKeyValue(const OUString& rKey, const OUString& rValue, tAddKeyHandling AddKeyHandling)
{
    osl::MutexGuard aGuard(maMutex);

    if (IsDumpEnable())
    {
        if (!rKey.isEmpty())
            maKeyValues.push_back(mpair(rKey, rValue));

        if (AddKeyHandling != AddItem)
        {
            if (mbInit)
                writeToFile(std::ios_base::app);
            else if (AddKeyHandling == Create)
                writeCommonInfo();
        }
    }
}

void CrashReporter::writeCommonInfo()
{
    writeSystemInfo();

    ucbhelper::InternetProxyDecider proxy_decider(::comphelper::getProcessComponentContext());

    const OUString protocol = "https";
    const OUString url = "crashreport.libreoffice.org";
    const sal_Int32 port = 443;

    const ucbhelper::InternetProxyServer proxy_server = proxy_decider.getProxy(protocol, url, port);

    // save the new Keys
    vmaKeyValues atlast = maKeyValues;
    // clear the keys, the following Keys should be at the begin
    maKeyValues.clear();

    // limit the amount of code that needs to be executed before the crash reporting
    addKeyValue("ProductName", "LibreOffice", AddItem);
    addKeyValue("Version", LIBO_VERSION_DOTTED, AddItem);
    addKeyValue("BuildID", utl::Bootstrap::getBuildIdData(""), AddItem);
    addKeyValue("URL", protocol + "://" + url + "/submit/", AddItem);

    if (proxy_server.aName != OUString())
    {
        addKeyValue("Proxy", proxy_server.aName + ":" + OUString::number(proxy_server.nPort), AddItem);
    }

    // write the new keys at the end
    maKeyValues.insert(maKeyValues.end(), atlast.begin(), atlast.end());

    mbInit = true;

    writeToFile(std::ios_base::trunc);

    updateMinidumpLocation();
}


namespace {

OUString getCrashDirectory()
{
    OUString aCrashURL;
    rtl::Bootstrap::get("CrashDirectory", aCrashURL);
    // Need to convert to URL in case of user-defined path
    osl::FileBase::getFileURLFromSystemPath(aCrashURL, aCrashURL);

    if (aCrashURL.isEmpty()) { // Fall back to user profile
        aCrashURL = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/crash/";
        rtl::Bootstrap::expandMacros(aCrashURL);
    }

    if (!aCrashURL.endsWith("/"))
        aCrashURL += "/";

    osl::Directory::create(aCrashURL);
    OUString aCrashPath;
    osl::FileBase::getSystemPathFromFileURL(aCrashURL, aCrashPath);
    return aCrashPath;
}

}

void CrashReporter::updateMinidumpLocation()
{
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    OUString aURL = getCrashDirectory();
    OString aOStringUrl = OUStringToOString(aURL, RTL_TEXTENCODING_UTF8);
    google_breakpad::MinidumpDescriptor descriptor(aOStringUrl.getStr());
    mpExceptionHandler->set_minidump_descriptor(descriptor);
#elif defined _WIN32
    OUString aURL = getCrashDirectory();
    mpExceptionHandler->set_dump_path(o3tl::toW(aURL.getStr()));
#endif
}

bool CrashReporter::crashReportInfoExists()
{
    static bool first = true;
    static bool InfoExist = false;

    if (first)
    {
        first = false;
        InfoExist = crashreport::readConfig(CrashReporter::getIniFileName(), nullptr);
    }

    return InfoExist;
}

bool CrashReporter::readSendConfig(std::string& response)
{
    return crashreport::readConfig(CrashReporter::getIniFileName(), &response);
}

void CrashReporter::installExceptionHandler()
{
    if (!IsDumpEnable())
        return;
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    google_breakpad::MinidumpDescriptor descriptor("/tmp");
    mpExceptionHandler = std::make_unique<google_breakpad::ExceptionHandler>(descriptor, nullptr, dumpCallback, nullptr, true, -1);
#elif defined _WIN32
    mpExceptionHandler = std::make_unique<google_breakpad::ExceptionHandler>(L".", nullptr, dumpCallback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif
}

void CrashReporter::removeExceptionHandler()
{
    mpExceptionHandler.reset();
}



bool CrashReporter::IsDumpEnable()
{
    OUString sToken;
    OString  sEnvVar(std::getenv("CRASH_DUMP_ENABLE"));
    bool     bEnable = true;   // default, always on
    // read configuration item 'CrashDumpEnable' -> bool on/off
    if (rtl::Bootstrap::get("CrashDumpEnable", sToken) && sEnvVar.isEmpty())
    {
        bEnable = sToken.toBoolean();
    }

    return bEnable;
}


std::string CrashReporter::getIniFileName()
{
    OUString url = getCrashDirectory() + "dump.ini";
    OString aUrl = OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    std::string aRet(aUrl.getStr());
    return aRet;
}

// Write system-specific information such as the CPU name and features.
// This may allow us to get some statistics for decisions (such as when
// deciding whether SSE2 can be made a hard-requirement for Windows).
// Breakpad provides this information poorly or not at all.
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
void CrashReporter::writeSystemInfo()
{
    // Get 'model name' and 'flags' from /proc/cpuinfo.
    if( std::ifstream cpuinfo( "/proc/cpuinfo" ); cpuinfo )
    {
        bool haveModel = false;
        bool haveFlags = false;
        std::regex modelRegex( "^model name[ \t]*:[ \t]*(.*)$" );
        std::regex flagsRegex( "^flags[ \t]*:[ \t]*(.*)$" );
        for( std::string line; std::getline( cpuinfo, line ); )
        {
            std::smatch match;
            if( !haveModel && std::regex_match( line, match, modelRegex ) && match.size() == 2)
            {
                addKeyValue("CPUModelName", OUString::fromUtf8( match[ 1 ].str()), AddItem);
                haveModel = true;
            }
            if( !haveFlags && std::regex_match( line, match, flagsRegex ) && match.size() == 2)
            {
                addKeyValue("CPUFlags", OUString::fromUtf8( match[ 1 ].str()), AddItem);
                haveFlags = true;
            }
            if( haveModel && haveFlags )
                break;
        }
    }
    // Get 'MemTotal' from /proc/meminfo.
    if( std::ifstream meminfo( "/proc/meminfo" ); meminfo )
    {
        std::regex memTotalRegex( "^MemTotal[ \t]*:[ \t]*(.*)$" );
        for( std::string line; std::getline( meminfo, line ); )
        {
            std::smatch match;
            if( std::regex_match( line, match, memTotalRegex ) && match.size() == 2)
            {
                addKeyValue("MemoryTotal", OUString::fromUtf8( match[ 1 ].str()), AddItem);
                break;
            }
        }
    }
}
#elif defined _WIN32
void CrashReporter::writeSystemInfo()
{
    // Get CPU model name and flags.
    // See https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
    // and https://en.wikipedia.org/wiki/CPUID .
    int cpui[ 4 ];
    __cpuid( cpui, 0x80000000 ); // Get the highest extended ID.
    int exIds = cpui[ 0 ];
    if( exIds >= 0x80000004 )
    {
        int brand[ 16 ];
        __cpuidex( brand, 0x80000002, 0 );
        __cpuidex( brand + 4, 0x80000003, 0 );
        __cpuidex( brand + 8, 0x80000004, 0 );
        brand[ 12 ] = 0;;
        addKeyValue( "CPUModelName", OUString::fromUtf8( reinterpret_cast< const char* >( brand )),
            AddItem );
    }
    __cpuid( cpui, 0 ); // Get the highest ID.
    int ids = cpui[ 0 ];
    unsigned int ecx1 = 0, edx1 = 0, ebx7 = 0, ecx7 = 0, ecx81 = 0, edx81 = 0;
    if( ids >= 0x1 )
    {
        __cpuidex( cpui, 0x1, 0 );
        ecx1 = cpui[ 2 ];
        edx1 = cpui[ 3 ];
    }
    if( ids >= 0x7 )
    {
        __cpuidex( cpui, 0x7, 0 );
        ebx7 = cpui[ 1 ];
        ecx7 = cpui[ 2 ];
    }
    if( exIds >= 0x80000001 )
    {
        __cpuidex( cpui, 0x80000001, 0 );
        ecx81 = cpui[ 2 ];
        edx81 = cpui[ 3 ];
    }
    struct FlagItem
    {
        unsigned int* reg;
        int bit;
        const char* name;
    };
    const FlagItem flagItems[] =
    {
        { &ecx1, 0, "sse3" },
        { &ecx1, 1, "pclmulqdq" },
        { &ecx1, 3, "monitor" },
        { &ecx1, 9, "ssse3" },
        { &ecx1, 12, "fma" },
        { &ecx1, 13, "cpmxch16b" },
        { &ecx1, 19, "sse41" },
        { &ecx1, 20, "sse42" },
        { &ecx1, 22, "movbe" },
        { &ecx1, 23, "popcnt" },
        { &ecx1, 25, "aes" },
        { &ecx1, 26, "xsave" },
        { &ecx1, 27, "osxsave" },
        { &ecx1, 28, "avx" },
        { &ecx1, 29, "f16c" },
        { &ecx1, 30, "rdrand" },
        { &edx1, 5, "msr" },
        { &edx1, 8, "cx8" },
        { &edx1, 11, "sep" },
        { &edx1, 15, "cmov" },
        { &edx1, 19, "clfsh" },
        { &edx1, 23, "mmx" },
        { &edx1, 24, "fxsr" },
        { &edx1, 25, "sse" },
        { &edx1, 26, "sse2" },
        { &edx1, 28, "ht" },
        { &ebx7, 0, "fsgsbase" },
        { &ebx7, 3, "bmi1" },
        { &ebx7, 4, "hle" },
        { &ebx7, 5, "avx2" },
        { &ebx7, 8, "bmi2" },
        { &ebx7, 9, "erms" },
        { &ebx7, 10, "invpcid" },
        { &ebx7, 11, "rtm" },
        { &ebx7, 16, "avx512f" },
        { &ebx7, 18, "rdseed" },
        { &ebx7, 19, "adx" },
        { &ebx7, 26, "avx512pf" },
        { &ebx7, 27, "avx512er" },
        { &ebx7, 28, "avx512cd" },
        { &ebx7, 29, "sha" },
        { &ecx7, 0, "prefetchwt1" },
        { &ecx81, 0, "lahf" },
        { &ecx81, 5, "abm" },
        { &ecx81, 6, "sse4a" },
        { &ecx81, 11, "xop" },
        { &ecx81, 21, "tbm" },
        { &edx81, 11, "syscall" },
        { &edx81, 22, "mmxext" },
        { &edx81, 27, "rdtscp" },
        { &edx81, 30, "3dnowext" },
        { &edx81, 31, "3dnow" }
    };
    OUStringBuffer flags;
    for( const FlagItem& item : flagItems )
    {
        if( *item.reg & ( 1U << item.bit ))
        {
            if( !flags.isEmpty())
                flags.append( " " );
            flags.appendAscii( item.name );
        }
    }
    if( !flags.isEmpty())
        addKeyValue( "CPUFlags", flags.makeStringAndClear(), AddItem );
    // Get total memory.
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof( memoryStatus );
    if( GlobalMemoryStatusEx( &memoryStatus ))
    {
        addKeyValue( "MemoryTotal", OUString::number( int( memoryStatus.ullTotalPhys / 1024 ))
            + " kB", AddItem );
    }
}
#else
void CrashReporter::writeSystemInfo()
{
}
#endif

#endif //HAVE_FEATURE_BREAKPAD

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
