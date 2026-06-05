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
 * Implementation of storage backends and WOPI protocol.
 * Classes: StorageBase, LocalStorage
 */

#include <config.h>

#include "Storage.hpp"

#if !MOBILEAPP

#include <net/HttpRequest.hpp>
#include <wopi/WopiStorage.hpp>
#include <wsd/Auth.hpp>
#include <wsd/HostUtil.hpp>
#include <wsd/ProofKey.hpp>

#include <cassert>
#include <cerrno>

#endif // !MOBILEAPP

#include <common/CommandControl.hpp>
#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/TraceEvent.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/NetUtil.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/Exceptions.hpp>

#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include <chrono>
#include <memory>
#include <string>

#if MOBILEAPP
#ifdef IOS
#include <ios.h>
#elif defined(MACOS)
#include <macos.h>
#elif defined(__ANDROID__)
#include <androidapp.hpp>
#elif defined(_WIN32)
#include <windows.hpp>
#elif defined(QTAPP)
#include <qt.hpp>
#elif WASMAPP
#include <wasmapp.hpp>
#endif
#endif // MOBILEAPP

#if ENABLE_LOCAL_FILESYSTEM
bool StorageBase::FilesystemEnabled;
#endif

#if !MOBILEAPP

namespace {

std::string getLocalJailPath(const std::string& localStorePath, const std::string& jailPath)
{
    std::string localPath = jailPath;
    if (localPath[0] == '/')
    {
        // Remove the leading /
        localPath.erase(0, 1);
    }

    return FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces, localStorePath, std::move(localPath));
}

}

std::string StorageBase::getLocalRootPath() const
{
    return getLocalJailPath(_localStorePath, _jailPath);
}

std::string StorageBase::getJailPresetsPath() const
{
    return getLocalJailPath(_localStorePath, JAILED_CONFIG_ROOT);
}

#endif

void StorageBase::initialize()
{
#if !MOBILEAPP

#if ENABLE_LOCAL_FILESYSTEM
    FilesystemEnabled = ConfigUtil::getBool("storage.filesystem[@allow]", false);
#endif

    //parse wopi.storage.host only when there is no storage.wopi.alias_groups entry in config
    if (!ConfigUtil::has("storage.wopi.alias_groups"))
    {
        HostUtil::parseWopiHost();
    }

#if ENABLE_FEATURE_LOCK
    CommandControl::LockManager::parseLockedHost();
#endif

    const auto& app = Poco::Util::Application::instance();
    HostUtil::parseAliases(app.config());

    if (COOLWSD::IndirectionServerEnabled && COOLWSD::GeolocationSetup)
        HostUtil::parseAllowedWSOrigins();

#else // MOBILEAPP
    FilesystemEnabled = true;
#endif // MOBILEAPP
}

#if !MOBILEAPP

static bool isLocalhost(const std::string& targetHost)
{
    const std::string targetAddress = net::resolveHostAddress(targetHost);

    if (net::isLocalhost(targetHost))
    {
        LOG_INF("WOPI host [" << targetHost << "] is on the same host as the WOPI client: \""
                              << targetAddress << "\". Connection is allowed.");
        return true;
    }

    LOG_INF("WOPI host [" << targetHost << "] is not on the same host as the WOPI client: \""
                          << targetAddress << "\". Connection is not allowed.");
    return false;
}

#endif

