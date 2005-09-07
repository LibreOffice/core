/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XOutputStreamToByteArrayAdapter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:38:41 $
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
