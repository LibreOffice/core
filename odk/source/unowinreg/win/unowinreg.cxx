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


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, phkey)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenCurrentConfig(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx(HKEY_CURRENT_CONFIG, NULL, 0, KEY_READ, phkey)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenCurrentUser(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_READ, phkey)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenLocalMachine(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, phkey)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenUsers(
    JNIEnv *env, jclass, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx(HKEY_USERS, NULL, 0, KEY_READ, phkey) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegOpenKeyEx(
    JNIEnv *env, jclass, jlong parent, jstring name, jlongArray hkresult)
{
    jboolean ret = JNI_FALSE;
    const char *namestr = env->GetStringUTFChars(name, 0);
    PHKEY phkey = (PHKEY)env->GetLongArrayElements(hkresult, 0);
    if (RegOpenKeyEx((HKEY)parent, namestr, 0, KEY_READ, phkey)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseStringUTFChars(name, namestr);
    env->ReleaseLongArrayElements(hkresult, (jlong *)phkey, 0);
    return ret;
}


extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegCloseKey(
    JNIEnv *, jclass, jlong hkey)
{
    jboolean ret = JNI_FALSE;
    if (RegCloseKey((HKEY)hkey) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    return ret;
}

extern "C" JNIEXPORT jboolean
    JNICALL Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegQueryValueEx(
    JNIEnv *env, jclass, jlong hkey, jstring value, jlongArray type,
    jbyteArray data, jlongArray size)
{
    jboolean ret = JNI_FALSE;
    const char* valuestr = env->GetStringUTFChars(value, 0);
    LPDWORD ptype = (LPDWORD)env->GetLongArrayElements(type, 0);
    LPBYTE  pdata = (LPBYTE)env->GetByteArrayElements(data, 0);
    LPDWORD psize = (LPDWORD)env->GetLongArrayElements(size, 0);
    if (RegQueryValueEx((HKEY)hkey, valuestr, NULL, ptype, pdata, psize)
        == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseStringUTFChars(value, valuestr);
    env->ReleaseLongArrayElements(type, (jlong *)ptype, 0);
    env->ReleaseByteArrayElements(data, (jbyte *)pdata, 0);
    env->ReleaseLongArrayElements(size, (jlong *)psize, 0);
    return ret;
}

extern "C" JNIEXPORT jboolean JNICALL
    Java_com_sun_star_lib_loader_WinRegKey_winreg_1RegQueryInfoKey(
    JNIEnv *env, jclass, jlong hkey, jlongArray subkeys,
    jlongArray maxSubkeyLen, jlongArray values, jlongArray maxValueNameLen,
    jlongArray maxValueLen, jlongArray secDescriptor)
{
    jboolean ret = JNI_FALSE;
    LPDWORD psubkeys = (LPDWORD)env->GetLongArrayElements(subkeys, 0);
    LPDWORD pmaxSubkeyLen =
        (LPDWORD)env->GetLongArrayElements(maxSubkeyLen, 0);
    LPDWORD pvalues = (LPDWORD)env->GetLongArrayElements(values, 0);
    LPDWORD pmaxValueNameLen =
        (LPDWORD)env->GetLongArrayElements(maxValueNameLen, 0);
    LPDWORD pmaxValueLen =
        (LPDWORD)env->GetLongArrayElements(maxValueLen, 0);
    LPDWORD psecDescriptor =
        (LPDWORD)env->GetLongArrayElements(secDescriptor, 0);
    FILETIME ft;
    if (RegQueryInfoKey((HKEY)hkey, NULL, NULL, NULL, psubkeys, pmaxSubkeyLen,
                        NULL, pvalues, pmaxValueNameLen, pmaxValueLen,
                        psecDescriptor, &ft) == ERROR_SUCCESS)
        ret = JNI_TRUE;
    env->ReleaseLongArrayElements(subkeys, (jlong*)psubkeys, 0);
    env->ReleaseLongArrayElements(maxSubkeyLen, (jlong*)pmaxSubkeyLen, 0);
    env->ReleaseLongArrayElements(values, (jlong*)pvalues, 0);
    env->ReleaseLongArrayElements(maxValueNameLen, (jlong*)pmaxValueNameLen, 0);
    env->ReleaseLongArrayElements(maxValueLen, (jlong*)pmaxValueLen, 0);
    env->ReleaseLongArrayElements(secDescriptor, (jlong*)psecDescriptor, 0);
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
