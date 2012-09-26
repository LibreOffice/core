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

package org.openoffice.xmerge.converter.xml.sxc.minicalc;

import jmc.Workbook;
import jmc.Worksheet;
import jmc.CellAttributes;
import jmc.CellDescriptor;
import jmc.JMCconstants;
import jmc.JMCException;

import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentDeserializer;
import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetDecoder;
import org.openoffice.xmerge.converter.xml.sxc.Format;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocumentDeserializerImpl}
 *  SxcDocumentDeserializerImpl} to decode the MiniCalc format.
 *
 */
final class MinicalcDecoder extends SpreadsheetDecoder {

    /** MiniCalc WorkBook to store sheets. */
    private Workbook wb;

    /** MiniCalc sheet - only one sheet can be open at a time. */
    private Worksheet ws;

    /** The current cell - only one cell can be active at a time. */
    private CellDescriptor cell = null;

    /** Format object describing the current cell. */
    private Format fmt = null;

    /** The password for the WorkBook. */
    private String password = null;

    /** The number of rows in the current WorkSheet. */
    private int maxRows = 0;

    /** The number of columns in the current WorkSheet. */
    private int maxCols = 0;

    /** The names of the worksheets. */
    private String[] worksheetNames = null;

    /**
     *  Constructor creates a MiniCalc WorkBook.
     *
     *  @param  name      The name of the WorkBook.
     *  @param  password  The password for the workBook.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    MinicalcDecoder(String name, String[] worksheetNames, String password) throws IOException {

        super(name, password);

        fmt = new Format();

        this.password = password;
        this.worksheetNames = worksheetNames;

        try {

            wb = new Workbook(name, password);

        }
        catch (JMCException e) {

            Debug.log(Debug.ERROR, "MinicalcDecoder.constructor:" + e.getMessage());

            throw new IOException(e.getMessage());
        }
    }


    /**
     *  This method takes a <code>ConvertData</code> as input and
     *  converts it into a MiniCalc WorkSheet.  The WorkSheet is then
     *  added to the WorkBook.
     *
     *  @param  InputStream An <code>ConvertData</code> containing a
     *                      MiniCalc WorkSheet.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addDeviceContent(ConvertData cd) throws IOException {

        try {
            PalmDocument palmDoc;
            int j = 0;

        Enumeration e = cd.getDocumentEnumeration();
        while(e.hasMoreElements()) {

            palmDoc = (PalmDocument) e.nextElement();
                // Convert PDB to WorkBook/WorkSheet format
            PalmDB pdb = palmDoc.getPdb();

                // This will be done at least once
                String sheetName = worksheetNames[j];

                // Get number of records in the pdb
                int numRecords = pdb.getRecordCount();

                // sheetName does not contain the WorkBook name, but we need the
                // full name.
                String fullSheetName = new String(wb.getWorkbookName() + "-" + sheetName);

                // Create a new (empty) WorkSheet
                ws = new Worksheet();

                // Initialize the WorkSheet
                ws.initWorksheet(fullSheetName, numRecords);

                // Loop over the number of records in the PDB
                for (int i = 0; i < numRecords; i++) {

                    // Read record i from the PDB
                    Record rec = pdb.getRecord(i);

                      byte cBytes[] = rec.getBytes();

                    // Create an InputStream
                    ByteArrayInputStream bis = new ByteArrayInputStream(cBytes);

                    // Get the size of the stream
                    int bisSize = cBytes.length;

                    // Add each record to the WorkSheet
                    ws.readNextRecord(bis, bisSize);
                }


                // Add the WorkSheet to the WorkBook
                wb.addWorksheet(ws);
                    j++;
            }
        }
        catch (JMCException e) {

            Debug.log(Debug.ERROR, "MinicalcDecoder.addPDB:" + e.getMessage());

            throw new IOException(e.getMessage());
        }
    }


    /**
     *  This method returns the number of spreadsheets
     *  stored in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public int getNumberOfSheets() {

        return wb.getNumberOfSheets();
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

        try {

            ws = wb.getWorksheet(sheetIndex);

            // Initialize access to the WorkSheet so that we can calculate
            // the number of rows and columns
            ws.initAccess(password);

            maxRows = 0;
            maxCols = 0;

            while (goToNextCell()) {
                maxRows = Math.max(maxRows, cell.getRowNumber());
                maxCols = Math.max(maxCols, cell.getColNumber());
            }

            // Re-initialize access to the WorkSheet
            ws.initAccess(password);

        }
        catch (JMCException e) {

            Debug.log(Debug.ERROR, "MinicalcDecoder.setWorksheet:" + e.getMessage());

            throw new IOException(e.getMessage());
        }
    }


    /**
     *  This method returns the name of the current spreadsheet.
     *
     *  @return  The name of the current WorkSheet.
     */
    public String getSheetName() {

        String sheetName = ws.getName();

        return sheetName;
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

        boolean gotCell = false;

        try {
            cell = ws.getNextCell();

            if (cell != null) {
                gotCell = true;
            }

            // As we read each cell, get its formatting info
            readCellFormat();
        }
        catch (JMCException e) {

            Debug.log(Debug.ERROR, "MinicalcDecoder.goToNextCell:" + e.getMessage());

            throw new IOException(e.getMessage());
        }

        return gotCell;
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

            row = cell.getRowNumber();
        }

