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
#include <fstream>

#include <config_folders.h>
#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Update.hxx>

#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/configmgr.hxx>
#include <osl/file.hxx>

#include <curl/curl.h>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

namespace {

class error_updater : public std::exception
{
};

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

// Callback to get the response data from server to a file.
static size_t WriteCallbackFile(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  SvStream* response = static_cast<SvStream *>(userp);
  size_t real_size = size * nmemb;
  response->WriteBytes(static_cast<char *>(ptr), real_size);
  return real_size;
}


class invalid_update_info : public std::exception
{
};

struct update_file
{
    OUString aURL;
    OUString aHash;
    size_t nSize;
};

struct language_file
{
    update_file aUpdateFile;
    OUString aLangCode;
};

struct update_info
{
    OUString aFromBuildID;
    OUString aSeeAlsoURL;
    OUString aNewVersion;

    update_file aUpdateFile;
    std::vector<language_file> aLanguageFiles;
};

OUString toOUString(const std::string& rStr)
{
    return OUString::fromUtf8(rStr.c_str());
}

update_file parse_update_file(const orcus::json::detail::node& rNode)
{
    if (rNode.type() != orcus::json::detail::node_t::object)
    {
        SAL_WARN("desktop.update", "invalid update or language file entry");
        throw invalid_update_info();
    }

    if (rNode.child_count() != 4)
    {
        SAL_WARN("desktop.update", "invalid update or language file entry");
        throw invalid_update_info();
    }

    orcus::json::detail::node aURLNode = rNode.child("url");
    orcus::json::detail::node aHashNode = rNode.child("hash");
    orcus::json::detail::node aHashTypeNode = rNode.child("hash_function");
    orcus::json::detail::node aSizeNode = rNode.child("size");

    if (aHashTypeNode.string_value() != "sha512")
    {
        SAL_WARN("desktop.update", "invalid hash type");
        throw invalid_update_info();
    }

    update_file aUpdateFile;
    aUpdateFile.aURL = toOUString(aURLNode.string_value().str());

    if (aUpdateFile.aURL.isEmpty())
        throw invalid_update_info();

    aUpdateFile.aHash = toOUString(aHashNode.string_value().str());
    aUpdateFile.nSize = toOUString(aSizeNode.string_value().str()).toUInt32();

    return aUpdateFile;
}

update_info parse_response(const std::string& rResponse)
{
    orcus::json_document_tree aJsonDoc;
    orcus::json_config aConfig;
    aJsonDoc.load(rResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json_node_t::object)
    {
        SAL_WARN("desktop.Update", "invalid root entries: " << rResponse);
        throw invalid_update_info();
    }

    auto aRootKeys = aDocumentRoot.keys();
    if (aRootKeys.size() != 5)
    {
        SAL_WARN("desktop.Update", "invalid root entries: " << rResponse);
        throw invalid_update_info();
    }

    orcus::json::detail::node aFromNode = aDocumentRoot.child("from");
    if (aFromNode.type() != orcus::json_node_t::string)
    {
        throw invalid_update_info();
    }

    orcus::json::detail::node aSeeAlsoNode = aDocumentRoot.child("see also");
    if (aSeeAlsoNode.type() != orcus::json_node_t::string)
    {
        throw invalid_update_info();
    }

    orcus::json::detail::node aVersionNode = aDocumentRoot.child("version");
    if (aVersionNode.type() != orcus::json_node_t::string)
    {
        throw invalid_update_info();
    }

    orcus::json::detail::node aUpdateNode = aDocumentRoot.child("update");
    if (aUpdateNode.type() != orcus::json_node_t::object)
    {
        throw invalid_update_info();
    }

    orcus::json::detail::node aLanguageNode = aDocumentRoot.child("languages");
    if (aUpdateNode.type() != orcus::json_node_t::object)
    {
        throw invalid_update_info();
    }

    update_info aUpdateInfo;
    aUpdateInfo.aFromBuildID = toOUString(aFromNode.string_value().str());
    aUpdateInfo.aNewVersion = toOUString(aVersionNode.string_value().str());
    aUpdateInfo.aSeeAlsoURL = toOUString(aSeeAlsoNode.string_value().str());

    aUpdateInfo.aUpdateFile = parse_update_file(aUpdateNode);

    std::vector<orcus::pstring> aLanguages = aLanguageNode.keys();
    for (auto itr = aLanguages.begin(), itrEnd = aLanguages.end(); itr != itrEnd; ++itr)
    {
        language_file aLanguageFile;
        auto aLangEntry = aLanguageNode.child(*itr);
        aLanguageFile.aLangCode = toOUString(itr->str());
        aLanguageFile.aUpdateFile = parse_update_file(aLangEntry);
        aUpdateInfo.aLanguageFiles.push_back(aLanguageFile);
    }

    return aUpdateInfo;
}

std::string download_content(const OString& rURL, bool bFile)
{
    CURL* curl = curl_easy_init();

    if (!curl)
        return std::string();

    curl_easy_setopt(curl, CURLOPT_URL, rURL.getStr());
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

    std::string response_body;
    utl::TempFile aTempFile;
    if (!bFile)
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                static_cast<void *>(&response_body));

