/*************************************************************************
 *
 *  $RCSfile: XInputStreamWrapper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:00:42 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package com.sun.star.script.framework.io;

import java.io.InputStream;
import com.sun.star.io.XInputStream;

public class XInputStreamWrapper extends InputStream {
    private XInputStream m_xInputStream;

    public XInputStreamWrapper(XInputStream xInputStream) {
        m_xInputStream = xInputStream;
    }

    public int read() throws java.io.IOException
    {
        byte[][] byteRet = new byte[1][0];
        long numRead;

        try {
            numRead = m_xInputStream.readBytes(byteRet, 1);
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }

        if (numRead != 1) {
            return -1;
        }
        return byteRet[0][0];
    }

    public int read( byte[] b ) throws java.io.IOException
    {
        byte[][] byteRet = new byte[1][];
        byteRet[0] = b;
        try
        {
           return m_xInputStream.readBytes( byteRet, b.length );
        }
        catch ( com.sun.star.io.IOException ioe)
        {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public long skip(long n) throws java.io.IOException
    {
        try {
            m_xInputStream.skipBytes((int)n);
            return n;
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public int available() throws java.io.IOException
    {
        try {
            return m_xInputStream.available();
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

    public void close() throws java.io.IOException
    {
        try {
            m_xInputStream.closeInput();
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new java.io.IOException(ioe.getMessage());
        }
    }

}
