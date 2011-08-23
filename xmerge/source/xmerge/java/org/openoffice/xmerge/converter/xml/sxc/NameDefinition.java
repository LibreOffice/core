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

package org.openoffice.xmerge.converter.xml.sxc;

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

    private String name; 						// name which identifies the definition
    private String definition;					// the definition itself
    private String baseCellAddress;				// the basecelladdress
    private boolean rangeType = false;			// true if definition of type range
    private boolean expressionType = false;		// true if definition of type expression

    /**
     * Default Constructor for a <code>NameDefinition</code>
     *
     */
    public NameDefinition() {

    }
    
    /**
     * Constructor that takes a <code>Node</code> to build a
     * <code>NameDefinition</code>
     *
     * @param root XML Node to read from 
     */
    public NameDefinition(Node root) {
        readNode(root);
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
