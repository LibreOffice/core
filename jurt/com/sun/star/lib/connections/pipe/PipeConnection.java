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
package com.sun.star.lib.connections.pipe;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;
import com.sun.star.io.XStreamListener;
import com.sun.star.lib.util.NativeLibraryLoader;

/**
 * The PipeConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>PipeConnector</code> and the <code>PipeAcceptor</code>.
 * This class is not part of the provided <code>api</code>.
 * <p>
 * @see         com.sun.star.lib.connections.pipe.pipeAcceptor
 * @see         com.sun.star.lib.connections.pipe.pipeConnector
 * @see         com.sun.star.connection.XConnection
 * @since       UDK1.0
 */
public class PipeConnection implements XConnection, XConnectionBroadcaster {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    static {
        // load shared library for JNI code
        NativeLibraryLoader.loadLibrary(PipeConnection.class.getClassLoader(), "jpipe");
    }

    protected String    _aDescription;
    protected long      _nPipeHandle;
    protected ArrayList<XStreamListener>    _aListeners;
    protected boolean   _bFirstRead;

    /**
     * Constructs a new <code>PipeConnection</code>.
     * <p>
     * @param  description   the description of the connection
     */
    public PipeConnection(String description)
        throws IOException
    {
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - instantiated " + description );

        _aListeners = new ArrayList<XStreamListener>();
        _bFirstRead = true;

        // get pipe name from pipe descriptor
        String aPipeName  ;
        StringTokenizer aTokenizer = new StringTokenizer( description, "," );
        if ( aTokenizer.hasMoreTokens() )
        {
            String aConnType = aTokenizer.nextToken();
            if ( !aConnType.equals( "pipe" ) )
                throw new RuntimeException( "invalid pipe descriptor: does not start with 'pipe,'" );

            String aPipeNameParam = aTokenizer.nextToken();
            if ( !aPipeNameParam.substring( 0, 5 ).equals( "name=" ) )
                throw new RuntimeException( "invalid pipe descriptor: no 'name=' parameter found" );
            aPipeName = aPipeNameParam.substring( 5 );
             }
        else
            throw new RuntimeException( "invalid or empty pipe descriptor" );

        // create the pipe
        try
        { createJNI( aPipeName ); }
        catch ( java.lang.NullPointerException aNPE )
        { throw new IOException( aNPE.getMessage() ); }
        catch ( com.sun.star.io.IOException aIOE )
        { throw new IOException( aIOE.getMessage() ); }
        catch ( java.lang.Exception aE )
        { throw new IOException( aE.getMessage() ); }
    }

    public void addStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _aListeners.add(aListener);
    }

    public void removeStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _aListeners.remove(aListener);
    }

    private void notifyListeners_open() {
        Iterator<XStreamListener> elements = _aListeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.started();
        }
    }

    private void notifyListeners_close() {
        Iterator<XStreamListener> elements = _aListeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.closed();
        }
    }

    private void notifyListeners_error(com.sun.star.uno.Exception exception) {
        Iterator<XStreamListener> elements = _aListeners.iterator();
        while(elements.hasNext()) {
            XStreamListener xStreamListener = elements.next();
            xStreamListener.error(exception);
        }
    }

    // JNI implementation to create the pipe
    private native int createJNI( String name )
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    // JNI implementation to read from the pipe
    private native int readJNI(/*OUT*/byte[][] bytes, int nBytesToRead)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    // JNI implementation to write to the pipe
    private native void writeJNI(byte aData[])
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    // JNI implementation to flush the pipe
    private native void flushJNI()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    // JNI implementation to close the pipe
    private native void closeJNI()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    /**
     * Read the required number of bytes.
     * <p>
     * @return   the number of bytes read
     * @param    bytes   the outparameter, where the bytes have to be placed
     * @param    nBytesToRead the number of bytes to read
     * @see       com.sun.star.connection.XConnection#read
     */
    public int read(/*OUT*/byte[][] bytes, int nBytesToRead)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        if(_bFirstRead) {
            _bFirstRead = false;

            notifyListeners_open();
        }

        return readJNI( bytes, nBytesToRead );
    }

    /**
     * Write bytes.
     * <p>
     * @param    aData the bytes to write
     * @see       com.sun.star.connection.XConnection#write
     */
    public void write(byte aData[])
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        writeJNI( aData );
    }

    /**
     * Flushes the buffer.
     * <p>
     * @see       com.sun.star.connection.XConnection#flush
     */
    public void flush()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        flushJNI();
    }

    /**
     * Closes the connection.
     * <p>
     * @see       com.sun.star.connection.XConnection#close
     */
    public void close()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        if (DEBUG) System.out.print( "PipeConnection::close() " );
        closeJNI();
        notifyListeners_close();
        if (DEBUG) System.out.println( "done" );
    }

    /**
     * Gives a description of the connection.
     * <p>
     * @return  the description
      * @see       com.sun.star.connection.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _aDescription;
    }

}

