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

import com.sun.star.io.XOutputStream;

import java.io.IOException;
import java.io.OutputStream;

public class XOutputStreamWrapper extends OutputStream {

    private final XOutputStream m_xOutputStream;

    public XOutputStreamWrapper(XOutputStream xOs) {
        this.m_xOutputStream = xOs;
    }

    @Override
    public void write(int b) throws java.io.IOException {
        if (m_xOutputStream == null) {
            throw new java.io.IOException("Stream is null");
        }

        byte[] bytes = new byte[1];
        bytes[0] = (byte) b;

        try {
            m_xOutputStream.writeBytes(bytes);
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }

    @Override
    public void write(byte[] b) throws java.io.IOException {

        if (m_xOutputStream == null) {
            throw new java.io.IOException("Stream is null");
        }

        try {
            m_xOutputStream.writeBytes(b);
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }
    @Override
    public void write(byte[] b, int off, int len) throws java.io.IOException {
        if (m_xOutputStream == null) {
            throw new java.io.IOException("Stream is null");
        }

        byte[] bytes = new byte[len];
        System.arraycopy(b, off, bytes, 0, len);

        try {
            m_xOutputStream.writeBytes(bytes);
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }

    @Override
    public void flush() throws java.io.IOException {
        if (m_xOutputStream == null) {
            throw new java.io.IOException("Stream is null");
        }

        try {
            m_xOutputStream.flush();
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }

    @Override
    public void close() throws java.io.IOException {
        if (m_xOutputStream == null) {
            throw new java.io.IOException("Stream is null");
        }

        try {
            m_xOutputStream.closeOutput();
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }
}