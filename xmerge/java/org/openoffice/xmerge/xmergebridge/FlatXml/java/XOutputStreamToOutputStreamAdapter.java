/*************************************************************************
 *
 *  $RCSfile: XOutputStreamToOutputStreamAdapter.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aidan $ $Date: 2002-07-17 15:03:20 $
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

//package com.sun.star.lib.uno.adapter;

import java.io.IOException;
import com.sun.star.io.XOutputStream;
import java.io.OutputStream;

/**
 * The <code>XOutputStreamToOutputStreamAdapter</code> wraps
 * the UNO <code>XOutputStream</code> object in a Java
 * <code>OutputStream</code>.  This allows users to access
 * an <code>XOutputStream</code> as if it were an
 * <code>OutputStream</code>.
 *
 * @author  Brian Cameron
 */
public class XOutputStreamToOutputStreamAdapter extends OutputStream {

    /**
     *  Internal handle to the XInputStream
     */
    XOutputStream xout;

    /**
     *  Constructor.
     *
     *  @param  out  The <code>XOutputStream</code> to be
     *          accessed as an <code>OutputStream</code>.
     */
    XOutputStreamToOutputStreamAdapter(XOutputStream out) {
        xout = out;
    }

    public void close() throws IOException {
        try {
            xout.closeOutput();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void flush() throws IOException {
        try {
            xout.flush();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(byte[] b) throws IOException {
    //System.out.println("write1\n");
    try {      
            xout.writeBytes(b);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(byte[] b, int off, int len) throws IOException {
    
        byte[] tmp = new byte[len];
    //System.out.println("write2\n");
        // Copy the input array into a temp array, and write it out.
        //
        System.arraycopy(b, off, tmp, 0, len);
        
        try {
            xout.writeBytes(tmp);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public void write(int b) throws IOException {
    
        byte [] oneByte = new byte [1];
        oneByte[0] = (byte) b;
    //System.out.println("write3\n");
        try {
            xout.writeBytes(oneByte);
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }
}





