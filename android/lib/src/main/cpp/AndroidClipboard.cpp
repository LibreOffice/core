/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The COKitClipboardProvider for the Android app. Moves the bytes between the engine and the
// static methods of SystemClipboard.java, which does the ClipboardManager calls.

#include <config.h>

#include <jni.h>

#include <string>
#include <string_view>
#include <vector>

#include <COKit/COKit.hxx>
#include <common/Log.hpp>

#include <androidapp.hpp>

namespace
{
// the engine, for reading the process-shared clipboard
COKit* sOffice = nullptr;

// Set while the advertise pulls the formats from the engine. The engine serves them from its
// in-memory copy only while the clipboard is ours, so the ownership check says so meanwhile.
bool sAdvertising = false;

// Global references, resolved in JNI_OnLoad: FindClass from the kit thread, which the JVM did not
// create, only sees the system classes.
jclass sSystemClipboardClass = nullptr;
jclass sStringClass = nullptr;
jclass sByteArrayClass = nullptr;
jmethodID sAdvertiseMethod = nullptr;
jmethodID sOwnsClipboardMethod = nullptr;
jmethodID sGetMimeTypesMethod = nullptr;
jmethodID sGetDataMethod = nullptr;

/// the JNIEnv of this thread, or null when the Java side was not resolved or the thread could
/// not be attached
JNIEnv* envFor(const JNIThreadContext& rContext)
{
    return sSystemClipboardClass != nullptr ? rContext.getEnv() : nullptr;
}

/// report and clear a pending Java exception, true when there was one
bool clearException(JNIEnv* env, const char* what)
{
    if (!env->ExceptionCheck())
        return false;

    env->ExceptionDescribe();
    env->ExceptionClear();
    LOG_ERR("clipboard provider: " << what << " threw a Java exception");
    return true;
}

// Copy. ClipData has no lazy owner like the macOS pasteboard, so every format is pulled from
// the engine now and handed to Java in one call.
void clipboardProviderAdvertise(const char** pMimeTypes)
{
    std::vector<std::string> mimeTypes;
    std::vector<std::vector<char>> payloads;

    sAdvertising = true;
    for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i)
    {
        std::vector<char> bytes;
        if (sOffice && sOffice->getGlobalClipboardData(pMimeTypes[i], bytes))
        {
            // the engine's text exports end with a zero byte, a receiver would take it as text
            if (std::string_view(pMimeTypes[i]).starts_with("text/"))
            {
                while (!bytes.empty() && bytes.back() == '\0')
                    bytes.pop_back();
            }
            if (bytes.empty())
                continue;
            mimeTypes.emplace_back(pMimeTypes[i]);
            payloads.push_back(std::move(bytes));
        }
    }
    sAdvertising = false;

    LOG_DBG("clipboard provider: advertising " << mimeTypes.size() << " of the engine's formats");
    if (mimeTypes.empty())
        return;

    JNIThreadContext jni(16);
    JNIEnv* env = envFor(jni);
    if (!env)
        return;

    const jsize count = static_cast<jsize>(mimeTypes.size());
    jobjectArray jMimeTypes = env->NewObjectArray(count, sStringClass, nullptr);
    jobjectArray jPayloads = env->NewObjectArray(count, sByteArrayClass, nullptr);
    if (clearException(env, "advertise array allocation") || !jMimeTypes || !jPayloads)
        return;

    for (jsize i = 0; i < count; ++i)
    {
        jstring jMime = env->NewStringUTF(mimeTypes[i].c_str());
        env->SetObjectArrayElement(jMimeTypes, i, jMime);
        env->DeleteLocalRef(jMime);

        const jsize size = static_cast<jsize>(payloads[i].size());
        jbyteArray jData = env->NewByteArray(size);
        if (clearException(env, "advertise payload allocation") || !jData)
            return;
        env->SetByteArrayRegion(jData, 0, size, reinterpret_cast<const jbyte*>(payloads[i].data()));
        env->SetObjectArrayElement(jPayloads, i, jData);
        env->DeleteLocalRef(jData);
    }

    env->CallStaticVoidMethod(sSystemClipboardClass, sAdvertiseMethod, jMimeTypes, jPayloads);
    clearException(env, "advertise");
}

bool clipboardProviderOwns()
{
    if (sAdvertising)
        return true;

    JNIThreadContext jni(4);
    JNIEnv* env = envFor(jni);
    if (!env)
        return false;

    const jboolean owns = env->CallStaticBooleanMethod(sSystemClipboardClass, sOwnsClipboardMethod);
    if (clearException(env, "ownsClipboard"))
        return false;
    return owns == JNI_TRUE;
}

