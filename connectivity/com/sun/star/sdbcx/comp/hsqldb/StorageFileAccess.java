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
 * StorageFileAccess.java
 *
 * Created on 31. August 2004, 11:56
 */

package com.sun.star.sdbcx.comp.hsqldb;
import org.hsqldb.lib.FileAccess;
import org.hsqldb.lib.FileSystemRuntimeException;

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
