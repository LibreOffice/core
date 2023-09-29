/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
package com.sun.star.lib.connections.websocket;


import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.net.ProtocolException;
import java.net.Socket;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;

import javax.swing.text.html.HTMLDocument.Iterator;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;
import com.sun.star.io.XStreamListener;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

/**
 * The WebsocketConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>WebsocketConnector</code>.
 *
 * <p>This class is not part of the provided <code>api</code>.</p>
 *
 * @see         com.sun.star.lib.connections.socket.socketAcceptor
 * @see         com.sun.star.lib.connections.socket.socketConnector
 * @see         com.sun.star.connection.XConnection
 */
public class WebsocketConnection extends WebSocketClient implements XConnection, XConnectionBroadcaster {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;
    static final byte[] outgoingPrefix = { 'u', 'r', 'p', ' ' };

    protected String       _description;
    protected InputStream  _inputStream;
    protected OutputStream _inputStreamWriter;
    protected ByteArrayOutputStream _outputStream;

    protected ArrayList<XStreamListener>       _listeners;

    /**
     * Constructs a new <code>WebsocketConnection</code>.
     *
     * @param  description   the description of the connection.
     * @param  desc          the websocket ConnectionDescriptor containing information such as the websocket URL
     */
    public WebsocketConnection(String description, ConnectionDescriptor desc) throws IOException, URISyntaxException, InterruptedException {
        super(new URI(desc.getURL()));

        if (DEBUG) System.err.println("##### " + getClass().getName() + " - instantiated " + description + " " + desc);

        _description = description;

        PipedOutputStream inputStreamWriter = new PipedOutputStream();
        PipedInputStream inputPipe = new PipedInputStream(inputStreamWriter);

        _inputStream = new BufferedInputStream(inputPipe);
        _inputStreamWriter = inputStreamWriter;
        _outputStream = new ByteArrayOutputStream();

        _listeners = new ArrayList<XStreamListener>();

        connectBlocking();
    }

    public void addStreamListener(XStreamListener aListener )
    throws com.sun.star.uno.RuntimeException {
        _listeners.add(aListener);
    }

    public void removeStreamListener(XStreamListener aListener )
    throws com.sun.star.uno.RuntimeException {
        _listeners.remove(aListener);
    }

    private void notifyListeners_open() {
        for (XStreamListener xStreamListener : _listeners) {
            xStreamListener.started();
        }
    }

    private void notifyListeners_close() {
        for (XStreamListener xStreamListener : _listeners) {
            xStreamListener.closed();
        }
    }

    private void notifyListeners_error(com.sun.star.uno.Exception exception) {
        for (XStreamListener xStreamListener : _listeners) {
            xStreamListener.error(exception);
        }
    }

