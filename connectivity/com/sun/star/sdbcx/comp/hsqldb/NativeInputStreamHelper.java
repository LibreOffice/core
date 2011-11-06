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
