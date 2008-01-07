/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NativeStorageAccess.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 09:45:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
import com.sun.star.embed.ElementModes;

public class NativeStorageAccess {
    static { NativeLibraries.load(); }

    /** Creates a new instance of StorageAccess */
    public NativeStorageAccess(String name,String _mode,Object key) throws java.io.IOException{
        try {
            int mode = ElementModes.SEEKABLEREAD;
            if ( _mode.equals("rw") )
                mode = ElementModes.READWRITE | ElementModes.SEEKABLE;

            openStream(name, (String)key, mode);
        } catch(Exception e){
            throw new java.io.IOException();
        }
    }
    public native void openStream(String name,String key, int mode);
    public native void close(String name,String key) throws java.io.IOException;

    public native long getFilePointer(String name,String key) throws java.io.IOException;

    public native long length(String name,String key) throws java.io.IOException;

    public native int read(String name,String key) throws java.io.IOException;

    public native int read(String name,String key,byte[] b, int off, int len) throws java.io.IOException;

    public native int readInt(String name,String key) throws java.io.IOException;

    public native void seek(String name,String key,long position) throws java.io.IOException;

    public native void write(String name,String key,byte[] b, int offset, int length) throws java.io.IOException;

    public native void writeInt(String name,String key,int v) throws java.io.IOException;
}
