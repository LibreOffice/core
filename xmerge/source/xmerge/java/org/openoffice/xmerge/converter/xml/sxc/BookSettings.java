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

import java.util.Vector;
import java.util.Enumeration;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;

/**
 * This is a class representing the different attributes for a worksheet
 * contained in settings.xml.  
 *
 * @author Martin Maher
 */
public class BookSettings implements OfficeConstants {

    /**  A w3c <code>Document</code>. */
    private org.w3c.dom.Document settings = null;
    
    private boolean hasColumnRowHeaders = true;
    private String 	activeSheet			= new String();
    private Vector	worksheetSettings	= new Vector();
    
    /**
     * Default Constructor for a <code>BookSettings</code>
     *
     * @param dimension if it's a row the height, a column the width 
     * @param repeated 
     */
    public BookSettings(Node root) {
        readNode(root);
    }

    /**
     * Default Constructor for a <code>BookSettings</code>
     *
     * @param worksheetSettings if it's a row the height, a column the width 
     */
    public BookSettings(Vector worksheetSettings) {
        this.worksheetSettings = worksheetSettings;
    }

    /**
     *
     */
    public void setColumnRowHeaders(boolean hasColumnRowHeaders) {
        this.hasColumnRowHeaders = hasColumnRowHeaders;
    }
    
    /**
     *
     */
    public boolean hasColumnRowHeaders() {
        return hasColumnRowHeaders;
    }

    /**
     * Gets the <code>Vector</code> of <code>SheetSettings</code>
     *
     * @return <code>Vector</code> of <code>SheetSettings</code>
     */
    public Vector getSheetSettings() {
        return worksheetSettings;
    }

    /**
     * Gets the active sheet name  
     *
     * @return the active sheet name 
     */
    public String getActiveSheet() {

        return activeSheet;
    }
    
    /**
     * Sets the active sheet name 
     *
     * @param activeSheet the active sheet name 
     */
    public void setActiveSheet(String activeSheet) {

        this.activeSheet = activeSheet;
    }
    

    /**
     * Adds an XML entry for a particular setting
     *
     * @param root the root node at which to add the xml entry
     * @param attriute the name of the attribute to add
     * @param type the attribute type (int, short etc)
     * @param value the value of the attribute
     */
    private void addConfigItem(Node root, String attribute, String type, String value) {

        Element configItem = settings.createElement(TAG_CONFIG_ITEM);
        configItem.setAttribute(ATTRIBUTE_CONFIG_NAME, attribute);
        configItem.setAttribute(ATTRIBUTE_CONFIG_TYPE, type);
                
        configItem.appendChild(settings.createTextNode(value));

        root.appendChild(configItem);
    }

    /**
     * Writes out a settings.xml entry for this BookSettings object 
     *
     * @param settings a <code>Document</code> object representing the settings.xml 
     * @param root the root xml node to add to
     */
    public void writeNode(org.w3c.dom.Document settings, Node root) {
    
        this.settings = settings;
        Element configItemMapNamed		= (Element) settings.createElement(TAG_CONFIG_ITEM_MAP_NAMED);
        configItemMapNamed.setAttribute(ATTRIBUTE_CONFIG_NAME, "Tables");
        for(Enumeration e = worksheetSettings.elements();e.hasMoreElements();) {
            SheetSettings s = (SheetSettings) e.nextElement();
            s.writeNode(settings, configItemMapNamed);
        }
        addConfigItem(root, "ActiveTable", "string", activeSheet);
        String booleanValue = Boolean.toString(hasColumnRowHeaders);
        addConfigItem(root, "HasColumnRowHeaders", "boolean", booleanValue);
        root.appendChild(configItemMapNamed);
    }

    /**
     * Sets a variable based on a String value read from XML 
     *
     * @param name xml name of the attribute to set 
     * @param value String value fo the attribute 
     */
    public void addAttribute(String name, String value) {

        if(name.equals("ActiveTable")) {
            activeSheet = value;
        } else if(name.equals("HasColumnRowHeaders")) {
            Boolean b = Boolean.valueOf(value);
            hasColumnRowHeaders = b.booleanValue();
        }
    }

    /**
     * Reads document settings from xml and inits SheetSettings variables
     *
     * @param root XML Node to read from 
     */
    public void readNode(Node root) {

        if (root.hasChildNodes()) {
        
            NodeList nodeList = root.getChildNodes();
            int len = nodeList.getLength();
            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_CONFIG_ITEM)) {

                        NamedNodeMap cellAtt = child.getAttributes();

                        Node configNameNode =
                            cellAtt.getNamedItem(ATTRIBUTE_CONFIG_NAME);
                    
                        String name = configNameNode.getNodeValue();
                        NodeList nodeList2 = child.getChildNodes();
                        int len2 = nodeList2.getLength();
                        String s = "";	
                        for (int j = 0; j < len2; j++) {
                            Node child2 = nodeList2.item(j);
                            if (child2.getNodeType() == Node.TEXT_NODE) {
                                s = child2.getNodeValue();
                            }
                        }
                        addAttribute(name, s);
                       
                     } else if (nodeName.equals(TAG_CONFIG_ITEM_MAP_NAMED)) {

                        readNode(child);
                    
                     } else if (nodeName.equals(TAG_CONFIG_ITEM_MAP_ENTRY)) {

                        SheetSettings s = new SheetSettings(child);
                        worksheetSettings.add(s);

                    } else {

                        Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }		
        }
    }
}
