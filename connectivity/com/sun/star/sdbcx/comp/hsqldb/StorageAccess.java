/*************************************************************************
 *
 *  $RCSfile: StorageAccess.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 12:06:02 $
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

public class StorageAccess implements org.hsqldb.Storage {
    String key;
    String name;
    NativeStorageAccess access;
    /** Creates a new instance of StorageAccess */
    public StorageAccess(String name,String _mode,Object key) throws java.lang.Exception{
        this.key = (String)key;
        this.name = name;
        try {
            access = new NativeStorageAccess(name,_mode,key);
        } catch(Exception e){
            throw new java.lang.Exception();
        }
    }
    public void close() throws java.lang.Exception{
        access.close(name,key);
    }

    public long getFilePointer() throws java.lang.Exception{
        return access.getFilePointer(name,key);
    }

    public long length() throws java.lang.Exception{
        return access.length(name,key);
    }

    public int read() throws java.lang.Exception{
        return access.read(name,key);
    }

    public void read(byte[] b, int off, int len) throws java.lang.Exception{
        access.read(name,key,b,off,len);
    }

    public int readInt() throws java.lang.Exception{
        byte [] tmp = new byte [4];
        read(tmp,0, 4);

        int ch [] = new int[4];
        for(int i = 0;i < 4; ++i){
            ch[i] = tmp[i];
            if (ch[i] < 0 ){
                ch[i] = 256 + ch[i];
            }
        }

    if ((ch[0] | ch[1] | ch[2] | ch[3]) < 0)
        throw new Exception();
    return ((ch[0] << 24) + (ch[1] << 16) + (ch[2] << 8) + (ch[3] << 0));
        //return access.readInt(name,key);
    }

    public void seek(long position) throws java.lang.Exception{
        access.seek(name,key,position);
    }

    public void write(byte[] b, int offset, int length) throws java.lang.Exception{
        access.write(name,key,b,offset,length);
    }

    public void writeInt(int v) throws java.lang.Exception{
        byte [] oneByte = new byte [4];
        oneByte[0] = (byte) ((v >>> 24) & 0xFF);
        oneByte[1] = (byte) ((v >>> 16) & 0xFF);
        oneByte[2] = (byte) ((v >>>  8) & 0xFF);
        oneByte[3] = (byte) ((v >>>  0) & 0xFF);

        write(oneByte,0,4);
        //access.writeInt(name,key,v);
    }
}
