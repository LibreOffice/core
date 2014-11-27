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

#include "sal/config.h"

#include <stddef.h>

#include <windows.h>

#include "jni.h"
#include "sal/types.h"



static HINSTANCE hInstDLL = NULL;     // handle for this DLL, need for get the DLL-Name
static HMODULE   module   = NULL;     // handle for the loading DLL, need in Init and close

// temp function pointer
typedef void (__cdecl* FnPtr)();

// type define for the function variables
typedef void (__cdecl* pc_Create) (JNIEnv *, jobject, jstring);
typedef void (__cdecl* pc_Close ) (JNIEnv *, jobject);
typedef jint (__cdecl* pc_Read  ) (JNIEnv *, jobject, jobjectArray, jint);
typedef void (__cdecl* pc_Write ) (JNIEnv *, jobject, jbyteArray);
typedef void (__cdecl* pc_Flush ) (JNIEnv *, jobject);


// forward declaration for the Internal function
void __cdecl I_PipeConnection_create_Dum(JNIEnv * env, jobject obj_this, jstring name);
void __cdecl I_PipeConnection_create_Init(JNIEnv * env, jobject obj_this, jstring name);
void __cdecl I_PipeConnection_close_Dum(JNIEnv * env, jobject obj_this);
void __cdecl I_PipeConnection_close_Lib(JNIEnv * env, jobject obj_this);
jint __cdecl I_PipeConnection_read(JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len);
void __cdecl I_PipeConnection_write(JNIEnv * env, jobject obj_this, jbyteArray buffer);
void __cdecl I_PipeConnection_flush(JNIEnv * env, jobject obj_this);


// Set the function pointer to the internal function, in this case not need a test for the right pointer
//  that is the fastest way, for runtime
static pc_Create PipeConnection_create     = I_PipeConnection_create_Init;
static pc_Close  PipeConnection_close      = I_PipeConnection_close_Dum;
static pc_Close  PipeConnection_close_DLL  = I_PipeConnection_close_Dum;
static pc_Read   PipeConnection_read       = I_PipeConnection_read;
static pc_Write  PipeConnection_write      = I_PipeConnection_write;
static pc_Flush  PipeConnection_flush      = I_PipeConnection_flush;



// In this is not recommended to call any other WinAPI's
// see http://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx  in section Remarks
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH)
        hInstDLL = hinstDLL;

    return TRUE;
}




// when the ""I_PipeConnection_create_Init" can not load the other DLL set the "PipeConnection_create" to this dummy
void __cdecl  I_PipeConnection_create_Dum(JNIEnv * env, jobject obj_this, jstring name)
{
    (void)env;
    (void)obj_this;
    (void)name;
}



// the first call of ""PipeConnection_create" starting here load the DLL and search the function pointer
void __cdecl I_PipeConnection_create_Init(JNIEnv * env, jobject obj_this, jstring name)
{
    #define  MAXPATH  512
    wchar_t  path[MAXPATH];
    DWORD    size;

    // set to the dummy, now
    PipeConnection_create = I_PipeConnection_create_Dum;

    size = GetModuleFileNameW(hInstDLL, path, MAXPATH);
    if (size == 0)
        return;

    path[size - 5] = L'x';           // ...\jpipe.dll -> ...\jpipx.dll


    module = LoadLibraryExW(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (module != NULL)
    {
        FnPtr pfunctionPtr;

        pfunctionPtr                  = (FnPtr) GetProcAddress(module, "PipeConnection_create");
        if(pfunctionPtr != NULL)
            PipeConnection_create     = (pc_Create) pfunctionPtr;

        pfunctionPtr                  = (FnPtr) GetProcAddress(module, "PipeConnection_close");

        if(pfunctionPtr != NULL)
            PipeConnection_close_DLL  = (pc_Close)  pfunctionPtr;
        PipeConnection_close          = I_PipeConnection_close_Lib;   // always call the close library, the DLL is now loaded

        pfunctionPtr                  = (FnPtr) GetProcAddress(module, "PipeConnection_read");
        if(pfunctionPtr != NULL)
            PipeConnection_read       = (pc_Read)   pfunctionPtr;

        pfunctionPtr                  = (FnPtr) GetProcAddress(module, "PipeConnection_write");
        if(pfunctionPtr != NULL)
            PipeConnection_write      = (pc_Write)  pfunctionPtr;

        pfunctionPtr                  = (FnPtr) GetProcAddress(module, "PipeConnection_flush");
        if(pfunctionPtr != NULL)
            PipeConnection_flush      = (pc_Flush)  pfunctionPtr;

        PipeConnection_create(env, obj_this, name);  // call the Create in the DLL or call the dummy-function
    }
}





// Close dummy function
void __cdecl I_PipeConnection_close_Dum(JNIEnv * env, jobject obj_this)
{
    (void)env;
    (void)obj_this;
}


// when the DLL opened then close the DLL (normally), set the function pointer back to the internal function
void __cdecl I_PipeConnection_close_Lib(JNIEnv * env, jobject obj_this)
{
    PipeConnection_close_DLL(env, obj_this);

    PipeConnection_create    = I_PipeConnection_create_Init;
    PipeConnection_close     = I_PipeConnection_close_Dum;
    PipeConnection_close_DLL = I_PipeConnection_close_Dum;
    PipeConnection_read      = I_PipeConnection_read;
    PipeConnection_write     = I_PipeConnection_write;
    PipeConnection_flush     = I_PipeConnection_flush;

    // FreeLibrary(module); // when Start WullMuxBar remotly and close the swriter then hang here !??
}



// internal dummy function for "PipeConnection_read"
jint __cdecl I_PipeConnection_read(JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len)
{
    (void)env;
    (void)obj_this;
    (void)buffer;
    (void)len;

    return -2;
}


// internal dummy function for "PipeConnection_write"
void __cdecl I_PipeConnection_write(JNIEnv * env, jobject obj_this, jbyteArray buffer)
{
    (void)env;
    (void)obj_this;
    (void)buffer;
}


// internal dummy function for "PipeConnection_flush"
void __cdecl I_PipeConnection_flush(JNIEnv * env, jobject obj_this)
{
    (void)env;
    (void)obj_this;
}



SAL_DLLPUBLIC_EXPORT void
Java_com_sun_star_lib_connections_pipe_PipeConnection_createJNI(
    JNIEnv * env, jobject obj_this, jstring name)
{
    PipeConnection_create(env, obj_this, name);
}

SAL_DLLPUBLIC_EXPORT void
Java_com_sun_star_lib_connections_pipe_PipeConnection_closeJNI(
    JNIEnv * env, jobject obj_this)
{
    PipeConnection_close(env, obj_this);
}

SAL_DLLPUBLIC_EXPORT jint
Java_com_sun_star_lib_connections_pipe_PipeConnection_readJNI(
    JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len)
{
    return PipeConnection_read(env, obj_this, buffer, len);
}

SAL_DLLPUBLIC_EXPORT void
Java_com_sun_star_lib_connections_pipe_PipeConnection_writeJNI(
    JNIEnv * env, jobject obj_this, jbyteArray buffer)
{
    PipeConnection_write(env, obj_this, buffer);
}

SAL_DLLPUBLIC_EXPORT void
Java_com_sun_star_lib_connections_pipe_PipeConnection_flushJNI(
    JNIEnv * env, jobject obj_this)
{
    PipeConnection_flush(env, obj_this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
