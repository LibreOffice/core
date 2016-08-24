/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updater.hxx"

#include <unistd.h>
#include <errno.h>

#include <config_folders.h>
#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Update.hxx>

#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/configmgr.hxx>
#include <osl/file.hxx>

#include <curl/curl.h>

namespace {

static const char kUserAgent[] = "UpdateChecker/1.0 (Linux)";

const char* pUpdaterName = "updater";

void CopyFileToDir(const OUString& rTempDirURL, const OUString rFileName, const OUString& rOldDir)
{
    OUString aSourceURL = rOldDir + "/" + rFileName;
    OUString aDestURL = rTempDirURL + "/" + rFileName;

    osl::File::RC eError = osl::File::copy(aSourceURL, aDestURL);
    if (eError != osl::File::E_None)
    {
        SAL_WARN("desktop.updater", "could not copy the file to a temp directory: " << rFileName);
        throw std::exception();
    }
}

OUString getPathFromURL(const OUString& rURL)
{
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(rURL, aPath);

    return aPath;
}

void CopyUpdaterToTempDir(const OUString& rInstallDirURL, const OUString& rTempDirURL)
{
    OUString aUpdaterName = OUString::fromUtf8(pUpdaterName);
    CopyFileToDir(rTempDirURL, aUpdaterName, rInstallDirURL);
}

void createStr(const char* pSrc, char** pArgs, size_t i)
{
    size_t nLength = std::strlen(pSrc);
    char* pFinalStr = new char[nLength + 1];
    std::strncpy(pFinalStr, pSrc, nLength);
    pFinalStr[nLength] = '\0';
    pArgs[i] = pFinalStr;
}

void createStr(const OUString& rStr, char** pArgs, size_t i)
{
    OString aStr = OUStringToOString(rStr, RTL_TEXTENCODING_UTF8);
    char* pStr = new char[aStr.getLength() + 1];
    std::strncpy(pStr, aStr.getStr(), aStr.getLength());
    pStr[aStr.getLength()] = '\0';
    pArgs[i] = pStr;
}

char** createCommandLine(const OUString& rInstallDir)
{
    size_t nArgs = 6;
    char** pArgs = new char*[nArgs];
    {
        createStr(pUpdaterName, pArgs, 0);
    }
    {
        OUString aPatchDir("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/");
        rtl::Bootstrap::expandMacros(aPatchDir);
        OUString aTempDirPath = getPathFromURL(aPatchDir);
        createStr(aPatchDir, pArgs, 1);
    }
    {
        createStr(rInstallDir, pArgs, 2);
    }
    {
        OUString aWorkingDir = rInstallDir + "/updated";
        createStr(aWorkingDir, pArgs, 3);
    }
    {
        const char* pPID = "/replace";
        createStr(pPID, pArgs, 4);
    }
    pArgs[nArgs - 1] = nullptr;

    return pArgs;
}

}

void Update(const OUString& rInstallDirURL)
{
    utl::TempFile aTempDir(nullptr, true);
    OUString aTempDirURL = aTempDir.GetURL();
    CopyUpdaterToTempDir(rInstallDirURL, aTempDirURL);

    OUString aTempDirPath = getPathFromURL(aTempDirURL);
    OString aPath = OUStringToOString(aTempDirPath + "/" + OUString::fromUtf8(pUpdaterName), RTL_TEXTENCODING_UTF8);

    char** pArgs = createCommandLine(rInstallDirURL);

    if (execv(aPath.getStr(), pArgs))
    {
        printf("execv failed with error %d %s\n",errno,strerror(errno));
    }
    for (size_t i = 0; i < 6; ++i)
    {
        delete[] pArgs[i];
    }
    delete[] pArgs;
}

void CreateValidUpdateDir(const OUString& rInstallDir)
{
    OUString aInstallPath = getPathFromURL(rInstallDir);
    OUString aWorkdirPath = getPathFromURL(rInstallDir + "/updated");

    OUString aPatchDirPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/");
    rtl::Bootstrap::expandMacros(aPatchDirPath);
    OUString aPatchDir = getPathFromURL(aPatchDirPath);

    OUString aUpdaterPath = getPathFromURL(rInstallDir + "/program/" + OUString::fromUtf8(pUpdaterName));

    OUString aCommand = aUpdaterPath + " " + aPatchDir + " " + aInstallPath + " " + aWorkdirPath + " -1";

    OString aOCommand = OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8);

    int nResult = std::system(aOCommand.getStr());
    if (nResult)
    {
        // TODO: remove the update directory
        SAL_WARN("desktop.updater", "failed to update");
    }
}

namespace {

// Callback to get the response data from server.
static size_t WriteCallback(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  std::string* response = static_cast<std::string *>(userp);
  size_t real_size = size * nmemb;
  response->append(static_cast<char *>(ptr), real_size);
  return real_size;
}

}

void update_checker()
{
    OUString aDownloadCheckBaseURL = officecfg::Office::Update::Update::URL::get();

    OUString aProductName = utl::ConfigManager::getProductName();
    OUString aBuildID("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(aBuildID);
    OUString aVersion = "5.3.0.0.alpha0+";
    OUString aBuildTarget = "${_OS}-${_ARCH}";
    rtl::Bootstrap::expandMacros(aBuildTarget);
    OUString aLocale = "en-US";
    OUString aChannel = officecfg::Office::Update::Update::UpdateChannel::get();
    OUString aOSVersion = "0";

    OUString aDownloadCheckURL = aDownloadCheckBaseURL + "update/3/" + aProductName +
        "/" + aVersion + "/" + aBuildID + "/" + aBuildTarget + "/" + aLocale +
        "/" + aChannel + "/" + aOSVersion + "/default/default/update.xml?force=1";
    OString aURL = OUStringToOString(aDownloadCheckURL, RTL_TEXTENCODING_UTF8);
    SAL_DEBUG(aDownloadCheckURL);
    SAL_DEBUG("update_checker");

    CURL* curl = curl_easy_init();

    if (!curl)
        return;

    curl_easy_setopt(curl, CURLOPT_URL, aURL.getStr());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    bool bUseProxy = false;
    if (bUseProxy)
    {
        /*
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_user_pwd.c_str());
        */
    }

    char buf[] = "Expect:";
    curl_slist* headerlist = nullptr;
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string response_body;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
            static_cast<void *>(&response_body));

    // Fail if 400+ is returned from the web server.
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    SAL_DEBUG(http_code);
    SAL_DEBUG(cc);
    SAL_DEBUG(response_body);
    if (cc == CURLE_OK)
    {
        SAL_DEBUG(response_body);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
