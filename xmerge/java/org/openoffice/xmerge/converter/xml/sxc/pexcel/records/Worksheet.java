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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Vector;
import java.util.Enumeration;

import org.openoffice.xmerge.util.IntArrayList;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.ColumnRowInfo;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 *  This class is used by <code>PxlDocument</code> to maintain pexcel
 *  worksheets.
 *
 *  @author  Martin Maher
 */
public class Worksheet {

    private String name;
    private Workbook wb;
    private Vector rows     = new Vector();
    private Vector colInfo  = new Vector();
    private Vector cells    = new Vector();
    private DefColWidth dcw = new DefColWidth();
    private DefRowHeight drh    = new DefRowHeight();
    private Window2 win2    = new Window2();
    private Selection s     = new Selection();
    private BeginningOfFile bof;
    private Eof eof;

    /**
      * Writes the current workbook to the <code>Outputstream</code>
      *
      * @param  os The destination outputstream
      */
    public Worksheet(Workbook wb) {
        this.wb = wb;
    }

    /**
      * Default Contructor
      *
      * @param  os The destination outputstream
      */
    public Worksheet() {
    }

    /**
      * Writes the current workbook to the <code>Outputstream</code>
      *
      * @param  os The destination outputstream
      */
    public void write(OutputStream os) throws IOException {

        bof     = new BeginningOfFile(false);
        bof.write(os);
        dcw.write(os);
        for(Enumeration e = colInfo.elements();e.hasMoreElements();) {
            ColInfo ci = (ColInfo) e.nextElement();
            ci.write(os);
        }
        drh.write(os);
        for(Enumeration e = rows.elements();e.hasMoreElements();) {
            Row rw = (Row) e.nextElement();
            rw.write(os);
        }
        for(Enumeration e = cells.elements();e.hasMoreElements();) {
            BIFFRecord cv = (BIFFRecord) e.nextElement();
            cv.write(os);
        }
        win2.write(os);
        s.write(os);
        eof     = new Eof();
        eof.write(os);
    }

    /**
      * Reads a worksheet from the <code>InputStream</code> and contructs a
     *  workbook object from it
     *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public boolean read(InputStream is) throws IOException {

        int b = is.read();

        if (b==-1)
            return false;

        while(b!=-1) {
            switch (b)
            {
                case PocketExcelConstants.BLANK_CELL:
                    Debug.log(Debug.TRACE,"Blank Cell (01h)");
                    BlankCell bc = new BlankCell(is);
                    cells.add(bc);
                    break;

                case PocketExcelConstants.NUMBER_CELL:
                    Debug.log(Debug.TRACE,"NUMBER: Cell Value, Floating-Point Number (03h)");
                    FloatNumber fn = new FloatNumber(is);
                    cells.add(fn);
                    break;

                case PocketExcelConstants.LABEL_CELL:
                    Debug.log(Debug.TRACE,"LABEL: Cell Value, String Constant (04h)");
                    LabelCell lc = new LabelCell(is);
                    cells.add(lc);
                    break;

                case PocketExcelConstants.BOOLERR_CELL:
                    Debug.log(Debug.TRACE,"BOOLERR: Cell Value, Boolean or Error (05h)");
                    BoolErrCell bec = new BoolErrCell(is);
                    break;

                case PocketExcelConstants.FORMULA_CELL:
                    Debug.log(Debug.TRACE,"FORMULA: Cell Formula (06h)");
                    Formula f = new Formula(is, wb);
                    cells.add(f);
                    break;

                 case PocketExcelConstants.FORMULA_STRING:
                    Debug.log(Debug.TRACE,"String Value of a Formula (07h)");
                    StringValue sv = new StringValue(is);
                    break;

                case PocketExcelConstants.ROW_DESCRIPTION:
                    Debug.log(Debug.TRACE,"ROW: Describes a Row (08h)");
                    Row rw = new Row(is);
                    rows.add(rw);
                    break;

                case PocketExcelConstants.BOF_RECORD:
                    Debug.log(Debug.TRACE,"BOF Record");
                    bof = new BeginningOfFile(is);
                    break;

                case PocketExcelConstants.EOF_MARKER:
                    Debug.log(Debug.TRACE,"EOF Marker");
                    eof = new Eof();
                    return true;
                   /*
                case PocketExcelConstants.DEFINED_NAME:
                    Debug.log(Debug.TRACE,"NAME: Defined Name (18h)");
                    DefinedName dn = new DefinedName(is, wb);
                    break;
                     */
                case PocketExcelConstants.CURRENT_SELECTION:
                    Debug.log(Debug.TRACE,"SELECTION: Current Selection (1Dh)");
                    Selection s = new Selection(is);
                    break;