        aTempFile.EnableKillingFile(true);
    }
    else
    {
        OUString aTempFileURL = aTempFile.GetURL();
        OString aTempFileURLOString = OUStringToOString(aTempFileURL, RTL_TEXTENCODING_UTF8);
        response_body.append(aTempFileURLOString.getStr(), aTempFileURLOString.getLength());

        aTempFile.EnableKillingFile(false);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackFile);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                static_cast<void *>(aTempFile.GetStream(StreamMode::WRITE)));
    }

    // Fail if 400+ is returned from the web server.
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    CURLcode cc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200)
    {
        SAL_WARN("desktop.updater", "download did not succeed. Error code: " << http_code);
        throw error_updater();
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("desktop.updater", "curl error: " << cc);
        throw error_updater();
    }

    return response_body;
}

OUString generateHash(const OUString& rURL)
{
    return OUString();
}

void handle_file_error(osl::FileBase::RC eError)
{
    switch (eError)
    {
        case osl::FileBase::E_None:
        break;
        default:
            SAL_WARN("desktop.updater", "file error code: " << eError);
            throw error_updater();
    }
}

void download_file(const OUString& rURL, size_t nFileSize, const OUString& rHash, const OUString& aFileName)
{
    OString aURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    std::string temp_file = download_content(aURL, true);
    if (temp_file.empty())
        throw error_updater();

    OUString aTempFile = OUString::fromUtf8(temp_file.c_str());
    osl::File aDownloadedFile(aTempFile);
    osl::FileBase::RC eError = aDownloadedFile.open(1);
    handle_file_error(eError);

    sal_uInt64 nSize = 0;
    eError = aDownloadedFile.getSize(nSize);
    handle_file_error(eError);
    if (nSize != nFileSize)
    {
        SAL_WARN("desktop.updater", "File sizes don't match. File might be corrupted.");
    }

    OUString aHash = generateHash(aTempFile);
    if (aHash != rHash)
    {
        SAL_WARN("desktop.updater", "File hash don't match. File might be corrupted.");
    }

    OUString aPatchDirURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/");
    rtl::Bootstrap::expandMacros(aPatchDirURL);
    osl::Directory::create(aPatchDirURL);

    OUString aDestFile = aPatchDirURL + aFileName;
    eError = osl::File::move(aTempFile, aDestFile);
    handle_file_error(eError);
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

#if 0
    std::string response_body = download_content(aURL, false);
#else
    std::string response_body;
    if(std::ifstream is{"/lo/users/moggi/update.json", std::ios::binary | std::ios::ate})
    {
        auto size = is.tellg();
        std::string str(size, '\0'); // construct string to stream size
        is.seekg(0);
        is.read(&str[0], size);
        response_body = str;
    }
#endif

    try
    {
        if (!response_body.empty())
        {
            update_info aUpdateInfo = parse_response(response_body);
            download_file(aUpdateInfo.aUpdateFile.aURL, aUpdateInfo.aUpdateFile.nSize, aUpdateInfo.aUpdateFile.aHash, "update.mar");
        }
    }
    catch (const invalid_update_info&)
    {
        SAL_WARN("desktop.updater", "invalid update information");
    }
    catch (const error_updater&)
    {
        SAL_WARN("desktop.updater", "error during the update check");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
