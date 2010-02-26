/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import java.io.OutputStream;
import java.io.InputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.IOException;

/**
 *  <p>Contains the raw bytes for a record in a pdb.</p>
 *
 *  <p>Note that it is not associated with a record number or id.</p>
 *
 *  @author    Akhil Arora, Herbie Ong
 *  @see    PalmDB
 */


public final class Record {

    /** record bytes */
    private byte[] data;

    /**
     *  Default constructor.
     */

    public Record() {

        data = new byte[0];
    }

    /**
     *  Constructor to create a Record filled with bytes.
     *  Note that this does not check for 64k record sizes.
     *  User of this class has to check for that.
     *
     *  @param    d    byte array contents for this object.
     */

    public Record(byte[] d) {

        data = new byte[d.length];
        System.arraycopy(d, 0, data, 0, d.length);
    }

    /**
     *  This method returns the number of bytes in this object.
     *
     *  @return    int    number of bytes in this object.
     */

    public int getSize() {

        return data.length;
    }

    /**
     *  This method returns the contents of this object.
     *
     *  @return    byte[]    contents in byte array
     */

    public byte[] getBytes() {

        return data;
    }

    /**
     *  Write out the record length followed by the data
     *  in this Record object.
     *
     *  @param   out    the stream to write the object to
     *  @throws  IOException    if any I/O error occurs
     */


    public void write(OutputStream outs) throws IOException {

        DataOutputStream out = new DataOutputStream(outs);
        out.writeShort(data.length);
        out.write(data);
    }

    /**
     *  Read the necessary data to create a pdb from
     *  the input stream.
     *
     *  @param   in    the stream to read data from in order to
     *                 restore the object
     *  @throws  IOException    if any I/O error occurs
     */

    public void read(InputStream ins) throws IOException {

        DataInputStream in = new DataInputStream(ins);
        int len = in.readUnsignedShort();
        data = new byte[len];
        in.readFully(data);
    }

    /**
     *  Override equals method of Object.
     *
     *  2 Record objects are equal if they contain the same bytes
     *  in the array.
     *
     *  This is used primarily for testing purposes only for now.
     *
     *  @param   obj    a Record object to compare with
     *  @return   boolean    true if obj is equal to this, else false.
     */

    public boolean equals(Object obj) {

        boolean bool = false;

        if (obj instanceof Record) {

            Record rec = (Record) obj;

            checkLabel: {

                if (rec.getSize() == data.length) {

                    for (int i = 0; i < data.length; i++) {

                        if (data[i] != rec.data[i]) {
                            break checkLabel;
                        }
                    }

                    bool = true;
                }
            }
        }

        return bool;
    }
}