StorageBase::StorageType StorageBase::validate(const Poco::URI& uri,
                                               [[maybe_unused]] bool takeOwnership)
{
    if (uri.isRelative() || uri.getScheme() == "file")
    {
        LOG_DBG("Public URI [" << Anonymizer::anonymizeUrl(uri.toString()) << "] is a file");

#if ENABLE_DEBUG
        if (std::getenv("FAKE_UNAUTHORIZED"))
        {
            LOG_DBG("FAKE_UNAUTHORIZED envar is set, unauthorized uri ["
                    << Anonymizer::anonymizeUrl(uri.toString()) << ']');
            return StorageBase::StorageType::Unauthorized;
        }
#endif

        if (takeOwnership)
        {
            LOG_DBG("Validated URI [" << Anonymizer::anonymizeUrl(uri.toString())
                                      << "] as Conversion");
            // Normalize the path.
            Poco::Path path = Poco::Path(uri.getPath());
            if (!path.isAbsolute() || !path.isFile() ||
                !path.makeAbsolute().toString().starts_with(COOLWSD::ChildRoot))
            {
                LOG_ERR("Invalid path to document to convert [" << uri.toString() << ']');
                return StorageBase::StorageType::Unsupported;
            }

            return StorageBase::StorageType::Conversion;
        }

#if ENABLE_LOCAL_FILESYSTEM
        if (FilesystemEnabled)
        {
            LOG_DBG("Validated URI [" << Anonymizer::anonymizeUrl(uri.toString())
                                      << "] as FileSystem");
            return StorageBase::StorageType::FileSystem;
        }

        LOG_DBG("Local Storage is disabled by default. Enable in the config file or on the "
                "command-line to enable.");
#else
        LOG_DBG("Local Storage is disabled in this build. Enable in the config file.");
#endif // ENABLE_LOCAL_FILESYSTEM
    }
#if !MOBILEAPP // Breaks IOS when removed.
    else if (HostUtil::isWopiEnabled())
    {
        const auto& targetHost = uri.getHost();
        HostUtil::setFirstHost(uri);
        if (HostUtil::allowedWopiHost(targetHost) || isLocalhost(targetHost))
        {
            LOG_DBG("Validated URI [" << Anonymizer::anonymizeUrl(uri.toString()) << "] as WOPI");
            return StorageBase::StorageType::Wopi;
        }

        // check if the IP address is in the list of allowed hosts
        const auto hostAddresses(net::resolveAddresses(targetHost));
        for (const auto& address : hostAddresses)
        {
            if (HostUtil::allowedWopiHost(address))
            {
                LOG_DBG("Validated URI [" << Anonymizer::anonymizeUrl(uri.toString())
                                          << "] as WOPI");
                return StorageBase::StorageType::Wopi;
            }
        }

        LOG_DBG("No acceptable WOPI hosts found matching the target host ["
                << targetHost << "] in config for URI [" << Anonymizer::anonymizeUrl(uri.toString())
                << ']');
        return StorageBase::StorageType::Unauthorized;
    }
#endif

    LOG_DBG("No Storage configured or invalid URI [" << Anonymizer::anonymizeUrl(uri.toString())
                                                     << ']');
    return StorageBase::StorageType::Unsupported;
}

std::unique_ptr<StorageBase> StorageBase::create(const Poco::URI& uri, const std::string& jailRoot,
                                                 const std::string& jailPath, bool takeOwnership,
                                                 const AdditionalFilePocoUris& additionalFileUrisPublic)
{
    // FIXME: By the time this gets called we have already sent to the client three
    // 'progress:' messages: "id":"find", "id":"connect" and "id":"ready". We should ideally do the checks
    // here much earlier. Also, using exceptions is lame and makes understanding the code harder,
    // but that is just my personal preference.

    std::unique_ptr<StorageBase> storage;
    if (UnitBase::isUnitTesting() && UnitWSD::get().createStorage(uri, jailRoot, jailPath, storage))
    {
        if (storage)
        {
            LOG_INF("Storage create hooked via UnitWSD");
            return storage;
        }
    }

    const StorageBase::StorageType type = validate(uri, takeOwnership);
    switch (type)
    {
        case StorageBase::StorageType::Unsupported:
            LOG_ERR("Unsupported URI [" << Anonymizer::anonymizeUrl(uri.toString())
                                        << "] or no storage configured");
            throw BadRequestException("No Storage configured or invalid URI " +
                                      Anonymizer::anonymizeUrl(uri.toString()) + ']');

            break;
        case StorageBase::StorageType::Unauthorized:
            LOG_ERR("No acceptable WOPI hosts found matching the target host [" << uri.getHost()
                                                                                << "] in config");
            throw UnauthorizedRequestException(
                "No acceptable WOPI hosts found matching the target host [" + uri.getHost() +
                "] in config");
            break;

        case StorageBase::StorageType::Conversion:
            return std::make_unique<LocalStorage>(uri, jailRoot, jailPath, /*takeOwnership=*/true, additionalFileUrisPublic);
            break;

#if ENABLE_LOCAL_FILESYSTEM
        case StorageBase::StorageType::FileSystem:
            return std::make_unique<LocalStorage>(uri, jailRoot, jailPath, takeOwnership);
            break;
#endif // ENABLE_LOCAL_FILESYSTEM

#if !MOBILEAPP
        case StorageBase::StorageType::Wopi:
            return std::make_unique<WopiStorage>(uri, jailRoot, jailPath);
            break;
#endif //!MOBILEAPP
    }

    throw BadRequestException("No Storage configured or invalid URI " +
                              Anonymizer::anonymizeUrl(uri.toString()) + ']');
}

std::atomic<unsigned> LocalStorage::LastLocalStorageId;

