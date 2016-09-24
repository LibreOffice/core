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
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

/**
 * Provides functionality to decode a PDB formatted file into a {@code PalmDB}
 * object given an {@code InputStream}.
 *
 * <p>This class is only used by the {@code PalmDB} object.</p>
 *
 * <p>Sample usage:</p>
 * <blockquote><pre>{@code PdbDecoder decoder = new PdbDecoder("sample.pdb");
 * PalmDB palmDB = decoder.parse();}</pre></blockquote>
 *
 * <p>This decoder has the following assumptions on the PDB file:</p>
 * <ol>
 *   <li>There is only one RecordList section in the PDB.</li>
 *   <li>The {@code Record} indices in the RecordList are sorted in order, i.e.
 *       the first {@code Record} index refers to {@code Record} 0, and so
 *       forth.</li>
 *   <li>The raw {@code Record} in the {@code Record} section are sorted as
 *       well in order, i.e. first {@code Record} comes ahead of second
 *       {@code Record}, etc.</li>
 * </ol>
 *
 * <p>Other decoders assume these as well.</p>
 *
 * @see  PalmDB
 * @see  Record
 */
public final class PdbDecoder {

    /**
     * This method decodes a PDB file into a {@code PalmDB} object.
     *
     * <p>First, the header data is read using the {@code PdbHeader.read}
     * method.  Next, the RecordList section is read and the {@code Record}
     * offsets are stored for use when parsing the Records.  Based on these
     * offsets, the bytes corresponding to each {@code Record} are read and
     * each is stored in a {@code Record} object.  Lastly, the data is used
     * to create a {@code PalmDB} object.</p>
     *
     * @param   b  {@code byte[]} containing PDB.
     *
     * @throws  IOException  If I/O error occurs.
     */
     public PalmDB parse(byte[] b) throws IOException {

     ByteArrayInputStream bais = new ByteArrayInputStream(b);
     DataInputStream dis = new DataInputStream(bais);

        // read the PDB header
        PdbHeader header = new PdbHeader();
        header.read(dis);

        Record recArray[] = new Record[header.numRecords];
        if (header.numRecords != 0) {

            // read in the record indices + offsets
            int recOffset[] = new int[header.numRecords];
            byte recAttrs[] = new byte[header.numRecords];

            for (int i = 0; i < header.numRecords; i++) {

                recOffset[i] = dis.readInt();

                // read in attributes (1 byte) + unique id (3 bytes)
                // take away the unique id, store the attributes
                int attr = dis.readInt();
                recAttrs[i] = (byte) (attr >>> 24);
            }

            // read the records
            int lastIndex = header.numRecords - 1;

            for (int i = 0; i < lastIndex; i++) {

                //dis.seek(recOffset[i]);
                dis.reset();
                int nBytesToSkip = recOffset[i];
                while (nBytesToSkip > 0) {
                    nBytesToSkip -= dis.skip(nBytesToSkip);
                }
                int len = recOffset[i+1] - recOffset[i];
                byte[] bytes = new byte[len];
                dis.readFully(bytes);
                recArray[i] = new Record(bytes, recAttrs[i]);
            }

            // last record
            dis.reset();
            int len = dis.available() - recOffset[lastIndex];
            int nBytesToSkip = recOffset[lastIndex];
            while (nBytesToSkip > 0) {
                nBytesToSkip -= dis.skip(nBytesToSkip);
            }
            byte[] bytes = new byte[len];
            dis.readFully(bytes);
            recArray[lastIndex] = new Record(bytes, recAttrs[lastIndex]);
        }

        // create PalmDB and return it
        PalmDB pdb = new PalmDB(header.pdbName, header.creatorID,
            header.typeID, header.version, header.attribute, recArray);

        return pdb;
    }
}
