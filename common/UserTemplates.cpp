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

#include <common/Log.hpp>
#include <common/UserTemplates.hpp>
#include <common/Util.hpp>

#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>

namespace filesystem = std::filesystem;

namespace
{
// The name of a template is the name of its file, so path separators and control characters
// become underscores.
std::string fileStemForName(const std::string& name)
{
    std::string stem = Util::trimmed(name);
    for (char& character : stem)
    {
        if (character == '/' || character == '\\' || static_cast<unsigned char>(character) < 0x20)
            character = '_';
    }
    if (stem == "." || stem == "..")
        return std::string();
    return stem;
}

// This is the list of ViewFilter_Application::isFilteredExtension in
// engine/sfx2/source/control/templatelocalview.cxx, plus dotm.
const std::map<std::string, std::string>& typeOfTemplateExtension()
{
    static const std::map<std::string, std::string> types = {
        { "ott", "writer" },  { "ots", "calc" },   { "otp", "impress" },  { "otg", "draw" },
        { "otm", "writer" },  { "oth", "writer" }, { "stw", "writer" },   { "stc", "calc" },
        { "sti", "impress" }, { "std", "draw" },   { "dotx", "writer" },  { "dotm", "writer" },
        { "xltx", "calc" },   { "xltm", "calc" },  { "potx", "impress" }, { "potm", "impress" },
        { "dot", "writer" },  { "xlt", "calc" },   { "pot", "impress" },
    };
    return types;
}

// The extension of the document that each template file creates. The older StarOffice templates
// create an ODF document.
const std::map<std::string, std::string>& documentExtensionOfTemplateExtension()
{
    static const std::map<std::string, std::string> extensions = {
        { "ott", "odt" },   { "ots", "ods" },   { "otp", "odp" },   { "otg", "odg" },
        { "otm", "odm" },   { "oth", "html" },  { "stw", "odt" },   { "stc", "ods" },
        { "sti", "odp" },   { "std", "odg" },   { "dotx", "docx" }, { "dotm", "docm" },
        { "xltx", "xlsx" }, { "xltm", "xlsm" }, { "potx", "pptx" }, { "potm", "pptm" },
        { "dot", "doc" },   { "xlt", "xls" },   { "pot", "ppt" },
    };
    return extensions;
}

// Extensions of ordinary documents: ODF, flat ODF, OOXML, the older binary formats and RTF.
const std::map<std::string, std::string>& typeOfDocumentExtension()
{
    static const std::map<std::string, std::string> types = {
        { "odt", "writer" },   { "ods", "calc" },     { "odp", "impress" },  { "odg", "draw" },
        { "fodt", "writer" },  { "fods", "calc" },    { "fodp", "impress" }, { "fodg", "draw" },
        { "docx", "writer" },  { "docm", "writer" },  { "xlsx", "calc" },    { "xlsm", "calc" },
        { "pptx", "impress" }, { "pptm", "impress" }, { "doc", "writer" },   { "rtf", "writer" },
        { "xls", "calc" },     { "ppt", "impress" },
    };
    return types;
}

std::string lowerCaseExtension(const std::string& fileName)
{
    std::string extension = filesystem::path(fileName).extension().string();
    return extension.empty() ? extension : Util::toLower(extension.substr(1));
}

std::string typeOfExtension(const std::map<std::string, std::string>& types,
                            const std::string& extension)
{
    const auto it = types.find(extension);
    return it == types.end() ? std::string() : it->second;
}

constexpr const char* previewFileName = "preview.png";

// An ODF file carries an image of its first page as Thumbnails/thumbnail.png. Copies that image to
// the target path, and nothing when the file has no such image.
void extractPreview(const filesystem::path& templatePath, const filesystem::path& target)
{
    // An image of one page is far smaller than this.
    constexpr std::size_t maxPreviewSize = 1024 * 1024;
    try
    {
        std::ifstream in(templatePath, std::ios::binary);
        const Poco::Zip::ZipArchive archive(in);
        const auto header = archive.findHeader("Thumbnails/thumbnail.png");
        if (header == archive.headerEnd() || header->second.getUncompressedSize() > maxPreviewSize)
            return;

        Poco::Zip::ZipInputStream zipStream(in, header->second);
        std::string image;
        char buffer[8192];
        while (zipStream.read(buffer, sizeof(buffer)) || zipStream.gcount() > 0)
        {
            image.append(buffer, zipStream.gcount());
            if (image.size() > maxPreviewSize)
                return;
        }

        std::ofstream out(target, std::ios::binary);
        out.write(image.data(), image.size());
        out.close();
        if (!out)
        {
            LOG_DBG("UserTemplates: could not write the preview '" << target.string() << '\'');
            std::error_code error;
            filesystem::remove(target, error);
        }
    }
    catch (const std::exception& exception)
    {
        LOG_DBG("UserTemplates: no preview in '" << templatePath.string()
                                                 << "': " << exception.what());
    }
}

// The ids are made of hexadecimal digits only, so an id is always one plain folder name.
bool isPlainId(const std::string& id)
{
    return !id.empty() && id.size() <= 64 &&
           std::all_of(id.begin(), id.end(),
                       [](unsigned char character) { return std::isxdigit(character); });
}
} // namespace

UserTemplates::UserTemplates(filesystem::path folder)
    : _folder(std::move(folder))
{
}

