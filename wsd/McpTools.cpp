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

#include <wsd/McpTools.hpp>

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/McpUploads.hpp>

#include <Poco/Path.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <string>

namespace
{

// Write a fresh incoming file named after filename with write, and return its path. When the file
// cannot be opened or write fails, the directory made for it is removed with whatever was written,
// and the path is empty.
std::string writeIncomingFile(const std::string& filename,
                              const std::function<bool(std::ostream&)>& write)
{
    const std::string path = McpTools::makeIncomingPath(filename);
    if (path.empty())
        return std::string();

    std::ofstream fileStream(path, std::ios::binary);
    const bool written = fileStream.good() && write(fileStream);
    fileStream.close();
    if (!written || !fileStream.good())
    {
        LOG_ERR("MCP: failed to write incoming file [" << path << ']');
        FileUtil::removeFile(Poco::Path(path).parent().toString(), /*recursive=*/true);
        return std::string();
    }
    return path;
}

McpTools::ResolvedInput failure(const std::string& message)
{
    McpTools::ResolvedInput result;
    result.ok = false;
    result.errorText = message;
    return result;
}

McpTools::ResolvedInput success(std::string fromPath, std::string filename)
{
    McpTools::ResolvedInput result;
    result.ok = true;
    result.fromPath = std::move(fromPath);
    result.filename = std::move(filename);
    return result;
}

McpTools::ResolvedInput resolveFromData(const std::string& data, const std::string& filenameArg)
{
    // Four characters of base64 carry three bytes, so the text alone says whether the document is
    // too large. It is weighed first, and only a document within the accepted size is decoded.
    const std::size_t maxBytes = McpTools::maxDocumentBytes();
    if (maxBytes != 0 && data.size() / 4 * 3 > maxBytes)
        return failure("The document is larger than the server accepts.");
    if (data.size() / 4 * 3 > McpTools::MaxInlineDocumentBytes)
        return failure("A document of more than 16 MB is too large to pass inline. Upload it and "
                       "pass its file_id.");

    std::string decoded;
    const std::string decodeError = macaron::Base64::Decode(data, decoded);
    if (!decodeError.empty())
        return failure("The data argument is not valid base64: " + decodeError);
    if (decoded.empty())
        return failure("The data argument decoded to an empty document.");
    if (maxBytes != 0 && decoded.size() > maxBytes)
        return failure("The document is larger than the server accepts.");

    const std::string filename = McpTools::filenameOrDefault(filenameArg);
    const std::string fromPath = McpTools::writeIncomingDocument(filename, decoded);
    if (fromPath.empty())
        return failure("The server could not store the document.");
    return success(fromPath, filename);
}

McpTools::ResolvedInput resolveFromFileId(const std::string& fileId, const std::string& filenameArg)
{
    const std::string uploadedPath = McpUploads::instance().lookup(fileId);
    if (uploadedPath.empty())
        return failure("The file_id is unknown or has expired. Upload the document again.");

    // Each tool run gets its own link to the upload, so the upload stays available to other calls
    // until its time to live passes.
    std::string filename = filenameArg;
    if (filename.empty())
        filename = Poco::Path(uploadedPath).getFileName();
    filename = McpTools::filenameOrDefault(filename);

    // The upload and this copy both live under the child root, so a hard link stands in for copying
    // the bytes. Removing one leaves the other in place.
    const std::string copyPath = McpTools::linkIncomingDocument(filename, uploadedPath);
    if (copyPath.empty())
        return failure("The server could not store the document.");
    return success(copyPath, filename);
}
}

