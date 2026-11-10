/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <jni.h>
#include <android/log.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include <DocumentBroker.hpp>
#include <FakeSocket.hpp>
#include <Kit.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <COOLWSD.hpp>
#include <Protocol.hpp>
#include <SetupKitEnvironment.hpp>
#include <common/Util.hpp>

#include <COKit/COKit.hxx>

#include <osl/detail/android-bootstrap.h>

#include <androidapp.hpp>

const int SHOW_JS_MAXLEN = 70;

int coolwsd_server_socket_fd = -1;

const char* user_name;

static std::atomic<unsigned> appDocIdCounter(1);
static JavaVM* javaVM = nullptr;
static bool lokInitialized = false;

/// The app side of one open document, the counterpart of CODocument in the iOS app.
struct AndroidDocument
{
    enum class State
    {
        /// The document has its number, and the page has not said HULLO yet.
        Opened,
        /// The fake socket is connected to the server and the forwarding thread runs.
        Connected,
        /// The close pipe is closed. The server saves and unloads in the background.
        Closed
    };

    /// Global reference.
    jobject activity = nullptr;
    std::string fileURL;
    State state = State::Opened;
    int fakeClientFd = -1;
    /// Our end of the pipe that stops the forwarding thread.
    int closeNotificationFd = -1;
    std::chrono::steady_clock::time_point closedAt;
};

/// A closed document is swept only after this long, by when the server has created its broker
/// if it ever will.
constexpr std::chrono::seconds ClosedDocumentGracePeriod(5);

// The open documents by document number. The save that follows a close is reported to the
// activity after the page has let go of the document, so an entry stays until the kit has
// dropped the document.
static std::mutex documentsMutex;
static std::map<unsigned, AndroidDocument> documents;

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void*) {
    javaVM = vm;
    cokit_set_javavm(vm);

    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }


    // Uncomment the following to see the logs from the core too
    //setenv("SAL_LOG", "+WARN+INFO", 0);
#if ENABLE_DEBUG
    Log::initialize("Mobile", "debug");
#else
    Log::initialize("Mobile", "information");
#endif

    resolveSystemClipboardClass(env);

    return JNI_VERSION_1_6;
}

// declared in androidapp.hpp
JNIThreadContext::JNIThreadContext(jint localCapacity)
{
    assert(javaVM != nullptr);
    const jint res = javaVM->GetEnv(reinterpret_cast<void**>(&_env), JNI_VERSION_1_6);
    if (res == JNI_EDETACHED)
    {
        LOG_DBG("Attach worker thread");
        if (javaVM->AttachCurrentThread(&_env, nullptr) != JNI_OK)
        {
            LOG_ERR("Failed to AttachCurrentThread");
            _env = nullptr;
            return;
        }
        _attachedHere = true;
    }
    else if (res != JNI_OK)
    {
        LOG_ERR("GetEnv failed with " << res);
        _env = nullptr;
        return;
    }

    _framePushed = _env->PushLocalFrame(localCapacity) == 0;
    if (!_framePushed)
    {
        _env->ExceptionClear();
        LOG_ERR("Cannot push a JNI local frame");
    }
}

JNIThreadContext::~JNIThreadContext()
{
    if (_env && _framePushed)
        _env->PopLocalFrame(nullptr);
    if (_attachedHere)
        javaVM->DetachCurrentThread();
}

static void forgetDocument(JNIEnv* env, unsigned documentId)
{
    jobject activity = nullptr;
    {
        const std::lock_guard<std::mutex> lock(documentsMutex);
        const auto it = documents.find(documentId);
        if (it != documents.end())
        {
            activity = it->second.activity;
            documents.erase(it);
        }
    }
    if (activity)
        env->DeleteGlobalRef(activity);
}

