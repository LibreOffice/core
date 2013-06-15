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
    private static final int SPECIAL_PARCELABLE_OBJECTS_BITMASK = 0;

    public static enum Protocol {
        NETWORK, BLUETOOTH
    }

    private final Protocol mProtocol;
    private final String mAddress;
    private final String mName;
    private final long mTimeDiscovered;

    /**
     * Signifies a Server that shouldn't be automatically removed from the list.
     * Used e.g. for the emulator.
     */
    protected boolean mNoTimeout = false;

    protected Server(Protocol aProtocol, String aAddress, String aName, long aTimeDiscovered) {
        this.mProtocol = aProtocol;
        this.mAddress = aAddress;
        this.mName = aName;
        this.mTimeDiscovered = aTimeDiscovered;
    }

    public Protocol getProtocol() {
        return mProtocol;
    }

    public String getAddress() {
        return mAddress;
    }

    public String getName() {
        return mName;
    }

    public long getTimeDiscovered() {
        return mTimeDiscovered;
    }

    public String toString() {
        return getClass().getName() + '@' + Integer
            .toHexString(hashCode()) + ":{" + mAddress + "," + mName + "}";
    }

    @Override
    public int describeContents() {
        return SPECIAL_PARCELABLE_OBJECTS_BITMASK;
    }

    @Override
    public void writeToParcel(Parcel parcel, int flags) {
        parcel.writeString(mAddress);
        parcel.writeString(mName);
        parcel.writeString(mProtocol.name());
        parcel.writeLong(mTimeDiscovered);
    }

    public static final Parcelable.Creator<Server> CREATOR = new Parcelable.Creator<Server>() {
        public Server createFromParcel(Parcel parcel) {
            return new Server(parcel);
        }

        public Server[] newArray(int size) {
            return new Server[size];
        }
    };

    private Server(Parcel parcel) {
        this.mAddress = parcel.readString();
        this.mName = parcel.readString();
        this.mProtocol = Protocol.valueOf(parcel.readString());
        this.mTimeDiscovered = parcel.readLong();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
