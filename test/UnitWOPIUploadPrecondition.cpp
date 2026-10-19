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
 * Unit tests for a host refusing an upload with 428 (Precondition Required).
 */

#include <config.h>

#include <test/SecurityLabelTest.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

/// The host's message for the user, and the error frame that carries it to a view.
static constexpr const char* PreconditionMessage = "Apply a security label first.";
static constexpr const char* PreconditionError =
    "error: cmd=storage kind=saveprecondition errordetail=Apply%20a%20security%20label%20first.";

/// The editor saves and the host refuses the upload with 428. The editor and the user who may
/// only comment are told, with the host's message; the reader, who cannot upload, is not. The
/// refused file is not sent again, but the next save is, and the host takes it.
class UnitWOPIUploadPrecondition : public SecurityLabelTest
{
    /// How long to watch for a retry of the refused file. Many times the minimum time between
    /// uploads, so a retry would have happened by then.
    static constexpr std::chrono::seconds NoRetryDuration = std::chrono::seconds(3);

    std::atomic<std::chrono::steady_clock::time_point> _refusedTime;
    std::atomic_bool _savedAgain;

public:
    UnitWOPIUploadPrecondition()
        : SecurityLabelTest("UnitWOPIUploadPrecondition", { "editor", "reader", "commenter" })
        , _savedAgain(false)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        SecurityLabelTest::configure(config);

        config.setUInt("per_document.min_time_between_uploads_ms", 100); // Short retry interval.
    }

    void configFileInfo(const std::string& token, const std::string& /*label*/,
                        Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (token == "reader")
            makeReader(fileInfo);

        if (token == "commenter")
            makeCommenter(fileInfo);
    }

    std::unique_ptr<http::Response> answerUpload(std::size_t count) override
    {
        if (count > 1)
        {
            LOK_ASSERT_MESSAGE("Expected no upload before the next save", _savedAgain);
            return nullptr;
        }

        _refusedTime = std::chrono::steady_clock::now();
        return makePreconditionResponse(PreconditionMessage);
    }

    void onViewsLoaded() override { modifyAndSave(); }

    void onTick() override
    {
        if (getUploadCount() == 1 &&
            std::chrono::steady_clock::now() - _refusedTime.load() >= NoRetryDuration &&
            !_savedAgain.exchange(true))
        {
            TST_LOG("No retry of the refused file; saving again");
            modifyAndSave();
        }
    }

    bool isSettled() override { return getUploadedCount() == 1; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected the editor and the commenter alone to be told",
                                 std::size_t(2), getErrorCount("saveprecondition"));
        LOK_ASSERT_EQUAL_STR(PreconditionError, getLastError("saveprecondition"));
        LOK_ASSERT_EQUAL(std::size_t(2), getUploadCount());
    }
};

/// The editor saves the document as a new file and the host refuses it with 428. Only the editor
/// is told, as the other editor did not ask for it, and the document itself is uploaded as usual
/// on its next save.
class UnitWOPISaveAsPrecondition : public SecurityLabelTest
{
    std::atomic_bool _saved;

public:
    UnitWOPISaveAsPrecondition()
        : SecurityLabelTest("UnitWOPISaveAsPrecondition", { "editor", "other" })
        , _saved(false)
    {
    }

    std::unique_ptr<http::Response>
    answerPutRelativeFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Refusing the save-as with 428");
        return makePreconditionResponse(PreconditionMessage);
    }

    void onViewsLoaded() override
    {
        WSD_CMD_BY_CONNECTION_INDEX(0, "saveas url=wopi:///path/to/hello.pdf");
    }

    void onTick() override
    {
        if (getErrorCount("saveprecondition") == 1 && !_saved.exchange(true))
            modifyAndSave();
    }

    bool isSettled() override { return getUploadedCount() == 1; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected the editor alone to be told", std::size_t(1),
                                 getErrorCount("saveprecondition"));
        LOK_ASSERT_EQUAL_STR(PreconditionError, getLastError("saveprecondition"));
        LOK_ASSERT_EQUAL(std::size_t(1), getUploadCount());
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []{ new UnitWOPIUploadPrecondition(), new UnitWOPISaveAsPrecondition(),
                             nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
