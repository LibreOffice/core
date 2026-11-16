/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/FileExtensions.hpp>
#include <common/Log.hpp>
#include <common/UserTemplates.hpp>
#include <common/Util.hpp>

#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Node.h>
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <optional>
#include <string_view>

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

std::string lowerCaseExtension(const std::string& fileName)
{
    std::string extension = filesystem::path(fileName).extension().string<char>();
    return extension.empty() ? extension : Util::toLower(extension.substr(1));
}

std::string typeOfExtension(const std::map<std::string, std::string>& types,
                            const std::string& extension)
{
    const auto it = types.find(extension);
    return it == types.end() ? std::string() : it->second;
}

// The copy of the image of the first page in the folder of a template, by image format.
constexpr const char* pngPreviewFileName = "preview.png";
constexpr const char* jpegPreviewFileName = "preview.jpeg";
constexpr const char* previewFileNames[] = { pngPreviewFileName, jpegPreviewFileName };

// The bytes of one entry of the zip file, or nothing when the entry is missing or larger than
// maxSize. The name matches without regard to case, as part names in an OOXML package do.
std::optional<std::string> readZipEntry(std::istream& in, const Poco::Zip::ZipArchive& archive,
                                        const std::string& name, std::size_t maxSize)
{
    auto header = archive.findHeader(name);
    if (header == archive.headerEnd())
    {
        const std::string lowerCaseName = Util::toLower(name);
        header = std::find_if(archive.headerBegin(), archive.headerEnd(),
                              [&lowerCaseName](const auto& item)
                              { return Util::toLower(item.first) == lowerCaseName; });
    }
    if (header == archive.headerEnd() || header->second.getUncompressedSize() > maxSize)
        return std::nullopt;

    Poco::Zip::ZipInputStream zipStream(in, header->second);
    std::string content;
    char buffer[8192];
    while (zipStream.read(buffer, sizeof(buffer)) || zipStream.gcount() > 0)
    {
        content.append(buffer, zipStream.gcount());
        if (content.size() > maxSize)
            return std::nullopt;
    }
    return content;
}

// The name of the zip entry that _rels/.rels gives as the thumbnail of an OOXML package, or empty
// when there is none. A target is relative to the root of the package, which is also the root of
// the zip file.
std::string ooxmlThumbnailEntry(const std::string& relationships)
{
    constexpr std::string_view thumbnailType =
        "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail";

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> document = parser.parseString(relationships);
    Poco::XML::Element* root = document->documentElement();
    if (!root)
        return std::string();

    for (Poco::XML::Node* node = root->firstChild(); node; node = node->nextSibling())
    {
        if (node->nodeType() != Poco::XML::Node::ELEMENT_NODE ||
            node->localName() != "Relationship")
            continue;
        auto* element = static_cast<Poco::XML::Element*>(node);
        if (element->getAttribute("Type") != thumbnailType ||
            element->getAttribute("TargetMode") == "External")
            continue;
        std::string target = element->getAttribute("Target");
        if (!target.empty() && target[0] == '/')
            target.erase(0, 1);
        return target;
    }
    return std::string();
}

