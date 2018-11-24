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

 public interface BIFFRecord {

    /**
     * Get the type of the record. In the BIFF file format each record has a type
     * designated with a byte value. See @link PocketExcelBiffConstants
     * for a list of the BIFF constants and what they mean.
     *
     * @return byte The BIFF record value.
     */
    public short getBiffType();

    /**
     * Read from the input stream <b>NB</b>The input stream is assumed to be in
     * Little Endian format. The Biff identifier is expected to be in the stream.
     *
     *  @param input The InputStream to read from.
     *  @return The number of bytes that were read in.
     */
    public int read(InputStream input) throws IOException;

    /**
     * Writes the record, including the BIFF record byte to the outputstream
     * @param output The output stream to write to in LittleEndian format.
     */
    public void write(OutputStream output) throws IOException;

}

