/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: com_sun_star_lib_connections_pipe_PipeConnection.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:08:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "osl/security.h"
#include <osl/pipe.h>

#include "com_sun_star_lib_connections_pipe_PipeConnection.h"

/*****************************************************************************/
/* exception macros */

static void ThrowException(JNIEnv * env, char const * type, char const * msg) {
    jclass c;
    (*env)->ExceptionClear(env);
    c = (*env)->FindClass(env, "java/lang/RuntimeException");
    if (c == NULL) {
        (*env)->ExceptionClear(env);
        (*env)->FatalError(
            env, "JNI FindClass(\"java/lang/RuntimeException\") failed");
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
    while (1)
    {
        tclass  = (*env)->GetObjectClass(env, obj_this);
        if (tclass == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find class");
            break;
        }

        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        if (fid == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find field");
            break;
        }
        return ((oslPipe)((*env)->GetLongField(env, obj_this, fid)));
    }
    return NULL;
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
JNIEXPORT void JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_createJNI
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
    while (1)
    {
        if ((*env)->MonitorEnter(env, obj_this) != 0)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot synchronize on the object");
            break;
        }
        state   = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        if ((*env)->ExceptionOccurred(env) != NULL)
            break;
        if (npipe != NULL)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe is already connected");
            break;
        }

        /* save the pipe name */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        if (tclass == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find class");
            break;
        }

        fid     = (*env)->GetFieldID(env, tclass,
            "_aDescription", "Ljava/lang/String;");
        if (fid == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find field");
            break;
        }

        (*env)->SetObjectField(env, obj_this, fid, (jobject)name);

        /* convert pipe name to rtl_uString */
        pname   = jstring2ustring(env, name);
        if (pname == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot convert name");
            break;
        }
        state   = GOTNAME;

        /* try to connect */
        npipe   = osl_createPipe(pname, osl_Pipe_OPEN, psec);
        if (npipe == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "cannot create native pipe");
            break;
        }
        state   = CREATED;

        /* save the pipe */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        if (tclass == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find class");
            break;
        }

        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        if (fid == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find field");
            break;
        }
        (*env)->SetLongField(env, obj_this, fid, (jlong)npipe);

        /* done */
        rtl_uString_release(pname);
        (*env)->MonitorExit(env, obj_this);
        osl_freeSecurityHandle(psec);
        return;
    }
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
JNIEXPORT void JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_closeJNI
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

    while (1)
    {
        if ((*env)->MonitorEnter(env, obj_this) != 0)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot synchronize on the object");
            break;
        }
        state   = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        if ((*env)->ExceptionOccurred(env) != NULL)
            break;
        if (npipe == NULL)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe is not connected");
            break;
        }

        /* remove the reference to the pipe */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        if (tclass == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find class");
            break;
        }

        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        if (fid == NULL)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot find field");
            break;
        }

        (*env)->SetLongField(env, obj_this, fid, (jlong)0);

        /* release the pipe */
        osl_closePipe(npipe);
        osl_releasePipe(npipe);

        /* done */
        (*env)->MonitorExit(env, obj_this);
        return;
    }
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
JNIEXPORT jint JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_readJNI
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

    while (1)
    {
        /* enter monitor */
        if ((*env)->MonitorEnter(env, obj_this) != 0)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot synchronize on the object");
            break;
        }
        state = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        if ((*env)->ExceptionOccurred(env) != NULL)
            break;
        if (npipe == NULL)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe is not connected");
            break;
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
            break;
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
            break;
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
                break;
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
    }
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
JNIEXPORT void JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_writeJNI
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

    while (1)
    {
        if ((*env)->MonitorEnter(env, obj_this) != 0)
        {
            ThrowException(env,
                           "java/lang/RuntimeException",
                           "native pipe cannot synchronize on the object");
            break;
        }
        state   = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        if ((*env)->ExceptionOccurred(env) != NULL)
            break;
        if (npipe == NULL)
        {
            ThrowException(env,
                           "com/sun/star/io/IOException",
                           "native pipe is not connected");
            break;
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
                break;
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
                break;
            }
            if (count != nwrite)
            {
                ThrowException(env,
                               "com/sun/star/io/IOException",
                               "native pipe is failed to write");
                break;
            }
        }
        /* done */
        (*env)->ReleaseByteArrayElements(env, buffer, nbuff, JNI_ABORT);
        (*env)->MonitorExit(env, obj_this);
        return;
    }
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
JNIEXPORT void JNICALL Java_com_sun_star_lib_connections_pipe_PipeConnection_flushJNI
  (JNIEnv * env, jobject obj_this)
{
    return;
}
