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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;

/**
 *  This class represents a single text record in a WordSmith document.
 *  A record is composed of one or more "WordSmith elements", which
 *  include: WordSmith header, font table, color table, paragraphs,
 *  and text runs.
 *
 */

class textRecord {

    java.util.ArrayList<Wse> elements;


    /**
     *  Default constructor
     */
    textRecord() {
        elements = new java.util.ArrayList<Wse>(10);
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
            Wse e = elements.get(i);
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

        try {
            bs = new ByteArrayOutputStream();
            os = new DataOutputStream(bs);
            int nElements = elements.size();
            for (int i = 0; i < nElements; i++) {
                Wse e = elements.get(i);
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

