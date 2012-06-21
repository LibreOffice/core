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

