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

import java.io.IOException;
import java.util.Vector;

/**
 *  <p>This class is a abstract class for encoding an SXC into an 
 *  alternative spreadsheet format.</p>
 *
 *  <p>TODO - Add appropriate exceptions to each of the methods.</p>
 *
 *  @author  Mark Murnane
 */
public abstract class SpreadsheetEncoder {


    /**
     *  Creates new SpreadsheetEncoder.
     * 
     *  @param  name      The name of the WorkBook to be created.
     *  @param  password  An optional password for the WorkBook.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public SpreadsheetEncoder(String name, String password) throws IOException { };


    /**
     *  Create a new WorkSheet within the WorkBook.
     *
     *  @param  sheetName  The name of the WorkSheet.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public abstract void createWorksheet(String sheetName) throws IOException;


    /**
     *  Set a cell's formatting options via a separately create
     *  <code>Format</code> object.
     *
     *  @param  row     The row number of the cell to be changed
     *  @param  column  The column number of the cell to be changed
     *  @param  fmt     Object containing formatting settings for this cell.
     */
    public abstract void setCellFormat(int row, int column, Format fmt);


    /**
     *  Add a cell to the current WorkSheet.
     * 
     *  @param  row           The row number of the cell
     *  @param  column        The column number of the cell
     *  @param  fmt           The <code>Format</code> object describing the
     *                        appearance of this cell.
     *  @param  cellContents  The text or formula of the cell's contents.
     */
    public abstract void addCell(int row, int column, 
                                 Format fmt, String cellContents) throws IOException;


    /**
     *  Get the number of sheets in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public abstract int getNumberOfSheets();


    /**
     *  Get the names of the sheets in the WorkBook.
     *
     *  @param  sheet  The required sheet.
     */
    public abstract String getSheetName(int sheet);


    /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public abstract void setColumnRows(Vector columnRows) throws IOException;

    /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public abstract void setNameDefinition(NameDefinition nd) throws IOException;

    /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public abstract void addSettings(BookSettings s) throws IOException;		
}

