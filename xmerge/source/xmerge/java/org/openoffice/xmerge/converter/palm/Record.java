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

import java.io.OutputStream;
import java.io.InputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.IOException;

/**
 * Contains the raw bytes for a {@code Record} in a PDB.
 *
 * <p>Note that it is not associated with a {@code Record} number or ID.</p>
 *
 * @see  PalmDocument
 * @see  PalmDB
 */
public final class Record {

    /** {@code Record} {@code byte} array. */
    private byte[] data;

    /** {@code Record} attributes. */
    private byte attributes = 0;

    /**
     * Default constructor.
     */
    public Record() {
        data = new byte[0];
    }

    /**
     * Constructor to create a {@code Record} filled with bytes.
     *
     * <p>Note that this does not check for 64k {@code Record} sizes.  User of
     * this class must check for that.</p>
     *
     * @param  d  {@code byte} array contents for this object.
     */
    public Record(byte[] d) {
        this(d, (byte) 0);
    }

    /**
     * Constructor to create a {@code Record} filled with bytes and assign
     * {@code Record} attributes.
     *
     * <p>Note that this does not check for 64k {@code Record} sizes.  User of
     * this class must check for that.</p>
     *
     * @param  d      {@code byte} array contents for this object.
     * @param  attrs  {@code Record} attributes.
     */
    public Record(byte[] d, byte attrs) {
        data = new byte[d.length];
        attributes = attrs;
        System.arraycopy(d, 0, data, 0, d.length);
    }

    /**
     * This method returns the number of bytes in this object.
     *
     * @return  Number of bytes in this object.
     */
    public int getSize() {
        return data.length;
    }

    /**
     * This method returns the contents of this {@code Object}.
     *
     * @return  Contents in {@code byte} array
     */
    public byte[] getBytes() {
        return data;
    }

    /**
     * <p>This method returns the {@code Record} attributes.</p>
     *
     * <blockquote><pre>{@code Record} attributes consists of (from high to low bit)
     *
     * delete (1) - dirty (1) - busy (1) - secret (1) - category (4)</pre></blockquote>
     *
     * @return   {@code Record} attribute.
     */
    public byte getAttributes() {
        return attributes;
    }

    /**
     * Write out the {@code Record} attributes and {@code Record} length
     * followed by the data in this {@code Record} object.
     *
     * @param   outs  The {@code OutputStream} to write the object.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream outs) throws IOException {
        DataOutputStream out = new DataOutputStream(outs);
        out.writeByte(attributes);
        out.writeShort(data.length);
        out.write(data);
    }

    /**
     * Read the necessary data to create a PDB from the {@code InputStream}.
     *
     * @param   ins  The {@code InputStream} to read data from in order to
     *               restore the {@code object}.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream ins) throws IOException {
        DataInputStream in = new DataInputStream(ins);
        attributes = in.readByte();
        int len = in.readUnsignedShort();
        data = new byte[len];
        in.readFully(data);
    }

    /**
     * Override equals method of {@code Object}.
     *
     * <p>Two {@code Record} objects are equal if they contain the same bytes
     * in the array and the same attributes.</p>
     *
     * <p>This is used primarily for testing purposes only for now.</p>
     *
     * @param   obj  A {@code Record} object to compare with
     *
     * @return  {@code true} if {@code obj} is equal, otherwise {@code false}.
     */
    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Record)) {
            return false;
        }
        Record rec = (Record) obj;
        if (rec.getAttributes() != attributes) {
            return false;
        }
        if (rec.getSize() == data.length) {
            for (int i = 0; i < data.length; i++) {
                if (data[i] != rec.data[i]) {
                    return false;
                }
            }
        }
        return false;
    }

    @Override
    public int hashCode() {
        return 0;
    }

}
