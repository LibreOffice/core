/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <common/Session.hpp>
#include <common/StateEnum.hpp>
#include <common/ThreadPool.hpp>
#include <common/Util.hpp>
#include <kit/KitQueue.hpp>
#include <kit/LogUI.hpp>
#include <net/Socket.hpp>
#include <wsd/TileDesc.hpp>

#include <COKit/COKitTypes.h>

#include <Poco/Util/XMLConfiguration.h>

#include <map>
#include <string>

#if MOBILEAPP

#include <future>

#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

#endif

#ifdef IOS
void runKitLoopInAThread();
#endif

namespace kit
{
class Document;
class Office;
}
struct COKitStruct;
using COKit = COKitStruct;

void lokit_main(
#if !MOBILEAPP
    const std::string& childRoot, const std::string& jailId, const std::string& configId,
    const std::string& sysTemplate, const std::string& loTemplate, bool noCapabilities,
    bool noSeccomp, bool useMountNamespaces, bool queryVersionInfo, bool displayVersion,
    bool sysTemplateIncomplete,
#else
    int docBrokerSocket, const std::string& userInterface,
#endif
    std::size_t numericIdentifier);

bool globalPreinit(const std::string& loTemplate);
/// Wrapper around private Document::ViewCallback().
void documentViewCallback(int type, const char* p, void* data);

class Document;
class DeltaGenerator;

/// Descriptor class used to link a COKit
/// callback to a specific view.
struct CallbackDescriptor
{
    CallbackDescriptor(Document* const doc, const int viewId)
        : _doc(doc), _viewId(viewId)
    {
    }
    Document* getDoc() const { return _doc; }
    int getViewId() const { return _viewId; }

private:
    Document* const _doc;
    const int _viewId;
};

/// User Info container used to store user information
/// till the end of process lifecycle - including
/// after any child session goes away
struct UserInfo
{
    UserInfo()
        : _readOnly(false)
        , _connected(false)
    {
    }

    UserInfo(const std::string& userId, const std::string& userName,
             const std::string& userExtraInfo, const std::string& userPrivateInfo, bool readOnly)
        : _userId(userId)
        , _userName(userName)
        , _userExtraInfo(userExtraInfo)
        , _userPrivateInfo(userPrivateInfo)
        , _readOnly(readOnly)
        , _connected(true)
    {
    }

    const std::string& getUserId() const { return _userId; }

    const std::string& getUserName() const { return _userName; }

    const std::string& getUserExtraInfo() const { return _userExtraInfo; }

    const std::string& getUserPrivateInfo() const { return _userPrivateInfo; }

    bool isReadOnly() const { return _readOnly; }

    bool isConnected() const { return _connected; }

    void setDisconnected() { _connected = false; }

private:
    std::string _userId;
    std::string _userName;
    std::string _userExtraInfo;
    std::string _userPrivateInfo;
    bool _readOnly;
    bool _connected;
};

/// We have two types of password protected documents
/// 1) Documents which require password to view
/// 2) Document which require password to modify
enum class DocumentPasswordType : std::uint8_t
{
    ToView,
    ToModify
};

/// Check the ForkCounter, and if non-zero, fork more of them accordingly.
void forkCOKit(const std::string& childRoot, const std::string& sysTemplate,
                        const std::string& loTemplate, bool useMountNamespaces);

class Document;

/// The main main-loop of the Kit process
class KitSocketPoll final : public SocketPoll
{
    std::chrono::steady_clock::time_point _pollEnd;
    std::shared_ptr<Document> _document;

    static KitSocketPoll* mainPoll;

    KitSocketPoll();

public:
    ~KitSocketPoll();
    void drainQueue();

    static void dumpGlobalState(std::ostream& oss);
    static std::shared_ptr<KitSocketPoll> create();

    static void cleanupChildProcess();

    void wakeupHook() override;

    static KitSocketPoll* getMainPoll() { return mainPoll; }

#if ENABLE_DEBUG
    struct ReEntrancyGuard
    {
        std::atomic<int>& _count;
        ReEntrancyGuard(std::atomic<int>& count)
            : _count(count)
        {
            count++;
        }
        ~ReEntrancyGuard() { _count--; }
    };
#endif

    /// Handle the poll from the unipoll callback.
    int kitPoll(int timeoutMicroS);

