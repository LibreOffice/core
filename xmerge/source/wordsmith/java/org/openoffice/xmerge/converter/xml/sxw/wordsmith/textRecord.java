/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

