package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Collection;
import java.util.HashMap;

import android.content.Context;
import android.content.Intent;

public class ServerFinder {

    private Context mContext;

    private static final int PORT = 1598;
    private static final String GROUPADDRESS = "239.0.0.1";

    private static final String CHARSET = "UTF-8";

    private static final long SEARCH_INTERVAL = 1000 * 20;

    private DatagramSocket mSocket = null;

    private Thread mListenerThread = null;

    private boolean mFinishRequested = false;

    private HashMap<String, Server> mServerList = new HashMap<String, Server>();

    public ServerFinder(Context aContext) {
        mContext = aContext;
    }

    private void listenForServer() {
        byte[] aBuffer = new byte[500];
        DatagramPacket aPacket = new DatagramPacket(aBuffer, aBuffer.length);

        try {
            String aCommand = null;
            String aName = null;
            mSocket.receive(aPacket);
            int i;
            for (i = 0; i < aBuffer.length; i++) {
                if (aPacket.getData()[i] == '\n') {
                    aCommand = new String(aPacket.getData(), 0, i, CHARSET);
                    break;
                }
            }
            if (i == aBuffer.length || !aCommand.equals("LOREMOTE_ADVERTISE")) {
                return;
            }
            for (int j = i + 1; j < aBuffer.length; j++) {
                if (aPacket.getData()[j] == '\n') {
                    aName = new String(aPacket.getData(), i + 1, j - (i + 1),
                                    CHARSET);
                    break;
                }
            }
            if (aName == null) {
                return;
            }
            Server aServer = new Server(Server.Protocol.NETWORK, aPacket
                            .getAddress().getHostAddress(), aName,
                            System.currentTimeMillis());
            mServerList.put(aServer.getAddress(), aServer);
            System.out.println("Contains:<<" + aName + ">>");

            Intent aIntent = new Intent(
                            CommunicationService.MSG_SERVERLIST_CHANGED);
            mContext.sendBroadcast(aIntent);
        } catch (java.net.SocketTimeoutException e) {
            // Ignore -- we want to timeout to enable checking whether we
            // should stop listening periodically
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    public void startFinding() {
        if (mSocket != null)
            return;

        mFinishRequested = false;

        if (mListenerThread == null) {
            mListenerThread = new Thread() {
                @Override
                public void run() {
                    long aTime = 0;
                    try {
                        mSocket = new DatagramSocket();
                        mSocket.setSoTimeout(1000 * 10);
                        while (!mFinishRequested) {
                            if (System.currentTimeMillis() - aTime > SEARCH_INTERVAL) {
                                String aString = "LOREMOTE_SEARCH\n";
                                DatagramPacket aPacket = new DatagramPacket(
                                                aString.getBytes(CHARSET),
                                                aString.length(),
                                                InetAddress.getByName(GROUPADDRESS),
                                                PORT);
                                mSocket.send(aPacket);
                                aTime = System.currentTimeMillis();
                                for (Server aServer : mServerList.values()) {
                                    if (System.currentTimeMillis()
                                                    - aServer.getTimeDiscovered() > 60 * 1000) {
                                        mServerList.remove(aServer.getAddress());
                                        Intent aIntent = new Intent(
                                                        CommunicationService.MSG_SERVERLIST_CHANGED);
                                        mContext.sendBroadcast(aIntent);

                                    }
                                }
                            }

                            listenForServer();
                        }
                    } catch (SocketException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (UnsupportedEncodingException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                }
            };
            mListenerThread.start();
        }

    }

    public void stopFinding() {
        if (mListenerThread != null) {
            mFinishRequested = true;
            mListenerThread = null;
        }
    }

    public Collection<Server> getServerList() {
        return mServerList.values();
    }
}
