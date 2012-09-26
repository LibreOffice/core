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

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;

import org.openoffice.xmerge.util.EndianConverter;


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
 * @version 1.1
 */
class DocumentDescriptor {
    private short numParagraphs = 0;
    private short length = 0;
    private short numLines = 0;

    private ArrayList<ParagraphDescriptor> paragraphDesc = null;

    DocumentDescriptor() {
        paragraphDesc = new ArrayList<ParagraphDescriptor>(0);
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
            descStream.write(EndianConverter.writeShort(length));
            descStream.write(EndianConverter.writeShort((short)0));

            descStream.write(EndianConverter.writeShort(numLines));
            descStream.write(new byte[] { 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00 } );

            for (int i = 0; i < paragraphDesc.size(); i++) {
                ParagraphDescriptor pd = paragraphDesc.get(i);

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
