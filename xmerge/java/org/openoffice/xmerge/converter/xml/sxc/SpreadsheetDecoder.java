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

import java.io.IOException;
import java.io.InputStream;
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

