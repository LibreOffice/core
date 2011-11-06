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

