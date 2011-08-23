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
import java.io.UnsupportedEncodingException;

/**
 *  <p>This class contains data for a single Palm database for use during
 *  a conversion process.</p>
 *
 *  <p>It contains zero or more <code>Record</code> objects stored in an
 *  array. The index of the <code>Record</code> object in the array is
 *  the record id or number for that specific <code>Record</code> object.
 *  Note that this class does not check for maximum number of records
 *  allowable in an actual pdb.</p>
 *
 *  <p>This class also contains the pdb name associated with the Palm database
 *  it represents. A pdb name consists of 32 bytes of a certain encoding
 *  (extended ASCII in this case).</p>
 *
 *  <p>The non default constructors take in a name parameter which may not
 *  be the exact pdb name to be used.  The name parameter in
 *  <code>String</code> or <code>byte[]</code> are converted to an exact
 *  <code>NAME_LENGTH</code> byte array.  If the length of the name is less
 *  than <code>NAME_LENGTH</code>, it is padded with '\0' characters.  If it
 *  is more, it gets truncated.  The last character in the resulting byte
 *  array is always a '\0' character.  The resulting byte array is stored in
 *  <code>bName</code>, and a corresponding String object <code>sName</code>
 *  that contains characters without the '\0' characters.</p>
 *
 *  <p>The {@link #write write} method is called within the
 *  {@link zensync.util.palm.PalmDBSet#write PalmDBSet.write} method
 *  for writing out its data to the <code>OutputStream</code> object.</p>
 *
 *  <p>The {@link #read read} method is called within the
 *  {@link zensync.util.palm.PalmDBSet#read PalmDBSet.read} method
 *  for reading in its data from the <code>InputStream</code> object.</p>
 *
 *  @author    Akhil Arora, Herbie Ong
 *  @see    PalmDBSet
 *  @see    Record
 */

public final class PalmDB {

    /** number of bytes for the name field in the pdb */
    public final static int NAME_LENGTH = 32;

    /** list of Record objects */
    private Record[] records;

    /** pdb name in bytes */
    private byte[] bName = null;

    /** pdb name in String */
    private String sName = null;


    /**
     *  Default constructor for use after a read().
     */

    public PalmDB() {

        records = new Record[0];
    }

    /**
     *  Constructor to create object with Record objects.
     *  recs.length can be zero for an empty pdb.
     *
     *  @param   name    suggested pdb name in String
     *  @param   recs    array of Record objects
     *  @throws  NullPointerException    if recs is null
     */

    public PalmDB(String name, Record[] recs)
        throws UnsupportedEncodingException {

        this(name.getBytes(PDBUtil.ENCODING), recs);
    }

    /**
     *  Constructor to create object with Record objects.
     *  recs.length can be zero for an empty pdb.
     *
     *  @param   name    suggested pdb name in byte array
     *  @param   recs    array of Record objects
     *  @throws  NullPointerException    if recs is null
     */

    public PalmDB(byte[] name, Record[] recs)
        throws UnsupportedEncodingException {

        store(name);

        records = new Record[recs.length];
        System.arraycopy(recs, 0, records, 0, recs.length);
    }

    /**
     *  This private method is mainly used by the constructors above.
     *  to store bytes into name and also create a String representation.
     *  and also by the read method.
     *
     *  TODO: Note that this method assumes that the byte array parameter
     *  contains one character per byte, else it would truncate
     *  improperly.
     *
     *  @param   bytes    pdb name in byte array
     *  @throws   UnsupportedEncodingException    if ENCODING is not supported
     */

    private void store(byte[] bytes) throws UnsupportedEncodingException {

        // note that this will initialize all bytes in name to 0.
        bName = new byte[NAME_LENGTH];

        // determine minimum length to copy over from bytes to bName.
        // Note that the last byte in bName has to be '\0'.

        int lastIndex = NAME_LENGTH - 1;

        int len = (bytes.length < lastIndex)? bytes.length: lastIndex;

        int i;

        for (i = 0; i < len; i++) {

            if (bytes[i] == 0) {
                break;
            }

            bName[i] = bytes[i];
        }

        // set sName, no need to include the '\0' character.
        sName = new String(bName, 0, i, PDBUtil.ENCODING);
    }

    /**
     *  Return the number of records contained in this
     *  pdb PalmDB object.
     *
     *  @return  int    number of Record objects
     */

    public int getRecordCount() {

        return records.length;
    }

    /**
     *  Return the specific Record object associated
     *  with the record number.
     *
     *  @param   index    record index number
     *  @return  Record   the Record object in the specified index
     *  @throws  ArrayIndexOutOfBoundsException    if index is out of bounds
     */

    public Record getRecord(int index) {

        return records[index];
    }

    /**
     *  Return the list of Record objects
     *
     *  @return  Record[]   the list of Record objects
     */

    public Record[] getRecords() {

        return records;
    }

    /**
     *  Return the PDBName associated with this object in String
     *
     *  @return  String    pdb name in String
     */

    public String getPDBNameString() {

        return sName;
    }

    /**
     *  Return the PDBName associated with this object
     *  in byte array of exact length of 32 bytes.
     *
     *  @return  byte[]    pdb name in byte[] of length 32.
     */

    public byte[] getPDBNameBytes() {

        return bName;
    }

    /**
     *  Write out the number of records followed by what
     *  will be written out by each Record object.
     *
     *  @param   os    the stream to write the object to
     *  @throws  IOException    if any I/O error occurs
     */

    public void write(OutputStream os) throws IOException {

        DataOutputStream out = new DataOutputStream(os);

        // write out pdb name
        out.write(bName);

        // write out 2 bytes for number of records
        out.writeShort(records.length);

        // let each Record object write out its own info.
        for (int i = 0; i < records.length; i++)
            records[i].write(out);
    }

    /**
     *  Read the necessary data to create a pdb from
     *  the input stream.
     *
     *  @param   is    the stream to read data from in order
     *                 to restore the object
     *  @throws  IOException    if any I/O error occurs
     */

    public void read(InputStream is) throws IOException {

        DataInputStream in = new DataInputStream(is);

        // read in the pdb name.
        byte[] bytes = new byte[NAME_LENGTH];
        in.readFully(bytes);
        store(bytes);

        // read in number of records
        int nrec = in.readUnsignedShort();
        records = new Record[nrec];

        // read in the Record infos
        for (int i = 0; i < nrec; i++) {

            records[i] = new Record();
            records[i].read(in);
        }
    }

    /**
     *  Override equals method of Object.
     *
     *  2 PalmDB objects are equal if they contain the same information,
     *  i.e. pdb name and records.
     *
     *  This is used primarily for testing purposes only for now.
     *
     *  @param   obj    a PalmDB object to compare with
     *  @return   boolean    true if obj is equal to this, else false.
     */

    public boolean equals(Object obj) {

        boolean bool = false;

        if (obj instanceof PalmDB) {

            PalmDB pdb = (PalmDB) obj;

            checkLabel: {

                // compare sName

                if (!sName.equals(pdb.sName)) {

                    break checkLabel;
                }

                // compare bName

                if (bName.length != pdb.bName.length) {

                    break checkLabel;
                }

                for (int i = 0; i < bName.length; i++) {

                    if (bName[i] != pdb.bName[i]) {

                        break checkLabel;
                    }
                }

                // compare each Record

                if (records.length != pdb.records.length) {

                    break checkLabel;
                }

                for (int i = 0; i < records.length; i++) {

                    if (!records[i].equals(pdb.records[i])) {

                        break checkLabel;
                    }
                }

                // all checks done
                bool = true;
            }
        }

        return bool;
    }
}
