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

public class NativeOutputStreamHelper extends java.io.OutputStream{

    private final String key;
    private final String file;
    private final StorageNativeOutputStream out;
    /** Creates a new instance of NativeOutputStreamHelper */
    public NativeOutputStreamHelper(String key,String _file) {
        file = _file;
        this.key = key;
        out = new StorageNativeOutputStream(file,key);
    }

    @Override
    public void write(byte[] b, int off, int len)  throws java.io.IOException{
        out.write(key,file,b, off, len);
    }

    @Override
    public void write(byte[] b)  throws java.io.IOException{
        out.write(key,file,b);
    }

    @Override
    public void close()  throws java.io.IOException{
        out.close(key,file);
    }

    @Override
    public void write(int b)  throws java.io.IOException{
        out.write(key,file,b);
    }

    @Override
    public void flush()  throws java.io.IOException{
        out.flush(key,file);
    }

    public void sync()  throws java.io.IOException{
        out.sync(key,file);
    }
}
