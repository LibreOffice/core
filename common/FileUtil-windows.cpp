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

#include <config.h>

#include <filesystem>
#include <iostream>

#include <direct.h>
#include <io.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <common/FileUtil.hpp>
#include <common/Util.hpp>

namespace FileUtil
{
    void removeFile(const std::string& path, const bool recursive)
    {
        LOG_DBG("Removing [" << path << "] " << (recursive ? "recursively." : "only."));

        try
        {
            if (recursive)
                std::filesystem::remove_all(Util::string_to_wide_string(path));
            else
                std::filesystem::remove(Util::string_to_wide_string(path));
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            // Don't complain if already non-existent.
            if (FileUtil::Stat(path).exists())
            {
                // Error only if it still exists.
                LOG_ERR("Failed to remove ["
                        << path << "] " << (recursive ? "recursively: " : "only: ") << e.what());
            }
        }
    }

    /// Remove directories only, which must be empty for this to work.
    static void removeEmptyDirTreeTakingPath(const std::filesystem::path& path)
    {
        for (auto const& dirent :
                 std::filesystem::directory_iterator{path, std::filesystem::directory_options::skip_permission_denied})
        {
            if (dirent.is_directory())
            {
                std::error_code ec;
                removeEmptyDirTreeTakingPath(dirent.path());
                std::filesystem::remove(dirent.path(), ec);
            }
        }
    }

    void removeEmptyDirTree(const std::string& path)
    {
        LOG_DBG("Removing empty directories at [" << path << "] recursively");

        removeEmptyDirTreeTakingPath(std::filesystem::path(Util::string_to_wide_string(path)));
    }

    bool isEmptyDirectory(const char* path)
    {
        bool empty = true;
        for (auto const& dirent :
                 std::filesystem::directory_iterator{std::filesystem::path(Util::string_to_wide_string(path)),
                                                     std::filesystem::directory_options::skip_permission_denied})
        {
            (void) dirent;
            empty = false;
            break;
        }
        return empty;
    }

    bool linkOrCopyFile(const std::string& source, const std::string& newPath)
    {
        return FileUtil::copy(source, newPath, /*log=*/true, /*throw_on_error=*/false);
    }

    std::string realpath(const char* path)
    {
        return path;
    }

    bool platformDependentCheckDiskSpace(const std::string& path, int64_t enoughSpace)
    {
        // FIXME
        return true;
    }

    int openFileAsFD(const std::string& file, int oflag, int mode)
    {
        return _wopen(Util::string_to_wide_string(file).c_str(), oflag | O_BINARY, mode);
    }

    int readFromFD(int fd, void *buf, size_t nbytes)
    {
        return _read(fd, buf, nbytes);
    }

    int writeToFD(int fd, const void *buf, size_t nbytes)
    {
        return _write(fd, buf, nbytes);
    }

    int closeFD(int fd)
    {
        return _close(fd);
    }

    void openFileToIFStream(const std::string& file, std::ifstream& stream, std::ios_base::openmode mode)
    {
        stream.open(Util::string_to_wide_string(file), mode | std::ios_base::binary);
    }

    void openFileToOFStream(const std::string& file, std::ofstream& stream, std::ios_base::openmode mode)
    {
        stream.open(Util::string_to_wide_string(file), mode | std::ios_base::binary);
    }

    int getStatOfFile(const std::string& file, struct stat& sb)
    {
        return _wstat64i32(Util::string_to_wide_string(file).c_str(), (struct _stat64i32*) &sb);
    }

    int getLStatOfFile(const std::string& file, struct stat& sb)
    {
        return getStatOfFile(file, sb);
    }

    int unlinkFile(const std::string& file)
    {
        return _wunlink(Util::string_to_wide_string(file).c_str());
    }

    int makeDirectory(const std::string& dir)
    {
        return _wmkdir(Util::string_to_wide_string(dir).c_str());
    }

    void createDirectory(std::string_view dir)
    {
        std::filesystem::create_directory(Util::string_to_wide_string(dir));
    }

    void setSysTempDirectoryPath(const std::string& /*path*/)
    {
        // Not implemented as we're in the user's environment anyway (i.e. no sharing).
    }

    std::string getSysTempDirectoryPath()
    {
        std::error_code ec;
        std::wstring path = std::filesystem::temp_directory_path(ec).wstring();

        if (!ec && !path.empty() && path.back() == L'\\')
            path.pop_back();

        if (!ec && !path.empty())
            return Util::wide_string_to_string(path);

        // Try some fallbacks
        wchar_t* tmp = _wgetenv(L"TMP");
        if (!tmp || tmp[0] == L'\0')
            tmp = _wgetenv(L"TEMP");
        if (tmp && tmp[0] == L'\0')
            tmp = NULL;

        // We don't want to modify the environment string directly.
        if (tmp)
        {
            tmp = _wcsdup(tmp);
            if (tmp[wcslen(tmp)-1] == L'\\')
                tmp[wcslen(tmp)-1] = L'\0';
        }

        // This folder seems to be protected somehow on modern Windows, but oh well.
        // Duplicate here, too, so we can free() below.
        if (!tmp)
            tmp = _wcsdup(L"C:/Windows/Temp");

        auto result = Util::wide_string_to_string(tmp);
        free(tmp);
        return result;
    }

    bool isWritable(const char* path)
    {
        if (_waccess(Util::string_to_wide_string(path).c_str(), 0) == 0)
            return true;

        LOG_INF("No write access to path [" << path << "]: " << strerror(errno));
        return false;
    }
} // namespace FileUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
