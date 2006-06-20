/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signal.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 11:21:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include <vos/diagnose.hxx>
#include <vos/object.hxx>
#include <vos/signal.hxx>

using namespace vos;

oslSignalAction vos::signalHandlerFunction_impl(
    void * pthis, oslSignalInfo * pInfo)
{
    NAMESPACE_VOS(OSignalHandler)* pThis= (NAMESPACE_VOS(OSignalHandler)*)pthis;

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

