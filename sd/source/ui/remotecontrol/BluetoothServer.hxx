/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <osl/thread.hxx>
#include <memory>
#include <vector>

#include <config_dbus.h>

#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && ENABLE_DBUS && DBUS_HAVE_GLIB
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

#if defined(MACOSX)
        void addCommunicator( Communicator* pCommunicator );
#endif
    private:
        explicit BluetoothServer( std::vector<Communicator*>* pCommunicators );
        virtual ~BluetoothServer() override;

        enum { UNKNOWN, DISCOVERABLE, NOT_DISCOVERABLE } meWasDiscoverable;
        static BluetoothServer *spServer;

#ifdef LINUX_BLUETOOTH
        struct Impl;
        std::unique_ptr<Impl> mpImpl;
#endif
        virtual void SAL_CALL run() override;

        void cleanupCommunicators();
        std::vector<Communicator*>* mpCommunicators;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
