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
 * Special document brokers for system operations.
 * Classes: ThumbnailBroker, SystemTemplatesBroker
 */

#pragma once

#if MOBILEAPP
#error This file should be excluded from Mobile App builds
#endif // MOBILEAPP

#include <wsd/DocumentBroker.hpp>

#include <cstddef>
#include <memory>
#include <string>

#include <Poco/URI.h>

class ClientSession;

class StatelessBatchBroker : public DocumentBroker
{
protected:
    std::shared_ptr<ClientSession> _clientSession;

public:
    StatelessBatchBroker(const std::string& uri, const Poco::URI& uriPublic,
                         const std::string& docKey)
        : DocumentBroker(ChildType::Batch, uri, uriPublic, docKey, /*configId=*/std::string())
    {
    }

    /// Cleanup path and its parent
    static void removeFile(const std::string& uri);
};

class ConvertToBroker : public StatelessBatchBroker
{
    const std::string _format;
    const std::string _options;
    const std::string _inFilterOptions;
    const std::string _lang;

public:
    /// Construct DocumentBroker with URI and docKey
    ConvertToBroker(const std::string& uri, const Poco::URI& uriPublic, const std::string& docKey,
                    const std::string& format, const std::string& options,
                    const std::string& lang);
    virtual ~ConvertToBroker();

    /// _lang accessors
    const std::string& getLang() { return _lang; }

    /// Move socket to this broker for response & do conversion
    bool startConversion(SocketDisposition& disposition, const std::string& id, const AdditionalFilePocoUris& additionalFileUrisPublic);

    /// When the load completes - lets start saving
    void setLoaded() override;

    /// Called when removed from the DocBrokers list
    void dispose() override;

    /// How many live conversions are running.
    static std::size_t getInstanceCount();

protected:
    bool isConvertTo() const override { return true; }

    virtual bool isReadOnly() const { return true; }

    virtual bool isGetThumbnail() const { return false; }

    virtual void sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                                  const std::string& encodedFrom);
};

class ExtractLinkTargetsBroker final : public ConvertToBroker
{
public:
    /// Construct DocumentBroker with URI and docKey
    ExtractLinkTargetsBroker(const std::string& uri, const Poco::URI& uriPublic,
                             const std::string& docKey, const std::string& lang)
        : ConvertToBroker(uri, uriPublic, docKey, "", "", lang)
    {
    }

private:
    void sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                          const std::string& encodedFrom) override;
};

class ExtractDocumentStructureBroker final : public ConvertToBroker
{
public:
    const std::string _filter;
    /// Construct DocumentBroker with URI and docKey
    ExtractDocumentStructureBroker(const std::string& uri, const Poco::URI& uriPublic,
                                   const std::string& docKey, const std::string& lang,
                                   const std::string& filter)
        : ConvertToBroker(uri, uriPublic, docKey, Poco::Path(uri).getExtension(), "", lang)
        , _filter(filter)
    {
    }

private:
    void sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                          const std::string& encodedFrom) override;
};

class TransformDocumentStructureBroker final : public ConvertToBroker
{
public:
    const std::string _transformJSON;
    /// Construct DocumentBroker with URI and docKey
    TransformDocumentStructureBroker(const std::string& uri, const Poco::URI& uriPublic,
                                     const std::string& docKey, const std::string& format,
                                     const std::string& lang, const std::string& transformJSON)
        : ConvertToBroker(uri, uriPublic, docKey, format, "", lang)
        , _transformJSON(transformJSON)
    {
    }

private:
    virtual bool isReadOnly() const override { return false; }

    void sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                          const std::string& encodedFrom) override;
};

class GetThumbnailBroker final : public ConvertToBroker
{
    std::string _target;

public:
    /// Construct DocumentBroker with URI and docKey
    GetThumbnailBroker(const std::string& uri, const Poco::URI& uriPublic,
                       const std::string& docKey, const std::string& lang,
                       const std::string& target)
        : ConvertToBroker(uri, uriPublic, docKey, std::string(), std::string(), lang)
        , _target(target)
    {
    }

protected:
    bool isGetThumbnail() const override { return true; }

private:
    void sendStartMessage(const std::shared_ptr<ClientSession>& clientSession,
                          const std::string& encodedFrom) override;
};

class RenderSearchResultBroker final : public StatelessBatchBroker
{
    std::shared_ptr<std::vector<char>> _searchResultContent;
    std::vector<char> _responseData;
    std::weak_ptr<StreamSocket> _socket;

public:
    RenderSearchResultBroker(std::string const& uri, Poco::URI const& uriPublic,
                             std::string const& docKey,
                             std::shared_ptr<std::vector<char>> const& searchResultContent);

    virtual ~RenderSearchResultBroker();

    void setResponseSocket(std::shared_ptr<StreamSocket> const& socket) { _socket = socket; }

    /// Execute command(s) and move the socket to this broker
    bool executeCommand(SocketDisposition& disposition, std::string const& id);

    /// Override method to start executing when the document is loaded
    void setLoaded() override;

    /// Called when removed from the DocBrokers list
    void dispose() override;

    /// Override to filter out the data that is returned by a command
    bool handleInput(const std::shared_ptr<Message>& message) override;

    /// How many instances are running.
    static std::size_t getInstanceCount();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
