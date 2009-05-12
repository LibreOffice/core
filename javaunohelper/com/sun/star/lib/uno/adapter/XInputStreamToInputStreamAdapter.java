/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XInputStreamToInputStreamAdapter.java,v $
 * $Revision: 1.7 $
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
package com.sun.star.lib.uno.adapter;

import java.io.IOException;
import com.sun.star.io.XInputStream;
import java.io.InputStream;

/**
 * The <code>XInputStreamToInputStreamAdapter</code> wraps
 * the UNO <code>XInputStream</code> object in a Java
 * <code>InputStream</code>.  This allows users to access
 * an <code>XInputStream</code> as if it were an
 * <code>InputStream</code>.
 *
 * @author  Brian Cameron
 */
public class XInputStreamToInputStreamAdapter extends InputStream {

    /**
     *  Internal handle to the XInputStream
     */
    private XInputStream xin;

    /**
     *  Constructor.
     *
     *  @param  in  The <code>XInputStream</code> to be
     *              accessed as an <code>InputStream</code>.
     */
    public XInputStreamToInputStreamAdapter (XInputStream in) {
        xin = in;
    }

    public int available() throws IOException {

        int bytesAvail;

        try {
            bytesAvail = xin.available();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }

        return(bytesAvail);
    }

    public void close() throws IOException {
        try {
            xin.closeInput();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public int read () throws IOException {
        byte [][] tmp = new byte [1][1];
        try {
            long bytesRead = xin.readBytes(tmp, 1);

            if (bytesRead <= 0) {
               return (-1);
            } else {
        int tmpInt = tmp[0][0];
        if (tmpInt< 0 ){
            tmpInt = 256 +tmpInt;
        }
                return(tmpInt);
            }

        } catch (Exception e) {
            throw new IOException(e.toString());
        }
    }

    public int read (byte[] b) throws IOException {

        byte [][] tmp = new byte [1][b.length];
        int bytesRead;

        try {
            bytesRead = xin.readBytes(tmp, b.length);
            if (bytesRead <= 0) {
                return(-1);
            } else if (bytesRead < b.length) {
                System.arraycopy(tmp[0], 0, b, 0, bytesRead);
            } else {
                System.arraycopy(tmp[0], 0, b, 0, b.length);
            }
        } catch (Exception e) {
            throw new IOException(e.toString());
        }

        return (bytesRead);
    }

    public int read(byte[] b, int off, int len) throws IOException {
        int count = 0;
        byte [][] tmp = new byte [1][b.length];
        try {
        long bytesRead=0;
            int av = xin.available();
        if ( av != 0 && len > av) {
          bytesRead = xin.readBytes(tmp, av);
        }
        else{
        bytesRead = xin.readBytes(tmp,len);
        }
            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.
            //
            if (bytesRead <= 0) {
                return(-1);
        } else if (bytesRead < len) {
        System.arraycopy(tmp[0], 0, b, off, (int)bytesRead);
        } else {
                System.arraycopy(tmp[0], 0, b, off, len);
        }

        return ((int)bytesRead);


        } catch (Exception e) {
            throw new IOException("reader error: "+e.toString());
        }
    }

    public long skip(long n) throws IOException {

        int avail;
        long tmpLongVal = n;
        int  tmpIntVal;

        try {
            avail = xin.available();
        } catch (Exception e) {
            throw new IOException(e.toString());
        }

        do {
            if (tmpLongVal >= Integer.MAX_VALUE) {
               tmpIntVal = Integer.MAX_VALUE;
            } else {
               // Casting is safe here.
               tmpIntVal = (int)tmpLongVal;
            }
            tmpLongVal -= tmpIntVal;

            try {
                xin.skipBytes(tmpIntVal);
            } catch (Exception e) {
                throw new IOException(e.toString());
            }
        } while (tmpLongVal > 0);

        if ( avail != 0 && avail < n) {
            return(avail);
        } else {
            return(n);
        }
    }

   /**
    *  Tests if this input stream supports the mark and reset methods.
    *  The markSupported method of
    *  <code>XInputStreamToInputStreamAdapter</code> returns false.
    *
    *  @returns  false
    */
    public boolean markSupported() {
       return false;
    }

    public void mark(int readlimit) {
        // Not supported.
    }

    public void reset() throws IOException {
        // Not supported.
    }
}

