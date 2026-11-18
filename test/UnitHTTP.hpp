/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * HTTP test utilities and helper classes.
 */

#pragma once

#include <sstream>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/SocketAddress.h>

#include <Common.hpp>

/// Unit test stub for a server response
class UnitHTTPServerResponse : public Poco::Net::HTTPServerResponse
{
    bool _sent;
    std::stringstream _dummyStream;

public:
    UnitHTTPServerResponse() :
        _sent(false)
    {
    }
    virtual void sendContinue() override {}
    virtual std::ostream& send() override
    {
        _sent = true;
        return _dummyStream;
    }
    virtual void sendFile(const std::string& /* path */,
                          const std::string& /* mediaType */) override {}
    virtual void sendBuffer(const void* /* buffer */,
                            std::size_t /* length */) override {}
    virtual void redirect(const std::string& /* uri */,
                          HTTPStatus /* status = HTTP_FOUND */) override {}
    virtual void requireAuthentication(const std::string& /* realm */) override {}
    virtual bool sent() const override { return _sent; }
};

/// Unit test stub for server params with a public dtor
class UnitHTTPServerParams : public Poco::Net::HTTPServerParams
{
public:
    ~UnitHTTPServerParams() {}
};

/// Unit test stub for a server request
class UnitHTTPServerRequest : public Poco::Net::HTTPServerRequest
{
private:
    UnitHTTPServerResponse& _response;
    Poco::Net::SocketAddress _clientAddress;
    Poco::Net::SocketAddress _serverAddress;
    std::stringstream _dummyStream;
    UnitHTTPServerParams _dummyParams;

public:
    UnitHTTPServerRequest(UnitHTTPServerResponse& inResponse,
                          const std::string& uri) :
        _response(inResponse),
        _serverAddress(9981) // FIXME: Unix Sockets now ...
    {
        setURI(uri);
    }
    virtual std::istream& stream() override
    {
        return _dummyStream;
    }
    virtual bool secure() const override
    {
        return true;
    }
    virtual const Poco::Net::SocketAddress& clientAddress() const override
    {
        return _clientAddress;
    }
    virtual const Poco::Net::SocketAddress& serverAddress() const override
    {
        return _serverAddress;
    }
    virtual const Poco::Net::HTTPServerParams& serverParams() const override
    {
        return _dummyParams;
    }
    virtual Poco::Net::HTTPServerResponse& response() const override
    {
        return _response;
    }
};

namespace UnitHTTP
{
    inline Poco::Net::HTTPClientSession* createSession()
    {
        // HTTP forced in configure hook.
        return new Poco::Net::HTTPClientSession ("127.0.0.1",
                                                 ClientPortNumber);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
