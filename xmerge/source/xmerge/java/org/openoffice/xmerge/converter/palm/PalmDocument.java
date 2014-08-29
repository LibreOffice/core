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

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

import java.io.OutputStream;
import java.io.UnsupportedEncodingException;

import org.openoffice.xmerge.Document;

/**
 * A {@code PalmDocument} is palm implementation of the {@code Document}
 * interface.
 *
 * <p>This implementation allows the Palm device format to be read via an
 * {@code InputStream} and written via an {@code OutputStream}.</p>
 */
public class PalmDocument
    implements Document {

    /** The internal representation of a pdb. */
    private PalmDB pdb;

    /** The file name. */
    private String fileName;

    /**
     * Constructor to create a {@code PalmDocument} from an {@code InputStream}.
     *
     * @param   is  {@code InputStream} containing a PDB.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public PalmDocument(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Constructor to create a {@code PalmDocument} with {@code Record} objects.
     *
     * <p>{@code recs.length} can be zero for an empty PDB.</p>
     *
     * @param   name       Suggested PDB name in {@code String}.
     * @param   creatorID  The PDB Creator ID.
     * @param   typeID     The PDB Type ID.
     * @param   version    The PDB header version.
     * @param   attribute  The PDB header attribute.
     * @param   recs       Array of {@code Record} objects.
     *
     * @throws  NullPointerException  If {@code recs} is {@code null}.
     */
    public PalmDocument(String name, int creatorID, int typeID, int version,
        short attribute, Record[] recs)
        throws UnsupportedEncodingException {
        pdb = new PalmDB(name, creatorID, typeID, version, attribute, recs);
        fileName = pdb.getPDBNameString();
    }

    /**
     * Reads in a file from the {@code InputStream}.
     *
     * @param   is  {@code InputStream} to read in its content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void read(InputStream is) throws IOException {
        PdbDecoder decoder = new PdbDecoder();
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int n;
    while ((n = is.read(buf)) > 0) {
        baos.write(buf, 0, n);
        }
        byte[] bytearr = baos.toByteArray();
        pdb = decoder.parse(bytearr);
        fileName = pdb.getPDBNameString();
    }

    /**
     * Writes the {@code PalmDocument} to an {@code OutputStream}.
     *
     * @param   os  The {@code OutputStream} to write the content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        PdbEncoder encoder = new PdbEncoder(pdb);
        encoder.write(os);
    }

    /**
     * Returns the {@code PalmDB} contained in this object.
     *
     * @return  The {@code PalmDB}.
     */
    public PalmDB getPdb() {
        return pdb;
    }

    /**
     * Sets the {@code PalmDocument} to a new {@code PalmDB} value.
     *
     * @param  pdb  The new {@code PalmDB} value.
     */
    public void setPdb(PalmDB pdb) {
        this.pdb = pdb;

        String name = pdb.getPDBNameString();
        fileName = name;
    }

    /**
     * Returns the name of the file.
     *
     * @return  The name of the file represented in the {@code PalmDocument}.
     */
    public String getFileName() {
        return fileName + ".pdb";
    }

    /**
     * Returns the {@code Document} name.
     *
     * @return  The {@code Document} name.
     */
    public String getName() {
        return fileName;
    }
}