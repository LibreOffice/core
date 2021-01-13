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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HSTORAGEACCESS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HSTORAGEACCESS_HXX

#include <sal/config.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

namespace connectivity::hsqldb
{
    class DataLogFile;
}

jint read_from_storage_stream( JNIEnv * env, jstring name, jstring key );
jint read_from_storage_stream_into_buffer( JNIEnv * env, jstring name, jstring key, jbyteArray buffer, jint off, jint len );
void write_to_storage_stream_from_buffer( JNIEnv* env, jstring name, jstring key, jbyteArray buffer, jint off, jint len );
void write_to_storage_stream( JNIEnv* env, jstring name, jstring key, jint v );

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HSTORAGEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