std::vector<UserTemplates::Entry> UserTemplates::list() const
{
    std::vector<Entry> entries;
    std::error_code error;
    for (const filesystem::directory_entry& item : filesystem::directory_iterator(_folder, error))
    {
        if (!isPlainId(item.path().filename().string()) || item.is_symlink(error) ||
            !item.is_directory(error))
            continue;
        if (std::optional<Entry> entry = entryInFolder(item.path()))
            entries.push_back(std::move(*entry));
    }

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
              { return Util::toLower(a.name) < Util::toLower(b.name); });
    return entries;
}

std::optional<UserTemplates::Entry> UserTemplates::addFile(const std::string& sourcePath,
                                                           const std::string& name)
{
    std::error_code error;
    if (!filesystem::is_regular_file(sourcePath, error))
    {
        LOG_ERR("UserTemplates: '" << sourcePath << "' is not a file");
        return std::nullopt;
    }

    if (documentTypeOfFile(sourcePath).empty())
    {
        LOG_ERR("UserTemplates: '" << sourcePath << "' is neither a template nor a document");
        return std::nullopt;
    }

    const std::string stem = fileStemForName(name);
    if (stem.empty())
        return std::nullopt;

    const filesystem::path folder = _folder / Util::rng::getHexString(16);
    filesystem::create_directories(folder, error);
    if (error)
    {
        LOG_ERR("UserTemplates: could not create '" << folder.string() << "': " << error.message());
        return std::nullopt;
    }

    const filesystem::path target =
        folder / (stem + filesystem::path(sourcePath).extension().string());
    filesystem::copy_file(sourcePath, target, error);
    if (error)
    {
        LOG_ERR("UserTemplates: could not copy '" << sourcePath << "' to '" << target.string()
                                                  << "': " << error.message());
        filesystem::remove_all(folder, error);
        return std::nullopt;
    }

    extractPreview(target, folder / previewFileName);
    return entryInFolder(folder);
}

std::optional<UserTemplates::Entry> UserTemplates::rename(const std::string& id,
                                                          const std::string& name)
{
    const std::optional<filesystem::path> folder = templateFolder(id);
    if (!folder)
        return std::nullopt;

    const std::optional<Entry> entry = entryInFolder(*folder);
    const std::string stem = fileStemForName(name);
    if (!entry || stem.empty())
        return std::nullopt;

    const filesystem::path oldPath(entry->path);
    std::error_code error;
    filesystem::rename(oldPath, *folder / (stem + oldPath.extension().string()), error);
    if (error)
    {
        LOG_ERR("UserTemplates: could not rename '" << oldPath.string()
                                                    << "': " << error.message());
        return std::nullopt;
    }

    return entryInFolder(*folder);
}

bool UserTemplates::remove(const std::string& id)
{
    const std::optional<filesystem::path> folder = templateFolder(id);
    if (!folder)
        return false;

    std::error_code error;
    filesystem::remove_all(*folder, error);
    if (error)
        LOG_ERR("UserTemplates: could not remove '" << folder->string()
                                                    << "': " << error.message());
    return !error;
}

std::string UserTemplates::documentTypeOfTemplate(const std::string& fileName)
{
    return typeOfExtension(typeOfTemplateExtension(), lowerCaseExtension(fileName));
}

std::string UserTemplates::documentTypeOfFile(const std::string& fileName)
{
    const std::string extension = lowerCaseExtension(fileName);
    const std::string type = typeOfExtension(typeOfTemplateExtension(), extension);
    return type.empty() ? typeOfExtension(typeOfDocumentExtension(), extension) : type;
}

std::string UserTemplates::documentExtensionOfFile(const std::string& fileName)
{
    const std::string extension = lowerCaseExtension(fileName);
    if (typeOfDocumentExtension().contains(extension))
        return extension;
    return typeOfExtension(documentExtensionOfTemplateExtension(), extension);
}

std::vector<std::string> UserTemplates::acceptedExtensions()
{
    std::vector<std::string> extensions;
    for (const auto& [extension, type] : typeOfTemplateExtension())
        extensions.push_back(extension);
    for (const auto& [extension, type] : typeOfDocumentExtension())
        extensions.push_back(extension);
    return extensions;
}

std::optional<UserTemplates::Entry>
UserTemplates::entryInFolder(const filesystem::path& templateFolder) const
{
    std::error_code error;
    for (const filesystem::directory_entry& item :
         filesystem::directory_iterator(templateFolder, error))
    {
        if (item.is_symlink(error) || !item.is_regular_file(error))
            continue;

        const filesystem::path& path = item.path();
        const std::string type = documentTypeOfFile(path.filename().string());
        if (type.empty())
            continue;

        Entry entry{ templateFolder.filename().string(),
                     path.stem().string(),
                     type,
                     path.string(),
                     std::string() };
        const filesystem::path preview = templateFolder / previewFileName;
        if (!filesystem::is_symlink(preview, error) && filesystem::is_regular_file(preview, error))
            entry.previewPath = preview.string();
        return entry;
    }
    return std::nullopt;
}

// The folder of a template counts only when its real path is the templates folder plus the id, so
// a symbolic link in its place is never followed.
std::optional<filesystem::path> UserTemplates::templateFolder(const std::string& id) const
{
    if (!isPlainId(id))
        return std::nullopt;

    const filesystem::path folder = _folder / id;
    std::error_code error;
    if (filesystem::is_symlink(folder, error) || !filesystem::is_directory(folder, error) ||
        filesystem::canonical(folder, error) != filesystem::canonical(_folder, error) / id)
    {
        LOG_ERR("UserTemplates: '" << folder.string() << "' is not a template folder");
        return std::nullopt;
    }
    return folder;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
