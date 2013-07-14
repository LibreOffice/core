package org.libreoffice.impressremote.communication;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.os.Build;

import org.libreoffice.impressremote.Preferences;

public final class PairingProvider {
    private Context mContext;

    private PairingProvider(Context aContext) {
        mContext = aContext;
    }

    public static boolean isPairingNecessary(Server aServer) {
        return aServer.getProtocol() == Server.Protocol.TCP;
    }

    public static String getPairingPin(Context aContext, Server aServer) {
        return new PairingProvider(aContext).getPairingPin(aServer);
    }

    private String getPairingPin(Server aServer) {
        if (isPinSaved(aServer)) {
            return getSavedPin(aServer);
        }

        String aPin = Protocol.Pin.generate();

        savePin(aServer, aPin);

        return aPin;
    }

    private boolean isPinSaved(Server aServer) {
        return getSavedPin(aServer) != null;
    }

    private String getSavedPin(Server aServer) {
        String aLocation = Preferences.Locations.AUTHORIZED_REMOTES;
        String aServerAddress = aServer.getAddress();

        return Preferences.getString(mContext, aLocation, aServerAddress);
    }

    private void savePin(Server aServer, String aPin) {
        String aLocation = Preferences.Locations.AUTHORIZED_REMOTES;
        String aServerAddress = aServer.getAddress();

        Preferences.set(mContext, aLocation, aServerAddress, aPin);
    }

    public static String getPairingDeviceName(Context aContext) {
        return new PairingProvider(aContext).getPairingDeviceName();
    }

    public String getPairingDeviceName() {
        if (BluetoothAdapter.getDefaultAdapter() == null) {
            return Build.MODEL;
        }

        if (BluetoothAdapter.getDefaultAdapter().getName() == null) {
            return Build.MODEL;
        }

        return BluetoothAdapter.getDefaultAdapter().getName();
    }
}
