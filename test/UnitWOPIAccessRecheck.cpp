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
 * Unit tests for re-checking each view's access after a security label change.
 */

#include <config.h>

#include <test/SecurityLabelTest.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <thread>

using namespace std::literals;

/// Five users edit a document: the one changing its security label, one the host makes a reader
/// with the new label, one it denies, one it gives no decision on, and one who may only comment,
/// before and after. The label change is uploaded as an autosave, and the host is then asked
/// about each of them: the denied view is disconnected, as is the reader's (after a transient
/// failure is retried), and the undecided, the editor's and the commenter's views are left
/// alone. The editor's token is refreshed before the label change, and the host is asked
/// with the new one.
class UnitWOPIAccessRecheck : public SecurityLabelTest
{
    /// The editor's token after the refresh. The host receives its "%41" as it is, decoded once
    /// from the URL like any token.
    static constexpr const char* RefreshedToken = "editor%41";

    std::atomic_bool _refreshPending;
    std::atomic_bool _refreshed;

public:
    UnitWOPIAccessRecheck()
        : SecurityLabelTest("UnitWOPIAccessRecheck",
                            { "editor", "reader", "denied", "undecided", "commenter" })
        , _refreshPending(false)
        , _refreshed(false)
    {
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& /*label*/,
                               std::size_t count,
                               std::unique_ptr<http::Response>& response) override
    {
        // The editor's first token was replaced by a refresh, and the host takes it no more.
        if (token == "denied" || token == "editor")
            response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        else if (token == "reader" && count == 1)
            response = std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        else if (token == "undecided")
            response = std::make_unique<http::Response>(http::StatusCode::BadRequest);

        return response ? Answer::Respond : Answer::Default;
    }

    void configFileInfo(const std::string& token, const std::string& label,
                        Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (!label.empty() && token == "reader")
            makeReader(fileInfo);

        if (token == "commenter")
            makeCommenter(fileInfo);
    }

    void onViewsLoaded() override
    {
        TST_LOG("Refreshing the editor's token");
        WSD_CMD_BY_CONNECTION_INDEX(0, "resetaccesstoken "s + RefreshedToken);
    }

    void onTick() override
    {
        // The token is the broker's, so look at it there, and change the label once the editor
        // has the new one.
        if (_refreshed || _refreshPending.exchange(true))
            return;

        const std::shared_ptr<ClientSession> editor = getSession("editor");
        editor->getDocumentBroker()->addCallback(
            [this, editor]()
            {
                if (editor->getAuthorization().matchesToken(RefreshedToken))
                {
                    _refreshed = true;
                    changeLabel("Secret");
                }

                _refreshPending = false;
            });
    }

    bool isSettled() override
    {
        return getRemoved() == "denied,reader" && getErrorCount("accessrevoked") >= 1 &&
               getErrorCount("accessdowngraded") >= 1 && getCheckCount("undecided") >= 1;
    }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected one view to lose its access", std::size_t(1),
                                 getErrorCount("accessrevoked"));
        LOK_ASSERT_EQUAL_MESSAGE("Expected one view to be disconnected as a reader",
                                 std::size_t(1), getErrorCount("accessdowngraded"));
        LOK_ASSERT_EQUAL_STR("denied,reader", getRemoved());

        LOK_ASSERT_EQUAL_MESSAGE("Expected the reader to be checked again after the transient "
                                 "failure",
                                 std::size_t(2), getCheckCount("reader"));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the editor to be checked once, with its new token",
                                 std::size_t(1), getCheckCount(RefreshedToken));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the editor's replaced token to go unused",
                                 std::size_t(0), getCheckCount("editor"));
        LOK_ASSERT_EQUAL(std::size_t(1), getCheckCount("denied"));
        LOK_ASSERT_EQUAL(std::size_t(1), getCheckCount("undecided"));
        LOK_ASSERT_EQUAL(std::size_t(1), getCheckCount("commenter"));

        LOK_ASSERT_EQUAL_MESSAGE("Expected the label change to be uploaded as an autosave",
                                 "autosave"s, getAutosaves());

        const std::shared_ptr<ClientSession> editor = getSession("editor");
        LOK_ASSERT_MESSAGE("Expected the editor to keep editing",
                           editor->isWritable() && !editor->isReadOnly());
        const std::shared_ptr<ClientSession> commenter = getSession("commenter");
        LOK_ASSERT_MESSAGE("Expected the commenter to keep commenting",
                           commenter->isWritable() && commenter->isAllowChangeComments());
    }
};

