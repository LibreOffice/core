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

package org.openoffice.xmerge.util;

import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;

/**
 *  Class containing static util methods for handling XML trees.
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

