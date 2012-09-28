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

/**
 *  <p>This class is a abstract class for encoding an SXC into an
 *  alternative spreadsheet format.</p>
 *
 *  <p>TODO - Add appropriate exceptions to each of the methods.</p>
 *
 */
public abstract class SpreadsheetEncoder {


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
     *  @param  columnRows  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public abstract void setColumnRows(ArrayList<ColumnRowInfo> columnRows) throws IOException;

    /**
     *  Set the name definition of this spreadsheet
     *
     *  @param  nd  The <code>NameDefinition</code> to use.
     */
    public abstract void setNameDefinition(NameDefinition nd) throws IOException;

    /**
     *  Adds settings to the WorkBook.
     *
     *  @param  s  The <code>BookSettings</code> to add.
     */
    public abstract void addSettings(BookSettings s) throws IOException;
}

