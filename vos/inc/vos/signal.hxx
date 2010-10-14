/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

class OSignalHandler : public NAMESPACE_VOS(OObject)
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

    /// Destructor kills thread if neccessary
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
