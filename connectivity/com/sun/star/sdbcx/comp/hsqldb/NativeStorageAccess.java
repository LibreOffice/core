/*************************************************************************
 *
 *  $RCSfile: NativeStorageAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:36:44 $
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
 * StorageAccess.java
 *
 * Created on 17. August 2004, 13:32
 */

package com.sun.star.sdbcx.comp.hsqldb;

/**
 *
 * @author  oj93728
 */
import com.sun.star.document.XDocumentSubStorageSupplier;
import com.sun.star.embed.ElementModes;
import com.sun.star.embed.XStorage;
import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XSeekable;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.NamedValue;
import com.sun.star.lib.util.NativeLibraryLoader;

public class NativeStorageAccess {
    static
    {
        if ( System.getProperty( "os.name" ).startsWith( "Windows" ) )
        {
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "msvcr71");
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "uwinapi");
            NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "sal3");
        }
        // load shared library for JNI code
        NativeLibraryLoader.loadLibrary(StorageNativeOutputStream.class.getClassLoader(), "hsqldb2");
    }
    /** Creates a new instance of StorageAccess */
    public NativeStorageAccess(String name,String _mode,Object key) throws java.lang.Exception{
        try {
            int mode = ElementModes.SEEKABLEREAD;
            if ( _mode.equals("rw") )
                mode |= ElementModes.WRITE;

            openStream(name, (String)key, mode);
        } catch(Exception e){
            throw new java.lang.Exception();
        }
    }
    public native void openStream(String name,String key, int mode);
    public native void close(String name,String key) throws java.lang.Exception;

    public native long getFilePointer(String name,String key) throws java.lang.Exception;

    public native long length(String name,String key) throws java.lang.Exception;

    public native int read(String name,String key) throws java.lang.Exception;

    public native void read(String name,String key,byte[] b, int off, int len) throws java.lang.Exception;

    public native int readInt(String name,String key) throws java.lang.Exception;

    public native void seek(String name,String key,long position) throws java.lang.Exception;

    public native void write(String name,String key,byte[] b, int offset, int length) throws java.lang.Exception;

    public native void writeInt(String name,String key,int v) throws java.lang.Exception;
}
