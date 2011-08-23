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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Vector;
import java.util.Enumeration;
import java.awt.Point;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.converter.xml.sxc.SheetSettings;
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
    private Vector rows			= new Vector();
    private Vector colInfo		= new Vector();
    private Vector cells		= new Vector();
    private DefColWidth dcw		= new DefColWidth();
    private DefRowHeight drh	= new DefRowHeight();
    private Window2 win2		= new Window2();
    private Selection sel		= new Selection();
    private Pane p				= new Pane();
    private BeginningOfFile bof;
    private Eof eof;
    
    /**
      *	Writes the current workbook to the <code>Outputstream</code> 
      * 
      * @param	os The destination outputstream	
      */
    public Worksheet(Workbook wb) {
        this.wb = wb;
    }
    
    /**
      * Default Contructor	
      * 
      * @param	os The destination outputstream	
      */
    public Worksheet() {
    }	

    /**
      *	Writes the current workbook to the <code>Outputstream</code> 
      * 
      * @param	os The destination outputstream	
      */
    public void write(OutputStream os) throws IOException {
        
        bof		= new BeginningOfFile(false);
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
        p.write(os);
        sel.write(os);
        eof		= new Eof();
        eof.write(os);
    }

    /**
      *	Reads a worksheet from the <code>InputStream</code> and contructs a
     *	workbook object from it 
     *
      * @param	is InputStream containing a Pocket Excel Data file.
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

                case PocketExcelConstants.CURRENT_SELECTION:
                    Debug.log(Debug.TRACE,"SELECTION: Current Selection (1Dh)");
                    sel = new Selection(is);
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
                    p = new Pane(is);
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
      *	Returns an enumerator which will be used to access individual cells	
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
      * @param	f the font recrod to add	
      */
    public void addCell(BIFFRecord br) {
        cells.add(br);
    }

    /**
      * Adds a number of ColInfo Records to the worksheet base on a list of
     * clumnwidths passed in
     *
      * @param	list of column widths	
      */
    public void addRow(Row r) {
        rows.add(r);
    }

    /**
      * Adds a number of ColInfo Records to the worksheet base on a list of
     * clumnwidths passed in
     *
      * @param	list of column widths	
      */
    public void addCol(ColInfo c) {
        colInfo.add(c);
    }
    /**
     * Returns an <code>Enumeration</code> to the ColInfo's for this worksheet
     *
     * @return an <code>Enumeration</code> to the ColInfo's 
     */
    public void addSettings(SheetSettings s) {

        sel.setActiveCell(s.getCursor());
        p.setLeft(s.getLeft());
        p.setTop(s.getTop());
        p.setPaneNumber(s.getPaneNumber());
        Point split = s.getSplit();
        if(split.getX()!=0 || split.getY()!=0) {
            p.setSplitPoint(s.getSplitType(), split);
            win2.setSplitType(s.getSplitType());
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
     * Returns a <code>SheetSettings</code> object containing a collection of data
     * contained in <code>Pane</code>, <code>Window2</code> and
     * <code>Selection</code>
     *
     * @return an <code>SheetSettings</code>
     */
     public SheetSettings getSettings() {

        SheetSettings s = new SheetSettings();
        s.setCursor(sel.getActiveCell());
        if(win2.isFrozen()) {
            s.setFreeze(p.getFreezePoint());
        } else if(win2.isSplit()) {
            s.setSplit(p.getSplitPoint());
        }
        s.setPaneNumber(p.getPaneNumber());
        s.setTopLeft(p.getTop(), p.getLeft());
         return s;
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
