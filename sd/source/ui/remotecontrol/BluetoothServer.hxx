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

#include <salhelper/thread.hxx>

namespace sd
{
    class BluetoothServer:
        public salhelper::Thread
    {
    public:
        static void setup();
    private:
        BluetoothServer();
        ~BluetoothServer();
        static BluetoothServer *spServer;


    public:
    private:
        void execute();
    };
}

#endif // _SD_IMPRESSREMOTE_BLUETOOTHSERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */