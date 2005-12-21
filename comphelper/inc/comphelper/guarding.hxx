/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guarding.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-12-21 13:46:15 $
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

#ifndef _COMPHELPER_GUARDING_HXX_
#define _COMPHELPER_GUARDING_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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

