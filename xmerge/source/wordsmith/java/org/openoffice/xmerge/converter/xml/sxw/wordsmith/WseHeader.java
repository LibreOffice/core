/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WseHeader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:15:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

