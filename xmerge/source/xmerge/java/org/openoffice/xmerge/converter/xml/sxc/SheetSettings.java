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

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import java.awt.Point;

import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 * This is a class representing the different attributes for a worksheet
 * contained in settings.xml.  
 *
 * @author Martin Maher
 */
public class SheetSettings implements OfficeConstants {

    /**  A w3c <code>Document</code>. */
    private org.w3c.dom.Document settings = null;

    private String	sheetName;
    private int		cursorX		= 0;
    private int		cursorY		= 0;	
    private int		splitTypeX;
    private int		splitTypeY;
    private int		splitPointX	= 0;
    private int		splitPointY	= 0;
    private int		posLeft 	= 0;
    private int		posRight	= 0;
    private int		posBottom	= 0;
    private int		posTop		= 0;
    private int		paneNumber	= 2;
    
    final public static int NONE	= 0x00;
    final public static int SPLIT	= 0x01;
    final public static int FREEZE	= 0x02;
    
    
    /**
     * Default Constructor for a <code>ColumnRowInfo</code>
     *
     */
    public SheetSettings() {
    }
    
    /**
     * Constructor that takes a <code>Node</code> to build a <code>SheetSettings</code>
     *
     * @param root XML Node to read from 
     */
    public SheetSettings(Node root) {
        readNode(root);
    }

    /**
     * Constructor for a <code>ColumnRowInfo</code>
     *
     * @param dimension if it's a row the height, a column the width 
     * @param repeated 
     */
    public SheetSettings(String name) {
        sheetName = name;
    }
    
    /**
     * sets the position of the acitve cell 
     *
     * @param activeCell the current curor position 
     */
    public void setCursor(Point activeCell) {

        cursorX = (int) activeCell.getX();
        cursorY = (int) activeCell.getY();
    }
    
    /**
     * Gets the position of the acitve cell 
     *
     * @return The position as a <code>Point</code>
     */
    public Point getCursor() {

        return (new Point(cursorX, cursorY));
    }

    /**
     * Sets the position  of the freeze 
     *
     * @param splitPoint the point at where the split occurs
     */
    public void setFreeze(Point splitPoint) {

        splitTypeX		= FREEZE;
        splitTypeY		= FREEZE;
        splitPointX		= (int) splitPoint.getX();
        splitPointY		= (int) splitPoint.getY();
    }

    /**
     * Sets the position of the split 
     *
     * @param splitPoint the point at where the split occurs
     */
    public void setSplit(Point splitPoint) {

        splitTypeX		= SPLIT;
        splitTypeY		= SPLIT;
        splitPointX		= (int) splitPoint.getX();
        splitPointY		= (int) splitPoint.getY();
    }
    
    /**
     * sets the position and type of the split 
     *
     * @return The position as a <code>Point</code> where the split occurs
     */
    public Point getSplit() {

        return (new Point(splitPointX, splitPointY));
    }
    
    /**
     * sets the position and type of the split 
     *
     * @return The position as a <code>Point</code> where the split occurs
     */
    public Point getSplitType() {

        return (new Point(splitTypeX, splitTypeY));
    }

    /**
     * Sets the top row visible in the lower pane and the leftmost column
     * visibile in the right pane.
     *
     * @param top The top row visible in the lower pane 
     * @param left The leftmost column visibile in the right pane
     */
    public void setTopLeft(int top, int left) {

        posLeft = left;
        posTop = top;
    }
    
    /**
     * Gets the the leftmost column visibile in the right pane.
     *
     * @return the 0-based index to the column 
     */
    public int getLeft() {

        return posLeft;
    }
    /**
     * Sets the top row visible in the lower pane and the leftmost column
     * visibile in the right pane.
     *
     * @param top The top row visible in the lower pane 
     * @param left The leftmost column visibile in the right pane
     */
    public int getTop() {

        return posTop;
    }
    
    /**
     * Gets the active Panel  
     * 0 - Bottom Right, 1 - Top Right
     * 2 - Bottom Left, 3 - Top Left
     *
     * @return int representing the active panel
     */
    public int getPaneNumber() {

        return paneNumber;
    }
    