// Copies the image of the first page out of the template file into the folder, and nothing when
// the file has no such image. An ODF file carries it as Thumbnails/thumbnail.png. An OOXML file
// can name one in _rels/.rels, and only a PNG or JPEG image is copied.
void extractPreview(const filesystem::path& templatePath, const filesystem::path& folder)
{
    // An image of one page is far smaller than this.
    constexpr std::size_t maxPreviewSize = 1024 * 1024;
    constexpr std::size_t maxRelationshipsSize = 64 * 1024;
    try
    {
        std::ifstream in(templatePath, std::ios::binary);
        const Poco::Zip::ZipArchive archive(in);

        std::string entry = "Thumbnails/thumbnail.png";
        filesystem::path target = folder / pngPreviewFileName;
        if (archive.findHeader(entry) == archive.headerEnd())
        {
            const std::optional<std::string> relationships =
                readZipEntry(in, archive, "_rels/.rels", maxRelationshipsSize);
            if (!relationships)
                return;

            entry = ooxmlThumbnailEntry(*relationships);
            const std::string extension = lowerCaseExtension(entry);
            if (extension == "png")
                target = folder / pngPreviewFileName;
            else if (extension == "jpeg" || extension == "jpg")
                target = folder / jpegPreviewFileName;
            else
                return;
        }

        const std::optional<std::string> image = readZipEntry(in, archive, entry, maxPreviewSize);
        if (!image)
            return;

        std::ofstream out(target, std::ios::binary);
        out.write(image->data(), image->size());
        out.close();
        if (!out)
        {
            LOG_DBG(
                "UserTemplates: could not write the preview '" << target.string<char>() << '\'');
            std::error_code error;
            filesystem::remove(target, error);
        }
    }
    catch (const std::exception& exception)
    {
        LOG_DBG("UserTemplates: no preview in '" << templatePath.string<char>()
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
        if (!isPlainId(item.path().filename().string<char>()) || item.is_symlink(error) ||
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
        LOG_ERR(
            "UserTemplates: could not create '" << folder.string<char>() << "': "
            << error.message());
        return std::nullopt;
    }

    const filesystem::path target =
        folder / (stem + filesystem::path(sourcePath).extension().string<char>());
    filesystem::copy_file(sourcePath, target, error);
    if (error)
    {
        LOG_ERR("UserTemplates: could not copy '" << sourcePath << "' to '" << target.string<char>()
                                                  << "': " << error.message());
        filesystem::remove_all(folder, error);
        return std::nullopt;
    }

    extractPreview(target, folder);
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
    filesystem::rename(oldPath, *folder / (stem + oldPath.extension().string<char>()), error);
    if (error)
    {
        LOG_ERR("UserTemplates: could not rename '" << oldPath.string<char>()
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
        LOG_ERR("UserTemplates: could not remove '" << folder->string<char>()
                                                    << "': " << error.message());
    return !error;
}

std::string UserTemplates::documentTypeOfTemplate(const std::string& fileName)
{
    return typeOfExtension(FileExtensions::typeOfTemplateExtension(), lowerCaseExtension(fileName));
}

std::string UserTemplates::documentTypeOfFile(const std::string& fileName)
{
    const std::string extension = lowerCaseExtension(fileName);
    const std::string type = typeOfExtension(FileExtensions::typeOfTemplateExtension(), extension);
    return type.empty() ? typeOfExtension(FileExtensions::typeOfDocumentExtension(), extension)
                        : type;
}

std::string UserTemplates::documentExtensionOfFile(const std::string& fileName)
{
    const std::string extension = lowerCaseExtension(fileName);
    if (FileExtensions::typeOfDocumentExtension().contains(extension))
        return extension;
    return typeOfExtension(FileExtensions::documentExtensionOfTemplateExtension(), extension);
}

std::vector<std::string> UserTemplates::acceptedExtensions()
{
    std::vector<std::string> extensions;
    for (const auto& [extension, type] : FileExtensions::typeOfTemplateExtension())
        extensions.push_back(extension);
    for (const auto& [extension, type] : FileExtensions::typeOfDocumentExtension())
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
        const std::string type = documentTypeOfFile(path.filename().string<char>());
        if (type.empty())
            continue;

        Entry entry{ templateFolder.filename().string<char>(),
                     path.stem().string<char>(),
                     type,
                     path.string<char>(),
                     std::string() };
        for (const char* previewFileName : previewFileNames)
        {
            const filesystem::path preview = templateFolder / previewFileName;
            if (!filesystem::is_symlink(preview, error) &&
                filesystem::is_regular_file(preview, error))
            {
                entry.previewPath = preview.string<char>();
                break;
            }
        }
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
        LOG_ERR("UserTemplates: '" << folder.string<char>() << "' is not a template folder");
        return std::nullopt;
    }
    return folder;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