namespace McpTools
{
std::size_t maxDocumentBytes()
{
    return ConfigUtil::getConfigValue<std::size_t>("ai.mcp.max_file_size", 0);
}

std::string describeFailure(const std::string& kind, const std::string& action)
{
    if (kind.starts_with("passwordrequired") || kind == "wrongpassword")
        return "The document is protected with a password, and the server cannot open it. Ask "
               "for a copy without the password.";
    if (kind == "faileddocloading")
        return "The document could not be loaded. The file may be damaged, or in a format the "
               "server does not read. Check that filename carries the right extension.";
    if (kind == "docloadtimeout")
        return "Loading the document took too long. Try a smaller document.";
    if (kind == "savefailed")
        return "The document could not be saved in the requested format. Check that the format "
               "suits this kind of document.";
    return "The document could not be " + action + ": " + kind;
}

std::string filenameOrDefault(const std::string& requested)
{
    std::string cleaned = Util::cleanupFilename(requested);
    if (cleaned.empty() || cleaned.find_first_not_of('.') == std::string::npos)
        return "document";

    if (cleaned.size() > MaxFilenameBytes)
    {
        // The extension is kept, and the name before it is cut. A byte of the form 10xxxxxx
        // continues a UTF-8 character, so the cut moves back to the first byte of a character.
        constexpr std::size_t MaxExtensionBytes = 16;
        const std::size_t dot = cleaned.rfind('.');
        const std::string extension =
            dot != std::string::npos && cleaned.size() - dot <= MaxExtensionBytes
                ? cleaned.substr(dot)
                : std::string();
        std::size_t stemEnd = MaxFilenameBytes - extension.size();
        while (stemEnd > 0 && (static_cast<unsigned char>(cleaned[stemEnd]) & 0xc0) == 0x80)
            --stemEnd;
        cleaned = cleaned.substr(0, stemEnd) + extension;
    }

    return cleaned;
}

std::string makeIncomingPath(const std::string& filename)
{
    const std::string root = COOLWSD::ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH;
    std::string directory;
    try
    {
        directory = FileUtil::createRandomTmpDir(root);
    }
    catch (const std::exception& exc)
    {
        // The root is made first, and a root that cannot be made throws.
        LOG_ERR("MCP: failed to create [" << root << "]: " << exc.what());
    }

    if (directory.empty() || directory == root)
    {
        LOG_ERR("MCP: failed to create an incoming directory under [" << root << ']');
        return std::string();
    }

    Poco::Path tempPath = Poco::Path::forDirectory(directory + '/');
    tempPath.setFileName(McpTools::filenameOrDefault(filename));
    return tempPath.toString();
}

std::string writeIncomingDocument(const std::string& filename, const std::string& bytes)
{
    return writeIncomingFile(filename,
                             [&bytes](std::ostream& out)
                             {
                                 out.write(bytes.data(), bytes.size());
                                 return out.good();
                             });
}

std::string writeIncomingDocument(const std::string& filename, std::istream& source,
                                  std::size_t size)
{
    return writeIncomingFile(filename,
                             [&source, size](std::ostream& out)
                             {
                                 char buffer[64 * 1024];
                                 std::size_t left = size;
                                 while (left > 0)
                                 {
                                     source.read(buffer, std::min(left, sizeof(buffer)));
                                     const std::streamsize got = source.gcount();
                                     if (got <= 0)
                                         return false;
                                     out.write(buffer, got);
                                     left -= static_cast<std::size_t>(got);
                                 }
                                 return out.good();
                             });
}

std::string linkIncomingDocument(const std::string& filename, const std::string& source)
{
    const std::string path = makeIncomingPath(filename);
    if (path.empty())
        return std::string();

    if (!FileUtil::linkOrCopyFile(source, path))
    {
        LOG_ERR("MCP: failed to link [" << source << "] to [" << path << ']');
        FileUtil::removeFile(Poco::Path(path).parent().toString(), /*recursive=*/true);
        return std::string();
    }
    return path;
}

ResolvedInput resolveInput(const Poco::JSON::Object::Ptr& arguments)
{
    const std::string data = arguments->optValue<std::string>("data", std::string());
    const std::string fileId = arguments->optValue<std::string>("file_id", std::string());
    const std::string filenameArg = arguments->optValue<std::string>("filename", std::string());

    if (data.empty() && fileId.empty())
        return failure("Provide the document as the data or the file_id argument.");
    if (!data.empty() && !fileId.empty())
        return failure("Provide the document as only one of the data or file_id arguments.");

    if (!data.empty())
        return resolveFromData(data, filenameArg);
    return resolveFromFileId(fileId, filenameArg);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
