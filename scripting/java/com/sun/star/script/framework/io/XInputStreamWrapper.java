/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.script.framework.io;

import java.io.InputStream;
import com.sun.star.io.XInputStream;

public class XInputStreamWrapper extends InputStream {
    private XInputStream m_xInputStream;

    public XInputStreamWrapper(XInputStream xInputStream) {
        m_xInputStream = xInputStream;
    }

    public int read() throws java.io.IOException
    {
        byte[][] byteRet = new byte[1][0];
        long numRead;

        try {
            numRead = m_xInputStream.readBytes(byteRet, 1);
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }

        if (numRead != 1) {
            return -1;
        }
        return byteRet[0][0];
    }

    public int read( byte[] b ) throws java.io.IOException
    {
        byte[][] byteRet = new byte[1][];
        byteRet[0] = b;
        try
        {
           return m_xInputStream.readBytes( byteRet, b.length );
        }
        catch ( com.sun.star.io.IOException ioe)
        {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public long skip(long n) throws java.io.IOException
    {
        try {
            m_xInputStream.skipBytes((int)n);
            return n;
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public int available() throws java.io.IOException
    {
        try {
            return m_xInputStream.available();
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public void close() throws java.io.IOException
    {
        try {
            m_xInputStream.closeInput();
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

}
