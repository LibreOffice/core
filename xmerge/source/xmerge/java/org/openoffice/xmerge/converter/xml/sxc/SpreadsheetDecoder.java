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

