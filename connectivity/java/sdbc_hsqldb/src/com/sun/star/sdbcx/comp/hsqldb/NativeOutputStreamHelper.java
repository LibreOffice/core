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
 * NativeOutputStreamHelper.java
 *
 * Created on 1. September 2004, 10:39
 */

package com.sun.star.sdbcx.comp.hsqldb;

/**
 *
 * @author  oj93728
 */
public class NativeOutputStreamHelper extends java.io.OutputStream{

    private String key;
    private String file;
    private StorageNativeOutputStream out;
    /** Creates a new instance of NativeOutputStreamHelper */
    public NativeOutputStreamHelper(String key,String _file) {
        file = _file;
        this.key = key;
        out = new StorageNativeOutputStream(file,key);
    }

    public void write(byte[] b, int off, int len)  throws java.io.IOException{
        out.write(key,file,b, off, len);
    }

    public void write(byte[] b)  throws java.io.IOException{
        out.write(key,file,b);
    }

    public void close()  throws java.io.IOException{
        out.close(key,file);
    }

    public void write(int b)  throws java.io.IOException{
        out.write(key,file,b);
    }

    public void flush()  throws java.io.IOException{
        out.flush(key,file);
    }

    public void sync()  throws java.io.IOException{
        out.sync(key,file);
    }
}
