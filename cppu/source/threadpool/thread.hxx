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
#ifndef _CPPU_THREADPOOL_THREAD_HXX
#define _CPPU_THREADPOOL_THREAD_HXX

#include <osl/thread.hxx>
#include <sal/types.h>
#include <salhelper/simplereferenceobject.hxx>

#include "jobqueue.hxx"
#include "threadpool.hxx"

namespace cppu_threadpool {

    class JobQueue;

    //-----------------------------------------
    // private thread class for the threadpool
    // independent from vos
    //-----------------------------------------
    class ORequestThread:
        public salhelper::SimpleReferenceObject, public osl::Thread
    {
    public:
        ORequestThread( ThreadPoolHolder const &aThreadPool,
                        JobQueue * ,
                        const ::rtl::ByteSequence &aThreadId,
                        sal_Bool bAsynchron );
        virtual ~ORequestThread();

        void setTask( JobQueue * , const ::rtl::ByteSequence & aThreadId , sal_Bool bAsynchron );

        void launch();

        static inline void * operator new(std::size_t size)
        { return SimpleReferenceObject::operator new(size); }

        static inline void operator delete(void * pointer)
        { SimpleReferenceObject::operator delete(pointer); }

    private:
        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();

        ThreadPoolHolder m_aThreadPool;
        JobQueue *m_pQueue;
        ::rtl::ByteSequence m_aThreadId;
        sal_Bool m_bAsynchron;
    };

} // end cppu_threadpool


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
