/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.lib.connections.socket;


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Iterator;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;
import com.sun.star.io.XStreamListener;

/**
 * The SocketConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>SocketConnector</code> and the <code>SocketAcceptor</code>.
 * This class is not part of the provided <code>api</code>.
 * <p>
 * @see         com.sun.star.lib.connections.socket.socketAcceptor
 * @see         com.sun.star.lib.connections.socket.socketConnector
 * @see         com.sun.star.connection.XConnection
 * @since       UDK1.0
 */
public class SocketConnection implements XConnection, XConnectionBroadcaster {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    protected String       _description;
    protected Socket       _socket;
    protected InputStream  _inputStream;
    protected OutputStream _outputStream;
    protected ArrayList<XStreamListener>       _listeners;
    protected boolean      _firstRead;

    /**
     * Constructs a new <code>SocketConnection</code>.
     * <p>
     * @param  description   the description of the connection
     * @param  socket        the socket of the connection
     */
    public SocketConnection(String description, Socket socket) throws IOException {
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - instantiated " + description + " " + socket);

        _description = description
            + ",localHost=" + socket.getLocalAddress().getHostName()
            + ",localPort=" + socket.getLocalPort()
            + ",peerHost=" + socket.getInetAddress().getHostName()
            + ",peerPort=" + socket.getPort();

        _socket = socket;
        _inputStream = new BufferedInputStream(socket.getInputStream());
        _outputStream = new BufferedOutputStream(socket.getOutputStream());

        _listeners = new ArrayList<XStreamListener>();
        _firstRead = true;
    }




    public void addStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _listeners.add(aListener);
    }

    public void removeStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _listeners.remove(aListener);
    }

    private void notifyListeners_open() {
        Iterator<XStreamListener> elements = _listeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.started();
        }
    }

    private void notifyListeners_close() {
        Iterator<XStreamListener> elements = _listeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.closed();
        }
    }

    private void notifyListeners_error(com.sun.star.uno.Exception exception) {
        Iterator<XStreamListener> elements = _listeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.error(exception);
        }
    }


    /**
     * Read the required number of bytes.
     * <p>
     * @return   the number of bytes read
     * @param    bytes   the outparameter, where the bytes have to be placed
     * @param    nBytesToRead the number of bytes to read
     * @see       com.sun.star.connection.XConnection#read
     */
    public int read(/*OUT*/byte[][] bytes, int nBytesToRead) throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        if(_firstRead) {
            _firstRead = false;

            notifyListeners_open();
        }

        String errMessage = null;

        int read_bytes = 0;
        bytes[0] = new byte[nBytesToRead];

        try {
            int count  ;

            do {
                count = _inputStream.read(bytes[0], read_bytes, nBytesToRead - read_bytes);
                if(count == -1)
                    errMessage = "EOF reached - " + getDescription();

                read_bytes += count;
            }
            while(read_bytes >= 0 && read_bytes < nBytesToRead && count >= 0);
        }
        catch(IOException ioException) {
            if(DEBUG) {
                System.err.println("##### " + getClass().getName() + ".read - exception occurred:" + ioException);
                ioException.printStackTrace();
            }

            errMessage = ioException.toString();
        }

        if(errMessage != null) {
            com.sun.star.io.IOException unoIOException = new com.sun.star.io.IOException(errMessage);
            notifyListeners_error(unoIOException);

            throw unoIOException;
        }

        if (DEBUG) System.err.println("##### " + getClass().getName() + " - read byte:" + read_bytes + " " + bytes[0]);

        return read_bytes;
    }

    /**
     * Write bytes.
     * <p>
     * @param    aData the bytes to write
     * @see       com.sun.star.connection.XConnection#write
     */
    public void write(byte aData[]) throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        try {
            _outputStream.write(aData);
        }
        catch(IOException ioException) {
            com.sun.star.io.IOException unoIOException = new com.sun.star.io.IOException(ioException.toString());
            notifyListeners_error(unoIOException);

            throw unoIOException;
        }

        if (DEBUG) System.err.println("##### " + getClass().getName() + " - written bytes:" + aData + " " + aData.length);
    }

    /**
     * Flushes the buffer.
     * <p>
     * @see       com.sun.star.connection.XConnection#flush
     */
    public void flush() throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        try {
            _outputStream.flush();
        }
        catch(IOException ioException) {
            com.sun.star.io.IOException unoIOException = new com.sun.star.io.IOException(ioException.toString());
            notifyListeners_error(unoIOException);

            throw unoIOException;
        }
    }

    /**
     * Closes the connection.
     * <p>
     * @see       com.sun.star.connection.XConnection#close
     */
    public void close() throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        try {
            _socket.close();
        }
        catch(IOException ioException) {
            com.sun.star.io.IOException unoIOException = new com.sun.star.io.IOException(ioException.toString());
            notifyListeners_error(unoIOException);

            throw unoIOException;
        }
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - socket closed");

        notifyListeners_close();
    }

    /**
     * Gives a description of the connection.
     * <p>
     * @return  the description
     * @see       com.sun.star.connection.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _description;
    }

}

