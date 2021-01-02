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

#include <sal/config.h>

#include <stdlib.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <osl/security.h>
#include <osl/pipe.h>

/* On Windows, jpipe.dll must not have static dependencies on any other URE DLLs
   (sal3.dll, uwinapi.dll), as Java System.LoadLibrary could otherwise not load
   it.  Therefore, on Windows, this code goes into a jpipx.dll that the jpipe.dll
   wrapper loads with LoadLibraryEx(LOAD_WITH_ALTERED_SEARCH_PATH).
   The function names in this wrapped code are truncated from the long JNICALL
   names, as JNICALL causes some "@N" with different numeric values for
   N (and probably different across 32 and 64 bit) to be added to the symbol
   names, which the calls to GetProcAddress in wrapper/wrapper.c would otherwise
   have to take into account.
*/

/*****************************************************************************/
/* exception macros */

static void ThrowException(JNIEnv * env, char const * type, char const * msg) {
    jclass c;
    (*env)->ExceptionClear(env);
    c = (*env)->FindClass(env, type);
    if (c == NULL) {
        (*env)->ExceptionClear(env);
        (*env)->FatalError(env, "JNI FindClass failed");
    }
    if ((*env)->ThrowNew(env, c, msg) != 0) {
        (*env)->ExceptionClear(env);
        (*env)->FatalError(env, "JNI ThrowNew failed");
    }
}

/*****************************************************************************/
/* helper functions prototypes */

static oslPipe getPipe(JNIEnv * env, jobject obj_this);
static rtl_uString * jstring2ustring(JNIEnv * env, jstring jstr);

/*****************************************************************************/
/* get pipe */

static oslPipe getPipe(JNIEnv * env, jobject obj_this)
{
    jclass      tclass;
    jfieldID    fid;
    tclass  = (*env)->GetObjectClass(env, obj_this);
    if (tclass == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find class");
        return NULL;
    }

    fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
    if (fid == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find field");
        return NULL;
    }
    return (oslPipe) SAL_INT_CAST(
        sal_IntPtr, (*env)->GetLongField(env, obj_this, fid));
}

/*****************************************************************************/
/* convert jstring to rtl_uString */

static rtl_uString * jstring2ustring(JNIEnv * env, jstring jstr)
{
    const char *    cstr;
    rtl_uString *   ustr    = NULL;
    cstr    = (*env)->GetStringUTFChars(env, jstr, NULL);
    rtl_uString_newFromAscii(&ustr, cstr);
    (*env)->ReleaseStringUTFChars(env, jstr, cstr);
    return ustr;
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    connect
 * Signature: (Lcom/sun/star/beans/NativeService;)V
 */
SAL_DLLPUBLIC_EXPORT void
#if defined(_WIN32)
PipeConnection_create
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_createJNI
#endif
  (JNIEnv * env, jobject obj_this, jstring name)
{
    enum {
        START   = 0,
        INMONITOR,
        GOTNAME,
        CREATED
    };

    short       state   = START;

    jclass      tclass;
    jfieldID    fid;

    oslSecurity     psec    = osl_getCurrentSecurity();
    oslPipe         npipe   = NULL;
    rtl_uString *   pname   = NULL;
    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto error;
    }
    state   = INMONITOR;

    /* check connection state */
    npipe   = getPipe(env, obj_this);
    if ((*env)->ExceptionOccurred(env) != NULL)
        goto error;
    if (npipe != NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is already connected");
        goto error;
    }

    /* save the pipe name */
    tclass  = (*env)->GetObjectClass(env, obj_this);
    if (tclass == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find class");
        goto error;
    }

    fid     = (*env)->GetFieldID(env, tclass,
                                 "_aDescription", "Ljava/lang/String;");
    if (fid == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find field");
        goto error;
    }

    (*env)->SetObjectField(env, obj_this, fid, (jobject)name);

    /* convert pipe name to rtl_uString */
    pname   = jstring2ustring(env, name);
    if (pname == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot convert name");
        goto error;
    }
    state   = GOTNAME;

    /* try to connect */
    npipe   = osl_createPipe(pname, osl_Pipe_OPEN, psec);
    if (npipe == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "cannot create native pipe");
        goto error;
    }
    state   = CREATED;

    /* save the pipe */
    tclass  = (*env)->GetObjectClass(env, obj_this);
    if (tclass == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find class");
        goto error;
    }

    fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
    if (fid == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find field");
        goto error;
    }
    (*env)->SetLongField(
        env, obj_this, fid, SAL_INT_CAST(jlong, (sal_IntPtr) npipe));

    /* done */
    rtl_uString_release(pname);
    (*env)->MonitorExit(env, obj_this);
    osl_freeSecurityHandle(psec);
    return;

 error:
    switch (state)
    {
        case CREATED:
            osl_closePipe(npipe);
            osl_releasePipe(npipe);
            /* fall through */
        case GOTNAME:
            rtl_uString_release(pname);
            /* fall through */
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
            /* fall through */
        case START:
            osl_freeSecurityHandle(psec);
        default:
            break;
    }
    return;
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    closeJNI
 * Signature: ()V
 */
