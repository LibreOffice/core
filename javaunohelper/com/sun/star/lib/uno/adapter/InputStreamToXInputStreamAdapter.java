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
        if (len >iIn.available()) {
            bytesRead = iIn.read(b[0], 0, iIn.available());
        }
        else{
            bytesRead = iIn.read(b[0], 0, len);
        }
            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.

            if (bytesRead <= 0) {
                return 0;
        }
        return ((int)bytesRead);


        } catch (IOException e) {
            throw new com.sun.star.io.IOException("reader error", e);
        }
    }

    public int readSomeBytes(byte[][] b, int len) throws
            com.sun.star.io.IOException
    {
        try {
        long bytesRead;
        if (len >iIn.available()) {
            bytesRead = iIn.read(b[0], 0, iIn.available());
        }
        else{
            bytesRead = iIn.read(b[0], 0, len);
        }
            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.

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

