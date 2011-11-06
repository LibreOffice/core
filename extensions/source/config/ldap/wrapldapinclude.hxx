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



#ifdef WNT
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif

#ifdef WITH_OPENLDAP
#include <ldap.h>
#ifndef LDAP_API
#    define LDAP_API(rt) rt
#endif
#ifndef LDAP_CALL
#    define LDAP_CALL
#endif
#else
#ifndef LDAP_INCLUDED
#define LDAP_INCLUDED
#include <ldap/ldap.h>
#endif // LDAP_INCLUDED
#endif

#ifdef WNT
#pragma warning (pop)
#endif // WNT
