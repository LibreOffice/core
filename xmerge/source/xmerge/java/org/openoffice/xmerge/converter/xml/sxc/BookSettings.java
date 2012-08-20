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

package org.openoffice.xmerge.converter.xml.sxc;

import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

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
    private String  activeSheet         = new String();
    private ArrayList<SheetSettings>  worksheetSettings   = new ArrayList<SheetSettings>();

    /**
     * Constructor for a <code>BookSettings</code>. Reads document settings
     * from xml and inits SheetSettings variables.
     *
     * @param root The root XML node to read from.
     */
    public BookSettings(Node root) {
        readNode(root);
    }

    /**
     * Constructor for a <code>BookSettings</code>
     *
     * @param worksheetSettings If it's a row the height, a column the width
     */
    public BookSettings(ArrayList<SheetSettings> worksheetSettings) {
        this.worksheetSettings = worksheetSettings;
    }

    /**
     * Set the flag indicating whether we have row/column headers.
     *
     * @param hasColumnRowHeaders Flag to enable or disable headers.
     */
    public void setColumnRowHeaders(boolean hasColumnRowHeaders) {
        this.hasColumnRowHeaders = hasColumnRowHeaders;
    }

    /**
     * Get the flag indicating whether we have row/column headers.
     *
     * @return Flag indicating whether we have row/column headers.
     */
    public boolean hasColumnRowHeaders() {
        return hasColumnRowHeaders;
    }

    /**
     * Gets the <code>Vector</code> of <code>SheetSettings</code>
     *
     * @return <code>Vector</code> of <code>SheetSettings</code>
     */
    public ArrayList<SheetSettings> getSheetSettings() {
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
     * @param attribute the name of the attribute to add
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
        Element configItemMapNamed      = settings.createElement(TAG_CONFIG_ITEM_MAP_NAMED);
        configItemMapNamed.setAttribute(ATTRIBUTE_CONFIG_NAME, "Tables");
        for(Iterator<SheetSettings> e = worksheetSettings.iterator();e.hasNext();) {
            SheetSettings s = e.next();
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
