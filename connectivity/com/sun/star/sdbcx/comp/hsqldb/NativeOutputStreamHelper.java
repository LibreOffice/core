/*
 * NativeOutputStreamHelper.java
 *
 * Created on 1. September 2004, 10:39
 */

package com.sun.star.sdbcx.comp.hsqldb;

import com.sun.star.embed.XStorage;
/**
 *
 * @author  oj93728
 */
public class NativeOutputStreamHelper extends java.io.OutputStream{

    private String key;
    private String file;
    XStorage storage;
    private StorageNativeOutputStream out;
    /** Creates a new instance of NativeOutputStreamHelper */
    public NativeOutputStreamHelper(String key,String _file,XStorage _storage) {
        file = _file;
        storage = _storage;
        this.key = key;
        out = new StorageNativeOutputStream(file,key);
    }

    public void write(byte[] b, int off, int len)  throws java.io.IOException{
        out.write(storage,key,file,b, off, len);
    }

    public void write(byte[] b)  throws java.io.IOException{
        out.write(storage,key,file,b);
    }

    public void close()  throws java.io.IOException{
        out.close(storage,key,file);
    }

    public void write(int b)  throws java.io.IOException{
        out.write(storage,key,file,b);
    }

    public void flush()  throws java.io.IOException{
        out.flush(storage,key,file);
    }

}
