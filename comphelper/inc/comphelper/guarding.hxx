/*************************************************************************
 *
 *  $RCSfile: guarding.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        clear();
        pT = _rMaster.pT;
        if (pT)
            pT->acquire();
        return *this;
    }

    void attach(T& rMutex)
    {
        clear();
        pT = &rMutex;
        pT->acquire();
    }
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_GUARDING_HXX_

