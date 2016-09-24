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

package com.sun.star.script.framework.io;

import com.sun.star.io.XInputStream;

import java.io.IOException;
import java.io.InputStream;

public class XInputStreamImpl implements XInputStream {

    private final InputStream is;

    public XInputStreamImpl(InputStream is) {
        this.is = is;
    }

    public int readBytes(/*OUT*/byte[][] aData, /*IN*/int nBytesToRead) throws
        com.sun.star.io.NotConnectedException,
        com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {

        aData[ 0 ] = new byte[ nBytesToRead ];
        int totalBytesRead = 0;

        try {
            int bytesRead;

            while ((bytesRead = is.read(aData[ 0 ], totalBytesRead, nBytesToRead)) > 0
                   && (totalBytesRead < nBytesToRead)) {
                totalBytesRead += bytesRead;
                nBytesToRead -= bytesRead;
            }
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        } catch (IndexOutOfBoundsException aie) {
            throw new com.sun.star.io.BufferSizeExceededException(aie);
        }

        return totalBytesRead;
    }

    public int readSomeBytes(/*OUT*/byte[][] aData, /*IN*/int nMaxBytesToRead)
    throws com.sun.star.io.NotConnectedException,
        com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {

        int bytesToRead = nMaxBytesToRead;
        int availableBytes = available();

        if (availableBytes < nMaxBytesToRead) {
            bytesToRead = availableBytes;
        }

        int read =  readBytes(aData, bytesToRead);
        return read;
    }

    public void skipBytes(/*IN*/int nBytesToSkip) throws
        com.sun.star.io.NotConnectedException,
        com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {

        try {
            do {
                nBytesToSkip -= is.skip(nBytesToSkip);
            } while (nBytesToSkip > 0);
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }
    }

    public int available() throws
        com.sun.star.io.NotConnectedException, com.sun.star.io.IOException {

        int bytesAvail = 0;

        try {
            bytesAvail = is.available();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }

        return bytesAvail;
    }

    public void closeInput() throws
        com.sun.star.io.NotConnectedException, com.sun.star.io.IOException {

        try {
            is.close();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e);
        }
    }
}