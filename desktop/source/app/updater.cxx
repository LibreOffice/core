/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updater.hxx"

#if UNX
#include <unistd.h>
#include <errno.h>

#endif

#ifdef _WIN32
#include <comphelper/windowsStart.hxx>
#endif

#include <fstream>
#include <config_folders.h>
#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Update.hxx>

#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/configmgr.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/file.hxx>
#include <rtl/process.h>
#include <sal/log.hxx>
#include <tools/stream.hxx>

#include <curl/curl.h>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>

#include <systools/curlinit.hxx>
#include <comphelper/hash.hxx>

#include <com/sun/star/container/XNameAccess.hpp>

#include <officecfg/Setup.hxx>

#include <functional>
#include <memory>
#include <set>
#include <string_view>

namespace {

class error_updater : public std::exception
{
    OString maStr;
public:

    error_updater(const OString& rStr):
        maStr(rStr)
    {
    }

    virtual const char* what() const  throw() override
    {
        return maStr.getStr();
    }
};

#ifdef UNX
static const char kUserAgent[] = "LibreOffice UpdateChecker/1.0 (Linux)";
#else
static const char kUserAgent[] = "LibreOffice UpdateChecker/1.0 (unknown platform)";
#endif

#ifdef UNX
const char* const pUpdaterName = "updater";
const char* const pSofficeExeName = "soffice";
#elif defined(_WIN32)
const char* pUpdaterName = "updater.exe";
const char* pSofficeExeName = "soffice.exe";
#else
#error "Need implementation"
#endif

OUString normalizePath(const OUString& rPath)
{
    OUString aPath =  rPath;
#if defined WNT
    aPath = aPath.replace('\\', '/');
#endif

    aPath = aPath.replaceAll("//", "/");

    // remove final /
    if (aPath.endsWith("/"))
    {
        aPath = aPath.copy(0, aPath.getLength() - 1);
    }

    while (aPath.indexOf("/..") != -1)
    {
        sal_Int32 nIndex = aPath.indexOf("/..");
        sal_Int32 i = nIndex - 1;
        for (; i > 0; --i)
        {
            if (aPath[i] == '/')
                break;
        }

        OUString aTempPath = aPath;
        aPath = aTempPath.copy(0, i) + aPath.copy(nIndex + 3);
    }

#if defined WNT
    aPath = aPath.replace('/', '\\');
#endif
    return aPath;
}

void CopyFileToDir(const OUString& rTempDirURL, const OUString & rFileName, const OUString& rOldDir)
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

    return normalizePath(aPath);
}

void CopyUpdaterToTempDir(const OUString& rInstallDirURL, const OUString& rTempDirURL)
{
    OUString aUpdaterName = OUString::fromUtf8(pUpdaterName);
    CopyFileToDir(rTempDirURL, aUpdaterName, rInstallDirURL);
    CopyFileToDir(rTempDirURL, u"updater.ini"_ustr, rInstallDirURL);
}

#ifdef UNX
typedef char CharT;
#define tstrncpy std::strncpy
char const * toStream(char const * s) { return s; }
#elif defined(_WIN32)
typedef wchar_t CharT;
#define tstrncpy std::wcsncpy
OUString toStream(wchar_t const * s) { return OUString(o3tl::toU(s)); }
#else
#error "Need an implementation"
#endif

void createStr(const OUString& rStr, CharT** pArgs, size_t i)
{
#ifdef UNX
    OString aStr = OUStringToOString(rStr, RTL_TEXTENCODING_UTF8);
#elif defined(_WIN32)
    OUString aStr = rStr;
#else
#error "Need an implementation"
#endif
    CharT* pStr = new CharT[aStr.getLength() + 1];
    tstrncpy(pStr, (CharT*)aStr.getStr(), aStr.getLength());
    pStr[aStr.getLength()] = '\0';
    pArgs[i] = pStr;
}

