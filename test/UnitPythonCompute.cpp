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

#include <WopiTestServer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Util.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if defined(ENABLE_PYTHON_COMPUTE) && ENABLE_PYTHON_COMPUTE

/// End to end: =PY() cells -> AddIn -> kit -> wsd -> mock service -> cell values, read back
/// with gettextselection. One instance per scenario.
class UnitPythonCompute final : public WopiTestServer
{
public:
    enum class Scenario
    {
        Load, ///< a saved =PY() cell is computed on load
        Disabled, ///< disabled in coolwsd.xml: no POST, #DISABLED
        TooLarge, ///< request over max_payload_bytes: no POST, #VALUE!
        CloseMidFlight, ///< the doc closes while POSTs are open: the broker goes promptly
        QueueFifo, ///< past 8 in flight requests wait, unrefused, and are sent in order
        ResponseTooLarge, ///< replies over max_payload_bytes, with and without Content-Length
        ViewClosed, ///< the view that sent the request closes; the result still lands
        ServiceErrors, ///< a service that never answers, answers 500, 503s or non-JSON
        Refused, ///< nothing listens at the service URL
    };

private:
    enum class Phase
    {
        Load,
        WaitLoad,
        Check, ///< reading the cells back
        CloseView, ///< CloseMidFlight and ViewClosed: close a view from invokeWSDTest
        WaitDestroy,
        WaitViewGone, ///< ViewClosed: the held result is released once wsd dropped view 0
        Done
    };

    /// HTTP timeout for the service; a broker kept alive by its POST would live this long.
    static constexpr int kTimeoutSecs = 10;
    /// CloseMidFlight passes when the broker is destroyed well inside kTimeoutSecs.
    static constexpr std::chrono::seconds kPromptTeardown{ 5 };
    /// TooLarge: below any request. (The kit reads its own coolwsd.xml, so here wsd refuses.)
    static constexpr int kSmallPayloadCap = 16;
    /// ResponseTooLarge: far above its ~60-byte requests (checked per POST); replies pad
    /// past it.
    static constexpr int kSmallReplyCap = 4096;
    static constexpr int kDefaultPayloadCap = 16 * 1024 * 1024; ///< matches coolwsd.xml
    static constexpr std::size_t kInFlight = 8; ///< wsd's MAX_PYTHON_COMPUTE_IN_FLIGHT
    static constexpr std::size_t kQueueTotal = kInFlight + 4; ///< QueueFifo: 4 wait

    const Scenario _scenario;
    std::atomic<Phase> _phase{ Phase::Load };
    std::chrono::steady_clock::time_point _closedAt;

    // Cells to read back, in order, and the view that reads them.
    std::vector<std::pair<std::string, std::string>> _expect; ///< cell -> text
    std::size_t _checked = 0;
    std::atomic<bool> _querying{ false };
    std::atomic<bool> _viewGone{ false };
    int _readView = 0;

    // The service side. Sockets are only touched on websrv_poll, where requests arrive.
    std::vector<std::pair<std::shared_ptr<StreamSocket>, std::string>> _held; ///< with code
    bool _releaseAll = false; ///< answer new POSTs at once
    std::size_t _toRelease = 0; ///< POSTs still to answer, held or not yet arrived
    std::atomic<std::size_t> _postCount{ 0 };
    std::atomic<std::size_t> _answered{ 0 };
    int _lastQueued = 0; ///< QueueFifo: the last waiting request POSTed
    /// ServiceErrors: the error texts the kit got as transient errors. Static: the kit's socket
    /// keeps the instance that was running when it connected, so another may see the frame.
    static inline std::mutex TransientErrorsMutex;
    static inline std::set<std::string> TransientErrors;
    static constexpr std::string_view k500Body = R"({"status":"ok","result":5})";
    static constexpr std::string_view k503Text = "Service \"down\": \\ caf\xC3\xA9\n";

