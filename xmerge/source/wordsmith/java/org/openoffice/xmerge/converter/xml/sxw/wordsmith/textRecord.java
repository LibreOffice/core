/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textRecord.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:16:19 $
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

