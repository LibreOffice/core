/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OutputStreamToXOutputStreamAdapter.java,v $
 * $Revision: 1.4 $
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
import com.sun.star.io.XOutputStream;
import java.io.OutputStream;

/** The <code>OutputStreamToXOutputStreamAdapter</code> wraps
   a a UNO <code>XOutputStream</code> into a Java <code>OutputStream</code>
   object in a Java.  This allows users to access an <code>OutputStream</code>
   as if it were an <code>XOutputStream</code>.
 */
public class OutputStreamToXOutputStreamAdapter implements XOutputStream {

    /**
     *  Internal handle to the OutputStream
     */
    OutputStream iOut;

    /**
     *  Constructor.
     *
     *  @param  out  The <code>XOutputStream</code> to be
     *          accessed as an <code>OutputStream</code>.
     */
    public OutputStreamToXOutputStreamAdapter(OutputStream out) {
        iOut = out;
    }

    public void closeOutput() throws
            com.sun.star.io.IOException
    {
        try {
            iOut.close();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }
    }

    public void flush() throws
            com.sun.star.io.IOException
    {
        try {
            iOut.flush();
        } catch (IOException e) {
            throw new com.sun.star.io.IOException(e.toString());
        }
    }

    public void writeBytes(byte[] b) throws
            com.sun.star.io.IOException
    {

        try {
                iOut.write(b);
            } catch (IOException e) {
                throw new com.sun.star.io.IOException(e.toString());
            }
    }

}
