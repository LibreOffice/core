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

#include <boost/scoped_ptr.hpp>

namespace sd
{
    class Communicator;

    struct BluetoothServerImpl;
    class BluetoothServer:
        public osl::Thread
    {
    public:
        static void setup( std::vector<Communicator*>* pCommunicators );

        /// ensure that Bluetooth discoverability is on
        static void ensureDiscoverable();
        /// restore the state of discoverability from before ensureDiscoverable
        static void restoreDiscoverable();

        // called by C / idle callbacks
        static void doEnsureDiscoverable();
        static void doRestoreDiscoverable();

        void addCommunicator( Communicator* pCommunicator );
    private:
        BluetoothServer( std::vector<Communicator*>* pCommunicators );
        ~BluetoothServer();

        enum { UNKNOWN, DISCOVERABLE, NOT_DISCOVERABLE } meWasDiscoverable;
        static BluetoothServer *spServer;

#ifdef LINUX_BLUETOOTH
        boost::scoped_ptr<BluetoothServerImpl> mpImpl;
#endif
        virtual void SAL_CALL run();

        void cleanupCommunicators();
        std::vector<Communicator*>* mpCommunicators;
    };
}

#endif // _SD_IMPRESSREMOTE_BLUETOOTHSERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
