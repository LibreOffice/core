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
