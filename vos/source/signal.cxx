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

