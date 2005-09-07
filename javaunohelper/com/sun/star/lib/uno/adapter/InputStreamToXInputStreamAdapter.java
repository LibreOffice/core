/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InputStreamToXInputStreamAdapter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:37:52 $
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
package com.sun.star.lib.uno.adapter;

import java.io.IOException;
import com.sun.star.io.XInputStream;
import java.io.InputStream;

/** The <code>InputStreamToInputXStreamAdapter</code> wraps the
    Java <code>InputStream</code> object into a
    UNO <code>XInputStream</code> object.
    This allows users to access an <code>InputStream</code>
    as if it were an <code>XInputStream</code>.
 */
public class InputStreamToXInputStreamAdapter implements XInputStream {

    /**
     *  Internal store to the InputStream
     */
    private InputStream iIn;

    /**
     *  Constructor.
     *
     *  @param  in  The <code>XInputStream</code> to be
     *              accessed as an <code>InputStream</code>.
     */
    public InputStreamToXInputStreamAdapter (InputStream in)
    {
        iIn = in;
    }

    public int available() throws
            com.sun.star.io.IOException
    {

        int bytesAvail;

        try {
            bytesAvail = iIn.available();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }

        return(bytesAvail);
    }

    public void closeInput() throws
            com.sun.star.io.IOException
    {
        try {
            iIn.close();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }
    }

    public int readBytes(byte[][] b, int len) throws
            com.sun.star.io.IOException
    {
        int count = 0;
        try {
        long bytesRead=0;
        if (len >iIn.available()) {
            bytesRead = iIn.read(b[0], 0, iIn.available());
        }
        else{
            bytesRead = iIn.read(b[0], 0, len);
        }
            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.
            //
            if (bytesRead <= 0) {
                return(0);
        }
        return ((int)bytesRead);


        } catch (IOException e) {
            throw new com.sun.star.io.IOException("reader error: "+e.toString());
        }
    }

    public int readSomeBytes(byte[][] b, int len) throws
            com.sun.star.io.IOException
    {
        int count = 0;
        try {
        long bytesRead=0;
        if (len >iIn.available()) {
            bytesRead = iIn.read(b[0], 0, iIn.available());
        }
        else{
            bytesRead = iIn.read(b[0], 0, len);
        }
            // Casting bytesRead to an int is okay, since the user can
            // only pass in an integer length to read, so the bytesRead
            // must <= len.
            //
            if (bytesRead <= 0) {
                return(0);
        }
        return ((int)bytesRead);


        } catch (IOException e) {
            throw new com.sun.star.io.IOException("reader error: "+e.toString());
        }
    }

    public void skipBytes(int n) throws
            com.sun.star.io.IOException
    {
        int avail;
        int tmpLongVal = n;
        int  tmpIntVal;

        try {
            avail = iIn.available();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
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
                iIn.skip(tmpIntVal);
            } catch (IOException e) {
                throw new com.sun.star.io.IOException(e.toString());
            }
        } while (tmpLongVal > 0);
    }
}

