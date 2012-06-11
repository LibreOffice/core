/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jni.h>

#include <sal/ByteBufferWrapper.hxx>

using org::libreoffice::touch::ByteBufferWrapper;

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
