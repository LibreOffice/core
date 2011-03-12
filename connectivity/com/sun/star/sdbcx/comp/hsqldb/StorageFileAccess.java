/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
/*
 * StorageFileAccess.java
 *
 * Created on 31. August 2004, 11:56
 */

package com.sun.star.sdbcx.comp.hsqldb;
import org.hsqldb.lib.FileAccess;
import org.hsqldb.lib.FileSystemRuntimeException;

/**
 *
 * @author  oj93728
 */
public class StorageFileAccess implements org.hsqldb.lib.FileAccess{
    static { NativeLibraries.load(); }

    String ds_name;
    String key;
    /** Creates a new instance of StorageFileAccess */
    public StorageFileAccess(Object key) throws java.lang.Exception{
        this.key = (String)key;
    }

    public void createParentDirs(java.lang.String filename) {
    }

    public boolean isStreamElement(java.lang.String elementName)  {
        return isStreamElement(key,elementName);
    }

    public java.io.InputStream openInputStreamElement(java.lang.String streamName) throws java.io.IOException {
        return new NativeInputStreamHelper(key,streamName);
    }

    public java.io.OutputStream openOutputStreamElement(java.lang.String streamName) throws java.io.IOException {
        return new NativeOutputStreamHelper(key,streamName);
    }

    public void removeElement(java.lang.String filename) throws java.util.NoSuchElementException {
        try {
            if ( isStreamElement(key,filename) )
                removeElement(key,filename);
        } catch (java.io.IOException e) {
           throw new FileSystemRuntimeException( e, FileSystemRuntimeException.fileAccessRemoveElementFailed );
       }
    }

    public void renameElement(java.lang.String oldName, java.lang.String newName) throws java.util.NoSuchElementException {
        try {
            if ( isStreamElement(key,oldName) ){
                removeElement(key,newName);
                renameElement(key,oldName, newName);
            }
       } catch (java.io.IOException e) {
           throw new FileSystemRuntimeException( e, FileSystemRuntimeException.fileAccessRenameElementFailed );
       }
    }

    public class FileSync implements FileAccess.FileSync
    {
        NativeOutputStreamHelper os;
        FileSync(NativeOutputStreamHelper _os) throws java.io.IOException
        {
            os = _os;
        }
        public void sync() throws java.io.IOException
        {
            os.sync();
        }
    }

    public FileAccess.FileSync getFileSync(java.io.OutputStream os) throws java.io.IOException
    {
        return new FileSync((NativeOutputStreamHelper)os);
    }

    static native boolean isStreamElement(java.lang.String key,java.lang.String elementName);
    static native void removeElement(java.lang.String key,java.lang.String filename) throws java.util.NoSuchElementException, java.io.IOException;
    static native void renameElement(java.lang.String key,java.lang.String oldName, java.lang.String newName) throws java.util.NoSuchElementException, java.io.IOException;
}
