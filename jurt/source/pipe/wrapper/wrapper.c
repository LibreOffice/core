/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
