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
import java.io.InputStream;

import com.sun.star.connection.XConnection;


class XConnectionInputStream_Adapter extends InputStream {
    private static final boolean DEBUG = false;

    protected XConnection _xConnection;
    protected byte        _bytes[][] = new byte[1][];

    XConnectionInputStream_Adapter(XConnection xConnection) {
        if(xConnection == null) throw new NullPointerException("the XConnection must not be null");

        if(DEBUG) System.err.println("#### " + getClass().getName()  + " - instantiated ");

        _xConnection = xConnection;
    }

    @Override
    public int read() throws IOException {
        int len;

        try {
            len = _xConnection.read(_bytes, 1);
        } catch(com.sun.star.io.IOException ioException) {
            IOException ex = new IOException(ioException.getMessage());
            ex.initCause(ioException);
            throw ex;
        }

        if(DEBUG) System.err.println("#### " + getClass().getName()  + " - one byte read:" +  _bytes[0][0]);

        return len == 0 ? -1 : _bytes[0][0] & 0xff;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        try {
            len = _xConnection.read(_bytes, len - off);
        } catch(com.sun.star.io.IOException ioException) {
            IOException ex = new IOException(ioException.getMessage());
            ex.initCause(ioException);
            throw ex;
        }

        System.arraycopy(_bytes[0], 0, b, off, len);

        return len == 0 ? -1 : len;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
