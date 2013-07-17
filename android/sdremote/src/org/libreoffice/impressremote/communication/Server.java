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
        TCP, BLUETOOTH
    }

    private final Protocol mProtocol;
    private final String mAddress;
    private final String mName;

    public Server(Protocol aProtocol, String aAddress, String aName) {
        this.mProtocol = aProtocol;
        this.mAddress = aAddress;
        this.mName = aName;
    }

    public static Server newTcpInstance(String aAddress, String aName) {
        return new Server(Protocol.TCP, aAddress, aName);
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

    @Override
    public int describeContents() {
        return SPECIAL_PARCELABLE_OBJECTS_BITMASK;
    }

    @Override
    public void writeToParcel(Parcel aParcel, int aFlags) {
        aParcel.writeString(mAddress);
        aParcel.writeString(mName);
        aParcel.writeString(mProtocol.name());
    }

    public static final Parcelable.Creator<Server> CREATOR = new Parcelable.Creator<Server>() {
        public Server createFromParcel(Parcel aParcel) {
            return new Server(aParcel);
        }

        public Server[] newArray(int aSize) {
            return new Server[aSize];
        }
    };

    private Server(Parcel aParcel) {
        this.mAddress = aParcel.readString();
        this.mName = aParcel.readString();
        this.mProtocol = Protocol.valueOf(aParcel.readString());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