/// Let go of the closed documents that never reached the engine, since documentClosed() never
/// reports those.
static void forgetDocumentsWithoutEngine(JNIEnv* env)
{
    std::vector<unsigned> remembered;
    {
        const auto closedBefore = std::chrono::steady_clock::now() - ClosedDocumentGracePeriod;
        const std::lock_guard<std::mutex> lock(documentsMutex);
        for (const auto& entry : documents)
            if (entry.second.state == AndroidDocument::State::Closed
                && entry.second.closedAt < closedBefore)
                remembered.push_back(entry.first);
    }

    for (const unsigned id : remembered)
    {
        const std::optional<DocumentData> data = DocumentData::getCopy(id);
        if (data && data->loKitDocument)
            continue;
        if (findBrokerByMobileAppDocId(id))
            continue;

        DocumentData::deallocate(id);
        forgetDocument(env, id);
    }
}

static unsigned openDocument(JNIEnv* env, jobject activity, const std::string& fileURL)
{
    const unsigned documentId = appDocIdCounter++;
    DocumentData::allocate(documentId);

    const std::lock_guard<std::mutex> lock(documentsMutex);
    AndroidDocument& document = documents[documentId];
    document.activity = env->NewGlobalRef(activity);
    document.fileURL = fileURL;
    return documentId;
}

/// A new local reference, null when the document is gone.
static jobject documentActivity(JNIEnv* env, unsigned documentId)
{
    const std::lock_guard<std::mutex> lock(documentsMutex);
    const auto it = documents.find(documentId);
    if (it == documents.end())
        return nullptr;
    return env->NewLocalRef(it->second.activity);
}

static jmethodID fakeWebsocketMethod(JNIEnv* env, jobject activity)
{
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(activityClass, "rawCallFakeWebsocketOnMessage", "([B)V");
    env->DeleteLocalRef(activityClass);
    return method;
}

static void send2JS(const JNIThreadContext &jctx, jobject activity, jmethodID callFakeWebsocket,
                    const std::vector<char>& buffer)
{
    LOG_DBG("Send to JS: " << COOLProtocol::getAbbreviatedMessage(buffer.data(), buffer.size()));

    JNIEnv *env = jctx.getEnv();
    if (!env)
        return;

    jbyteArray jmessage = env->NewByteArray(buffer.size());
    env->SetByteArrayRegion(jmessage, 0, buffer.size(),
                            reinterpret_cast<const jbyte *>(buffer.data()));

    env->CallVoidMethod(activity, callFakeWebsocket, jmessage);
    env->DeleteLocalRef(jmessage);

    if (env->ExceptionCheck())
        env->ExceptionDescribe();
}

void postDirectMessage(unsigned documentId, const std::string& message)
{
    JNIThreadContext ctx;
    JNIEnv *env = ctx.getEnv();
    if (!env)
        return;

    jobject activity = documentActivity(env, documentId);
    if (!activity)
    {
        LOG_WRN("No activity for document " << documentId << ", dropping: " << message);
        return;
    }

    jstring jstr = env->NewStringUTF(message.c_str());
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID callPostNativeMessage =
        env->GetMethodID(activityClass, "postNativeMessage", "(Ljava/lang/String;)V");
    env->CallVoidMethod(activity, callPostNativeMessage, jstr);
    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);

    if (env->ExceptionCheck())
        env->ExceptionDescribe();
}

static COKitDocument* engineDocument(unsigned documentId)
{
    const std::optional<DocumentData> data = DocumentData::getCopy(documentId);
    return data ? data->loKitDocument : nullptr;
}

