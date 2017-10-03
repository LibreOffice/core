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
#include <unotools/bootstrap.hxx>

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
std::map<OUString, OUString> CrashReporter::maKeyValues;

namespace {

void writeToStream(std::ofstream& strm, const OUString& rKey, const OUString& rValue)
{
    strm << rtl::OUStringToOString(rKey, RTL_TEXTENCODING_UTF8).getStr() << "=";
    strm << rtl::OUStringToOString(rValue, RTL_TEXTENCODING_UTF8).getStr() << "\n";
}

}

void CrashReporter::AddKeyValue(const OUString& rKey, const OUString& rValue)
{
    osl::MutexGuard aGuard(maMutex);
    if (mbInit)
    {
        std::string ini_path = getIniFileName();
        std::ofstream ini_file(ini_path, std::ios_base::app);
        writeToStream(ini_file, rKey, rValue);
    }
    else
    {
        maKeyValues.insert(std::pair<OUString, OUString>(rKey, rValue));
    }
}

#endif

void CrashReporter::writeCommonInfo()
{
    osl::MutexGuard aGuard(maMutex);
    // limit the amount of code that needs to be executed before the crash reporting
    std::string ini_path = CrashReporter::getIniFileName();
    std::ofstream minidump_file(ini_path, std::ios_base::trunc);
    minidump_file << "ProductName=LibreOffice\n";
    minidump_file << "Version=" LIBO_VERSION_DOTTED "\n";
    minidump_file << "BuildID=" << utl::Bootstrap::getBuildIdData("") << "\n";
    minidump_file << "URL=https://crashreport.libreoffice.org/submit/\n";
    for (auto& keyValue : maKeyValues)
    {
        writeToStream(minidump_file, keyValue.first, keyValue.second);
    }
    maKeyValues.clear();
    minidump_file.close();

    mbInit = true;

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
    mpExceptionHandler->set_dump_path(SAL_W(aURL.getStr()));
#endif
}

void CrashReporter::storeExceptionHandler(google_breakpad::ExceptionHandler* pExceptionHandler)
{
    mpExceptionHandler = pExceptionHandler;
}

std::string CrashReporter::getIniFileName()
{
    OUString url = getCrashDirectory() + "dump.ini";
    OString aUrl = OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    std::string aRet(aUrl.getStr());
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
