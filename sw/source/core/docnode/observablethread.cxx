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
#include <observablethread.hxx>

#include <boost/shared_ptr.hpp>

/** class for an observable thread

   #i73788#

    @author OD
*/
ObservableThread::ObservableThread()
    : mnRefCount( 0 ),
      mnThreadID( 0 ),
      mpThreadListener()
{
}

ObservableThread::~ObservableThread()
{
}

oslInterlockedCount ObservableThread::acquire()
{
    return osl_atomic_increment( &mnRefCount );
}

oslInterlockedCount ObservableThread::release()
{
    oslInterlockedCount nCount( osl_atomic_decrement( &mnRefCount ) );
    if ( nCount == 0 )
        delete this;
    return nCount;
}

void ObservableThread::SetListener( boost::weak_ptr< IFinishedThreadListener > pThreadListener,
                                    const oslInterlockedCount nThreadID )
{
    mpThreadListener = pThreadListener;
    mnThreadID = nThreadID;
}

void SAL_CALL ObservableThread::run()
{
    acquire();

    threadFunction();
}

void SAL_CALL ObservableThread::onTerminated()
{
    threadFinished();

    // notify observer
    boost::shared_ptr< IFinishedThreadListener > pThreadListener = mpThreadListener.lock();
    if ( pThreadListener )
    {
        pThreadListener->NotifyAboutFinishedThread( mnThreadID );
    }

    release();
}

void ObservableThread::threadFinished()
{
    // empty default implementation
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