        return row;
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

            col = cell.getColNumber();
        }

        return col;
    }


    /**
     *  This method returns the contents of the current cell.
     *
     *  @return  The contents of the current cell.  Returns
     *           null if no cell is currently selected.
     */
    public String getCellContents() {

        String contents = null;

        if (cell != null) {
            contents = cell.getCellContents();

            // Active cell, but no content
            if (contents == null)
                return new String("");

              // Does the cell contain a formula?
            if (contents.startsWith("=")) {
                contents = parseFormula(contents);
            }
            // Make sure that any MiniCalc peculiarities are stripped off
            if (fmt.getCategory().equalsIgnoreCase(OfficeConstants.CELLTYPE_CURRENCY)) {
                contents = currencyRemoveSign(contents);
            }
            else if (fmt.getCategory().equalsIgnoreCase(OfficeConstants.CELLTYPE_PERCENT)) {
                contents = percentRemoveSign(contents);
            }
            else if (fmt.getCategory().equalsIgnoreCase(OfficeConstants.CELLTYPE_DATE)) {
                contents = convertToStarDate(contents);
            }
            else if (fmt.getCategory().equalsIgnoreCase(OfficeConstants.CELLTYPE_TIME)) {
                contents = convertToStarTime(contents);
            }
        }

        return contents;
    }

    /**
     *  This method is meant to return the value of the formula cell. However
     *  in minicalc this value is not used so hence the stubbed function
     *
     *  @return the value fo the formula cell
     */
    public String getCellValue() {
        return null;
    }

    /**
     *  <p>This method takes a formula and parses it into
     *  StarOffice XML formula format.</p>
     *
     *  <p>Many spreadsheets use ',' as a separator.
     *  StarOffice XML format uses ';' as a separator instead.</p>
     *
     *  <p>Many spreadsheets use '!' as a separator when refencing
     *  a cell in a different sheet.</p>
     *
     *  <blockquote>
     *  Example: =sheet1!A1
     *  </blockquote>
     *
     *  <p>StarOffice XML format uses '.' as a separator instead.</p>
     *
     *  <blockquote>
     *  Example: =sheet1.A1
     *  </blockquote>
     *
     *  @param  formula  A formula string.
     *
     *  @return  A StarOffice XML format formula string.
     */
    protected String parseFormula(String formula) {

        formula = formula.replace(',', ';');
        formula = formula.replace('!', '.');

        return formula;
    }

    /**
     *  <p>This method returns the type of the data in the current cell.</p>
     *
     *  <p>Possible Data Types:</p>
     *
     *  <ul><li>
     *  Percent -  MiniCalc can store as a number or as a string.
     *
     *             When stored as a string, the % sign is stored in the
     *             string . The MiniCalc format is "string".
     *             Example 10.1% is stored as the string "10.1%"
     *
     *             When stored as a number, the decimal representation
     *             is stored.  The MiniCalc format is "percentage".
     *             Example: 10.1% is stored as "0.101"
     *  </li><li>
     *  Currency - MiniCalc stores currency as a number with the format
     *             set to "currency".
     *             A user can also enter a value with a dollar sign
     *             (example $18.56) into MiniCalc and not set the format
     *             as currency.  We treat this type of data as a
     *             currency data type.
     *  </li><li>
     *  Boolean - MiniCalc stores in a string as "true" or "false"
     *  </li><li>
     *
     *  Date - MiniCalc stores a date in a string as either
     *         MM/DD/YY or MM/DD/YYYY.  Any variation from the above
     *         format will not be considered a date.
     *  </li><li>
     *  Time - MiniCalc stores a time in a string as hh:mm:ss.  Any
     *         variation from this format will not be considered a time.
     *  </li><li>
     *  Float - MiniCalc stores as a number and it is not percent
     *          or currency.
     *  </li><li>
     *  String - MiniCalc stores as a string (surprise).  Doesn't parse
     *           to any of the other data types.
     *  </li><li>
     *  @return  The type of the data in the current cell.
     *  </li></ul>
     */
    public String getCellDataType() {

        boolean isNumber = false;

        // Get format value set on the cell in MiniCalc
        String format = getCellFormatType();

        // Initialize the data type to the format
        String type = format;

        String contents = getCellContents();

        if (contents != null) {

            MinicalcDataString data = new MinicalcDataString(contents);

            // Check if it is a formula
            if (data.isFormula()) {
                Debug.log(Debug.INFO, "   " + contents + " Is a Function   Format = "
                    + format + "\n");
                return type;
            }

            try {
                // Check to see if it is a number
                Double d = Double.valueOf(contents);
                isNumber = true;
                Debug.log(Debug.INFO, "   " + contents + " Is a Number   Format = " + format);

            } catch (NumberFormatException e) {
                Debug.log(Debug.INFO, "    " + contents + " Not a Number   Format= " + format);
                // no, it is not a number
                isNumber = false;
            }


            if (isNumber) {

                // Numbers are Float, Currency, and Percent
                if (format.equals(OfficeConstants.CELLTYPE_CURRENCY)) {

                    type = OfficeConstants.CELLTYPE_CURRENCY;

                } else if (format.equals(OfficeConstants.CELLTYPE_PERCENT)) {

                    type = OfficeConstants.CELLTYPE_PERCENT;

                } else {

                    type = OfficeConstants.CELLTYPE_FLOAT;
                }

            } else if (data.isBoolean()) {

                // Data is a Boolean type
                type = OfficeConstants.CELLTYPE_BOOLEAN;

            } else if (data.isDate()) {

                // Data is a Date type
                type = OfficeConstants.CELLTYPE_DATE;

            } else if (data.isTime()) {

                // Data is a Time type
                type = OfficeConstants.CELLTYPE_TIME;

            } else if (data.isPercent()) {

                // Data is percent
                type = OfficeConstants.CELLTYPE_PERCENT;

            } else if (data.isCurrency()) {

                // Data is a Currency type
                type = OfficeConstants.CELLTYPE_CURRENCY;

             } else {

                // Data can't be float, since it isn't a number

                // We've already tried parsing it as all other data
                // types, the only remaining option is a string
                type = OfficeConstants.CELLTYPE_STRING;
             }
        }

        Debug.log(Debug.INFO, " TYPE = " + type + "\n");

        return type;
    }


    /**
     *  This method returns the format of the data in the current cell.
     *
     *  @return  The format of the data in the current cell.
     */
    String getCellFormatType() {

        // Set type to default data type
        String type = OfficeConstants.CELLTYPE_FLOAT;

        if (cell != null) {

            // Get the attributes for the current cell
            CellAttributes att = cell.getCellAttributes();

            if (att != null) {

                // Extract the format info from the attributes
                long format = att.getFormat();

                // The cell type is stored in bits 5-8
                long cellType = format &  0x000000F0L;

                // The number of decimal places is stored in bits 1-4
                long decimals = format &  0x0000000FL;

                if (cellType == JMCconstants.FF_FORMAT_GENERIC) {

                    // MiniCalc stores both Strings and Booleans
                    // in the generic type.  We must check the contents
                    // to differentiate between the two.

                    // Get cell's contents
                    String contents = getCellContents();

                    if (contents.equalsIgnoreCase("false") ||
                        contents.equalsIgnoreCase("true")) {

                        type = OfficeConstants.CELLTYPE_BOOLEAN;


                    } else {

                    type = OfficeConstants.CELLTYPE_STRING;

                    }

                } else if (cellType == JMCconstants.FF_FORMAT_DECIMAL) {

                    type = OfficeConstants.CELLTYPE_FLOAT;

                } else if (cellType == JMCconstants.FF_FORMAT_TIME) {

                    type = OfficeConstants.CELLTYPE_TIME;

                } else if (cellType == JMCconstants.FF_FORMAT_DATE) {

                    type = OfficeConstants.CELLTYPE_DATE;

                } else if (cellType == JMCconstants.FF_FORMAT_CURRENCY) {

                    type = OfficeConstants.CELLTYPE_CURRENCY;

                } else if (cellType == JMCconstants.FF_FORMAT_PERCENT) {

                    type = OfficeConstants.CELLTYPE_PERCENT;
                }

            }
        }

        return type;
    }


    /**
     *  This method takes a <code>String</code> that contains a
     *  currency value and removes the $ from the <code>String</code>.
     *  If the dollar sign is not the first or last character of the
     *  input <code>String</code>, the input <code>String</code> is
     *  simply returned.
     *
     *  @param  contents  The input <code>String</code> from which to
     *                    remove the dollar sign.
     *
     *  @return  The input <code>String</code> minus the dollar sign.
     *           If the input <code>String</code> did not begin or end
     *           with a dollar sign, contents is returned.
     */
    private String currencyRemoveSign(String contents) {
        MinicalcDataString mcString = new MinicalcDataString(contents);
        String currencyString = mcString.currencyRemoveSign();
        return currencyString;
    }


    /**
     *  This method takes a <code>String</code> that contains a percent
     *  value and removes the % from the <code>String</code>.  If the
     *  percent sign is not the last character of the input
     *  <code>String</code>, the input <code>String</code> is simply
     *  returned.
     *
     *  @param  contents  The input String from which to remove the
     *                    percent sign.
     *
     *  @return  The input <code>String</code> minus the percent sign.
     *           If the input <code>String</code> did not begin with
     *           a percent sign, contents is returned.
     */
    private String percentRemoveSign(String contents) {
        MinicalcDataString mcString = new MinicalcDataString(contents);
        String percentString = mcString.percentRemoveSign();
        return percentString;
    }


    /**
     *  This method returns takes a <code>String</code> that contains
     *  a time value and converts it from MiniCalc format to StarOffice
     *  XML time format.
     *
     *  @param   contents  The input <code>String</code> containing a
     *                     MiniCalc time.
     *
     *  @return  The input <code>String</code> converted to StarOffice
     *           XML time format.
     */
    private String convertToStarTime(String contents) {
        MinicalcDataString mcString = new MinicalcDataString(contents);
        String timeString = mcString.convertToStarTime();
        return timeString;
    }

    /**
     *  This method returns takes a <code>String</code> that contains
     *  a date value and converts it from MiniCalc format to StarOffice
     *  XML date format.
     *
     *  @param   contents  The input <code>String</code> containing a
     *                     MiniCalc date.
     *
     *  @return  The input <code>String</code> converted to StarOffice
     *           XML date format.
     */
    private String convertToStarDate(String contents) {
        MinicalcDataString mcString = new MinicalcDataString(contents);
        String dateString = mcString.convertToStarDate();
        return dateString;
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
        // Make sure there are no remnants from the last time
        fmt.clearFormatting();

        fmt.setCategory(getCellFormatType());

        // TODO - Get any more formatting data here
    }
}

