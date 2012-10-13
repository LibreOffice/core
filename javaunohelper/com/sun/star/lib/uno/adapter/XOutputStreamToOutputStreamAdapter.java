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

package com.sun.star.lib.uno.adapter;

import java.io.IOException;
import com.sun.star.io.XOutputStream;
import java.io.OutputStream;

/**
 * The <code>XOutputStreamToOutputStreamAdapter</code> wraps
 * the UNO <code>XOutputStream</code> object in a Java
 * <code>OutputStream</code>.  This allows users to access
 * an <code>XOutputStream</code> as if it were an
 * <code>OutputStream</code>.
 */
public class XOutputStreamToOutputStreamAdapter extends OutputStream {

    /**
     *  Internal handle to the XInputStream
     */
    XOutputStream xout;

    /**
     *  Constructor.
     *
     *  @param  out  The <code>XOutputStream</code> to be
     *          accessed as an <code>OutputStream</code>.
     */
    public XOutputStreamToOutputStreamAdapter(XOutputStream out) {
        xout = out;
    }

    public void close() throws IOException {
        try {
            xout.closeOutput();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void flush() throws IOException {
        try {
            xout.flush();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(byte[] b) throws IOException {

    try {
            xout.writeBytes(b);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(byte[] b, int off, int len) throws IOException {

        byte[] tmp = new byte[len];

        // Copy the input array into a temp array, and write it out.
        //
        System.arraycopy(b, off, tmp, 0, len);

        try {
            xout.writeBytes(tmp);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(int b) throws IOException {

        byte [] oneByte = new byte [1];
        oneByte[0] = (byte) b;

        try {
            xout.writeBytes(oneByte);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }
}