    /// Handle the wake up from the unipoll callback.
    void kitWakeup();

    /// Handle the 'has any input?' unipoll callback.
    bool kitHasAnyInput(int mostUrgentPriority);

    void setDocument(std::shared_ptr<Document> document) { _document = std::move(document); }
    const std::shared_ptr<Document>& getDocument() const { return _document; }

    // unusual COKit event from another thread, push into our loop to process.
    static bool pushToMainThread(COKitCallback callback, int type, const char* p,
                                 void* data);

#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    static std::mutex KSPollsMutex;
    static std::condition_variable KSPollsCV;
    static std::vector<std::weak_ptr<KitSocketPoll>> KSPolls;

    struct TerminationData
    {
        std::mutex mutex;
        std::condition_variable cv;
        bool flag;
    };
    std::shared_ptr<TerminationData> termination;
#endif
};

class ChildSession;

/// A document container.
/// Owns LOKitDocument instance and connections.
/// Manages the lifetime of a document.
/// Technically, we can host multiple documents
/// per process. But for security reasons don't.
/// However, we could have a coolkit instance
/// per user or group of users (a trusted circle).
class Document final : public std::enable_shared_from_this<Document>, private TilePrioritizer
{
public:
    Document(const std::shared_ptr<kit::Office>& loKit, const std::string& jailId,
             const std::string& docKey, const std::string& docId, const std::string& url,
             const std::shared_ptr<WebSocketHandler>& websocketHandler, unsigned mobileAppDocId);
    ~Document() final;

    const std::string& getUrl() const { return _url; }

    /// Post the message - in the unipoll world we're in the right thread anyway
    bool postMessage(const std::string_view data, WSOpCode code) const;

    bool createSession(const std::string& sessionId);

    /// Purges dead connections and returns
    /// the remaining number of clients.
    /// Returns -1 on failure.
    std::size_t purgeSessions();

    void setDocumentPassword(int passwordType);

    void renderTiles(TileCombined& tileCombined);

    bool sendTextFrame(const std::string_view message) const { return sendFrame(message); }

    bool sendFrame(std::string_view data, WSOpCode opCode = WSOpCode::Text) const;

    void alertNotAsync() const
    {
        // load unfortunately enables inputprocessing in some cases.
        if (processInputEnabled() && !_duringLoad && !isBackgroundSaveProcess())
            notifyAll("error: cmd=notasync kind=failure");
    }

    void alertAllUsers(const std::string& cmd, const std::string& kind) const
    {
        sendTextFrame("errortoall: cmd=" + cmd + " kind=" + kind);
    }

    /// Notify all views with the given message
    bool notifyAll(const std::string& msg) const
    {
        // Broadcast updated viewinfo to all clients.
        return sendTextFrame("client-all " + msg);
    }

    unsigned getMobileAppDocId() const { return _mobileAppDocId; }
    const std::string& getDocId() const { return _docId; }

    /// See if we should clear out our memory
    void trimIfInactive();
    void trimAfterInactivity();

    // COKit callback entry points
    static void GlobalCallback(int type, const char* p, void* data);
    static void ViewCallback(int type, const char* p, void* data);

private:
    /// Helper method to broadcast callback and its payload to all clients
    void broadcastCallbackToClients(const int type, const std::string& payload)
    {
        _queue->putCallback(-1, type, payload);
    }

    /// Cleanup bgSave child processes.
    static void reapZombieChildren();

    /// Calculate tile rendering priority from a TileDesc
    Priority getTilePriority(const TileDesc& desc) const override;
    std::vector<ViewIdInactivity> getViewIdsByInactivity() const override;

public:
    /// Request loading a document, or a new view, if one exists,
    /// and register callbacks.
    bool onLoad(const std::string& sessionId, const std::string& uriAnonym,
                const std::string& renderOpts);

    /// Unload a client session, which unloads the document
    /// if it is the last and only.
    void onUnload(const ChildSession& session);

    /// Get a view ID <-> UserInfo map.
    const std::map<int, UserInfo>& getViewInfo() const { return _sessionUserInfo; }

    int getEditorId() const { return _editorId; }

    bool isDocPasswordProtected() const { return _isDocPasswordProtected; }

    bool haveDocPassword() const { return _haveDocPassword; }

    const std::string& getDocPassword() const { return _docPassword; }

    DocumentPasswordType getDocPasswordType() const { return _docPasswordType; }

