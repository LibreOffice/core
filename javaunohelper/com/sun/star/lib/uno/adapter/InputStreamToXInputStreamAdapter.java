/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InputStreamToXInputStreamAdapter.java,v $
 * $Revision: 1.5 $
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

