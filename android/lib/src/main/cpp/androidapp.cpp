/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <jni.h>
#include <android/log.h>

#include <chrono>
#include <thread>

#include <FakeSocket.hpp>
#include <Kit.hpp>
#include <common/Log.hpp>
#include <COOLWSD.hpp>
#include <Protocol.hpp>
#include <SetupKitEnvironment.hpp>
#include <common/Util.hpp>

#define LIBO_INTERNAL_ONLY
#include <COKit/COKit.hxx>

#include <osl/detail/android-bootstrap.h>

const int SHOW_JS_MAXLEN = 70;

int coolwsd_server_socket_fd = -1;

const char* user_name;

static std::string fileURL;
static int fakeClientFd;
static int closeNotificationPipeForForwardingThread[2] = {-1, -1};
static JavaVM* javaVM = nullptr;
static bool lokInitialized = false;
static std::mutex coolwsdRunningMutex;

// Remember the reference to the LOActivity
jclass g_loActivityClz = nullptr;
jobject g_loActivityObj = nullptr;

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
    return JNI_VERSION_1_6;
}

// Exception safe JVM detach, JNIEnv is TLS for Java - so per-thread.
class JNIThreadContext
{
    JNIEnv *_env;
public:
    JNIThreadContext()
    {
        assert(javaVM != nullptr);
        jint res = javaVM->GetEnv((void**)&_env, JNI_VERSION_1_6);
        if (res == JNI_EDETACHED) {
            LOG_DBG("Attach worker thread");
            res = javaVM->AttachCurrentThread(&_env, nullptr);
            if (JNI_OK != res) {
                LOG_DBG("Failed to AttachCurrentThread");
            }
        }
        else if (res == JNI_EVERSION) {
            LOG_DBG("GetEnv version not supported");
            return;
        }
        else if (res != JNI_OK) {
            LOG_DBG("GetEnv another error " << res);
            return;
        }
    }

    ~JNIThreadContext()
    {
        javaVM->DetachCurrentThread();
    }

    JNIEnv *getEnv() const { return _env; }
};

static void send2JS(const JNIThreadContext &jctx, const std::vector<char>& buffer)
{
    LOG_DBG("Send to JS: " << COOLProtocol::getAbbreviatedMessage(buffer.data(), buffer.size()));

    JNIEnv *env = jctx.getEnv();

    jbyteArray jmessage = env->NewByteArray(buffer.size());
    env->SetByteArrayRegion(jmessage, 0, buffer.size(),
                            reinterpret_cast<const jbyte *>(buffer.data()));

    jmethodID callFakeWebsocket = env->GetMethodID(g_loActivityClz, "rawCallFakeWebsocketOnMessage", "([B)V");
    env->CallVoidMethod(g_loActivityObj, callFakeWebsocket, jmessage);
    env->DeleteLocalRef(jmessage);

    if (env->ExceptionCheck())
        env->ExceptionDescribe();
}

void postDirectMessage(std::string message)
{
    JNIThreadContext ctx;
    JNIEnv *env = ctx.getEnv();

    jstring jstr = env->NewStringUTF(message.c_str());
    jmethodID callPostMobileMessage = env->GetMethodID(g_loActivityClz, "postMobileMessage", "(Ljava/lang/String;)V");
    env->CallVoidMethod(g_loActivityObj, callPostMobileMessage, jstr);
    env->DeleteLocalRef(jstr);

    if (env->ExceptionCheck())
        env->ExceptionDescribe();
}

/// Close the document.
void closeDocument()
{
    // Close one end of the socket pair, that will wake up the forwarding thread that was constructed in HULLO
    fakeSocketClose(closeNotificationPipeForForwardingThread[0]);
    LOG_DBG("Waiting for COOLWSD to finish...");
    std::unique_lock<std::mutex> coolwsdLock(coolwsdRunningMutex);
    LOG_DBG("COOLWSD has finished.");
}

