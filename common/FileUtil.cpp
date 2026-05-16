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
 * Cross-platform file and directory utilities.
 * Functions: copy(), removeFile(), stat(), updateTimestamps()
 */

#include <config.h>

#include "FileUtil.hpp"

#include <common/Anonymizer.hpp>
#include <common/Log.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

namespace FileUtil
{
    DownloadJailPath createDownloadJailPath(const std::string& jailDocRoot,
                                            const std::string& filename)
    {
        DownloadJailPath result;
        result.tmpDir = Util::rng::getFilename(64);
        createDirectory(jailDocRoot + '/' + result.tmpDir);
        result.urlInJail = result.tmpDir + '/' + filename;
        result.absolutePath = jailDocRoot + result.urlInJail;
        return result;
    }

    namespace
    {
    // Handle short writes and EINTR
    ssize_t writeBuffer(int to, const char* buffer, size_t size, const std::string& toPath)
    {
        size_t count = size;
        const char* ptr = buffer;
        while (count)
        {
            ssize_t written;
            while ((written = writeToFD(to, ptr, count)) < 0 && errno == EINTR)
                LOG_TRC("EINTR writing to " << anonymizeUrl(toPath));
            if (written < 0)
                return -1;
            count -= written;
            ptr += written;
        }
        return size;
    }
    } // namespace

    bool copy(const std::string& fromPath, const std::string& toPath, bool log, bool throw_on_error,
              LOG_CAPTURE_CALLER)
    {
        int from = -1, to = -1;
        try
        {
            from = openFileAsFD(fromPath, O_RDONLY);
            if (from < 0)
                throw std::runtime_error("Failed to open src " + anonymizeUrl(fromPath));

            struct stat st;
            if (fstat(from, &st) != 0)
                throw std::runtime_error("Failed to fstat src " + anonymizeUrl(fromPath));

            to = openFileAsFD(toPath, O_CREAT | O_TRUNC | O_WRONLY, st.st_mode);
            if (to < 0)
                throw std::runtime_error("Failed to open dest " + anonymizeUrl(toPath));

            // Logging may be redundant and/or noisy.
            if (log)
                LOG_INF("Copying " << st.st_size << " bytes from " << anonymizeUrl(fromPath)
                                   << " to " << anonymizeUrl(toPath));

            char buffer[64 * 1024];

            off_t bytesIn = 0;
            do
            {
                ssize_t n;
                while ((n = readFromFD(from, buffer, sizeof(buffer))) < 0 && errno == EINTR)
                    LOG_TRC("EINTR reading from " << anonymizeUrl(fromPath));
                if (n < 0)
                    throw std::runtime_error("Failed to read from " + anonymizeUrl(fromPath)
                                             + " at " + std::to_string(bytesIn) + " bytes in");

                bytesIn += n;
                if (n == 0) // EOF
                    break;
                assert (off_t(sizeof (buffer)) >= n);

                if (writeBuffer(to, buffer, n, toPath) < 0)
                {
                    throw std::runtime_error("Failed to write " + std::to_string(n)
                                             + " bytes to " + anonymizeUrl(toPath) + " at "
                                             + std::to_string(bytesIn) + " bytes into "
                                             + anonymizeUrl(fromPath));
                }
            } while (true);
            if (bytesIn != st.st_size)
            {
                LOG_WRN("Unusual: file " << anonymizeUrl(fromPath) << " changed size "
                        "during copy from " << st.st_size << " to " << bytesIn);
            }
            closeFD(from);
            closeFD(to);
            return true;
        }
        catch (const std::exception& ex)
        {
            std::ostringstream oss;
            oss << "Error while copying from " << anonymizeUrl(fromPath) << " to "
                << anonymizeUrl(toPath) << ": " << ex.what();
            const std::string err = oss.str();
            LOG_ERR(err);
            closeFD(from);
            closeFD(to);
            unlinkFile(toPath);
            if (throw_on_error)
                throw std::runtime_error(err);
        }

        return false;
    }

    std::string createRandomTmpDir(std::string root)
    {
        return createTmpDir("cool-" + Util::rng::getFilename(16), std::move(root));
    }

    std::string createTmpDir(const std::string& dirName, std::string root)
    {
        if (root.empty())
            root = getSysTempDirectoryPath();

        Poco::File(root).createDirectories();

        // Don't const to allow for automatic move on return.
        std::string newTmp = root + '/' + dirName;
        if (makeDirectory(newTmp) < 0)
        {
            LOG_SYS("Failed to create temp directory [" << newTmp << ']');
            return root;
        }
        return newTmp;
    }

