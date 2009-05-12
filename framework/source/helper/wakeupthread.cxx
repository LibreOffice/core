/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wakeupthread.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// include files of own module
#include <helper/wakeupthread.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

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
