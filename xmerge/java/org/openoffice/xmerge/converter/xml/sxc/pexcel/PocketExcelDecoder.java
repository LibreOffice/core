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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Stack;
import java.util.LinkedList;
import java.util.Vector;
import java.util.Enumeration;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentDeserializer;
import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetDecoder;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.Formula;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.LabelCell;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.CellValue;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.FloatNumber;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocumentDeserializerImpl}
 *  SxcDocumentDeserializerImpl} to decode the Pocket Excel format.
 *
 *  @author   Paul Rank
 */
final class PocketExcelDecoder extends SpreadsheetDecoder {

    private PxlDocument pxlDoc;
    private LinkedList ws = new LinkedList();
    private CellValue cell;
    private int maxRows = 0;
    private int maxCols = 0;
    private int wsIndex;
    private int cellValue=0;
    private Format fmt = null;

    /**
     *  Constructor creates a Pocket Excel WorkBook.
     *
     *  @param  name            The name of the WorkBook.
     *  @param  worksheetNames  set of Strings equivalent to the worksheets
     *                          contained in the workbook
     *  @param  password        The password for the workBook.
     *
     *  @throws  IOException    If any I/O error occurs.
     */
    PocketExcelDecoder(String name, String[] worksheetNames, String password) throws IOException {
        super(name, password);

        fmt = new Format();
    }


    /**
     *  This method takes a <code>ConvertData</code> as input and
     *  converts it into a PocketWord WorkSheet.  The WorkSheet is then
     *  added to the WorkBook.
     *
     *  @param  InputStream An <code>ConvertData</code> containing a
     *                      Pocket Excel WorkSheet.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addDeviceContent(ConvertData cd) throws IOException {

        Enumeration e = cd.getDocumentEnumeration();
        pxlDoc = (PxlDocument) e.nextElement();
    }


    /**
     *  This method returns the number of spreadsheets
     *  stored in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public int getNumberOfSheets() {
        Vector v = pxlDoc.getWorksheetNames();
        Debug.log(Debug.TRACE,"Total Number of Sheets : " + v.size());
        return (v.size());
    }


    /**
     *  This method gets the requested WorkSheet from the
     *  WorkBook and sets it as the selected WorkSheet.  All
     *  other "get" methods will now get data from this WorkSheet.
     *
     *  @param  sheetIndex  The index number of the sheet to open.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void setWorksheet(int sheetIndex) throws IOException {
        Debug.log(Debug.TRACE,"Setting to worksheet : " + sheetIndex);
        ws =  pxlDoc.getWorksheet(sheetIndex);
        wsIndex = sheetIndex;
        while(goToNextCell()) {
            maxRows = Math.max(maxRows, cell.getRow());
            maxCols = Math.max(maxCols, cell.getCol());
        }
        cellValue = 0;
        Debug.log(Debug.TRACE,"Max Cols : " + maxCols + " MaxRows : " + maxRows);
    }


    /**
     *  This method returns the name of the current spreadsheet.
     *
     *  @return  The name of the current WorkSheet.
     */
    public String getSheetName() {
        Vector v = pxlDoc.getWorksheetNames();
        String wsName = (String) (v.elementAt(wsIndex));
        Debug.log(Debug.TRACE,"The name of the current Worksheet is : " + wsName);
        return wsName;
    }


    /**
     *  This method gets the next cell from the WorkSheet
     *  and sets it as the selected cell.  All other "get"
     *  methods will now get data from this cell.
     *
     *  @return  True if we were able to go to another cell
     *           in the sheet, false if there were no cells
     *           left.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public boolean goToNextCell() throws IOException {

        boolean success = false;

        if (cellValue < ws.size()){
            cell = (CellValue) ws.get(cellValue);
            cellValue++;
            readCellFormat();
            // Debug.log(Debug.TRACE,"Current Cell : " + cell.getString());
            return true;
        }
        else {
            Debug.log(Debug.TRACE,"Could't find current cell");
        }

        return success;
    }


    /**
     *  This method returns the row number of the current cell.
     *
     *  @return  The row number of the current cell.  Returns
     *            -1 if no cell is currently selected.
     */
    public int getRowNumber() {

        int row = -1;

        if (cell != null) {
            row = cell.getRow();
            Debug.log(Debug.TRACE,"cell row is " + row);
        }
        return (row);
    }

    /**
     *  This method returns the number of rows in the current sheet.
     *
     *  @return  The number of rows in the current sheet.
     */
    public int getNumberOfRows() {
        return maxRows;
    }

    /**
     *  This method returns the number of columns in the current sheet.
     *
     *  @return  The number of columns in the current sheet.
     */
    public int getNumberOfColumns() {
        return maxCols;
    }


