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

package org.openoffice.xmerge.converter.palm;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

/**
 *  <p>Provides functionality to decode a PDB formatted file into
 *  a <code>PalmDB</code> object given an <code>InputStream</code>.
 *  This class is only used by the <code>PalmDB</code> object.</p>
 *
 *  <p>Sample usage:</p>
 *
 *  <blockquote><pre><code>
 *     PdbDecoder decoder = new PdbDecoder("sample.pdb");
 *     PalmDB palmDB = decoder.parse();
 *  </code></pre></blockquote>
 *
 *  <p>This decoder has the following assumptions on the PDB file:</p>
 *
 *  <p><ol>
 *  <li>There is only one RecordList section in the PDB.</li>
 *  <li>The <code>Record</code> indices in the RecordList are sorted in
 *      order, i.e. the first <code>Record</code> index refers to
 *      <code>Record</code> 0, and so forth.</li>
 *  <li>The raw <code>Record</code> in the <code>Record</code> section
 *      are sorted as well in order, i.e. first <code>Record</code>
 *      comes ahead of second <code>Record</code>, etc.</li>
 *  </ol></p>
 *
 *  <p>Other decoders assume these as well.</p>
 *
 *  @author  Herbie Ong
 *  @see     PalmDB
 *  @see     Record
 */
public final class PdbDecoder {


    /**
     *  <p>This method decodes a PDB file into a <code>PalmDB</code>
     *  object.</p>
     *
     *  <p>First, the header data is read using the <code>PdbHeader</code>
     *  <code>read</code> method.  Next, the RecordList section is
     *  read and the <code>Record</code> offsets are stored for use when
     *  parsing the Records.  Based on these offsets, the bytes
     *  corresponding to each <code>Record</code> are read and each is
     *  stored in a  <code>Record</code> object.  Lastly, the data is
     *  used to create a <code>PalmDB</code> object.</p>
     *
     *  @param  fileName  PDB file name.
     *
     *  @throws  IOException  If I/O error occurs.
     */
    public PalmDB parse(String fileName) throws IOException {

        RandomAccessFile file = new RandomAccessFile(fileName, "r");

        // read the PDB header
        PdbHeader header = new PdbHeader();
        header.read(file);

        Record recArray[] = new Record[header.numRecords];
        if (header.numRecords != 0) {

            // read in the record indices + offsets

            int recOffset[] = new int[header.numRecords];
            byte recAttrs[] = new byte[header.numRecords];

            for (int i = 0; i < header.numRecords; i++) {

                recOffset[i] = file.readInt();

                // read in attributes (1 byte) + unique id (3 bytes)
                // take away the unique id, store the attributes

                int attr = file.readInt();
                recAttrs[i] = (byte) (attr >>> 24);
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
        recArray[i] = new Record(bytes, recAttrs[i]);
        }
        
            // last record
            file.seek(recOffset[lastIndex]);
            len = (int) file.length() - recOffset[lastIndex];
            bytes = new byte[len];
            file.readFully(bytes);
            recArray[lastIndex] = new Record(bytes, recAttrs[lastIndex]);
       
        }

        file.close();

        // create PalmDB and return it
        PalmDB pdb = new PalmDB(header.pdbName, header.creatorID,
            header.typeID, header.version, header.attribute, recArray);

        return pdb;
    }
    
    /**
     *  <p>This method decodes a PDB file into a <code>PalmDB</code>
     *  object.</p>
     *
     *  <p>First, the header data is read using the <code>PdbHeader</code>
     *  <code>read</code> method.  Next, the RecordList section is
     *  read and the <code>Record</code> offsets are stored for use when
     *  parsing the Records.  Based on these offsets, the bytes
     *  corresponding to each <code>Record</code> are read and each is
     *  stored in a  <code>Record</code> object.  Lastly, the data is
     *  used to create a <code>PalmDB</code> object.</p>
     *
     *  @param  b  <code>byte[]</code> containing PDB.
     *
     *  @throws  IOException  If I/O error occurs.
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

            int len = 0;
            byte[] bytes = null;

            int lastIndex = header.numRecords - 1;

            for (int i = 0; i < lastIndex; i++) {

                //dis.seek(recOffset[i]);
        dis.reset();
        dis.skip(recOffset[i]);
                len = recOffset[i+1] - recOffset[i];
                bytes = new byte[len];
                dis.readFully(bytes);
                recArray[i] = new Record(bytes, recAttrs[i]);
            }
       
            // last record
          
            dis.reset();
        len = (int) dis.available() - recOffset[lastIndex];
        dis.skip(recOffset[lastIndex]);	   
        bytes = new byte[len];
            dis.readFully(bytes);
            recArray[lastIndex] = new Record(bytes, recAttrs[lastIndex]);
        }
    


        // create PalmDB and return it
    
        PalmDB pdb = new PalmDB(header.pdbName, header.creatorID,
            header.typeID, header.version, header.attribute, recArray);

        return pdb;
    }



}

