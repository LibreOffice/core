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

#pragma once

#include <common/Log.hpp>

#include <Poco/Path.h>

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/stat.h>

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#if !defined(S_ISLNK)
#if defined(S_IFMT) && defined(S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(m) 0
#endif
#endif

#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

namespace FileUtil
{
    // Wrappers for actual file handling library API. Needed because the file names we handle are in
    // UTF-8, and on Windows we can't pass such to C and C++ library APIs. We need to convert to
    // UTF-16 strings and call the proper wide character APIs.

    // Also needed because Visual Studio insists on claiming that some POSIXy functions are "deprecated" and
    // wants you to call the variant prefixed with an underscore instead, for example _close().

    // As open(). Returns the file descriptor (which on Windows is just a thing the C library knows
    // about, not the OS). On error returns -1 and sets errno.
    int openFileAsFD(const std::string& file, int oflag, int mode = 0);

    // As read() and write().
    int readFromFD(int fd, void *buf, size_t nbytes);
    int writeToFD(int fd, const void *buf, size_t nbytes);

    // As close().
    int closeFD(int fd);

    // As std::ifstream::open.
    void openFileToIFStream(const std::string& file, std::ifstream& stream, std::ios_base::openmode mode = std::ios_base::in);

    // As std::ofstream::open.
    void openFileToOFStream(const std::string& file, std::ofstream& stream, std::ios_base::openmode mode = std::ios_base::out);

    // As stat().
    int getStatOfFile(const std::string& file, struct stat& sb);

    // As lstat().
    int getLStatOfFile(const std::string& file, struct stat& sb);

    // Wraps unlink()
    int unlinkFile(const std::string& file);

    // Wraps mkdir(dir.c_str(), S_IRWXU)
    int makeDirectory(const std::string& dir);

    // Wraps std::filesystem::create_directory.
    void createDirectory(std::string_view dir);

    /// Wraps std::filesystem::create_directories.
    void createDirectories(std::string_view dir);

    /// Returns true iff the path given is writable by our *real* UID.
    /// On Windows "real UID" is meaningless.
    bool isWritable(const char* path);

    /// Update the access-time and modified-time metadata for the given file.
    /// Not implemented on Windows.
    bool updateTimestamps(const std::string& filename, timespec tsAccess, timespec tsModified);

    // End of wrappers for platform-dependent API.

    /// Used for anonymizing URLs
    void setUrlAnonymization(bool anonymize, std::uint64_t salt);

    /// Anonymize the basename of filenames, preserving the path and extension.
    std::string anonymizeUrl(const std::string& url);

    /// Anonymize user names and IDs.
    /// Will use the Obfuscated User ID if one is provided via WOPI.
    std::string anonymizeUsername(const std::string& username);

    // Components of a per-download tmp path under the kit jail's document root.
    // WSD recovers the file from a downloadId by looking up the registered URL
    // (a path relative to the jail doc root) and joining it back with the jail
    // (see ClientRequestDispatcher's GET handler under /cool/.../<downloadId>).
    struct DownloadJailPath
    {
        std::string tmpDir;       // the random dir name; doubles as downloadId
        std::string urlInJail;    // tmpDir + "/" + filename - relative to jailDocRoot
        std::string absolutePath; // jailDocRoot + urlInJail - the path inside the jail
    };

    // Create a fresh random tmp dir under jailDocRoot and return path components
    // for delivering a saveAs/export to that location plus its subsequent download.
    DownloadJailPath createDownloadJailPath(const std::string& jailDocRoot,
                                            const std::string& filename);

    /// return the local path to the jailPath under localJailRoot
    /// localJailRoot /chroot/jailId
    /// jailPath /tmp/user/doc/childId
    /// with usingMountNamespaces false then simply return:
    /// -> /chroot/jailId/tmp/user/doc/childId
    /// otherwise replaces jailPath's in /tmp with the tmp dir that is mounted
    /// from, e.g. return:
    /// -> /chroot/tmp/cool-jailId/tmp/user/doc/childId
    std::string buildLocalPathToJail(bool usingMountNamespaces, std::string localJailRoot, std::string jailPath);

    // returns two strings
    //   the first is the local path to the jailPath under localJailRoot as seen outside the jail
    //   the second is that path as seen inside the jail, taking into account if capabilities are available.
    std::pair<std::string, std::string> buildPathsToJail(bool usingMountNamespaces, bool noCapsForKit,
                                                         std::string localJailRoot, std::string jailDir);

    // We work around some of the mess of using the same sources both on the server side and in unit
    // tests with conditional compilation based on BUILDING_TESTS.

    // Add the file system that 'path' is located on to a list of file systems that are periodically
    // checked for available space. The list is initially empty.
    void registerFileSystemForDiskSpaceChecks(const std::string& path);

    // Perform the check. If the free space on any of the registered file systems is below 5%, call
    // 'alertAllUsers("internal", "diskfull")'. The check will be made no more often than once a
    // minute if cacheLastCheck is set to true.
    std::string checkDiskSpaceOnRegisteredFileSystems(bool cacheLastCheck = true);

