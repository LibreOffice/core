/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

