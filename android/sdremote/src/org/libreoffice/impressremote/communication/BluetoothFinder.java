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

public class BluetoothFinder {

    // TODO: add removal of cached items
    private Context mContext;

    private static final String CHARSET = "UTF-8";

    BluetoothAdapter mAdapter;

    public BluetoothFinder(Context aContext) {
        mContext = aContext;
        mAdapter = BluetoothAdapter.getDefaultAdapter();

    }

    public void startFinding() {
        System.out.println("BT:Discovery starting");
        IntentFilter aFilter = new IntentFilter(
                        BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        aFilter.addAction(BluetoothDevice.ACTION_FOUND);
        mContext.registerReceiver(mReceiver, aFilter);

        mAdapter.enable();
        mAdapter.startDiscovery();
    }

    public void stopFinding() {
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
            System.out.println("Received intent");
            System.out.println(aIntent.getAction());
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
                mContext.sendBroadcast(aNIntent);
                //                System.out.println("Found " + aDevice.getName());
                //                try {
                //                    // "f36d0a20-e876-11e1-aff1-0800200c9a66"
                //                    BluetoothSocket aSocket = aDevice
                //                                    .createRfcommSocketToServiceRecord(UUID
                //                                                    .fromString("00001101-0000-1000-8000-00805F9B34FB"));
                //                    aSocket.connect();
                //                } catch (IOException e) {
                //                    // TODO Auto-generated catch block
                //                    e.printStackTrace();
                //                    System.out.println("Fallback");
                //                    Method m;
                //                    try {
                //                        m = aDevice.getClass().getMethod("createRfcommSocket",
                //                                        new Class[] { int.class });
                //                        BluetoothSocket aFSocket = (BluetoothSocket) m.invoke(
                //                                        aDevice, 1);
                //
                //                        mAdapter.cancelDiscovery();
                //                        aFSocket.connect();
                //                    } catch (NoSuchMethodException e1) {
                //                        // TODO Auto-generated catch block
                //                        e1.printStackTrace();
                //                    } catch (IllegalArgumentException e1) {
                //                        // TODO Auto-generated catch block
                //                        e1.printStackTrace();
                //                    } catch (IllegalAccessException e1) {
                //                        // TODO Auto-generated catch block
                //                        e1.printStackTrace();
                //                    } catch (InvocationTargetException e1) {
                //                        // TODO Auto-generated catch block
                //                        e1.printStackTrace();
                //                    } catch (IOException e1) {
                //                        // TODO Auto-generated catch block
                //                        e1.printStackTrace();
                //                    }
                //                    System.out.println("Fallback complete");
                //
                //                }
            }

        }

    };
}
