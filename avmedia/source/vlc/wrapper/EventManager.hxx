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
#ifndef _WRAPPER_EVENT_MANAGER_HXX
#define _WRAPPER_EVENT_MANAGER_HXX
#include <boost/function.hpp>
#include "Player.hxx"

struct libvlc_event_manager_t;
struct libvlc_event_t;
namespace VLC
{
    class EventManager
    {
        friend void EventManagerEventHandler( const libvlc_event_t *event, void *pData );
    public:
        typedef boost::function<void()> Callback;

        EventManager(VLC::Player& player);
        virtual ~EventManager();

        void onPaused(const Callback& callback = Callback());
        void onEndReached(const Callback& callback = Callback());

    private:
        libvlc_event_manager_t *mManager;
        boost::function<void()> mOnPaused;
        boost::function<void()> mOnEndReached;

        void registerSignal(int signal, const Callback& callback);
    };
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */