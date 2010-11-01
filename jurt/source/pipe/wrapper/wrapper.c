/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

#include "sal/config.h"

#include <stddef.h>

#include <Windows.h>

#include "jni.h"
#include "sal/types.h"

static HMODULE module;

static FARPROC getFunction(char const * name) {
    return GetProcAddress(module, name);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void) lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        wchar_t path[32767];
        DWORD size;
        size = GetModuleFileNameW(hinstDLL, path, 32767);
        if (size == 0) {
            return FALSE;
        }
        path[size - 5] = L'x'; /* ...\jpipe.dll -> ...\jpipx.dll */
        module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (module == NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_connections_pipe_PipeConnection_createJNI(
    JNIEnv * env, jobject obj_this, jstring name)
{
    (*(void (*)(JNIEnv *, jobject, jstring))
     getFunction("PipeConnection_create"))(env, obj_this, name);
}

SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_connections_pipe_PipeConnection_closeJNI(
    JNIEnv * env, jobject obj_this)
{
    (*(void (*)(JNIEnv *, jobject))
     getFunction("PipeConnection_close"))(env, obj_this);
}

SAL_DLLPUBLIC_EXPORT jint JNICALL
Java_com_sun_star_lib_connections_pipe_PipeConnection_readJNI(
    JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len)
{
    return (*(jint (*)(JNIEnv *, jobject, jobjectArray, jint))
            getFunction("PipeConnection_read"))(env, obj_this, buffer, len);
}

SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_connections_pipe_PipeConnection_writeJNI(
    JNIEnv * env, jobject obj_this, jbyteArray buffer)
{
    (*(void (*)(JNIEnv *, jobject, jbyteArray))
     getFunction("PipeConnection_write"))(env, obj_this, buffer);
}

SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_connections_pipe_PipeConnection_flushJNI(
    JNIEnv * env, jobject obj_this)
{
    (*(void (*)(JNIEnv *, jobject))
     getFunction("PipeConnection_flush"))(env, obj_this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
