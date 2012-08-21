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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.NoSuchElementException;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxc.BookSettings;
import org.openoffice.xmerge.converter.xml.sxc.ColumnRowInfo;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.NameDefinition;
import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetDecoder;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.BIFFRecord;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.CellValue;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.ColInfo;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.DefinedName;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.ExtendedFormat;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.FloatNumber;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.FontDescription;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Formula;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Row;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Worksheet;
import org.openoffice.xmerge.util.Debug;

/**
 *  This class is used by {@link SxcDocumentDeserializerImpl}
 *  SxcDocumentDeserializerImpl} to decode the Pocket Excel format.
 *
 *  @author   Paul Rank
 */
final class PocketExcelDecoder extends SpreadsheetDecoder {

    private Workbook wb;
    private Worksheet ws;
    private CellValue cell;
    private int maxRows = 0;
    private int maxCols = 0;
    private int wsIndex;
    private Iterator<BIFFRecord> cellValue;
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
     *  @param  cd An <code>ConvertData</code> containing a
     *                      Pocket Excel WorkSheet.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addDeviceContent(ConvertData cd) throws IOException {

        Iterator<Object> e = cd.getDocumentEnumeration();
        wb = (Workbook) e.next();
    }


    /**
     *  This method returns the number of spreadsheets
     *  stored in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public int getNumberOfSheets() {

        ArrayList<Object> v = wb.getWorksheetNames();
        Debug.log(Debug.TRACE,"Total Number of Sheets : " + v.size());
        return (v.size());
    }

    /**
     *  This method returns the number of spreadsheets
     *  stored in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public Iterator<NameDefinition> getNameDefinitions() {

        Iterator<DefinedName> e  = wb.getDefinedNames();
        ArrayList<NameDefinition> nameDefinitionVector = new ArrayList<NameDefinition>();
        while(e.hasNext()) {
            DefinedName dn = e.next();
            NameDefinition nameDefinitionEntry = dn.getNameDefinition();
            nameDefinitionVector.add(nameDefinitionEntry);
        }
        Debug.log(Debug.TRACE,"Getting " + nameDefinitionVector.size() + " DefinedName records");
        return (nameDefinitionVector.iterator());
    }

    /**
     *  This method returns an enumeration of Settings object(s),
     *  one for each worksheet
     *
     *  @return An enumerattion of <code>Settings</code>
     */
    public BookSettings getSettings() {

        return (wb.getSettings());
    }
    /**
     *  This method returns the number of spreadsheets
     *  stored in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public Iterator<ColumnRowInfo> getColumnRowInfos() {

        ArrayList<ColumnRowInfo> colRowVector = new ArrayList<ColumnRowInfo>();

        // Collect Columns from worksheet and add them to the vector
        for(Iterator<ColInfo> e  = ws.getColInfos();e.hasNext();) {
            ColInfo ci = e.next();
            int repeated = ci.getLast() - ci.getFirst() + 1;
            ColumnRowInfo colInfo = new ColumnRowInfo(  ci.getColWidth(),
                                                        repeated,
                                                        ColumnRowInfo.COLUMN);
            colRowVector.add(colInfo);
        }

        // Collect Rows from worksheet and add them to the vector
        for(Iterator<Row> e  = ws.getRows();e.hasNext();) {
            Row rw = e.next();
            // We will use the repeat field for number (unlike columns rows
            // cannot be repeated, we have unique record for each row in pxl
            int repeated = rw.getRowNumber();
            ColumnRowInfo rowInfo = new ColumnRowInfo(  rw.getRowHeight(),
                                                        repeated,
                                                        ColumnRowInfo.ROW);
            colRowVector.add(rowInfo);
        }
        Debug.log(Debug.TRACE,"Getting " + colRowVector.size() + " ColRowInfo records");
        return (colRowVector.iterator());
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
        ws =  wb.getWorksheet(sheetIndex);
        cellValue = ws.getCellEnumerator();
        wsIndex = sheetIndex;
        while(goToNextCell()) {
            maxRows = Math.max(maxRows, cell.getRow());
            maxCols = Math.max(maxCols, cell.getCol());
        }
        cellValue = ws.getCellEnumerator();
        Debug.log(Debug.TRACE,"Max Cols : " + maxCols + " MaxRows : " + maxRows);
    }


    /**
     *  This method returns the name of the current spreadsheet.
     *
     *  @return  The name of the current WorkSheet.
     */
    public String getSheetName() {

        String wsName = wb.getSheetName(wsIndex);
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

        try {
            cell = (CellValue) cellValue.next();
            Debug.log(Debug.TRACE,"Current Cell : " + cell.getString());
            readCellFormat();
            success = true;
        } catch (NoSuchElementException e) {
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
                if (contents.startsWith("=")) {
                    contents = parseFormula(contents);
                }
            }
            catch (IOException e) {
                System.err.println("Could Not retrieve Cell contents");
                System.err.println("Setting contents of cell(" + cell.getRow()
                                    + "," + cell.getCol() + ") to an empty string");
                System.err.println("Error msg: " + e.getMessage());
            }
        }

        return contents;
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
    private void readCellFormat() throws IOException {

        fmt.clearFormatting();

        Debug.log(Debug.TRACE," ixfe for Current Cell " + cell.getIxfe());
        ExtendedFormat xf = wb.getExtendedFormat(cell.getIxfe());
        Debug.log(Debug.TRACE," ixfnt for Current Cell " + xf.getFontIndex());
        FontDescription fd = wb.getFontDescription(xf.getFontIndex());

        fmt.setAttribute(Format.ITALIC, fd.isItalic());
        fmt.setAttribute(Format.BOLD, fd.isBold());
        fmt.setAttribute(Format.UNDERLINE, fd.isUnderline());
        fmt.setForeground(fd.getForeground());

        fmt.setBackground(xf.getBackground());
        fmt.setAlign(xf.getAlign());
        fmt.setVertAlign(xf.getVertAlign());
        fmt.setAttribute(Format.WORD_WRAP, xf.isWordWrap());

        fmt.setAttribute(Format.TOP_BORDER, xf.isBorder(ExtendedFormat.TOP_BORDER));
        fmt.setAttribute(Format.BOTTOM_BORDER, xf.isBorder(ExtendedFormat.BOTTOM_BORDER));
        fmt.setAttribute(Format.RIGHT_BORDER, xf.isBorder(ExtendedFormat.RIGHT_BORDER));
        fmt.setAttribute(Format.LEFT_BORDER, xf.isBorder(ExtendedFormat.LEFT_BORDER));

        fmt.setFontName(fd.getFont());
        fmt.setFontSize(fd.getFontSize());

        fmt.setCategory(getCellDataType());

    }
}

