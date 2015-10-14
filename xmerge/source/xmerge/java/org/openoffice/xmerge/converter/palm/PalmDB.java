/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.converter.palm;

import java.io.UnsupportedEncodingException;

/**
 * This class contains data for a single Palm database for use during a
 * conversion process.
 *
 * <p>It contains zero or more {@code Record} objects stored in an array.  The
 * index of the {@code Record} object in the array is the {@code Record} id or
 * number for that specific {@code Record} object.  Note that this class does
 * not check for maximum number of Records allowable in an actual PDB.</p>
 *
 * <p>This class also contains the PDB name associated with the Palm database
 * it represents.  A PDB name consists of 32 bytes of a certain encoding
 * (extended ASCII in this case).</p>
 *
 * <p>The non default constructors take in a name parameter which may not be
 * the exact PDB name to be used.  The name parameter in {@code String} or
 * {@code byte} array are converted to an exact {@code NAME_LENGTH} byte array.
 * If the length of the name is less than {@code NAME_LENGTH}, it is padded
 * with {@code '\0'} characters.  If it is more, it gets truncated.  The last
 * character in the resulting byte array is always a {@code '\0'} character.
 * The resulting byte array is stored in {@code bName}, and a corresponding
 * {@code String} object {@code sName} that contains characters without the
 * {@code '\0'} characters.</p>
 *
 * <p>The {@code write} method is called within the {@link
 * org.openoffice.xmerge.converter.palm.PalmDocument#write
 * PalmDocument.write} method for writing out its data to the
 * {@code OutputStream} object.</p>
 *
 * <p>The {@code read} method is called within the {@link
 * org.openoffice.xmerge.converter.palm.PalmDocument#read PalmDocument.read}
 * method for reading in its data from the {@code InputStream} object.</p>
 *
 * @see  PalmDocument
 * @see  Record
 */

public final class PalmDB {

    /** Number of bytes for the name field in the PDB. */
    public static final int NAME_LENGTH = 32;

    /** List of {@code Record} objects. */
    private Record[] records;

    /** PDB name in bytes. */
    private byte[] bName = null;

    /** PDB name in String. */
    private String sName = null;

    /** Creator ID. */
    private int creatorID = 0;

    /** Type ID */
    private int typeID = 0;

    /**
     * PDB version. Palm UInt16.
     * It is treated as a number here, since there is no unsigned 16 bit in Java,
     * {@code int} is used instead, but only 2 bytes are written out or read in.
     */
    private int version = 0;

    /**
     * PDB attribute - flags for the database.
     * Palm UInt16. Unsignedness should be irrelevant.
     */
    private short attribute = 0;

    /**
     * Default constructor.
     *
     * @param  creatorID  The PDB Creator ID.
     * @param  typeID     The PDB Type ID.
     * @param  version    The PDB header version.
     * @param  attribute  The PDB header attribute.
     */
    public PalmDB(int creatorID, int typeID, int version, short attribute) {
        records = new Record[0];
        setAttributes(creatorID, typeID, version, attribute);
    }

    /**
     * Constructor to create {@code PalmDB} object with {@code Record} objects.
     *
     * <p>{@code recs.length} can be zero for an empty PDB.</p>
     *
     * @param  name       Suggested PDB name in a {@code String}.
     * @param  creatorID  The PDB Creator ID.
     * @param  typeID     The PDB Type ID.
     * @param  version    The PDB header version.
     * @param  attribute  The PDB header attribute.
     * @param  recs       Array of {@code Record} objects.
     *
     * @throws  UnsupportedEncodingException  If {@code name} is not properly
     *                                        encoded.
     * @throws  NullPointerException          If {@code recs} is {@code null}.
     */
    public PalmDB(String name, int creatorID, int typeID, int version,
                  short attribute, Record[] recs)
            throws UnsupportedEncodingException {

        this(name.getBytes(PdbUtil.ENCODING), creatorID, typeID, version,
            attribute, recs);
    }

