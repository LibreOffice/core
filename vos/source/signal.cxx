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




#include <vos/diagnose.hxx>
#include <vos/object.hxx>
#include <vos/signal.hxx>

using namespace vos;

oslSignalAction vos::signalHandlerFunction_impl(
    void * pthis, oslSignalInfo * pInfo)
{
    vos::OSignalHandler* pThis= (vos::OSignalHandler*)pthis;

    return ((oslSignalAction)pThis->signal(pInfo));
}

/////////////////////////////////////////////////////////////////////////////
// Thread class

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OSignalHandler, vos),
                        VOS_NAMESPACE(OSignalHandler, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

OSignalHandler::OSignalHandler()
{
    m_hHandler = osl_addSignalHandler(signalHandlerFunction_impl, this);
}

OSignalHandler::~OSignalHandler()
{
    osl_removeSignalHandler(m_hHandler);
}

OSignalHandler::TSignalAction OSignalHandler::raise(sal_Int32 Signal, void *pData)
{
    return (TSignalAction)osl_raiseSignal(Signal, pData);
}

