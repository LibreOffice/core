/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DefColWidth.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:01:34 $
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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.DataInputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF record defiuning the defualt column width
 */
public class DefColWidth implements BIFFRecord {

    private byte[] grbit = new byte[2];
    private byte[] coldx = new byte[2];
    private byte[] ixfe  = new byte[2];

/**
 * Constructs a pocket Excel Document from the
 * <code>InputStream</code> and assigns it the document name passed in
 *
 * @param   is InputStream containing a Pocket Excel Data file.
 */
    public DefColWidth() {
        grbit   = new byte[] {0x00, 0x00};
        coldx   = new byte[] {0x00, 0x09};
        ixfe    = new byte[] {0x00, 0x00};
    }

    public DefColWidth(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>DefColWidth</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.DEF_COL_WIDTH;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(grbit);
        output.write(coldx);
        output.write(ixfe);

        Debug.log(Debug.TRACE,  "Writing DefColWidth record");
    }

    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(grbit);
        numOfBytesRead      += input.read(coldx);
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE,"\tgrbit : "+ EndianConverter.readShort(grbit) +
                            " coldx : " + EndianConverter.readShort(coldx) +
                            " ixfe : " + EndianConverter.readShort(ixfe));
        return 0;
    }

}