    /**
     *  This method returns the col number of the current cell.
     *
     *  @return  The col number of the current cell.  Returns
     *           -1 if no cell is currently selected.
     */
    public int getColNumber() {

        int col = -1;

        if (cell != null) {
            col = cell.getCol();
            Debug.log(Debug.TRACE,"cell col is " + col);
        }
        return (col);
    }

    /**
     * Parses a Pexcel Formula in Reverse Polish Notation into an sxc formula
     * that uses infix notation
     *
     *  @return the parsed formula using infix notation
     */
    protected String parseFormula(String inputString) {

        Debug.log(Debug.TRACE,"parseFormula : " + inputString);

        Stack myStack = new Stack();
        String nextChar;
        String topOfStack;
        String outputString = "=";
        String op1,op2;

        for (int i = 1;i<inputString.length();i++) {

            char ch = inputString.charAt(i);        // Check to see if this is a cell or an operator
            if(((ch>='a') && (ch<='z')) || (ch>='A') && (ch<='Z'))  {
                int interval = 1;
                char nextRef = inputString.charAt(i+interval);
                if(((nextRef>='a') && (nextRef<='z')) || (nextRef>='A') && (nextRef<='Z')) { // if cell col > 26
                    interval++;
                    nextRef = inputString.charAt(i+interval);
                }
                while(nextRef>='0' && nextRef<='9') {
                // Keep reading until we reach another operator or cell reference
                    interval++;
                    if((i+interval)<inputString.length())
                        nextRef = inputString.charAt(i+interval);
                    else
                        nextRef = 0;
                }
                nextChar = inputString.substring(i,i+interval); // if cell then read all of the cell reference
                i += interval-1;
            } else
                nextChar = inputString.substring(i,i+1);

            if (    nextChar.equals("+") ||
                    nextChar.equals("-") ||
                    nextChar.equals("*") ||
                    nextChar.equals("/") ||
                    nextChar.equals("^")) {

                op2 = (String)myStack.pop();
                if(!myStack.empty()) {
                    op1 = (String)myStack.pop();

                    outputString += op1;
                    outputString += nextChar;
                    outputString += op2;
                } else {
                    outputString += nextChar;
                    outputString += op2;
                }
            } else {
                myStack.push(nextChar);
            }
        }
        while(!myStack.empty()) {
            topOfStack = (String)myStack.pop();
            outputString += topOfStack;
        }
        return outputString;
    }

    /**
     *  This method returns the contents of the current cell.
     *
     *  @return  The contents of the current cell.  Returns
     *           null if no cell is currently selected.
     */
    public String getCellContents() {

        String contents = new String("");

        if (cell != null) {
            try {
            contents = cell.getString();
            }
            catch (IOException e) {
                System.err.println("Could Not retrieve Cell contents");
                System.err.println("Setting contents of cell(" + cell.getRow()
                                    + "," + cell.getCol() + ") to an empty string");
                System.err.println("Error msg: " + e.getMessage());
            }
        }
        if (contents.startsWith("=")) {
            contents = parseFormula(contents);
        }
        return contents;
    }

    /**
     *  This method returns the contents of the current cell.
     *
     *  @return  The contents of the current cell.  Returns
     *           null if no cell is currently selected.
     */
    public String getCellValue() {

        String contents = new String("");

        if (cell != null) {
            try {
            contents = ((Formula)cell).getValue();
            }
            catch (IOException e) {
                System.err.println("Could Not retrieve Cell value");
                System.err.println("Setting value of cell(" + cell.getRow()
                                    + "," + cell.getCol() + ") to an empty string");
                System.err.println("Error msg: " + e.getMessage());
            }
        }
        return contents;
    }

    /**
     *  <p>This method returns the type of the data in the current cell.
     *  Currently the only type supported is String.</p>
     *
     *  @return  The type of the data in the current cell.
     */
    public String getCellDataType() {

        String type = getCellFormatType();

        return type;
    }


    /**
     *  This method returns the format of the data in the current cell.
     *  Currently it only supports strings.
     *
     *  @return  The format of the data in the current cell.
     */
    String getCellFormatType() {

        String type = OfficeConstants.CELLTYPE_STRING;

        if(cell instanceof FloatNumber)
            type = OfficeConstants.CELLTYPE_FLOAT;
        if(cell instanceof Formula)
            type = OfficeConstants.CELLTYPE_FLOAT;

        return type;
    }


    /**
     *  Return the Format object describing the active cell formatting.
     *
     *  @return The Format object describing the active cell formatting.
     */
    public Format getCellFormat() {
        return new Format(fmt);
    }


    /**
     *  Create the format data for the new cell.
     */
    private void readCellFormat() {

        fmt.clearFormatting();

        fmt.setCategory(getCellFormatType());
    }
}

