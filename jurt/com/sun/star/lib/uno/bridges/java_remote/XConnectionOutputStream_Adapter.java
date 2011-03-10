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
        byte bytes[]  ;

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