                case PocketExcelConstants.NUMBER_FORMAT:
                    Debug.log(Debug.TRACE,"FORMAT: Number Format (1Eh)");
                    NumberFormat nf = new NumberFormat(is);
                    break;

                case PocketExcelConstants.DEFAULT_ROW_HEIGHT:
                    Debug.log(Debug.TRACE,"DEFAULTROWHEIGHT: Default Row Height (25h)");
                    drh = new DefRowHeight(is);
                    break;

                case PocketExcelConstants.SHEET_WINDOW_INFO:
                    Debug.log(Debug.TRACE,"WINDOW2: Sheet Window Information (3Eh) [PXL 2.0]");
                    win2 = new Window2(is);
                    break;

                case PocketExcelConstants.PANE_INFO:
                    Debug.log(Debug.TRACE,"PANE: Number of Panes and their Position (41h) [PXL 2.0]");
                    Pane p = new Pane(is);
                    break;

                case PocketExcelConstants.DEF_COL_WIDTH:
                    Debug.log(Debug.TRACE,"DEFCOLWIDTH: Default Column Width (55h) [PXL 2.0]");
                    dcw = new DefColWidth(is);
                    break;

                case PocketExcelConstants.COLINFO:
                    Debug.log(Debug.TRACE,"COLINFO: Column Formatting Information (7Dh) [PXL 2.0]");
                    ColInfo ci = new ColInfo(is);
                    colInfo.add(ci);
                    break;

                default:
                    break;
            }
            b = is.read();

        }
        Debug.log(Debug.TRACE,"Leaving Worksheet:");

        return true;
    }

    /**
      * Returns an enumerator which will be used to access individual cells
     *
      * @return an enumerator to the worksheet cells
      */
    public Enumeration getCellEnumerator() throws IOException {
        return (cells.elements());
    }

    /**
      * Adds a cell to this worksheet. Current valdid celltypes are
     * <code>FloatNumber</code>, <code>LabelCell</code> or <code>Formula</code>
     *
      * @param  f the font recrod to add
      */
    public void addCell(BIFFRecord br) {
        cells.add(br);
    }

    /**
      * Adds a number of ColInfo Records to the worksheet base on a list of
     * clumnwidths passed in
     *
      * @param  list of column widths
      */
    public void addColRows(Vector columnWidths) {

        int nCols = 0;
        int nRows = 0;

        Debug.log(Debug.TRACE,"Worksheet: addColInfo : " + columnWidths);
        for(Enumeration e = columnWidths.elements();e.hasMoreElements();) {
            ColumnRowInfo cri =(ColumnRowInfo) e.nextElement();
            int size = cri.getSize();
            int repeated = cri.getRepeated();
            if(cri.isColumn()) {
                Debug.log(Debug.TRACE,"Worksheet: adding ColInfo width = " + size);
                ColInfo newColInfo = new ColInfo(   nCols,
                                                    nCols+repeated,
                                                    size, (byte) 2);
                colInfo.add(newColInfo);
                nCols += repeated;
            } else if(cri.isRow()) {
                Debug.log(Debug.TRACE,"Worksheet: adding Row Height = " + size);
                if(size!=255) {
                    for(int i=0;i<repeated;i++) {
                        Row newRow = new Row(nRows++, size);
                        rows.add(newRow);
                    }
                } else {
                    // If it is the Default Row we don't need to add it
                    nRows += repeated;
                }
            }
        }
    }

    /**
     * Returns an <code>Enumeration</code> to the ColInfo's for this worksheet
     *
     * @return an <code>Enumeration</code> to the ColInfo's
     */
     public Enumeration getColInfos() {

         return (colInfo.elements());
     }

    /**
     * Returns an <code>Enumeration</code> to the Rows for this worksheet
     *
     * @return an <code>Enumeration</code> to the Rows
     */
     public Enumeration getRows() {

         return (rows.elements());
     }

}
