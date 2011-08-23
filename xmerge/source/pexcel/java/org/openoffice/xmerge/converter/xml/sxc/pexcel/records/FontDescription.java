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
import java.io.UnsupportedEncodingException;
import java.awt.Color;

import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.util.ColourConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;


/**
 * Represents a BIFF Record descibing a font used 
 */
public class FontDescription implements BIFFRecord {

    private byte[] dwHeight		= new byte[2];
    private byte[] grbit		= new byte[2];
    private byte[] icvFore		= new byte[2];
    private byte[] bls			= new byte[2];
    private byte[] Reserved2	= new byte[2];
    private byte uls;
    private byte bFamily;
    private byte bCharSet;
    private byte Reserved3;
    private byte cch;
    private byte[] rgch;

    public static final int UNDERLINE	= 0x01;
    public static final int ITALIC		= 0x02;

    /**
      * Constructs a FontDescription from the bold italic and undelrine attributes  
      *
      * @param	italic		Italic attribute 	
      * @param	bold		Bold attribute 	
      * @param	underline	Underline attribute 	
      */
    public FontDescription(Format fmt) throws IOException {

        Debug.log(Debug.TRACE,"Building FontDescriptor based on Format : " + fmt);

        this.dwHeight	= EndianConverter.writeShort((short) (fmt.getFontSize()*20)); 
            
        grbit = new byte[] {(byte)0x00, (byte)0x00};
        bls = EndianConverter.writeShort((short) 400);
        uls = 0;
            
        if (fmt.getAttribute(Format.ITALIC))
            grbit[0] |= ITALIC;

        if (fmt.getAttribute(Format.BOLD))
            bls		= EndianConverter.writeShort((short) 700);

        if (fmt.getAttribute(Format.UNDERLINE))
            uls	|= UNDERLINE;


        bFamily		= 0;
        bCharSet	= 0;

        String fontName = fmt.getFontName();
        if(	!fontName.equals("Tahoma") && 
            !fontName.equals("Courier New")) { 
            // We will set our default font to be Tahoma
            fontName = new String("Tahoma");
        }

        cch = (byte) fontName.length();
        rgch = fontName.getBytes("UTF-16LE");

        Color foreground  = fmt.getForeground();
        if( foreground != null ) {
            ColourConverter cc = new ColourConverter(PocketExcelConstants.cLookup);
            icvFore = EndianConverter.writeShort(cc.convertFromRGB(foreground));
        } else {
            icvFore	= new byte[] {(byte)0xFF,(byte)0x00};
        }

        Reserved2		= EndianConverter.writeShort((short) 0);
        Reserved3		= 0;

    }

    /**
     * Tests if this font descriptor defines italic
     *
     * @return true if italic otherwise false
     */
    public boolean isItalic() {
    
        return (EndianConverter.readShort(grbit) == 2);
    }

    /**
     * Tests if this font descriptor defines underline 
     *
     * @return true if underline otherwise false
     */
    public boolean isUnderline() {

        return (uls == 1);
    }

    /**
     * Tests if this font descriptor defines bold 
     *
     * @return true if bold otherwise false
     */
    public boolean isBold() {

        return (EndianConverter.readShort(bls) == 700);
    }

    /**
     * Get the background color this format uses 
     *
     * @return the background color
     */
    public Color getForeground() {
        short rgb = EndianConverter.readShort(icvFore);
        Color c = null;
        if(rgb!=0xFF) {
            ColourConverter cc = new ColourConverter(PocketExcelConstants.cLookup);
            c = cc.convertToRGB(rgb);
        } 
        return c;
    }
    
    /**
     * Compares current font descriptor against one passed in
     *
     * @return true if attrbitues are the same 
     */
    public boolean compareTo(FontDescription rhs) {

        if(EndianConverter.readShort(icvFore) !=
        EndianConverter.readShort(rhs.icvFore)) 
            return false;
            
        if (EndianConverter.readShort(dwHeight) !=
        EndianConverter.readShort(dwHeight))
            return false;

        if (this.getFont() != rhs.getFont())
            return false;

        if (this.isBold() != rhs.isBold())
            return false;

        if (this.isUnderline() != rhs.isUnderline())
            return false;

        if (this.isItalic() != rhs.isItalic())
            return false;

        return true;
    }
    

    /**
      * Constructs a Font Description from the <code>InputStream</code> 
      *
      * @param	is InputStream containing a <code>FontDescription</code> 
      */
    public FontDescription(InputStream is) throws IOException {
        read(is);
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code> 
     *
     * @return the hex code for <code>FontDescription</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.FONT_DESCRIPTION;
    }
   
       /**
     * Get the Font size
     *
     */
    public int getFontSize() {
        return EndianConverter.readShort(dwHeight)/20;
    }

    /**
     * Get the font name
     *
     */
    public String getFont() {
    
        String name;

        try {
            name = new String(rgch, "UTF-16LE");
        } catch (UnsupportedEncodingException e){
            name = "Tahoma";
        }
        return name;
    }
   
    /**
      * Constructs a Font Description from the <code>InputStream</code> 
      *
      * @param	is InputStream containing a <code>FontDescription</code> 
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead	= input.read(dwHeight);
        numOfBytesRead		+= input.read(grbit);    
        numOfBytesRead		+= input.read(icvFore);        
        numOfBytesRead		+= input.read(bls);
        numOfBytesRead		+= input.read(Reserved2);
        uls 				= (byte) input.read();
        bFamily				= (byte) input.read();
        bCharSet			= (byte) input.read();    
        Reserved3			= (byte) input.read();
        cch					= (byte) input.read();
        numOfBytesRead += 5;
        
        rgch = new byte[cch*2];
        input.read(rgch, 0, cch*2);        
    
        Debug.log(Debug.TRACE,"\tdwHeight : "+ EndianConverter.readShort(dwHeight) + 
                            " grbit : " + EndianConverter.readShort(grbit) +
                            " bls : " + EndianConverter.readShort(bls) +
                            " uls : " + uls +
                            "\n\tFamily : " + bFamily +        
                            " bCharSet : " + bCharSet +
                            " cch : " + cch +
                            " rgch : " + new String(rgch,"UTF-16LE"));
                            
        return numOfBytesRead;
    }
    
    public void write(OutputStream output) throws IOException {
            
        output.write(getBiffType());
        output.write(dwHeight);
        output.write(grbit);
        output.write(icvFore);
        output.write(bls);
        output.write(Reserved2);
        output.write(uls);
        output.write(bFamily);
        output.write(bCharSet);
        output.write(Reserved3);
        output.write(cch);
        output.write(rgch);

        Debug.log(Debug.TRACE,"Writing FontDescription record");
    }
    
}
