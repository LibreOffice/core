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

        int len  ;

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