    /**
     * Read the required number of bytes.
     *
     * @param    bytes   the outparameter, where the bytes have to be placed.
     * @param    nBytesToRead the number of bytes to read.
     * @return   the number of bytes read.
     *
     * @see      com.sun.star.connection.XConnection#read
     */
    public int read(/*OUT*/byte[][] bytes, int nBytesToRead)
    throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {

        String errMessage = null;

        int read_bytes = 0;
        bytes[0] = new byte[nBytesToRead];

        try {
            _inputStreamWriter.flush();

            int count  ;

            do {
                count = _inputStream.read(bytes[0], read_bytes, nBytesToRead - read_bytes);
                if(count == -1)
                    errMessage = "EOF reached - " + getDescription();

                read_bytes += count;
            }
            while(read_bytes >= 0 && read_bytes < nBytesToRead && count >= 0);
        } catch(IOException ioException) {
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

        if (DEBUG) System.err.println(String.format("##### %s - read %s bytes of %s requested", getClass().getName(), Integer.toString(read_bytes), Integer.toString(nBytesToRead)));

        return read_bytes;
    }

    /**
     * Write bytes.
     *
     * @param    aData the bytes to write.
     * @see       com.sun.star.connection.XConnection#write
     */
    public void write(byte aData[]) throws com.sun.star.io.IOException,
        com.sun.star.uno.RuntimeException {
        try {
            _outputStream.write(aData);
        } catch(IOException ioException) {
            com.sun.star.io.IOException unoIOException = new com.sun.star.io.IOException(ioException);
            notifyListeners_error(unoIOException);

            throw unoIOException;
        }

        if (DEBUG) System.err.println(String.format("##### %s - wrote %s bytes", getClass().getName(), Integer.toString(aData.length)));
    }

    /**
     * Sends the data over the websocket to whatever is on the other side.
     *
     * **NOTE**: unlike with genuine streams, without flushing the data is
     *           never sent
     *
     * @see       com.sun.star.connection.XConnection#flush
     */
    public void flush() throws com.sun.star.io.IOException,
        com.sun.star.uno.RuntimeException {

        byte[] accumulatedBytes = _outputStream.toByteArray();
        _outputStream.reset();

        byte[] outputBytes = new byte[accumulatedBytes.length + outgoingPrefix.length];
        System.arraycopy(outgoingPrefix, 0, outputBytes, 0, outgoingPrefix.length);
        System.arraycopy(accumulatedBytes, 0, outputBytes, outgoingPrefix.length, accumulatedBytes.length);
        send(outputBytes);

        if (DEBUG)
            System.err.println(String.format("##### %s - flushed", getClass().getName()));
    }

    /**
     * Closes the connection.
     *
     * @see       com.sun.star.connection.XConnection#close
     */
    public void close() throws com.sun.star.uno.RuntimeException {
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - socket closed");
        super.close();
    }

    /**
     * Gives a description of the connection.
     *
     * @return  the description.
     * @see       com.sun.star.connection.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _description;
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {
        notifyListeners_open();
    }

    @Override
    public void onClose(int code, String reason, boolean remote) {
        notifyListeners_close();
    }

    @Override
    public void onMessage(String message) {
        String[] messageParts = message.split(": ", 2);
        if (messageParts.length != 2)
        {
            notifyListeners_error(new com.sun.star.uno.Exception(new ProtocolException(String.format("Received URP/WS message (%s) without a type specifier. Messages must be proceeded by 'urp: '", message))));
            return;
        }

        String messageType = messageParts[0];

        if (!messageType.equals("urp"))
        {
            if (DEBUG) System.err.println(String.format("##### %s - received %s message but that is not URP", getClass().getName(), messageType));
            return;
        }

        byte[] messageBytes = messageParts[1].getBytes(StandardCharsets.UTF_8);

        try {
            _inputStreamWriter.write(messageBytes);
            _inputStreamWriter.flush();
        } catch (IOException e) {
            notifyListeners_error(new com.sun.star.uno.Exception(e));
            return;
        }

        if (DEBUG) System.err.println(String.format("##### %s - received %s chars", getClass().getName(), Integer.toString(messageBytes.length)));
    }

    @Override
    public void onMessage(ByteBuffer message) {
        byte[] prefixedMessageBytes = message.array();

        StringBuffer messageTypeBuf = new StringBuffer();
        boolean hasType = false;
        int i;
        for (i = 0; i < prefixedMessageBytes.length - 1; i++) {
            if (prefixedMessageBytes[i] == ':' && prefixedMessageBytes[i+1] == ' ') {
                hasType = true;
                break;  // The type ends with ": ", so if we find this sequence we found the end of our type
            }
            messageTypeBuf.append((char)prefixedMessageBytes[i]);
        }

        if(!hasType) {
            notifyListeners_error(new com.sun.star.uno.Exception(new ProtocolException(String.format("Received URP/WS message (%s) without a type specifier. Binary messages must be proceeded by 'urp: '", message))));
            return;
        }

        String messageType = messageTypeBuf.toString();

        int messageStartIndex = i + 2;

        if (!messageType.equals("urp")) {
            if (DEBUG) System.err.println(String.format("##### %s - received %s binary message but that is not URP", getClass().getName(), messageType));
            return;
        }

        byte[] messageBytes = Arrays.copyOfRange(prefixedMessageBytes, messageStartIndex, prefixedMessageBytes.length);

        try {
            _inputStreamWriter.write(messageBytes);
            _inputStreamWriter.flush();
        } catch (IOException e) {
            notifyListeners_error(new com.sun.star.uno.Exception(e));
            return;
        }

        if (DEBUG) System.err.println(String.format("##### %s - received %s bytes", getClass().getName(), Integer.toString(prefixedMessageBytes.length)));
    }

    @Override
    public void onError(Exception ex) {
        notifyListeners_error(new com.sun.star.uno.Exception(ex));
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
