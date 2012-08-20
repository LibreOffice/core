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

/*
 * ByteArrayXInputStram.java
 *
 * Created on 10. April 2003, 15:45
 */

package com.sun.star.lib.uno.adapter;

/**
 *
 * @author  lo119109
 */

import com.sun.star.io.XInputStream;
import com.sun.star.io.XSeekable;
import  com.sun.star.lib.uno.helper.ComponentBase;

public class ByteArrayToXInputStreamAdapter
    extends ComponentBase
    implements XInputStream, XSeekable
{

    byte[] m_bytes;
    int m_length;
    int m_pos;

    boolean m_open;

    /** Creates a new instance of ByteArrayXInputStram */
    public ByteArrayToXInputStreamAdapter(byte[] bytes) {
        init(bytes);
    }

    public void init(byte[] bytes) {
        // System.err.println("ByteArrayXInputStream");
        m_bytes = bytes;
        m_length = bytes.length;
        m_pos = 0;
        m_open = true;
    }

    private void _check() throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException {
        if (m_bytes == null) {
            // System.err.println("check failed no bytes!");
            throw new com.sun.star.io.NotConnectedException("no bytes");
        }
        if(!m_open) {
            // System.err.println("check failed: closed");
            throw new com.sun.star.io.IOException("input closed");
        }
    }

    public int available() throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException {
        _check();
        long a = m_length - m_pos;
        if (a != (int)a)
            throw new com.sun.star.io.IOException("integer overflow");
        else {
            // System.err.println("available() -> "+a);
            return (int)a;
        }
    }

    public void closeInput() throws com.sun.star.io.NotConnectedException, com.sun.star.io.IOException {
        // System.err.println("closeInput()");
        _check();
        m_open = false;
    }

    public int readBytes(byte[][] values, int param) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {
        // System.err.println("readbytes(..., "+param+")");
        _check();
    try {
            int remain = (m_length - m_pos);
            if (param > remain) param = remain;
            /* ARGH!!! */
            if (values[0] == null){
                values[0] = new byte[param];
                // System.err.println("allocated new buffer of "+param+" bytes");
            }
            System.arraycopy(m_bytes, m_pos, values[0], 0, param);
            // System.err.println("readbytes() -> "+param);
            m_pos += param;
            return param;
        } catch (ArrayIndexOutOfBoundsException ae) {
            // System.err.println("readbytes() -> ArrayIndexOutOfBounds");
            ae.printStackTrace();
            throw new com.sun.star.io.BufferSizeExceededException("buffer overflow");
        } catch (Exception e) {
            // System.err.println("readbytes() -> Exception: "+e.getMessage());
            e.printStackTrace();
            throw new com.sun.star.io.IOException("error accessing buffer");
        }
    }

    public int readSomeBytes(byte[][] values, int param) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {
        // System.err.println("readSomebytes()");
        return readBytes(values, param);
    }

    public void skipBytes(int param) throws com.sun.star.io.NotConnectedException, com.sun.star.io.BufferSizeExceededException, com.sun.star.io.IOException {
        // System.err.println("skipBytes("+param+")");
        _check();
        if (param > (m_length - m_pos))
            throw new com.sun.star.io.BufferSizeExceededException("buffer overflow");
        m_pos += param;
    }

    public long getLength() throws com.sun.star.io.IOException {
        // System.err.println("getLength() -> "+m_length);
        if (m_bytes != null) return m_length;
        else throw new com.sun.star.io.IOException("no bytes");
    }

    public long getPosition() throws com.sun.star.io.IOException {
        // System.err.println("getPosition() -> "+m_pos);
        if (m_bytes != null) return m_pos;
        else throw new com.sun.star.io.IOException("no bytes");
    }

    public void seek(long param) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.io.IOException {
        // System.err.println("seek("+param+")");
        if (m_bytes != null){
            if (param < 0 || param > m_length) throw new com.sun.star.lang.IllegalArgumentException("invalid seek position");
            else m_pos = (int)param;
        }else throw new com.sun.star.io.IOException("no bytes");
    }

    public void finalize() throws Throwable{
        // System.err.println("finalizer called for ByteArrayXInputStream!");
        super.finalize();
    }

}