    static const char* name(Scenario e)
    {
        switch (e)
        {
            case Scenario::Load: return "UnitPythonComputeLoad";
            case Scenario::Disabled: return "UnitPythonComputeDisabled";
            case Scenario::TooLarge: return "UnitPythonComputeTooLarge";
            case Scenario::CloseMidFlight: return "UnitPythonComputeCloseMidFlight";
            case Scenario::QueueFifo: return "UnitPythonComputeQueueFifo";
            case Scenario::ResponseTooLarge: return "UnitPythonComputeResponseTooLarge";
            case Scenario::ViewClosed: return "UnitPythonComputeViewClosed";
            case Scenario::ServiceErrors: return "UnitPythonComputeServiceErrors";
            case Scenario::Refused: return "UnitPythonComputeRefused";
        }
        return "UnitPythonCompute";
    }

    bool enabled() const { return _scenario != Scenario::Disabled; }

    static std::string cellName(std::size_t row) { return "A" + std::to_string(row); }

    void goToCell(int view, const std::string& cell)
    {
        sendCommand(view, R"(uno .uno:GoToCell {"ToPoint":{"type":"string","value":")" + cell
                              + "\"}}");
    }

    /// Types =PY("code") into \p cell, which then reads back as \p expected.
    void enterPy(int view, const std::string& cell, const std::string& code,
                 const std::string& expected)
    {
        goToCell(view, cell);
        sendCommand(view, "uno .uno:EnterString "
                          "{\"StringName\":{\"type\":\"string\",\"value\":\"=PY(\\\""
                              + code + "\\\")\"}}");
        _expect.emplace_back(cell, expected);
    }

    /// Calc interprets a typed cell lazily (on paint or read), and the test paints nothing:
    /// F9 interprets all of them now. Unchanged cells hit the AddIn's cache, so no new requests.
    void recalc(int view) { sendCommand(view, "uno .uno:Calculate"); }

    void finish(const std::string& reason)
    {
        _phase = Phase::Done;
        passTest(reason);
    }

    void answer(const std::shared_ptr<StreamSocket>& socket, const std::string& code)
    {
        ++_answered;
        if (code == "http500")
        {
            // A valid result body: only the status check keeps it out of the cell.
            http::Response response(http::StatusCode::InternalServerError);
            response.setBody(std::string(k500Body), "application/json");
            socket->sendAndShutdown(response);
            return;
        }
        if (code == "http503deep")
        {
            // JSON wsd parses but nested past the AddIn's depth limit: forwarded as is, the
            // AddIn would cache it for the document's life.
            http::Response response(http::StatusCode::ServiceUnavailable);
            response.setBody(R"({"error":"busy","detail":)" + std::string(100, '[')
                                 + std::string(100, ']') + '}',
                             "application/json");
            socket->sendAndShutdown(response);
            return;
        }
        if (code == "http503text" || code == "http503badutf8")
        {
            // Not JSON: valid UTF-8 goes on as the error text, else wsd's own text (a UTF-16
            // surrogate, which only a strict check rejects).
            http::Response response(http::StatusCode::ServiceUnavailable);
            response.setBody(code == "http503text" ? std::string(k503Text)
                                                   : std::string("\xED\xA0\x80 down"),
                             "text/plain");
            socket->sendAndShutdown(response);
            return;
        }
        if (code == "nonjson")
        {
            http::Response response(http::StatusCode::OK);
            response.setBody("not json", "text/plain");
            socket->sendAndShutdown(response);
            return;
        }
        if (code.starts_with("big-"))
            return answerTooLarge(socket, code);

        // "result=N" gives N.
        LOK_ASSERT(code.starts_with("result="));
        const std::string body = R"({"status":"ok","result":)" + code.substr(7) + '}';
        http::Response response(http::StatusCode::OK);
        response.setBody(body, "application/json; charset=utf-8");
        socket->sendAndShutdown(response);
    }

    /// Answers \p count POSTs (all, also later ones, if 0) on websrv_poll, the sockets'
    /// thread. POSTs not yet arrived are answered when they arrive.
    void releaseHeld(std::size_t count)
    {
        COOLWSD::getWebServerPoll()->addCallback(
            [this, count]()
            {
                _releaseAll = _releaseAll || count == 0;
                _toRelease += count;
                while (!_held.empty() && (_releaseAll || _toRelease > 0))
                {
                    auto [socket, code] = std::move(_held.front());
                    _held.erase(_held.begin());
                    answerHeldOrNew(socket, code);
                }
            });
    }

    /// Answers the POST if a release covers it; returns false to keep holding it.
    bool answerHeldOrNew(const std::shared_ptr<StreamSocket>& socket, const std::string& code)
    {
        if (!_releaseAll && _toRelease == 0)
            return false;
        if (!_releaseAll)
            --_toRelease;
        answer(socket, code);
        return true;
    }

    void answerTooLarge(const std::shared_ptr<StreamSocket>& socket, const std::string& code)
    {
        const std::string body
            = R"({"status":"ok","result":")" + std::string(kSmallReplyCap, 'x') + "\"}";
        if (code == "big-sized")
        {
            http::Response response(http::StatusCode::OK);
            response.setBody(body, "application/json");
            socket->sendAndShutdown(response);
            return;
        }
        // No Content-Length: the body ends when the connection closes.
        socket->send("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"
                     + body);
        socket->asyncShutdown();
    }

    std::size_t expectedPosts() const
    {
        switch (_scenario)
        {
            case Scenario::Disabled:
            case Scenario::TooLarge:
            case Scenario::Refused: return 0;
            case Scenario::Load:
            case Scenario::ViewClosed: return 1;
            case Scenario::ResponseTooLarge: return 2;
            case Scenario::ServiceErrors: return 6;
            case Scenario::QueueFifo: return kQueueTotal;
            case Scenario::CloseMidFlight: return kInFlight;
        }
        return 0;
    }

