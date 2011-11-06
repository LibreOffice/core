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



#ifndef CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX
#define CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX

#include "hsqldb/HStorageAccess.h"

namespace connectivity { namespace hsqldb
{
    class DataLogFile;
} }

jint read_from_storage_stream( JNIEnv * env, jobject obj_this, jstring name, jstring key, ::connectivity::hsqldb::DataLogFile* logger = NULL );
jint read_from_storage_stream_into_buffer( JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len, ::connectivity::hsqldb::DataLogFile* logger = NULL );
void write_to_storage_stream_from_buffer( JNIEnv* env, jobject obj_this, jstring name, jstring key, jbyteArray buffer, jint off, jint len, ::connectivity::hsqldb::DataLogFile* logger = NULL );
void write_to_storage_stream( JNIEnv* env, jobject obj_this, jstring name, jstring key, jint v, ::connectivity::hsqldb::DataLogFile* logger = NULL );

#endif // CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX
