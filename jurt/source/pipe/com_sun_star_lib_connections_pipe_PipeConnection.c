/*************************************************************************
 *
 *  $RCSfile: com_sun_star_lib_connections_pipe_PipeConnection.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:33:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <osl/pipe.h>

#include "com_sun_star_lib_connections_pipe_PipeConnection.h"

/*****************************************************************************/
/* exception macros */

#define Throw_IfException(env) { \
    if ((*env)->ExceptionOccurred(env) != NULL) break; }

#define ThrowException(env, type, msg) { \
    (*env)->ThrowNew(env, (*env)->FindClass(env, type), msg); }

#define ThrowIO_If(cond, env, msg) { if (cond) \
    { ThrowException(env, "com/sun/star/io/IOException", msg); break; } }

#define ThrowRT_If(cond, env, msg) { if (cond) \
    { ThrowException(env, "java/lang/RuntimeException", msg); break; } }

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
        ThrowRT_If((tclass == NULL),
            env, "native pipe cannot find class");
        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        ThrowRT_If((fid == NULL),
            env, "native pipe cannot find field");
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

    oslPipe         npipe   = NULL;
    rtl_uString *   pname   = NULL;
    while (1)
    {
        ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
            env, "native pipe cannot synchronize on the object");
        state   = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        Throw_IfException(env);
        ThrowIO_If((npipe != NULL),
            env, "native pipe is already connected");

        /* save the pipe name */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        ThrowRT_If((tclass == NULL),
            env, "native pipe cannot find class");
        fid     = (*env)->GetFieldID(env, tclass,
            "_aDescription", "Ljava/lang/String;");
        ThrowRT_If((fid == NULL),
            env, "native pipe cannot find field");
        (*env)->SetObjectField(env, obj_this, fid, (jobject)name);

        /* convert pipe name to rtl_uString */
        pname   = jstring2ustring(env, name);
        ThrowRT_If((pname == NULL),
            env, "native pipe cannot convert name");
        state   = GOTNAME;

        /* try to connect */
        npipe   = osl_createPipe(pname, osl_Pipe_OPEN, NULL);
        ThrowRT_If((npipe == NULL),
            env, "cannot create native pipe");
        state   = CREATED;

        /* save the pipe */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        ThrowRT_If((tclass == NULL),
            env, "native pipe cannot find class");
        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        ThrowRT_If((fid == NULL),
            env, "native pipe cannot find field");
        (*env)->SetLongField(env, obj_this, fid, (jlong)npipe);

        /* done */
        rtl_uString_release(pname);
        (*env)->MonitorExit(env, obj_this);
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
        ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
            env, "native pipe cannot synchronize on the object");
        state   = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        Throw_IfException(env);
        ThrowIO_If((npipe == NULL),
            env, "native pipe is not connected");

        /* remove the reference to the pipe */
        tclass  = (*env)->GetObjectClass(env, obj_this);
        ThrowRT_If((tclass == NULL),
            env, "native pipe cannot find class");
        fid     = (*env)->GetFieldID(env, tclass, "_nPipeHandle", "J");
        ThrowRT_If((fid == NULL),
            env, "native pipe cannot find field");
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
    oslPipe     npipe;      /* native pipe */
    void *      nbuff;      /* native read buffer */
    jbyteArray  bytes;      /* java read buffer */
    jint        nread;      /* number of bytes has been read */

    while (1)
    {
        /* enter monitor */
        ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
            env, "native pipe cannot synchronize on the object");
        state = INMONITOR;

        /* check connection state */
        npipe   = getPipe(env, obj_this);
        Throw_IfException(env);
        ThrowIO_If((npipe == NULL),
            env, "native pipe is not connected");

        /* aquire pipe */
        osl_acquirePipe( npipe );
        state = AQUIRED;

        /* allocate a buffer */
        ThrowRT_If(((nbuff = malloc(len)) == NULL),
            env, "native pipe out of memory");
        state = GOTBUFFER;

        /* exit monitor */
        (*env)->MonitorExit(env, obj_this);

        /* reading */
        nread = osl_readPipe(npipe, nbuff, len);

        /* enter monitor again */
        ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
            env, "native pipe cannot synchronize on the object");

        /* copy buffer */
        if (nread >= 0)
        {
            bytes   = (*env)->NewByteArray(env, len);
            ThrowRT_If((bytes == NULL),
                env, "native pipe out of memory");
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
    oslPipe npipe;      /* native pipe */
    long    count;      /* number of bytes has been written */
    jsize   nwrite;     /* number of bytes to write */
    jbyte * nbuff;      /* native buffer */

    while (1)
    {
        ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
            env, "native pipe cannot synchronize on the object");
        state   = INMONITOR;
        /* check connection state */
        npipe   = getPipe(env, obj_this);
        Throw_IfException(env);
        ThrowIO_If((npipe == NULL),
            env, "native pipe is not connected");
        nwrite  = (*env)->GetArrayLength(env, buffer);
        if (nwrite > 0)
        {
            nbuff   = (*env)->GetByteArrayElements(env, buffer, NULL);
            ThrowRT_If((nbuff == NULL),
                env, "native pipe out of memory");
            state   = GOTBUFFER;
            (*env)->MonitorExit(env, obj_this);
            /* writing */
            count   = osl_writePipe(npipe, nbuff, nwrite);
            ThrowRT_If(((*env)->MonitorEnter(env, obj_this) != 0),
                env, "native pipe cannot synchronize on the object");
            ThrowIO_If((count != nwrite),
                env, "native pipe is failed to write");
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
