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


#ifndef _TRACE_HXX_
#define _TRACE_HXX_

#if defined(DBG_UTIL)

#include <tools/string.hxx>
#include <vos/thread.hxx>
#include <vos/mutex.hxx>
#include <comphelper/stl_types.hxx>


class Tracer
{
    ByteString m_sBlockDescription;

    DECLARE_STL_STDKEY_MAP( ::vos::OThread::TThreadIdentifier, sal_Int32, MapThreadId2Int );
    static MapThreadId2Int s_aThreadIndents;

    static ::vos::OMutex    s_aMapSafety;

public:
    Tracer(const char* _pBlockDescription);
    ~Tracer();

    void TraceString(const char* _pMessage);
    void TraceString1StringParam(const char* _pMessage, const char* _pParam);
};


#define TRACE_RANGE(range_description)          Tracer aTrace(range_description);
#define TRACE_RANGE_MESSAGE(message)            { aTrace.TraceString(message); }
#define TRACE_RANGE_MESSAGE1(message, param)    { aTrace.TraceString1StringParam(message, param); }

#else

#define TRACE_RANGE(range_description)          ;
#define TRACE_RANGE_MESSAGE(message)            ;
#define TRACE_RANGE_MESSAGE1(message, param)    ;

#endif

#endif // _TRACE_HXX_
