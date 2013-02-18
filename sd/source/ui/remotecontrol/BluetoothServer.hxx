/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_BLUETOOTHSERVER_HXX
#define _SD_IMPRESSREMOTE_BLUETOOTHSERVER_HXX

#include <osl/thread.hxx>
#include <vector>

namespace sd
{
    class Communicator;

    class BluetoothServer:
        public osl::Thread
    {
    public:
        static void setup( std::vector<Communicator*>* pCommunicators );

        /// ensure that Bluetooth discoverability is on
        static void ensureDiscoverable();
        /// restore the state of discoverability from before ensureDiscoverable
        static void restoreDiscoverable();

        void addCommunicator( Communicator* pCommunicator );

    private:
        BluetoothServer( std::vector<Communicator*>* pCommunicators );
        ~BluetoothServer();

        bool isDiscoverable();
        void setDiscoverable( bool bDiscoverable );

        enum { UNKNOWN, DISCOVERABLE, NOT_DISCOVERABLE } meWasDiscoverable;
        static BluetoothServer *spServer;

        virtual void SAL_CALL run();
        std::vector<Communicator*>* mpCommunicators;
    };
}

#endif // _SD_IMPRESSREMOTE_BLUETOOTHSERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
