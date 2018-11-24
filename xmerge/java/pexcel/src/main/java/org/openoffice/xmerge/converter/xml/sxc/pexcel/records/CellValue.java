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



package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;

public abstract class CellValue implements BIFFRecord {

    protected byte[] rw = new byte[2];
    protected byte   col;
    protected byte[] ixfe = new byte[2];

    /**
     * Get the row number of this cell
     *
     * @return the row number of this cell
     */
    public int getRow() {
        return EndianConverter.readShort(rw) + 1;
    }

    /**
     * Set the row number of this cell
     *
     * @param row sets the row number for this cell
     */
    public void setRow(int row) {
        this.rw = EndianConverter.writeShort((short) (row - 1));
    }
    /**
     * Get the Index to the <code>ExtendedFormat</code>
     *
     * @return the index number of this cell's <code>ExtendedFormat</code>
     */
    public int getIxfe() {
        return EndianConverter.readShort(ixfe);
    }

    /**
     * Sets the Index to the <code>ExtendedFormat</code>
     *
     * @param ixfe sets the index number for this cell's <code>ExtendedFormat</code>
     */
    public void setIxfe(int ixfe) {
        this.ixfe = EndianConverter.writeShort((short) (ixfe));
    }

    /**
     * Get the column number of this cell
     *
     * @return the column number of this cell
     */
    public int getCol() {
        return col + 1;         // The cols start at 1
    }

    /**
     * Set the row number of this cell
     *
     * @param col sets the row number for this cell
     */
    public void setCol(int col) {
        this.col = (byte) (col - 1);        // The cols start at 1
    }

    /**
     * Writes basic cell value attributes to the specified <code>Outputstream</code>
     *
     * @param output the <code>OutputStream</code> to write to
     */
    public void write(OutputStream output) throws IOException {

        output.write(rw);
        output.write(col);
        output.write(ixfe);
    }

    /**
     * Read a <code>LabelCell</code> from the specified <code>InputStream</code>
     *
     * @param input the <code>InputStream</code> to read from
     */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(rw);
        col                 += input.read();
        numOfBytesRead++;
        numOfBytesRead      += input.read(ixfe);

        Debug.log(Debug.TRACE, "\tRow : "+ EndianConverter.readShort(rw) +
                            " Column : " + col +
                            " ixfe : " + EndianConverter.readShort(ixfe));

        return numOfBytesRead;
    }


    /**
     * Returns the contents of the cell as a String
     *
     * @return the contents of the cell
     */
    abstract public String getString() throws IOException;

}