CharT** createCommandLine(OUString const & argv0, int * argc)
{
    OUString aInstallDir = Updater::getInstallationPath();

    size_t nCommandLineArgs = rtl_getAppCommandArgCount();
    size_t nArgs = 8 + nCommandLineArgs;
    CharT** pArgs = new CharT*[nArgs];
    createStr(argv0, pArgs, 0);
    {
        // directory with the patch log
        OUString aPatchDir = Updater::getPatchDirURL();
        rtl::Bootstrap::expandMacros(aPatchDir);
        OUString aTempDirPath = getPathFromURL(aPatchDir);
        Updater::log("Patch Dir: " + aTempDirPath);
        createStr(aTempDirPath, pArgs, 1);
    }
    {
        // the actual update directory
        Updater::log("Install Dir: " + aInstallDir);
        createStr(aInstallDir, pArgs, 2);
    }
    {
        // the temporary updated build
        Updater::log("Working Dir: " + aInstallDir);
        createStr(aInstallDir, pArgs, 3);
    }
    {
#ifdef UNX
        OUString aPID("0");
#elif defined(_WIN32)
        oslProcessInfo aInfo;
        aInfo.Size = sizeof(oslProcessInfo);
        osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER, &aInfo);
        OUString aPID = OUString::number(aInfo.Ident);
#else
#error "Need an implementation"
#endif
        createStr(aPID, pArgs, 4);
    }
    {
        OUString aExeDir = Updater::getExecutableDirURL();
        OUString aSofficePath = getPathFromURL(aExeDir);
        Updater::log("soffice Path: " + aSofficePath);
        createStr(aSofficePath, pArgs, 5);
    }
    {
        // the executable to start after the successful update
        OUString aExeDir = Updater::getExecutableDirURL();
        OUString aSofficePathURL = aExeDir + OUString::fromUtf8(pSofficeExeName);
        OUString aSofficePath = getPathFromURL(aSofficePathURL);
        createStr(aSofficePath, pArgs, 6);
    }

    // add the command line arguments from the soffice list
    for (size_t i = 0; i < nCommandLineArgs; ++i)
    {
        OUString aCommandLineArg;
        rtl_getAppCommandArg(i, &aCommandLineArg.pData);
        createStr(aCommandLineArg, pArgs, 7 + i);
    }

    pArgs[nArgs - 1] = nullptr;

    *argc = nArgs - 1;
    return pArgs;
}

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
    OUString aMessage;

    update_file aUpdateFile;
    std::vector<language_file> aLanguageFiles;
};

bool isUserWritable(const OUString& rFileURL)
{
    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    osl::DirectoryItem aDirectoryItem;

    osl::FileBase::RC eRes = osl::DirectoryItem::get(rFileURL, aDirectoryItem);
    if (eRes != osl::FileBase::E_None)
    {
        Updater::log("Could not get the directory item for: " + rFileURL);
        return false;
    }

    osl::FileBase::RC eResult = aDirectoryItem.getFileStatus(aStatus);
    if (eResult != osl::FileBase::E_None)
    {
        Updater::log("Could not get the file status for: " + rFileURL);
        return false;
    }

    bool bReadOnly = (aStatus.getAttributes() & static_cast<sal_uInt64>(osl_File_Attribute_ReadOnly)) != 0;
    if (bReadOnly)
    {
        Updater::log("Update location as determined by: " + rFileURL + " is read-only.");
        return false;
    }

    return true;
}

}

bool update()
{
    utl::TempFileNamed aTempDir(nullptr, true);
    OUString aTempDirURL = aTempDir.GetURL();
    CopyUpdaterToTempDir(Updater::getExecutableDirURL(), aTempDirURL);

    OUString aUpdaterPath = getPathFromURL(aTempDirURL + "/" + OUString::fromUtf8(pUpdaterName));

    Updater::log("Calling the updater with parameters: ");
    int argc;
    CharT** pArgs = createCommandLine(aUpdaterPath, &argc);

    bool bSuccess = true;
    const char* pUpdaterTestReplace = std::getenv("LIBO_UPDATER_TEST_REPLACE");
    if (!pUpdaterTestReplace)
    {
#if UNX
        OString aPath = OUStringToOString(aUpdaterPath, RTL_TEXTENCODING_UTF8);
        if (execv(aPath.getStr(), pArgs))
        {
            printf("execv failed with error %d %s\n",errno,strerror(errno));
            bSuccess = false;
        }
#elif defined(_WIN32)
        bSuccess = WinLaunchChild((wchar_t*)aUpdaterPath.getStr(), argc, pArgs);
#endif
    }
    else
    {
        SAL_WARN("desktop.updater", "Updater executable path: " << aUpdaterPath);
        for (size_t i = 0; i < 8 + rtl_getAppCommandArgCount(); ++i)
        {
            SAL_WARN("desktop.updater", toStream(pArgs[i]));
        }
        bSuccess = false;
    }

    for (size_t i = 0; i < 8 + rtl_getAppCommandArgCount(); ++i)
    {
        delete[] pArgs[i];
    }
    delete[] pArgs;

    return bSuccess;
}

namespace {

// Callback to get the response data from server.
size_t WriteCallback(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  std::string* response = static_cast<std::string *>(userp);
  size_t real_size = size * nmemb;
  response->append(static_cast<char *>(ptr), real_size);
  return real_size;
}



class invalid_update_info : public std::exception
{
};

class invalid_hash : public std::exception
{
    OString maMessage;
public:

    invalid_hash(const OUString& rExpectedHash, const OUString& rReceivedHash)
      : maMessage(
          OUStringToOString(
            OUString("Invalid hash found.\nExpected: " + rExpectedHash + ";\nReceived: " + rReceivedHash),
            RTL_TEXTENCODING_UTF8)
         )
    {
    }

    const char* what() const noexcept override
    {
        return maMessage.getStr();
    }
};

class invalid_size : public std::exception
{
    OString maMessage;
public:

    invalid_size(const size_t nExpectedSize, const size_t nReceivedSize)
      : maMessage(
          OUStringToOString(
            OUString("Invalid file size found.\nExpected: " + OUString::number(nExpectedSize) + ";\nReceived: " + OUString::number(nReceivedSize)),
            RTL_TEXTENCODING_UTF8)
         )
    {
    }

    const char* what() const noexcept override
    {
        return maMessage.getStr();
    }
};

OUString toOUString(const std::string_view& rStr)
{
    return OUString::fromUtf8(rStr);
}

update_file parse_update_file(orcus::json::node& rNode)
{
    if (rNode.type() != orcus::json::node_t::object)
    {
        SAL_WARN("desktop.updater", "invalid update or language file entry");
        throw invalid_update_info();
    }

    if (rNode.child_count() < 4)
    {
        SAL_WARN("desktop.updater", "invalid update or language file entry");
        throw invalid_update_info();
    }

    orcus::json::node aURLNode = rNode.child("url");
    orcus::json::node aHashNode = rNode.child("hash");
    orcus::json::node aHashTypeNode = rNode.child("hash_function");
    orcus::json::node aSizeNode = rNode.child("size");

    if (aHashTypeNode.string_value() != "sha512")
    {
        SAL_WARN("desktop.updater", "invalid hash type");
        throw invalid_update_info();
    }

    update_file aUpdateFile;
    aUpdateFile.aURL = toOUString(aURLNode.string_value());

    if (aUpdateFile.aURL.isEmpty())
        throw invalid_update_info();

    aUpdateFile.aHash = toOUString(aHashNode.string_value());
    aUpdateFile.nSize = static_cast<sal_uInt32>(aSizeNode.numeric_value());
    return aUpdateFile;
}

update_info parse_response(const std::string& rResponse)
{
    orcus::json::document_tree aJsonDoc;
    orcus::json_config aConfig;
    aJsonDoc.load(rResponse, aConfig);

    auto aDocumentRoot = aJsonDoc.get_document_root();
    if (aDocumentRoot.type() != orcus::json::node_t::object)
    {
        SAL_WARN("desktop.updater", "invalid root entries: " << rResponse);
        throw invalid_update_info();
    }

    auto aRootKeys = aDocumentRoot.keys();
    if (std::find(aRootKeys.begin(), aRootKeys.end(), "error") != aRootKeys.end())
    {
        throw invalid_update_info();
    }
    else if (std::find(aRootKeys.begin(), aRootKeys.end(), "response") != aRootKeys.end())
    {
        update_info aUpdateInfo;
        auto aMsgNode = aDocumentRoot.child("response");
        aUpdateInfo.aMessage = toOUString(aMsgNode.string_value());
        return aUpdateInfo;
    }

    orcus::json::node aFromNode = aDocumentRoot.child("from");
    if (aFromNode.type() != orcus::json::node_t::string)
    {
        throw invalid_update_info();
    }

    orcus::json::node aSeeAlsoNode = aDocumentRoot.child("see also");
    if (aSeeAlsoNode.type() != orcus::json::node_t::string)
    {
        throw invalid_update_info();
    }

    orcus::json::node aUpdateNode = aDocumentRoot.child("update");
    if (aUpdateNode.type() != orcus::json::node_t::object)
    {
        throw invalid_update_info();
    }

    orcus::json::node aLanguageNode = aDocumentRoot.child("languages");
    if (aLanguageNode.type() != orcus::json::node_t::object)
    {
        throw invalid_update_info();
    }

    update_info aUpdateInfo;
    aUpdateInfo.aFromBuildID = toOUString(aFromNode.string_value());
    aUpdateInfo.aSeeAlsoURL = toOUString(aSeeAlsoNode.string_value());

    aUpdateInfo.aUpdateFile = parse_update_file(aUpdateNode);

    std::vector<std::string_view> aLanguages = aLanguageNode.keys();
    for (auto const& language : aLanguages)
    {
        language_file aLanguageFile;
        auto aLangEntry = aLanguageNode.child(language);
        aLanguageFile.aLangCode = toOUString(language);
        aLanguageFile.aUpdateFile = parse_update_file(aLangEntry);
        aUpdateInfo.aLanguageFiles.push_back(aLanguageFile);
    }

    return aUpdateInfo;
}

struct WriteDataFile
{
    comphelper::Hash maHash;
    SvStream* mpStream;