    bool copyAtomic(const std::string& fromPath, const std::string& toPath, bool preserveTimestamps,
                    LOG_CAPTURE_CALLER)
    {
        const std::string randFilename = toPath + Util::rng::getFilename(12);
        if (copy(fromPath, randFilename, /*log=*/false, /*throw_on_error=*/false))
        {
            if (preserveTimestamps)
            {
                const Stat st(fromPath);
#ifndef _WIN32
                updateTimestamps(randFilename,
#if defined(IOS) || defined(MACOS)
                                 st.sb().st_atimespec, st.sb().st_mtimespec
#else
                                 st.sb().st_atim, st.sb().st_mtim
#endif
                                 );
#endif
            }

            // Now rename atomically, replacing any existing files with the same name.
            if (rename(randFilename.c_str(), toPath.c_str()) == 0)
                return true;

            LOG_SYS("Failed to copy [" << fromPath << "] -> [" << toPath
                                       << "] while atomically renaming:");
            removeFile(randFilename, false); // Cleanup.
        }

        return false;
    }

    bool compareFileContents(const std::string& rhsPath, const std::string& lhsPath)
    {
        std::ifstream rhs;
        openFileToIFStream(rhsPath, rhs, std::ifstream::binary | std::ifstream::ate);
        if (rhs.fail())
            return false;

        std::ifstream lhs;
        openFileToIFStream(lhsPath, lhs, std::ifstream::binary | std::ifstream::ate);
        if (lhs.fail())
            return false;

        if (rhs.tellg() != lhs.tellg())
            return false;

        rhs.seekg(0, std::ifstream::beg);
        lhs.seekg(0, std::ifstream::beg);
        return std::equal(std::istreambuf_iterator<char>(rhs.rdbuf()),
                          std::istreambuf_iterator<char>(),
                          std::istreambuf_iterator<char>(lhs.rdbuf()));
    }

    void copyDirectoryRecursive(std::string_view srcDir, std::string_view destDir, bool log)
    {
        namespace fs = std::filesystem;
        try
        {
            for (const auto& entry : fs::recursive_directory_iterator(srcDir))
            {
                // Calculate relative path from source
                const auto relativePath = fs::relative(entry.path(), srcDir);
                const auto targetPath = fs::path(destDir) / relativePath;

                // Create directory with writable permissions (default)
                if (entry.is_directory())
                    fs::create_directories(targetPath);
                else if (entry.is_regular_file())
                    FileUtil::copy(entry.path().string(), targetPath.string(), log, false);
            }
        }
        catch (const std::exception& ex)
        {
            LOG_ERR("Failed to copy srcDir[" << srcDir << "to destDir[" << destDir
                                             << "] with error[" << ex.what() << ']');
        }
    }

    std::unique_ptr<std::vector<char>> readFile(const std::string& path, int maxSize)
    {
        auto data = std::make_unique<std::vector<char>>(maxSize);
        data->resize(0);
        return (readFile(path, *data, maxSize) >= 0) ? std::move(data) : nullptr;
    }

    std::string buildLocalPathToJail(bool usingMountNamespaces, std::string localStorePath, std::string localPath)
    {
        // Use where mountJail of kit/Kit.cpp mounts /tmp for this path *from* rather than
        // where it is mounted *to*, so this process doesn't need the mount visible to it
        if (usingMountNamespaces && !localPath.empty())
        {
            Poco::Path jailPath(localPath);
            const std::string jailPathDir = jailPath[0];
            if (jailPathDir == "tmp")
            {
                jailPath.popFrontDirectory();

                Poco::Path localStorageDir(localStorePath);
                localStorageDir.makeDirectory();
                const std::string jailId = localStorageDir[localStorageDir.depth() - 1];
                localStorageDir.popDirectory();

                localStorageDir.pushDirectory(jailPathDir);

                std::string tmpMapping("cool-");
                tmpMapping.append(jailId);

                localStorageDir.pushDirectory(tmpMapping);

                localStorePath = localStorageDir.toString();

                localPath = jailPath.toString();
            }
        }

        // /chroot/jailId/user/doc/childId
        const Poco::Path rootPath = Poco::Path(localStorePath, localPath);
        Poco::File(rootPath).createDirectories();

        return rootPath.toString();
    }

