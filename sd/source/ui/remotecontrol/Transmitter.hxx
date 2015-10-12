/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_TRANSMITTER_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_TRANSMITTER_HXX

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include "IBluetoothSocket.hxx"
#include <osl/thread.hxx>
#include <rtl/string.hxx>

#include <queue>

namespace sd
{

class Transmitter
: public osl::Thread
{
public:
    enum Priority { PRIORITY_LOW = 1, PRIORITY_HIGH };
    Transmitter( ::sd::IBluetoothSocket* aSocket );
    virtual ~Transmitter();
    void addMessage( const OString& aMessage, const Priority aPriority );
    void notifyFinished();

private:
    virtual void SAL_CALL run() override;

    ::sd::IBluetoothSocket* pStreamSocket;

    ::osl::Condition mQueuesNotEmpty;
    ::osl::Condition mFinishRequested;

    ::osl::Mutex mQueueMutex;

    std::queue<OString> mLowPriority;
    std::queue<OString> mHighPriority;
};

}
#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_TRANSMITTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