    WriteDataFile(SvStream* pStream):
        maHash(comphelper::HashType::SHA512),
        mpStream(pStream)
    {
    }

    OUString getHash()
    {
        auto final_hash = maHash.finalize();
        std::stringstream aStrm;
        for (auto& i: final_hash)
        {
            aStrm << std::setw(2) << std::setfill('0') << std::hex << (int)i;
        }

        return toOUString(aStrm.str());
    }
};

// Callback to get the response data from server to a file.
size_t WriteCallbackFile(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  WriteDataFile* response = static_cast<WriteDataFile *>(userp);
  size_t real_size = size * nmemb;
  response->mpStream->WriteBytes(ptr, real_size);
  response->maHash.update(static_cast<const unsigned char*>(ptr), real_size);
  return real_size;
}

std::string download_content(const OString& rURL, bool bFile, OUString& rHash)
{
    Updater::log("Download: " + rURL);
    std::unique_ptr<CURL, std::function<void(CURL *)>> curl(
        curl_easy_init(), [](CURL * p) { curl_easy_cleanup(p); });

    if (!curl)
        return std::string();

    ::InitCurl_easy(curl.get());

    curl_easy_setopt(curl.get(), CURLOPT_URL, rURL.getStr());
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, kUserAgent);
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
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1); // follow redirects
    // only allow redirect to https://
#if (LIBCURL_VERSION_MAJOR > 7) || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR >= 85)
    curl_easy_setopt(curl.get(), CURLOPT_REDIR_PROTOCOLS_STR, "https");
#else
    curl_easy_setopt(curl.get(), CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
#endif

    std::string response_body;
    utl::TempFileNamed aTempFile;
    WriteDataFile aFile(aTempFile.GetStream(StreamMode::WRITE));
    if (!bFile)
    {
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA,
                static_cast<void *>(&response_body));

        aTempFile.EnableKillingFile(true);
    }
    else
    {
        OUString aTempFileURL = aTempFile.GetURL();
        OString aTempFileURLOString = OUStringToOString(aTempFileURL, RTL_TEXTENCODING_UTF8);
        response_body.append(aTempFileURLOString.getStr(), aTempFileURLOString.getLength());

        aTempFile.EnableKillingFile(false);

        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallbackFile);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA,
                static_cast<void *>(&aFile));
    }

    // Fail if 400+ is returned from the web server.
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1);

    CURLcode cc = curl_easy_perform(curl.get());
    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200)
    {
        SAL_WARN("desktop.updater", "download did not succeed. Error code: " << http_code);
        throw error_updater("download did not succeed");
    }

    if (cc != CURLE_OK)
    {
        SAL_WARN("desktop.updater", "curl error: " << cc);
        throw error_updater("curl error");
    }

    if (bFile)
        rHash = aFile.getHash();

    return response_body;
}

void handle_file_error(osl::FileBase::RC eError, const OUString& rMsg)
{
    switch (eError)
    {
        case osl::FileBase::E_None:
        break;
        default:
            SAL_WARN("desktop.updater", "file error code: " << eError << ", " << rMsg);
            throw error_updater(OUStringToOString(rMsg, RTL_TEXTENCODING_UTF8));
    }
}

