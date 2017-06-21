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
#include <comphelper/hash.hxx>

#include <com/sun/star/container/XNameAccess.hpp>

#include <officecfg/Setup.hxx>

#include <set>

namespace {

class error_updater : public std::exception
{
};

#ifdef UNX
static const char kUserAgent[] = "LibreOffice UpdateChecker/1.0 (Linux)";
#else
static const char kUserAgent[] = "LibreOffice UpdateChecker/1.0 (unknown platform)";
#endif

#ifdef UNX
const char* pUpdaterName = "updater";
const char* pSofficeExeName = "soffice";
#elif defined(WNT)
const char* pUpdaterName = "updater.exe";
const char* pSofficeExeName = "soffice.exe";
#else
#error "Need implementation"
#endif

OUString normalizePath(const OUString& rPath)
{
    OUString aPath =  rPath.replaceAll("//", "/");

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

    SAL_DEBUG(aPath);

    return aPath;
}

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

    return normalizePath(aPath);
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

char** createCommandLine()
{
    OUString aInstallDir( "$BRAND_BASE_DIR/" );
    rtl::Bootstrap::expandMacros(aInstallDir);

    size_t nArgs = 8;
    char** pArgs = new char*[nArgs];
    {
        createStr(pUpdaterName, pArgs, 0);
    }
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
        OUString aInstallPath = getPathFromURL(aInstallDir);
        Updater::log("Install Dir: " + aInstallPath);
        createStr(aInstallPath, pArgs, 2);
    }
    {
        // the temporary updated build
        OUString aUpdateDirURL = Updater::getUpdateDirURL();
        OUString aWorkingDir = getPathFromURL(aUpdateDirURL);
        Updater::log("Working Dir: " + aWorkingDir);
        createStr(aWorkingDir, pArgs, 3);
    }
    {
        const char* pPID = "/replace";
        createStr(pPID, pArgs, 4);
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
    pArgs[nArgs - 1] = nullptr;

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

void update()
{
    utl::TempFile aTempDir(nullptr, true);
    OUString aTempDirURL = aTempDir.GetURL();
    CopyUpdaterToTempDir(Updater::getExecutableDirURL(), aTempDirURL);

    OUString aTempDirPath = getPathFromURL(aTempDirURL);
    OString aPath = OUStringToOString(aTempDirPath + "/" + OUString::fromUtf8(pUpdaterName), RTL_TEXTENCODING_UTF8);

    Updater::log("Calling the updater with parameters: ");
    char** pArgs = createCommandLine();


#if UNX
    if (execv(aPath.getStr(), pArgs))
    {
        printf("execv failed with error %d %s\n",errno,strerror(errno));
    }
#endif

    for (size_t i = 0; i < 8; ++i)
    {
        delete[] pArgs[i];
    }
    delete[] pArgs;
}

void CreateValidUpdateDir(const update_info& update_info)
{
    Updater::log(OString("Create Update Dir"));
    OUString aInstallDir("$BRAND_BASE_DIR");
    rtl::Bootstrap::expandMacros(aInstallDir);
    OUString aInstallPath = getPathFromURL(aInstallDir);
    OUString aWorkdirPath = getPathFromURL(Updater::getUpdateDirURL());

    OUString aPatchDir = getPathFromURL(Updater::getPatchDirURL());

    OUString aUpdaterPath = getPathFromURL(Updater::getExecutableDirURL() + OUString::fromUtf8(pUpdaterName));

    OUString aCommand = aUpdaterPath + " " + aPatchDir + " " + aInstallPath + " " + aWorkdirPath + " -1";

    OString aOCommand = OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8);

    int nResult = std::system(aOCommand.getStr());
    if (nResult)
    {
        // TODO: remove the update directory
        SAL_WARN("desktop.updater", "failed to update");
        Updater::log(OUString("failed to create update dir"));
    }
    else
    {
        OUString aUpdateInfoURL(Updater::getPatchDirURL() + "/update.info");
        OUString aUpdateInfoPath = getPathFromURL(aUpdateInfoURL);
        SvFileStream aUpdateInfoFile(aUpdateInfoPath, StreamMode::WRITE | StreamMode::TRUNC);
        aUpdateInfoFile.WriteCharPtr("[UpdateInfo]\nOldBuildId=");
        aUpdateInfoFile.WriteByteStringLine(update_info.aFromBuildID, RTL_TEXTENCODING_UTF8);
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



class invalid_update_info : public std::exception
{
};

class invalid_hash : public std::exception
{
    OString maMessage;
public:

    invalid_hash(const OUString& rExpectedHash, const OUString& rReceivedHash)
    {
        OUString aMsg = "Invalid hash found.\nExpected: " + rExpectedHash + ";\nReceived: " + rReceivedHash;
        maMessage = OUStringToOString(aMsg, RTL_TEXTENCODING_UTF8);
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
    {
        OUString aMsg = "Invalid file size found.\nExpected: " + OUString::number(nExpectedSize) + ";\nReceived: " + OUString::number(nReceivedSize);
        maMessage = OUStringToOString(aMsg, RTL_TEXTENCODING_UTF8);
    }

    const char* what() const noexcept override
    {
        return maMessage.getStr();
    }
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

    if (rNode.child_count() < 4)
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
    aUpdateFile.nSize = static_cast<sal_uInt32>(aSizeNode.numeric_value());
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
    if (std::find(aRootKeys.begin(), aRootKeys.end(), "error") != aRootKeys.end())
    {
        throw invalid_update_info();
    }
    else if (std::find(aRootKeys.begin(), aRootKeys.end(), "response") != aRootKeys.end())
    {
        update_info aUpdateInfo;
        auto aMsgNode = aDocumentRoot.child("response");
        aUpdateInfo.aMessage = toOUString(aMsgNode.string_value().str());
        return aUpdateInfo;
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
  response->mpStream->WriteBytes(static_cast<char *>(ptr), real_size);
  response->maHash.update(static_cast<const unsigned char*>(ptr), real_size);
  return real_size;
}

std::string download_content(const OString& rURL, bool bFile, OUString& rHash)
{
    Updater::log("Download: " + rURL);
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
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // follow redirects

    std::string response_body;
    utl::TempFile aTempFile;
    WriteDataFile aFile(aTempFile.GetStream(StreamMode::WRITE));
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
                static_cast<void *>(&aFile));
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

    if (bFile)
        rHash = aFile.getHash();

    return response_body;
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
    OUString aHash;
    std::string temp_file = download_content(aURL, true, aHash);
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
        throw invalid_size(nFileSize, nSize);
    }

    if (aHash != rHash)
    {
        SAL_WARN("desktop.updater", "File hash don't match. File might be corrupted.");
        throw invalid_hash(rHash, aHash);
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
    OUString aBuildID("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("version") ":buildid}");
    rtl::Bootstrap::expandMacros(aBuildID);
    OUString aBuildTarget = "${_OS}_${_ARCH}";
    rtl::Bootstrap::expandMacros(aBuildTarget);
    OUString aChannel = officecfg::Office::Update::Update::UpdateChannel::get();
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
                CreateValidUpdateDir(aUpdateInfo);
                OUString aSeeAlsoURL = aUpdateInfo.aSeeAlsoURL;
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                        comphelper::ConfigurationChanges::create());
                officecfg::Office::Update::Update::SeeAlso::set(aSeeAlsoURL, batch);
                batch->commit();
            }
        }
    }
    catch (const invalid_update_info&)
    {
        SAL_WARN("desktop.updater", "invalid update information");
        Updater::log(OString("warning: invalid update info"));
    }
    catch (const error_updater&)
    {
        SAL_WARN("desktop.updater", "error during the update check");
        Updater::log(OString("warning: error by the updater"));
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

OUString Updater::getUpdateInfoURL()
{
    OUString aUpdateInfoURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/update.info");
    rtl::Bootstrap::expandMacros(aUpdateInfoURL);

    return aUpdateInfoURL;
}

OUString Updater::getUpdateInfoLog()
{
    OUString aUpdateInfoURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/updating.log");
    rtl::Bootstrap::expandMacros(aUpdateInfoURL);

    return aUpdateInfoURL;
}

OUString Updater::getPatchDirURL()
{
    OUString aPatchDirURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/patch/");
    rtl::Bootstrap::expandMacros(aPatchDirURL);

    return aPatchDirURL;
}

OUString Updater::getUpdateDirURL()
{
    OUString aUpdateDirURL("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/update_dir/");
    rtl::Bootstrap::expandMacros(aUpdateDirURL);

    return aUpdateDirURL;
}

OUString Updater::getExecutableDirURL()
{
    OUString aExeDir( "$BRAND_BASE_DIR/" LIBO_BIN_FOLDER "/" );
    rtl::Bootstrap::expandMacros(aExeDir);

    return aExeDir;
}

void Updater::log(const OUString& rMessage)
{
    OUString aUpdateLog = getUpdateInfoLog();
    SvFileStream aLog(aUpdateLog, StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteLine(OUStringToOString(rMessage, RTL_TEXTENCODING_UTF8));
}

void Updater::log(const OString& rMessage)
{
    OUString aUpdateLog = getUpdateInfoLog();
    SvFileStream aLog(aUpdateLog, StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteLine(rMessage);
}

void Updater::log(const char* pMessage)
{
    OUString aUpdateLog = getUpdateInfoLog();
    SvFileStream aLog(aUpdateLog, StreamMode::STD_READWRITE);
    aLog.Seek(aLog.Tell() + aLog.remainingSize()); // make sure we are at the end
    aLog.WriteCharPtr(pMessage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
