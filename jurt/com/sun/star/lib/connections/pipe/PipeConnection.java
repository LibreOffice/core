/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.lib.connections.pipe;

import java.io.IOException;

import java.util.StringTokenizer;
import java.util.Enumeration;
import java.util.Vector;

import com.sun.star.lib.util.NativeLibraryLoader;

import com.sun.star.io.XStreamListener;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;

/**
 * The PipeConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>PipeConnector</code> and the <code>PipeAcceptor</code>.
 * This class is not part of the provided <code>api</code>.
 * <p>
 * @version     $Revision: 1.7 $ $ $Date: 2008-04-11 11:13:00 $
 * @author      Kay Ramme
 * @see         com.sun.star.comp.connections.PipeAcceptor
 * @see         com.sun.star.comp.connections.PipeConnector
 * @see         com.sun.star.connections.XConnection
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
    protected Vector    _aListeners;
    protected boolean   _bFirstRead;

    /**
     * Constructs a new <code>PipeConnection</code>.
     * <p>
     * @param  description   the description of the connection
     * @param  pipe        the pipe of the connection
     */
    public PipeConnection(String description)
        throws IOException
    {
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - instantiated " + description );

        _aListeners = new Vector();
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
        _aListeners.addElement(aListener);
    }

    public void removeStreamListener(XStreamListener aListener ) throws com.sun.star.uno.RuntimeException {
        _aListeners.removeElement(aListener);
    }

    private void notifyListeners_open() {
        Enumeration elements = _aListeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
            xStreamListener.started();
        }
    }

    private void notifyListeners_close() {
        Enumeration elements = _aListeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
            xStreamListener.closed();
        }
    }

    private void notifyListeners_error(com.sun.star.uno.Exception exception) {
        Enumeration elements = _aListeners.elements();
        while(elements.hasMoreElements()) {
            XStreamListener xStreamListener = (XStreamListener)elements.nextElement();
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
     * @param    aReadBytes   the outparameter, where the bytes have to be placed
     * @param    nBytesToRead the number of bytes to read
     * @see       com.sun.star.connections.XConnection#read
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
     * @see       com.sun.star.connections.XConnection#write
     */
    public void write(byte aData[])
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        writeJNI( aData );
    }

    /**
     * Flushes the buffer.
     * <p>
     * @see       com.sun.star.connections.XConnection#flush
     */
    public void flush()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException
    {
        flushJNI();
    }

    /**
     * Closes the connection.
     * <p>
     * @see       com.sun.star.connections.XConnection#close
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
      * @see       com.sun.star.connections.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _aDescription;
    }

}

