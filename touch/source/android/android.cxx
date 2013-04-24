/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jni.h>

#include <sal/ByteBufferWrapper.hxx>
#include <osl/detail/android-bootstrap.h>

using org::libreoffice::touch::ByteBufferWrapper;

static JNIEnv *get_env()
{
    JavaVMAttachArgs args = {
        JNI_VERSION_1_2,
        NULL,
        NULL
    };

    JavaVM *jvm = lo_get_javavm();
    JNIEnv *env = NULL;

    jvm->AttachCurrentThread(&env, &args);
    return env;
}

__attribute__ ((visibility("default")))
ByteBufferWrapper::ByteBufferWrapper(JNIEnv *env, jobject o)
{
    object = env->NewGlobalRef(o);
}

__attribute__ ((visibility("default")))
sal_uInt8* ByteBufferWrapper::pointer()
{
    return (sal_uInt8 *) get_env()->GetDirectBufferAddress(object);
}

__attribute__ ((visibility("default")))
void ByteBufferWrapper::operator()(sal_uInt8 * /* p */)
{
    get_env()->DeleteGlobalRef(object);
}

extern "C"
__attribute__ ((visibility("default")))
jlong
Java_org_libreoffice_android_Bootstrap_new_1byte_1buffer_1wrapper(JNIEnv *env,
                                                                  jobject /* clazz */,
                                                                  jobject bytebuffer)
{
  return (jlong) (intptr_t) new ByteBufferWrapper(env, bytebuffer);
}

extern "C"
__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_delete_1byte_1buffer_1wrapper(JNIEnv * /* env */,
                                                                     jobject /* clazz */,
                                                                     jlong bbw)
{
  delete (ByteBufferWrapper*) (intptr_t) bbw;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
