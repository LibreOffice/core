package org.libreoffice.impressremote.communication;

import java.util.Collection;
import java.util.HashMap;

import org.libreoffice.impressremote.communication.Server.Protocol;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.LocalBroadcastManager;

public class BluetoothFinder {

    // TODO: add removal of cached items
    private Context mContext;

    BluetoothAdapter mAdapter;

    public BluetoothFinder(Context aContext) {
        mContext = aContext;
        mAdapter = BluetoothAdapter.getDefaultAdapter();

    }

    public void startFinding() {
        if (mAdapter == null) {
            return; // No bluetooth adapter found (emulator, special devices)
        }
        System.out.println("BT:Discovery starting");
        IntentFilter aFilter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        aFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        mContext.registerReceiver(mReceiver, aFilter);

        mAdapter.enable();
        mAdapter.startDiscovery();
    }

    public void stopFinding() {
        if (mAdapter == null) {
            return; // No bluetooth adapter found (emulator, special devices)
        }
        mAdapter.cancelDiscovery();
        try {
            mContext.unregisterReceiver(mReceiver);
        } catch (IllegalArgumentException e) {
            // The receiver wasn't registered
        }
    }

    private HashMap<String, Server> mServerList = new HashMap<String, Server>();

    public Collection<Server> getServerList() {
        return mServerList.values();
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent aIntent) {
            // TODO Auto-generated method stub
            if (aIntent.getAction().equals(BluetoothDevice.ACTION_FOUND)) {
                BluetoothDevice aDevice = (BluetoothDevice) aIntent.getExtras()
                                .get(BluetoothDevice.EXTRA_DEVICE);
                Server aServer = new Server(Protocol.BLUETOOTH,
                                aDevice.getAddress(), aDevice.getName(),
                                System.currentTimeMillis());
                mServerList.put(aServer.getAddress(), aServer);
                System.out.println("Added " + aServer.getName());
                System.out.println("Now we have: " + mServerList.size());
                Intent aNIntent = new Intent(
                                CommunicationService.MSG_SERVERLIST_CHANGED);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aNIntent);
            } else if (aIntent.getAction().equals(
                            BluetoothAdapter.ACTION_DISCOVERY_FINISHED)) {
                // Start discovery again after a small delay.
                Handler aHandler = new Handler();
                aHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        mAdapter.startDiscovery();
                    }
                }, 1000 * 15);
                ;
            }

        }

    };
}
