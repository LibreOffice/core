/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guarding.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:31:37 $
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

// ===================================================================================================
// = class OCountedMutex - a Mutex which counts the acquire-/release-calls and "ensures" (with
// =                        assertions) that the acquire-counter isn't negative ....
// =                        (best used with MutexRelease to ensure that this "dangerous" class is used
// =                        correctly)
// ===================================================================================================

// CAN'T USE THIS AS THERE NO SUCH THING AS IMutex ANYMORE ...
// AND OMutex DOESN'T HAVE THE VIRTUAL METHODS IMutex HAD ...
typedef ::osl::Mutex    OCountedMutex;

// ===================================================================================================
// = class OReusableGuard
// = a mutex guard which can be cleared and reattached
// ===================================================================================================
template<class T>
class OReusableGuard : public ::osl::ClearableGuard<T>
{
public:
    OReusableGuard(T& _rMutex) : ::osl::ClearableGuard<T>(_rMutex) { }
    ~OReusableGuard() { }

    const OReusableGuard& operator= (const OReusableGuard& _rMaster)
    {
        this->clear();
        this->pT = _rMaster.pT;
        if (this->pT)
            this->pT->acquire();
        return *this;
    }

    void attach(T& rMutex)
    {
        this->clear();
        this->pT = &rMutex;
        this->pT->acquire();
    }
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_GUARDING_HXX_

