/*************************************************************************
 *
 *  $RCSfile: PipedConnection.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.comp.connections;


import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnection;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

/**
 * The PipedConnection is a component that implements the
 * <code>XConnection</code> Interface.
 * It is useful for <code>Thread</code> communication
 * in one Process.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:51 $
 * @author      Kay Ramme
 * @see         com.sun.star.connections.XConnection
 * @see         com.sun.star.loader.JavaLoader
 * @since       UDK1.0
 */
public class PipedConnection implements XConnection {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;

    /**
     * The name of the service, the <code>JavaLoader</code> acceses this through reflection.
     */
    static private final String __serviceName = "com.sun.star.connection.PipedConnection";

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be uses if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                          XMultiServiceFactory multiFactory,
                                                          XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(PipedConnection.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(PipedConnection.class,
                                                                    __serviceName,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(PipedConnection.class.getName(), __serviceName, regKey);
    }


    /**
     * The amount of time in milliseconds, to wait to
     * see check the buffers.
     */
    protected static final int __waitTime = 10000;

    protected byte             _buffer[] = new byte[4096];
    protected int              _in,
                               _out;
    protected boolean          _closed;
    protected PipedConnection  _otherSide;

    /**
     * Constructs a new <code>PipedConnection</code>, sees if there
     * is an other side, which it should be connected to.
     * <p>
     * @param    args   Another side could be in index 0.
     */
    public PipedConnection(Object args[]) throws com.sun.star.uno.RuntimeException {
        if (DEBUG) System.err.println("##### " + getClass().getName() + " - instantiated");

        _otherSide = (args.length == 1) ? (PipedConnection)args[0] : null;
        if(_otherSide != null) {
            if(_otherSide == this)
                throw new RuntimeException("can not connect to myself");

            _otherSide._otherSide = this;
        }
    }

    /**
     * This is a private method, used to cummunicate
     * internal in the pipe.
     */
    private synchronized void receive(byte aData[]) throws com.sun.star.io.IOException {
        int bytesWritten = 0;

        if(DEBUG) System.err.println("##### PipedConnection.receive - bytes:" + aData.length + " at:" + _out);

        while(bytesWritten < aData.length) {
            // wait until it is not full anymore
            while(_out == (_in - 1) || (_in == 0 && _out == _buffer.length - 1)) {
                try {
                    notify(); // the buffer is full, signal it

                    wait(__waitTime);
                }
                catch(InterruptedException interruptedException) {
                    throw new com.sun.star.io.IOException(interruptedException.toString());
                }
            }

            if(_closed) throw new com.sun.star.io.IOException("connection has been closed");

            int bytes = 0;

            if(_out < _in) {
                bytes = Math.min(aData.length - bytesWritten, _in - _out - 1);

                System.arraycopy(aData, bytesWritten, _buffer, _out, bytes);
            }
            else {
                if(_in > 0){
                    bytes = Math.min(aData.length - bytesWritten, _buffer.length - _out);
                }
                else {
                    bytes = Math.min(aData.length - bytesWritten, _buffer.length - _out - 1);
                }

                System.arraycopy(aData, bytesWritten, _buffer, _out, bytes);
            }

            bytesWritten += bytes;
            _out += bytes;
            if(_out >= _buffer.length)
                _out = 0;
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
    public synchronized int read(/*OUT*/byte[][] aReadBytes, int nBytesToRead) throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        aReadBytes[0] = new byte[nBytesToRead];

        if(DEBUG) System.err.println("##### PipedConnection.read - bytes:" + nBytesToRead + " at:" + _in);

        // loop while not all bytes read or when closed but there is still data
        while(nBytesToRead > 0 && (_in != _out || !_closed)) {
            while(_in == _out && !_closed) {
                try {
                    notify(); // the buffer is empty, signal it

                    wait(__waitTime); // we wait for data or for the pipe to be closed
                }
                catch(InterruptedException interruptedException) {
                    throw new com.sun.star.io.IOException(interruptedException.toString());
                }
            }

            if(_in < _out) {
                int bytes = Math.min(nBytesToRead, _out - _in);

                System.arraycopy(_buffer, _in, aReadBytes[0], aReadBytes[0].length - nBytesToRead, bytes);

                nBytesToRead -= bytes;
                _in += bytes;
            }
            else if(_in > _out) {
                int bytes = Math.min(nBytesToRead, _buffer.length - _in);

                System.arraycopy(_buffer, _in, aReadBytes[0], aReadBytes[0].length - nBytesToRead, bytes);

                nBytesToRead -= bytes;
                _in += bytes;
                if(_in >= _buffer.length)
                    _in = 0;
            }
        }

        if(nBytesToRead > 0) { // not all bytes read
            byte tmp[] = new byte[aReadBytes[0].length - nBytesToRead];
            System.arraycopy(aReadBytes[0], 0, tmp, 0, tmp.length);

            aReadBytes[0] = tmp;
        }

        return aReadBytes[0].length;
    }

    /**
     * Write bytes.
     * <p>
     * @param    aData the bytes to write
     * @see       com.sun.star.connections.XConnection#write
     */
    public void write(byte aData[]) throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        _otherSide.receive(aData);
    }

    /**
     * Flushes the buffer, notifies if necessary the other side that new data has arrived.
     * <p>
     * @see       com.sun.star.connections.XConnection#flush
     */
    public void flush() throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        synchronized(_otherSide) {
            _otherSide.notify();
        }
    }

    /**
     * Closes the pipe.
     * <p>
     * @see       com.sun.star.connections.XConnection#closed
     */
    public synchronized void close() throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException {
        if(!_closed) {
            _closed = true;

            _otherSide.close();

            notify();
        }
    }

    /**
     * Gives a description of this pipe.
     * <p>
     * @return  the description
     * @see       com.sun.star.connections.XConnection#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return getClass().getName();
    }

}

