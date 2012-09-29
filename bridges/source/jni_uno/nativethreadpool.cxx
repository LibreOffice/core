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


#include <string.h>
#include "jvmaccess/virtualmachine.hxx"
#include "rtl/byteseq.h"
#include "rtl/byteseq.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"
#include "uno/threadpool.h"

#include "jni.h"

#include <new>

/* The native implementation part of
 * jurt/com/sun/star/lib/uno/environments/remote/NativeThreadPool.java.
 */

namespace {

struct Pool {
    Pool(rtl::Reference< jvmaccess::VirtualMachine > const & theVirtualMachine,
         jmethodID theExecute, uno_ThreadPool thePool):
        virtualMachine(theVirtualMachine), execute(theExecute), pool(thePool) {}

    rtl::Reference< jvmaccess::VirtualMachine > virtualMachine;
    jmethodID execute;
    uno_ThreadPool pool;
};

struct Job {
    Job(Pool * thePool, jobject theJob): pool(thePool), job(theJob) {}

    Pool * pool;
    jobject job;
};

void throwOutOfMemory(JNIEnv * env) {
    jclass c = env->FindClass("java/lang/OutOfMemoryError");
    if (c != 0) {
        env->ThrowNew(c, "");
    }
}

}

extern "C" {

static void SAL_CALL executeRequest(void * data) {
    Job * job = static_cast< Job * >(data);
    try {
        jvmaccess::VirtualMachine::AttachGuard guard(job->pool->virtualMachine);
        JNIEnv * env = guard.getEnvironment();
        // Failure of the following Job.execute Java call is ignored; if that
        // call fails, it should be due to a java.lang.Error, which is not
        // handled well, anyway:
        env->CallObjectMethod(job->job, job->pool->execute);
        env->DeleteGlobalRef(job->job);
        delete job;
    } catch (const jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
        //TODO: DeleteGlobalRef(job->job)
        delete job;
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT jbyteArray JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_threadId(
    JNIEnv * env, SAL_UNUSED_PARAMETER jclass) SAL_THROW_EXTERN_C()
{
    sal_Sequence * s = 0;
    uno_getIdOfCurrentThread(&s); //TODO: out of memory
    uno_releaseIdFromCurrentThread();
    rtl::ByteSequence seq(s);
    rtl_byte_sequence_release(s);
    sal_Int32 n = seq.getLength();
    jbyteArray a = env->NewByteArray(n);
        // sal_Int32 and jsize are compatible here
    if (a == 0) {
        return 0;
    }
    void * p = env->GetPrimitiveArrayCritical(a, 0);
    if (p == 0) {
        return 0;
    }
    memcpy(p, seq.getConstArray(), n);
        // sal_Int8 and jbyte ought to be compatible
    env->ReleasePrimitiveArrayCritical(a, p, 0);
    return a;
}

extern "C" SAL_DLLPUBLIC_EXPORT jlong JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_create(
    JNIEnv * env, SAL_UNUSED_PARAMETER jclass) SAL_THROW_EXTERN_C()
{
    JavaVM * vm;
    if (env->GetJavaVM(&vm) != JNI_OK) { //TODO: no Java exception raised?
        jclass c = env->FindClass("java/lang/RuntimeException");
        if (c != 0) {
            env->ThrowNew(c, "JNI GetJavaVM failed");
        }
        return 0;
    }
    jclass c = env->FindClass("com/sun/star/lib/uno/environments/remote/Job");
    if (c == 0) {
        return 0;
    }
    jmethodID execute = env->GetMethodID(c, "execute", "()Ljava/lang/Object;");
    if (execute == 0) {
        return 0;
    }
    try {
        return reinterpret_cast< jlong >(new Pool(
            new jvmaccess::VirtualMachine(vm, env->GetVersion(), false, env),
            execute, uno_threadpool_create()));
    } catch (const std::bad_alloc &) {
        throwOutOfMemory(env);
        return 0;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_attach(
    SAL_UNUSED_PARAMETER JNIEnv *, SAL_UNUSED_PARAMETER jclass, jlong pool)
    SAL_THROW_EXTERN_C()
{
    uno_threadpool_attach(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" SAL_DLLPUBLIC_EXPORT jobject JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_enter(
    JNIEnv * env, SAL_UNUSED_PARAMETER jclass, jlong pool) SAL_THROW_EXTERN_C()
{
    jobject job;
    uno_threadpool_enter(
        reinterpret_cast< Pool * >(pool)->pool,
        reinterpret_cast< void ** >(&job));
    if (job == 0) {
        return 0;
    }
    jobject ref = env->NewLocalRef(job);
    env->DeleteGlobalRef(job);
    return ref;
}

extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_detach(
    SAL_UNUSED_PARAMETER JNIEnv *, SAL_UNUSED_PARAMETER jclass, jlong pool)
    SAL_THROW_EXTERN_C()
{
    uno_threadpool_detach(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_putJob(
    JNIEnv * env, SAL_UNUSED_PARAMETER jclass, jlong pool, jbyteArray threadId,
    jobject job, jboolean request, jboolean oneWay) SAL_THROW_EXTERN_C()
{
    void * s = env->GetPrimitiveArrayCritical(threadId, 0);
    if (s == 0) {
        return;
    }
    rtl::ByteSequence seq(
        static_cast< sal_Int8 * >(s), env->GetArrayLength(threadId));
        // sal_Int8 and jbyte ought to be compatible; sal_Int32 and jsize are
        // compatible here
        //TODO: out of memory
    env->ReleasePrimitiveArrayCritical(threadId, s, JNI_ABORT);
    Pool * p = reinterpret_cast< Pool * >(pool);
    jobject ref = env->NewGlobalRef(job);
    if (ref == 0) {
        return;
    }
    Job * j = 0;
    if (request) {
        j = new(std::nothrow) Job(p, ref);
        if (j == 0) {
            env->DeleteGlobalRef(ref);
            throwOutOfMemory(env);
            return;
        }
    }
    uno_threadpool_putJob(
        p->pool, seq.getHandle(),
        request ? static_cast< void * >(j) : static_cast< void * >(ref),
        request ? executeRequest : 0, oneWay);
}

extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_dispose(
    SAL_UNUSED_PARAMETER JNIEnv *, SAL_UNUSED_PARAMETER jclass, jlong pool)
    SAL_THROW_EXTERN_C()
{
    uno_threadpool_dispose(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" SAL_DLLPUBLIC_EXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_destroy(
    SAL_UNUSED_PARAMETER JNIEnv *, SAL_UNUSED_PARAMETER jclass, jlong pool)
    SAL_THROW_EXTERN_C()
{
    Pool * p = reinterpret_cast< Pool * >(pool);
    uno_threadpool_destroy(p->pool);
    delete p;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
