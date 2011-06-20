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
#ifndef _SWTHREADMANAGER_HXX
#define _SWTHREADMANAGER_HXX

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>

#include <boost/utility.hpp>
#include <memory>


class ObservableThread;
class ThreadManager;

/** class to manage threads in Writer using a <ThreadManager> instance

    #i73788#
    Conforms the singleton pattern
*/
class SwThreadManager
    : private ::boost::noncopyable
{
    public:

        static SwThreadManager& GetThreadManager();

        static bool ExistsThreadManager();

        // private: don't call!
        SwThreadManager();
        // private: don't call!
        ~SwThreadManager();

        oslInterlockedCount AddThread( const rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.
        */
        void SuspendStartingOfThreads();

        /** continues the starting of threads after it has been suspended
        */
        void ResumeStartingOfThreads();

        bool StartingOfThreadsSuspended();

    private:
        static bool mbThreadManagerInstantiated;

        ::std::auto_ptr<ThreadManager> mpThreadManagerImpl;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
