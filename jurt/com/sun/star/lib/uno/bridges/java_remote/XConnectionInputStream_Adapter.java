/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XConnectionInputStream_Adapter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:57:33 $
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
import java.io.InputStream;

import com.sun.star.connection.XConnection;


class XConnectionInputStream_Adapter extends InputStream {
    static private final boolean DEBUG = false;

    protected XConnection _xConnection;
    protected byte        _bytes[][] = new byte[1][];

    XConnectionInputStream_Adapter(XConnection xConnection) {
        if(xConnection == null) throw new NullPointerException("the XConnection must not be null");

        if(DEBUG) System.err.println("#### " + getClass().getName()  + " - instantiated ");

        _xConnection = xConnection;
    }

    public int read() throws IOException {

        int len = 0;

        try {
            len = _xConnection.read(_bytes, 1);
        }
        catch(com.sun.star.io.IOException ioException) {
            throw new IOException(ioException.toString());
        }

        if(DEBUG) System.err.println("#### " + getClass().getName()  + " - one byte read:" +  _bytes[0][0]);

        return len == 0 ? -1 : _bytes[0][0] & 0xff;
    }

    public int read(byte[] b, int off, int len) throws IOException {
//      byte bytes[][] = new byte[1][];

        try {
            len = _xConnection.read(_bytes, len - off);
        }
        catch(com.sun.star.io.IOException ioException) {
            throw new IOException(ioException.toString());
        }

        System.arraycopy(_bytes[0], 0, b, off, len);

        return len == 0 ? -1 : len;
    }
}

