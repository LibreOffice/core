/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: guarding.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _COMPHELPER_GUARDING_HXX_
#define _COMPHELPER_GUARDING_HXX_

#include <osl/mutex.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

// ===================================================================================================
// = class MutexRelease -
// ===================================================================================================

/** opposite of OGuard :)
    (a mutex is released within the constructor and acquired within the desctructor)
    use only when you're sure the mutex is acquired !
*/
template <class MUTEX>
class ORelease
{
    MUTEX&  m_rMutex;

public:
    ORelease(MUTEX& _rMutex) : m_rMutex(_rMutex) { _rMutex.release(); }
    ~ORelease() { m_rMutex.acquire(); }
};

typedef ORelease< ::osl::Mutex >    MutexRelease;

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_GUARDING_HXX_

