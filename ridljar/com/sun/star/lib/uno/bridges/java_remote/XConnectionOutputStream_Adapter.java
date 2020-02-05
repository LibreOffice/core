/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.uno.bridges.java_remote;


import java.io.IOException;
import java.io.OutputStream;

import com.sun.star.connection.XConnection;


class XConnectionOutputStream_Adapter extends OutputStream {
    private static final boolean DEBUG = false;

    protected XConnection _xConnection;
    protected byte _bytes[] = new byte[1];

    XConnectionOutputStream_Adapter(XConnection xConnection) {
        if(DEBUG) System.err.println("#### " + this.getClass()  + " - instantiated ");

        _xConnection = xConnection;
    }

    @Override
    public void write(int b) throws IOException {
        _bytes[0] = (byte)b;

        try {
            _xConnection.write(_bytes);
        } catch(com.sun.star.io.IOException ioException) {
            IOException ex = new IOException(ioException.getMessage());
            ex.initCause(ioException);
            throw ex;
        }

        if(DEBUG) System.err.println("#### " + this.getClass()  + " - one byte written:" +  _bytes[0]);
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException {
        byte bytes[]  ;

        if(off == 0 && len == b.length) {
            bytes = b;
        } else {
            bytes = new byte[len];

            System.arraycopy(b, off, bytes, 0, len);
        }

        try {
            _xConnection.write(bytes);
        } catch(com.sun.star.io.IOException ioException) {
            IOException ex = new IOException(ioException.getMessage());
            ex.initCause(ioException);
            throw ex;
        }
    }

    @Override
    public void flush() throws IOException {
        try {
            _xConnection.flush();
        } catch(com.sun.star.io.IOException ioException) {
            IOException ex = new IOException(ioException.getMessage());
            ex.initCause(ioException);
            throw ex;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