/// Returns true when the engine holds a loaded document, which documentClosed() reports once the
/// engine has dropped it.
static bool closeDocument(JNIEnv* env, unsigned documentId)
{
    int notificationFd = -1;
    bool neverConnected = false;
    {
        const std::lock_guard<std::mutex> lock(documentsMutex);
        const auto it = documents.find(documentId);
        if (it == documents.end())
            return false;
        AndroidDocument& document = it->second;
        if (document.state == AndroidDocument::State::Closed)
            return false;
        neverConnected = document.state == AndroidDocument::State::Opened;
        document.state = AndroidDocument::State::Closed;
        document.closedAt = std::chrono::steady_clock::now();
        std::swap(notificationFd, document.closeNotificationFd);
    }
    if (neverConnected)
    {
        DocumentData::deallocate(documentId);
        forgetDocument(env, documentId);
        return false;
    }

    const bool loaded = engineDocument(documentId) != nullptr;

    // Close one end of the socket pair, that wakes up the forwarding thread of the document
    fakeSocketClose(notificationFd);
    return loaded;
}

void documentClosed(unsigned documentId)
{
    JNIThreadContext ctx;
    postDirectMessage(documentId, "CLOSED " + std::to_string(documentId));
    forgetDocument(ctx.getEnv(), documentId);
}

