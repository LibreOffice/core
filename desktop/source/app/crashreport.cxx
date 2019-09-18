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
#include <fstream>

osl::Mutex CrashReporter::maMutex;

#if HAVE_FEATURE_BREAKPAD

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

google_breakpad::ExceptionHandler* CrashReporter::mpExceptionHandler = nullptr;
bool CrashReporter::mbInit = false;
CrashReporter::vmaKeyValues CrashReporter::maKeyValues;


void CrashReporter::WriteToFile(std::ios_base::openmode Openmode)
{
    std::ofstream ini_file(getIniFileName(), Openmode);

    for (auto& keyValue : maKeyValues)
    {
        ini_file << OUStringToOString(keyValue.first, RTL_TEXTENCODING_UTF8).getStr() << "=";
        ini_file << OUStringToOString(keyValue.second, RTL_TEXTENCODING_UTF8).getStr() << "\n";
    }

    maKeyValues.clear();
    ini_file.close();
}

void CrashReporter::AddKeyValue(const OUString& rKey, const OUString& rValue, tAddKeyHandling AddKeyHandling)
{
    osl::MutexGuard aGuard(maMutex);

    if (IsDump())
    {
        if (!rKey.isEmpty())
            maKeyValues.push_back(mpair(rKey, rValue));

        if (AddKeyHandling != AddItem)
        {
            if (mbInit)
                WriteToFile(std::ios_base::app);
            else if (AddKeyHandling == Create)
                WriteCommonInfo();
        }
    }
}

void CrashReporter::WriteCommonInfo()
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
    AddKeyValue("ProductName", "LibreOffice", AddItem);
    AddKeyValue("Version", LIBO_VERSION_DOTTED, AddItem);
    AddKeyValue("BuildID", utl::Bootstrap::getBuildIdData(""), AddItem);
    AddKeyValue("URL", protocol + "://" + url + "/submit/", AddItem);

    if (proxy_server.aName != OUString())
    {
        AddKeyValue("Proxy", proxy_server.aName + ":" + OUString::number(proxy_server.nPort), AddItem);
    }

    // write the new keys at the end
    maKeyValues.insert(maKeyValues.end(), atlast.begin(), atlast.end());

    mbInit = true;

    WriteToFile(std::ios_base::trunc);

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


std::string CrashReporter::getIniFileName()
{
    OUString url = getCrashDirectory() + "dump.ini";
    OString aUrl = OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    std::string aRet(aUrl.getStr());
    return aRet;
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

bool CrashReporter::CrashReportInfoExists()
{
    return crashreport::readConfig(CrashReporter::getIniFileName(), nullptr);
}

bool CrashReporter::ReadSendConfig(std::string& response)
{
    return crashreport::readConfig(CrashReporter::getIniFileName(), &response);
}

void CrashReporter::StoreExceptionHandler(google_breakpad::ExceptionHandler* pExceptionHandler)
{
    if(IsDump())
        mpExceptionHandler = pExceptionHandler;
}



bool CrashReporter::IsDump()
{
    OUString sToken;
    OString  sEnvVar(std::getenv("CRASH_DUMP_ENABLE"));
    bool     bEnable = true;   // default, always on
    // read configuration item 'CrashDumpEnable' -> bool on/off
    if (rtl::Bootstrap::get("CrashDump", sToken) && sEnvVar.isEmpty())
    {
        bEnable = sToken.toBoolean();
    }

    return bEnable;
}


#endif   // HAVE_FEATURE_BREAKPAD



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
