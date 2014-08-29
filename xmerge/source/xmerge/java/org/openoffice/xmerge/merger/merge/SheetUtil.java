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

package org.openoffice.xmerge.merger.merge;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 * Utility methods to handle sheet XML tree.
 */
public class SheetUtil {

    /**
     * Empty the content of a cell value.
     *
     * <p>This includes the following:</p>
     *
     * <ul>
     *   <li>
     *     Remove all of the supported attributes.
     *   </li><li>
     *     Remove the first <i>text:p</i> {@code Node} for most of the cells.
     *   </li>
     * </ul>
     *
     *  @param  cc    The {@code ConverterCapabilities}.
     *  @param  node  The {@code Node}.
     */
    public static void emptyCell(ConverterCapabilities cc, Node node) {

        NamedNodeMap attrNodes = node.getAttributes();

        if (attrNodes != null) {

            // empty the first text:p node.
            // Note: it's not necessary only string type cell contain text:p
            // basically, all different type of cell will contain one
            Element cell = (Element)node;

            // get the paragraph node list
            NodeList paraNodes =
                cell.getElementsByTagName(OfficeConstants.TAG_PARAGRAPH);

            Node firstParaNode = paraNodes.item(0);

            // remove the first paragraph element node
            if (firstParaNode != null) {
                Node parent = firstParaNode.getParentNode();
                parent.removeChild(firstParaNode);
            }

            // check all the attributes and remove those we supported in
            // converter
            // NOTE: for attribute list, refer to section 4.7.2 in specification
            int len = attrNodes.getLength();

            for (int i = 0; i < len; ) {
                Node attr = attrNodes.item(i);

                // when we hit the end of the attribute nodes, return
                // it may happen sooner as we keep on removing nodes
                if (attr == null) {
                    break;
                }
                // remove the supported attr except columns repeated attribute
                if (cc.canConvertAttribute(OfficeConstants.TAG_TABLE_CELL,
                                           attr.getNodeName()) &&
                    !attr.getNodeName().equals(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED)) {

                    attrNodes.removeNamedItem(attr.getNodeName());
                } else {
                    i++;
                }
            }
        }
    }
}