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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.StyleCatalog;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.XmlUtil;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 *  <p>General spreadsheet implementation of <code>DocumentSerializer</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcPluginFactory
 *  SxcPluginFactory}.  Used with SXC <code>Document</code> objects.</p>
 *
 *  <p>The <code>serialize</code> method traverses the DOM
 *  <code>Document</code> from the given <code>Document</code> object.
 *  It uses a <code>DocEncoder</code> object for the actual conversion
 *  of contents to the device spreadsheet format.</p>
 *
 */
public abstract class SxcDocumentSerializer implements OfficeConstants,
    DocumentSerializer {

    /**  <code>Format</code> object describing the cell. */
    private Format fmt = null;

    /**  The row number. */
    private int rowID = 1;

    /**  The column number. */
    private int colID = 1;

    /**  The number of times the current row is repeated. */
    private int rowsRepeated = 1;

    /**  The number of times the current column is repeated. */
    private int colsRepeated = 1;

    /**  The number of times the current column is repeated. */
    private StyleCatalog styleCat = null;
    /**
     *  An array of column widths of the current worksheet.  Width is
     *  measured in number of characters.
     */
    private ArrayList<ColumnRowInfo> ColumnRowList;

    /**
     *  A <code>SpreadsheetEncoder</code> object for encoding to
     *  appropriate format.
     */
    protected SpreadsheetEncoder encoder = null;

    /**  <code>SxcDocument</code> object that this converter processes. */
    protected SxcDocument sxcDoc = null;


    /**
     *  Constructor.
     *
     *  @param  document  Input <code>SxcDocument</code>
     *                    <code>Document</code>.
     */
    public SxcDocumentSerializer(Document document) {
        fmt = new Format();
        sxcDoc = (SxcDocument) document;
    }


    /**
     *  <p>Method to convert a DOM <code>Document</code> into
     *  &quot;Device&quot; <code>Document</code> objects.</p>
     *
     *  <p>This method is not thread safe for performance reasons.
     *  This method should not be called from within two threads.
     *  It would be best to call this method only once per object
     *  instance.</p>
     *
     *  @return  <code>ConvertData</code> containing &quot;Device&quot;
     *           <code>Document</code> objects.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public abstract ConvertData serialize() throws ConvertException,
        IOException;


    /**
     *  This method traverses <i>office:settings</i> <code>Element</code>.
     *
     *  @param  node  <i>office:settings</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void traverseSettings(Node node) throws IOException {
        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();
            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_CONFIG_ITEM_SET)) {

                        traverseSettings(child);

                    } else if (nodeName.equals(TAG_CONFIG_ITEM_MAP_INDEXED)) {

                        traverseSettings(child);

                     } else if (nodeName.equals(TAG_CONFIG_ITEM_MAP_ENTRY)) {

                        BookSettings bs = new BookSettings(child);
                        encoder.addSettings(bs);

                    } else {

                        Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }
    }

    /*
     * Handles the loading of defined styles from the style.xml file as well
     * as automatic styles from the content.xml file.
     *
     * Any change to a defined style, such as a short bold section, falls into
     * the latter category.
     */
    protected void loadStyles(SxcDocument sxcDoc) {

        styleCat = new StyleCatalog(25);
        NodeList nl = null;
        String families[] = new String[] {  SxcConstants.COLUMN_STYLE_FAMILY,
                                            SxcConstants.ROW_STYLE_FAMILY,
                                            SxcConstants.TABLE_CELL_STYLE_FAMILY };
        Class<?> classes[]   = new Class[] {   ColumnStyle.class,
                                            RowStyle.class,
                                            CellStyle.class};
        /*
         * Process the content XML for any other style info.
         */
        org.w3c.dom.Document contentDom = sxcDoc.getContentDOM();
        nl = contentDom.getElementsByTagName(TAG_OFFICE_AUTOMATIC_STYLES);
        if (nl.getLength() != 0) {
            styleCat.add(nl.item(0), families, classes, null, false);
        }

        org.w3c.dom.Document stylesDom = sxcDoc.getStyleDOM();
        nl = stylesDom.getElementsByTagName(TAG_OFFICE_STYLES);
        if (nl.getLength() != 0) {
            styleCat.add(nl.item(0), families, classes, null, false);
        }
    }

    /**
     *  This method traverses <i>office:body</i> <code>Element</code>.
     *
     *  @param  node  <i>office:body</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseBody(Node node) throws IOException {

        Debug.log(Debug.TRACE, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        Debug.log(Debug.TRACE, "<DEBUGLOG>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {
                Node searchNode = nodeList.item(i);
                if (searchNode.getNodeType() == Node.ELEMENT_NODE) {

                    String nodeName = searchNode.getNodeName();

                    if (nodeName.equals(TAG_NAMED_EXPRESSIONS)) {

                        traverseNamedExpressions(searchNode);

                    } else {

                        Debug.log(Debug.TRACE, "Skipping " + XmlUtil.getNodeInfo(searchNode) + " />");
                    }
                }
            }

            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_TABLE)) {

                        traverseTable(child);

                    } else {

                        Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        Debug.log(Debug.TRACE, "</DEBUGLOG>");
    }


    /**
     *  This method traverses the <i>table:table</i> element
     *  <code>Node</code>.
     *
     *  @param  node  A <i>table:table</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseNamedExpressions(Node node) throws IOException {

        Debug.log(Debug.TRACE, "<NAMED:EXPRESSIONS>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    NameDefinition nd = new NameDefinition(child);
                    encoder.setNameDefinition(nd);
                }
            }
        }

        Debug.log(Debug.TRACE, "</NAMED:EXPRESSIONS>");
    }

    /**
     *  This method traverses the <i>table:table</i> element
     *  <code>Node</code>.
     *
     *  @param  node  A <i>table:table</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseTable(Node node) throws IOException {

        Debug.log(Debug.TRACE, "<TABLE>");

        ColumnRowList = new ArrayList<ColumnRowInfo>();

        // Get table attributes
        // TODO - extract style from attribute

        NamedNodeMap att = node.getAttributes();

        String tableName =
            att.getNamedItem(ATTRIBUTE_TABLE_NAME).getNodeValue();

        rowID = 1;

        encoder.createWorksheet(tableName);

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_TABLE_ROW)) {
                        // TODO - handle all the possible rows
                        // spelled out in the entities

                        traverseTableRow(child);

                    } else if (nodeName.equals(TAG_TABLE_COLUMN)) {

                        traverseTableColumn(child);

                    } else if (nodeName.equals(TAG_TABLE_SCENARIO)) {

                        // TODO

                    } else {

                        Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        // Add column width info to the current sheet
        encoder.setColumnRows(ColumnRowList);

        Debug.log(Debug.TRACE, "</TABLE>");
    }

    /**
     *  This method traverses the <i>table:table-row</i> element
     *  <code>Node</code>.
     *
     *  @param  node  A <i>table:table-row</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseTableRow(Node node) throws IOException {

        // Get the attributes of the row
        NamedNodeMap cellAtt = node.getAttributes();

        if (cellAtt != null) {

            Node rowStyle =
                cellAtt.getNamedItem(ATTRIBUTE_TABLE_STYLE_NAME);

            Node tableNumRowRepeatingNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);
            int repeatedRows = 1;

            if(tableNumRowRepeatingNode!=null) {
                String repeatStr = tableNumRowRepeatingNode.getNodeValue();
                Debug.log(Debug.TRACE, "traverseTableRow() repeated-rows : " + repeatStr);
                repeatedRows = Integer.parseInt(repeatStr);
            }

            String styleName = new String("");

            if ( rowStyle != null) {
                styleName = rowStyle.getNodeValue();
            }
            if(styleName.equalsIgnoreCase("Default") || styleName.length()==0) {

                Debug.log(Debug.TRACE, "No defined Row Style Attribute was found");

            } else {

                RowStyle rStyle = ( RowStyle)styleCat.lookup(styleName,
                                        SxcConstants.ROW_STYLE_FAMILY, null,
                                        RowStyle.class);

                int rowHeight = rStyle.getRowHeight();

                Debug.log(Debug.TRACE, "traverseTableRow() Row Height : " + rowHeight);
                ColumnRowInfo ri = new ColumnRowInfo(   rowHeight,
                                                            repeatedRows,
                                                            ColumnRowInfo.ROW,
                                                            rowHeight!=0);
                ColumnRowList.add(ri);
            }

            // Get the attribute representing the number of rows repeated
            Node rowsRepeatedNode =
                cellAtt.getNamedItem(ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);

            // There is a number of rows repeated attribute:
            if (rowsRepeatedNode != null) {

                // Get the number of times the row is repeated
                String rowsRepeatedString = rowsRepeatedNode.getNodeValue();

                Integer rowsRepeatedInt = new Integer(rowsRepeatedString);

                rowsRepeated = rowsRepeatedInt.intValue();

            } else {

                // The row is not repeated
                rowsRepeated = 1;
            }
        }

        Debug.log(Debug.TRACE, "<TR>");

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();
            int len = nodeList.getLength();

            for (int i = 0; i < len; i++) {
                Node child = nodeList.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    if (nodeName.equals(TAG_TABLE_CELL)) {

                        traverseCell(child);

                    } else {

                        Debug.log(Debug.TRACE, "<OTHERS " + XmlUtil.getNodeInfo(child) + " />");
                    }
                }
            }
        }

        // Increase the row counter by the number of rows which are repeated
        rowID += rowsRepeated;

        // Re-initialize number of rows repeated before processing the next
        // row data.
        rowsRepeated = 1;

        // When starting a new row, set the column counter back to the
        // first column.
        colID = 1;

        // Re-initialize number of columns repeated before processing
        // the next row data.
        colsRepeated = 1;

        Debug.log(Debug.TRACE, "</TR>");
    }


    /**
     *  This method traverses the <i>table:table-column</i>
     *  <code>Node</code>.  Not yet implemented.
     *
     *  @param  node  A <i>table:table-column</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseTableColumn(Node node) throws IOException {

        Debug.log(Debug.TRACE, "traverseColumn() : ");
        NamedNodeMap cellAtt = node.getAttributes();
        Node tableStyleNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_STYLE_NAME);
        Node tableNumColRepeatingNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);
        Node tableDefaultCellStyle = cellAtt.getNamedItem(ATTRIBUTE_DEFAULT_CELL_STYLE);

        int repeatedColumns = 1;
        int columnWidth = 0;
        ColumnRowInfo col = new ColumnRowInfo(ColumnRowInfo.COLUMN);

        if(tableNumColRepeatingNode!=null) {
            Debug.log(Debug.TRACE, "traverseColumn() repeated-cols : " + tableNumColRepeatingNode.getNodeValue());
            repeatedColumns = Integer.parseInt(tableNumColRepeatingNode.getNodeValue());
            col.setRepeated(repeatedColumns);
        }

        String cellStyleName = new String("");

        if(tableDefaultCellStyle!=null) {
            cellStyleName = tableDefaultCellStyle.getNodeValue();

            Debug.log(Debug.TRACE, "traverseColumn() default-cell-style : " + cellStyleName);
        }

        if(cellStyleName.equalsIgnoreCase("Default") || cellStyleName.length()==0) {

            Debug.log(Debug.TRACE, "No default cell Style Attribute was found");

        } else {

            CellStyle cellStyle = (CellStyle)styleCat.lookup(cellStyleName,
                                SxcConstants.TABLE_CELL_STYLE_FAMILY, null,
                                CellStyle.class);
            Format defaultFmt = new Format(cellStyle.getFormat());
            col.setFormat(defaultFmt);
        }

        String styleName = new String("");

        if(tableStyleNode!=null) {
            styleName = tableStyleNode.getNodeValue();
        }

        if(styleName.equalsIgnoreCase("Default") || styleName.length()==0) {

            Debug.log(Debug.TRACE, "No defined Style Attribute was found");

        } else {

            ColumnStyle cStyle = (ColumnStyle)styleCat.lookup(styleName,
                                SxcConstants.COLUMN_STYLE_FAMILY, null,
                                ColumnStyle.class);

            columnWidth = cStyle.getColWidth();
            col.setSize(columnWidth);
            Debug.log(Debug.TRACE, "traverseColumn() Column Width : " + columnWidth);

        }
        ColumnRowList.add(col);
    }

    /**
     *  This method traverses a <i>table:table-cell</i> element
     *  <code>Node</code>.
     *
     *  @param  node  a <i>table:table-cell</i> <code>Node</code>.
     *
     *  @throws  IOException  if any I/O error occurs.
     */
    protected void traverseCell(Node node) throws IOException {

        NamedNodeMap cellAtt = node.getAttributes();

        int debug_i=0;
        Node debug_attrib = null;
        fmt.clearFormatting();
        if (cellAtt == null || cellAtt.item(0) == null)
        {
           Debug.log(Debug.INFO, "No Cell Attributes\n");
           // return;
        }
        else
        {
           while ((debug_attrib = cellAtt.item(debug_i++)) != null)
           {
              Debug.log(Debug.INFO, "Cell Attribute " + debug_i +
                 ": " + debug_attrib.getNodeName() + " : " +
                 debug_attrib.getNodeValue() + "\n");
           }
        }

        // Get the type of data in the cell
        Node tableValueTypeNode =
            cellAtt.getNamedItem(ATTRIBUTE_TABLE_VALUE_TYPE);

        // Get the number of columns this cell is repeated
        Node colsRepeatedNode =
            cellAtt.getNamedItem(ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);

        // Get the style type
        Node tableStyleNode =
            cellAtt.getNamedItem(ATTRIBUTE_TABLE_STYLE_NAME);

        String styleName = new String("");

        if(tableStyleNode!=null) {
            styleName = tableStyleNode.getNodeValue();
        }

        if(styleName.equalsIgnoreCase("Default")) {

            Debug.log(Debug.TRACE, "No defined Style Attribute was found");

        } else if(styleName.length()!=0) {

            CellStyle cStyle = (CellStyle)styleCat.lookup(styleName,
                                SxcConstants.TABLE_CELL_STYLE_FAMILY, null,
                                CellStyle.class);

            Format definedFormat = cStyle.getFormat();
            fmt = new Format(definedFormat);
        }

        // There is a number of cols repeated attribute
        if (colsRepeatedNode != null) {

            // Get the number of times the cell is repeated
            String colsRepeatedString = colsRepeatedNode.getNodeValue();

            Integer colsRepeatedInt = new Integer(colsRepeatedString);
            colsRepeated = colsRepeatedInt.intValue();
        } else {

            // The cell is not repeated
            colsRepeated = 1;
        }


        // if there is no style we need to check to see if there is a default
        // cell style defined in the table-column's

        if (fmt.isDefault() && styleName.length()==0) {
            int index = 1;
            for(Iterator<ColumnRowInfo> e = ColumnRowList.iterator();e.hasNext();) {
                ColumnRowInfo cri = e.next();
                if(cri.isColumn()) {
                    if(colID>=index && colID<(index+cri.getRepeated())) {
                        fmt = new Format(cri.getFormat());
                    }
                    index += cri.getRepeated();
                }
            }
        }

        if (tableValueTypeNode != null) {

            String cellType =
                tableValueTypeNode.getNodeValue();

            if (cellType.equalsIgnoreCase(CELLTYPE_STRING)) {

                // has text:p tag
                fmt.setCategory(CELLTYPE_STRING);
                Node tableStringValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_STRING_VALUE);
                Debug.log(Debug.TRACE,"Cell Type String :  " + tableStringValueNode);
                if(tableStringValueNode != null) {
                    fmt.setValue(tableStringValueNode.getNodeValue());
                }

            } else if (cellType.equalsIgnoreCase(CELLTYPE_FLOAT)) {

                // has table:value attribute
                // has text:p tag

                // Determine the number of decimal places StarCalc
                // is displaying for this floating point output.
                fmt.setCategory(CELLTYPE_FLOAT);
                fmt.setDecimalPlaces(getDecimalPlaces(node));
                  Node tableValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_VALUE);
                fmt.setValue(tableValueNode.getNodeValue());


            } else if (cellType.equalsIgnoreCase(CELLTYPE_TIME)) {

                // has table:time-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_TIME);
                Node tableTimeNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_TIME_VALUE);
                fmt.setValue(tableTimeNode.getNodeValue());

            } else if (cellType.equalsIgnoreCase(CELLTYPE_DATE)) {

                // has table:date-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_DATE);
                Node tableDateNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_DATE_VALUE);
                fmt.setValue(tableDateNode.getNodeValue());

            } else if (cellType.equalsIgnoreCase(CELLTYPE_CURRENCY)) {

                // has table:currency
                // has table:value attribute
                // has text:p tag

                fmt.setCategory(CELLTYPE_CURRENCY);
                fmt.setDecimalPlaces(getDecimalPlaces(node));
                Node tableValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_VALUE);
                fmt.setValue(tableValueNode.getNodeValue());

            } else if (cellType.equalsIgnoreCase(CELLTYPE_BOOLEAN)) {

                // has table:boolean-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_BOOLEAN);
                Node tableBooleanNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_BOOLEAN_VALUE);
                fmt.setValue(tableBooleanNode.getNodeValue());

            } else if (cellType.equalsIgnoreCase(CELLTYPE_PERCENT)) {

                // has table:value attribute
                // has text:p tag

                fmt.setCategory(CELLTYPE_PERCENT);
                fmt.setDecimalPlaces(getDecimalPlaces(node));
                Node tableValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_VALUE);
                fmt.setValue(tableValueNode.getNodeValue());

            } else {

                Debug.log(Debug.TRACE,"No defined value type" + cellType);
                // Should never get here

            }
        }

        Node tableFormulaNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_FORMULA);

        if(tableFormulaNode != null)
        {
            if(tableValueTypeNode == null) {            // If there is no value-type Node we must assume string-value
                fmt.setCategory(CELLTYPE_STRING);
                Node tableStringValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_STRING_VALUE);
                fmt.setValue(tableStringValueNode.getNodeValue());
            }
            String cellFormula = tableFormulaNode.getNodeValue();
            addCell(cellFormula);
        } else {

            // Text node, Date node, or Time node
            //
            Debug.log(Debug.INFO,
            "TextNode, DateNode, TimeNode or BooleanNode\n");
            // This handles the case where we have style information but no content
            if (node.hasChildNodes()) {
                NodeList childList = node.getChildNodes();
                int len = childList.getLength();

                for (int i = 0; i < len; i++) {
                    Node child = childList.item(i);
                    if (child.getNodeType() == Node.ELEMENT_NODE) {
                        String childName = child.getNodeName();
                        if (childName.equals(TAG_PARAGRAPH)) {
                            traverseParagraph(child);
                        }
                    }
                }
            } else if(!fmt.isDefault()) {
                addCell("");
            }
        }

        // Increase the column counter by the number of times the
        // last cell was repeated.
        colID += colsRepeated;

        // Re-initialize the number of columns repeated before processing
        // the next cell data.
        colsRepeated = 1;

    }


    /**
     *  This method traverses the <i>text:p</i> element <code>Node</code>.
     *
     *  @param  node  A <i>text:p</i> <code>Node</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void traverseParagraph(Node node) throws IOException {

        NamedNodeMap cellAtt = node.getAttributes();

        int debug_i=0;
        Node debug_attrib = null;
        if (cellAtt == null || cellAtt.item(0) == null)
        {
           Debug.log(Debug.INFO, "No Paragraph Attributes\n");
        }
        else
        {
           while ((debug_attrib = cellAtt.item(debug_i++)) != null)
           {
              Debug.log(Debug.INFO, "Paragraph Attribute " + debug_i +
                 ": " + debug_attrib.getNodeName() + " : " +
                 debug_attrib.getNodeValue() + "\n");
           }
        }

        if (node.hasChildNodes()) {

            NodeList nodeList = node.getChildNodes();

            int len = nodeList.getLength();

            StringBuffer buffer = new StringBuffer();

            for (int i = 0; i < len; i++) {

                Node child = nodeList.item(i);

                // TODO: need to handle space/tabs/newline nodes later
                short nodeType = child.getNodeType();

                switch (nodeType) {

                    case Node.TEXT_NODE:
                        buffer.append(child.getNodeValue());
                        break;

                    case Node.ENTITY_REFERENCE_NODE:

                        NodeList nodeList2 = child.getChildNodes();
                        int len2 = nodeList2.getLength();

                        for (int j = 0; j < len2; j++) {
                            Node child2 = nodeList2.item(j);

                            if (child2.getNodeType() == Node.TEXT_NODE) {
                                buffer.append(child2.getNodeValue());
                            }
                        }

                        break;
                }
            }

            String s = buffer.toString();
            addCell(s);

        }
    }


    /**
     *  This method will take the input cell value and add
     *  it to the spreadsheet <code>Document</code> we are currently
     *  encoding.  This method correctly handles cells that are
     *  repeated in either the row, cell, or both directions.
     *
     *  @param  cellValue  The contents of the cell we want to add
     *                     to the spreadsheet <code>Document</code>.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    protected void addCell(String cellValue) throws IOException {

        int col = colID;
        int row = rowID;

        for (int i = 0; i < rowsRepeated; i++) {

            // Log the columns when there are rowsRepeated.
            if (i > 0) {
                Debug.log(Debug.TRACE, "</TR>");
                Debug.log(Debug.TRACE, "<TR>");
            }

            col = colID;

            for (int j = 0; j < colsRepeated; j++) {

                Debug.log(Debug.TRACE, "<TD>");


                // Add the cell data to the encoded spreadsheet document
                encoder.addCell(row, col, fmt, cellValue);

                Debug.log(Debug.TRACE, cellValue);
                Debug.log(Debug.TRACE, "</TD>");

                col++;
            }

            row++;

        }

    }



    /**
     *  This method takes a <i>table:table-cell</i> <code>Node</code>
     *  and traverses down to the <i>text:p</i> tag.  The value is
     *  extracted from the <i>text:p</i> tag and the number of decimal
     *  places is calculated.
     *
     *  @param  node  A <i>table:table-cell</i> <code>Node</code>.
     *
     *  @return  The number of decimal places in the display
     *           string of the data in the input <code>Node</code>.
     */
    protected int getDecimalPlaces(Node node) {

        int decimals = 0;

        Element element = null;

        // cast org.w3c.dom.Node to org.w3c.dom.Element
        if (node instanceof Element) {
            element = (Element) node;
        } else {
            return decimals;
        }

        //  Traverse to the text:p element, there should only be one.
        NodeList list = element.getElementsByTagName(TAG_PARAGRAPH);

        if (list.getLength() != 1) {
            return decimals;
        }

        Node paragraph = list.item(0);

        if (paragraph.hasChildNodes()) {

            NodeList nodeList = paragraph.getChildNodes();

            int len = nodeList.getLength();

            for (int j = 0; j < len; j++) {

                Node child = nodeList.item(j);

                if (child.getNodeType() == Node.TEXT_NODE) {

                    String s = child.getNodeValue();

                    int k = s.lastIndexOf(".");
                    if (k > 0) {
                        s = s.substring(k+1);
                        decimals = s.length();
                    }
                }
            }
        }

        return decimals;
    }

}

