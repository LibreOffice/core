/*************************************************************************
 *
 *  $RCSfile: jobqueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 15:30:39 $
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

#ifndef _CPPU_THREADPOOL_JOBQUEUE_HXX_
#define _CPPU_THREADPOOL_JOBQUEUE_HXX_

#include <list>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <osl/conditn.h>
#include <osl/mutex.hxx>

namespace cppu_threadpool
{
    struct Job
    {
        void *pThreadSpecificData;
        void ( SAL_CALL *doRequest ) ( void * );
    };

    typedef ::std::list < struct Job > JobList;

    typedef ::std::list < sal_Int64 > CallStackList;

    class JobQueue
    {
    public:
        JobQueue(  sal_Bool bAsynchron );
        ~JobQueue();

        void add( void *pThreadSpecificData ,
                  void ( SAL_CALL * doRequest ) ( void *pThreadSpecificData ) );

        void *enter( sal_Int64 nDisposeId , sal_Bool bReturnWhenNoJob = sal_False );
        void dispose( sal_Int64 nDisposeId );

        void suspend();
        void resume();

        sal_Bool isEmpty();
        sal_Bool isCallstackEmpty();
        sal_Bool isBusy();

    private:
        ::osl::Mutex m_mutex;
        JobList      m_lstJob;
        CallStackList m_lstCallstack;
        sal_Int32 m_nToDo;
        sal_Bool m_bSuspended;
        oslCondition m_cndWait;
    };
}

#endif
