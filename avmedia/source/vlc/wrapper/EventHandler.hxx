/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _WRAPPER_EVENT_HANDLER_HXX
#define _WRAPPER_EVENT_HANDLER_HXX
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <salhelper/thread.hxx>
#include "ThreadsafeQueue.hxx"

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
    class EventHandler : public ::osl::Thread, boost::noncopyable
    {
    public:
        EventHandler();
        void stop();

    protected:
        virtual void SAL_CALL run();

    public:
        typedef boost::function< void() > TCallback;
        ThreadsafeQueue< TCallback > mCallbackQueue;
    };
}
}
}

#endif // _WRAPPER_EVENT_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
