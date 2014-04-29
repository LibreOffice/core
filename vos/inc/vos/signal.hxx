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




#ifndef _VOS_SIGNAL_HXX_
#define _VOS_SIGNAL_HXX_

#   include <vos/types.hxx>
#   include <vos/object.hxx>
#   include <osl/signal.h>

namespace vos
{

extern "C"
typedef oslSignalAction SignalHandlerFunction_impl(void *, oslSignalInfo *);
SignalHandlerFunction_impl signalHandlerFunction_impl;

/** OSignalHandler is an objectoriented interface for signal handlers.

    @author  Ralf Hofmann
    @version 1.0
*/

class OSignalHandler : public vos::OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OSignalHandler, vos));

public:

    enum TSignal
    {
        TSignal_System              = osl_Signal_System,
        TSignal_Terminate           = osl_Signal_Terminate,
        TSignal_AccessViolation     = osl_Signal_AccessViolation,
        TSignal_IntegerDivideByZero = osl_Signal_IntegerDivideByZero,
        TSignal_FloatDivideByZero   = osl_Signal_FloatDivideByZero,
        TSignal_DebugBreak          = osl_Signal_DebugBreak,
        TSignal_SignalUser          = osl_Signal_User
    };

    enum TSignalAction
    {
        TAction_CallNextHandler  = osl_Signal_ActCallNextHdl,
        TAction_Ignore           = osl_Signal_ActIgnore,
        TAction_AbortApplication = osl_Signal_ActAbortApp,
        TAction_KillApplication  = osl_Signal_ActKillApp
    };

    typedef oslSignalInfo TSignalInfo;

    /// Constructor
    OSignalHandler();

    /// Destructor kills thread if necessary
    virtual ~OSignalHandler();

    static TSignalAction SAL_CALL raise(sal_Int32 Signal, void *pData = 0);

protected:

    /// Working method which should be overridden.
    virtual TSignalAction SAL_CALL signal(TSignalInfo *pInfo) = 0;

protected:
    oslSignalHandler m_hHandler;

    friend oslSignalAction signalHandlerFunction_impl(void *, oslSignalInfo *);
};

}

#endif // _VOS_SIGNAL_HXX_

