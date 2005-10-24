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

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.xml.*;


/**
 * This is the superclass for all elements in a WordSmith document.
 * Elements can be paragraphs, text runs, font tables, or color tables.
 *
 *  @author   David Proulx
 */
abstract class Wse {

    /**
     *  Return true if <code>dataArray[startIndex]</code> is the start
     *  of a valid element of this type.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     *
     *  @return  true if <code>dataArray[startIndex]</code> is the
     *           start of a valid element of this type, false otherwise.
     */
    static boolean isValid(byte dataArray[], int startIndex) {
        return false;
    }


    /**
     *  Compute and return the index of the first <code>byte</code>
     *  following this element.  It is assumed that the element
     *  starting at <code>dataArray[startIndex]</code> is valid.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     *
     *  @return  The index of the first <code>byte</code> following
     *           this element.
     */
    static int computeNewIndex(byte dataArray[], int startIndex) {
        return 0;
    }


    /**
     *  Return the total number of bytes needed to represent this
     *  object.
     *
     *  @return  The total number of bytes needed to represent this
     *           object.
     */
    abstract int getByteCount();


    /**
     *  Return an <code>byte</code> array representing this element.
     *
     *  @return  An <code>bytes</code> array representing this element.
     */
    abstract byte[] getBytes();
}

