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

#ifndef INCLUDED_JAVAUNOHELPER_SOURCE_JUHX_EXPORT_TYPES_HXX
#define INCLUDED_JAVAUNOHELPER_SOURCE_JUHX_EXPORT_TYPES_HXX

#include "sal/config.h"

#include "jni.h"

#if defined DISABLE_DYNLOADING
#define JAVAUNOHELPER_DETAIL_CALLCONV JNICALL
#else
#define JAVAUNOHELPER_DETAIL_CALLCONV
#endif

extern "C" {

namespace javaunohelper { namespace detail {

typedef jobject JAVAUNOHELPER_DETAIL_CALLCONV Func_bootstrap(
    JNIEnv *_env, jclass, jstring, jobjectArray, jobject);

typedef jobject JAVAUNOHELPER_DETAIL_CALLCONV Func_getFactory(
    JNIEnv *, jclass, jstring, jstring, jobject, jobject, jobject);

typedef jboolean JAVAUNOHELPER_DETAIL_CALLCONV Func_writeInfo(
    JNIEnv *, jclass, jstring, jobject, jobject, jobject);

} }

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
