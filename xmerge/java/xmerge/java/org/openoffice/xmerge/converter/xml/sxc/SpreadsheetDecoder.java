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
import java.util.Enumeration;

import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.ConvertData;

/**
 *  This class is a abstract class for encoding a &quot;Device&quot;
 *  <code>Document</code> format into an alternative spreadsheet format.
 *
 *  @author  Mark Murnane
 */
public abstract class SpreadsheetDecoder {

    /**
     *  Constructor for creating new <code>SpreadsheetDecoder</code>.
     */
    public SpreadsheetDecoder(String name, String password) throws IOException {
    }

    /**
     *  Returns the total number of sheets in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public abstract int getNumberOfSheets();

    /**
     *  Returns an Enumeration to a Vector of <code>NameDefinition</code>.
     *
     *  @return  The Enumeration
     */
    public abstract Enumeration getNameDefinitions();

    /**
     *  Returns an <code>BookSettings</code>
     *
     *  @return  The Enumeration
     */
    public abstract BookSettings getSettings();

     /**
     *  Returns an Enumeration to a Vector of <code>ColumnRowInfo</code>.
     *
     *  @return  The Enumeration
     */
    public abstract Enumeration getColumnRowInfos();

    /**
     *  Returns the number of populated rows in the current WorkSheet.
     *
     *  @return  the number of populated rows in the current WorkSheet.
     */
    public abstract int getNumberOfRows();


    /**
     *  Returns the number of populated columns in the current WorkSheet.
     *
     *  @return  The number of populated columns in the current WorkSheet.
     */
    public abstract int getNumberOfColumns();


    /**
     *  Returns the name of the current WorkSheet.
     *
     *  @return  Name of the current WorkSheet.
     */
    public abstract String getSheetName();


    /**
     *  Returns the number of the active column.
     *
     *  @return  The number of the active column.
     */
    public abstract int getColNumber();


    /**
     *  Returns the number of the active row.
     *
     *  @return  The number of the active row.
     */
    public abstract int getRowNumber();


    /**
     *  Sets the active WorkSheet.
     *
     *  @param  sheetIndex  The index of the sheet to be made active.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public abstract void setWorksheet(int sheetIndex) throws IOException;


    /**
     *  Move on the next populated cell in the current WorkSheet.
     *
     *  @return  true if successful, false otherwise.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public abstract boolean goToNextCell() throws IOException;


    /**
     *  Return the contents of the active cell.
     *
     *  @return  The cell contents.
     */
    public abstract String getCellContents();

    /**
     *  Return the value of the active cell. Used in the case of Formula where
     *  the cell contents and the cell value are not the same thing.
     *
     *  @return  The cell value.
     */
    public abstract String getCellValue();

    /**
     *  Return the data type of the active cell.
     *
     *  @return  The cell data type.
     */
    public abstract String getCellDataType();


    /**
     *  Return a <code>Format</code> object describing the active cells
     *  formatting.
     *
     *  @return  <code>Format</code> object for the cell.
     */
    public abstract Format getCellFormat();


    /**
     *  Add the contents of a <code>ConvertData</code> to the workbook.
     *
     *  @param  cd  The <code>ConvertData</code> containing the
     *              content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public abstract void addDeviceContent(ConvertData cd) throws IOException;
}

