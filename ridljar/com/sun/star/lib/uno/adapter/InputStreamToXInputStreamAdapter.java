/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

import com.sun.star.io.XInputStream;

import java.io.InputStream;

/** The <code>InputStreamToInputXStreamAdapter</code> wraps the
    Java <code>InputStream</code> object into a
    UNO <code>XInputStream</code> object.
    This allows users to access an <code>InputStream</code>
    as if it were an <code>XInputStream</code>.
 */
public final class InputStreamToXInputStreamAdapter implements XInputStream {

    /**
     *  Internal store to the InputStream
     */
    private final InputStream iIn;

    /**
     *  Constructor.
     *
     *  @param  in  The <code>XInputStream</code> to be
     *              accessed as an <code>InputStream</code>.
     */
    public InputStreamToXInputStreamAdapter (InputStream in)
    {
        iIn = in;
    }

    public int available() throws
            com.sun.star.io.IOException
    {

        int bytesAvail;

        try {
            bytesAvail = iIn.available();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }

        return bytesAvail;
    }

    public void closeInput() throws
            com.sun.star.io.IOException
    {
        try {
            iIn.close();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }
    }

    public int readBytes(byte[][] b, int len) throws
            com.sun.star.io.IOException
    {
        try {
            long bytesRead;
            int totalBytesRead = 0;
            if (b[0] == null || b[0].length < len) {
                b[0] = new byte[len];
            }

            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.
            while ((len > 0) && ((bytesRead = iIn.read(b[0], totalBytesRead, len)) > 0)) {
                totalBytesRead += (int)bytesRead;
                len -= (int)bytesRead;
            }
            if (totalBytesRead < b[0].length) {
                byte[] out = new byte[totalBytesRead];
                System.arraycopy(b[0], 0, out, 0, totalBytesRead);
                b[0] = out;
            }
            return totalBytesRead;
        } catch (IOException e) {
            throw new com.sun.star.io.IOException("reader error", e);
        }
    }

    public int readSomeBytes(byte[][] b, int len) throws
            com.sun.star.io.IOException
    {
        try {
            long bytesRead;
            if (b[0] == null || b[0].length < len) {
                b[0] = new byte[len];
            }
            if (len >iIn.available()) {
                bytesRead = iIn.read(b[0], 0, iIn.available());
            }
            else{
                bytesRead = iIn.read(b[0], 0, len);
            }

            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.
            if (bytesRead < b[0].length) {
                int outSize = bytesRead > 0 ? (int)bytesRead : 0;
                byte[] out = new byte[outSize];
                System.arraycopy(b[0], 0, out, 0, outSize);
                b[0] = out;
            }
            if (bytesRead <= 0) {
                return 0;
            }
            return ((int)bytesRead);
        } catch (IOException e) {
            throw new com.sun.star.io.IOException("reader error", e);
        }
    }

    public void skipBytes(int n) throws
            com.sun.star.io.IOException
    {
        try {
            iIn.available();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }

        do {
            try {
                n -= iIn.skip(n);
            } catch (IOException e) {
                throw new com.sun.star.io.IOException(e);
            }
        } while (n > 0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