    void updateActivityHeader() const;

    /// Really important that if we drop, we re-start for the kit.
    class ThreadDropper final {
        Document *_doc;
    public:
        ThreadDropper() : _doc(nullptr) { }
        ~ThreadDropper()
        {
            if (_doc) _doc->startThreads();
        }
        void clear()
        {
            _doc = nullptr;
        }
        bool dropThreads(Document *doc)
        {
            if (doc->joinThreads())
            {
                // only this path starts later.
                _doc = doc;
                return true;
            }
            return false;
        }
        void startThreads()
        {
            if (_doc)
                _doc->startThreads();
            _doc = nullptr;
        }
    };

    bool joinThreads();
    void startThreads();

    bool forkToSave(const std::function<void()> &childSave, int viewId);

    void handleSaveMessage(const std::string &msg);

    /// Notify all views of viewId and their associated usernames
    void notifyViewInfo();

    std::shared_ptr<ChildSession> findSessionByViewId(int viewId);

    void invalidateCanonicalId(const std::string& sessionId);

    std::string getViewProps(const std::shared_ptr<ChildSession>& session);

    void updateEditorSpeeds(int id, int speed);

private:
    // Get the color value for all author names from the core
    std::map<std::string, int> getViewColors();

    std::string getDefaultTheme(const std::shared_ptr<ChildSession>& session) const;

    std::string getDefaultBackgroundTheme(const std::shared_ptr<ChildSession>& session) const;

    std::shared_ptr<kit::Document> load(const std::shared_ptr<ChildSession>& session,
                                        const std::string& renderOpts);

    bool forwardToChild(std::string_view prefix, const std::vector<char>& payload);

    static std::string makeRenderParams(const std::string& renderOpts, const std::string& userName,
                                        const std::string& spellOnline, const std::string& theme,
                                        const std::string& backgroundTheme,
                                        const std::string& userPrivateInfo);

    /// Returns true iff at least one session is loaded.
    bool haveLoadedSessions() const;

public:
    bool processInputEnabled() const;

    /// A new message from wsd for the queue
    void queueMessage(const std::string &msg) { _queue->put(msg); }
    /// Do we have incoming messages from wsd ?
    bool hasQueueItems() const { return _queue && !_queue->isEmpty(); }
    bool canRenderTiles() const {
        return processInputEnabled() && !isLoadOngoing() &&
            !isBackgroundSaveProcess() && _queue &&
            !_queue->isTileQueueEmpty();
    }
    bool hasCallbacks() const { return _queue && _queue->callbackSize() > 0; }

    /// Should we get through the SocketPoll fast to process queues ?
    bool needsQuickPoll() const
    {
        if (hasCallbacks())
            return true;
        // not processing input messages or tile renders
        if (!processInputEnabled())
            return false;
        if (hasQueueItems() || canRenderTiles())
            return true;
        return false;
    }

    void drainQueue();
    void drainCallbacks();

    void dumpState(std::ostream& oss);

    /// Returns true iff we have a LOKit Document instance.
    bool isLoaded() const { return !!_loKitDocument; }

    /// Return access to the kit::Office instance.
    std::shared_ptr<kit::Office> getLOKit() const { return _loKit; }

    /// Return access to the kit::Document instance.
    std::shared_ptr<kit::Document> getLOKitDocument();

    const std::string& getObfuscatedFileId() const { return _obfuscatedFileId; }

    bool isBackgroundSaveProcess() const { return _isBgSaveProcess; }

    static void shutdownBackgroundWatchdog();

    /// Save is async, so we need to set 'unmodified' while we are saving
    /// but this can transition back to modified if save fails.
    STATE_ENUM(ModifiedState, Modified, UnModifiedButSaving, UnModified);

    ModifiedState getModified() const { return _modified; }

    /// Lets us get notified whether a doc is modified during bgsave.
    void forceDocUnmodifiedForBgSave(int viewId);

    /// Restore the Document's 'modified' state if necessary
    void updateModifiedOnFailedBgSave();

    /// Let WSD know our true modified state after bg save success.
    void notifySyntheticUnmodifiedState();

    /// Snoop document modified, and return true if filtering notification
    bool trackDocModifiedState(const std::string &stateChanged);

    /// Permanently disable background save for this process
    void disableBgSave(const std::string &reason);

