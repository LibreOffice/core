/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutputStreamToXOutputStreamAdapter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:38:08 $
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
