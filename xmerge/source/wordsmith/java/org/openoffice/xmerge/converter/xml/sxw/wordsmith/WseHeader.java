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

import java.io.IOException;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;

/**
 *  This class represents a WordSmith document header.
 *
 *  @author   David Proulx
 */
class WseHeader extends Wse {

    private int nParagraphs = 0;
    private int nAtoms = 0;
    private int nChars = 0;
    private int miscSize = 0;

    /**
     *  Constructor for use when going from DOM to WordSmith.
     *
     *  @param  nPara   The number of paragraphs.
     *  @param  nAtoms  The number of atoms.
     *  @param  nChars  The number of characters.
     *  @param  ft      The font table.
     *  @param  ct      The color table.
     */
    public WseHeader(int nPara, int nAtoms, int nChars, WseFontTable ft,
                     WseColorTable ct) {
        nParagraphs = nPara;
        this.nAtoms = nAtoms;
        this.nChars = nChars;
        if (ft != null) miscSize += ft.getByteCount();
        if (ct != null) miscSize += ct.getByteCount();
    }


    /**
     *  Constructor for use when going from WordSmith to DOM.
     *
     *  @param  dataArray  <code>byte</code> array.
     *  @param  i          Index.
     */
    public WseHeader(byte dataArray[], int i) {
    // DJP: write this!
    }
    
    /**
     *  Return true if <code>dataArray[startIndex]</code> is the start
     *  of a document header.
     *
     *  @param dataArray   <code>byte</code> array.
     *  @param startIndex  The index.
     *
     *  @return  true if <code>dataArray[startIndex]</code> is the start
     *           of a document header, false otherwise.
     */
    static boolean isValid(byte dataArray[], int startIndex) {
        return ((dataArray[startIndex] == 2) 
              && (dataArray[startIndex + 1] == 4));
    }

    
    /**
     *  Compute and return the index of the first <code>byte</code>
     *  following this element.  It is assumed that the element
     *  starting at <code>dataArray[startIndex]</code> is valid.
     * 
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     * 
     *  @return  The first <code>byte</code> following this element.
     */
    static int computeNewIndex(byte dataArray[], int startIndex) {
        return startIndex + 18;
    }


    /**
     *  Return the total number of bytes needed to represent this.
     *
     *  @return  The total number of bytes needed to represent this.
     */
    int getByteCount() {
        return 18;
    }

        
    /**
     *  Return a <code>byte</code> array representing this element.
     *
     *  @return  A <code>byte</code> array representing this element.
     */
    byte[] getBytes() {
        DataOutputStream os;  // Used for storing the data
        ByteArrayOutputStream bs = null;  // Used for storing the data

        try {
            bs = new ByteArrayOutputStream();
            os = new DataOutputStream(bs);
            os.write(2);  // binary doc indicator
            os.write(4);  // binary header indicator

            os.writeInt(nParagraphs);
            os.writeInt(nAtoms);
            os.writeInt(nChars);
            os.writeInt(miscSize);

        } catch (IOException e) { 
            e.printStackTrace(); 
        }

        if (bs != null) {
            return bs.toByteArray();
        } else return null;
    }
}

