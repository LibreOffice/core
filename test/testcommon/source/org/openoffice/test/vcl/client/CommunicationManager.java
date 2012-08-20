/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.client;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.List;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.openoffice.test.common.SystemUtil;

/**
 * Manage the communication with the automation server.
 * It's used to establish the connection, send and receive data package.
 * Data package format:
 *
 * | [Force Multi Channel (0xFFFFFFFF)] | Data Length (32 bits) | Check Byte (8 bits) | Header Length (16 bits) | Header Data | Body Data |
 *
 * To handle the received data, add a communication listener to the manager.
 * The listner will be called back when data package arrives.
 *
 */
public class CommunicationManager implements Runnable, Constant{

    private static Logger logger = Logger.getLogger("CommunicationManager");

    private final static int DEFAULT_PORT = 12479;

    private String host = "localhost";

    private int port = DEFAULT_PORT;

    private Socket socket = null;

    private double reconnectInterval = 2;

    private int reconnectCount = 5;

    private List<CommunicationListener> listeners = new Vector<CommunicationListener>();

    /**
     * Create a communication manager with the default host and port.
     * The default host is local and the default port is 12479.
     *
     */
    public CommunicationManager() {
        try {
            String portValue = System.getProperty("openoffice.automation.port");
            if (portValue != null)
                port = Integer.parseInt(portValue);
        } catch (NumberFormatException e) {
            // use default
        }
    }

    /**
     * Create a communication manager with the given host and port
     * @param host
     * @param port
     */
    public CommunicationManager(String host, int port) {
        this.host = host;
        this.port = port;
    }

    /**
     * Send a data package to server
     * @param headerType the package header type
     * @param header the data in the header
     * @param data the data in the body
     */
    public synchronized void sendPackage(int headerType, byte[] header, byte[] data) throws CommunicationException {
        if (socket == null)
            start();

        try {
            if (header == null)
                header = new byte[0];

            if (data == null)
                data = new byte[0];

            DataOutputStream os = new DataOutputStream(socket.getOutputStream());
            int len = 1 + 2 + 2 + header.length + data.length;
            // Total len
            os.writeInt(len);
            // Check byte
            os.writeByte(calcCheckByte(len));
            // Header len
            os.writeChar(2 + header.length);
            // Header
            os.writeChar(headerType);
            os.write(header);
            // Data
            os.write(data);
            os.flush();
        } catch (IOException e) {
            stop();
            throw new CommunicationException("Failed to send data to automation server!", e);
        }
    }

    /**
     * Start a new thread to read the data sent by sever
     */
    public void run() {
        try {
            while (socket != null) {
                DataInputStream is = new DataInputStream(socket.getInputStream());

                int len = is.readInt();
                if (len == 0xFFFFFFFF)
                    len = is.readInt();

                byte checkByte = is.readByte();
                if (calcCheckByte(len) != checkByte)
                    throw new CommunicationException("Bad data package. Wrong check byte.");

                int headerLen = is.readUnsignedShort();
                int headerType = is.readUnsignedShort();
                byte[] header = new byte[headerLen - 2];
                is.readFully(header);
                byte[] data = new byte[len - headerLen - 3];
                is.readFully(data);
                for (int i = 0; i < listeners.size(); i++)
                    ((CommunicationListener) listeners.get(i)).received(headerType, header, data);
            }
        } catch (Exception e) {
            logger.log(Level.FINEST, "Failed to receive data!", e);
            stop();
        }
    }

    /**
     * Add a communication listener
     * @param listener
     */
    public void addListener(CommunicationListener listener) {
        if (listener != null && !listeners.contains(listener))
            listeners.add(listener);
    }


    /**
     * Stop the communication manager.
     *
     */
    public synchronized void stop() {
        if (socket == null)
            return;

        try {
            socket.close();
        } catch (IOException e) {
            //ignore
        }
        socket = null;
        logger.log(Level.CONFIG, "Stop Communication Manager");
        for (int i = 0; i < listeners.size(); i++)
            ((CommunicationListener) listeners.get(i)).stop();
    }

    public synchronized boolean isConnected() {
        return socket != null;
    }


    public synchronized void connect() throws IOException {
        if (socket != null)
            return;

        try{
            socket = new Socket();
            socket.setTcpNoDelay(true);
            socket.setSoTimeout(240 * 1000); // if in 4 minutes we get nothing from server, an exception will thrown.
            socket.connect(new InetSocketAddress(host, port));
            Thread thread = new Thread(this);
            thread.setDaemon(true);
            thread.start();
        } catch (IOException e){
            socket = null;
            throw e;
        }
    }

    /**
     * Start the communication manager.
     *
     */
    public synchronized void start() {
        logger.log(Level.CONFIG, "Start Communication Manager");
        //connect and retry if fails
        for (int i = 0; i < reconnectCount; i++) {
            try {
                connect();
                return;
            } catch (IOException e) {
                logger.log(Level.FINEST, "Failed to connect! Tried " + i, e);
            }

            SystemUtil.sleep(reconnectInterval);
        }

        throw new CommunicationException("Failed to connect to automation server on: " + host + ":" + port);
    }


    private static byte calcCheckByte(int i) {
        int nRes = 0;
        int[] bytes = new int[4];
        bytes[0] = (i >>> 24) & 0x00FF;
        bytes[1] = (i >>> 16) & 0x00FF;
        bytes[2] = (i >>> 8) & 0x00FF;
        bytes[3] = (i >>> 0) & 0x00FF;
        nRes += bytes[0] ^ 0xf0;
        nRes += bytes[1] ^ 0x0f;
        nRes += bytes[2] ^ 0xf0;
        nRes += bytes[3] ^ 0x0f;
        nRes ^= (nRes >>> 8);
        return (byte) nRes;
    }
}
