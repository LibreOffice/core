/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Static file serving for browser UI resources.
 * Classes: FileServerRequestHandler
 */

#pragma once

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/Net/PartHandler.h>

#include <string>
#include <unordered_map>

class RequestDetails;

namespace Poco
{
namespace Net
{
class HTTPRequest;
class HTTPResponse;
class HTTPBasicCredentials;
} // namespace Net

} // namespace Poco

/// Represents a file that is preprocessed for variable
/// expansion/replacement before serving.
class PreProcessedFile
{
    friend class FileServeTests;

public:
    enum class SegmentType : char
    {
        Data,
        Variable,
        CommentedVariable
    };

    PreProcessedFile(std::string filename, const std::string& data);

    const std::string& filename() const { return _filename; }
    std::size_t size() const { return _size; }

    /// Substitute variables per the given map.
    std::string substitute(const std::unordered_map<std::string, std::string>& values);

private:
    const std::string _filename; ///< Filename on disk, with extension.
    const std::size_t _size; ///< Number of bytes in original file.
    /// The segments of the file in <IsVariable, Data> pairs.
    std::vector<std::pair<SegmentType, std::string>> _segments;
};

inline std::ostream& operator<<(std::ostream& os, const PreProcessedFile::SegmentType type)
{
    switch (type)
    {
        case PreProcessedFile::SegmentType::Data:
            os << "Data";
            break;
        case PreProcessedFile::SegmentType::Variable:
            os << "Variable";
            break;
        case PreProcessedFile::SegmentType::CommentedVariable:
            os << "CommentedVariable";
            break;
    }

    return os;
}

/// Handles file requests over HTTP(S).
class FileServerRequestHandler
{
public:
    /// The WOPI URL and authentication details,
    /// as extracted from the cool.html file-serving request.
    class ResourceAccessDetails
    {
    public:
        ResourceAccessDetails() = default;

        ResourceAccessDetails(std::string wopiSrc, std::string accessToken,
                              std::string noAuthHeader,
                              std::string permission, std::string wopiConfigId)
            : _wopiSrc(std::move(wopiSrc))
            , _accessToken(std::move(accessToken))
            , _noAuthHeader(std::move(noAuthHeader))
            , _permission(std::move(permission))
            , _wopiConfigId(std::move(wopiConfigId))
        {
        }

        bool isValid() const { return !_wopiSrc.empty() && !_accessToken.empty(); }

        const std::string& wopiSrc() const { return _wopiSrc; }
        const std::string& accessToken() const { return _accessToken; }
        const std::string& noAuthHeader() const { return _noAuthHeader; }
        const std::string& permission() const { return _permission; }
        // only exists in debugging mode, so built-in wopi debuging server
        // can support multiple 'shared' configs depending on configid=something
        const std::string& wopiConfigId() const { return _wopiConfigId; }

    private:
        std::string _wopiSrc;
        std::string _accessToken;
        std::string _noAuthHeader;
        std::string _permission;
        std::string _wopiConfigId;
    };

private:
    friend class FileServeTests; // for unit testing

    static std::string getRequestPathname(const Poco::Net::HTTPRequest& request,
                                          const RequestDetails& requestDetails);

    ResourceAccessDetails preprocessFile(const Poco::Net::HTTPRequest& request,
                                         http::Response& httpResponse,
                                         const RequestDetails& requestDetails,
                                         std::istream& message,
                                         const std::shared_ptr<StreamSocket>& socket);
    void preprocessWelcomeFile(const Poco::Net::HTTPRequest& request,
                               http::Response& httpResponse,
                               const RequestDetails& requestDetails,
                               std::istream& message,
                               const std::shared_ptr<StreamSocket>& socket);

    void replaceServiceRoot(const Poco::Net::HTTPRequest& request, http::Response& httpResponse,
                            const RequestDetails& requestDetails,
                            const std::shared_ptr<StreamSocket>& socket);

    static void uploadFileToIntegrator(const Poco::Net::HTTPRequest& request,
                                       std::istream& message,
                                       const std::shared_ptr<StreamSocket>& socket);

    static void fetchWopiSettingConfigs(const Poco::Net::HTTPRequest& request,
                                        std::istream& message,
                                        const std::shared_ptr<StreamSocket>& socket);

    static void fetchSettingFile(const Poco::Net::HTTPRequest& request,
                                   std::istream& message,
                                   const std::shared_ptr<StreamSocket>& socket);
    static void fetchModels(const Poco::Net::HTTPRequest& request, std::istream& message,
                            const std::shared_ptr<StreamSocket>& socket);