    // Check disk space on a specific file system, the one where 'path' is located. This does not
    // add that file system to the list used by 'registerFileSystemForDiskSpaceChecks'. If the free
    // space on the file system is below 5%, return false, otherwise true. Note that this function
    // does not call 'alertAllUsers'.
    bool checkDiskSpace(const std::string& path);

    bool platformDependentCheckDiskSpace(const std::string& path, int64_t enoughSpace);

    /// Safely remove a file or directory.
    /// Suppresses exception when the file is already removed.
    /// This can happen when there is a race (unavoidable) or when
    /// we don't care to check before we remove (when no race exists).
    void removeFile(const std::string& path, bool recursive = false);

    inline void removeFile(const Poco::Path& path, const bool recursive = false)
    {
        removeFile(path.toString(), recursive);
    }

    /// Remove empty directories recursively.
    /// We seem to leave behind empty directories in jails and that causes a lot of noise.
    void removeEmptyDirTree(const std::string& path);

    /// Returns true iff the directory is empty (or doesn't exist).
    bool isEmptyDirectory(const char* path);
    inline bool isEmptyDirectory(const std::string& path) { return isEmptyDirectory(path.c_str()); }

    inline bool isWritable(const std::string& path) { return isWritable(path.c_str()); }

    /// Copy the source file to the target.
    bool copy(const std::string& fromPath, const std::string& toPath, bool log, bool throw_on_error,
              LOG_CAPTURE_CALLER_DECLARATION);

    /// Atomically copy a file and optionally preserve its timestamps.
    /// The file is copied with a temporary name, and then atomically renamed.
    /// NOTE: toPath must be a valid filename, not a directory.
    /// Does not log (except errors), does not throw. Returns true on success.
    bool copyAtomic(const std::string& fromPath, const std::string& toPath, bool preserveTimestamps,
                    LOG_CAPTURE_CALLER_DECLARATION);

    /// Copy a file from @fromPath to @toPath, throws on failure.
    inline void copyFileTo(const std::string& fromPath, const std::string& toPath)
    {
        copy(fromPath, toPath, /*log=*/true, /*throw_on_error=*/true);
    }

    /// Try to hard-link, and fallback to copying it linking fails.
    /// Returns true iff either linking or copying succeeds.
    /// Platform-dependent implementations.
    bool linkOrCopyFile(const std::string& source, const std::string& newPath);

    /// Changes the various envars used for the system temporary-files directory.
    /// Used to privatize the directory we use for temporary files,
    /// which some libraries use these envars to query.
    void setSysTempDirectoryPath(const std::string& path);

    /// Wraps std::filesystem::temp_directory_path(), and if that fails, uses obvious fallbacks.
    /// Returns as UTF-8 on Windows. (And surely also on any sane Unix?)
    std::string getSysTempDirectoryPath();

    /// Create randomized temporary directory in the root provided.
    /// See createTmpDir for details.
    std::string createRandomTmpDir(std::string root = std::string());

    /// Create a temporary directory in the root provided
    /// with S_IRWXU (read, write, and execute by owner) permissions.
    /// If root is empty, the current system temp directory is used.
    std::string createTmpDir(const std::string& dirName, std::string root = std::string());

    /// Returns the realpath(3) of the provided path. This also has a separate implementation for
    /// Windows.
    std::string realpath(const char* path);

    inline std::string realpath(const std::string& path)
    {
        return realpath(path.c_str());
    }

    /// Returns file extension from the path
    std::string extractFileExtension(const std::string& path);

    /// Returns true iff the two files both exist, can be read,
    /// have equal size and every byte of their contents match.
    bool compareFileContents(const std::string& rhsPath, const std::string& lhsPath);

    /// Read nbytes from fd into buf. Retries on EINTR.
    /// Returns the number of bytes read, or -1 on error.
    ssize_t read(int fd, void* buf, size_t nbytes);

    /// Reads the whole file appending onto the given buffer. Only for small files.
    /// Does *not* clear the buffer before writing to it. Returns the number of bytes read, -1 for error.
    template <typename T>
    ssize_t readFile(const std::string& path, T& data, int maxSize = 256 * 1024)
    {
        const int fd = FileUtil::openFileAsFD(path, O_RDONLY);
        if (fd < 0)
            return -1;

        struct stat st;
        if (::fstat(fd, &st) != 0 || st.st_size > maxSize)
        {
            closeFD(fd);
            return -1;
        }

        const std::size_t originalSize = data.size();
        const auto remainingSize = (st.st_size > 0 ? st.st_size : maxSize);
        data.resize(originalSize + remainingSize);

        const ssize_t n = read(fd, &data[originalSize], remainingSize);
        closeFD(fd);

        data.resize(originalSize + (n <= 0 ? 0 : n));
        data.shrink_to_fit();

        return n;
    }

    /// Reads the whole file to memory. Only for small files.
    std::unique_ptr<std::vector<char>> readFile(const std::string& path, int maxSize = 256 * 1024);

