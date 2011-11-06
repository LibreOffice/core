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


package com.sun.star.tooling.converter;

import java.io.*;
import java.util.*;

/**
 * Gets Line counting from LineNumberReader all Converter Reader classes inherit
 * from this.
 * The abstract parent class of all converter reader classes
 *
 * @author Christian Schmidt
 */
abstract public class DataReader extends LineNumberReader {

    /**
     * Creates a new instance of DataReader
     *
     * @param isr
     *            InputStreamReader used as Source for this class
     */
    public DataReader(InputStreamReader isr) {
        super(isr);
    }

    /**

     * @throws java.io.IOException
     * @throws ConverterException
     *
     * TODO this should no longer use an array as return type better a Map
     *
     */
    /**
     * The next block of the SDF file is reviewed and the Line including the
     * source language and the Line including the target Language are given back
     * in an array
     *
     *
     * @return    A Map including the source language
     *            and the target Language content are given back
     *
     * @throws java.io.IOException
     * @throws ConverterException
     */
    public Map getData() throws java.io.IOException, ConverterException {
        return null;
    }


}