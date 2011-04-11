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

package com.sun.star.sdbcx.comp.hsqldb;

public class StorageAccess implements org.hsqldb.lib.Storage {
    String key;
    String name;
    boolean readonly;
    NativeStorageAccess access;
    /** Creates a new instance of StorageAccess */
    public StorageAccess(String name,Boolean readonly,Object key) throws java.io.IOException{
        this.key = (String)key;
        this.name = name;
        this.readonly = readonly.booleanValue();
        try {
            access = new NativeStorageAccess(name,
                    this.readonly ? "r" : "rw"
                    ,key);
        } catch(Exception e){
            throw new java.io.IOException();
        }
    }
    public void close() throws java.io.IOException{
        access.close(name,key);
    }

    public long getFilePointer() throws java.io.IOException{
        return access.getFilePointer(name,key);
    }

    public long length() throws java.io.IOException{
        return access.length(name,key);
    }

    public int read() throws java.io.IOException{
        return access.read(name,key);
    }

    public void read(byte[] b, int off, int len) throws java.io.IOException{
        access.read(name,key,b,off,len);
    }

    // based on the same code that reads an int from the .data file in HSQLDB
    public int readInt() throws java.io.IOException{
        byte [] tmp = new byte [4];

        int count = access.read(name,key,tmp,0, 4);

        if (count != 4){
            throw new java.io.IOException();
        }

        count = 0;
        int ch0 = tmp[count++] & 0xff;
        int ch1 = tmp[count++] & 0xff;
        int ch2 = tmp[count++] & 0xff;
        int ch3 = tmp[count] & 0xff;

        return ((ch0 << 24) + (ch1 << 16) + (ch2 << 8) + (ch3));
    }

    public void seek(long position) throws java.io.IOException{
        access.seek(name,key,position);
    }

    public void write(byte[] b, int offset, int length) throws java.io.IOException{
        access.write(name,key,b,offset,length);
    }

    public void writeInt(int v) throws java.io.IOException{
        byte [] oneByte = new byte [4];
        oneByte[0] = (byte) ((v >>> 24) & 0xFF);
        oneByte[1] = (byte) ((v >>> 16) & 0xFF);
        oneByte[2] = (byte) ((v >>>  8) & 0xFF);
        oneByte[3] = (byte) ((v >>>  0) & 0xFF);

        write(oneByte,0,4);
    }

    public boolean isReadOnly() {
        return readonly;
    }

    public long readLong() throws java.io.IOException {
        return (((long) readInt()) << 32) + (((long) readInt()) & 0xFFFFFFFFL);
    }

    public boolean wasNio() {
        return false;
    }

    public void writeLong(long v) throws java.io.IOException {
        byte [] oneByte = new byte [8];

        oneByte[0] = (byte) ((v >>> 56) & 0xFF);
        oneByte[1] = (byte) ((v >>> 48) & 0xFF);
        oneByte[2] = (byte) ((v >>> 40) & 0xFF);
        oneByte[3] = (byte) ((v >>> 32) & 0xFF);
        oneByte[4] = (byte) ((v >>> 24) & 0xFF);
        oneByte[5] = (byte) ((v >>> 16) & 0xFF);
        oneByte[6] = (byte) ((v >>>  8) & 0xFF);
        oneByte[7] = (byte) ((v >>>  0) & 0xFF);

        write(oneByte,0,8);
    }
}
