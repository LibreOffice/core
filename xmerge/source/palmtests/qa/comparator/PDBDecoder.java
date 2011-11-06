/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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

