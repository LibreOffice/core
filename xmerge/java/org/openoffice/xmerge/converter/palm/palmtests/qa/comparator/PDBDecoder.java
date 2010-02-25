/************************************************************************
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


import java.io.RandomAccessFile;
import java.io.IOException;


/**
 *  <p>Provides functionality to decode a pdb formatted file into
 *  a <code>PalmDB</code> object given a file input stream</p>
 *
 *  <p>Sample usage:</p>
 *
 *  <p><blockquote><pre>
 *     PDBDecoder decoder = new PDBDecoder("sample.pdb");
 *     PalmDB palmDB = decoder.parse();
 *  </pre></blockquote></p>
 *
 *  <p>Refer to the
 *  <a href="http://starlite.eng/zensync/eng/converters/palmfileformats.pdf">
 *  Palm file format specification</a> for details on the pdb format.</p>
 *
 *  <p>This decoder has the following assumptions on the pdb file ...</p>
 *  <ol>
 *  <li><p>There is only one RecordList section in the pdb.</p></li>
 *  <li><p>The record indices in the RecordList are sorted in order, i.e. the
 *      first record index refers to record 0, and so forth.</p></li>
 *  <li><p>The raw records in the record section are sorted as well in order,
 *      i.e. first record comes ahead of second record, etc.</p></li>
 *  </ol>
 *
 *  Other decoders assume these as well.
 *
 *  @author    Herbie Ong
 *  @see    PalmDB
 *  @see    PDBHeader
 *
 *  @author    Herbie Ong
 */

public final class PDBDecoder {

    /**
     *  <p>This method decodes a pdb file into a PalmDB object.</p>
     *
     *  <p>First, read in the header data using <code>PDBHeader</code>'s
     *  <code>read</code> method</p>.  Next, read in the record list
     *  section.  Store the record offsets for use when parsing the records.
     *  Based on these offsets, read in each record's bytes and store
     *  each in a <code>Record</code> object.  Lastly, create a
     *  <code>PalmDB</code> object with the read in <code>Record</code>s.
     *
     *  @param   fileName    pdb file name
     *  @throws   IOException    if I/O error occurs
     */

    public PalmDB parse(String fileName) throws IOException {

        RandomAccessFile file = new RandomAccessFile(fileName, "r");

        // read the pdb header
        PDBHeader header = new PDBHeader();
        header.read(file);

        Record recArray[] = new Record[header.numRecords];

        if (header.numRecords != 0) {

            // read in the record indices + offsets

            int recOffset[] = new int[header.numRecords];

            for (int i = 0; i < header.numRecords; i++) {

                recOffset[i] = file.readInt();
                int attr = file.readInt();    // read in attribute.
            }

            // read the records

            int len = 0;
            byte[] bytes = null;

            int lastIndex = header.numRecords - 1;

            for (int i = 0; i < lastIndex; i++) {

                file.seek(recOffset[i]);
                len = recOffset[i+1] - recOffset[i];
                bytes = new byte[len];
                file.readFully(bytes);
                recArray[i] = new Record(bytes);
            }

            // last record
            file.seek(recOffset[lastIndex]);
            len = (int) file.length() - recOffset[lastIndex];
            bytes = new byte[len];
            file.readFully(bytes);
            recArray[lastIndex] = new Record(bytes);
        }

        file.close();

        // create PalmDB and return it
        PalmDB pdb = new PalmDB(header.pdbName, recArray);
        return pdb;
    }
}

