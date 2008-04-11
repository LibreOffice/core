/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ByteArrayToXInputStreamAdapter.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
import java.io.*;

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
            int remain = (int)(m_length - m_pos);
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
