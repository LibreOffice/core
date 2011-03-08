/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _OBSERVABLETHREAD_HXX
#define _OBSERVABLETHREAD_HXX

#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <osl/interlck.h>

#include <boost/weak_ptr.hpp>
#include <ithreadlistenerowner.hxx>

/** class for an observable thread

    OD 2007-01-29 #i73788#
    Note: A thread is ref-counted. Thus, an instance of a derived class has to
    to be hold via a reference. The thread itself assures during its execution,
    that the ref-count is increased. Its execution starts with its <run()> method
    and ends with its <onTerminated()> method.
    Note: A thread can be only observed by one or none thread observer in order
    to notify, that the thread has finished its work.

    @author OD
*/
class ObservableThread : public osl::Thread,
                         public rtl::IReference
{
    public:

        virtual ~ObservableThread();

        void SetListener( boost::weak_ptr< IFinishedThreadListener > pThreadListener,
                          const oslInterlockedCount nThreadID );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:

        ObservableThread();

        /** intrinsic function of the thread

            Important note:
            Do not override this method again. Instead override <threadFunction()>.
            Otherwise, it's not guaranteed, that its ref-count is increased
            during the execution of the thread.

            @author OD
        */
        virtual void SAL_CALL run();

        virtual void threadFunction() = 0;

        /** method called, when thread has finished its work

            Important note:
            Do not override this method again. Instead override <threadFinished()>.
            Otherwise, it's not guaranteed, that the ref-count is decreased at
            the end of its execution and that the observer is notified, that
            the thread has finished its work.

            @author OD
        */
        virtual void SAL_CALL onTerminated();

        virtual void threadFinished();

    private:

        oslInterlockedCount mnRefCount;

        oslInterlockedCount mnThreadID;

        boost::weak_ptr< IFinishedThreadListener > mpThreadListener;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
