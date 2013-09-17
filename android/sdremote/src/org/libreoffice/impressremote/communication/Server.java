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
import android.text.TextUtils;

public class Server implements Parcelable {
    private static final int SPECIAL_PARCELABLE_OBJECTS_BITMASK = 0;

    public static enum Protocol {
        TCP, BLUETOOTH
    }

    public static enum Type {
        COMPUTER, PHONE, UNDEFINED
    }

    private final Protocol mProtocol;
    private final Type mType;
    private final String mAddress;
    private final String mName;

    private Server(Protocol aProtocol, Type aType, String aAddress, String aName) {
        mProtocol = aProtocol;
        mType = aType;
        mAddress = aAddress;
        mName = aName;
    }

    public static Server newTcpInstance(String aAddress, String aName) {
        return new Server(Protocol.TCP, Type.UNDEFINED, aAddress, aName);
    }

    public static Server newBluetoothInstance(Type aClass, String aAddress, String aName) {
        return new Server(Protocol.BLUETOOTH, aClass, aAddress, aName);
    }

    public Protocol getProtocol() {
        return mProtocol;
    }

    public Type getType() {
        return mType;
    }

    public String getAddress() {
        return mAddress;
    }

    public String getName() {
        if (mName == null) {
            return mAddress;
        }

        if (TextUtils.isEmpty(mName)) {
            return mAddress;
        }

        return mName;
    }

    @Override
    public int describeContents() {
        return SPECIAL_PARCELABLE_OBJECTS_BITMASK;
    }

    @Override
    public void writeToParcel(Parcel aParcel, int aFlags) {
        aParcel.writeString(mProtocol.name());
        aParcel.writeString(mType.name());
        aParcel.writeString(mAddress);
        aParcel.writeString(mName);
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
        this.mProtocol = Protocol.valueOf(aParcel.readString());
        this.mType = Type.valueOf(aParcel.readString());
        this.mAddress = aParcel.readString();
        this.mName = aParcel.readString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