    void bgSaveStarted() { _bgSavesOngoing++; }
    void bgSaveEnded();

    /// Are we currently performing a load ?
    bool isLoadOngoing() const { return _duringLoad > 0; }

    LogUiCmd& getLogUiCmd() { return logUiCmd; }

    /// Get a thread-pool to perform an operation in
    /// all operations must complete by the time we
    /// return to the poll
    ThreadPool& getSyncPool() { return _deltaPool; }

    int getViewsCount() const;

private:
    void postForceModifiedCommand(bool modified);

    /// Stops theads, flushes buffers, and exits the process.
    void flushAndExit(int code);

private:
    std::shared_ptr<kit::Office> _loKit;
    const std::string _jailId;
    /// URL-based key. May be repeated during the lifetime of WSD.
    const std::string _docKey;
    /// Short numerical ID. Unique during the lifetime of WSD.
    const std::string _docId;
    const std::string _url;
    const std::string _obfuscatedFileId;
    std::string _jailedUrl;
    std::string _renderOpts;

    std::shared_ptr<kit::Document> _loKitDocument;
#ifdef __ANDROID__
    static std::shared_ptr<kit::Document> _loKitDocumentForAndroidOnly;
    static std::weak_ptr<DocumentBroker> _documentBrokerForAndroidOnly;
#endif
    std::unique_ptr<KitQueue> _queue;

    // Connection to the coolwsd process
    std::shared_ptr<WebSocketHandler> _websocketHandler;

    // Connection a child background save process has to its parent: a precious thing.
    std::weak_ptr<WebSocketHandler> _saveProcessParent;
    ModifiedState _modified;
    bool _isBgSaveProcess;
    bool _isBgSaveDisabled;
    bool _trimIfInactivePostponed;

    // Document password provided
    std::string _docPassword;
    // Whether password was provided or not
    bool _haveDocPassword;
    // Whether document is password protected
    bool _isDocPasswordProtected;
    // Whether password is required to view the document, or modify it
    DocumentPasswordType _docPasswordType;

    std::atomic<bool> _stop;

    ThreadPool _deltaPool;
    std::unique_ptr<DeltaGenerator> _deltaGen;

    int _editorId;
    bool _editorChangeWarning;
    std::map<int, std::unique_ptr<CallbackDescriptor>> _viewIdToCallbackDescr;
    SessionMap<ChildSession> _sessions;

    /// The timestamp of the last memory trimming.
    std::chrono::steady_clock::time_point _lastMemTrimTime;

    std::map<int, std::chrono::steady_clock::time_point> _lastUpdatedAt;
    std::map<int, int> _speedCount;
    /// For showing disconnected user info in the doc repair dialog.
    std::map<int, UserInfo> _sessionUserInfo;
#ifdef __ANDROID__
    friend std::shared_ptr<kit::Document> getLOKDocumentForAndroidOnly();
    friend std::shared_ptr<DocumentBroker> getDocumentBrokerForAndroidOnly();
#endif

    const unsigned _mobileAppDocId;
    int _duringLoad;
    int _bgSavesOngoing;

    LogUiCmd logUiCmd;
};

/// main function of the forkit process or thread
int forkit_main(int argc, char** argv);

/// Anonymize the basename of filenames, preserving the path and extension.
std::string anonymizeUrl(const std::string& url);

/// Anonymize usernames.
std::string anonymizeUsername(const std::string& username);

/// Ensure there is no fatal system setup problem
void consistencyCheckJail();

/// check how many theads we have currently
int getCurrentThreadCount();

/// Fetch the latest monotonically incrementing wire-id
TileWireId getCurrentWireId(bool increment = false);

#ifdef __ANDROID__
/// For the Android app, for now, we need access to the one and only document open to perform eg. saveAs() for printing.
std::shared_ptr<kit::Document> getLOKDocumentForAndroidOnly();
std::shared_ptr<DocumentBroker> getDocumentBrokerForAndroidOnly();
#endif

extern COKit* loKitPtr;

/// Check if URP is enabled
bool isURPEnabled();

extern Util::LoadTimings KitLoadTimings;

/// Start a URP connection, checking if URP is enabled and there is not already an active URP session
bool startURP(const std::shared_ptr<kit::Office>& LOKit, void** ppURPContext);

/// Ensure all recorded traces hit the disk
void flushTraceEventRecordings();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
