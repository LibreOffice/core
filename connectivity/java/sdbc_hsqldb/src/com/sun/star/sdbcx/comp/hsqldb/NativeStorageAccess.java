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
