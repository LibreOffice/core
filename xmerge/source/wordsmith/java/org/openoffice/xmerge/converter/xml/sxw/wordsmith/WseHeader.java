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
     *  @param  startIndex  The start index.
     *
     *  @return  The first <code>byte</code> following this element.
     */
    static int computeNewIndex(int startIndex) {
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
        ByteArrayOutputStream bs = new ByteArrayOutputStream();
        DataOutputStream os = new DataOutputStream(bs);

        try {
            os.write(2);  // binary doc indicator
            os.write(4);  // binary header indicator

            os.writeInt(nParagraphs);
            os.writeInt(nAtoms);
            os.writeInt(nChars);
            os.writeInt(miscSize);

        } catch (IOException e) {
            e.printStackTrace();
        }

        return bs.toByteArray();
    }
}

