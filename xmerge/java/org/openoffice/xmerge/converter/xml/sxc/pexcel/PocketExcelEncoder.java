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
import java.util.Stack;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.IntArrayList;

import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetEncoder;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 *  This class is used by {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocumentSerializerImpl
 * SxcDocumentSerializerImpl} to encode the Pocket Excel format.
 *
 *  @author  Martin Maher
 */
final class PocketExcelEncoder extends SpreadsheetEncoder {

    private PxlDocument pxlDoc;

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
        pxlDoc = new PxlDocument(name);

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

        pxlDoc.addWorksheet(sheetName);
    }


    /**
     *  This method gets the number of sheets in the WorkBook.
     *
     *  @return  The number of sheets in the WorkBook.
     */
    public int getNumberOfSheets() {

        Vector v = pxlDoc.getWorksheetNames();
        return (v.size());
    }


    /**
     *  This method returns the PxlDoc created.
     *
     *  @return  Returns a <code>PxlDocument</code>
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public PxlDocument getPxlDoc() throws IOException {

        return pxlDoc;
    }

    /**
     *  This method returns the priority of operators for the RPN conversion
     *
     *  @return the operator priority level
     */
    private int getPriority(String op) {

        int operatorPriority = -1;

        if (op.equals("("))
            operatorPriority = 1;
        else if (op.equals("+"))
            operatorPriority = 2;
        else if (op.equals("-"))
            operatorPriority = 2;
        else if (op.equals("*"))
            operatorPriority = 3;
        else if (op.equals("/"))
            operatorPriority = 3;
        else if (op.equals("^"))
            operatorPriority = 4;

        return(operatorPriority);
    }

    /**
     *  This method converts a String containing a formula in infix notation
     *  to a String in Reverse Polish Notation (RPN)
     *
     *  @return a parsed pexcel formula in RPN
     */
    protected String parseFormula(String formula) {

        Debug.log(Debug.TRACE,"parseFormula : " + formula);

        Stack aStack = new Stack();
        String nextChar;
        String topOfStack;
        String outputString = "=";
        String inputString;
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
                inBrace = true;

                // If the next character is a '.', we want to strip it out
                firstCharAfterBrace = true;
                break;

            case ']':
                // We are exiting a StarOffice cell reference
                // We are stripping out the ']'
                inBrace = false;
                break;

            case ':':
                // We have a cell range reference.
                // May need to strip out the leading '.'
                if (inBrace)
                    firstCharAfterColon = true;
                outFormula.append(inFormula.charAt(in));
                break;

            case '.':
                if (inBrace == true) {
                    if (firstCharAfterBrace == false &&
                            firstCharAfterColon == false) {
                        // Not the first character after the open brace.
                        // We have hit a separator between a sheet reference
                        // and a cell reference.  MiniCalc uses a ! as
                        // this type of separator.
                        outFormula.append('!');
                    }
                    else {
                        firstCharAfterBrace = false;
                        firstCharAfterColon = false;
                        // Since we are in a StarOffice cell reference,
                        // and we are the first character, we need to
                        // strip out the '.'
                    }
                    break;
                } else {
                    // We hit valid data, lets add it to the formula string
                    outFormula.append(inFormula.charAt(in));
                    break;
                }

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

        inputString = outFormula.toString();

        for (int i = 1;i<=inputString.length()-1;i++) {

            char ch = inputString.charAt(i);        // Check to see if this is a cell or an operator
            if(((ch>='a') && (ch<='z')) || (ch>='A') && (ch<='Z'))  {
                int interval = 1;
                char nextRef = inputString.charAt(i+interval);
                if(((nextRef>='a') && (nextRef<='z')) || (nextRef>='A') && (nextRef<='Z')) { // if cell col > 26
                    interval++;
                    nextRef = inputString.charAt(i+interval);
                }
                while(nextRef>='0' && nextRef<='9') { // Keep reading until we reach another operator or cell reference
                    interval++;
                    if((i+interval)<inputString.length())
                        nextRef = inputString.charAt(i+interval);
                    else
                        nextRef = 0;
                }
                nextChar = inputString.substring(i,i+interval); // if cell then read all of the cell reference
                i += interval-1;
            } else
                nextChar = inputString.substring(i,i+1);

            if (nextChar.equals(")")) {
                topOfStack = (String)aStack.pop();
                while (!topOfStack.equals("(") && !aStack.empty()) {
                    outputString += topOfStack;
                    topOfStack = (String)aStack.pop();
                }
            } else if (nextChar.equals("(") || i==1) {
                    aStack.push(nextChar);
            } else if ( nextChar.equals("+") ||
                        nextChar.equals("-") ||
                        nextChar.equals("*") ||
                        nextChar.equals("/") ||
                        nextChar.equals("^")) {

                if(!aStack.empty()) {
                    topOfStack = (String)aStack.peek();
                while (getPriority(nextChar) <= getPriority(topOfStack) && !aStack.empty()) {
                    topOfStack = (String)aStack.pop();
                    outputString += topOfStack;
                    if(!aStack.empty())
                        topOfStack = (String)aStack.peek();
                    }
                }
                aStack.push(nextChar);
            } else {
                outputString += nextChar;
            }
        }
        while(!aStack.empty()) {
            topOfStack = (String)aStack.pop();
            outputString += topOfStack;
        }
        return outputString;
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
        }
        pxlDoc.addCell(row, column, fmt, cellContents);
    }


    /**
     *  Set the width of the columns in the WorkBook.
     *
     *  @param  columnWidths  An <code>IntArrayList</code> of column
     *                        widths.
     */
    public void setColumnWidths(IntArrayList columnWidths) throws IOException {

        pxlDoc.addColInfo(columnWidths);
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
        Debug.log(Debug.TRACE,"bold : " + fmt.isBold() +
                            ",Italic : " + fmt.isItalic() +
                            ",Underline : " + fmt.isUnderline());
    }


    /**
     *  Get the names of the sheets in the WorkBook.
     *
     *  @param  sheet  The required sheet.
     */
    public String getSheetName(int sheet) {

        Vector v = pxlDoc.getWorksheetNames();
        String wsName = (String) (v.elementAt(sheet));

        return wsName;
    }


    /*
     *  This method returns a MiniCalc style format from the
     *  <code>Format</code> object.
     */
     /*
    private long getFormat(Format fmt) {
        return 0;
    }*/
}

