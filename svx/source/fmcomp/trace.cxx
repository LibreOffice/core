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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "trace.hxx"
#include <tools/debug.hxx>

#if defined(DBG_UTIL)

//==============================================================================

//------------------------------------------------------------------------------
::vos::OMutex Tracer::s_aMapSafety;
::std::map< ::vos::OThread::TThreadIdentifier, sal_Int32, ::std::less< ::vos::OThread::TThreadIdentifier > >
        Tracer::s_aThreadIndents;

//------------------------------------------------------------------------------
Tracer::Tracer(const char* _pBlockDescription)
    :m_sBlockDescription(_pBlockDescription)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ]++;

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (sal_Int32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += " {";
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
Tracer::~Tracer()
{
    ::vos::OGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = --s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (sal_Int32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += "} // ";
    sMessage += m_sBlockDescription;
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
void Tracer::TraceString(const char* _pMessage)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (sal_Int32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
void Tracer::TraceString1StringParam(const char* _pMessage, const char* _pParam)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (sal_Int32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE1(sMessage.GetBuffer(), _pParam);
}
#endif
