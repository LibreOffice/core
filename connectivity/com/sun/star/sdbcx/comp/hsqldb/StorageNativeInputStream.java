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
 * StorageNativeInputStream.java
 *
 * Created on 9. September 2004, 11:52
 */

package com.sun.star.sdbcx.comp.hsqldb;

public class StorageNativeInputStream {
    static { NativeLibraries.load(); }

    /** Creates a new instance of StorageNativeInputStream */
    public StorageNativeInputStream(String key,String _file) {
        openStream(key,_file, NativeStorageAccess.READ);
    }
    public native void openStream(String key,String name, int mode);
    public native int read(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b, int off, int len) throws java.io.IOException;
    public native void close(String key,String name) throws java.io.IOException;
    public native long skip(String key,String name,long n) throws java.io.IOException;
    public native int available(String key,String name) throws java.io.IOException;
    public native int read(String key,String name,byte[] b) throws java.io.IOException;
}
