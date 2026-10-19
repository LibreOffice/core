/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/mockhttpserver.hxx>

#include <mutex>
#include <utility>

#include <osl/socket.hxx>
#include <osl/thread.hxx>
#include <rtl/string.hxx>

namespace test
{
class MockHttpServer::Thread : public osl::Thread
{
public:
    Thread(sal_uInt16 nPort, Handler aHandler)
        : m_nPort(nPort)
        , m_aHandler(std::move(aHandler))
        , m_aSocketAddr(u"127.0.0.1"_ustr, nPort)
    {
    }

    bool start()
    {
        m_aAcceptorSocket.setOption(osl_Socket_OptionReuseAddr, 1);
        if (!m_aAcceptorSocket.bind(m_aSocketAddr) || !m_aAcceptorSocket.listen())
            return false;
        create();
        m_bStarted = true;
        return true;
    }

    void stop()
    {
        if (!m_bStarted)
            return;
        m_bStarted = false;
        m_aAcceptorSocket.close();
        join();
    }

    sal_uInt16 getPort() const { return m_nPort; }

    std::vector<Request> getRequests() const
    {
        std::scoped_lock aGuard(m_aMutex);
        return m_aRequests;
    }

private:
    virtual void run() override
    {
        osl::StreamSocket aSocket;
        while (m_aAcceptorSocket.acceptConnection(aSocket) == osl_Socket_Ok)
        {
            sal_Int32 nTcpNoDelay = sal_Int32(true);
            aSocket.setOption(osl_Socket_OptionTcpNoDelay, &nTcpNoDelay, sizeof(nTcpNoDelay),
                              osl_Socket_LevelTcp);
            const Request aRequest = read(aSocket);
            {
                std::scoped_lock aGuard(m_aMutex);
                m_aRequests.push_back(aRequest);
            }
            write(aSocket, m_aHandler(aRequest));
            aSocket.close();
        }
    }

    // Reads the whole request, as far as Content-Length says: closing the
    // socket with part of it unread resets the connection, and the client
    // then reports the reset instead of the answer.
    static Request read(osl::StreamSocket& rSocket)
    {
        std::string aText;
        size_t nHeaderEnd = std::string::npos;
        size_t nContentLength = 0;
        while (true)
        {
            char aBuffer[1024];
            const sal_Int32 nRead = rSocket.recv(aBuffer, sizeof(aBuffer));
            if (nRead <= 0)
                break;
            aText.append(aBuffer, nRead);
            if (nHeaderEnd == std::string::npos)
            {
                nHeaderEnd = aText.find("\r\n\r\n");
                if (nHeaderEnd == std::string::npos)
                    continue;
                const size_t nField = aText.find("Content-Length: ");
                if (nField != std::string::npos && nField < nHeaderEnd)
                    nContentLength = std::stoul(aText.substr(nField + 16));
            }
            if (aText.size() >= nHeaderEnd + 4 + nContentLength)
                break;
        }

        Request aRequest;
        const size_t nLineEnd = aText.find("\r\n");
        const std::string aLine = aText.substr(0, nLineEnd);
        const size_t nMethodEnd = aLine.find(' ');
        if (nMethodEnd != std::string::npos)
        {
            aRequest.aMethod = aLine.substr(0, nMethodEnd);
            const size_t nPathEnd = aLine.find(' ', nMethodEnd + 1);
            aRequest.aPath = aLine.substr(nMethodEnd + 1, nPathEnd - nMethodEnd - 1);
        }
        if (nHeaderEnd != std::string::npos && nLineEnd != std::string::npos)
        {
            aRequest.aHeaders = aText.substr(nLineEnd + 2, nHeaderEnd + 2 - (nLineEnd + 2));
            aRequest.aBody = aText.substr(nHeaderEnd + 4);
        }
        return aRequest;
    }

    static void write(osl::StreamSocket& rSocket, const Response& rResponse)
    {
        std::string aText = "HTTP/1.1 " + std::to_string(rResponse.nStatus) + " "
                            + rResponse.aStatusText + "\r\nServer: MockHttpServer\r\n";
        if (!rResponse.aContentType.empty())
            aText += "Content-Type: " + rResponse.aContentType + "\r\n";
        aText += "Content-Length: " + std::to_string(rResponse.aBody.size()) + "\r\n\r\n"
                 + rResponse.aBody;
        rSocket.write(aText.data(), aText.size());
    }

    const sal_uInt16 m_nPort;
    const Handler m_aHandler;
    osl::SocketAddr m_aSocketAddr;
    osl::AcceptorSocket m_aAcceptorSocket;
    bool m_bStarted = false;
    mutable std::mutex m_aMutex;
    std::vector<Request> m_aRequests;
};

MockHttpServer::MockHttpServer(sal_uInt16 nPort, Handler aHandler)
    : m_pThread(new Thread(nPort, std::move(aHandler)))
{
}

MockHttpServer::~MockHttpServer() { stop(); }

bool MockHttpServer::start() { return m_pThread->start(); }

void MockHttpServer::stop() { m_pThread->stop(); }

OUString MockHttpServer::getUrl(std::u16string_view rPath) const
{
    return "http://127.0.0.1:" + OUString::number(m_pThread->getPort()) + rPath;
}

std::vector<MockHttpServer::Request> MockHttpServer::getRequests() const
{
    return m_pThread->getRequests();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
