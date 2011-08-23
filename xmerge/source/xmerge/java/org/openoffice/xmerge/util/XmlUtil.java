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

package org.openoffice.xmerge.util;

import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;

/**
 *  Class containing static util methods for handling XML trees.
 *
 *  @author     smak
 */
public final class XmlUtil {


    /**
     *  Perform a deep clone of certain <code>Node</code> which
     *  will base on the document <code>Node</code> of the old
     *  <code>Node</code>.
     *
     *  @param  oldNode  The <code>Document</code> of this
     *                   <code>Node</code> is used to clone
     *                   the <code>Node</code>
     *  @param  newNode  The <code>Node</code> to clone.
     *
     *  @return  The cloned <code>Node</code>.
     */
    public static Node deepClone(Node oldNode, Node newNode) {
        Document docNode = oldNode.getOwnerDocument();

        // clone the starting node
        Node clonedNode = cloneNode(docNode, newNode);

        // then clone the sub-tree recursively
        cloneTree(docNode, clonedNode, newNode);

        return clonedNode;
    }


    /**
     *  Clone the sub-tree under certain given <code>Node</code>
     *
     *  @param  docNode  The <code>Document</code> used to clone
     *                   the <code>Node</code>.
     *  @param  oldNode  The <code>Node</code> to clone.
     *  @param  newNode  The destination <code>Node</code>.
     */
    private static void cloneTree(Document docNode, Node oldNode, Node newNode) {

        NodeList nodeList = newNode.getChildNodes();
        int nodeListLen = nodeList.getLength();

        for (int i = 0; i < nodeListLen; i++) {
            Node newClonedChild = cloneNode(docNode, nodeList.item(i));
            if (newClonedChild != null) {
                oldNode.appendChild(newClonedChild);
                cloneTree(docNode, newClonedChild, nodeList.item(i));
            }
        }
    }


    /**
     * Clone a <code>Node</code> (either text or element).
     *
     * @param  docNode  The <code>Document</code> used to
     *                  clone the <code>Node</code>.
     * @param  newNode  The <code>Node</code> to clone.
     *
     * @return  The cloned <code>Node</code>.
     */
    private static Node cloneNode(Document docNode, Node newNode) {

        Node clonedNode = null;

        // only support text node and element node (will copy the attributes)
        switch (newNode.getNodeType()) {
            case Node.TEXT_NODE:
                String textStr = newNode.getNodeValue();
                clonedNode = docNode.createTextNode(textStr);
                break;
            case Node.ELEMENT_NODE:
                Element oldElem = (Element)newNode;
                String tagName  = newNode.getNodeName();
                Element newElem = (docNode.createElement(tagName));

                // copy the attributes
                NamedNodeMap attrs = oldElem.getAttributes();

                for (int i = 0; i < attrs.getLength(); i++) {
                    newElem.setAttribute(attrs.item(i).getNodeName(),
                                         attrs.item(i).getNodeValue());
                }
                clonedNode = newElem;
                break;
        }
        return clonedNode;
    }


    /**
     *  Returns the name and type of an XML DOM <code>Node</code>.
     *
     *  @param  node  <code>Node</code> to query.
     *
     *  @return  Name and type of XML DOM <code>Node</code>.
     */
    public static String getNodeInfo(Node node) {
    
        String str = null;
        switch (node.getNodeType()) {
    
            case Node.ELEMENT_NODE:
                str = "ELEMENT";
                break;
            case Node.ATTRIBUTE_NODE:
                str = "ATTRIBUTE";
                break;
            case Node.TEXT_NODE:
                str = "TEXT";
                break;
            case Node.CDATA_SECTION_NODE:
                str = "CDATA_SECTION";
                break;
            case Node.ENTITY_REFERENCE_NODE:
                str = "ENTITY_REFERENCE";
                break;
            case Node.ENTITY_NODE:
                str = "ENTITY";
                break;
            case Node.PROCESSING_INSTRUCTION_NODE:
                str = "PROCESSING_INSTRUCTION";
                break;
            case Node.COMMENT_NODE:
                str = "COMMENT";
                break;
            case Node.DOCUMENT_NODE:
                str = "DOCUMENT";
                break;
            case Node.DOCUMENT_TYPE_NODE:
                str = "DOCUMENT_TYPE";
                break;
            case Node.DOCUMENT_FRAGMENT_NODE:
                str = "DOCUMENT_FRAGMENT";
                break;
            case Node.NOTATION_NODE:
                str = "NOTATION";
                break;
        }

        StringBuffer buffer = new StringBuffer("name=\"");
        buffer.append(node.getNodeName());
        buffer.append("\"  type=\"");
        buffer.append(str);
        buffer.append("\"");

        return buffer.toString();
    }
}

