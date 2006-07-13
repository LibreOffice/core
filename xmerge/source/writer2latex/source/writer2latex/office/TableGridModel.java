/************************************************************************
 *
 *  TableGridModel.java
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
 *  Copyright: 2002 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3f (2004-09-30)
 *
 */

package writer2latex.office;

import java.util.LinkedList;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import writer2latex.util.Misc;

/**
 * <p> This class represents a table in an OOo XML document - presented
 * as an n by m grid.</p>
 */
public class TableGridModel {
    private Node tableNode;
    private LinkedList cols = new LinkedList();
    private LinkedList rows = new LinkedList();
    private LinkedList cells = new LinkedList();
    private int nMaxCols = 0; // real number of columns (count to last non-empty)

    /**
     * <p> Reads the table from a table:table node </p>
     */
    public TableGridModel(Node node) {
        tableNode = node;
        if (!node.hasChildNodes()) { return; } // empty table!
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String sName = child.getNodeName();
                if (sName.equals(XMLString.TABLE_TABLE_COLUMN)) {
                    readTableColumn(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_COLUMNS)) {
                    readTableColumns(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_COLUMN_GROUP)) {
                    readTableColumnGroup(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_HEADER_COLUMNS)) {
                    readTableHeaderColumns(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROW)) {
                    readTableRow(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROWS)) {
                    readTableRows(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROW_GROUP)) {
                    readTableRowGroup(child,false,false);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_HEADER_ROWS)) {
                    readTableHeaderRows(child,false,false);
                }
            }
        }

        // Now determine the actual number of columns
        int nRows = cells.size();
        for (int nRow=0; nRow<nRows; nRow++) {
            LinkedList row = (LinkedList) cells.get(nRow);
            int nCols = row.size();
            int nMaxCol = 0;
            for (int nCol=0; nCol<nCols; nCol++) {
                Element cell = (Element) row.get(nCol);
                if (cell.hasChildNodes()) {
                    nMaxCol = nCol + Misc.getPosInteger(cell.getAttribute(
                            XMLString.TABLE_NUMBER_COLUMNS_SPANNED),1);
                    if (nMaxCol>nMaxCols) { nMaxCols = nMaxCol; }
                }
            }
        }

    }

    private void readTableColumn(Node node, boolean bHeader, boolean bDisplay) {
        int nRepeat = Misc.getPosInteger(Misc.getAttribute(node,
                                         XMLString.TABLE_NUMBER_COLUMNS_REPEATED),1);
        while (nRepeat-->0) {
            cols.add(new TableLine(node,bHeader,bDisplay));
        }
    }

    private void readTableColumns(Node node, boolean bHeader, boolean bDisplay) {
        if (!node.hasChildNodes()) { return; } // no columns here!
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String sName = child.getNodeName();
                if (sName.equals(XMLString.TABLE_TABLE_COLUMN)) {
                    readTableColumn(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_COLUMN_GROUP)) {
                    readTableColumnGroup(child,bHeader,bDisplay);
                }
            }
        }
    }

    private void readTableColumnGroup(Node node, boolean bHeader, boolean bDisplay) {
        bDisplay = !"false".equals(Misc.getAttribute(node,XMLString.TABLE_DISPLAY));
        if (!node.hasChildNodes()) { return; } // no columns here!
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String sName = child.getNodeName();
                if (sName.equals(XMLString.TABLE_TABLE_HEADER_COLUMNS)) {
                    readTableHeaderColumns(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_COLUMN)) {
                    readTableColumn(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_COLUMN_GROUP)) {
                    readTableColumnGroup(child,bHeader,bDisplay);
                }
            }
        }
    }

    private void readTableHeaderColumns(Node node, boolean bHeader, boolean bDisplay) {
        readTableColumns(node,true,bDisplay);
    }

    private void readTableRow(Node node, boolean bHeader, boolean bDisplay) {
        int nRepeat = Misc.getPosInteger(Misc.getAttribute(node,
                                         XMLString.TABLE_NUMBER_ROWS_REPEATED),1);
        while (nRepeat-->0) {
            rows.add(new TableLine(node,bHeader,bDisplay));
        }
        // Read the cells in the row
        LinkedList row = new LinkedList();
        if (node.hasChildNodes()) {
            NodeList nl = node.getChildNodes();
            int nLen = nl.getLength();
            for (int i = 0; i < nLen; i++) {
                Node child = nl.item(i);
                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    Element cell = (Element) child;
                    String sName = cell.getTagName();
                    if (sName.equals(XMLString.TABLE_TABLE_CELL)) {
                        nRepeat = Misc.getPosInteger(cell.getAttribute(
                                         XMLString.TABLE_NUMBER_COLUMNS_REPEATED),1);
                        while (nRepeat-->0) { row.add(cell); }
                    }
                    else if (sName.equals(XMLString.TABLE_COVERED_TABLE_CELL)) {
                        nRepeat = Misc.getPosInteger(cell.getAttribute(
                                         XMLString.TABLE_NUMBER_COLUMNS_REPEATED),1);
                        while (nRepeat-->0) { row.add(cell); }
                    }
                }
            }
        }
        cells.add(row);
    }

    private void readTableRows(Node node, boolean bHeader, boolean bDisplay) {
        if (!node.hasChildNodes()) { return; } // no rows here!
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String sName = child.getNodeName();
                if (sName.equals(XMLString.TABLE_TABLE_ROW)) {
                    readTableRow(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROW_GROUP)) {
                    readTableRowGroup(child,bHeader,bDisplay);
                }
            }
        }
    }

    private void readTableRowGroup(Node node, boolean bHeader, boolean bDisplay) {
        bDisplay = !"false".equals(Misc.getAttribute(node,XMLString.TABLE_DISPLAY));
        if (!node.hasChildNodes()) { return; } // no rows here!
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType() == Node.ELEMENT_NODE) {
                String sName = child.getNodeName();
                if (sName.equals(XMLString.TABLE_TABLE_HEADER_ROWS)) {
                    readTableHeaderRows(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROW)) {
                    readTableRow(child,bHeader,bDisplay);
                }
                else if (sName.equals(XMLString.TABLE_TABLE_ROW_GROUP)) {
                    readTableRowGroup(child,bHeader,bDisplay);
                }
            }
        }
    }

    private void readTableHeaderRows(Node node, boolean bHeader, boolean bDisplay) {
        readTableRows(node,true,bDisplay);
    }

    public int getColCount() { return cols.size(); }

    public int getMaxColCount() { return nMaxCols; }

    public int getRowCount() { return rows.size(); }

    public Node getTableNode() { return tableNode; }

    public Node getCell(int nRow, int nCol) {
        if (nRow<0 || nRow>=cells.size()) { return null; }
        LinkedList row = (LinkedList) cells.get(nRow);
        if (nCol<0 || nCol>=row.size()) { return null; }
        return (Node) row.get(nCol);
    }

    public TableLine getRow(int nRow) {
        if (nRow<0 || nRow>=rows.size()) { return null; }
        return (TableLine) rows.get(nRow);
    }

    public TableLine getCol(int nCol) {
        if (nCol<0 || nCol>=cols.size()) { return null; }
        return (TableLine) cols.get(nCol);
    }

}