/// The host answers each user's CheckFileInfo after the label change in another way. A refusal
/// (403, 404) disconnects the view. A transient failure (429, 500, 502, 504) and a connection
/// closed with no answer are retried, and the view stays once the host lets it. An answer that
/// cannot be read gives no decision, and the view stays as it was.
class UnitWOPIAccessRecheckAnswers : public SecurityLabelTest
{
    /// The first answer to each user's check, by token. The ones after it are 200.
    static const std::map<std::string, http::StatusCode>& firstAnswers()
    {
        static const std::map<std::string, http::StatusCode> answers = {
            { "forbidden", http::StatusCode::Forbidden },
            { "notfound", http::StatusCode::NotFound },
            { "toomany", http::StatusCode::TooManyRequests },
            { "servererror", http::StatusCode::InternalServerError },
            { "badgateway", http::StatusCode::BadGateway },
            { "gatewaytimeout", http::StatusCode::GatewayTimeout },
        };
        return answers;
    }

public:
    UnitWOPIAccessRecheckAnswers()
        : SecurityLabelTest("UnitWOPIAccessRecheckAnswers",
                            { "editor", "forbidden", "notfound", "toomany", "servererror",
                              "badgateway", "gatewaytimeout", "dropped", "unreadable" })
    {
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& /*label*/,
                               std::size_t count,
                               std::unique_ptr<http::Response>& response) override
    {
        if (count > 1)
            return Answer::Default;

        if (token == "dropped")
            return Answer::Drop;

        if (token == "unreadable")
        {
            response = std::make_unique<http::Response>(http::StatusCode::OK);
            response->setBody("This is not JSON", "application/json; charset=utf-8");
            return Answer::Respond;
        }

        const auto it = firstAnswers().find(token);
        if (it == firstAnswers().end())
            return Answer::Default;

        response = std::make_unique<http::Response>(it->second);
        return Answer::Respond;
    }

    bool isSettled() override
    {
        return getRemoved() == "forbidden,notfound" && getCheckCount("unreadable") == 1 &&
               getCheckCount("toomany") == 2 &&
               getCheckCount("servererror") == 2 && getCheckCount("badgateway") == 2 &&
               getCheckCount("gatewaytimeout") == 2 && getCheckCount("dropped") == 2;
    }

    void verify() override
    {
        LOK_ASSERT_EQUAL(std::size_t(2), getErrorCount("accessrevoked"));
        LOK_ASSERT_EQUAL(std::size_t(0), getErrorCount("accessdowngraded"));
        LOK_ASSERT_EQUAL_STR("forbidden,notfound", getRemoved());

        for (const char* token : { "editor", "forbidden", "notfound", "unreadable" })
            LOK_ASSERT_EQUAL_MESSAGE(token, std::size_t(1), getCheckCount(token));
    }
};

/// The host never decides on two users after the label change: it closes the connection of one
/// with no answer each time, and is busy for the other each time. Each is asked five times in
/// all, and the view stays as it was.
class UnitWOPIAccessRecheckGiveUp : public SecurityLabelTest
{
public:
    UnitWOPIAccessRecheckGiveUp()
        : SecurityLabelTest("UnitWOPIAccessRecheckGiveUp", { "editor", "silent", "busy" })
    {
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& /*label*/,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        if (token == "silent")
            return Answer::Drop;

        if (token == "busy")
        {
            response = std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
            return Answer::Respond;
        }

        return Answer::Default;
    }

    bool isSettled() override
    {
        return getCheckCount("silent") == 5 && getCheckCount("busy") == 5;
    }