void download_file(const OUString& rURL, size_t nFileSize, const OUString& rHash, const OUString& aFileName)
{
    Updater::log("Download File: " + rURL + "; FileName: " + aFileName);
    OString aURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    OUString aHash;
    std::string temp_file = download_content(aURL, true, aHash);
    if (temp_file.empty())
        throw error_updater("empty temp file string");

    OUString aTempFile = OUString::fromUtf8(temp_file.c_str());
    Updater::log("TempFile: " + aTempFile);
    osl::File aDownloadedFile(aTempFile);
    osl::FileBase::RC eError = aDownloadedFile.open(1);
    handle_file_error(eError, "Could not open the download file: " + aTempFile);

    sal_uInt64 nSize = 0;
    eError = aDownloadedFile.getSize(nSize);
    handle_file_error(eError, "Could not get the file size of the downloaded file: " + aTempFile);
    if (nSize != nFileSize)
    {
        SAL_WARN("desktop.updater", "File sizes don't match. File might be corrupted.");
        throw invalid_size(nFileSize, nSize);
    }

    if (aHash != rHash)
    {
        SAL_WARN("desktop.updater", "File hash don't match. File might be corrupted.");
        throw invalid_hash(rHash, aHash);
    }

    OUString aPatchDirURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/updates/");
    rtl::Bootstrap::expandMacros(aPatchDirURL);
    osl::Directory::create(aPatchDirURL);
    aPatchDirURL += "0/";
    osl::Directory::create(aPatchDirURL);

    OUString aDestFile = aPatchDirURL + aFileName;
    Updater::log("Destination File: " + aDestFile);
    aDownloadedFile.close();
    eError = osl::File::move(aTempFile, aDestFile);
    handle_file_error(eError, "Could not move the file from the Temp directory to the user config: TempFile: " + aTempFile + "; DestFile: " + aDestFile);
}

}

void update_checker()
{
    OUString aBrandBaseDir("${BRAND_BASE_DIR}");
    rtl::Bootstrap::expandMacros(aBrandBaseDir);
    bool bUserWritable = isUserWritable(aBrandBaseDir);
    if (!bUserWritable)
    {
        Updater::log("Can't update as the update location is not user writable");
        return;
    }

    OUString aDownloadCheckBaseURL = officecfg::Office::Update::Update::URL::get();
    static const char* pDownloadCheckBaseURLEnv = std::getenv("LIBO_UPDATER_URL");
    if (pDownloadCheckBaseURLEnv)
    {
        aDownloadCheckBaseURL = OUString::createFromAscii(pDownloadCheckBaseURLEnv);
    }

    OUString aProductName = utl::ConfigManager::getProductName();
    OUString aBuildID = Updater::getBuildID();

    static const char* pBuildIdEnv = std::getenv("LIBO_UPDATER_BUILD");
    if (pBuildIdEnv)
    {
        aBuildID = OUString::createFromAscii(pBuildIdEnv);
    }

    OUString aBuildTarget = "${_OS}_${_ARCH}";
    rtl::Bootstrap::expandMacros(aBuildTarget);
    OUString aChannel = Updater::getUpdateChannel();
    static const char* pUpdateChannelEnv = std::getenv("LIBO_UPDATER_CHANNEL");
    if (pUpdateChannelEnv)
    {
        aChannel = OUString::createFromAscii(pUpdateChannelEnv);
    }

    OUString aDownloadCheckURL = aDownloadCheckBaseURL + "update/check/1/" + aProductName +
        "/" + aBuildID + "/" + aBuildTarget + "/" + aChannel;
    OString aURL = OUStringToOString(aDownloadCheckURL, RTL_TEXTENCODING_UTF8);
    Updater::log("Update check: " + aURL);

    try
    {
        OUString aHash;
        std::string response_body = download_content(aURL, false, aHash);
        if (!response_body.empty())
        {

            update_info aUpdateInfo = parse_response(response_body);
            if (aUpdateInfo.aUpdateFile.aURL.isEmpty())
            {
                // No update currently available
                // add entry to updating.log with the message
                SAL_WARN("desktop.updater", "Message received from the updater: " << aUpdateInfo.aMessage);
                Updater::log("Server response: " + aUpdateInfo.aMessage);
            }
            else
            {
                css::uno::Sequence<OUString> aInstalledLanguages(officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
                std::set<OUString> aInstalledLanguageSet(std::begin(aInstalledLanguages), std::end(aInstalledLanguages));
                download_file(aUpdateInfo.aUpdateFile.aURL, aUpdateInfo.aUpdateFile.nSize, aUpdateInfo.aUpdateFile.aHash, "update.mar");
                for (auto& lang_update : aUpdateInfo.aLanguageFiles)
                {
                    // only download the language packs for installed languages
                    if (aInstalledLanguageSet.find(lang_update.aLangCode) != aInstalledLanguageSet.end())
                    {
                        OUString aFileName = "update_" + lang_update.aLangCode + ".mar";
                        download_file(lang_update.aUpdateFile.aURL, lang_update.aUpdateFile.nSize, lang_update.aUpdateFile.aHash, aFileName);
                    }
                }
                OUString aSeeAlsoURL = aUpdateInfo.aSeeAlsoURL;
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                        comphelper::ConfigurationChanges::create());
                officecfg::Office::Update::Update::SeeAlso::set(aSeeAlsoURL, batch);
                batch->commit();
                OUString const statUrl = Updater::getPatchDirURL() + "update.status";
                SvFileStream stat(statUrl, StreamMode::WRITE | StreamMode::TRUNC);
                stat.WriteOString("pending-service");
                stat.Flush();
                if (auto const e = stat.GetError()) {
                    Updater::log("Writing <" + statUrl + "> failed with " + e.toString());
                }
                stat.Close();
            }
        }
    }
    catch (const invalid_update_info&)
    {
        SAL_WARN("desktop.updater", "invalid update information");
        Updater::log(OString("warning: invalid update info"));
    }
    catch (const error_updater& e)
    {
        SAL_WARN("desktop.updater", "error during the update check: " << e.what());
        Updater::log(OString("warning: error by the updater") + e.what());
    }
    catch (const invalid_size& e)
    {
        SAL_WARN("desktop.updater", e.what());
        Updater::log(OString("warning: invalid size"));
    }
    catch (const invalid_hash& e)
    {
        SAL_WARN("desktop.updater", e.what());
        Updater::log(OString("warning: invalid hash"));
    }
    catch (...)
    {
        SAL_WARN("desktop.updater", "unknown error during the update check");
        Updater::log(OString("warning: unknown exception"));
    }
}

