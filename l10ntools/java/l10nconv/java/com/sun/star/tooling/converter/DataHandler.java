/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataHandler.java,v $
 * $Revision: 1.3 $
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