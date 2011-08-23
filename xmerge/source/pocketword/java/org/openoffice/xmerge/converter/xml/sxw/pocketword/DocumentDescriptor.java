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

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import org.openoffice.xmerge.util.EndianConverter;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.IOException;

import java.util.Vector;


/**
 * This class to represent the data structure stored by a Pocket Word file that 
 * describes that file. 
 *
 * The data structure is of variable length, beginning at the end of the
 * font declarations and ending 10 bytes before the first instance of 0xFF 0xFF
 * marking a paragraph block.
 *
 * The variable length component arises from an 8 byte structure describing each
 * paragraph in the document.  These paragraph descriptors appear at the end
 * of the Document Descriptor.
 *
 * @author  Mark Murnane
 * @version 1.1 
 */
class DocumentDescriptor {
    private short numParagraphs = 0;
    private short length = 0;
    private short numLines = 0;
    
    private Vector paragraphDesc = null;   
    
    DocumentDescriptor() {
        paragraphDesc = new Vector(0, 1);
    }
    
    
    
    /**
     * Updates the <code>DocumentDescriptor</code> to include details of another 
     * paragraph in the document.
     *
     * @param   len     The number of characters in the paragraph.
     * @param   lines   The number of lines on screen that the paragraph uses.
     */
    public void addParagraph(short len, short lines) {
        ParagraphDescriptor pd = new ParagraphDescriptor(len, lines);
        
        paragraphDesc.add(pd);
        numParagraphs++;
        numLines += lines;
        length += pd.length;
    }
    
    
    /**
     * Retrieve the <code>DocumentDescriptor's</code> data.  Due to the variable 
     * length nature of the descriptor, certain fields can only be 
     * calculated/written after the addition of all paragraphs.
     *
     * @return  Byte array containing the Pocket Word representation of this 
     *          <code>DocumentDescriptor</code>.
     */
    public byte[] getDescriptor () {       
        ByteArrayOutputStream descStream = new ByteArrayOutputStream();
        
        writeHeader(descStream);
        
        /* 
         * This value seems to increment by 0x02 for each paragraph.
         * For a single paragraph doc, the value is 0x08, 0x0A for two, 
         * 0x0C for three ...
         */
        try {
            descStream.write(EndianConverter.writeShort((short)(6 + 
                                                        (numParagraphs * 2))));
        
            descStream.write(EndianConverter.writeShort(numParagraphs));
            descStream.write(EndianConverter.writeShort((short)0));
            descStream.write(EndianConverter.writeShort(numParagraphs));
            
            descStream.write(EndianConverter.writeShort((short)0));
            descStream.write(EndianConverter.writeShort((short)length));
            descStream.write(EndianConverter.writeShort((short)0));
        
            descStream.write(EndianConverter.writeShort(numLines));
            descStream.write(new byte[] { 0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00 } );
                                          
            for (int i = 0; i < paragraphDesc.size(); i++) {
                ParagraphDescriptor pd = (ParagraphDescriptor)paragraphDesc.elementAt(i);
            
                descStream.write(pd.getDescriptor());
            }
        
            // Byte sequence marking the end of this DocumentDescriptor
            descStream.write(EndianConverter.writeShort((short)0));
            descStream.write(EndianConverter.writeShort((short)0x41));
        }
        catch (IOException ioe) {
            // Should never happen as this is a memory based stream.
        }
        
        return descStream.toByteArray();
    }
    

    /*
     * This method loads the intial fixed portion of the descriptor and the
     * mid-section.  The mid-section is variable but Pocket Word doesn't seem
     * to mind default values.
     */  
    private void writeHeader(OutputStream descStream) {
       
        try {
            descStream.write(new byte[] { 0x00, 0x00, 0x00, 0x00, 
                                          0x07, 0x00, 0x06, 0x00,
                                          0x15, 0x00, 0x10, 0x00, 
                                          0x01, 0x00, (byte)0xD0, 0x2F,
                                          0x00, 0x00, (byte)0xE0, 0x3D, 
                                          0x00, 0x00, (byte)0xF0, 0x00,
                                          0x00, 0x00, (byte)0xA0, 0x05, 
                                          0x00, 0x00, (byte)0xA0, 0x05,
                                          0x00, 0x00, (byte)0xA0, 0x05, 
                                          0x00, 0x00, (byte)0xA0, 0x05,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x0A, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x04, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x0A, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x04, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x08, 0x00,
                                          0x07, 0x00, 0x10, 0x00, 
                                          0x01, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x12, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x1F, 0x04, 0x00, 0x00 } );
                                      
            /* 
             * The next four bytes are variable, but a pattern hasn't yet been
             * established.  Pocket Word seems to accept this constant value.
             *
             * The bytes are repeated after another 12 byte sequence which does
             * not seem to change from one file to the next.
             */
            descStream.write(new byte[] { (byte)0xE2, 0x02, 0x00, 0x00 } );
            descStream.write(new byte[] { 0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x3D, 0x04, 0x00, 0x00 } );
            descStream.write(new byte[] { (byte)0xE2, 0x02, 0x00, 0x00 } );
            
            descStream.write(new byte[] { 0x00, 0x00, 0x00, 0x00, 
                                          0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00,
                                          0x40, 0x00, 0x08, 0x00 } );
        }
        catch (IOException ioe) {
            /* Shouldn't happen with a ByteArrayOutputStream */
        }
    }
    
    
    /**
     * <code>ParagraphDescriptor</code> represents the data structure used to 
     * describe individual paragraphs within a <code>DocumentDescriptor.</code>
     *
     * It is used solely by the <code>DocumentDescriptor<code> class.
     */
    private class ParagraphDescriptor {
        private short filler  = 0;
        private short lines   = 0;
        private short length  = 0;
        private short unknown = 0x23;
        
        public ParagraphDescriptor(short len, short numLines) {
            lines = numLines;
            length = (short)(len + 1);
        }
        
        public byte[] getDescriptor() {
            ByteArrayOutputStream desc = new ByteArrayOutputStream();
            
            try {                
                desc.write(EndianConverter.writeShort(filler));
                desc.write(EndianConverter.writeShort(lines));
                desc.write(EndianConverter.writeShort(length));
                desc.write(EndianConverter.writeShort(unknown));
            }
            catch (IOException ioe) {
                /* Should never happen */
            }
            
            return desc.toByteArray();
        }
    }
}