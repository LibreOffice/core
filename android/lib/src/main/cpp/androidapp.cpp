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

static std::string fileURL;
static std::atomic<int> fakeClientFd(-1);
static std::atomic<unsigned> appDocIdCounter(1);
static std::atomic<unsigned> appDocId(0);
static std::atomic<int> closeNotificationFd(-1);
static JavaVM* javaVM = nullptr;
static bool lokInitialized = false;

// The activity of each document the server may still report on, by document number. The save
// that follows a close is reported after the page has let go of the document, so an entry stays
// until the kit has dropped the document.
static std::mutex documentActivitiesMutex;
static std::map<unsigned, jobject> documentActivities;

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

static void forgetDocumentActivity(JNIEnv* env, unsigned documentId)
{
    jobject activity = nullptr;
    {
        const std::lock_guard<std::mutex> lock(documentActivitiesMutex);
        const auto it = documentActivities.find(documentId);
        if (it != documentActivities.end())
        {
            activity = it->second;
            documentActivities.erase(it);
        }
    }
    if (activity)
        env->DeleteGlobalRef(activity);
}

/// Let go of the documents below this number that never reached the engine, since
/// documentClosed() never reports those. The one just closed may still get its broker.
static void forgetDocumentsWithoutEngine(JNIEnv* env, unsigned belowDocumentId)
{
    std::vector<unsigned> remembered;
    {
        const std::lock_guard<std::mutex> lock(documentActivitiesMutex);
        for (const auto& entry : documentActivities)
            if (entry.first < belowDocumentId)
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
        forgetDocumentActivity(env, id);
    }
}

static void rememberDocumentActivity(JNIEnv* env, unsigned documentId, jobject activity)
{
    const std::lock_guard<std::mutex> lock(documentActivitiesMutex);
    documentActivities[documentId] = env->NewGlobalRef(activity);
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

    jobject activity = nullptr;
    {
        const std::lock_guard<std::mutex> lock(documentActivitiesMutex);
        const auto it = documentActivities.find(documentId);
        if (it != documentActivities.end())
            activity = env->NewLocalRef(it->second);
    }
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

static COKitDocument* currentDocument()
{
    const std::optional<DocumentData> data = DocumentData::getCopy(appDocId);
    return data ? data->loKitDocument : nullptr;
}

/// Returns true when the engine holds a loaded document, which documentClosed() reports once the
/// engine has dropped it.
static bool closeDocument()
{
    const int notificationFd = closeNotificationFd.exchange(-1);
    if (notificationFd == -1)
        return false;

    const bool loaded = currentDocument() != nullptr;

    // Close one end of the socket pair, that will wake up the forwarding thread that was constructed in HULLO
    fakeSocketClose(notificationFd);
    return loaded;
}

void documentClosed(unsigned documentId)
{
    JNIThreadContext ctx;
    postDirectMessage(documentId, "CLOSED");
    forgetDocumentActivity(ctx.getEnv(), documentId);
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

/// Handle a message from JavaScript.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_postMobileMessageNative(JNIEnv* env, jobject instance,
                                                                    jstring message)
{
    const char *string_value = env->GetStringUTFChars(message, nullptr);

    if (string_value)
    {
        LOG_DBG("From JS: cool: " << string_value);

        if (strcmp(string_value, "HULLO") == 0)
        {
            // Now we know that the JS has started completely

            // A page that starts again without a BYE still has its old document open.
            const unsigned previousDocId = appDocId;
            closeDocument();
            waitForDocumentBroker(previousDocId);
            forgetDocumentsWithoutEngine(env, previousDocId);

            appDocId = appDocIdCounter++;
            DocumentData::allocate(appDocId);
            rememberDocumentActivity(env, appDocId, instance);

            const int currentFakeClientFd = fakeSocketSocket();
            fakeClientFd = currentFakeClientFd;

            // Contact the permanently (during app lifetime) listening COOLWSD server
            // "public" socket
            assert(coolwsd_server_socket_fd != -1);

            int rc = fakeSocketConnect(currentFakeClientFd, coolwsd_server_socket_fd);
            assert(rc != -1);

            // Create a socket pair to notify the below thread when the document has been closed
            int notificationPipe[2];
            fakeSocketPipe2(notificationPipe);
            closeNotificationFd = notificationPipe[0];
            const int threadNotificationFd = notificationPipe[1];

            // The thread below delivers to the activity this document was opened in, whatever
            // activity is current by then.
            jobject threadActivity = env->NewGlobalRef(instance);

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
            const std::string handshake(fileURL + " " + std::to_string(appDocId.load()));
            LOG_DBG("Actually sending to Online:" << handshake);

            // Send the document URL to COOLWSD to setup the docBroker URL
            fakeSocketWriteQueue(currentFakeClientFd, handshake.c_str(), handshake.size());
        }
        else
        {
            // Send the message to COOLWSD
            fakeSocketWriteQueue(fakeClientFd, string_value, strlen(string_value));
        }
    }
    else
        LOG_DBG("From JS: cool: some object");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_libreoffice_androidlib_LOActivity_closeDocumentNative(JNIEnv*, jobject)
{
    return closeDocument() ? JNI_TRUE : JNI_FALSE;
}

extern "C" jboolean cokit_initialize(JNIEnv* env, jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager);

/// Create the COOLWSD instance.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_createCOOLWSD(JNIEnv* env, jobject, jstring dataDir,
                                                         jstring cacheDir, jstring apkFile,
                                                         jobject assetManager,
                                                         jstring loadFileURL, jstring uiMode,
                                                         jstring userName)
{
    fileURL = std::string(env->GetStringUTFChars(loadFileURL, nullptr));

    // already initialized?
    if (lokInitialized)
    {
        closeDocument();
        return;
    }
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
Java_org_libreoffice_androidlib_LOActivity_saveAs(JNIEnv *env, jobject,
                                                  jstring fileUri_, jstring format_,
                                                  jstring options_) {
    const char *fileUri = env->GetStringUTFChars(fileUri_, 0);
    const char *format = env->GetStringUTFChars(format_, 0);
    const char *options = nullptr;
    if (options_ != nullptr)
        options = env->GetStringUTFChars(options_, 0);

    if (COKitDocument* document = currentDocument())
        document->saveAs(fileUri, format, options);

    env->ReleaseStringUTFChars(fileUri_, fileUri);
    env->ReleaseStringUTFChars(format_, format);
    if (options_ != nullptr)
        env->ReleaseStringUTFChars(options_, options);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_postUnoCommand(JNIEnv* pEnv, jobject,
                                                          jstring command, jstring arguments, jboolean bNotifyWhenFinished)
{
    const char* pCommand = pEnv->GetStringUTFChars(command, nullptr);
    const char* pArguments = nullptr;
    if (arguments != nullptr)
        pArguments = pEnv->GetStringUTFChars(arguments, nullptr);

    if (COKitDocument* document = currentDocument())
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
Java_org_libreoffice_androidlib_COWebViewClient_getEmbeddedMediaPath(JNIEnv *env, jobject, jstring inTag) {
    std::string tag = copyJavaString(env, inTag);
    std::string mediaPath;
    if (const std::optional<DocumentData> data = DocumentData::getCopy(appDocId))
    {
        if (std::shared_ptr<DocumentBroker> docBroker = data->docBroker.lock())
            mediaPath = docBroker->getEmbeddedMediaPath(tag);
    }
    return env->NewStringUTF(mediaPath.c_str());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
