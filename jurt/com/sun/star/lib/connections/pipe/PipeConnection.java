/*************************************************************************
 *
 *  $RCSfile: PipeConnection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:32:55 $
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
package com.sun.star.lib.connections.pipe;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.util.StringTokenizer;
import java.util.Enumeration;
import java.util.Vector;


import com.sun.star.io.XStreamListener;

import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnectionBroadcaster;

/**
 * The PipeConnection implements the <code>XConnection</code> interface
 * and is uses by the <code>PipeConnector</code> and the <code>PipeAcceptor</code>.
 * This class is not part of the provided <code>api</code>.
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2003-03-26 12:32:55 $
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
    static private final String PIPE_LIB_NAME = "jpipe";

    static
    {

        // determine name of executable soffice
        String aExec = "soffice"; // default for UNIX
        String aOS = System.getProperty("os.name");

        // running on Windows?
        if (aOS.startsWith("Windows"))
            aExec = "soffice.exe";

        // add other non-UNIX operating systems here

        // find soffice executable via CLASSPATH:
        // <INSTDIR>/program/classes/*.jar => <INSTDIR>/program/soffice
        String mProgramPath = null;
        String aClassPath = System.getProperty("java.class.path" );
        java.util.StringTokenizer aTokenizer = new java.util.StringTokenizer(
            aClassPath, java.io.File.pathSeparator );
        while ( mProgramPath == null && aTokenizer.hasMoreTokens() )
        {
            java.io.File aJAR = new java.io.File( aTokenizer.nextToken() ).getAbsoluteFile();
            String aPath = aJAR.getParentFile().getParent();
            java.io.File aProgFile = new java.io.File(
                aPath + java.io.File.separator + aExec );
            if ( aProgFile.exists() )
                mProgramPath = aProgFile.getParent();

        }

                // load libofficebean.so/officebean.dll
                System.load( mProgramPath + java.io.File.separator +
                        System.mapLibraryName(PIPE_LIB_NAME) );
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
        String aPipeName = null;
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

