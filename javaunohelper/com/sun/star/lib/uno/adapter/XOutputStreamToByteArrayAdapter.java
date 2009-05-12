/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XOutputStreamToByteArrayAdapter.java,v $
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
 * ByteArrayXOutputStream.java
 *
 * Created on 11. April 2003, 14:20
 */

package com.sun.star.lib.uno.adapter;

/**
 *
 * @author  lo119109
 */

import com.sun.star.io.*;
import  com.sun.star.lib.uno.helper.ComponentBase;

public class XOutputStreamToByteArrayAdapter
    extends ComponentBase
    implements XOutputStream
{
    private int initialSize = 100240; // 10 kb
    private int size = 0;
    private int position = 0;
    private boolean externalBuffer = false;
    private boolean closed = false;
    private byte[] buffer;

    /** Creates a new instance of ByteArrayXOutputStream */
    public XOutputStreamToByteArrayAdapter() {
        this(null);
    }

    public XOutputStreamToByteArrayAdapter(byte[] aBuffer) {
        if (aBuffer != null) {
            externalBuffer = true;
            buffer = aBuffer;
            size = buffer.length;
            // System.err.println("new outputbuffer with external storage");
        } else {
            size = initialSize;
            buffer = new byte[size];
            // System.err.println("new outputbuffer with internal storage");
        }
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public void closeOutput()
        throws com.sun.star.io.NotConnectedException,
            com.sun.star.io.BufferSizeExceededException,
            com.sun.star.io.IOException
    {
        // trim buffer
        if ( buffer.length > position && !externalBuffer )
        {
            byte[] newBuffer = new byte[position];
            System.arraycopy(buffer, 0, newBuffer, 0, position);
            buffer = newBuffer;
        }
        closed = true;
    }

    public void flush()
        throws com.sun.star.io.NotConnectedException,
            com.sun.star.io.BufferSizeExceededException,
            com.sun.star.io.IOException
    {
    }

    public void writeBytes(byte[] values)
        throws com.sun.star.io.NotConnectedException,
            com.sun.star.io.BufferSizeExceededException,
            com.sun.star.io.IOException
    {
        // System.err.println("writeBytes("+values.length+")");
        if ( values.length > size-position )
        {
            if ( externalBuffer )
                throw new BufferSizeExceededException("out of buffer space, cannot grow external buffer");
            byte[] newBuffer = null;
            while ( values.length > size-position )
                size *= 2;
            // System.err.println("new buffer size is "+size+" bytes.");
            newBuffer = new byte[size];
            System.arraycopy(buffer, 0, newBuffer, 0, position);
            buffer = newBuffer;
        }
        System.arraycopy(values, 0, buffer, position, values.length);
        position += values.length;
    }

}