    /**
     * Sets the sheetname this settings object applies to 
     *
     * @param sheetName the name of the worksheet 
     */
    public void setSheetName(String sheetName) {

        this.sheetName = sheetName;

    }
    
    /**
     * Sets the active pane number
     * 0 - Bottom Right, 1 - Top Right
     * 2 - Bottom Left, 3 - Top Left
     *
     * @param paneNumber the pane number 
     */
    public void setPaneNumber(int paneNumber) {

        this.paneNumber = paneNumber;
    }
    
    /**
     * Gets the name of the worksheet these <code>Settings</code> apply to 
     *
     * @return the name of the worksheet
     */
    public String getSheetName() {

        return sheetName;
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
     * Writes out a settings.xml entry for this SheetSettings object 
     *
     * @param settings a <code>Document</code> object representing the settings.xml 
     * @param root the root xml node to add to
     */
    public void writeNode(org.w3c.dom.Document settings, Node root) {
    
        this.settings = settings;
        Element configItemMapEntry		= (Element) settings.createElement(TAG_CONFIG_ITEM_MAP_ENTRY);
        configItemMapEntry.setAttribute(ATTRIBUTE_CONFIG_NAME, getSheetName());
        addConfigItem(configItemMapEntry, "CursorPositionX", "int", Integer.toString(cursorX));	
        addConfigItem(configItemMapEntry, "CursorPositionY", "int", Integer.toString(cursorY));	

        String splitMode = Integer.toString(splitTypeX);
        if(splitPointX==0) {
            splitMode = "0";
        }
        addConfigItem(configItemMapEntry, "HorizontalSplitMode", "short", splitMode);	

        splitMode = Integer.toString(splitTypeY);
        if(splitPointY==0) {
            splitMode = "0";
        }
        addConfigItem(configItemMapEntry, "VerticalSplitMode", "short", splitMode);	
        
        addConfigItem(configItemMapEntry, "HorizontalSplitPosition", "int", Integer.toString(splitPointX));	
        addConfigItem(configItemMapEntry, "VerticalSplitPosition", "int", Integer.toString(splitPointY));	
        addConfigItem(configItemMapEntry, "ActiveSplitRange", "short", Integer.toString(paneNumber));	
            
        addConfigItem(configItemMapEntry, "PositionLeft", "int", "0");	
        addConfigItem(configItemMapEntry, "PositionRight", "int", Integer.toString(posLeft));	
        addConfigItem(configItemMapEntry, "PositionTop", "int", "0");	
        addConfigItem(configItemMapEntry, "PositionBottom", "int", Integer.toString(posTop));	
        root.appendChild(configItemMapEntry);
    }

    /**
     * Sets a variable based on a String value read from XML 
     *
     * @param name xml name of the attribute to set 
     * @param value String value fo the attribute 
     */
    public void addAttribute(String name, String value) {

        if(name.equals("CursorPositionX")) {
            cursorX = Integer.parseInt(value);
        } else if(name.equals("CursorPositionY")) {
            cursorY = Integer.parseInt(value);
            
        } else if(name.equals("HorizontalSplitPosition")) {
            splitPointX = Integer.parseInt(value);
        } else if(name.equals("VerticalSplitPosition")) {
            splitPointY = Integer.parseInt(value);
        } else if(name.equals("ActiveSplitRange")) {
            paneNumber = Integer.parseInt(value);

        } else if(name.equals("PositionRight")) {
            posLeft = Integer.parseInt(value);
        } else if(name.equals("PositionBottom")) {
            posTop = Integer.parseInt(value);
        
        } else if(name.equals("HorizontalSplitMode")) {
            splitTypeX = Integer.parseInt(value);
        } else if(name.equals("VerticalSplitMode")) {
            splitTypeY = Integer.parseInt(value);
        }
    }

    /**
     * Reads document settings from xml and inits SheetSettings variables
     *
     * @param root XML Node to read from 
     */
    public void readNode(Node root) {
    
        NamedNodeMap sheetAtt = root.getAttributes();

        Node sheetNameNode = sheetAtt.getNamedItem(ATTRIBUTE_CONFIG_NAME);

        sheetName = sheetNameNode.getNodeValue();

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
                    }			
                }
            }
        }
    }
}
