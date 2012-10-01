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
 * ByteArrayXOutputStream.java
 *
 * Created on 11. April 2003, 14:20
 */

package com.sun.star.lib.uno.adapter;

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