std::vector<std::string> clipboardProviderGetMimeTypes()
{
    std::vector<std::string> result;

    JNIThreadContext jni(8);
    JNIEnv* env = envFor(jni);
    if (!env)
        return result;

    auto jMimeTypes = static_cast<jobjectArray>(
        env->CallStaticObjectMethod(sSystemClipboardClass, sGetMimeTypesMethod));
    if (clearException(env, "getMimeTypes") || !jMimeTypes)
        return result;

    const jsize count = env->GetArrayLength(jMimeTypes);
    result.reserve(count);
    for (jsize i = 0; i < count; ++i)
    {
        auto jMime = static_cast<jstring>(env->GetObjectArrayElement(jMimeTypes, i));
        if (!jMime)
            continue;
        const char* chars = env->GetStringUTFChars(jMime, nullptr);
        if (chars)
        {
            result.emplace_back(chars);
            env->ReleaseStringUTFChars(jMime, chars);
        }
        env->DeleteLocalRef(jMime);
    }
    return result;
}

bool clipboardProviderGetData(const char* pMimeType, std::vector<char>* pOutData)
{
    JNIThreadContext jni(4);
    JNIEnv* env = envFor(jni);
    if (!env)
        return false;

    jstring jMime = env->NewStringUTF(pMimeType);
    auto jData = static_cast<jbyteArray>(
        env->CallStaticObjectMethod(sSystemClipboardClass, sGetDataMethod, jMime));
    if (clearException(env, "getData") || !jData)
        return false;

    const jsize size = env->GetArrayLength(jData);
    if (size <= 0)
        return false;

    pOutData->resize(size);
    env->GetByteArrayRegion(jData, 0, size, reinterpret_cast<jbyte*>(pOutData->data()));
    return !clearException(env, "getData copy");
}

jclass globalClassRef(JNIEnv* env, const char* name)
{
    jclass local = env->FindClass(name);
    if (!local)
    {
        env->ExceptionClear();
        LOG_ERR("clipboard provider: class " << name << " not found");
        return nullptr;
    }
    auto global = static_cast<jclass>(env->NewGlobalRef(local));
    env->DeleteLocalRef(local);
    return global;
}

jmethodID staticMethod(JNIEnv* env, const char* name, const char* signature)
{
    jmethodID method = env->GetStaticMethodID(sSystemClipboardClass, name, signature);
    if (!method)
    {
        env->ExceptionClear();
        LOG_ERR("clipboard provider: method SystemClipboard." << name << signature << " not found");
    }
    return method;
}
}

void resolveSystemClipboardClass(JNIEnv* env)
{
    sSystemClipboardClass = globalClassRef(env, "org/libreoffice/androidlib/SystemClipboard");
    sStringClass = globalClassRef(env, "java/lang/String");
    sByteArrayClass = globalClassRef(env, "[B");
    if (!sSystemClipboardClass || !sStringClass || !sByteArrayClass)
    {
        sSystemClipboardClass = nullptr;
        return;
    }

    sAdvertiseMethod = staticMethod(env, "advertise", "([Ljava/lang/String;[[B)V");
    sOwnsClipboardMethod = staticMethod(env, "ownsClipboard", "()Z");
    sGetMimeTypesMethod = staticMethod(env, "getMimeTypes", "()[Ljava/lang/String;");
    sGetDataMethod = staticMethod(env, "getData", "(Ljava/lang/String;)[B");
    if (!sAdvertiseMethod || !sOwnsClipboardMethod || !sGetMimeTypesMethod || !sGetDataMethod)
    {
        env->DeleteGlobalRef(sSystemClipboardClass);
        sSystemClipboardClass = nullptr;
    }
}

// declared in androidapp.hpp
void install_clipboard_provider(COKit& rOffice)
{
    sOffice = &rOffice;

    static COKitClipboardProvider provider{};
    provider.advertiseToPlatform = clipboardProviderAdvertise;
    provider.ownsClipboard = clipboardProviderOwns;
    provider.getMimeTypes = clipboardProviderGetMimeTypes;
    provider.getDataForMimeType = clipboardProviderGetData;
    rOffice.installClipboardProvider(&provider);
    LOG_INF("clipboard provider: installed, Java class "
            << (sSystemClipboardClass ? "resolved" : "missing"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