/// Handle a message from JavaScript.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_postMobileMessageNative(JNIEnv *env, jobject, jstring message)
{
    const char *string_value = env->GetStringUTFChars(message, nullptr);

    if (string_value)
    {
        LOG_DBG("From JS: cool: " << string_value);

        // we need a copy, because we can get a new one while we are still
        // taking down the old one
        const int currentFakeClientFd = fakeClientFd;

        if (strcmp(string_value, "HULLO") == 0)
        {
            // Now we know that the JS has started completely

            // Contact the permanently (during app lifetime) listening COOLWSD server
            // "public" socket
            assert(coolwsd_server_socket_fd != -1);

            int rc = fakeSocketConnect(currentFakeClientFd, coolwsd_server_socket_fd);
            assert(rc != -1);

            // Create a socket pair to notify the below thread when the document has been closed
            fakeSocketPipe2(closeNotificationPipeForForwardingThread);

            // Start another thread to read responses and forward them to the JavaScript
            std::thread([currentFakeClientFd]
                        {
                            ProcUtil::setThreadName("app2js");
                            JNIThreadContext ctx;
                            while (true)
                            {
                               struct pollfd pollfd[2];
                               pollfd[0].fd = currentFakeClientFd;
                               pollfd[0].events = POLLIN;
                               pollfd[1].fd = closeNotificationPipeForForwardingThread[1];
                               pollfd[1].events = POLLIN;
                               if (fakeSocketPoll(pollfd, 2, -1) > 0)
                               {
                                   if (pollfd[1].revents == POLLIN)
                                   {
                                       LOG_DBG("app2js: closing the sockets");
                                       // The code below handling the "BYE" fake Websocket
                                       // message has closed the other end of the
                                       // closeNotificationPipeForForwardingThread. Let's close
                                       // the other end too just for cleanliness, even if a
                                       // FakeSocket as such is not a system resource so nothing
                                       // is saved by closing it.
                                       fakeSocketClose(closeNotificationPipeForForwardingThread[1]);

                                       // Close our end of the fake socket connection to the
                                       // ClientSession thread, so that it terminates
                                       fakeSocketClose(currentFakeClientFd);

                                       return;
                                   }
                                   if (pollfd[0].revents == POLLIN)
                                   {
                                       int n = fakeSocketAvailableDataLength(currentFakeClientFd);
                                       if (n == 0)
                                           return;
                                       std::vector<char> buf(n);
                                       n = fakeSocketRead(currentFakeClientFd, buf.data(), n);
                                       send2JS(ctx, buf);
                                   }
                               }
                               else
                                   break;
                           }
                           assert(false);
                        }).detach();

            // First we simply send it the URL. This corresponds to the GET request with Upgrade to
            // WebSocket.
            LOG_DBG("Actually sending to Online:" << fileURL);

            // Send the document URL to COOLWSD to setup the docBroker URL
            fakeSocketWriteQueue(currentFakeClientFd, fileURL.c_str(), fileURL.size());
        }
        else if (strcmp(string_value, "BYE") == 0)
        {
            LOG_DBG("Document window terminating on JavaScript side. Closing our end of the socket.");

            closeDocument();
        }
        else
        {
            // Send the message to COOLWSD
            fakeSocketWriteQueue(currentFakeClientFd, string_value, strlen(string_value));
        }
    }
    else
        LOG_DBG("From JS: cool: some object");
}

extern "C" jboolean cokit_initialize(JNIEnv* env, jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager);

