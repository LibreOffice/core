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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges_java_uno.hxx"

#include "jvmaccess/virtualmachine.hxx"
#include "rtl/byteseq.h"
#include "rtl/byteseq.hxx"
#include "rtl/memory.h"
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
    } catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &) {
        //TODO: DeleteGlobalRef(job->job)
        delete job;
    }
}

}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_threadId(
    JNIEnv * env, jclass) SAL_THROW_EXTERN_C()
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
    rtl_copyMemory(p, seq.getConstArray(), n);
        // sal_Int8 and jbyte ought to be compatible
    env->ReleasePrimitiveArrayCritical(a, p, 0);
    return a;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_create(
    JNIEnv * env, jclass) SAL_THROW_EXTERN_C()
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
    } catch (std::bad_alloc) {
        throwOutOfMemory(env);
        return 0;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_attach(
    JNIEnv *, jclass, jlong pool) SAL_THROW_EXTERN_C()
{
    uno_threadpool_attach(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_enter(
    JNIEnv * env, jclass, jlong pool) SAL_THROW_EXTERN_C()
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

extern "C" JNIEXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_detach(
    JNIEnv *, jclass, jlong pool) SAL_THROW_EXTERN_C()
{
    uno_threadpool_detach(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_putJob(
    JNIEnv * env, jclass, jlong pool, jbyteArray threadId, jobject job,
    jboolean request, jboolean oneWay) SAL_THROW_EXTERN_C()
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

extern "C" JNIEXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_dispose(
    JNIEnv *, jclass, jlong pool) SAL_THROW_EXTERN_C()
{
    uno_threadpool_dispose(reinterpret_cast< Pool * >(pool)->pool);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_destroy(
    JNIEnv *, jclass, jlong pool) SAL_THROW_EXTERN_C()
{
    Pool * p = reinterpret_cast< Pool * >(pool);
    uno_threadpool_destroy(p->pool);
    delete p;
}
