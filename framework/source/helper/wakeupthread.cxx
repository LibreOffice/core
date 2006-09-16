/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wakeupthread.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:01:54 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// include files of own module

#ifndef __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_
#include <helper/wakeupthread.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// declarations

//***********************************************
WakeUpThread::WakeUpThread(const css::uno::Reference< css::util::XUpdatable >& xListener)
    : ThreadHelpBase(         )
    , m_xListener   (xListener)
{
}

//***********************************************
void SAL_CALL WakeUpThread::run()
{
    ::osl::Condition aSleeper;

    TimeValue aTime;
    aTime.Seconds = 0;
    aTime.Nanosec = 25000000; // 25 msec

    while(schedule())
    {
        aSleeper.reset();
        aSleeper.wait(&aTime);

        // SAFE ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::util::XUpdatable > xListener(m_xListener.get(), css::uno::UNO_QUERY);
        aReadLock.unlock();
        // <- SAFE

        if (xListener.is())
            xListener->update();
    }
}

//***********************************************
void SAL_CALL WakeUpThread::onTerminated()
{
    delete this;
}

} // namespace framework