public:
    explicit UnitPythonCompute(Scenario scenario)
        : WopiTestServer(name(scenario), scenario == Scenario::Load ? "pythoncompute.ods"
                                                                    : "empty.ods")
        , _scenario(scenario)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("security.python_compute.enable", enabled());
        // ServiceErrors: the never-answering service times out quickly.
        config.setInt("security.python_compute.timeout_secs",
                      _scenario == Scenario::ServiceErrors ? 1 : kTimeoutSecs);
        // The configuration is shared by every instance in this binary, so set every
        // scenario-specific key each time; otherwise one instance's value leaks into the next.
        int cap = kDefaultPayloadCap;
        if (_scenario == Scenario::TooLarge)
            cap = kSmallPayloadCap;
        else if (_scenario == Scenario::ResponseTooLarge)
            cap = kSmallReplyCap;
        config.setInt("security.python_compute.max_payload_bytes", cap);
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uri(request.getURI());
        if (uri.getPath() != "/v1/execute")
            return WopiTestServer::handleHttpRequest(request, message, socket);

        LOK_ASSERT_MESSAGE("no POST may reach the service in this scenario",
                           expectedPosts() > 0);
        LOK_ASSERT_EQUAL_STR("POST", request.getMethod());
        ++_postCount;
        LOK_ASSERT_MESSAGE("at most 8 POSTs open at once",
                           _postCount - _answered <= kInFlight);

        std::ostringstream bodyStream;
        Poco::StreamCopier::copyStream(message, bodyStream);
        Poco::JSON::Object::Ptr requestObject;
        LOK_ASSERT(JsonUtil::parseJSON(bodyStream.str(), requestObject));
        LOK_ASSERT(requestObject);
        std::string requestId;
        std::string code;
        LOK_ASSERT(JsonUtil::findJSONValue(requestObject, "id", requestId));
        LOK_ASSERT(JsonUtil::findJSONValue(requestObject, "code", code));
        if (_scenario == Scenario::ResponseTooLarge)
            LOK_ASSERT_MESSAGE("the request must fit the reply cap",
                               bodyStream.str().size() < static_cast<std::size_t>(kSmallReplyCap));

        switch (_scenario)
        {
            case Scenario::CloseMidFlight:
                _held.emplace_back(socket, code); // never answered
                if (_postCount == kInFlight) // the 9th waits behind the 8
                    _phase = Phase::CloseView;
                break;
            case Scenario::QueueFifo:
                // One slot frees at a time, so the waiting requests go in the order they
                // came: ascending, as F9 interprets the column top down.
                if (_postCount > kInFlight)
                {
                    const int k = std::stoi(code.substr(7));
                    LOK_ASSERT(k > _lastQueued);
                    _lastQueued = k;
                }
                if (!answerHeldOrNew(socket, code))
                    _held.emplace_back(socket, code);
                if (_postCount == kQueueTotal)
                    releaseHeld(0);
                else if (_postCount >= kInFlight)
                    releaseHeld(1);
                break;
            case Scenario::ViewClosed:
                _held.emplace_back(socket, code); // answered once the view has closed
                _phase = Phase::CloseView;
                break;
            case Scenario::ServiceErrors:
                if (code == "hang")
                    _held.emplace_back(socket, code); // wsd times out after 1 s
                else
                    answer(socket, code);
                break;
            default:
                answer(socket, code);
                break;
        }
        return true;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Python compute document loaded: [" << message << ']');
        switch (_scenario)
        {
            case Scenario::Load:
                _expect.emplace_back("A1", "1");
                break;
            case Scenario::Disabled:
                enterPy(0, "A1", "result=1", "#DISABLED");
                break;
            case Scenario::TooLarge:
                enterPy(0, "A1", "result=1", "#VALUE!");
                break;
            case Scenario::Refused:
                enterPy(0, "A1", "result=1", "#VALUE!");
                break;
            case Scenario::CloseMidFlight:
                // Fill the 8 slots; the 9th waits.
                for (std::size_t i = 1; i <= kInFlight + 1; ++i)
                    enterPy(0, cellName(i), "result=" + std::to_string(i), "");
                recalc(0);
                return true; // no cells are read back
            case Scenario::QueueFifo:
                // 1..8 in flight, 9..12 wait.
                for (std::size_t i = 1; i <= kQueueTotal; ++i)
                    enterPy(0, cellName(i), "result=" + std::to_string(i), std::to_string(i));
                recalc(0);
                break;
            case Scenario::ResponseTooLarge:
                enterPy(0, "A1", "big-sized", "#VALUE!");
                enterPy(0, "A2", "big-unsized", "#VALUE!");
                recalc(0);
                break;
            case Scenario::ServiceErrors:
                enterPy(0, "A1", "hang", "#VALUE!");
                enterPy(0, "A2", "http500", "#VALUE!");
                enterPy(0, "A3", "nonjson", "#VALUE!");
                enterPy(0, "A4", "http503deep", "#VALUE!");
                enterPy(0, "A5", "http503text", "#VALUE!");
                enterPy(0, "A6", "http503badutf8", "#VALUE!");
                recalc(0);
                break;
            case Scenario::ViewClosed:
                addWebSocket(); // view 1 reads the result after view 0 closes
                sendCommand(1, "load url=" + getWopiSrc());
                return true; // typed in onViewLoaded
        }
        _phase = Phase::Check;
        return true;
    }

    bool onViewLoaded(const std::string& message) override
    {
        if (_scenario != Scenario::ViewClosed || message.find("views=2") == std::string::npos)
            return false;
        // Any live view carries the request; view 0 closes while the POST is held and
        // view 1 reads the result.
        enterPy(0, "A1", "result=7", "7");
        recalc(0);
        _readView = 1;
        return true;
    }

    bool onFilterSendWebSocketMessage(std::string_view data, WSOpCode, bool, int&) override
    {
        // ServiceErrors: a non-200 reaches the kit as wsd's own transient error, not the body.
        constexpr std::string_view resultPrefix = "pythoncomputeresult:";
        if (data.starts_with(resultPrefix))
        {
            Poco::JSON::Object::Ptr obj;
            if (JsonUtil::parseJSON(data.substr(resultPrefix.size()), obj) && obj
                && obj->optValue<std::string>("status", "") == "error"
                && obj->optValue<bool>("transient", false) && !obj->has("detail"))
            {
                std::lock_guard<std::mutex> lock(TransientErrorsMutex);
                TransientErrors.insert(obj->optValue<std::string>("error", ""));
            }
            return false;
        }

        constexpr std::string_view prefix = "textselectioncontent:";
        // In a multi-test run the socket filter can reach an instance that already finished.
        if (!data.starts_with(prefix) || _phase != Phase::Check || !_querying)
            return false;

        const std::string text(Util::trimmed(data.substr(prefix.size())));
        const auto& [cell, expected] = _expect[_checked];
        TST_LOG("Cell " << cell << " is [" << text << ']');
        if (text != "#BUSY!") // still waiting for its result: read it again
        {
            LOK_ASSERT_EQUAL_STR(expected, text);
            ++_checked;
        }
        _querying = false;
        return false;
    }

    void onDocBrokerRemoveSession(const std::string&,
                                  const std::shared_ptr<ClientSession>&) override
    {
        if (_scenario == Scenario::ViewClosed && _phase == Phase::WaitViewGone)
            _viewGone = true;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        if (_scenario != Scenario::CloseMidFlight || _phase != Phase::WaitDestroy)
            return;
        const auto elapsed = std::chrono::steady_clock::now() - _closedAt;
        TST_LOG("DocBroker [" << docKey << "] destroyed "
                              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
                              << " after the view closed");
        // A broker kept alive by its open POSTs would live until the HTTP timeout.
        LOK_ASSERT(elapsed < kPromptTeardown);
        // The queued request was dropped, not POSTed when the slots freed; no reply is owed,
        // the kit goes with the document.
        LOK_ASSERT_EQUAL(kInFlight, _postCount.load());
        finish("Python compute POSTs cancelled and queue dropped on document close");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
                break;
            case Phase::CloseView:
                if (_scenario == Scenario::CloseMidFlight)
                {
                    // Set before the close: onDocBrokerDestroy reads both.
                    _closedAt = std::chrono::steady_clock::now();
                    _phase = Phase::WaitDestroy; // the only view: the document closes
                }
                else
                    _phase = Phase::WaitViewGone;
                deleteSocketAt(0);
                return;
            case Phase::WaitViewGone:
                if (_viewGone)
                {
                    releaseHeld(0);
                    _phase = Phase::Check;
                }
                return;
            case Phase::Check:
                if (_querying)
                    return;
                if (_checked == _expect.size())
                {
                    LOK_ASSERT_EQUAL(expectedPosts(), _postCount.load());
                    if (_scenario == Scenario::ServiceErrors)
                    {
                        std::lock_guard<std::mutex> lock(TransientErrorsMutex);
                        // The "error" string; else the body, if valid UTF-8; else wsd's text.
                        for (const std::string_view error :
                             { std::string_view("busy"), k500Body, k503Text,
                               std::string_view("Python compute HTTP 503") })
                            LOK_ASSERT_MESSAGE(std::string(error),
                                               TransientErrors.count(std::string(error)) == 1);
                    }
                    finish(std::string(name(_scenario)) + ": cells as expected");
                    return;
                }
                _querying = true;
                goToCell(_readView, _expect[_checked].first);
                sendCommand(_readView, "gettextselection mimetype=text/plain;charset=utf-8");
                return;
            default:
                return;
        }

        // ClientPortNumber is assigned after configure(), so set the loopback
        // endpoint only once the WSD listener has its final port.
        auto& config = Poco::Util::Application::instance().config();
        config.setBool("security.python_compute.enable", enabled());
        // Refused: port 1 on loopback has no listener.
        config.setString("security.python_compute.url",
                         _scenario == Scenario::Refused
                             ? std::string("http://127.0.0.1:1/v1/execute")
                             : helpers::getTestServerURI() + "/v1/execute");
        _phase = Phase::WaitLoad;
        initWebsocket("/wopi/files/0?access_token=anything");
        sendCommand(0, "load url=" + getWopiSrc());
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    using S = UnitPythonCompute::Scenario;
    return new UnitBase*[10]{ new UnitPythonCompute(S::Load),
                              new UnitPythonCompute(S::Disabled),
                              new UnitPythonCompute(S::TooLarge),
                              new UnitPythonCompute(S::CloseMidFlight),
                              new UnitPythonCompute(S::QueueFifo),
                              new UnitPythonCompute(S::ResponseTooLarge),
                              new UnitPythonCompute(S::ViewClosed),
                              new UnitPythonCompute(S::ServiceErrors),
                              new UnitPythonCompute(S::Refused),
                              nullptr };
}

#else // !ENABLE_PYTHON_COMPUTE

/// The end-to-end test needs --enable-python-compute.
class UnitPythonComputeSkipped final : public UnitWSD
{
public:
    UnitPythonComputeSkipped()
        : UnitWSD("UnitPythonComputeSkipped")
    {
    }

    void invokeWSDTest() override
    {
        passTest("Python compute not built (needs --enable-python-compute)");
    }
};

UnitBase* unit_create_wsd(void) { return new UnitPythonComputeSkipped(); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
