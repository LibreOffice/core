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

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.Element;

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

