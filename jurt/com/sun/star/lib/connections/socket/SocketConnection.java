/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SocketConnection.java,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.lib.connections.socket;


import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.net.Socket;

import java.util.Enumeration;
import java.util.Vector;


import com.sun.star.io.XStreamListener;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;

/**
 * The SocketConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>SocketConnector</code> and the <code>SocketAcceptor</code>.
 * This class is not part of the provided <code>api</code>.
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2008-04-11 11:14:31 $
 * @author      Kay Ramme
 * @see         com.sun.star.comp.connections.SocketAcceptor
 * @see         com.sun.star.comp.connections.SocketConnector
 * @see         com.sun.star.connections.XConnection
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
    protected Vector       _listeners;
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

        _listeners = new Vector();
        _firstRead = true;
    }




    public void addStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _listeners.addElement(aListener);
    }

    public void removeStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _listeners.removeElement(aListener);
    }

    private void notifyListeners_open() {
        Enumeration elements = _listeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
            xStreamListener.started();
        }
    }

    private void notifyListeners_close() {
        Enumeration elements = _listeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
            xStreamListener.closed();
        }
    }

    private void notifyListeners_error(com.sun.star.uno.Exception exception) {
        Enumeration elements = _listeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
            xStreamListener.error(exception);
        }
    }


    /**
     * Read the required number of bytes.
     * <p>
     * @return   the number of bytes read
     * @param    aReadBytes   the outparameter, where the bytes have to be placed
     * @param    nBytesToRead the number of bytes to read
     * @see       com.sun.star.connections.XConnection#read
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
            int count = 0;

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
     * @see       com.sun.star.connections.XConnection#write
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
     * @see       com.sun.star.connections.XConnection#flush
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
     * @see       com.sun.star.connections.XConnection#close
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
     * @see       com.sun.star.connections.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _description;
    }

}