    /**
     * Constructor to create object with {@code Record} objects.
     *
     * <p>{@code recs.length} can be zero for an empty PDB.</p>
     *
     * @param   name       Suggested PDB name in a {@code byte} array.
     * @param   creatorID  The PDB Creator ID.
     * @param   typeID     The PDB Type ID.
     * @param   version    The PDB header version.
     * @param   attribute  The PDB header attribute.
     * @param   recs       Array of {@code Record} objects.
     *
     * @throws  UnsupportedEncodingException  If {@code name} is not properly
     *                                        encoded.
     * @throws  NullPointerException          If {@code recs} is {@code null}.
     */
    public PalmDB(byte[] name, int creatorID, int typeID, int version,
                  short attribute, Record[] recs)
            throws UnsupportedEncodingException {

        store(name);

        records = new Record[recs.length];
        System.arraycopy(recs, 0, records, 0, recs.length);
        setAttributes(creatorID, typeID, version, attribute);
    }

    /**
     * Set the attributes for the {@code PalmDB} object.
     *
     * @param  creatorID  The PDB Creator ID.
     * @param  typeID     The PDB Type ID.
     * @param  version    The PDB header version.
     * @param  attribute  The PDB header attribute.
     */
    private void setAttributes (int creatorID, int typeID, int version,
                                short attribute) {
        this.creatorID = creatorID;
        this.typeID = typeID;
        this.version = version;
        this.attribute = attribute;
    }

    /**
     * This private method is mainly used by the constructors above.
     *
     * <p>to store bytes into name and also create a {@code String}
     * representation, and also by the {@code read} method.</p>
     *
     * <p>TODO: Note that this method assumes that the {@code byte} array
     * parameter contains one character per {@code byte}, else it would
     * truncate improperly.</p>
     *
     * @param   bytes  PDB name in {@code byte<} array.
     *
     * @throws  UnsupportedEncodingException  If ENCODING is not supported.
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
        sName = new String(bName, 0, i, PdbUtil.ENCODING);
    }

    /**
     * Returns creator ID.
     *
     * @return  The creator ID.
     */
    public int getCreatorID() {
        return creatorID;
    }

    /**
     * Returns type ID.
     *
     * @return  The type ID.
     */
    public int getTypeID() {
        return typeID;
    }

    /**
     * Returns attribute flag.
     *
     * @return  The attribute flag.
     */
    public short getAttribute() {
        return attribute;
    }

    /**
     * Returns version.
     *
     * @return  The version.
     */
    public int getVersion() {
        return version;
    }

    /**
     * Return the number of Records contained in this PDB {@code PalmDB} object.
     *
     * @return  Number of {@code Record} objects.
     */
    public int getRecordCount() {
        return records.length;
    }

    /**
     * Return the specific {@code Record} object associated with the
     * {@code Record} number.
     *
     * @param   index  {@code Record} index number.
     *
     * @return  The {@code Record} object in the specified index
     *
     * @throws  ArrayIndexOutOfBoundsException  If index is out of bounds.
     */
    public Record getRecord(int index) {
        return records[index];
    }

    /**
     * Return the list of {@code Record} objects.
     *
     * @return  The array of {@code Record} objects.
     */
    public Record[] getRecords() {
        return records;
    }

    /**
     * Return the PDB name associated with this object.
     *
     * @return  The PDB name.
     */
    public String getPDBNameString() {
        return sName;
    }

    /**
     * Return the PDB name associated with this object in {@code byte} array of
     * exact length of 32 bytes.
     *
     * @return  The PDB name in {@code byte} array of length 32.
     */
    public byte[] getPDBNameBytes() {
        return bName;
    }

    /**
     * Override equals method of {@code Object}.
     *
     * <p>Two {@code PalmDB} objects are equal if they contain the same
     * information, i.e. PDB name and Records.</p>
     *
     * <p>This is used primarily for testing purposes only for now.</p>
     *
     * @param   obj  A {@code PalmDB} {@code Object} to compare.
     *
     * @return  {@code true} if {@code obj} is equal to this, otherwise
     *          {@code false}.
     */
    @Override
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

    @Override
    public int hashCode() {
        return 0;
    }

}
