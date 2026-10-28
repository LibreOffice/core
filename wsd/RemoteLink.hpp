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

#pragma once

#include <common/JsonUtil.hpp>

#include <Poco/JSON/Object.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

/// One remote link as a storage describes it, in an entry of a POST to /cool/links and in a
/// linkaccess answer: the remote document, the document as the user knows it, when it was
/// last modified and the persistent link the pages of the open document store for it. The
/// same for every view of the open document.
struct RemoteLink
{
    /// The remote document's WOPISrc, from WOPISrc.
    std::string wopiSrc;
    /// The remote document as the user knows it, from BaseFileName. Empty when the storage
    /// named none.
    std::string name;
    /// The time the remote document was last modified, from LastModifiedTime. Empty when the
    /// storage reported none.
    std::string lastModifiedTime;
    /// The persistent link to the remote document, from PersistentLink. It is the name after
    /// vnd.collabora.slide-source:, the reference the storage resolves to the document, and
    /// the one name a view knows the remote link by.
    std::string persistentLink;

    /// Reads the fields from the JSON object the storage sent. A field the object lacks is
    /// left empty.
    void readFrom(const Poco::JSON::Object::Ptr& object)
    {
        JsonUtil::findJSONValue(object, "WOPISrc", wopiSrc);
        JsonUtil::findJSONValue(object, "BaseFileName", name);
        JsonUtil::findJSONValue(object, "LastModifiedTime", lastModifiedTime);
        JsonUtil::findJSONValue(object, "PersistentLink", persistentLink);
    }

    /// Whether a persistent link is of the expected syntax: no control characters and not
    /// unusually long.
    static bool isValidPersistentLink(std::string_view persistentLink)
    {
        constexpr std::size_t MaxLength = 1024;
        return persistentLink.size() <= MaxLength &&
               std::none_of(persistentLink.begin(), persistentLink.end(),
                            [](unsigned char c) { return c < 0x20 || c == 0x7f; });
    }
};

/// A remote link together with the access token that reaches it, as a storage hands it to one
/// view. The token is private to that view.
struct RemoteLinkRegistration : RemoteLink
{
    /// The token the remote document is opened with, from AccessToken.
    std::string accessToken;

    /// Reads the fields from the JSON object the storage sent. A field the object lacks is
    /// left empty.
    void readFrom(const Poco::JSON::Object::Ptr& object)
    {
        RemoteLink::readFrom(object);
        JsonUtil::findJSONValue(object, "AccessToken", accessToken);
    }

    /// Whether the link can be reached: it names a document, a token for it and the
    /// persistent link a view knows it by.
    bool isComplete() const
    {
        return !wopiSrc.empty() && !accessToken.empty() && !persistentLink.empty();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