    void copyDirectoryRecursive(std::string_view srcDir, std::string_view destDir, bool log);
    /// File/Directory stat helper.
    class Stat
    {
    public:
        /// Stat the given path. Symbolic links are stat'ed when @link is true.
        Stat(const std::string& path, bool link = false)
            : _sb{}
            , _res(link ? FileUtil::getLStatOfFile(path, _sb) : FileUtil::getStatOfFile(path, _sb))
            , _stat_errno(errno)
        {
        }

        bool good() const { return _res == 0; }
        bool bad() const { return !good(); }
        const struct ::stat& sb() const { return _sb; }

        bool isDirectory() const { return S_ISDIR(_sb.st_mode); }
        bool isFile() const { return S_ISREG(_sb.st_mode); }
        bool isLink() const { return S_ISLNK(_sb.st_mode); }
        std::size_t hardLinkCount() const { return _sb.st_nlink; }
        ino_t inodeNumber() const { return _sb.st_ino; }

        /// Returns the filesize in bytes.
        std::size_t size() const { return _sb.st_size; }

        /// Returns the modified unix-time as timespec since epoch with
        /// nanosecond precision, if/when the filesystem supports it.
        timespec modifiedTime() const
        {
#if defined(__APPLE__)
            return _sb.st_mtimespec;
#elif defined(_WIN32)
            timespec result{ _sb.st_mtime, 0 };
            return result;
#else
            return _sb.st_mtim;
#endif
        }

        /// Returns the modified unix-time in microseconds since epoch.
        int64_t modifiedTimeUs() const
        {
            // cast to make sure the calculation happens with enough bits
            return (static_cast<int64_t>(modifiedTime().tv_sec) * 1000 * 1000) + (modifiedTime().tv_nsec / 1000);
        }

        /// Returns the modified unix-time in milliseconds since epoch.
        std::size_t modifiedTimeMs() const
        {
            return (modifiedTime().tv_sec * 1000) + (modifiedTime().tv_nsec / 1000000);
        }

        /// Returns the modified unix-time as time_point (in microsecond precision, if available).
        /// The units is system-dependent, but it's 100% safe as time_point does the conversion
        /// to whatever we request, remembering the original units.
        std::chrono::system_clock::time_point modifiedTimepoint() const
        {
            // The time in microseconds.
            const std::chrono::microseconds us{ modifiedTimeUs() };

            // Convert to the precision of the system_clock::time_point,
            // which can be different from microseconds.
            return std::chrono::system_clock::time_point(
                std::chrono::duration_cast<std::chrono::system_clock::duration>(us));
        }

        /// Returns true iff the path exists, regardless of access permission.
        bool exists() const { return good() || (_stat_errno != ENOENT && _stat_errno != ENOTDIR); }

        /// Returns true if both files exist and have
        /// the same size and same contents.
        static bool isIdenticalTo(const Stat& l, const std::string& lPath, const Stat& r,
                                  const std::string& rPath)
        {
            // No need to check whether they are linked or not,
            // since if they are, the following check will match,
            // and if they aren't, we still need to rely on the following.
            // Finally, compare the contents, to avoid costly copying if we fail to update.
            return (l.exists() && r.exists() && !l.isDirectory() && !r.isDirectory() &&
                    l.size() == r.size() && compareFileContents(lPath, rPath));
        }

        /// Returns true if both files exist and have
        /// the same size and modified timestamp.
        static bool isUpToDate(const Stat& l, const std::string& lPath, const Stat& r,
                               const std::string& rPath)
        {
            // No need to check whether they are linked or not,
            // since if they are, the following check will match,
            // and if they aren't, we still need to rely on the following.
            // Finally, compare the contents, to avoid costly copying if we fail to update.
            if (isIdenticalTo(l, lPath, r, rPath))
            {
                return true;
            }

            // Clearly, no match. Log something informative.
            LOG_DBG("File contents mismatch: ["
                    << lPath << "] " << (l.exists() ? "exists" : "missing") << ", " << l.size()
                    << " bytes, modified at " << l.modifiedTime().tv_sec << " =/= [" << rPath
                    << "]: " << (r.exists() ? "exists" : "missing") << ", " << r.size()
                    << " bytes, modified at " << r.modifiedTime().tv_sec);
            return false;
        }

    private:
        struct ::stat _sb;
        const int _res;
        const int _stat_errno;
    };

    /// File owning helper that removes it on destruction.
    struct OwnedFile final
    {
        std::string _file;
        bool _recursive;

        OwnedFile(std::string file, bool recursive = false)
            : _file(std::move(file))
            , _recursive(recursive)
        {
        }

        OwnedFile(const OwnedFile&) = delete;
        OwnedFile& operator=(const OwnedFile&) = delete;

        ~OwnedFile()
        {
            FileUtil::removeFile(_file, _recursive);
        }
    };

    void lslr(const std::string& dir);

    std::vector<std::string> getDirEntries(const std::string& dirPath);

} // end namespace FileUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
