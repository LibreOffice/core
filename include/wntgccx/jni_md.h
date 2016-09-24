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

/* Fake jni_md.h for use when cross-compiling to Windows */

#ifndef INCLUDED_WNTGCCX_JNI_MD_H
#define INCLUDED_WNTGCCX_JNI_MD_H

#define JNIEXPORT __declspec (dllexport)
#define JNIIMPORT __declspec (dllimport)
#define JNICALL

typedef long jint;
typedef __int64 jlong;
typedef char jbyte;

#endif // INCLUDED_WNTGCCX_JNI_MD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
