/*************************************************************************
 *
 *  $RCSfile: StorageFileAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:36:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
/*
 * StorageFileAccess.java
 *
 * Created on 31. August 2004, 11:56
 */

package com.sun.star.sdbcx.comp.hsqldb;
import org.hsqldb.lib.FileAccess;
import com.sun.star.embed.XStorage;
import com.sun.star.lib.util.NativeLibraryLoader;
/**
 *
 * @author  oj93728
 */
public class StorageFileAccess implements org.hsqldb.lib.FileAccess{
    static {
        if ( System.getProperty( "os.name" ).startsWith( "Windows" ) )
        {
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "msvcr71");
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "uwinapi");
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "sal3");
        //  NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "dbtools680mi");
        }
    // load shared library for JNI code
        NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "hsqldb2");
    }
    XStorage storage;
    String ds_name;
    String key;
    /** Creates a new instance of StorageFileAccess */
    public StorageFileAccess(Object key) throws java.lang.Exception{
        this.key = (String)key;
    }

    public void createParentDirs(java.lang.String filename) {
    }

    public boolean isStreamElement(java.lang.String elementName) throws java.util.NoSuchElementException, java.io.IOException {
        return isStreamElement(key,elementName);
    }

    public java.io.InputStream openInputStreamElement(java.lang.String streamName) throws java.io.IOException {
        return new NativeInputStreamHelper(key,streamName);
    }

    public java.io.OutputStream openOutputStreamElement(java.lang.String streamName) throws java.io.IOException {
        return new NativeOutputStreamHelper(key,streamName,storage);
    }

    public void removeElement(java.lang.String filename) throws java.util.NoSuchElementException, java.io.IOException {
        if ( isStreamElement(key,filename) )
            removeElement(key,filename);
    }

    public void renameElement(java.lang.String oldName, java.lang.String newName) throws java.util.NoSuchElementException, java.io.IOException {
        if ( isStreamElement(key,oldName) ){
            removeElement(key,newName);
            renameElement(key,oldName, newName);
        }
    }

    static native boolean isStreamElement(java.lang.String key,java.lang.String elementName) throws java.util.NoSuchElementException, java.io.IOException;
    static native void removeElement(java.lang.String key,java.lang.String filename) throws java.util.NoSuchElementException, java.io.IOException;
    static native void renameElement(java.lang.String key,java.lang.String oldName, java.lang.String newName) throws java.util.NoSuchElementException, java.io.IOException;
}
