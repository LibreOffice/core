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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;

import org.openoffice.xmerge.converter.xml.OfficeConstants;

import java.util.Vector;
import java.util.List;


/**
 *  <p>This is a parser to return a character array for difference purpose.
 *  It will use depth first search to traverse all the characters inside the
 *  text <code>Node</code> under a given <code>Node</code> (most likely to be
 *  a paragraph <code>Node</code>).</p>
 *
 *  <p>Note: Once the XML Tree is parsed, then the <code>Iterator</code> will be
 *  a snap shot of  that tree.  That means even the tree is modified later, than
 *  the cached paragraph <code>Node</code> list will not be updated accordingly.
 *  For this reason and for performance reasons this <code>Iterator</code> does
 *  not support any operation methods such as insert, remove or replace.  The
 *  main purpose of this <code>Iterator</code> is to be used with difference,
 *  not with merge.</p>
 *
 *  @author smak
 */
public class CharacterParser {

    private TextNodeIterator textNodes;
    private int currentPosition = 0;
    private List nodeList_ = null;
    private char[] charArray;


    /**
     *  Standard constructor.
     *
     *  @param  node  The initial root <code>Node</code>.
     */
    public CharacterParser(Node node) {
        textNodes = new TextNodeIterator(node);
        nodeList_ = new Vector();

        parseNodes();
    }


    /**
     *  Returns the <code>Node</code> pointer with the given character position.
     *
     *  @return  The <code>Node</code> pointer with the given character position.
     */
    public List getNodeList() {
        // will go through the nodeList to find the corresponding node
        return  nodeList_;
    }

    /**
     *  Returns the character array representation of the text.
     *
     *  @return  The character array representation of the text.
     */
    public char[] getCharArray() {
        return charArray;
    }

    private void parseNodes() {

        StringBuffer strBuf = new StringBuffer();

        /* create the character array by iterate the textnode iterator */
        Node currentNode = (Node)(textNodes.start());
        for (;
             currentNode != null;
             currentNode = (Node)(textNodes.next())) {

            // add the text value into the array
            String textValue  = null;
            String nodeName = currentNode.getNodeName();

            // TODO: Space node have a count attribute which is not handled!
            if (currentNode.getNodeType() == Node.TEXT_NODE) {
                textValue = currentNode.getNodeValue();
            } else if (nodeName.equals(OfficeConstants.TAG_SPACE)) {
                textValue = " ";
            } else if (nodeName.equals(OfficeConstants.TAG_TAB_STOP)) {
                textValue = "\t";
            }

            if (textValue != null) {
                strBuf.append(textValue);
                addNewNodeEntry(textValue.length(), currentNode);
            }
        }

        charArray = strBuf.toString().toCharArray();
    }


    /**
     *  Adds a new <code>Node</code> entry.
     *
     *  @param  textLen  The text length.
     *  @param  node     The <code>Node</code>.
     */
    private void addNewNodeEntry(int textLen, Node node) {

        TextNodeEntry nodeEntry = new TextNodeEntry(currentPosition,
                                      currentPosition + textLen - 1, node);
        currentPosition     = currentPosition + textLen;

        nodeList_.add(nodeEntry);
    }
}

