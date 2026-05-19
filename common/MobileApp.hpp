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

#if MOBILEAPP

#include <wsd/Storage.hpp>

#ifdef IOS
#import "CODocument.h"
#import <set>
#import <string>
#endif

// On iOS at least we want to be able to have several documents open in the same app process.

// It is somewhat complicated to make sure we access the same COKit object for the document
// in both the iOS-specific Objective-C++ code and in the mostly generic Online C++ code.

// We pass around a numeric ever-increasing document identifier that gets bumped for each document
// the system asks the app to open.

// For iOS, it is the static std::atomic<unsigned> appDocIdCounter in CODocument.mm.

// In practice it will probably be equivalent to the DocumentBroker::DocBrokerId or the number that
// the core SfxViewShell::GetDocId() returns, but there might be situations where multi-threading
// and opening of several documents in sequence very quickly might cause discrepancies, so it is
// better to use a different counter to be sure. Patches to use just one counter welcome.

namespace kit
{
class Document;
}

class DocumentData
{
private:
public:
    DocumentData() :
        loKitDocument(nullptr)
#ifdef IOS
        , coDocument(nil)
#endif
    {
    }

    kit::Document *loKitDocument;

    static DocumentData &allocate(unsigned docId);
    static DocumentData &get(unsigned docId);
    static DocumentData *getIfExists(unsigned docId);
    static void deallocate(unsigned docId);
    static int count();

#ifdef IOS
    CODocument *coDocument;
#endif
    std::weak_ptr<DocumentBroker> docBroker;
};

/// Stub/Dummy WOPI types/interface.
class WopiStorage : public StorageBase
{
public:
    class WOPIFileInfo final : public FileInfo
    {
    public:
        enum class TriState : std::uint8_t
        {
            False,
            True,
            Unset
        };

        std::string getTemplateSource() const { return std::string(); }

        bool getDisablePrint() const { return false; }
        bool getDisableExport() const { return false; }
        bool getDisableCopy() const { return false; }
        bool getEnableOwnerTermination() const { return false; }
        std::string getWatermarkText() const { return std::string(); }

        TriState getDisableChangeTrackingShow() const { return TriState::Unset; }
        TriState getDisableChangeTrackingRecord() const { return TriState::Unset; }
        TriState getHideChangeTrackingControls() const { return TriState::Unset; }
    };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
