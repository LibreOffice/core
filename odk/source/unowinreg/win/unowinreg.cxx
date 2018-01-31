/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <jni.h>

extern "C" BOOL __stdcall _DllMainCRTStartup(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenClassesRoot(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, nullptr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenCurrentConfig(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(HKEY_CURRENT_CONFIG, nullptr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenCurrentUser(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, nullptr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenLocalMachine(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, nullptr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenUsers(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(HKEY_USERS, nullptr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey)) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenKeyEx(
    JNIEnv *env, jclass, jlong parent, jstring name, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    const char *namestr = env->GetStringUTFChars(name, nullptr);
    auto phkey = env->GetLongArrayElements(hkresult, nullptr);
    if (RegOpenKeyEx(reinterpret_cast<HKEY>(parent), namestr, 0, KEY_READ, reinterpret_cast<PHKEY>(phkey))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseStringUTFChars(name, namestr);
    env->ReleaseLongArrayElements(hkresult, phkey, 0);
    return ret;
}


extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegCloseKey(
    JNIEnv *, jclass, jlong hkey)
{
    jboolean ret = JNI_FALSE;
    if (RegCloseKey(reinterpret_cast<HKEY>(hkey)) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    return ret;
}

extern "C" JNIEXPORT jboolean
    JNICALL Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegQueryValueEx(
    JNIEnv *env, jclass, jlong hkey, jstring value, jlongArray type,
    jbyteArray data, jlongArray size)
{
    jboolean ret = JNI_FALSE;
    const char* valuestr = env->GetStringUTFChars(value, nullptr);
    auto ptype = env->GetLongArrayElements(type, nullptr);
    auto pdata = env->GetByteArrayElements(data, nullptr);
    auto psize = env->GetLongArrayElements(size, nullptr);
    if (RegQueryValueEx(reinterpret_cast<HKEY>(hkey), valuestr, nullptr, reinterpret_cast<LPDWORD>(ptype), reinterpret_cast<LPBYTE>(pdata), reinterpret_cast<LPDWORD>(psize))
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseStringUTFChars(value, valuestr);
    env->ReleaseLongArrayElements(type, ptype, 0);
    env->ReleaseByteArrayElements(data, pdata, 0);
    env->ReleaseLongArrayElements(size, psize, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegQueryInfoKey(
    JNIEnv *env, jclass, jlong hkey, jlongArray subkeys,
    jlongArray maxSubkeyLen, jlongArray values, jlongArray maxValueNameLen,
    jlongArray maxValueLen, jlongArray secDescriptor)
{
    jboolean ret = JNI_FALSE;
    auto psubkeys = env->GetLongArrayElements(subkeys, nullptr);
    auto pmaxSubkeyLen = env->GetLongArrayElements(maxSubkeyLen, nullptr);
    auto pvalues = env->GetLongArrayElements(values, nullptr);
    auto pmaxValueNameLen = env->GetLongArrayElements(maxValueNameLen, nullptr);
    auto pmaxValueLen = env->GetLongArrayElements(maxValueLen, nullptr);
    auto psecDescriptor = env->GetLongArrayElements(secDescriptor, nullptr);
    FILETIME ft;
    if (RegQueryInfoKey(reinterpret_cast<HKEY>(hkey), nullptr, nullptr, nullptr, reinterpret_cast<LPDWORD>(psubkeys), reinterpret_cast<LPDWORD>(pmaxSubkeyLen),
                        nullptr, reinterpret_cast<LPDWORD>(pvalues), reinterpret_cast<LPDWORD>(pmaxValueNameLen), reinterpret_cast<LPDWORD>(pmaxValueLen),
                        reinterpret_cast<LPDWORD>(psecDescriptor), &ft) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(subkeys, psubkeys, 0);
    env->ReleaseLongArrayElements(maxSubkeyLen, pmaxSubkeyLen, 0);
    env->ReleaseLongArrayElements(values, pvalues, 0);
    env->ReleaseLongArrayElements(maxValueNameLen, pmaxValueNameLen, 0);
    env->ReleaseLongArrayElements(maxValueLen, pmaxValueLen, 0);
    env->ReleaseLongArrayElements(secDescriptor, psecDescriptor, 0);
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
