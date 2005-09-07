/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XConnectionOutputStream_Adapter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:57:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.java_remote;


import java.io.IOException;
import java.io.OutputStream;

import com.sun.star.connection.XConnection;


class XConnectionOutputStream_Adapter extends OutputStream {
    static private final boolean DEBUG = false;

    protected XConnection _xConnection;
    protected byte _bytes[] = new byte[1];

    XConnectionOutputStream_Adapter(XConnection xConnection) {
        if(DEBUG) System.err.println("#### " + this.getClass()  + " - instantiated ");

        _xConnection = xConnection;
    }

    public void write(int b) throws IOException {
        _bytes[0] = (byte)b;

        try {
            _xConnection.write(_bytes);
        }
        catch(com.sun.star.io.IOException ioException) {
            throw new IOException(ioException.toString());
        }

        if(DEBUG) System.err.println("#### " + this.getClass()  + " - one byte written:" +  _bytes[0]);
    }

    public void write(byte[] b, int off, int len) throws IOException {
        byte bytes[] = null;

        if(off == 0 && len == b.length)
            bytes = b;

        else {
            bytes = new byte[len];

            System.arraycopy(b, off, bytes, 0, len);
        }

        try {
            _xConnection.write(bytes);
        }
        catch(com.sun.star.io.IOException ioException) {
            throw new IOException(ioException.toString());
        }
    }

    public void flush() throws IOException {
        try {
            _xConnection.flush();
        }
        catch(com.sun.star.io.IOException ioException) {
            throw new IOException(ioException.toString());
        }
    }
}

