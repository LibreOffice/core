/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: observablethread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:31:08 $
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
#include "precompiled_sw.hxx"

#ifndef _OBSERVABLETHREAD_HXX
#include <observablethread.hxx>
#endif

#include <boost/shared_ptr.hpp>

/** class for an observable thread

    OD 2007-01-29 #i73788#

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
    return osl_incrementInterlockedCount( &mnRefCount );
}

oslInterlockedCount ObservableThread::release()
{
    oslInterlockedCount nCount( osl_decrementInterlockedCount( &mnRefCount ) );
    if ( nCount == 0 )
    {
        delete this;
        return nCount;
    }

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
