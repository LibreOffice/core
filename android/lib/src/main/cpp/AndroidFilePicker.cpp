/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// File picks of the engine on Android: LOActivity shows the system document picker.

#include <config.h>

#include <jni.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <COKit/COKit.hxx>
#include <Poco/UnicodeConverter.h>
#include <common/Log.hpp>

#include <androidapp.hpp>

namespace
{
struct PendingPick
{
    void (*pfnPicked)(void* pContext, const char* pUrl);
    void* pContext;
};

// resolved in JNI_OnLoad
jclass sActivityClass = nullptr;
jmethodID sShowFilePickerMethod = nullptr;

std::mutex sPicksMutex;
// the open picks by id
std::map<jint, PendingPick> sPicks;
jint sNextPickId = 1;

/// ex: ".png". Empty for any file.
std::vector<std::string> toExtensions(const COKitFilePickerFilter* pFilters, size_t nFilters)
{
    std::vector<std::string> extensions;
    for (size_t i = 0; pFilters && i < nFilters; ++i)
    {
        std::string wildcards = pFilters[i].pWildcards ? pFilters[i].pWildcards : "";
        size_t start = 0;
        while (start <= wildcards.size())
        {
            size_t end = wildcards.find(';', start);
            if (end == std::string::npos)
                end = wildcards.size();
            std::string wildcard = wildcards.substr(start, end - start);
            start = end + 1;

            const size_t first = wildcard.find_first_not_of(" \t");
            const size_t last = wildcard.find_last_not_of(" \t");
            if (first == std::string::npos)
                continue;
            wildcard = wildcard.substr(first, last - first + 1);

            if (wildcard.size() <= 2 || wildcard.compare(0, 2, "*.") != 0 || wildcard == "*.*")
                continue;
            std::string extension = wildcard.substr(1);
            std::transform(extension.begin(), extension.end(), extension.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (std::find(extensions.begin(), extensions.end(), extension) == extensions.end())
                extensions.push_back(std::move(extension));
        }
    }
    return extensions;
}

/// NewStringUTF takes modified UTF-8, which encodes characters outside the Basic Multilingual
/// Plane differently, so convert through UTF-16.
jstring toJavaString(JNIEnv* env, const std::string& text)
{
    Poco::UTF16String utf16;
    Poco::UnicodeConverter::convert(text, utf16);
    return env->NewString(reinterpret_cast<const jchar*>(utf16.data()),
                          static_cast<jsize>(utf16.size()));
}

bool completePick(jint id, const char* pUrl)
{
    PendingPick pick{};
    {
        std::lock_guard<std::mutex> lock(sPicksMutex);
        const auto it = sPicks.find(id);
        if (it == sPicks.end())
        {
            LOG_DBG("file picker provider: no open pick [" << id << "] to complete");
            return false;
        }
        pick = it->second;
        sPicks.erase(it);
    }

    LOG_DBG("file picker provider: pick [" << id << "] completes "
                                            << (pUrl ? "with a file" : "as cancelled"));
    pick.pfnPicked(pick.pContext, pUrl);
    return true;
}

// Called on the kit thread. The answer comes back through filePickerResult.
void filePickerProviderPick(const char* pTitle, const COKitFilePickerFilter* pFilters,
                            size_t nFilters,
                            void (*pfnPicked)(void* pContext, const char* pUrl), void* pContext)
{
    const std::string title = pTitle ? pTitle : "";
    const std::vector<std::string> extensions = toExtensions(pFilters, nFilters);

    jint id;
    {
        std::lock_guard<std::mutex> lock(sPicksMutex);
        id = sNextPickId++;
        sPicks[id] = PendingPick{ pfnPicked, pContext };
    }
    LOG_DBG("file picker provider: pick [" << id << "] asks for a file, " << extensions.size()
                                            << " extension(s)");

    JNIThreadContext context;
    JNIEnv* env = context.getEnv();
    if (!env || !sActivityClass || !sShowFilePickerMethod || !g_stringClass)
    {
        LOG_WRN("file picker provider: no activity to show the picker; the pick is cancelled");
        completePick(id, nullptr);
        return;
    }

    jstring jtitle = toJavaString(env, title);
    if (clearJavaException(env, "file picker provider: title string") || !jtitle)
    {
        completePick(id, nullptr);
        return;
    }
    jobjectArray jextensions =
        env->NewObjectArray(static_cast<jsize>(extensions.size()), g_stringClass, nullptr);
    if (clearJavaException(env, "file picker provider: NewObjectArray") || !jextensions)
    {
        completePick(id, nullptr);
        return;
    }
    for (size_t i = 0; i < extensions.size(); ++i)
    {
        jstring jextension = toJavaString(env, extensions[i]);
        if (clearJavaException(env, "file picker provider: extension string") || !jextension)
        {
            completePick(id, nullptr);
            return;
        }
        env->SetObjectArrayElement(jextensions, static_cast<jsize>(i), jextension);
        env->DeleteLocalRef(jextension);
    }

    const jboolean shown = env->CallStaticBooleanMethod(sActivityClass, sShowFilePickerMethod, id,
                                                        jtitle, jextensions);
    if (clearJavaException(env, "file picker provider: showFilePicker") || !shown)
    {
        LOG_DBG("file picker provider: no activity in the foreground for pick [" << id << ']');
        completePick(id, nullptr);
    }
}
} // namespace

extern "C" JNIEXPORT jboolean JNICALL
Java_org_libreoffice_androidlib_LOActivity_filePickerResult(JNIEnv* env, jobject, jint id,
                                                            jstring url)
{
    if (!url)
        return completePick(id, nullptr) ? JNI_TRUE : JNI_FALSE;

    const char* chars = env->GetStringUTFChars(url, nullptr);
    const std::string copy = chars ? chars : "";
    if (chars)
        env->ReleaseStringUTFChars(url, chars);
    return completePick(id, copy.c_str()) ? JNI_TRUE : JNI_FALSE;
}

// declared in androidapp.hpp
void resolveFilePickerClass(JNIEnv* env)
{
    jclass activityClass = env->FindClass("org/libreoffice/androidlib/LOActivity");
    if (!activityClass)
    {
        env->ExceptionClear();
        LOG_ERR("file picker provider: class LOActivity not found");
        return;
    }

    sShowFilePickerMethod = env->GetStaticMethodID(activityClass, "showFilePicker",
                                                   "(ILjava/lang/String;[Ljava/lang/String;)Z");
    if (!sShowFilePickerMethod)
    {
        env->ExceptionClear();
        LOG_ERR("file picker provider: LOActivity.showFilePicker not found");
        return;
    }
    sActivityClass = static_cast<jclass>(env->NewGlobalRef(activityClass));
}

// declared in androidapp.hpp
void install_filepicker_provider(COKit& rOffice)
{
    static COKitFilePickerProvider provider{};
    provider.pick = filePickerProviderPick;
    // The engine gets a copy in the app cache, so it inserts the content and keeps no link.
    provider.picksFilesInPlace = false;
    rOffice.installFilePickerProvider(&provider);
    LOG_INF("file picker provider: installed");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
