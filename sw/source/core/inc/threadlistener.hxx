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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_THREADLISTENER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_THREADLISTENER_HXX

#include <ifinishedthreadlistener.hxx>
#include <observablethread.hxx>

class ThreadManager;

/** helper class to observe threads

    OD 2007-03-30 #i73788#
    An instance of this class can be used to observe a thread in order to
    be notified, if the thread has finished its work. The notification is
    directly forward to its owner - an instance of ThreadManager
    Note:
    - A thread can only have one or none listener.
    - The notification is performed via the ThreadID
*/
class ThreadListener : public IFinishedThreadListener
{
    public:

        ThreadListener( ThreadManager& rThreadListenerOwner );
        virtual ~ThreadListener();

        void ListenToThread( const oslInterlockedCount nThreadID,
                             ObservableThread& rThread );

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) override;

    private:

        ThreadManager& mrThreadListenerOwner;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
