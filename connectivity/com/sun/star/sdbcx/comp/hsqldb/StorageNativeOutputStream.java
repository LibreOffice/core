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
 * StorageNativeOutputStream.java
 *
 * Created on 1. September 2004, 10:20
 */

package com.sun.star.sdbcx.comp.hsqldb;

public class StorageNativeOutputStream {
    static { NativeLibraries.load(); }

    String name;
    Object key;

    /** Creates a new instance of StorageNativeOutputStream */
    public StorageNativeOutputStream(String _name,Object _key) {
        name = _name;
        key = _key;
        openStream(name, (String)key, NativeStorageAccess.WRITE | NativeStorageAccess.TRUNCATE);
    }

    public native void openStream(String name,String key, int mode);
    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this output stream.
     * The general contract for <code>write(b, off, len)</code> is that
     * some of the bytes in the array <code>b</code> are written to the
     * output stream in order; element <code>b[off]</code> is the first
     * byte written and <code>b[off+len-1]</code> is the last byte written
     * by this operation.
     * <p>
     * The <code>write</code> method of <code>OutputStream</code> calls
     * the write method of one argument on each of the bytes to be
     * written out. Subclasses are encouraged to override this method and
     * provide a more efficient implementation.
     * <p>
     * If <code>b</code> is <code>null</code>, a
     * <code>NullPointerException</code> is thrown.
     * <p>
     * If <code>off</code> is negative, or <code>len</code> is negative, or
     * <code>off+len</code> is greater than the length of the array
     * <code>b</code>, then an <tt>IndexOutOfBoundsException</tt> is thrown.
     * @param key The name of the data source.
     * @param _file The name of the file to write to.
     * @param b the data.
     * @param off the start offset in the data.
     * @param len the number of bytes to write.
     * @exception java.io.IOException if an I/O error occurs. In particular,
     *             an <code>IOException</code> is thrown if the output
     *             stream is closed.
     */
    public native void write(String key,String _file,byte[] b, int off, int len) throws java.io.IOException;

    /**
     * Writes <code>b.length</code> bytes from the specified byte array
     * to this output stream. The general contract for <code>write(b)</code>
     * is that it should have exactly the same effect as the call
     * <code>write(b, 0, b.length)</code>.
     *
     * @param      b   the data.
     * @exception  java.io.IOException  if an I/O error occurs.
     * @see        java.io.OutputStream#write(byte[], int, int)
     */
    public native void write(String key,String _file,byte[] b) throws java.io.IOException;

    /**
     * Closes this output stream and releases any system resources
     * associated with this stream. The general contract of <code>close</code>
     * is that it closes the output stream. A closed stream cannot perform
     * output operations and cannot be reopened.
     * <p>
     * The <code>close</code> method of <code>OutputStream</code> does nothing.
     * @param key The name of the data source.
     * @param _file The name of the file to write to.
     *
     * @exception  java.io.IOException  if an I/O error occurs.
     */
    public native void close(String key,String _file) throws java.io.IOException;

    /**
     * Writes the specified byte to this output stream. The general
     * contract for <code>write</code> is that one byte is written
     * to the output stream. The byte to be written is the eight
     * low-order bits of the argument <code>b</code>. The 24
     * high-order bits of <code>b</code> are ignored.
     * <p>
     * Subclasses of <code>OutputStream</code> must provide an
     * implementation for this method.
     *
     * @param key The name of the data source.
     * @param _file The name of the file to write to.
     * @param      b   the <code>byte</code>.
     * @exception  java.io.IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> may be thrown if the
     *             output stream has been closed.
     */
    public native void write(String key,String _file,int b) throws java.io.IOException;

    /**
     * Flushes this output stream and forces any buffered output bytes
     * to be written out. The general contract of <code>flush</code> is
     * that calling it is an indication that, if any bytes previously
     * written have been buffered by the implementation of the output
     * stream, such bytes should immediately be written to their
     * intended destination.
     * <p>
     * The <code>flush</code> method of <code>OutputStream</code> does nothing.
     * @param key The name of the data source.
     * @param _file The name of the file to write to.
     *
     * @exception  java.io.IOException  if an I/O error occurs.
     */
    public native void flush(String key,String _file) throws java.io.IOException;

    /**
     * Force all system buffers to synchronize with the underlying
     * device.  This method returns after all modified data and
     * attributes have been written to the relevant device(s).
     *
     * sync is meant to be used by code that requires physical
     * storage (such as a file) to be in a known state  For
     * example, a class that provided a simple transaction facility
     * might use sync to ensure that all changes to a file caused
     * by a given transaction were recorded on a storage medium.
     *
     * sync only affects buffers downstream.  If
     * any in-memory buffering is being done by the application (for
     * example, by a BufferedOutputStream object), those buffers must
     * be flushed (for example, by invoking
     * OutputStream.flush) before that data will be affected by sync.
     *
     * @exception java.io.IOException
     *        Thrown when the buffers cannot be flushed,
     *        or because the system cannot guarantee that all the
     *        buffers have been synchronized with physical media.
     */
    public native void sync(String key,String _file) throws java.io.IOException;

}
