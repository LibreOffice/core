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
