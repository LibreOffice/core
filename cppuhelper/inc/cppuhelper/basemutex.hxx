/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basemutex.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#define _CPPUHELPER_BASEMUTEX_HXX_

#include <osl/mutex.hxx>

namespace cppu
{
    /** base class for all classes who want derive from
     ::cppu::WeakComponentImplHelperXX.

     Implmentation classes have first to derive from BaseMutex and then from
     ::cppu::WeakComponentImplHelperXX to ensure that the BaseMutex is completely
     initialized when the mutex is used to intialize the
     ::cppu::WeakComponentImplHelperXX
    */
    class BaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // _CPPUHELPER_BASEMUTEX_HXX_
