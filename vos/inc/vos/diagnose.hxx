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




#ifndef _VOS_DIAGNOSE_H_
#define _VOS_DIAGNOSE_H_


#ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
#endif


/*
    Diagnostic support
*/

#define VOS_THIS_FILE       __FILE__

#define VOS_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define VOS_TRACE           _OSL_TRACE
#define VOS_ASSERT(c)       _OSL_ASSERT(c, VOS_THIS_FILE, __LINE__)
#define VOS_VERIFY(c)       OSL_VERIFY(c)
#define VOS_ENSHURE(c, m)   _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)
#define VOS_ENSURE(c, m)    _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)

#define VOS_PRECOND(c, m)   VOS_ENSHURE(c, m)
#define VOS_POSTCOND(c, m)  VOS_ENSHURE(c, m)

#endif /* _VOS_DIAGNOSE_H_ */


