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



#ifndef INCLUDED_SLIDESHOW_DEBUG_HXX
#define INCLUDED_SLIDESHOW_DEBUG_HXX

#include <osl/diagnose.h>

#if OSL_DEBUG_LEVEL > 1

#include "animationnode.hxx"


namespace slideshow { namespace internal {

void Debug_ShowNodeTree (const AnimationNodeSharedPtr& rpNode);

// Change this to a valid filename.  The file is created anew with every
// office start (and execution of at least one TRACE... command.)
#define TRACE_LOG_FILE_NAME "d:\\tmp\\log.txt"

class DebugTraceScope
{
public:
    DebugTraceScope (const sal_Char* sFormat, ...);
    ~DebugTraceScope (void);
private:
    static const sal_Int32 mnBufferSize = 512;
    sal_Char* msMessage;
};

void SAL_CALL DebugTraceBegin (const sal_Char* sFormat, ...);
void SAL_CALL DebugTraceEnd (const sal_Char* sFormat, ...);
void SAL_CALL DebugTraceMessage (const sal_Char* sFormat, ...);

} } // end of namespace ::slideshow::internal


#define TRACE_BEGIN DebugTraceBegin
#define TRACE_END   DebugTraceEnd
#define TRACE       DebugTraceMessage
#define TRACE_SCOPE DebugTraceScope aTraceScope

#else // OSL_DEBUG_LEVEL > 1

#define TRACE_BEGIN 1 ? ((void)0) : DebugTraceBegin
#define TRACE_END   1 ? ((void)0) : DebugTraceEnd
#define TRACE       1 ? ((void)0) : DebugTraceMessage
#define TRACE_SCOPE

#endif // OSL_DEBUG_LEVEL > 1

#endif
