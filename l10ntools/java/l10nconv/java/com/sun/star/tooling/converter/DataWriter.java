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


/*
 * DataWriter.java
 *
 * parent of all XXXWriter classes used by
 * Converter
 */

package com.sun.star.tooling.converter;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

/**
 * The abstract parent class of all converter writer classes
 *
 * @author Christian Schmidt
 */
abstract public class DataWriter extends OutputStreamWriter {
    protected final String seperator = new String("|");

    /**
     * the char sequence used as line seperator
     */
    protected final String lineEnd   = java.lang.System.getProperty(
                                             "line.seperator", "\n");

    /** Creates a new instance of DataWriter */
    /**
     * @param bos the buffered output stream holding the data
     * @param encoding the encoding to use for read from bos
     * @throws java.io.UnsupportedEncodingException
     */
    public DataWriter(BufferedOutputStream bos, String encoding)
            throws java.io.UnsupportedEncodingException {
        super(bos, encoding);
    }

//    abstract protected void writeData(Map[] data) throws java.io.IOException;

    /**
     * get the data that should be written from the DataHandler
     *
     * @param handler the DataHandler having the data
     * @throws java.io.IOException
     */
    abstract protected void getDataFrom(DataHandler handler)
            throws java.io.IOException;

    /**
     * write the Data
     *
     * @throws java.io.IOException
     */
    abstract protected void writeData() throws java.io.IOException;

    /**
     * @param handler
     * @throws IOException
     */
    abstract protected void getDatafrom(DataHandler handler) throws IOException;
}