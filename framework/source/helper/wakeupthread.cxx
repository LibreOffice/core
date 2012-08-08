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


//_______________________________________________
// include files of own module
#include <helper/wakeupthread.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>


namespace framework{


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
