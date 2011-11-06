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



#ifndef _NEONTYPES_HXX_
#define _NEONTYPES_HXX_

#include <ne_session.h>
#include <ne_utils.h>
#include <ne_basic.h>
#include <ne_props.h>
#include <ne_locks.h>

typedef ne_session                  HttpSession;
typedef ne_status                   HttpStatus;
typedef ne_server_capabilities      HttpServerCapabilities;

typedef ne_propname                 NeonPropName;
typedef ne_prop_result_set          NeonPropFindResultSet;

typedef struct ne_lock              NeonLock;

#endif // _NEONTYPES_HXX_
