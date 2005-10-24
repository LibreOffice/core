/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

