/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
