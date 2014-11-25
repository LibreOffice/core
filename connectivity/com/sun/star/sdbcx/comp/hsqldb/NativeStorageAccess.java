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
package com.sun.star.sdbcx.comp.hsqldb;

public class NativeStorageAccess {
    static { NativeLibraries.load(); }

    public static final int READ            = 1;
    private static final int SEEKABLE        = 2;
    private static final int SEEKABLEREAD    = 3;
    public static final int WRITE           = 4;
    private static final int READWRITE       = 7;
    public static final int TRUNCATE        = 8;

    /** Creates a new instance of StorageAccess */
    public NativeStorageAccess(String name,String _mode,Object key) throws java.io.IOException{
        try {
            int mode = NativeStorageAccess.SEEKABLEREAD;
            if ( _mode.equals("rw") )
                mode = NativeStorageAccess.READWRITE | NativeStorageAccess.SEEKABLE;

            openStream(name, (String)key, mode);
        } catch(Exception ex1){
            java.io.IOException ex2 = new java.io.IOException();
            ex2.initCause(ex1);
            throw ex2;
        }
    }
    private native void openStream(String name,String key, int mode);
    public native void close(String name,String key) throws java.io.IOException;

    public native long getFilePointer(String name,String key) throws java.io.IOException;

    public native long length(String name,String key) throws java.io.IOException;

    public native int read(String name,String key) throws java.io.IOException;

    public native int read(String name,String key,byte[] b, int off, int len) throws java.io.IOException;



    public native void seek(String name,String key,long position) throws java.io.IOException;

    public native void write(String name,String key,byte[] b, int offset, int length) throws java.io.IOException;


}