    std::pair<std::string, std::string> buildPathsToJail(bool usingMountNamespaces, bool noCapsForKit,
                                                         std::string localJailRoot, std::string jailDir)
    {
        std::string localDir = FileUtil::buildLocalPathToJail(
            usingMountNamespaces, std::move(localJailRoot), jailDir);

        // This is the location of the file as seen by the kit, the location as
        // seen inside the jail. Ideally (and typically) noCaps is false, and
        // the path inside the jail is simply 'jailDir'. But if noCaps is true,
        // then chroot isn't possible and the jail is forced to work with the
        // same paths as used outside the jail.
        if (noCapsForKit)
            jailDir = localDir;

        return std::make_pair(localDir, jailDir);
    }

    ssize_t read(int fd, void* buf, size_t nbytes)
    {
        char* p = static_cast<char*>(buf);

        while (nbytes)
        {
            ssize_t n = readFromFD(fd, p, nbytes);
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;

                return -1; // Error.
            }

            if (n == 0) // EOF.
                break;

            assert(n >= 0 && "Expected a positive read byte-count");
            assert(static_cast<size_t>(n) <= nbytes && "Unexpectedly read more than requested");

            nbytes -= n;
            p += n;
        }

        return p - static_cast<char*>(buf);
    }

} // namespace FileUtil

#if !MOBILEAPP

namespace
{

    struct fs
    {
        fs(const std::string& path, dev_t dev)
            : _path(path), _dev(dev)
        {
        }

        const std::string& getPath() const { return _path; }

        dev_t getDev() const { return _dev; }

    private:
        std::string _path;
        dev_t _dev;
    };

    struct fsComparator
    {
        bool operator() (const fs& lhs, const fs& rhs) const
        {
            return (lhs.getDev() < rhs.getDev());
        }
    };

    static std::mutex fsmutex;
    static std::set<fs, fsComparator> filesystems;

} // anonymous namespace

#endif

namespace FileUtil
{
#if !MOBILEAPP
    void registerFileSystemForDiskSpaceChecks(const std::string& path)
    {
        const std::string::size_type lastSlash = path.rfind('/');
        assert(path.empty() || lastSlash != std::string::npos);
        if (lastSlash != std::string::npos)
        {
            const std::string dirPath = path.substr(0, lastSlash + 1) + '.';
            LOG_INF("Registering filesystem for space checks: [" << dirPath << ']');

            std::lock_guard<std::mutex> lock(fsmutex);

            struct stat s;
            if (stat(dirPath.c_str(), &s) == 0)
            {
                filesystems.insert(fs(dirPath, s.st_dev));
            }
        }
    }

    std::string checkDiskSpaceOnRegisteredFileSystems(const bool cacheLastCheck)
    {
        static std::chrono::steady_clock::time_point lastCheck;
        static std::string lastResult;
        std::chrono::steady_clock::time_point now(std::chrono::steady_clock::now());

        std::lock_guard<std::mutex> lock(fsmutex);

        if (cacheLastCheck)
        {
            // Don't check more often than once a minute
            if ((now - lastCheck) < std::chrono::minutes(1))
                return lastResult;

            lastCheck = now;
        }

        for (const auto& i: filesystems)
        {
            if (!checkDiskSpace(i.getPath()))
            {
                if (cacheLastCheck)
                    lastResult = i.getPath();
                return i.getPath();
            }
        }

        if (cacheLastCheck)
            lastResult = std::string();
        return std::string();
    }
#endif

    bool checkDiskSpace(const std::string& path)
    {
        assert(!path.empty());

        if constexpr (!Util::isMobileApp())
        {
            bool hookResult = true;
            if (UnitBase::get().filterCheckDiskSpace(path, hookResult))
                return hookResult;
        }

        // we should be able to run just OK with 5GB for production or 1GB for development
        constexpr int64_t gb = Util::isDebugEnabled() ? 1 : 5;

        constexpr int64_t ENOUGH_SPACE = gb*1024*1024*1024;

        return platformDependentCheckDiskSpace(path, ENOUGH_SPACE);

        return true;
    }

    /// Anonymize the basename of filenames, preserving the path and extension.
    std::string anonymizeUrl(const std::string& url) { return Anonymizer::anonymizeUrl(url); }

    /// Anonymize user names and IDs.
    /// Will use the Obfuscated User ID if one is provided via WOPI.
    std::string anonymizeUsername(const std::string& username)
    {
        return Anonymizer::anonymize(username);
    }

    std::string extractFileExtension(const std::string& path)
    {
        return Util::splitLast(path, '.', true).second;
    }

} // namespace FileUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
