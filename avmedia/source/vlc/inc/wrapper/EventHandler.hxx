/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <salhelper/thread.hxx>
#include <wrapper/ThreadsafeQueue.hxx>

namespace avmedia::vlc::wrapper
{
    class EventHandler : public ::osl::Thread
    {
    public:
        EventHandler(const EventHandler&) = delete;
        const EventHandler& operator=(const EventHandler&) = delete;

        EventHandler();
        void stop();

    protected:
        virtual void SAL_CALL run() override;

    public:
        typedef std::function< void() > TCallback;
        ThreadsafeQueue< TCallback > mCallbackQueue;
    };
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
