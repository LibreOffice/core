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
 * StorageNativeInputStream.java
 *
 * Created on 9. September 2004, 11:52
 */

package com.sun.star.sdbcx.comp.hsqldb;

/**
 *
 * @author  Ocke
 */
public class StorageNativeInputStream {
    static { NativeLibraries.load(); }

    /** Creates a new instance of StorageNativeInputStream */
    public StorageNativeInputStream(String key,String _file) {
        openStream(key,_file, NativeStorageAccess.READ);
    }
    public native void openStream(String key,String name, int mode);
    public native int read(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b, int off, int len) throws java.io.IOException;
    public native void close(String key,String name) throws java.io.IOException;
    public native long skip(String key,String name,long n) throws java.io.IOException;
    public native int available(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b) throws java.io.IOException;
}
