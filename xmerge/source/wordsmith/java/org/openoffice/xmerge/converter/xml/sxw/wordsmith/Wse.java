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

