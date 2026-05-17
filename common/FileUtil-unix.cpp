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
 * Unix-specific file utility implementations.
 * Functions: linkOrCopyFile(), realpath(), file operations
 */

#include <config.h>

#include <common/Anonymizer.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <dirent.h>
#include <filesystem>
#include <ftw.h>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/vfs.h>
#elif defined IOS
#import <Foundation/Foundation.h>
#elif defined __FreeBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif

namespace FileUtil
{
    bool linkOrCopyFile(const std::string& source, const std::string& newPath)
    {
        // first try a simple hard-link
        if (link(source.c_str(), newPath.c_str()) == 0)
            return true;

        LOG_DBG_SYS("Failed to link [" << source << "] to [" << newPath << "], will try to copy");

        return FileUtil::copy(source, newPath, /*log=*/true, /*throw_on_error=*/false);
    }

    std::string realpath(const char* path)
    {
        char* resolved = ::realpath(path, nullptr);
        if (resolved)
        {
            std::string real = resolved;
            free(resolved);
            return real;
        }

        LOG_SYS("Failed to get the realpath of [" << path << ']');
        return path;
    }

#if !defined(__APPLE__) // iOS-specific implementation in FileUtil-apple.cpp

    bool platformDependentCheckDiskSpace(const std::string& path, int64_t enoughSpace)
    {
#if defined __EMSCRIPTEN__
        //TODO:
        (void) path;
        (void) enoughSpace;
#else
        struct statfs sfs;
        if (statfs(path.c_str(), &sfs) == -1)
        {
            LOG_SYS("Failed to stat filesystem [" << path << ']');
            return true; // We assume the worst.
        }

        const int64_t freeBytes = static_cast<int64_t>(sfs.f_bavail) * sfs.f_bsize;

        LOG_INF("Filesystem [" << path << "] has " << (freeBytes / 1024 / 1024) <<
                " MB free (" << (sfs.f_bavail * 100. / sfs.f_blocks) << "%).");

        if (freeBytes > enoughSpace)
            return true;

        if (static_cast<double>(sfs.f_bavail) / sfs.f_blocks <= 0.05)
            return false;
#endif

        return true;
    }
#elif defined(MACOS) && !MOBILEAPP

    bool platformDependentCheckDiskSpace(const std::string&, int64_t)
    {
        // FIXME Use the FileUtil-apple.mm instead
        return true;
    }
#endif

    static int nftw_cb(const char *fpath, const struct stat*, int type, struct FTW*)
    {
        if (type == FTW_DP)
        {
            rmdir(fpath);
        }
        else if (type == FTW_F || type == FTW_SL)
        {
            unlink(fpath);
        }

        // Always continue even when things go wrong.
        return 0;
    }

    void removeFile(const std::string& path, const bool recursive)
    {
        LOG_DBG("Removing [" << Anonymizer::anonymizeUrl(path) << "] " << (recursive ? "recursively." : "only."));

        try
        {
            struct stat sb;
            errno = 0;
            if (!recursive || stat(path.c_str(), &sb) == -1 || S_ISREG(sb.st_mode))
            {
                // Non-recursive directories and files that exist.
                if (errno != ENOENT)
                    Poco::File(path).remove(recursive);
            }
            else
            {
                // Directories only.
                nftw(path.c_str(), nftw_cb, 128, FTW_DEPTH | FTW_PHYS);
            }
        }
        catch (const std::exception& e)
        {
            // Don't complain if already non-existent.
            if (FileUtil::Stat(path).exists())
            {
                // Error only if it still exists.
                LOG_ERR("Failed to remove ["
                        << Anonymizer::anonymizeUrl(path) << "] " << (recursive ? "recursively: " : "only: ") << e.what());
            }
        }
    }

    /// Remove directories only, which must be empty for this to work.
    static int nftw_rmdir_cb(const char* fpath, const struct stat*, int type, struct FTW*)
    {
        if (type == FTW_DP)
        {
            rmdir(fpath);
        }

        // Always continue even when things go wrong.
        return 0;
    }

    void removeEmptyDirTree(const std::string& path)
    {
        LOG_DBG("Removing empty directories at [" << path << "] recursively");

        nftw(path.c_str(), nftw_rmdir_cb, 128, FTW_DEPTH | FTW_PHYS);
    }

    bool isEmptyDirectory(const char* path)
    {
        DIR* dir = opendir(path);
        if (dir == nullptr)
            return errno != EACCES; // Assume it's not empty when EACCES.

        int count = 0;
        while (readdir(dir) && ++count < 3)
            ;

        closedir(dir);
        return count <= 2; // Discounting . and ..
    }

