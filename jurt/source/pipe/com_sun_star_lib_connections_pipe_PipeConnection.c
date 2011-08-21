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

#include "jni.h"
#include "osl/security.h"
#include <osl/pipe.h>

/* On Windows, jpipe.dll must not have dependencies on any other URE DLLs, as
   Java System.LoadLibrary could otherwise not load it.  Therefore, on Windows,
   this code goes into a jpipx.dll that the jpipe.dll wrapper loads with
   LoadLibraryEx(LOAD_WITH_ALTERED_SEARCH_PATH).  The function names in this
   wrapped code are truncated from the long JNICALL names, as JNICALL causes
   some "@N" with different numeric values for N (and probably different across
   32 and 64 bit) to be added to the symbol names, which the calls to
   GetProcAddress in wrapper/wrapper.c would otheriwse have to take into
   account.
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
#if defined WNT
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
        case GOTNAME:
            rtl_uString_release(pname);
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
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
#if defined WNT
PipeConnection_close
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_closeJNI
#endif
  (JNIEnv * env, jobject obj_this)
{
    enum {
        START   = 0,
        INMONITOR
    };

    short       state   = START;
    oslPipe     npipe;      /* native pipe */
    jclass      tclass;     /* this class */
    jfieldID    fid;        /* a field identifier */

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
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto error;
    }

    /* remove the reference to the pipe */
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

    (*env)->SetLongField(env, obj_this, fid, (jlong)0);

    /* release the pipe */
    osl_closePipe(npipe);
    osl_releasePipe(npipe);

    /* done */
    (*env)->MonitorExit(env, obj_this);
    return;

 error:
    switch (state)
    {
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
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
#if defined WNT
PipeConnection_read
#else
JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_readJNI
#endif
  (JNIEnv * env, jobject obj_this, jobjectArray buffer, jint len)
{
    enum {
        START   = 0,
        INMONITOR,
        AQUIRED,
        GOTBUFFER
    };

    short       state   = START;
    oslPipe     npipe;          /* native pipe */
    void *      nbuff = NULL;   /* native read buffer */
    jbyteArray  bytes;          /* java read buffer */
    jint        nread;          /* number of bytes has been read */

    /* enter monitor */
    if ((*env)->MonitorEnter(env, obj_this) != 0)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe cannot synchronize on the object");
        goto error;
    }
    state = INMONITOR;

    /* check connection state */
    npipe   = getPipe(env, obj_this);
    if ((*env)->ExceptionOccurred(env) != NULL)
        goto error;
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto error;
    }

    /* aquire pipe */
    osl_acquirePipe( npipe );
    state = AQUIRED;

    /* allocate a buffer */
    if ((nbuff = malloc(len)) == NULL)
    {
        ThrowException(env,
                       "java/lang/RuntimeException",
                       "native pipe out of memory");
        goto error;
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
        goto error;
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
            goto error;
        }

        /* save the data */
        (*env)->SetByteArrayRegion(env, bytes, 0, len, nbuff);
        (*env)->SetObjectArrayElement(env, buffer, 0, bytes);
        (*env)->DeleteLocalRef(env, bytes);
    }

    /* done */
    free(nbuff);
    if ( state >= AQUIRED )
        osl_releasePipe( npipe );

    /* exit monitor */
    (*env)->MonitorExit(env, obj_this);
    return nread;

 error:
    switch (state)
    {
        case GOTBUFFER:
            free(nbuff);
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
        case START:
        default:
            break;
    }
    return -1;
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_lib_connections_pipe_PipeConnection
 * Method:    writeJNI
 * Signature: ([B)V
 */
SAL_DLLPUBLIC_EXPORT void
#if defined WNT
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
    long    count;          /* number of bytes has been written */
    jsize   nwrite;         /* number of bytes to write */
    jbyte * nbuff = NULL;   /* native buffer */

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
    if (npipe == NULL)
    {
        ThrowException(env,
                       "com/sun/star/io/IOException",
                       "native pipe is not connected");
        goto error;
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
            goto error;
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
            goto error;
        }
        if (count != nwrite)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe is failed to write");
            goto error;
        }
    }
    /* done */
    (*env)->ReleaseByteArrayElements(env, buffer, nbuff, JNI_ABORT);
    (*env)->MonitorExit(env, obj_this);
    return;

 error:
    switch (state)
    {
        case GOTBUFFER:
            (*env)->ReleaseByteArrayElements(env, buffer, nbuff, JNI_ABORT);
        case INMONITOR:
            (*env)->MonitorExit(env, obj_this);
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
#if defined WNT
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
