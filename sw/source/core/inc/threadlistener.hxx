/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: threadlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:35:02 $
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
#ifndef _THREADLISTENER_HXX
#define _THREADLISTENER_HXX

#ifndef _IFINISHEDTHREADLISTENER_HXX
#include <ifinishedthreadlistener.hxx>
#endif

#ifndef _ITHREADLISTENEROWNER_HXX
#include <ithreadlistenerowner.hxx>
#endif

#ifndef _OBSERVABLETHREAD_HXX
#include <observablethread.hxx>
#endif

/** helper class to observe threads

    OD 2007-03-30 #i73788#
    An instance of this class can be used to observe a thread in order to
    be notified, if the thread has finished its work. The notification is
    directly forward to its owner - an instance of IThreadListenerOwner
    Note:
    - A thread can only have one or none listener.
    - The notification is performed via the ThreadID

    @author OD
*/
class ThreadListener : public IFinishedThreadListener
{
    public:

        ThreadListener( IThreadListenerOwner& rThreadListenerOwner );
        virtual ~ThreadListener();

        void ListenToThread( const oslInterlockedCount nThreadID,
                             ObservableThread& rThread );

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID );

    private:

        IThreadListenerOwner& mrThreadListenerOwner;
};
#endif
