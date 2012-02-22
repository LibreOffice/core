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

#ifndef _RTL_PROCESS_CONST_H_
#define _RTL_PROCESS_CONST_H_

//------------------------------------------------------------------------
#include <rtl/ustring.hxx>

using namespace ::rtl;

#ifdef __cplusplus
extern "C"
{
#endif
//------------------------------------------------------------------------
//::rtl::OUString suParam[4];
::rtl::OUString suParam0 = ::rtl::OUString::createFromAscii("-join");
::rtl::OUString suParam1 = OUString::createFromAscii("-with");
::rtl::OUString suParam2 = OUString::createFromAscii("-child");
::rtl::OUString suParam3 = OUString::createFromAscii("-process");

//------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif /* RTL_PROCESS_CONST_H*/
