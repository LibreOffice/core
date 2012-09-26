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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;

import org.openoffice.xmerge.converter.xml.OfficeConstants;

import java.util.ArrayList;
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
 */
public class CharacterParser {

    private TextNodeIterator textNodes;
    private int currentPosition = 0;
    private List<TextNodeEntry> nodeList_ = null;
    private char[] charArray;


    /**
     *  Standard constructor.
     *
     *  @param  node  The initial root <code>Node</code>.
     */
    public CharacterParser(Node node) {
        textNodes = new TextNodeIterator(node);
        nodeList_ = new ArrayList<TextNodeEntry>();

        parseNodes();
    }


    /**
     *  Returns the <code>Node</code> pointer with the given character position.
     *
     *  @return  The <code>Node</code> pointer with the given character position.
     */
    public List<TextNodeEntry> getNodeList() {
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

