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

package org.openoffice.xmerge.converter.xml.sxc;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;

/**
 * This is a class to define a Name Definition structure. This can then be
 * used by plugins to write or read their own definition types.
 *
 * @author Martin Maher
 */
public class NameDefinition implements OfficeConstants {

    private String name;                        // name which identifies the definition
    private String definition;                  // the definition itself
    private String baseCellAddress;             // the basecelladdress
    private boolean rangeType = false;          // true if definition of type range
    private boolean expressionType = false;     // true if definition of type expression

    /**
     * Default Constructor for a <code>NameDefinition</code>
     *
     */
    public NameDefinition() {

    }

    /**
     * Default Constructor for a <code>NameDefinition</code>
     *
     */
    public NameDefinition(String name, String definition, String
    baseCellAddress, boolean rangeType, boolean expressionType ) {
        this.name = name;
        this.definition = definition;
        this.baseCellAddress = baseCellAddress;
        this.rangeType = rangeType;
        this.expressionType = expressionType;
    }

    /**
     * returns Name of the definition
     *
     * @return the name which identifies the definition
     */
    public String getName() {

        return name;
    }
    /**
     * sets the definition
     *
     * @param newDefinition sets the definition
     */
    public void setDefinition(String newDefinition) {

        definition = newDefinition;
    }
    /**
     * Returns the definition itself
     *
     * @return the definition
     */
    public String getDefinition() {

        return definition;
    }

    /**
     * Returns the base Cell address
     *
     * @return the base cell address
     */
    public String getBaseCellAddress() {

        return baseCellAddress;
    }

    /**
     * Tests if definition is of type expression
     *
     * @return whether or not this name definition is of type expression
     */
    public boolean isExpressionType() {
        return expressionType;
    }

    /**
     * Tests if definition is of type range
     *
     * @return whether or not this name definition is of type range
     */
    public boolean isRangeType() {
        return rangeType;
    }

    /**
     * Writes out a content.xml entry for this NameDefinition object
     *
     * @param settings a <code>Document</code> object representing the settings.xml
     * @param root the root xml node to add to
     */
    public void writeNode(org.w3c.dom.Document doc, Node root) {

            if(isRangeType()) {

                Debug.log(Debug.TRACE, "Found Range Name : " + getName());
                Element namedRangeElement = (Element) doc.createElement(TAG_TABLE_NAMED_RANGE);
                namedRangeElement.setAttribute(ATTRIBUTE_TABLE_NAME, getName());
                namedRangeElement.setAttribute(ATTRIBUTE_TABLE_BASE_CELL_ADDRESS, getBaseCellAddress());
                namedRangeElement.setAttribute(ATTRIBUTE_TABLE_CELL_RANGE_ADDRESS, getDefinition());
                root.appendChild(namedRangeElement);
            } else if (isExpressionType()) {

                Debug.log(Debug.TRACE, "Found Expression Name : " + getName());
                Element namedExpressionElement = (Element) doc.createElement(TAG_TABLE_NAMED_EXPRESSION);
                namedExpressionElement.setAttribute(ATTRIBUTE_TABLE_NAME, getName());
                namedExpressionElement.setAttribute(ATTRIBUTE_TABLE_BASE_CELL_ADDRESS,getBaseCellAddress());
                namedExpressionElement.setAttribute(ATTRIBUTE_TABLE_EXPRESSION, getDefinition());
                root.appendChild(namedExpressionElement);
            } else {

                Debug.log(Debug.TRACE, "Unknown Name Definition : " + getName());
            }
    }

    /**
     * Reads document settings from xml and inits Settings variables
     *
     * @param root XML Node to read from
     */
    public void readNode(Node root) {

        String nodeName = root.getNodeName();
        NamedNodeMap cellAtt = root.getAttributes();

        if (nodeName.equals(TAG_TABLE_NAMED_RANGE)) {

            Node tableNameNode =
                cellAtt.getNamedItem(ATTRIBUTE_TABLE_NAME);
            Node tableBaseCellAddress =
                cellAtt.getNamedItem(ATTRIBUTE_TABLE_BASE_CELL_ADDRESS);
            Node tableCellRangeAddress =
                cellAtt.getNamedItem(ATTRIBUTE_TABLE_CELL_RANGE_ADDRESS);
            Debug.log(Debug.TRACE,"Named-range : " + tableNameNode.getNodeValue());
                        // Create a named-range name definition
            name = tableNameNode.getNodeValue();
            definition = tableCellRangeAddress.getNodeValue();
            baseCellAddress = tableBaseCellAddress.getNodeValue();
            expressionType = true;
            rangeType = false;

        } else if (nodeName.equals(TAG_TABLE_NAMED_EXPRESSION)) {

            Node tableNameNode =
                                cellAtt.getNamedItem(ATTRIBUTE_TABLE_NAME);
            Node tableBaseCellAddress =
                                cellAtt.getNamedItem(ATTRIBUTE_TABLE_BASE_CELL_ADDRESS);
            Node tableExpression=
                                cellAtt.getNamedItem(ATTRIBUTE_TABLE_EXPRESSION);
            Debug.log(Debug.TRACE,"Named-expression: " + tableNameNode.getNodeValue());
            // Create a named-range name definition
            name = tableNameNode.getNodeValue();
            definition = tableExpression.getNodeValue();
            baseCellAddress = tableBaseCellAddress.getNodeValue();
            expressionType = false;
            rangeType = true;
        } else {
            Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(root) + " />");
        }
    }

}
