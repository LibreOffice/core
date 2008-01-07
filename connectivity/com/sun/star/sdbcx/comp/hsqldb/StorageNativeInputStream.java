/*
 * StorageNativeInputStream.java
 *
 * Created on 9. September 2004, 11:52
 */

package com.sun.star.sdbcx.comp.hsqldb;
import com.sun.star.embed.ElementModes;
/**
 *
 * @author  Ocke
 */
public class StorageNativeInputStream {
    static { NativeLibraries.load(); }

    /** Creates a new instance of StorageNativeInputStream */
    public StorageNativeInputStream(String key,String _file) {
        openStream(key,_file, ElementModes.READ);
    }
    public native void openStream(String key,String name, int mode);
    public native int read(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b, int off, int len) throws java.io.IOException;
    public native void close(String key,String name) throws java.io.IOException;
    public native long skip(String key,String name,long n) throws java.io.IOException;
    public native int available(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b) throws java.io.IOException;
}