OUString Updater::getPatchDirURL()
{
    OUString aPatchDirURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/updates/0/");
    rtl::Bootstrap::expandMacros(aPatchDirURL);

    return aPatchDirURL;
}

OUString Updater::getUpdateFileURL()
{
    return getPatchDirURL() + "update.mar";
}

OUString Updater::getInstallationPath()
{
    OUString aInstallDir( "$BRAND_BASE_DIR/");
    rtl::Bootstrap::expandMacros(aInstallDir);

    return getPathFromURL(aInstallDir);
}

OUString Updater::getExecutableDirURL()
{
    OUString aExeDir( "$BRAND_BASE_DIR/" LIBO_BIN_FOLDER "/" );
    rtl::Bootstrap::expandMacros(aExeDir);

    return aExeDir;
}

void Updater::log(const OUString& rMessage)
{
    SAL_INFO("desktop.updater", rMessage);
    OUString dir("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/updates");
    rtl::Bootstrap::expandMacros(dir);
    osl::Directory::create(dir);
    SvFileStream aLog(dir + "/updating.log", StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteLine(OUStringToOString(rMessage, RTL_TEXTENCODING_UTF8));
}

void Updater::log(const OString& rMessage)
{
    SAL_INFO("desktop.updater", rMessage);
    OUString aUpdateLog = getUpdateInfoLog();
    SvFileStream aLog(aUpdateLog, StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteLine(rMessage);
}

void Updater::log(const char* pMessage)
{
    SAL_INFO("desktop.updater", pMessage);
    OUString aUpdateLog = getUpdateInfoLog();
    SvFileStream aLog(aUpdateLog, StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteOString(pMessage);
}

OUString Updater::getBuildID()
{
    OUString aBuildID("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(aBuildID);

    return aBuildID;
}

OUString Updater::getUpdateChannel()
{
    OUString aUpdateChannel("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":UpdateChannel}");
    rtl::Bootstrap::expandMacros(aUpdateChannel);

    return aUpdateChannel;
}

void Updater::removeUpdateFiles()
{
    Updater::log("Removing: " + getUpdateFileURL());
    osl::File::remove(getUpdateFileURL());

    OUString aPatchDirURL = getPatchDirURL();
    osl::Directory aDir(aPatchDirURL);
    aDir.open();

    osl::FileBase::RC eRC;
    do
    {
        osl::DirectoryItem aItem;
        eRC = aDir.getNextItem(aItem);
        if (eRC == osl::FileBase::E_None)
        {
            osl::FileStatus aStatus(osl_FileStatus_Mask_All);
            if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
                continue;

            if (!aStatus.isRegular())
                continue;

            OUString aURL = aStatus.getFileURL();
            if (!aURL.endsWith(".mar"))
                continue;

            Updater::log("Removing. " + aURL);
            osl::File::remove(aURL);
        }
    }
    while (eRC == osl::FileBase::E_None);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