    /**
     * Recursively lists the contents of a directory in a detailed format, similar to the "ls -lr" command.
     * For each file or directory, it displays metadata such as permissions, owner, size, and modification time.
     */
    void lslr(const std::string& path)
    {
        std::cout << path << ":\n";

        // Log error if unable to open
        DIR* dir = opendir(path.c_str());
        if (dir == nullptr)
        {
            std::cerr << "lslr: fail to open: " << dir << " error: " << std::strerror(errno) << std::endl;
            return;
        }

        /// Metadata for display purposes
        struct sb
        {
            mode_t _mode;          // File mode (permissions).
            nlink_t _nlink;        // Number of hard links.
            std::string _uid;      // User ID of the file owner.
            std::string _gid;      // Group ID of the file owner.
            off_t _size;           // File size in bytes.
            time_t _mtime;         // Last modification time.
            std::string _name;     // File or directory name.

            /// Constructor
            sb(mode_t mode, nlink_t nlink, std::string uid, std::string gid, off_t size, time_t mtime, std::string name)
                : _mode(mode)
                , _nlink(nlink)
                , _uid(std::move(uid))
                , _gid(std::move(gid))
                , _size(size)
                , _mtime(mtime)
                , _name(std::move(name))
            {
            }
        };

        std::vector<sb> entries;
        std::vector<std::string> subdirs;
        size_t nlink_len = 0;
        size_t size_len = 0;
        size_t uid_len = 0;
        size_t gid_len = 0;
        size_t blocks = 0;

        while (const dirent* f = readdir(dir))
        {
            std::string fullpath(path);
            if (!fullpath.ends_with("/"))
                fullpath.push_back('/');
            fullpath.append(f->d_name);

            struct stat statbuf;
            if (lstat(fullpath.c_str(), &statbuf) != 0)
            {
                std::cerr << "lslr: fail to lstat: " << fullpath << " error: " << std::strerror(errno) << std::endl;
                continue;
            }

            size_len = std::max(size_len, std::to_string(statbuf.st_size).size());
            nlink_len = std::max(nlink_len, std::to_string(statbuf.st_nlink).size());

            std::string uid;
            struct passwd *pwd = getpwuid(statbuf.st_uid);
            if (pwd && pwd->pw_name)
                uid = pwd->pw_name;
            else
                uid = std::to_string(statbuf.st_uid);
            uid_len = std::max(uid_len, uid.size());

            std::string gid;
            struct group *grp = getgrgid(statbuf.st_gid);
            if (grp && grp->gr_name)
                gid = grp->gr_name;
            else
                gid = std::to_string(statbuf.st_gid);

            entries.emplace_back(statbuf.st_mode, statbuf.st_nlink, uid, gid, statbuf.st_size, statbuf.st_mtime, f->d_name);

            if (f->d_name != std::string_view(".") && f->d_name != std::string_view("..") && (statbuf.st_mode & S_IFMT) == S_IFDIR)
                subdirs.push_back(std::move(fullpath));

            blocks += statbuf.st_blocks;
        }

        std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs)
                  { return strcasecmp(lhs._name.c_str(), rhs._name.c_str()) < 0; });
        std::sort(subdirs.begin(), subdirs.end(), [](const auto& lhs, const auto& rhs)
                  { return strcasecmp(lhs.c_str(), rhs.c_str()) < 0; });

        closedir(dir);

        // turn 512 blocks into ls-alike default 1024 byte blocks
        std::cout << "total " << (blocks + 1) / 2 << "\n";

        for (const auto& entry : entries)
        {
            bool symbolic_link = false;

            switch (entry._mode & S_IFMT)
            {
                case S_IFREG:
                    std::cout << '-';
                    break;
                case S_IFBLK:
                    std::cout << 'b';
                    break;
                case S_IFCHR:
                    std::cout << 'c';
                    break;
                case S_IFDIR:
                    std::cout << 'd';
                    break;
                case S_IFLNK:
                    std::cout << 'l';
                    symbolic_link = true;
                    break;
                case S_IFIFO:
                    std::cout << 'p';
                    break;
                case S_IFSOCK:
                    std::cout << 's';
                    break;
                default:
                    std::cout << '?';
                    break;
            }

            std::cout << ((entry._mode & S_IRUSR) ? "r" : "-");
            std::cout << ((entry._mode & S_IWUSR) ? "w" : "-");
            std::cout << ((entry._mode & S_IXUSR) ? "x" : "-");
            std::cout << ((entry._mode & S_IRGRP) ? "r" : "-");
            std::cout << ((entry._mode & S_IWGRP) ? "w" : "-");
            std::cout << ((entry._mode & S_IXGRP) ? "x" : "-");
            std::cout << ((entry._mode & S_IROTH) ? "r" : "-");
            std::cout << ((entry._mode & S_IWOTH) ? "w" : "-");
            std::cout << ((entry._mode & S_IXOTH) ? "x" : "-");

            std::cout << " " << std::right << std::setw(nlink_len) << entry._nlink;

            std::cout << " " << std::left << std::setw(uid_len) << entry._uid;

            std::cout << " " << std::left << std::setw(gid_len) << entry._gid;

            std::cout << " " << std::right << std::setw(size_len) << entry._size;

            struct tm tm;
            std::cout << " " << std::put_time(gmtime_r(&entry._mtime, &tm), "%F %R");

            std::cout << " " << entry._name;

            if (symbolic_link)
            {
                std::string fullpath(path);
                fullpath.append("/").append(entry._name);

                const std::size_t size = entry._size;
                std::vector<char> target(size + 1);
                char* target_data = target.data();
                const ssize_t read = readlink(fullpath.c_str(), target_data, size);
                if (read <= 0 || static_cast<std::size_t>(read) > size)
                    std::cerr << "lslr: fail to read: " << fullpath << " error: " << std::strerror(errno) << std::endl;
                else
                {
                    target_data[read] = '\0';
                    std::cout << " -> " << target.data();
                }
            }

            std::cout << "\n";
        }

        for (const auto& subdir : subdirs)
        {
            std::cout << "\n";
            lslr(subdir);
        }
    }

    std::vector<std::string> getDirEntries(const std::string& dirPath)
    {
        std::vector<std::string> names;
        DIR* dir = opendir(dirPath.data());
        if (!dir)
        {
            LOG_DBG("Read from non-existent directory " << dirPath);
            return names;
        }
        struct dirent *i;
        while ((i = readdir(dir)))
        {
            if (i->d_name[0] == '.')
                continue;
            names.push_back(i->d_name);
        }
        closedir(dir);
        return names;
    }

    int openFileAsFD(const std::string& file, int oflag, int mode)
    {
        return ::open(file.c_str(), oflag, mode);
    }

    int readFromFD(int fd, void *buf, size_t nbytes)
    {
        return ::read(fd, buf, nbytes);
    }

    int writeToFD(int fd, const void *buf, size_t nbytes)
    {
        return ::write(fd, buf, nbytes);
    }

    int closeFD(int fd)
    {
        return ::close(fd);
    }

    void openFileToIFStream(const std::string& file, std::ifstream& stream, std::ios_base::openmode mode)
    {
        stream.open(file, mode);
    }

    void openFileToOFStream(const std::string& file, std::ofstream& stream, std::ios_base::openmode mode)
    {
        stream.open(file, mode);
    }

    int getStatOfFile(const std::string& file, struct stat& sb)
    {
        return ::stat(file.c_str(), &sb);
    }

    int getLStatOfFile(const std::string& file, struct stat& sb)
    {
        return ::lstat(file.c_str(), &sb);
    }

    int unlinkFile(const std::string& file)
    {
        return ::unlink(file.c_str());
    }

    int makeDirectory(const std::string& dir)
    {
        return ::mkdir(dir.c_str(), S_IRWXU);
    }

    void createDirectory(std::string_view dir)
    {
        std::filesystem::create_directory(dir);
    }

    std::string getSysTempDirectoryPath()
    {
        // Don't const to allow for automatic move on return.
        std::string path = std::filesystem::temp_directory_path();

        if (!path.empty())
            return path;

        // Sensible fallback, though shouldn't be needed.
        const char *tmp = getenv("TMPDIR");
        if (!tmp)
            tmp = getenv("TEMP");
        if (!tmp)
            tmp = getenv("TMP");
        if (!tmp)
            tmp = "/tmp";
        return tmp;
    }

    bool isWritable(const char* path)
    {
        if (access(path, W_OK) == 0)
            return true;

        LOG_INF("No write access to path [" << path << "]: " << strerror(errno));
        return false;
    }

    bool updateTimestamps(const std::string& filename, timespec tsAccess, timespec tsModified)
    {
        // The timestamp is in seconds and microseconds.
        timeval timestamps[2]
                          {
                              {
                                  tsAccess.tv_sec,
#if defined(IOS) || defined(MACOS)
                                  (__darwin_suseconds_t)
#endif
                                  (tsAccess.tv_nsec / 1000)
                              },
                              {
                                  tsModified.tv_sec,
#if defined(IOS) || defined(MACOS)
                                  (__darwin_suseconds_t)
#endif
                                  (tsModified.tv_nsec / 1000)
                              }
                          };
        if (utimes(filename.c_str(), timestamps) != 0)
        {
            LOG_SYS("Failed to update the timestamp of [" << filename << ']');
            return false;
        }

        return true;
    }
} // namespace FileUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