/// Wait, for a bounded time, until the broker of the document with this number has ended. The
/// broker is keyed by the path of the document, so a document opened again from the same path
/// would otherwise join the one on its way out.
static void waitForDocumentBroker(unsigned documentId)
{
    for (int i = 0; i < 300; ++i)
    {
        const std::shared_ptr<DocumentBroker> broker = findBrokerByMobileAppDocId(documentId);
        if (!broker || !broker->isAlive())
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    LOG_WRN("The broker of document " << documentId << " is still alive");
}

static int documentSocket(unsigned documentId)
{
    const std::lock_guard<std::mutex> lock(documentsMutex);
    const auto it = documents.find(documentId);
    if (it == documents.end() || it->second.state != AndroidDocument::State::Connected)
        return -1;
    return it->second.fakeClientFd;
}

static void connectDocument(JNIEnv* env, unsigned documentId)
{
    std::string fileURL;
    // The thread below delivers to the activity the document was opened in.
    jobject threadActivity = nullptr;
    const int currentFakeClientFd = fakeSocketSocket();
    int notificationPipe[2];
    fakeSocketPipe2(notificationPipe);
    {
        const std::lock_guard<std::mutex> lock(documentsMutex);
        const auto it = documents.find(documentId);
        if (it == documents.end() || it->second.state != AndroidDocument::State::Opened)
        {
            LOG_WRN("No open document " << documentId << " to connect");
            fakeSocketClose(currentFakeClientFd);
            fakeSocketClose(notificationPipe[0]);
            fakeSocketClose(notificationPipe[1]);
            return;
        }
        AndroidDocument& document = it->second;
        document.state = AndroidDocument::State::Connected;
        document.fakeClientFd = currentFakeClientFd;
        document.closeNotificationFd = notificationPipe[0];
        fileURL = document.fileURL;
        threadActivity = env->NewGlobalRef(document.activity);
    }
    const int threadNotificationFd = notificationPipe[1];

    // Contact the permanently (during app lifetime) listening COOLWSD server "public" socket
    assert(coolwsd_server_socket_fd != -1);
    int rc = fakeSocketConnect(currentFakeClientFd, coolwsd_server_socket_fd);
    assert(rc != -1);

    // Start another thread to read responses and forward them to the JavaScript
    std::thread([currentFakeClientFd, threadNotificationFd, threadActivity]
                {
                    ProcUtil::setThreadName("app2js");
                    JNIThreadContext ctx;
                    const jmethodID callFakeWebsocket =
                        fakeWebsocketMethod(ctx.getEnv(), threadActivity);
                    while (true)
                    {
                       struct pollfd pollfd[2];
                       pollfd[0].fd = currentFakeClientFd;
                       pollfd[0].events = POLLIN;
                       pollfd[1].fd = threadNotificationFd;
                       pollfd[1].events = POLLIN;
                       if (fakeSocketPoll(pollfd, 2, -1) > 0)
                       {
                           if (pollfd[1].revents == POLLIN)
                           {
                               // The other end of the notification pipe has been
                               // closed: the document is closing.
                               LOG_DBG("app2js: closing the sockets");
                               break;
                           }
                           if (pollfd[0].revents == POLLIN)
                           {
                               int n = fakeSocketAvailableDataLength(currentFakeClientFd);
                               if (n == 0)
                               {
                                   // The server has closed its end.
                                   LOG_DBG("app2js: the server closed the connection");
                                   break;
                               }
                               std::vector<char> buf(n);
                               n = fakeSocketRead(currentFakeClientFd, buf.data(), n);
                               send2JS(ctx, threadActivity, callFakeWebsocket, buf);
                           }
                       }
                       else
                       {
                           assert(false);
                           break;
                       }
                   }

                   // Close this end of the notification pipe and our end of the fake
                   // socket connection to the ClientSession, so that the session
                   // terminates. A fake socket is not a system resource, but each open
                   // one keeps its slot in the fake socket table.
                   fakeSocketClose(threadNotificationFd);
                   fakeSocketClose(currentFakeClientFd);
                   ctx.getEnv()->DeleteGlobalRef(threadActivity);
                }).detach();

    // First we simply send it the URL and the document number. This corresponds to the
    // GET request with Upgrade to WebSocket.
    const std::string handshake(fileURL + " " + std::to_string(documentId));
    LOG_DBG("Actually sending to Online:" << handshake);

    // Send the document URL to COOLWSD to setup the docBroker URL
    fakeSocketWriteQueue(currentFakeClientFd, handshake.c_str(), handshake.size());
}

extern "C" JNIEXPORT jint JNICALL
Java_org_libreoffice_androidlib_LOActivity_openDocument(JNIEnv* env, jobject instance, jstring loadFileURL)
{
    const char* url = env->GetStringUTFChars(loadFileURL, nullptr);
    const unsigned documentId = openDocument(env, instance, url);
    env->ReleaseStringUTFChars(loadFileURL, url);
    LOG_DBG("Opened document " << documentId);
    return documentId;
}

/// A page that starts again without a BYE still has its old document connected.
extern "C" JNIEXPORT jint JNICALL
Java_org_libreoffice_androidlib_LOActivity_connectDocument(JNIEnv* env, jobject instance, jint documentId)
{
    unsigned connectedId = documentId;
    std::string fileURL;
    bool connected = false;
    {
        const std::lock_guard<std::mutex> lock(documentsMutex);
        const auto it = documents.find(documentId);
        if (it != documents.end())
        {
            connected = it->second.state == AndroidDocument::State::Connected;
            fileURL = it->second.fileURL;
        }
    }
    if (connected)
    {
        closeDocument(env, documentId);
        waitForDocumentBroker(documentId);
        connectedId = openDocument(env, instance, fileURL);
    }
    forgetDocumentsWithoutEngine(env);
    connectDocument(env, connectedId);
    return connectedId;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_libreoffice_androidlib_LOActivity_closeDocumentNative(JNIEnv* env, jobject,
                                                                jint documentId)
{
    LOG_DBG("Closing document " << documentId);
    return closeDocument(env, documentId) ? JNI_TRUE : JNI_FALSE;
}

/// Handle a message from JavaScript.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_postMobileMessageNative(JNIEnv* env, jobject,
                                                                    jint documentId,
                                                                    jstring message)
{
    const char *string_value = env->GetStringUTFChars(message, nullptr);

    if (string_value)
    {
        LOG_DBG("From JS: cool: " << string_value);

        const int fakeClientFd = documentSocket(documentId);
        if (fakeClientFd == -1)
            LOG_WRN("Document " << documentId << " is not connected, dropping: " << string_value);
        else
            fakeSocketWriteQueue(fakeClientFd, string_value, strlen(string_value));

        env->ReleaseStringUTFChars(message, string_value);
    }
    else
        LOG_DBG("From JS: cool: some object");
}

extern "C" jboolean cokit_initialize(JNIEnv* env, jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager);

/// Start the engine and the COOLWSD instance. Both live as long as the process, so the second
/// and later calls do nothing.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_initializeEngine(JNIEnv* env, jobject, jstring dataDir,
                                                            jstring cacheDir, jstring apkFile,
                                                            jobject assetManager, jstring uiMode,
                                                            jstring userName)
{
    if (lokInitialized)
        return;
    const std::string userInterfaceMode = std::string(env->GetStringUTFChars(uiMode, nullptr));
    setupKitEnvironment(userInterfaceMode);
    static const std::string userNameString = std::string(env->GetStringUTFChars(userName, nullptr));
    user_name = userNameString.c_str();
    lokInitialized = true;
    cokit_initialize(env, dataDir, cacheDir, apkFile, assetManager);

    ProcUtil::setThreadName("main");

    fakeSocketSetLoggingCallback([](const std::string& line)
                                 {
                                     LOG_DBG(line);
                                 });

    std::thread([]
                {
                    char *argv[2];
                    argv[0] = strdup("mobile");
                    argv[1] = nullptr;
                    ProcUtil::setThreadName("app");
                    // One COOLWSD serves every document for the whole life of the app.
                    LOG_DBG("Creating COOLWSD");
                    auto coolwsd = std::make_unique<COOLWSD>();
                    coolwsd->run(1, argv);

                    LOG_FTL("COOLWSD::run() returned");
                    std::abort();
                }).detach();
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_saveAsNative(JNIEnv *env, jobject, jint documentId,
                                                        jstring fileUri_, jstring format_,
                                                        jstring options_) {
    const char *fileUri = env->GetStringUTFChars(fileUri_, 0);
    const char *format = env->GetStringUTFChars(format_, 0);
    const char *options = nullptr;
    if (options_ != nullptr)
        options = env->GetStringUTFChars(options_, 0);

    if (COKitDocument* document = engineDocument(documentId))
        document->saveAs(fileUri, format, options);

    env->ReleaseStringUTFChars(fileUri_, fileUri);
    env->ReleaseStringUTFChars(format_, format);
    if (options_ != nullptr)
        env->ReleaseStringUTFChars(options_, options);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_postUnoCommandNative(JNIEnv* pEnv, jobject, jint documentId,
                                                                jstring command, jstring arguments, jboolean bNotifyWhenFinished)
{
    const char* pCommand = pEnv->GetStringUTFChars(command, nullptr);
    const char* pArguments = nullptr;
    if (arguments != nullptr)
        pArguments = pEnv->GetStringUTFChars(arguments, nullptr);

    if (COKitDocument* document = engineDocument(documentId))
        document->postUnoCommand(pCommand, pArguments, bNotifyWhenFinished);

    pEnv->ReleaseStringUTFChars(command, pCommand);
    if (arguments != nullptr)
        pEnv->ReleaseStringUTFChars(arguments, pArguments);
}

static jstring tojstringAndFree(JNIEnv *env, char *str)
{
    if (!str)
        return env->NewStringUTF("");
    jstring ret = env->NewStringUTF(str);
    free(str);
    return ret;
}

const char* copyJavaString(JNIEnv* pEnv, jstring aJavaString)
{
    const char* pTemp = pEnv->GetStringUTFChars(aJavaString, nullptr);
    const char* pClone = strdup(pTemp);
    pEnv->ReleaseStringUTFChars(aJavaString, pTemp);
    return pClone;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_libreoffice_androidlib_COWebViewClient_getEmbeddedMediaPath(JNIEnv *env, jobject, jint documentId, jstring inTag) {
    std::string tag = copyJavaString(env, inTag);
    std::string mediaPath;
    if (const std::optional<DocumentData> data = DocumentData::getCopy(documentId))
    {
        if (std::shared_ptr<DocumentBroker> docBroker = data->docBroker.lock())
            mediaPath = docBroker->getEmbeddedMediaPath(tag);
    }
    return env->NewStringUTF(mediaPath.c_str());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
