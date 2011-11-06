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

