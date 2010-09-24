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

package com.sun.star.beans;

import com.sun.star.connection.XConnection;
/* @deprecated
 */
/* package */ class NativeConnection
    implements XConnection
{
    public native void connect(NativeService aNativeService)
        throws com.sun.star.io.IOException;

    public native int read(/*OUT*/byte[][] aReadBytes, /*IN*/int nBytesToRead)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void write(/*IN*/byte[] aData)
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void flush()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public native void close()
        throws com.sun.star.io.IOException, com.sun.star.uno.RuntimeException;

    public synchronized String getDescription()
        throws com.sun.star.uno.RuntimeException
    {
        return Description;
    }

    private long    NativeHandle;
    private String  Description;
}
