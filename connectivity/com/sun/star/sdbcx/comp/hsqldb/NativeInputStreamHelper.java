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
 /*
 * NativeInputStreamHelper.java
 *
 * Created on 9. September 2004, 11:51
 */

package com.sun.star.sdbcx.comp.hsqldb;

/**
 *
 * @author  Ocke
 */
public class NativeInputStreamHelper extends java.io.InputStream{
    private String key;
    private String file;
    private StorageNativeInputStream in;
    /** Creates a new instance of NativeInputStreamHelper */
    public NativeInputStreamHelper(String key,String _file) {
        file = _file;
        this.key = key;
        in = new StorageNativeInputStream(key,file);
    }

    public int read() throws java.io.IOException {
        return in.read(key,file);
    }

    public int read(byte[] b, int off, int len) throws java.io.IOException {
        return in.read(key,file,b,off,len);
    }

    public void close() throws java.io.IOException {
        in.close(key,file);
    }

    public long skip(long n) throws java.io.IOException {
        return in.skip(key,file,n);
    }

    public int available() throws java.io.IOException {
        return in.available(key,file);
    }

    public int read(byte[] b) throws java.io.IOException {
        return in.read(key,file,b);
    }

}
