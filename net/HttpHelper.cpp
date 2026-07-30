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
 * Implementation of HTTP utility functions for file serving and compression.
 * Functions: sendFileAndShutdown(), sendCompressed/Uncompressed file helpers
 */

#include <config.h>

#include "HttpHelper.hpp"

#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <Poco/Net/HTTPResponse.h>

#include <algorithm>
#include <string>
#include <zlib.h>

namespace
{
void sendUncompressedFileContent(const std::shared_ptr<StreamSocket>& socket,
                                 const std::string& path, const int bufferSize)
{
    std::ifstream file(path, std::ios::binary);
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(bufferSize);
    do
    {
        file.read(buf.get(), bufferSize);
        const int size = file.gcount();
        if (size > 0)
            socket->send(buf.get(), size, true);
        else
            break;
    } while (file);
}

void sendDeflatedFileContent(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
                             const int fileSize)
{
    // FIXME: Should compress once ahead of time
    // compression of bundle.js takes significant time:
    //   200's ms for level 9 (468k), 72ms for level 1(587k)
    //   down from 2Mb.
    if (fileSize > 0)
    {
        std::ifstream file(path, std::ios::binary);
        std::unique_ptr<char[]> buf = std::make_unique<char[]>(fileSize);
        file.read(buf.get(), fileSize);

        static const unsigned int Level = 1;
        const long unsigned int size = file.gcount();
        long unsigned int compSize = compressBound(size);
        std::unique_ptr<char[]> cbuf = std::make_unique<char[]>(compSize);
        int result = compress2(reinterpret_cast<Bytef*>(cbuf.get()), &compSize, reinterpret_cast<Bytef*>(buf.get()), size, Level);
        if (result != Z_OK)
        {
             LOG_ERR("failed compress of: " << path << " result: " << result);
             return;
        }
        if (size > 0)
            socket->send(cbuf.get(), compSize, true);
    }
}

void sendFileImpl(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
                  http::Response& response, const bool noCache, const bool deflate,
                  const bool headerOnly, const bool closeSocket)
{
    FileUtil::Stat st(path);
    if (st.bad())
    {
        LOG_WRN('#' << socket->getFD() << ": Failed to stat [" << path
                    << "]. File will not be sent.");
        throw Poco::FileNotFoundException("Failed to stat [" + path + "]. File will not be sent.");
    }

    if (!noCache)
    {
        // 60 * 60 * 24 * 128 (days) = 11059200
        response.set("Cache-Control", "max-age=11059200");
        response.set("ETag", '"' + Util::getCoolVersionHash() + '"');
    }
    else
    {
        response.set("Cache-Control", "no-cache");
    }

    response.add("X-Content-Type-Options", "nosniff");

    if (closeSocket)
    {
        response.setConnectionToken(http::Header::ConnectionToken::Close);
    }

    int bufferSize = std::min<std::size_t>(st.size(), Socket::MaximumSendBufferSize);
    if (static_cast<long>(st.size()) >= socket->getSendBufferSize())
    {
        socket->setSocketBufferSize(bufferSize);
        bufferSize = socket->getSendBufferSize();
    }

    // Disable deflate for now - until we can cache deflated data.
    // FIXME: IE/Edge doesn't work well with deflate, so check with
    // IE/Edge before enabling the deflate again
    if (!deflate || true)
    {
        response.setContentLength(st.size());
        LOG_TRC('#' << socket->getFD() << ": Sending " << (headerOnly ? "header for " : "")
                    << " file [" << path << "].");
        socket->send(response);

        if (!headerOnly)
            sendUncompressedFileContent(socket, path, bufferSize);
    }
    else
    {
        response.set("Content-Encoding", "deflate");
        LOG_TRC('#' << socket->getFD() << ": Sending " << (headerOnly ? "header for " : "")
                    << " file [" << path << "].");
        socket->send(response);

        if (!headerOnly)
            sendDeflatedFileContent(socket, path, st.size());
    }
    if(closeSocket) {
        socket->asyncShutdown();
    }
}

} // namespace

namespace HttpHelper
{
void sendFile(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
              http::Response& response, const bool noCache,
              const bool deflate, const bool headerOnly)
{
    sendFileImpl(socket, path, response, noCache, deflate, headerOnly, false);
}

void sendFileAndShutdown(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
                         http::Response& response, const bool noCache,
                         const bool deflate, const bool headerOnly)
{
    sendFileImpl(socket, path, response, noCache, deflate, headerOnly, true);
}

} // namespace HttpHelper
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
