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
 * NativeInputStreamHelper.java
 *
 * Created on 9. September 2004, 11:51
 */

package com.sun.star.sdbcx.comp.hsqldb;

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