std::unique_ptr<LocalStorage::LocalFileInfo> LocalStorage::getLocalFileInfo()
{
    const Poco::Path path = getUri().getPath();
    LOG_DBG("Getting info for local uri [" << Anonymizer::anonymizeUrl(getUri().toString())
                                           << "], path ["
                                           << Anonymizer::anonymizeUrl(path.toString()) << "].");

    const FileUtil::Stat stat(path.toString());
    const std::chrono::system_clock::time_point lastModified = stat.modifiedTimepoint();

    setFileInfo(FileInfo(stat.size(), path.getFileName(), "LocalOwner",
                         Util::getIso8601FracformatTime(lastModified)));

    // Set automatic userid and username.
    const std::string userId = std::to_string(LastLocalStorageId++);
    std::string userNameString;

#if MOBILEAPP && !WASMAPP
    if (user_name != nullptr)
        userNameString = std::string(user_name);
#endif
    if (userNameString.empty())
        userNameString = "LocalUser#" + userId;

    return std::make_unique<LocalStorage::LocalFileInfo>("LocalUser" + userId, userNameString);
}

std::string LocalStorage::downloadStorageFileToLocal(const Authorization& /*auth*/,
                                                     LockContext& /*lockCtx*/,
                                                     const std::string& /*templateUri*/,
                                                     [[maybe_unused]]
                                                     AdditionalFilePaths& additionalFileLocalPaths)
{
#if !MOBILEAPP
    // /chroot/jailId/user/doc/childId/file.ext
    const std::string filename = Poco::Path(getUri().getPath()).getFileName();
    AdditionalFilePaths additionalFileFilenames;
    for (const auto& it : getAdditionalFileUris())
    {
        additionalFileFilenames[it.first] = Poco::Path(it.second.getPath()).getFileName();
    }
    setRootFilePath(Poco::Path(getLocalRootPath(), filename).toString());
    setRootFilePathAnonym(Anonymizer::anonymizeUrl(getRootFilePath()));
    LOG_INF("Public URI [" << Anonymizer::anonymizeUrl(getUri().getPath()) << "] jailed to ["
                           << getRootFilePathAnonym() << "].");
    AdditionalFilePaths additionalFileJailedFilePaths;
    for (const auto& it : additionalFileFilenames)
    {
        additionalFileJailedFilePaths[it.first] = Poco::Path(getLocalRootPath(), it.second).toString();
    }

    // Despite the talk about URIs it seems that _uri is actually just a pathname here
    const std::string publicFilePath = getUri().getPath();
    if (!Poco::File(publicFilePath).exists())
    {
        LOG_ERR("Local file URI [" << publicFilePath << "] invalid or doesn't exist.");
        throw BadRequestException("Invalid URI: " + getUri().toString());
    }
    AdditionalFilePaths additionalFilePublicFilePaths;
    for (const auto& it : getAdditionalFileUris())
    {
        additionalFilePublicFilePaths[it.first] = it.second.getPath();
    }

    // Make sure the path is valid.
    const Poco::Path downloadPath = Poco::Path(getRootFilePath()).parent();
    Poco::File(downloadPath).createDirectories();

    // Check for available space.
    if (!FileUtil::checkDiskSpace(downloadPath.toString()))
    {
        throw StorageSpaceLowException("Low disk space for " + getRootFilePathAnonym());
    }

    if (_isTemporaryFile)
    {
        try
        {
            // Neither link nor copy, just move, it's a temporary file.
            Poco::File(publicFilePath).moveTo(getRootFilePath());

            // Cleanup the directory after moving.
            const std::string dir = Poco::Path(publicFilePath).parent().toString();
            if (FileUtil::isEmptyDirectory(dir))
                FileUtil::removeFile(dir);

            for (const auto& it : additionalFilePublicFilePaths)
            {
                Poco::File(it.second).moveTo(additionalFileJailedFilePaths[it.first]);
                const std::string additionalFileDir = Poco::Path(it.second).parent().toString();
                if (FileUtil::isEmptyDirectory(additionalFileDir))
                    FileUtil::removeFile(additionalFileDir);
            }
        }
        catch (const Poco::Exception& exc)
        {
            LOG_ERR("Failed to move [" << Anonymizer::anonymizeUrl(publicFilePath) << "] to ["
                                       << getRootFilePathAnonym() << "]: " << exc.displayText());
        }
    }

    if (!FileUtil::Stat(getRootFilePath()).exists())
    {
        // Try to link.
        LOG_INF("Linking " << Anonymizer::anonymizeUrl(publicFilePath) << " to "
                           << getRootFilePathAnonym());
        if (!Poco::File(getRootFilePath()).exists()
            && link(publicFilePath.c_str(), getRootFilePath().c_str()) == -1)
        {
            // Failed
            LOG_INF_SYS("link(\"" << Anonymizer::anonymizeUrl(publicFilePath) << "\", \""
                                  << getRootFilePathAnonym() << "\") failed. Will copy");
        }
    }

    try
    {
        // Fallback to copying.
        if (!FileUtil::Stat(getRootFilePath()).exists())
        {
            FileUtil::copyFileTo(publicFilePath, getRootFilePath());
            _isCopy = true;
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOG_ERR("copyTo(\"" << Anonymizer::anonymizeUrl(publicFilePath) << "\", \""
                            << getRootFilePathAnonym() << "\") failed: " << exc.displayText());
        throw;
    }

    setDownloaded(true);

    // Now return the jailed path.
    if (COOLWSD::NoCapsForKit)
    {
        for (const auto& it : additionalFileJailedFilePaths)
            additionalFileLocalPaths[it.first] = it.second;
        return getRootFilePath();
    }
    else
    {
        for (const auto& it : additionalFileFilenames)
            additionalFileLocalPaths[it.first] = Poco::Path(getJailPath(), it.second).toString();
        return Poco::Path(getJailPath(), filename).toString();
    }

#else // MOBILEAPP

    // In the mobile app we use no jail
    setRootFilePath(getUri().getPath());

    return getRootFilePath();
#endif
}

std::size_t LocalStorage::uploadLocalFileToStorageAsync(
    const Authorization& /*auth*/, LockContext& /*lockCtx*/, const std::string& /*saveAsPath*/,
    const std::string& /*saveAsFilename*/, bool /*isRename*/, const Attributes&,
    const std::shared_ptr<SocketPoll>&,
    const AsyncUploadCallback& asyncUploadCallback)
{
    const std::string path = getUri().getPath();

    // Assume failure by default.
    UploadResult res = UploadResult(UploadResult::Result::FAILED, "Internal error");
    std::size_t size = 0;
    try
    {
        LOG_TRC("Copying local file to local file storage (isCopy: " << _isCopy << ") for "
                                                                     << getRootFilePathAnonym());

        // Copy the file back.
        if (_isCopy && Poco::File(getRootFilePathUploading()).exists())
            FileUtil::copyFileTo(getRootFilePathUploading(), path);

        const FileUtil::Stat stat(path); // Don't move 'path' as it's used in the catch.
        size = stat.size();

        // update its fileinfo object. This is used later to check if someone else changed the
        // document while we are/were editing it
        setLastModifiedTime(Util::getIso8601FracformatTime(stat.modifiedTimepoint()));
        LOG_TRC("New FileInfo modified time in storage " << getLastModifiedTime());
        res = UploadResult(UploadResult::Result::OK);
    }
    catch (const Poco::Exception& exc)
    {
        LOG_ERR("copyTo(\"" << getRootFilePathAnonym() << "\", \"" << Anonymizer::anonymizeUrl(path)
                            << "\") failed: " << exc.displayText());
        // Default UploadResult is failure.
    }

    if (asyncUploadCallback)
    {
        asyncUploadCallback(AsyncUpload(AsyncUpload::State::Complete, std::move(res)));
    }

    return size;
}

void LockContext::initSupportsLocks()
{
    if constexpr (Util::isMobileApp())
        _supportsLocks = false;
    else
    {
        if (_supportsLocks)
            return;

        // first time token setup
        _supportsLocks = true;
        _lockToken = "cool-lock" + Util::rng::getHexString(8);
    }
}

bool LockContext::needsRefresh(const std::chrono::steady_clock::time_point now) const
{
    return _supportsLocks && isLocked() && _refreshSeconds > std::chrono::seconds::zero() &&
           (now - _lastLockTime) >= _refreshSeconds;
}

void LockContext::dumpState(std::ostream& os) const
{
    if (!_supportsLocks)
    {
        os << "\n  LockContext: Unsupported";
        return;
    }

    os << "\n  LockContext:";
    os << "\n    locked: " << isLocked();
    os << "\n    token: " << _lockToken;
    os << "\n    last locked: " << Util::getSteadyClockAsString(_lastLockTime);
}

#if !MOBILEAPP

/// A helper class to invoke the AsyncUploadCallback
/// when it exits its scope.
/// By default it invokes the callback with a failure state.
class ScopedInvokeAsyncUploadCallback
{
public:
    ScopedInvokeAsyncUploadCallback(StorageBase::AsyncUploadCallback asyncUploadCallback)
        : _asyncUploadCallback(std::move(asyncUploadCallback))
        , _arg(StorageBase::AsyncUpload(
              StorageBase::AsyncUpload::State::Error,
              StorageBase::UploadResult(StorageBase::UploadResult::Result::FAILED)))
    {
    }

    ~ScopedInvokeAsyncUploadCallback()
    {
        if (_asyncUploadCallback)
            _asyncUploadCallback(_arg);
    }

    /// Set a new callback argument.
    void setArg(StorageBase::AsyncUpload arg) { _arg = std::move(arg); }

private:
    StorageBase::AsyncUploadCallback _asyncUploadCallback;
    StorageBase::AsyncUpload _arg;
};

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