    void verify() override
    {
        LOK_ASSERT_EQUAL_STR("", getRemoved());
        LOK_ASSERT_EQUAL(std::size_t(5), getCheckCount("silent"));
        LOK_ASSERT_EQUAL(std::size_t(5), getCheckCount("busy"));
        LOK_ASSERT_EQUAL(std::size_t(1), getCheckCount("editor"));
    }
};

/// Removing the label is saved and uploaded as an autosave, and nobody is asked about once the
/// host stores the document without a label, although the host would refuse them all.
class UnitWOPIAccessRecheckRemoved : public SecurityLabelTest
{
public:
    UnitWOPIAccessRecheckRemoved()
        : SecurityLabelTest("UnitWOPIAccessRecheckRemoved", { "editor", "reader" })
    {
    }

    Answer answerCheckFileInfo(const std::string& /*token*/, const std::string& /*label*/,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        return Answer::Respond;
    }

    void onViewsLoaded() override { changeLabel(std::string()); }

    bool isSettled() override { return getUploadedCount() == 1; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected the removal to be uploaded as an autosave",
                                 "autosave"s, getAutosaves());
        LOK_ASSERT_EQUAL_MESSAGE("Expected no CheckFileInfo after the removal", std::size_t(0),
                                 getCheckCount());
        LOK_ASSERT_EQUAL(std::size_t(0), getErrorCount("accessrevoked"));
        LOK_ASSERT_EQUAL_STR("", getRemoved());
    }
};

/// The host is slow to answer the checks after the label is applied, and makes the reader one
/// who may only read with it. Before it answers, the label is changed again, or removed, or the
/// document is only saved. A newer label that the host stores supersedes the slow answers. A
/// save without a new label does not, and nobody is asked again; nor does a removal that the
/// host refuses with 428, as it keeps the label.
class UnitWOPIAccessRecheckSlow : public SecurityLabelTest
{
public:
    /// What follows the label while the host is slow to answer.
    enum class Then
    {
        Change, ///< A new label, under which the host refuses the other user instead.
        Remove, ///< The label is removed.
        RemoveRefused, ///< The label is removed, and the host refuses that upload with 428.
        Save, ///< The document is modified and saved, with the label unchanged.
    };

private:
    const Then _then;
    std::atomic_bool _followed;
    std::atomic_bool _released;

    static std::string getName(Then then)
    {
        switch (then)
        {
            case Then::Change:
                return "UnitWOPIAccessRecheckSlowThenChange";
            case Then::Remove:
                return "UnitWOPIAccessRecheckSlowThenRemove";
            case Then::RemoveRefused:
                return "UnitWOPIAccessRecheckSlowThenRemoveRefused";
            case Then::Save:
                break;
        }

        return "UnitWOPIAccessRecheckSlowThenSave";
    }

public:
    explicit UnitWOPIAccessRecheckSlow(Then then)
        : SecurityLabelTest(getName(then), { "editor", "reader", "other" })
        , _then(then)
        , _followed(false)
        , _released(false)
    {
    }

    std::unique_ptr<http::Response> answerUpload(std::size_t count) override
    {
        if (count == 2 && _then == Then::RemoveRefused)
            return makePreconditionResponse("Keep the label.");
        return nullptr;
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& label,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        if (label == "Secret")
            return Answer::Hold;

        if (label == "TopSecret" && token == "other")
        {
            response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
            return Answer::Respond;
        }

        return Answer::Default;
    }

    void configFileInfo(const std::string& token, const std::string& label,
                        Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (label == "Secret" && token == "reader")
            makeReader(fileInfo);
    }

    void onTick() override
    {
        // Every view's check of the first label waits on the host.
        if (_followed || getHeldCount() < 3)
            return;

        _followed = true;
        switch (_then)
        {
            case Then::Change:
                changeLabel("TopSecret");
                break;
            case Then::Remove:
            case Then::RemoveRefused:
                changeLabel(std::string());
                break;
            case Then::Save:
                modifyAndSave();
                break;
        }
    }

    void onUploaded(std::size_t count, bool /*success*/) override
    {
        // The upload of what followed the label is done, and the host answers the slow checks
        // now.
        if (count == 2)
        {
            _released = true;
            releaseHeldCheckFileInfo();
        }
    }

