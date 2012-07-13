/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_TRANSMITTER_HXX
#define _SD_IMPRESSREMOTE_TRANSMITTER_HXX

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <osl/socket.hxx>
#include <salhelper/thread.hxx>
#include <rtl/string.hxx>

#include <queue>

namespace sd
{

class Transmitter
: public salhelper::Thread
{
public:
    enum Priority { LOW = 1, HIGH };
    Transmitter( osl::StreamSocket &aSocket );
    ~Transmitter();
    void addMessage( rtl::OString aMessage, Priority aPriority );

private:
    void execute();

    osl::StreamSocket mStreamSocket;

    osl::Condition mQueuesNotEmpty;

    osl::Mutex mQueueMutex;

    std::queue<rtl::OString> mLowPriority;
    std::queue<rtl::OString> mHighPriority;
};

}
#endif // _SD_IMPRESSREMOTE_TRANSMITTER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */