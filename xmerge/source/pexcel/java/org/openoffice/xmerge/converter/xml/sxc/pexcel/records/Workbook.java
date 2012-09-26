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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.converter.xml.sxc.BookSettings;
import org.openoffice.xmerge.converter.xml.sxc.ColumnRowInfo;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.NameDefinition;
import org.openoffice.xmerge.converter.xml.sxc.SheetSettings;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;
import org.openoffice.xmerge.util.Debug;

/**
 *  This class is used by <code> PxlDocument</code> to maintain pexcel
 *  workbooks.
 *
 */
public class Workbook implements org.openoffice.xmerge.Document,
OfficeConstants {

    private ArrayList<FontDescription> fonts                = new ArrayList<FontDescription>();
    private ArrayList<ExtendedFormat> extendedFormats      = new ArrayList<ExtendedFormat>();
    private ArrayList<Worksheet> worksheets           = new ArrayList<Worksheet>();
    private ArrayList<BoundSheet> boundsheets          = new ArrayList<BoundSheet>();
    private ArrayList<DefinedName> definedNames         = new ArrayList<DefinedName>();
    private static final CodePage cp;
    private static final Window1 win1;
    private static final BeginningOfFile bof;
    private static final Eof eof;
    private String fileName;

    static {
        cp              = new CodePage();
        win1            = new Window1();
        bof             = new BeginningOfFile(true);
        eof             = new Eof();
    }

    /**
     * Constructs a pocket Excel Workbook with the name of the file passed in
     * as an argument. Also fills out a basic header block containing the
     * minimum number of objects that can be created at this time.
     *
     * @param   name    Name of the Pocket Excel Data file. (excluding the file
     *                  extension)
     */
    public Workbook(String name) throws IOException {
        fileName = name + PocketExcelConstants.FILE_EXTENSION;
        Format defaultFormat = new Format();
        FontDescription fd = new FontDescription(defaultFormat);
        fonts.add(fd);
        ExtendedFormat xf = new ExtendedFormat(0, defaultFormat);
        extendedFormats.add(xf);
    }

    /**
     * Constructs a pocket Excel Workbook from the
     * <code>InputStream</code> and assigns it the document name passed in
     *
     * @param   name Name of the Pocket Excel Data file. (including the file
     *               extension)
     * @param   is   InputStream containing a Pocket Excel Data file.
     */
    public Workbook(String name, InputStream is) throws IOException {
        read(is);
        fileName = name;
    }

    /**
      * Writes the current workbook to the <code>Outputstream</code>
      *
      * @param  os The destination outputstream
      */
    public void write(OutputStream os) throws IOException {
        bof.write(os);
        cp.write(os);
        for(Iterator<DefinedName> e = definedNames.iterator();e.hasNext();) {
            DefinedName dn = e.next();
            dn.write(os);
        }
        win1.write(os);
        for(Iterator<FontDescription> e = fonts.iterator();e.hasNext();) {
            FontDescription fd = e.next();
            fd.write(os);
        }
        for(Iterator<ExtendedFormat> e = extendedFormats.iterator();e.hasNext();) {
            ExtendedFormat xf = e.next();
            xf.write(os);
        }
        for(Iterator<BoundSheet> e = boundsheets.iterator();e.hasNext();) {
            BoundSheet bs = e.next();
            bs.write(os);
        }
        eof.write(os);

        for(Iterator<Worksheet> e = worksheets.iterator();e.hasNext();) {
            Worksheet ws = e.next();
            ws.write(os);
        }
    }

    /**
      * Reads a workbook from the <code>InputStream</code> and contructs a
     *  workbook object from it
     *
      * @param  is InputStream containing a Pocket Excel Data file.
      */
    public void read(InputStream is) throws IOException {

        boolean done = false;

        int b = 0;
        while (!done)
        {
            b = is.read();
            if (b == -1)
            {
                Debug.log(Debug.TRACE,"End of file reached");
                break;
            }

            switch (b)
            {
                case PocketExcelConstants.DEFINED_NAME:
                    Debug.log(Debug.TRACE,"NAME: Defined Name (18h)");
                    DefinedName dn = new DefinedName(is, this);
                    definedNames.add(dn);
                    break;

                case PocketExcelConstants.BOF_RECORD:
                    Debug.log(Debug.TRACE,"BOF Record");
                    bof.read(is);
                    break;

                case PocketExcelConstants.EOF_MARKER:
                    Debug.log(Debug.TRACE,"EOF Marker");
                    eof.read(is);
                    Worksheet ws = new Worksheet(this);
                    while(ws.read(is)) {
                        worksheets.add(ws);
                        ws = new Worksheet(this);
                    }
                    break;

                case PocketExcelConstants.FONT_DESCRIPTION:
                    Debug.log(Debug.TRACE,"FONT: Font Description (31h)");
                    FontDescription fd = new FontDescription(is);
                    fonts.add(fd);
                    break;

                case PocketExcelConstants.WINDOW_INFO:
                    Debug.log(Debug.TRACE,"WINDOW1: Window Information (3Dh) [PXL 2.0]");
                    win1.read(is);
                    break;

                case PocketExcelConstants.CODEPAGE:
                    Debug.log(Debug.TRACE,"CODEPAGE : Codepage and unknown fields (42h)");
                    cp.read(is);
                    break;

                case PocketExcelConstants.BOUND_SHEET:
                    Debug.log(Debug.TRACE,"BOUNDSHEET: Sheet Information (85h)");
                    BoundSheet bs = new BoundSheet(is);
                    boundsheets.add(bs);
                    break;

                case PocketExcelConstants.EXTENDED_FORMAT:
                    Debug.log(Debug.TRACE,"XF: Extended Format (E0h) [PXL 2.0]");
                    ExtendedFormat xf = new ExtendedFormat(is);
                    extendedFormats.add(xf);
                    break;

                default:
                    b = is.read();
                    break;
            }

        }
        is.close();
    }

    /**
      * Adds a font recrod to the workbook
     *
      * @param  f the font record to add
      */
    public int addFont(FontDescription f) {

        boolean alreadyExists = false;
        int i = 0;

        for(Iterator<FontDescription> e = fonts.iterator();e.hasNext();) {
            FontDescription fd = e.next();
            if(fd.compareTo(f)) {
                alreadyExists = true;
                break;
            } else {
                i++;
            }
        }

        if(!alreadyExists)
                fonts.add(f);

        return i;
    }

    /**
      * Adds a ExtendedFormat record to the workbook
     *
      * @param  fmt the font record to add
      */
    public int addExtendedFormat(Format fmt) throws IOException {

        FontDescription fd = new FontDescription(fmt);
        int ixfnt = addFont(fd);
        ExtendedFormat xf = new ExtendedFormat(ixfnt, fmt);

        boolean alreadyExists = false;
        int i = 0;

        for(Iterator<ExtendedFormat> e = extendedFormats.iterator();e.hasNext();) {
            ExtendedFormat currentXF = e.next();
            if(xf.compareTo(currentXF)) {
                alreadyExists = true;
                break;
            } else if(!alreadyExists) {
                i++;
            }
        }

        if(!alreadyExists)
            extendedFormats.add(xf);

        return i;
    }

    /**
      * Gets a worksheet at a particular index from mthe current workbook.
     *
      * @param  index the index of the worksheet to retrieve
      */
    public Worksheet getWorksheet(int index) {

        return worksheets.get(index);
    }

    /**
     * Returns a FontDescription indictated by the
     * index parameter passed in to the method
     *
     * @param ixfnt index to the FontDescriptions, this is a 0 based index
     * @return FontDescription indexed by ixfe
     */
    public FontDescription getFontDescription(int ixfnt) {

        return fonts.get(ixfnt);
    }

    /**
     * Returns a ExtendedFormat indictated by the
     * index parameter passed in to the method
     *
     * @param ixfe index to the FontDescriptions, this is a 0 based index
     * @return FontDescription indexed by ixfe
     */
    public ExtendedFormat getExtendedFormat(int ixfe) {

        return extendedFormats.get(ixfe);
    }

    /**
      * Returns an enumeration of DefinedNames for this workbook
     *
      * @return Enumeration for the DefinedNames
      */
    public Iterator<DefinedName> getDefinedNames() {

        return definedNames.iterator();
    }

    /**
      * Returns an enumeration of <code>Settings</code> for this workbook
     *
      * @return Enumeration of <code>Settings</code>
      */
    public BookSettings getSettings() {

        ArrayList<SheetSettings> settingsVector = new ArrayList<SheetSettings>();
        int index = 0;
        for(Iterator<Worksheet> e = worksheets.iterator();e.hasNext();) {
            Worksheet ws = e.next();
            SheetSettings s = ws.getSettings();
            s.setSheetName(getSheetName(index++));
            settingsVector.add(s);
        }
        BookSettings bs = new BookSettings(settingsVector);
        String activeSheetName = getSheetName(win1.getActiveSheet());
        bs.setActiveSheet(activeSheetName);
        return bs;
    }

    /**
     * Returns a <code>Vector</code> containing all the worksheet Names
     *
     * @return a <code>Vector</code> containing all the worksheet Names
     */
    public ArrayList<Object> getWorksheetNames() {

        ArrayList<Object> wsNames = new ArrayList<Object>();

        for(int i = 0;i < boundsheets.size();i++) {
            wsNames.add(getSheetName(i));
        }

        return wsNames;
    }

    /**
     * Returns the name of the worksheet at the specified index
     *
     * @return a <code>String</code> containing the name of the worksheet
     */
    public String getSheetName(int index) {
        BoundSheet bs = boundsheets.get(index);

        return bs.getSheetName();
    }

    /**
     * Adds a <code>Worksheet</code> to the workbook.
     *
     * @param name the name of the <code>Worksheet</code> to be added
     */
    public void addWorksheet(String name) throws IOException {

        BoundSheet bs = new BoundSheet(name);
        boundsheets.add(bs);

        Worksheet ws = new Worksheet();
        worksheets.add(ws);
    }

    /**
     * Adds a cell to the current worksheet.
     *
     * @param row          Row index for the new cell.
     * @param col          Column index for the new cell.
     * @param fmt          Format description for the new cell.
     * @param cellContents Contents for the new cell.
     */
    public void addCell(int row,int col, Format fmt, String cellContents)
    throws IOException {

        Worksheet currentWS = worksheets.get(worksheets.size()-1);
        int ixfe = addExtendedFormat(fmt);

        String category = fmt.getCategory();

        // Now the formatting is out of the way add the cell
        Debug.log(Debug.TRACE,"Cell Format: " + fmt);
        Debug.log(Debug.TRACE,"Row : " + row);
        Debug.log(Debug.TRACE,"Col : " + col);
        if(cellContents.startsWith("=")) {
            try {
                Formula f = new Formula(row, col, cellContents, ixfe, fmt, this);
                currentWS.addCell(f);
                if(category.equalsIgnoreCase(CELLTYPE_STRING)) {
                    StringValue sv = new StringValue(fmt.getValue());
                    currentWS.addCell(sv);
                }
            } catch(Exception e) {
                Debug.log(Debug.TRACE, "Parsing Exception thrown : " + e.getMessage());
                BoolErrCell errorCell = new BoolErrCell(row, col, ixfe, 0x2A, 1);
                currentWS.addCell(errorCell);
            }
        } else if(category.equalsIgnoreCase(OfficeConstants.CELLTYPE_FLOAT)) {
            try {
                FloatNumber num = new FloatNumber(row, col, cellContents, ixfe);
                currentWS.addCell(num);
            } catch(Exception e) {
                Debug.log(Debug.TRACE,"Error could not parse Float " + cellContents);
                LabelCell lc = new LabelCell(row, col, cellContents, ixfe);
                currentWS.addCell(lc);
            }
        } else {
            if(cellContents.length()==0) {
                Debug.log(Debug.TRACE, "Blank Cell");
                BlankCell b = new BlankCell(row, col, ixfe);
                currentWS.addCell(b);
            } else {
                Debug.log(Debug.TRACE, "Label Cell : " + cellContents);
                LabelCell lc = new LabelCell(row, col, cellContents, ixfe);
                currentWS.addCell(lc);  // three because we assume the last three
                                        // Records in any worksheet is the selection,
                                        // window2 and eof Records
            }
        }
    }

    /**
      * Will create a number of ColInfo records based on the column widths
     * passed in.
     *
      * @param  columnRows <code>Vector</code> of <code>ColumnRowInfo</code>
      */
    public void addColInfo(ArrayList<ColumnRowInfo> columnRows) throws IOException {

        Worksheet currentWS = worksheets.get(worksheets.size()-1);

        int nCols = 0;
        int nRows = 0;

        Debug.log(Debug.TRACE,"Workbook: addColInfo()");
        for(Iterator<ColumnRowInfo> e = columnRows.iterator();e.hasNext();) {
            ColumnRowInfo cri =e.next();
            int ixfe = 0;
            int size = cri.getSize();
            int repeated = cri.getRepeated();
            if(cri.isColumn()) {
                Debug.log(Debug.TRACE,"Workbook: adding ColInfo width = " + size);
                ColInfo newColInfo = new ColInfo(   nCols,
                                                    nCols+repeated-1,
                                                    size, ixfe);
                currentWS.addCol(newColInfo);
                nCols += repeated;
            } else if(cri.isRow()) {

                Debug.log(Debug.TRACE,"Workbook: adding Row Height = " + size);
                if(!cri.isDefaultSize()) {
                    for(int i=0;i<repeated;i++) {
                        Row newRow = new Row(nRows++, size, cri.isUserDefined());
                        currentWS.addRow(newRow);
                    }
                } else {
                    // If it is the Default Row we don't need to add it
                    nRows += repeated;
                }

            }
        }
    }

   /**
    * Add a name definition to the workbook.
    *
    * @param nameDefinition The name definition to add.
    */
    public void addNameDefinition(NameDefinition nameDefinition) throws IOException {

        DefinedName dn = new DefinedName(nameDefinition, this);
        definedNames.add(dn);
    }

    /**
      * Adds the <code>BookSettings</code> for this workbook.
     *
      * @param book the <code>BookSettings</code> to add
      */
    public void addSettings(BookSettings book) throws IOException {

        int index = 0;
        ArrayList<SheetSettings> sheetSettings = book.getSheetSettings();
        String activeSheetName = book.getActiveSheet();

        for(Iterator<Worksheet> e = worksheets.iterator();e.hasNext();) {
            Worksheet ws = e.next();
            String name = getSheetName(index++);
            if(activeSheetName.equals(name)) {
                win1.setActiveSheet(index-1);
            }
            for(Iterator<SheetSettings> eSettings = sheetSettings.iterator();eSettings.hasNext();) {
                SheetSettings s = eSettings.next();
                if(name.equals(s.getSheetName())) {
                    ws.addSettings(s);
                }
            }
        }
    }

    /**
     * Return the filename of the pxl document without the file extension
     *
     * @return filename without the file extension
     */
    public String getName() {

        // We have to strip off the file extension
        int end = fileName.lastIndexOf(".");
        String name;
        if( end >= 0)   // check in case the filename is already stripped
            name = fileName.substring(0, end);
        else
            name = fileName;

        return name;
    }

    /**
     * Returns the filename of the pxl document with the file extension
     *
     * @return filename with the file extension
     */
    public String getFileName() {

        return fileName;
    }

}
