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



#ifndef INCLUDED_registry_version_h
#define INCLUDED_registry_version_h

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @HTML

/**
   The version of a binary blob that represents a UNOIDL type.

   <p>All values between <code>TYPEREG_VERSION_0</code> and
   <code>TYPEREG_MAX_VERSION</code> are valid, where currently unallocated
   values represent future versions.  Negative values are not valid.</p>

   @see typereg::Reader
   @see typereg::Writer

   @since UDK 3.2.0
 */
enum typereg_Version {
    /**
       Denotes the original version of UNOIDL type blobs.
     */
    TYPEREG_VERSION_0,

    /**
       Denotes the updated version of UNOIDL type blobs.

       <p>This version added support for multiple-inheritance interface types,
       extended interface type attributes, single-interface&ndash;based
       services, interface-based singletons, polymorphic struct types, and
       published entities.</p>
     */
    TYPEREG_VERSION_1,

    /**
       Denotes the maximum future version of UNOIDL type blobs.
     */
    TYPEREG_MAX_VERSION = SAL_MAX_INT32
};

#ifdef __cplusplus
}
#endif

#endif
