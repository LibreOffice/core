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
import java.util.Vector;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.IntArrayList;

import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetEncoder;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.NameDefinition;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocumentSerializerImpl
 * SxcDocumentSerializerImpl} to encode the Pocket Excel format.
 *
 *  @author  Martin Maher
 */
final class PocketExcelEncoder extends SpreadsheetEncoder {

    private Workbook wb;

    /**
     *  Constructor creates a Pocket Excel WorkBook.
     *
     *  @param  name      The name of the WorkBook.
     *  @param  password  The password for the WorkBook.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    PocketExcelEncoder(String name, String password) throws IOException {

        super(name, password);
        wb = new Workbook(name);

    }


    /**
     *  This method creates a WorkSheet belonging to the
     *  WorkBook.
     *
     *  @param  sheetName  The name of the WorkSheet.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void createWorksheet(String sheetName) throws IOException {

        wb.addWorksheet(sheetName);
    }


    /**
     *  This method gets the number of sheets in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public int getNumberOfSheets() {

        Vector v = wb.getWorksheetNames();
        return (v.size());
    }


    /**
     *  This method returns the Workbook created.
     *
     *  @return  Returns a <code>Workbook</code>
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public Workbook getWorkbook() throws IOException {

        return wb;
    }

    /**
     *  This method converts a String containing a formula in infix notation
     *  to a String in Reverse Polish Notation (RPN)
     *
     *  @return a parsed pexcel formula in RPN
     */
    protected String parseFormula(String formula) {

        Debug.log(Debug.TRACE,"Strip Formula (Before) : " + formula);

        StringBuffer inFormula = new StringBuffer(formula);
        StringBuffer outFormula = new StringBuffer();

        boolean inBrace = false;
        boolean firstCharAfterBrace = false;
        boolean firstCharAfterColon = false;

        int len = inFormula.length();

        for (int in = 0; in < len; in++) {
            switch (inFormula.charAt(in)) {
            case '[':
                // We are now inside a StarOffice cell reference.
                // We also need to strip out the '['
                Debug.log(Debug.TRACE,"brace Found");
                inBrace = true;

                // If the next character is a '.', we want to strip it out
                firstCharAfterBrace = true;
                break;

            case ']':
                // We are exiting a StarOffice cell reference
                // We are stripping out the ']'
                inBrace = false;
                break;
            case '.':
                if (inBrace == true && (firstCharAfterBrace == true ||
                    firstCharAfterColon == true) ) {

                    Debug.log(Debug.TRACE,"dot Found and in brace");
                    // Since we are in a StarOffice cell reference,
                    // and we are the first character, we need to
                    // strip out the '.'
                    firstCharAfterBrace = false;
                    firstCharAfterColon = false;

                } else if(firstCharAfterColon == true) {
                    firstCharAfterColon = false;
                } else {
                    outFormula.append(inFormula.charAt(in));
                }
                break;

            case ':':
                // We have a cell range reference.
                // May need to strip out the leading '.'
                firstCharAfterColon = true;
                outFormula.append(inFormula.charAt(in));
                break;

            case ';':
                // StarOffice XML format uses ';' as a separator.  MiniCalc (and
                // many spreadsheets) use ',' as a separator instead.
                outFormula.append(',');
                break;

            default:
                // We hit valid data, lets add it to the formula string
                outFormula.append(inFormula.charAt(in));

                // Need to make sure that firstCharAfterBrace is not true.
                firstCharAfterBrace = false;
                break;
            }
        }

        Debug.log(Debug.TRACE,"Strip Formula (After) : " + outFormula);
        return outFormula.toString();
    }

    /**
     *  Add a cell to the current WorkSheet.
     *
     *  @param   row             The row number of the cell.
     *  @param   column          The column number of the cell.
     *  @param   fmt             The <code>Format</code> object describing
     *                           the appearance of this cell.
     *  @param   cellContents    The text or formula of the cell's contents.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void addCell(int row, int column, Format fmt, String cellContents) throws IOException {

        if (cellContents.startsWith("=")) {
            cellContents = parseFormula(cellContents);
            Debug.log(Debug.TRACE,"Parsing Formula " + cellContents);
        }
        wb.addCell(row, column, fmt, cellContents);
    }


    /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public void setColumnRows(Vector columnRows) throws IOException {

        wb.addColInfo(columnRows);
    }

     /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public void setNameDefinition(NameDefinition nd) throws IOException {

        String parsedName = nd.getDefinition();
        nd.setDefinition(parseFormula(parsedName));

        wb.addNameDefinition(nd);
    }
    /**
     *  This method sets the format of a cell to <i>string</i>.
     *
     *  @param  format  The cell format-may already contain display info,
     *                  such as alignment or font type.
     *
     *  @return  The updated format of the cell.
     */
    private long setFormatString(long format) {

        return 0;
    }


    /**
     *  Set a cell's formatting options via a separately create
     *  <code>Format</code> object.
     *
     *  @param  row     The row number of the cell to be changed.
     *  @param  column  The column number of the cell to be changed.
     *  @param  fmt     Object containing formatting settings for
     *                 this cell.
     */
    public void setCellFormat(int row, int column, Format fmt) {
        Debug.log(Debug.TRACE,"bold : " + fmt.getAttribute(Format.BOLD) +
                            ",Italic : " + fmt.getAttribute(Format.ITALIC) +
                            ",Underline : " + fmt.getAttribute(Format.UNDERLINE));
    }


    /**
     *  Get the names of the sheets in the WorkBook.
     *
     *  @param  sheet  The required sheet.
     */
    public String getSheetName(int sheet) {

        Vector v = wb.getWorksheetNames();
        String wsName = (String) (v.elementAt(sheet));

        return wsName;
    }


}

