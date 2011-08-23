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
import java.awt.Point;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;
import org.openoffice.xmerge.converter.xml.sxc.SheetSettings;

/**
 * Represents a BIFF Record that describes the number and position of unfrozen
 * panes.
 */
public class Pane implements BIFFRecord {

    private byte[] x		= new byte[2];
    private byte[] y		= new byte[2];
    private byte[] rwTop	= new byte[2];
    private byte[] colLeft	= new byte[2];
    private byte   pnnAcct;
   
       /**
     * Default Constructor 
     */
    public Pane() {
        pnnAcct = (byte) 0x02;	// Default setting
    }
    
    /**
      * Constructs a Pane Record from the <code>InputStream</code>
      *
      * @param	is InputStream containing a Pane record 
      */
    public Pane(InputStream is) throws IOException {
        read(is);	
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>Pane</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.PANE_INFO;
    }
    
    /**
     * Gets the split point for this pane, in the case of splits this will be
     * in twips.
     *
     * @return the split point
     */
    public Point getSplitPoint() {

        int xTwips = EndianConverter.readShort(x)/11;
        int yTwips = EndianConverter.readShort(y)/15;
        return (new Point(xTwips, yTwips));
    }

    /**
     * Gets the freeze point for this pane, in the case of freezes this will
     * be a zero-based index to either the column or row.
     *
     * @return the freeze point
     */
    public Point getFreezePoint() {

        return (new Point(EndianConverter.readShort(x),
        EndianConverter.readShort(y)));
    }
    
    /**
     * Sets the split point for this pane, coordinates are in column row units
     * if the split type is freeze or twips if split type is split.
     *
     * @param splitType contains the X and Y split types (freeze or split)
     * @param p the split point
     */
    public void setSplitPoint(Point splitType, Point p) {
    
        if(splitType.getX()==SheetSettings.SPLIT 
            || splitType.getY()==SheetSettings.SPLIT) {
            int yTwips = (int) p.getY();
            short yPxl = (short) (yTwips * 15);
            y = EndianConverter.writeShort(yPxl);
            int xTwips = (int) p.getX();
            short xPxl = (short) (xTwips * 11);
            x = EndianConverter.writeShort(xPxl);
        } else {
            y = EndianConverter.writeShort((short) p.getY());
            x = EndianConverter.writeShort((short) p.getX());
        }

    } 	
    
    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>Pane</code>
     */
    public void setPaneNumber(int paneNumber) {
        pnnAcct = (byte) paneNumber;
    }
    
    /**
     * Get the pane number of the active pane 
     * 0 - bottom right, 1 - top right
     * 2 - bottom left, 3 - top left
     *
     * @return the hex code for <code>Pane</code>
     */
    public int getPaneNumber() {
        return pnnAcct;
    }	

    /**
     * Set the top row visible in the lower pane 
     *
     * @param top 0-based inex of the top row
     */
    public void setTop(int top) {
        rwTop = EndianConverter.writeShort((short)top);
    }
    
    /**
     * Set leftmost column visible in the right pane 
     *
     * @param left 0-based index of the leftmost column
     */
    public void setLeft(int left) {
        colLeft = EndianConverter.writeShort((short)left);
    }
    
    /**
     * Get the top row visible in the lower pane     
     *
     * @return the hex code for <code>Pane</code>
     */
    public int getTop() {
        return EndianConverter.readShort(rwTop);
    }
    
    /**
     * Get leftmost column visible in the right pane 
     *
     * @return 0-based index of the column 
     */
    public int getLeft() {
        return EndianConverter.readShort(colLeft);
    }


    /** 
     * Reads a <code>Pane</code> record from the <code>InputStream</code>
     *
     * @param input <code>InputStream</code> to read from 
     * @return the total number of bytes read
     */
    public int read(InputStream input) throws IOException {
        
        int numOfBytesRead	= input.read(x);
        numOfBytesRead		+= input.read(y);
        numOfBytesRead		+= input.read(rwTop);
        numOfBytesRead		+= input.read(colLeft);
        pnnAcct				= (byte) input.read();
        numOfBytesRead++;
        
        Debug.log(Debug.TRACE, "\tx : "+ EndianConverter.readShort(x) + 
                            " y : " + EndianConverter.readShort(y) +
                            " rwTop : " + EndianConverter.readShort(rwTop) +
                            " colLeft : " + EndianConverter.readShort(colLeft) +
                            " pnnAcct : " + pnnAcct);
        
        return numOfBytesRead;                
    }
    
    public void write(OutputStream output) throws IOException {

        output.write(getBiffType());
        output.write(x);
        output.write(y);
        output.write(rwTop);
        output.write(colLeft);
        output.write(pnnAcct);

        Debug.log(Debug.TRACE,"Writing Pane record");
    }
}
