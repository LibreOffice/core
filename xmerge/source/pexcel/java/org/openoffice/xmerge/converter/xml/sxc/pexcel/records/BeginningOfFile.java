/************************************************************************
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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 *  This class describes the beginning of file. It is the
 *  the Biff record that marks the beginning of a a worbook
 *  or the beginning of worksheets in the workbook
 *
 */
public class BeginningOfFile implements BIFFRecord {

    private byte[] version = new byte[2];
    private byte[] subStream = new byte[2];

   /**
    *  Constructor that initializes the member values.
    *
    *  @param  ver  Version Number
    *               Substream type (workbook = 0x05, worksheet = 0x10)
    */
    public BeginningOfFile(boolean global) {
        setVersion((short) 271);
        if(global)
            setSubStreamWBGlobal();
        else
            setSubStreamWorkSheet();
        // this.subStream = EndianConverter.writeShort(dt);
    }

    public BeginningOfFile(InputStream is) throws IOException {
        read(is);
    }

    private void setVersion(short version) {
        this.version = EndianConverter.writeShort(version);
    }

    int getVersion() {
        return EndianConverter.readShort(version);
    }

    private void setSubStreamWBGlobal() {
        // subStream = new byte[] {0x05};
        subStream = EndianConverter.writeShort((short) 0x05);
    }

    private void setSubStreamWorkSheet() {
        // subStream = new byte[] {0x10};
        subStream = EndianConverter.writeShort((short) 0x10);
    }

    int getSubStreamType() {
        return EndianConverter.readShort(subStream);
    }

    public int read(InputStream input) throws IOException {
        int numBytesRead = input.read(version);
        numBytesRead += input.read(subStream);
        Debug.log(Debug.TRACE,"\tVersion : "+ EndianConverter.readShort(version) +
                            " Stream : " + EndianConverter.readShort(subStream));

        return numBytesRead;
    }

    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(version);
        output.write(subStream);

        Debug.log(Debug.TRACE, "Writing BeginningOfFile record");
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>BeginningOfFile</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.BOF_RECORD;
   }
}