SAL_DLLPUBLIC_EXPORT void
#if defined(_WIN32)
PipeConnection_close
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_closeJNI
#endif
  (JNIEnv * env, jobject obj_this)
{
    enum {
        START   = 0,
        INMONITOR,
        GOTPIPE,
    };

    short     state = START;
    oslPipe   npipe = NULL;  /* native pipe */
    jclass    tclass;        /* this class */
    jfieldID  fid;           /* a field identifier */

    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto cleanup;
    }
    state   = INMONITOR;

    /* check connection state */
    npipe   = getPipe(env, obj_this);
    if ((*env)->ExceptionOccurred(env) != NULL)
        goto cleanup;
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto cleanup;
    }
    state = GOTPIPE;

    /* remove the reference to the pipe */
    tclass  = (*env)->GetObjectClass(env, obj_this);
    if (tclass == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find class");
        goto cleanup;
    }

    fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
    if (fid == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot find field");
        goto cleanup;
    }

    (*env)->SetLongField(env, obj_this, fid, (jlong)0);

    /* done */

 cleanup:
    switch (state)
    {
        case GOTPIPE:
            /* release the pipe */
            osl_closePipe(npipe);
            osl_releasePipe(npipe);
            /* fall-through */
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
            /* fall-through */
        case START:
        default:
            break;
    }
    return;
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    readJNI
 * Signature: ([[BI)I
 */
SAL_DLLPUBLIC_EXPORT jint
#if defined(_WIN32)
PipeConnection_read
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_readJNI
#endif
  (JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len)
{
    enum {
        START   = 0,
        INMONITOR,
        ACQUIRED,
        GOTBUFFER
    };

    short       state   = START;
    oslPipe     npipe = NULL;   /* native pipe */
    void *      nbuff = NULL;   /* native read buffer */
    jbyteArray  bytes;          /* java read buffer */
    jint        nread;          /* number of bytes has been read */
    jint        nreturn = -1;   /* actual return value */

    /* enter monitor */
    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto cleanup;
    }
    state = INMONITOR;

    /* check connection state */
    npipe   = getPipe(env, obj_this);
    if ((*env)->ExceptionOccurred(env) != NULL)
        goto cleanup;
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto cleanup;
    }

    /* acquire pipe */
    osl_acquirePipe( npipe );
    state = ACQUIRED;

    /* allocate a buffer */
    if ((nbuff = malloc(len)) == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe out of memory");
        goto cleanup;
    }

    state = GOTBUFFER;

    /* exit monitor */
    (*env)->MonitorExit(env, obj_this);

    /* reading */
    nread = osl_readPipe(npipe, nbuff, len);

    /* enter monitor again */
    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto cleanup;
    }

    /* copy buffer */
    if (nread >= 0)
    {
        bytes   = (*env)->NewByteArray(env, len);
        if (bytes == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe out of memory");
            goto cleanup;
        }

        /* save the data */
        (*env)->SetByteArrayRegion(env, bytes, 0, len, nbuff);
        (*env)->SetObjectArrayElement(env, buffer, 0, bytes);
        (*env)->DeleteLocalRef(env, bytes);
    }

    /* done */
    nreturn = nread;

 cleanup:
    switch (state)
    {
        case GOTBUFFER:
            free(nbuff);
            /* fall-through */
        case ACQUIRED:
            osl_releasePipe(npipe);
            /* fall-through */
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
            /* fall-through */
        case START:
        default:
            break;
    }
    return nreturn;
 }

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    writeJNI
 * Signature: ([B)V
 */
SAL_DLLPUBLIC_EXPORT void
#if defined(_WIN32)
PipeConnection_write
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_writeJNI
#endif
  (JNIEnv * env, jobject obj_this, jbyteArray buffer)
{
    enum {
        START   = 0,
        INMONITOR,
        GOTBUFFER
    };

    short   state   = START;
    oslPipe npipe;          /* native pipe */
    sal_Int32 count;        /* number of bytes has been written */
    jsize   nwrite;         /* number of bytes to write */
    jbyte * nbuff = NULL;   /* native buffer */

    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto cleanup;
    }
    state   = INMONITOR;

    /* check connection state */
    npipe   = getPipe(env, obj_this);
    if ((*env)->ExceptionOccurred(env) != NULL)
        goto cleanup;
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto cleanup;
    }

    nwrite  = (*env)->GetArrayLength(env, buffer);
    if (nwrite > 0)
    {
        nbuff   = (*env)->GetByteArrayElements(env, buffer, NULL);
        if (nbuff == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe out of memory");
            goto cleanup;
        }
        state   = GOTBUFFER;

        (*env)->MonitorExit(env, obj_this);
        /* writing */
        count   = osl_writePipe(npipe, nbuff, nwrite);
        if ((*env)->MonitorEnter(env, obj_this) != 0)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot synchronize on the object");
            goto cleanup;
        }
        if (count != nwrite)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe: failed to write");
            goto cleanup;
        }
    }
    /* done */

 cleanup:
    switch (state)
    {
        case GOTBUFFER:
            (*env)->ReleaseByteArrayElements(env, buffer, nbuff, JNI_ABORT);
            /* fall through */
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
            /* fall through */
        case START:
        default:
            break;
    }
    return;
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    flushJNI
 * Signature: ()V
 */
SAL_DLLPUBLIC_EXPORT void
#if defined(_WIN32)
PipeConnection_flush
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_flushJNI
#endif
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* not used */
    (void) obj_this; /* not used */
    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
