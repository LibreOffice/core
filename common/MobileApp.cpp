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

/*
 * Mobile app document data management.
 * Classes: DocumentData - Document lifecycle and data storage
 */

#include <config.h>

#if MOBILEAPP

#include "MobileApp.hpp"

#include <common/Log.hpp>

#include <cassert>
#include <map>
#include <mutex>

static std::map<unsigned, DocumentData*> idToDocDataMap;
static std::mutex idToDocDataMapMutex;

DocumentData &DocumentData::allocate(unsigned docId)
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);

    assert(idToDocDataMap.find(docId) == idToDocDataMap.end());
    auto p = new DocumentData();
    idToDocDataMap[docId] = p;
    return *p;
}

DocumentData & DocumentData::get(unsigned docId)
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);

    assert(idToDocDataMap.find(docId) != idToDocDataMap.end());
    return *idToDocDataMap[docId];
}

DocumentData * DocumentData::getIfExists(unsigned docId)
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);
    auto it = idToDocDataMap.find(docId);
    return it != idToDocDataMap.end() ? it->second : nullptr;
}

COKitDocument * DocumentData::loadedKitDocument(unsigned docId)
{
    const DocumentData* document = getIfExists(docId);
    return document ? document->loKitDocument : nullptr;
}

std::optional<DocumentData> DocumentData::getCopy(unsigned docId)
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);
    auto it = idToDocDataMap.find(docId);
    if (it == idToDocDataMap.end())
        return std::nullopt;
    return *it->second;
}

void DocumentData::deallocate(unsigned docId)
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);
    auto it = idToDocDataMap.find(docId);
    if (it == idToDocDataMap.end())
        return;
    delete it->second;
    idToDocDataMap.erase(it);
}

int DocumentData::count()
{
    const std::lock_guard<std::mutex> lock(idToDocDataMapMutex);
    return idToDocDataMap.size();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
