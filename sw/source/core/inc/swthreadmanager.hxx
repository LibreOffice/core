/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swthreadmanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:34:52 $
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
#ifndef _SWTHREADMANAGER_HXX
#define _SWTHREADMANAGER_HXX

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

class ObservableThread;
class ThreadManager;

/** class to manage threads in Writer using a <ThreadManager> instance

    OD 2007-04-13 #i73788#
    Conforms the singleton pattern

    @author OD
*/
class SwThreadManager
{
    public:

        static SwThreadManager& GetThreadManager();

        static bool ExistsThreadManager();

        ~SwThreadManager();

        oslInterlockedCount AddThread( const rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.

            @author OD
        */
        void SuspendStartingOfThreads();

        /** continues the starting of threads after it has been suspended

            @author OD
        */
        void ResumeStartingOfThreads();

        bool StartingOfThreadsSuspended();

    private:
        static SwThreadManager* mpThreadManager;
        static osl::Mutex* mpGetManagerMutex;

        ThreadManager* mpThreadManagerImpl;

        SwThreadManager();

};
#endif
