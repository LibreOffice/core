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
import java.util.Vector;

import org.openoffice.xmerge.util.IntArrayList;

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
}