    bool isSettled() override
    {
        if (!_released)
            return false;

        switch (_then)
        {
            case Then::Change:
                return getRemoved() == "other";
            case Then::Remove:
                return true;
            case Then::RemoveRefused:
            case Then::Save:
                break;
        }

        return getRemoved() == "reader";
    }

    void verify() override
    {
        LOK_ASSERT_EQUAL(std::size_t(2), getUploadCount());
        switch (_then)
        {
            case Then::Change:
                // Each view is asked again with the second label, and the slow answers for the
                // first count for nothing: the other user is refused, and the reader stays.
                LOK_ASSERT_EQUAL_STR("Secret,TopSecret", getChecks("reader"));
                LOK_ASSERT_EQUAL_STR("Secret,TopSecret", getChecks("other"));
                LOK_ASSERT_EQUAL_STR("other", getRemoved());
                LOK_ASSERT_EQUAL(std::size_t(1), getErrorCount("accessrevoked"));
                LOK_ASSERT_EQUAL(std::size_t(0), getErrorCount("accessdowngraded"));
                break;
            case Then::Remove:
                // Nobody is asked again, and the slow answers count for nothing.
                LOK_ASSERT_EQUAL_STR("Secret", getChecks("reader"));
                LOK_ASSERT_EQUAL(std::size_t(3), getCheckCount());
                LOK_ASSERT_EQUAL_STR("", getRemoved());
                LOK_ASSERT_EQUAL(std::size_t(0), getErrorCount("accessdowngraded"));
                break;
            case Then::RemoveRefused:
            case Then::Save:
                // Nobody is asked again, and the reader is disconnected once the host answers.
                LOK_ASSERT_EQUAL(std::size_t(3), getCheckCount());
                LOK_ASSERT_EQUAL_STR("reader", getRemoved());
                LOK_ASSERT_EQUAL(std::size_t(1), getErrorCount("accessdowngraded"));
                break;
        }
    }
};

/// The label is removed and applied again at once. Whether the two changes are uploaded
/// together or apart, the host stores the label last, and is asked about every user then: it
/// refuses the reader.
class UnitWOPIAccessRecheckRemovedThenApplied : public SecurityLabelTest
{
public:
    UnitWOPIAccessRecheckRemovedThenApplied()
        : SecurityLabelTest("UnitWOPIAccessRecheckRemovedThenApplied", { "editor", "reader" })
    {
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& /*label*/,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        if (token != "reader")
            return Answer::Default;

        response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        return Answer::Respond;
    }

    void onViewsLoaded() override
    {
        changeLabel(std::string());
        changeLabel("Secret");
    }

    bool isSettled() override { return getRemoved() == "reader"; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_STR("Secret", getChecks("reader"));
        LOK_ASSERT_EQUAL_STR("Secret", getChecks("editor"));
        LOK_ASSERT_EQUAL(std::size_t(1), getErrorCount("accessrevoked"));
    }
};

/// The label is changed while the host is still taking an upload of the document without it. The
/// label is saved and uploaded once that upload is done, and the host is asked about every user
/// only after it stores the label: it refuses the reader.
class UnitWOPIAccessRecheckDuringUpload : public SecurityLabelTest
{
public:
    UnitWOPIAccessRecheckDuringUpload()
        : SecurityLabelTest("UnitWOPIAccessRecheckDuringUpload", { "editor", "reader" })
    {
    }

    std::unique_ptr<http::Response> answerUpload(std::size_t count) override
    {
        if (count == 1)
        {
            // The host takes its time over the upload, on a thread of its own.
            changeLabel("Secret");
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        return nullptr;
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& /*label*/,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        if (token != "reader")
            return Answer::Default;

        response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        return Answer::Respond;
    }

    void onViewsLoaded() override { modifyAndSave(); }

    bool isSettled() override { return getRemoved() == "reader"; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected the save, then the label as an autosave",
                                 "save,autosave"s, getAutosaves());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the reader to be asked once, after the host stored "
                                 "the label",
                                 "Secret"s, getChecks("reader"));
        LOK_ASSERT_EQUAL_STR("reader", getRemoved());
    }
};

