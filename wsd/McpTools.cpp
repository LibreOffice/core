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

#include <Poco/Path.h>

#include <fstream>
#include <string>

namespace
{
// Name a file in a new random directory under child-root/tmp/incoming, so each document has a
// directory to itself. Returns an empty path when that directory could not be created, which
// FileUtil::createRandomTmpDir reports by handing back the root it was given.
std::string makeIncomingPath(const std::string& filename)
{
    const std::string root = COOLWSD::ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH;
    const std::string directory = FileUtil::createRandomTmpDir(root);
    if (directory.empty() || directory == root)
    {
        LOG_ERR("MCP: failed to create an incoming directory under [" << root << ']');
        return std::string();
    }

    Poco::Path tempPath = Poco::Path::forDirectory(directory + '/');
    tempPath.setFileName(filename);
    return tempPath.toString();
}

// Write the given bytes to a fresh file named after filename. Returns the path of the written file,
// or an empty string when the file cannot be created.
std::string writeIncomingFile(const std::string& filename, const std::string& bytes)
{
    const std::string path = makeIncomingPath(filename);
    if (path.empty())
        return std::string();

    std::ofstream fileStream;
    fileStream.open(path, std::ios::binary);
    if (!fileStream.good())
    {
        LOG_ERR("MCP: failed to open incoming file [" << path << ']');
        return std::string();
    }
    fileStream.write(bytes.data(), bytes.size());
    fileStream.close();
    if (!fileStream.good())
    {
        LOG_ERR("MCP: failed to write incoming file [" << path << ']');
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
    const std::string fromPath = writeIncomingFile(filename, decoded);
    if (fromPath.empty())
        return failure("The server could not store the document.");
    return success(fromPath, filename);
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
    const std::string cleaned = Util::cleanupFilename(requested);
    if (cleaned.empty() || cleaned.find_first_not_of('.') == std::string::npos)
        return "document";

    return cleaned;
}

ResolvedInput resolveInput(const Poco::JSON::Object::Ptr& arguments)
{
    const std::string data = arguments->optValue<std::string>("data", std::string());
    const std::string filenameArg = arguments->optValue<std::string>("filename", std::string());

    if (data.empty())
        return failure("Provide the document as the data argument.");

    return resolveFromData(data, filenameArg);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