    static void deleteWopiSettingConfigs(const Poco::Net::HTTPRequest& request,
                                         std::istream& message,
                                         const std::shared_ptr<StreamSocket>& socket);

    void preprocessAdminFile(const Poco::Net::HTTPRequest& request,
                             http::Response& httpResponse,
                             const RequestDetails& requestDetails,
                             const std::shared_ptr<StreamSocket>& socket);

    static void updateThemeResources(std::string& fileContent,
                                    const std::string& responseRoot,
                                    const std::string& theme,
                                    const Poco::Util::AbstractConfiguration& config);

    void preprocessIntegratorAdminFile(const Poco::Net::HTTPRequest& request,
                                       http::Response& httpResponse,
                                       const RequestDetails& requestDetails,
                                       std::istream& message,
                                       const std::shared_ptr<StreamSocket>& socket);

    /// Construct a JSON to be accepted by the cool.html from a list like
    /// UIMode=classic;TextRuler=true;PresentationStatusbar=false
    /// that is passed as "ui_defaults" hidden input during the iframe setup.
    /// Also returns the UIMode from uiDefaults in uiMode output param
    /// and SavedUIState as a stringified boolean (default "true")
    static std::string uiDefaultsToJSON(const std::string& uiDefaults, std::string& uiMode, std::string& uiTheme, std::string& savedUIState);

    static std::string checkFileInfoToJSON(const std::string& checkfileFileInfo);

    static std::string cssVarsToStyle(const std::string& cssVars);

public:
    FileServerRequestHandler(const std::string& root);
    ~FileServerRequestHandler();

    /// Evaluate if the cookie exists and returns it when it does.
    static bool isAdminLoggedIn(const Poco::Net::HTTPRequest& request, std::string& jwtToken);

    /// Evaluate if the cookie exists, and if not, ask for the credentials.
    static bool isAdminLoggedIn(const Poco::Net::HTTPRequest& request, http::Response& response);

    /// Authenticate the admin.
    static bool authenticateAdmin(const Poco::Net::HTTPBasicCredentials& credentials,
                                  http::Response& response, std::string& jwtToken);

    bool handleRequest(const Poco::Net::HTTPRequest& request,
                       const RequestDetails& requestDetails,
                       std::istream& message,
                       const std::shared_ptr<StreamSocket>& socket,
                       ResourceAccessDetails& accessDetails);

    void readDirToHash(const std::string& basePath, const std::string& path);

    const std::string *getCompressedFile(const std::string &path);

    const std::string *getUncompressedFile(const std::string &path);

    /// If configured and necessary, sets the HSTS headers.
    static void hstsHeaders([[maybe_unused]] http::Response& response)
    {
        // HSTS hardening. Disabled in debug builds.
        if constexpr (!Util::isDebugEnabled())
        {
            if (ConfigUtil::isSslEnabled() || ConfigUtil::isSSLTermination())
            {
                if (ConfigUtil::getBool("ssl.sts.enabled", false))
                {
                    static const auto maxAge =
                        ConfigUtil::getInt("ssl.sts.max_age", 31536000); // Default 1 year.
                    response.add("Strict-Transport-Security",
                                 "max-age=" + std::to_string(maxAge) + "; includeSubDomains");
                }
            }
        }
    }

    void dumpState(std::ostream& os);

private:
    using FileHashMap_t = std::unordered_map<std::string, std::pair<std::string, std::string>>;
    FileHashMap_t FileHash;
    static void sendError(http::StatusCode errorCode, const std::string& requestPath,
                          const std::shared_ptr<StreamSocket>& socket,
                          const std::string& shortMessage, const std::string& longMessage,
                          const std::string& extraHeader = std::string());
};

class FilePartHandler : public Poco::Net::PartHandler
{
public:
    void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override;
    const std::string& getFileName() const { return _fileName; }
    const std::string& getFilePath() const { return _filePath; }
    /// Take shared ownership of the temp dir (and file within it).
    /// Caller keeps this alive until the file is no longer needed.
    std::shared_ptr<FileUtil::OwnedFile> getFileOwnership() const { return _fileDir; }

private:
    std::string _fileName;
    /// Path to temp file containing the uploaded content.
    std::string _filePath;
    /// Temp directory holding the file, removed recursively on destruction.
    std::shared_ptr<FileUtil::OwnedFile> _fileDir;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
