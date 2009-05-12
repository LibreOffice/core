/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: broadcasthelper.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#define _COMPHELPER_BROADCASTHELPER_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

    //==================================================================================
    //= OMutexAndBroadcastHelper - a class which holds a Mutex and a OBroadcastHelper;
    //=                 needed because when deriving from OPropertySetHelper,
    //=                 the OBroadcastHelper has to be initialized before
    //=                 the OPropertySetHelper
    //==================================================================================
    class OMutexAndBroadcastHelper
    {
    protected:
        ::osl::Mutex                m_aMutex;
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }

    };

    // base class for all classes who are derived from OPropertySet and from OComponent
    // @deprecated, you should use cppu::BaseMutex instead (cppuhelper/basemutex.hxx)

    class OBaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // _COMPHELPER_BROADCASTHELPER_HXX_