/// Create the COOLWSD instance.
extern "C" JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_createCOOLWSD(JNIEnv *env, jobject instance, jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager, jstring loadFileURL, jstring uiMode, jstring userName)
{
    fileURL = std::string(env->GetStringUTFChars(loadFileURL, nullptr));

    // remember the LOActivity class and object to be able to call back
    env->DeleteGlobalRef(g_loActivityClz);
    env->DeleteGlobalRef(g_loActivityObj);

    jclass clz = env->GetObjectClass(instance);
    g_loActivityClz = (jclass) env->NewGlobalRef(clz);
    g_loActivityObj = env->NewGlobalRef(instance);

    // already initialized?
    if (lokInitialized)
    {
        // close the previous document so that we can wait for the new HULLO
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
                    while (true)
                    {
                        LOG_DBG("Creating COOLWSD");
                        {
                            std::unique_lock<std::mutex> lock(coolwsdRunningMutex);
                            fakeClientFd = fakeSocketSocket();
                            LOG_DBG("createCOOLWSD created fakeClientFd: " << fakeClientFd);
                            std::unique_ptr<COOLWSD> coolwsd = std::make_unique<COOLWSD>();
                            coolwsd->run(1, argv);
                        }
                        LOG_DBG("One run of COOLWSD completed");
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
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

    getLOKDocumentForAndroidOnly()->saveAs(fileUri, format, options);

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

    getLOKDocumentForAndroidOnly()->postUnoCommand(pCommand, pArguments, bNotifyWhenFinished);

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
JNIEXPORT jboolean JNICALL
Java_org_libreoffice_androidlib_LOActivity_getClipboardContent(JNIEnv *env, jobject, jobject lokClipboardData)
{
    const char** mimeTypes = nullptr;
    size_t outCount = 0;
    char  **outMimeTypes = nullptr;
    size_t *outSizes = nullptr;
    char  **outStreams = nullptr;
    bool bResult = false;

    jclass jclazz = env->FindClass("java/util/ArrayList");
    jmethodID methodId_ArrayList_Add = env->GetMethodID(jclazz, "add", "(Ljava/lang/Object;)Z");

    jclass class_LokClipboardEntry = env->FindClass("org/libreoffice/androidlib/lok/LokClipboardEntry");
    jmethodID methodId_LokClipboardEntry_Constructor = env->GetMethodID(class_LokClipboardEntry, "<init>", "()V");
    jfieldID fieldId_LokClipboardEntry_Mime = env->GetFieldID(class_LokClipboardEntry , "mime", "Ljava/lang/String;");
    jfieldID fieldId_LokClipboardEntry_Data = env->GetFieldID(class_LokClipboardEntry, "data", "[B");

    jclass class_LokClipboardData = env->GetObjectClass(lokClipboardData);
    jfieldID fieldId_LokClipboardData_clipboardEntries = env->GetFieldID(class_LokClipboardData , "clipboardEntries", "Ljava/util/ArrayList;");

    if (getLOKDocumentForAndroidOnly()->getClipboard(mimeTypes,
                                                     &outCount, &outMimeTypes,
                                                     &outSizes, &outStreams))
    {
        // return early
        if (outCount == 0)
            return bResult;

        for (size_t i = 0; i < outCount; ++i)
        {
            // Create new LokClipboardEntry instance
            jobject clipboardEntry = env->NewObject(class_LokClipboardEntry, methodId_LokClipboardEntry_Constructor);

            jstring mimeType = tojstringAndFree(env, outMimeTypes[i]);
            // clipboardEntry.mime= mimeType
            env->SetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Mime, mimeType);
            env->DeleteLocalRef(mimeType);

            size_t aByteArraySize = outSizes[i];
            jbyteArray aByteArray = env->NewByteArray(aByteArraySize);
            // Copy char* to bytearray
            env->SetByteArrayRegion(aByteArray, 0, aByteArraySize, (jbyte*) outStreams[i]);
            // clipboardEntry.data = aByteArray
            env->SetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Data, aByteArray);

            // clipboardData.clipboardEntries
            jobject lokClipboardData_clipboardEntries = env->GetObjectField(lokClipboardData, fieldId_LokClipboardData_clipboardEntries);

            // clipboardEntries.add(clipboardEntry)
            env->CallBooleanMethod(lokClipboardData_clipboardEntries, methodId_ArrayList_Add, clipboardEntry);
        }
        bResult = true;
    }
    else
        LOG_DBG("failed to fetch mime-types");

    const char* mimeTypesHTML[] = { "text/plain;charset=utf-8", "text/html", nullptr };

    if (getLOKDocumentForAndroidOnly()->getClipboard(mimeTypesHTML,
                                                     &outCount, &outMimeTypes,
                                                     &outSizes, &outStreams))
    {
        // return early
        if (outCount == 0)
            return bResult;

        for (size_t i = 0; i < outCount; ++i)
        {
            // Create new LokClipboardEntry instance
            jobject clipboardEntry = env->NewObject(class_LokClipboardEntry, methodId_LokClipboardEntry_Constructor);

            jstring mimeType = tojstringAndFree(env, outMimeTypes[i]);
            // clipboardEntry.mime= mimeType
            env->SetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Mime, mimeType);
            env->DeleteLocalRef(mimeType);

            size_t aByteArraySize = outSizes[i];
            jbyteArray aByteArray = env->NewByteArray(aByteArraySize);
            // Copy char* to bytearray
            env->SetByteArrayRegion(aByteArray, 0, aByteArraySize, (jbyte*) outStreams[i]);
            // clipboardEntry.data = aByteArray
            env->SetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Data, aByteArray);

            // clipboardData.clipboardEntries
            jobject lokClipboardData_clipboardEntries = env->GetObjectField(lokClipboardData, fieldId_LokClipboardData_clipboardEntries);

            // clipboardEntries.add(clipboardEntry)
            env->CallBooleanMethod(lokClipboardData_clipboardEntries, methodId_ArrayList_Add, clipboardEntry);
        }
        bResult = true;
    }
    else
        LOG_DBG("failed to fetch mime-types");

    return bResult;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_setClipboardContent(JNIEnv *env, jobject, jobject lokClipboardData) {
    jclass class_ArrayList= env->FindClass("java/util/ArrayList");
    jmethodID methodId_ArrayList_ToArray = env->GetMethodID(class_ArrayList, "toArray", "()[Ljava/lang/Object;");

    jclass class_LokClipboardEntry = env->FindClass("org/libreoffice/androidlib/lok/LokClipboardEntry");
    jfieldID fieldId_LokClipboardEntry_Mime = env->GetFieldID(class_LokClipboardEntry , "mime", "Ljava/lang/String;");
    jfieldID fieldId_LokClipboardEntry_Data = env->GetFieldID(class_LokClipboardEntry, "data", "[B");

    jclass class_LokClipboardData = env->GetObjectClass(lokClipboardData);
    jfieldID fieldId_LokClipboardData_clipboardEntries = env->GetFieldID(class_LokClipboardData , "clipboardEntries", "Ljava/util/ArrayList;");

    jobject lokClipboardData_clipboardEntries = env->GetObjectField(lokClipboardData, fieldId_LokClipboardData_clipboardEntries);

    jobjectArray clipboardEntryArray = (jobjectArray) env->CallObjectMethod(lokClipboardData_clipboardEntries, methodId_ArrayList_ToArray);

    size_t nEntrySize= env->GetArrayLength(clipboardEntryArray);

    if (nEntrySize == 0)
        return;

    size_t pSizes[nEntrySize];
    const char* pMimeTypes[nEntrySize];
    const char* pStreams[nEntrySize];

    for (size_t nEntryIndex = 0; nEntryIndex < nEntrySize; ++nEntryIndex)
    {
        jobject clipboardEntry = env->GetObjectArrayElement(clipboardEntryArray, nEntryIndex);

        jstring mimetype = (jstring) env->GetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Mime);
        jbyteArray data = (jbyteArray) env->GetObjectField(clipboardEntry, fieldId_LokClipboardEntry_Data);

        pMimeTypes[nEntryIndex] = copyJavaString(env, mimetype);

        size_t dataArrayLength = env->GetArrayLength(data);
        char* dataArray = new char[dataArrayLength];
        env->GetByteArrayRegion(data, 0, dataArrayLength, reinterpret_cast<jbyte*>(dataArray));

        pSizes[nEntryIndex] = dataArrayLength;
        pStreams[nEntryIndex] = dataArray;
    }

    getLOKDocumentForAndroidOnly()->setClipboard(nEntrySize, pMimeTypes, pSizes, pStreams);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libreoffice_androidlib_LOActivity_paste(JNIEnv *env, jobject, jstring inMimeType, jbyteArray inData) {
    const char* mimeType = env->GetStringUTFChars(inMimeType, nullptr);

    size_t dataArrayLength = env->GetArrayLength(inData);
    char* dataArray = new char[dataArrayLength];
    env->GetByteArrayRegion(inData, 0, dataArrayLength, reinterpret_cast<jbyte*>(dataArray));
    getLOKDocumentForAndroidOnly()->paste(mimeType, dataArray, dataArrayLength);
    env->ReleaseStringUTFChars(inMimeType, mimeType);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_org_libreoffice_androidlib_COWebViewClient_getEmbeddedMediaPath(JNIEnv *env, jobject, jstring inTag) {
    std::string tag = copyJavaString(env, inTag);
    std::string mediaPath = getDocumentBrokerForAndroidOnly()->getEmbeddedMediaPath(tag);
    return env->NewStringUTF(mediaPath.c_str());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
