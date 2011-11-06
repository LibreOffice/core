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



package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import org.openoffice.xmerge.util.Debug;
import java.io.IOException;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;

/**
 *  This class represents a single text record in a WordSmith document.
 *  A record is composed of one or more "WordSmith elements", which
 *  include: WordSmith header, font table, color table, paragraphs,
 *  and text runs.
 *
 *  @author   David Proulx
 */

class textRecord {

    java.util.Vector elements;


    /**
     *  Default constructor
     */
    textRecord() {
        elements = new java.util.Vector(10);
    }


    /**
     *  Add an element
     *
     *  @param  elem  The element to add
     */
    void addElement(Wse elem) {
        elements.add(elem);
    }


    /**
     *  Return the number of bytes needed to represent the current
     *  contents of this text record.
     *
     *  @return  The number of bytes needed to represent the current
     *           contents of this text record.
     */
    int getByteCount() {
        int totalBytes = 0;
        int nElements = elements.size();
        for (int i = 0; i < nElements; i++) {
            Wse e = (Wse)elements.elementAt(i);
            totalBytes += e.getByteCount();
        }
        return totalBytes;
    }


    /**
     *  Return the contents of this record as a <code>byte</code> array.
     *
     *  @return the contents of this record as a <code>byte</code> array.
     */
    byte[] getBytes() {
        DataOutputStream os = null;  // Used for storing the data
        ByteArrayOutputStream bs = null;  // Used for storing the data
        byte ftBytes[] = null;
        byte ctBytes[] = null;

        try {
            bs = new ByteArrayOutputStream();
            os = new DataOutputStream(bs);
            int nElements = elements.size();
            for (int i = 0; i < nElements; i++) {
                Wse e = (Wse)elements.get(i);
                os.write(e.getBytes());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

        if (bs != null)
            return bs.toByteArray();
        else
            return null;
    }
}

