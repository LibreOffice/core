/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.os.Parcel;
import android.os.Parcelable;

public class Server implements Parcelable {

    public enum Protocol {
        NETWORK, BLUETOOTH
    }

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

    public String toString() {
        return getClass().getName() + '@' + Integer.toHexString(hashCode()) + ":{" + mAddress + "," + mName + "}";
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mAddress);
        out.writeString(mName);
        out.writeString(mProtocol.name());
        out.writeLong(mTimeDiscovered);
    }

    public static final Parcelable.Creator<Server> CREATOR = new Parcelable.Creator<Server>() {
        public Server createFromParcel(Parcel in) {
            return new Server(in);
        }

        public Server[] newArray(int size) {
            return new Server[size];
        }
    };

    private Server(Parcel in) {
        mAddress = in.readString();
        mName = in.readString();
        mProtocol = Protocol.valueOf(in.readString());
        mTimeDiscovered = in.readLong();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
