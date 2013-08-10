/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <observablethread.hxx>
#include <boost/shared_ptr.hpp>

/* class for an observable thread

   #i73788#
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
