/*************************************************************************
 *
 *  $RCSfile: mutex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:40:09 $
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

#ifndef _OSL_MUTEX_HXX_
#define _OSL_MUTEX_HXX_

#ifdef __cplusplus

#include <osl/macros.hxx>

#include <osl/mutex.h>


#ifdef _USE_NAMESPACE
namespace osl
{
#endif

    class Mutex {
        oslMutex mutex;

        // these make no sense
        Mutex( oslMutex ) {}
        Mutex( const Mutex & ) {}

    public:
        /** Create a thread-local mutex.
            @return 0 if the mutex could not be created, otherwise a handle to the mutex.
        */
        Mutex()
        {
            mutex = osl_createMutex();
        }

        /** Release the OS-structures and free mutex data-structure.
        */
        ~Mutex()
        {
            osl_destroyMutex(mutex);
        }

        /** Acquire the mutex, block if already acquired by another thread.
            @return False if system-call fails.
        */
        sal_Bool acquire()
        {
            return osl_acquireMutex(mutex);
        }

        /** Try to acquire the mutex without blocking.
            @return False if it could not be acquired.
        */
        sal_Bool tryToAcquire()
        {
            return osl_tryToAcquireMutex(mutex);
        }

        /** Release the mutex.
            @return False if system-call fails.
        */
        sal_Bool release()
        {
            return osl_releaseMutex(mutex);
        }

        static Mutex * getGlobalMutex()
        {
            return (Mutex *)osl_getGlobalMutex();
        }
    };

    template<class T>
    class Guard
    {
    protected:
        T * pT;
    public:

        Guard(T * pT) : pT(pT)
        {
            pT->acquire();
        }

        Guard(T & t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases mutex. */
        ~Guard()
        {
            pT->release();
        }
    };

    template<class T>
    class ClearableGuard
    {
    protected:
        T * pT;
    public:

        ClearableGuard(T * pT) : pT(pT)
        {
            pT->acquire();
        }

        ClearableGuard(T & t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases mutex. */
        ~ClearableGuard()
        {
            if (pT)
                pT->release();
        }

        /** Releases mutex. */
        void clear()
        {
            if(pT)
            {
                pT->release();
                pT = NULL;
            }
        }
    };

    typedef Guard<Mutex> MutexGuard;
    typedef ClearableGuard<Mutex> ClearableMutexGuard;

#ifdef _USE_NAMESPACE
}
#endif

#endif  /* __cplusplus */
#endif  /* _OSL_MUTEX_HXX_ */

