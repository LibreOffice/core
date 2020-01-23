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

#include <config_version.h>
#include <config_folders.h>

#include <string>


#if HAVE_FEATURE_BREAKPAD

#include <fstream>
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
#include <client/linux/handler/exception_handler.h>
#elif defined WNT
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#endif
#include <client/windows/handler/exception_handler.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif
#endif

osl::Mutex CrashReporter::maMutex;
google_breakpad::ExceptionHandler* CrashReporter::mpExceptionHandler = nullptr;
bool CrashReporter::mbInit = false;
CrashReporter::vmaKeyValues CrashReporter::maKeyValues;


void CrashReporter::writeToFile(std::ios_base::openmode Openmode)
{
    std::ofstream ini_file(getIniFileName(), Openmode);

    for (auto& keyValue : maKeyValues)
    {
        ini_file << rtl::OUStringToOString(keyValue.first, RTL_TEXTENCODING_UTF8).getStr() << "=";
        ini_file << rtl::OUStringToOString(keyValue.second, RTL_TEXTENCODING_UTF8).getStr() << "\n";
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
#elif defined WNT
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

void CrashReporter::storeExceptionHandler(google_breakpad::ExceptionHandler* pExceptionHandler)
{
    if(IsDumpEnable())
        mpExceptionHandler = pExceptionHandler;
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


#endif //HAVE_FEATURE_BREAKPAD

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
