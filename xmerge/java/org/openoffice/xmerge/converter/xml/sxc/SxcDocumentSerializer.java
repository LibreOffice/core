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

import java.awt.Color;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.IntArrayList;
import org.openoffice.xmerge.util.XmlUtil;

import org.openoffice.xmerge.converter.xml.sxc.Format;


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
 * @author      Paul Rank
 * @author      Mark Murnane
 */
public abstract class SxcDocumentSerializer implements OfficeConstants,
    DocumentSerializer {

    /**  The cell foreground <code>Color</code>. */
    private Color foreground = Color.black;

    /**  The cell background <code>Color</code>. */
    private Color background = Color.white;

    /**  The cell format. */
    private long format = 0;

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

    /**
     *  An array of column widths of the current worksheet.  Width is
     *  measured in number of characters.
     */
    private IntArrayList ColumnWidthList;

    /**  Width, in characters, of the current cell display data. */
    private int displayWidth = 0;

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
    protected void traverseTable(Node node) throws IOException {

        Debug.log(Debug.TRACE, "<TABLE>");

        ColumnWidthList = new IntArrayList();

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
        encoder.setColumnWidths(ColumnWidthList);

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

        // TODO
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
        if (cellAtt == null || cellAtt.item(0) == null)
        {
           Debug.log(Debug.INFO, "No Cell Attributes\n");
           fmt.clearFormatting();
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

        if (tableValueTypeNode != null) {

            // Make sure we initialize to 0 the width of the current cell
            displayWidth = 0;

            String cellType =
                tableValueTypeNode.getNodeValue();

            if (cellType.equalsIgnoreCase(CELLTYPE_STRING)) {

                // has text:p tag
                fmt.setCategory(CELLTYPE_STRING);

            } else if (cellType.equalsIgnoreCase(CELLTYPE_FLOAT)) {

                // has table:value attribute
                // has text:p tag

                // Determine the number of decimal places StarCalc
                // is displaying for this floating point output.
                fmt.setCategory(CELLTYPE_FLOAT);
                fmt.setDecimalPlaces(getDecimalPlaces(node));


            } else if (cellType.equalsIgnoreCase(CELLTYPE_TIME)) {

                // has table:time-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_TIME);

            } else if (cellType.equalsIgnoreCase(CELLTYPE_DATE)) {

                // has table:date-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_DATE);

            } else if (cellType.equalsIgnoreCase(CELLTYPE_CURRENCY)) {

                // has table:currency
                // has table:value attribute
                // has text:p tag

                fmt.setCategory(CELLTYPE_CURRENCY);
                fmt.setDecimalPlaces(getDecimalPlaces(node));

            } else if (cellType.equalsIgnoreCase(CELLTYPE_BOOLEAN)) {

                // has table:boolean-value attribute
                // has text:p tag - which is the value we convert

                fmt.setCategory(CELLTYPE_BOOLEAN);

            } else if (cellType.equalsIgnoreCase(CELLTYPE_PERCENT)) {

                // has table:value attribute
                // has text:p tag

                fmt.setCategory(CELLTYPE_PERCENT);
                fmt.setDecimalPlaces(getDecimalPlaces(node));

            } else {

                // Should never get here

            }
        }

        Node tableFormulaNode =
            cellAtt.getNamedItem(ATTRIBUTE_TABLE_FORMULA);

        Node tableValueNode = cellAtt.getNamedItem(ATTRIBUTE_TABLE_VALUE);

        Node tableBooleanNode =
            cellAtt.getNamedItem(ATTRIBUTE_TABLE_BOOLEAN_VALUE);

        if (tableFormulaNode != null) {

            Debug.log(Debug.INFO, "TableFormulaNode\n");
            fmt.setValue(tableValueNode.getNodeValue());
            String cellFormula = tableFormulaNode.getNodeValue();
            addCell(cellFormula);

        } else if (tableValueNode != null) {

            // Float node, currency node, percent node
            //
            Debug.log(Debug.INFO, "TableValueNode\n");
            String cellValue = tableValueNode.getNodeValue();
            addCell(cellValue);

         } else if (tableBooleanNode != null) {

            // Boolean Node
            //
            String cellValue = tableBooleanNode.getNodeValue();

            if (cellValue.equalsIgnoreCase("true"))
            {
               Debug.log(Debug.INFO, "TableBooleanNode - TRUE\n");
               addCell("TRUE");
            }
            else
            {
               Debug.log(Debug.INFO, "TableBooleanNode - FALSE\n");
               addCell("FALSE");
            }

         } else {

            // Text node, Date node, or Time node
            //
            Debug.log(Debug.INFO,
              "TextNode, DateNode, TimeNode or BooleanNode\n");

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
            }
        }

        // Clear out format for current cell after it is written
        format = 0;

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
            displayWidth = calculateContentWidth(s);
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

                // Check the column width to see if it affects the limits
                // of the column
                processColumnWidth(col);

                Debug.log(Debug.TRACE, cellValue);
                Debug.log(Debug.TRACE, "</TD>");

                col++;
            }

            row++;

        }

    }


    /**
     *  This method calculates the width of the input
     *  <code>String</code>.  This value is used in the calculation
     *  of column widths.
     *
     *  @param  content  The contents from which to calculate
     *                   the width.
     */
    protected int calculateContentWidth(String content) {

        // We currently use number of characters as our measure
        // of width, so width is simply String.length().
        return content.length();
    }


    /**
     *  This method maintains an array of integers that
     *  contains the desired widths (in number of chars)
     *  for each column.
     *
     *  @param  column  The current column number.
     */
    protected void processColumnWidth(int column) {

        // Make sure the list has the current column
        ColumnWidthList.ensureCapacityAndFill(column, 0);

        int index = column - 1;

        int currentWidth = ColumnWidthList.get(index);

        // displayWidth is the size (in characters) of the expected
        // content of the cell.
        if (displayWidth > currentWidth) {
            ColumnWidthList.set(index, displayWidth);
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

                    displayWidth = calculateContentWidth(s);

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

