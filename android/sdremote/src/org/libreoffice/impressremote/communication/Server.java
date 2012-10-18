/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

public class Server {

    public enum Protocol {
        NETWORK, BLUETOOTH
    };

    private Protocol mProtocol;
    private String mAddress;
    private String mName;
    private long mTimeDiscovered;
    /**
     * Signifies a Server that shouldn't be automatically removed from the list.
     * Used e.g. for the emulator.
     */
    protected boolean mNoTimeout = false;

    protected Server(Protocol aProtocol, String aAddress, String aName,
                    long aTimeDiscovered) {
        mProtocol = aProtocol;
        mAddress = aAddress;
        mName = aName;
        mTimeDiscovered = aTimeDiscovered;
    }

    public Protocol getProtocol() {
        return mProtocol;
    }

    public String getAddress() {
        return mAddress;
    }

    /**
     * Get a human friendly name for the server.
     *
     * @return The name.
     */
    public String getName() {
        return mName;
    }

    public long getTimeDiscovered() {
        return mTimeDiscovered;
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */