/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XInputStreamWrapper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:59:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
