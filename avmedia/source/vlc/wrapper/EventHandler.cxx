/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EventHandler.hxx"

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
EventHandler::EventHandler()
    : ::osl::Thread()
{
}

void EventHandler::stop()
{
    mCallbackQueue.push(TCallback());
    join();
}

void EventHandler::run()
{
    osl_setThreadName("VLC EventHandler");

    TCallback callback;
    do
    {
        mCallbackQueue.pop( callback );

        if ( !callback )
            return;

        callback();
    } while ( true );
}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