/// The host fails the upload of the label once. The users are asked about once an upload of the
/// label succeeds, and the host refuses the reader with the label. After the failure, the host may
/// also be asked what it stores, which it answers as without the label.
class UnitWOPIAccessRecheckUploadFailed : public SecurityLabelTest
{
    std::atomic_bool _saved;

public:
    UnitWOPIAccessRecheckUploadFailed()
        : SecurityLabelTest("UnitWOPIAccessRecheckUploadFailed", { "editor", "reader" })
        , _saved(false)
    {
    }

    std::unique_ptr<http::Response> answerUpload(std::size_t count) override
    {
        if (count == 1)
            return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
        return nullptr;
    }

    Answer answerCheckFileInfo(const std::string& token, const std::string& label,
                               std::size_t /*count*/,
                               std::unique_ptr<http::Response>& response) override
    {
        if (token != "reader" || label != "Secret")
            return Answer::Default;

        response = std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        return Answer::Respond;
    }

    void onTick() override
    {
        // The next save uploads the document again.
        if (getUploadCount() == 1 && !_saved.exchange(true))
            modifyAndSave();
    }

    bool isSettled() override { return getRemoved() == "reader"; }

    void verify() override
    {
        // Each user is asked once with the label stored, and that is the last request.
        for (const char* token : { "reader", "editor" })
        {
            const std::string checks = getChecks(token);
            LOK_ASSERT_MESSAGE(std::string(token) + " was asked: " + checks,
                               (checks == "Secret" || checks.ends_with(",Secret")) &&
                                   checks.find("Secret") == checks.rfind("Secret"));
        }

        LOK_ASSERT_EQUAL_STR("reader", getRemoved());
    }
};

/// The host refuses the upload of the label with 428. Every view that can upload is told, with
/// the host's message, which includes the one that may only comment and not the reader. Nobody
/// is asked about their access, as the host does not store the label.
class UnitWOPIAccessRecheckRefused : public SecurityLabelTest
{
public:
    UnitWOPIAccessRecheckRefused()
        : SecurityLabelTest("UnitWOPIAccessRecheckRefused", { "editor", "reader", "commenter" })
    {
    }

    std::unique_ptr<http::Response> answerUpload(std::size_t /*count*/) override
    {
        return makePreconditionResponse("Choose another label.");
    }

    void configFileInfo(const std::string& token, const std::string& /*label*/,
                        Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (token == "reader")
            makeReader(fileInfo);

        if (token == "commenter")
            makeCommenter(fileInfo);
    }

    bool isSettled() override { return getErrorCount("saveprecondition") >= 2; }

    void verify() override
    {
        LOK_ASSERT_EQUAL_MESSAGE("Expected the editor and the commenter alone to be told",
                                 std::size_t(2), getErrorCount("saveprecondition"));
        LOK_ASSERT_EQUAL_STR("error: cmd=storage kind=saveprecondition "
                             "errordetail=Choose%20another%20label.",
                             getLastError("saveprecondition"));
        LOK_ASSERT_EQUAL(std::size_t(1), getUploadCount());
        LOK_ASSERT_EQUAL(std::size_t(0), getCheckCount());
        LOK_ASSERT_EQUAL_STR("", getRemoved());
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []
    {
        new UnitWOPIAccessRecheck(), new UnitWOPIAccessRecheckAnswers(),
            new UnitWOPIAccessRecheckGiveUp(), new UnitWOPIAccessRecheckRemoved(),
            new UnitWOPIAccessRecheckSlow(UnitWOPIAccessRecheckSlow::Then::Change),
            new UnitWOPIAccessRecheckSlow(UnitWOPIAccessRecheckSlow::Then::Remove),
            new UnitWOPIAccessRecheckSlow(UnitWOPIAccessRecheckSlow::Then::RemoveRefused),
            new UnitWOPIAccessRecheckSlow(UnitWOPIAccessRecheckSlow::Then::Save),
            new UnitWOPIAccessRecheckRemovedThenApplied(), new UnitWOPIAccessRecheckDuringUpload(),
            new UnitWOPIAccessRecheckUploadFailed(),
            new UnitWOPIAccessRecheckRefused(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
