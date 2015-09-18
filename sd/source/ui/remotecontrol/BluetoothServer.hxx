/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_BLUETOOTHSERVER_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_BLUETOOTHSERVER_HXX

#include <osl/thread.hxx>
#include <memory>
#include <vector>

#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
#  define LINUX_BLUETOOTH
#endif

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

        // called by C / idle callbacks
        static void doEnsureDiscoverable();
        static void doRestoreDiscoverable();

        void addCommunicator( Communicator* pCommunicator );
    private:
        BluetoothServer( std::vector<Communicator*>* pCommunicators );
        virtual ~BluetoothServer();

        enum { UNKNOWN, DISCOVERABLE, NOT_DISCOVERABLE } meWasDiscoverable;
        static BluetoothServer *spServer;

#ifdef LINUX_BLUETOOTH
        struct Impl;
        std::unique_ptr<Impl> mpImpl;
#endif
        virtual void SAL_CALL run() SAL_OVERRIDE;

        void cleanupCommunicators();
        std::vector<Communicator*>* mpCommunicators;
    };
}

#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_BLUETOOTHSERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
