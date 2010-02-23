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
 * StorageAccess.java
 *
 * Created on 17. August 2004, 13:32
 */

package com.sun.star.sdbcx.comp.hsqldb;

/**
 *
 * @author  oj93728
 */

public class NativeStorageAccess {
    static { NativeLibraries.load(); }

    public static final int READ            = 1;
    public static final int SEEKABLE        = 2;
    public static final int SEEKABLEREAD    = 3;
    public static final int WRITE           = 4;
    public static final int READWRITE       = 7;
    public static final int TRUNCATE        = 8;

    /** Creates a new instance of StorageAccess */
    public NativeStorageAccess(String name,String _mode,Object key) throws java.io.IOException{
        try {
            int mode = NativeStorageAccess.SEEKABLEREAD;
            if ( _mode.equals("rw") )
                mode = NativeStorageAccess.READWRITE | NativeStorageAccess.SEEKABLE;

            openStream(name, (String)key, mode);
        } catch(Exception e){
            throw new java.io.IOException();
        }
    }
    public native void openStream(String name,String key, int mode);
    public native void close(String name,String key) throws java.io.IOException;

    public native long getFilePointer(String name,String key) throws java.io.IOException;

    public native long length(String name,String key) throws java.io.IOException;

    public native int read(String name,String key) throws java.io.IOException;

    public native int read(String name,String key,byte[] b, int off, int len) throws java.io.IOException;

    public native int readInt(String name,String key) throws java.io.IOException;

    public native void seek(String name,String key,long position) throws java.io.IOException;

    public native void write(String name,String key,byte[] b, int offset, int length) throws java.io.IOException;

    public native void writeInt(String name,String key,int v) throws java.io.IOException;
}
