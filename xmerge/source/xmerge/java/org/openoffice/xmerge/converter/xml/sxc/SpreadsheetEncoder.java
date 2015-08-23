/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    public abstract void setColumnRows(Vector columnWidths) throws IOException;

    /**
     */
    public abstract void setNameDefinition(NameDefinition nd) throws IOException;

    /**
     */
    public abstract void addSettings(BookSettings s) throws IOException;
}

