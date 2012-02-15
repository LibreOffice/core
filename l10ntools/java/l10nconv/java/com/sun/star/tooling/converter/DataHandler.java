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
 * DataHandler.java
 *
 * take the data from the reader
 * and put it to the Writer
 *
 */

package com.sun.star.tooling.converter;

import java.util.*;

/**
 * Handle the Data to get it from the Source
 * readable to the Target
 *
 * @author Christian Schmidt
 */
public class DataHandler {

    /**
     * An arrays that holds the names that will be
     * keys for the HashMap containing the data
     *
     */
    private final String[]      dataNames = { "BlockNr", "Project",
            "SourceFile", "Dummy", "ResType", "GID", "LID", "HID", "Platform",
            "Width", "SourceLanguageID", "SourceText", "SourceHText",
            "SourceQText", "SourceTitle", "TargetLanguageID", "TargetText",
            "TargetHText", "TargetQText", "TargetTitle", "TimeStamp" };

    private static final String EMPTY     = new String("");

    /**
     * The HashMap containing the data
     */
    private final Map           data      = new ExtMap(dataNames, null);

    /** Creates a new instance of DataHandler */
    public DataHandler() {
    }

    /**
     * fill the data from the desired source
     *
     * @param source where to get the data from
     * @return true if data is read and false if null is read
     * @throws IOException
     * @throws ConverterException
     */
    public boolean fillDataFrom(DataReader source) throws java.io.IOException,
            ConverterException {

        Map line = null;

        line = source.getData();
        if (line == null){

            return false;
        }else{

            this.data.putAll(line);
            return true;
        }


    }

    /**
     * fill this data with the inData
     *
     * @param inData the data to handle by this handler
     */
    public void fillDataWith(Map inData) {
        data.putAll(inData);
    }

//    public void transfer(DataWriter target, DataReader source) {
//
//        source.setHandler(this);
//
//    }

    /**
     * The designated output is filled with the content of this handler
     *
     * @param output an array of Maps [0] should hold the source language data [1] the target language data
     * @throws java.io.IOException
     */
    public void putDataTo(Map[] output) throws java.io.IOException {
        String aKey = EMPTY;
        for (int j = 0; j < output.length; j++) {
            Set keys = output[j].keySet();
            Iterator iter = keys.iterator();
            while (iter.hasNext()) {
                aKey = (String) iter.next();
                output[j].put(aKey, data.get(aKey));
            }
        }

    }
    /**
     * The designated output is filled with the content of this handler
     *
     * @param output a Map that should hold the source language data and the target language data
     * @throws java.io.IOException
     */
    public void putDataTo(Map output) throws java.io.IOException {
        String aKey = EMPTY;

        Set keys = output.keySet();
        Iterator iter = keys.iterator();
        while (iter.hasNext()) {
            aKey = (String) iter.next();
            output.put(aKey, data.get(aKey));
        }

    }

}