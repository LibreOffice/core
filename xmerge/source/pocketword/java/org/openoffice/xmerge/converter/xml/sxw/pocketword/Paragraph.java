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

import java.awt.Color;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.converter.xml.ParaStyle;
import org.openoffice.xmerge.converter.xml.TextStyle;
import org.openoffice.xmerge.util.ColourConverter;
import org.openoffice.xmerge.util.EndianConverter;


/**
 * Represents a paragraph data structure within a Pocket Word document.
 *
 * @version 1.1
 */
class Paragraph implements PocketWordConstants {
    /*
     * The data elements of a Paragraph.
     *
     * As the 'unknown' values are not calculated they are declared static.
     * They are not declared final because they do have a calcuable value.
     */
    private static short unknown1 = 0x23;
    private short dataWords = 0;
    private short textLength = 0;
    private short lengthWithFormatting = 0;
    private short lines   = 0;

    private static final short marker  = (short)0xFFFF;
    private static int unknown2 = 0x22;          // May be two short values

    private short specialIndentation = 0;
    private short leftIndentation    = 0;
    private short rightIndentation   = 0;

    private byte bullets   = 0;
    private byte alignment = 0;

    private static int unknown3 = 0;

    // Will always have at least these formatting settings in each paragraph
    private short defaultFont = 2;          // Courier New for the time being
    private short defaultSize = 10;


    /*
     * Remaining elements assist in calculating correct values for the paragraph
     * representation.
     */

    private ArrayList<ParagraphTextSegment> textSegments = null;

    private ArrayList<LineDescriptor> lineDescriptors = null;

    private ParaStyle pStyle = null;

    private boolean isLastParagraph = false;


    /*
     * Private class constructor used by all constructors.  Ensures the proper
     * initialisation of the Vector storing the paragraph's text.
     */
    private Paragraph () {
        textSegments = new ArrayList<ParagraphTextSegment>();
    }


    /**
     * <p>Constructor for use when converting from SXW format to Pocket Word
     *    format.</p>
     *
     * @param   style   Paragraph style object describing the formatting style
     *                  of this paragraph.
     */
    public Paragraph (ParaStyle style) {
        this();

        lineDescriptors = new ArrayList<LineDescriptor>();
        pStyle = style;
    }


    /**
     * <p>Constructor for use when converting from Pocket Word format to SXW
     *    format.</p>
     *
     * @param   data    Byte array containing byte data describing this paragraph
     *                  from the Pocket Word file.
     */
    public Paragraph (byte[] data) {
        this();

        /*
         * Read in all fixed data from the array
         *
         * unknown1 appears at data[0] and data[1]
         */
        dataWords = EndianConverter.readShort(new byte[] { data[2], data[3] } );
        textLength = EndianConverter.readShort(new byte[] { data[4], data [5] } );
        lengthWithFormatting = EndianConverter.readShort(
                                                new byte[] { data[6], data[7] } );
        lines = EndianConverter.readShort(new byte[] { data[8], data [9] } );

        /*
         * The marker appears at data[10] and data[11].
         *
         * The value of unknown2 is at data[12], data[13], data[14] and data[15].
         */

        specialIndentation = EndianConverter.readShort(new byte[] { data[16], data[17] } );
        leftIndentation = EndianConverter.readShort(new byte[] { data[18], data [19] } );
        rightIndentation = EndianConverter.readShort(new byte[] { data[20], data [21] } );

        bullets = data[22];
        alignment = data[23];

        // The value of unknown3 is at data[24], data[25], data[26] and data[27].

        /*
         * The actual paragraph data is in the remainder of the byte sequence.
         *
         * Only the actual text seqence with the embedded formatting tags is
         * relevant to the conversion from Pocket Word to SXW format.
         */
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        bos.write(data, 28, lengthWithFormatting);
        parseText(bos.toByteArray());
    }


    /*
     * Processes the text portion of the raw paragraph data from the Pocket Word
     * file.  This data also includes formatting settings for the text in the
     * paragraph.
     *
     * Formatting changes appear like XML/HTML tags.  Formatted blocks are
     * preceded by a sequence of bytes switching on a formatting change and
     * followed by a sequence switching off that formatting change.
     */
    private void parseText (byte[] data) {

        int totalLength = data.length;

        StringBuffer sb = new StringBuffer("");

        // Setup text style information
        int mask = TextStyle.BOLD | TextStyle.ITALIC | TextStyle.UNDERLINE
                    | TextStyle.STRIKETHRU;


        String fontName = null;
        int fontSize = 0;
        Color textColour = null;
        Color backColour = null;
        int modifiers = 0;

        TextStyle ts = null;

        int attrsSet = 0;   // If this is 0, we have no extra style
        boolean inSequence = false;
        boolean sawText = false;

        // Start from the very beginning
        for (int i = 0; i < totalLength; i++) {
            // Will encounter at least two codes first
            if ((byte)(data[i] & 0xF0) == FORMATTING_TAG) {
                if (sawText) {
                    // Style change so dump previous segment and style info
                    addTextSegment(sb.toString(), ts);
                    sb = new StringBuffer("");
                    sawText = false;
                }

                switch (data[i]) {
                    case FONT_TAG:
                        int index = EndianConverter.readShort(
                                        new byte[] { data[i + 1], data[i + 2] } );

                        /*
                         * Standard font.
                         *
                         * Should really be one, but as the only supported font
                         * currently is Courier New, want to leave it at Courier
                         * New for round trip conversions.
                         *
                         * Also need to account for the fact that Tahoma is the
                         * correct standard font.
                         */
                        if (fontName == null || fontName.equals("2")) {
                            if (index != 2 && index != 1) {
                                fontName = String.valueOf(index);
                                attrsSet++;
                            }
                        }
                        else {
                            // Font is set, but not the default
                            if (index == 2 || index == 1) {
                                fontName = "2";
                                attrsSet--;
                            }
                            else {
                                fontName = String.valueOf(index);
                            }
                        }
                        i += 2;
                        break;


                    case FONT_SIZE_TAG:
                        int size = EndianConverter.readShort(
                                        new byte[] { data[i + 1], data[i + 2] } );

                        if (size == 0) {
                            // Flags the end of the last paragraph
                            isLastParagraph = true;
                            i += 2;
                            break;
                        }

                        // Standard size
                        if (fontSize == 0 || fontSize == 10) {
                            if (size != 10) {
                                fontSize = size;
                                attrsSet++;
                            }
                        }
                        else {
                            // Font size is set, but not to standard
                            if (size == 10) {
                                fontSize = 10;
                                attrsSet--;
                            }
                            else {
                                fontSize = size;
                            }
                        }
                        i += 2;
                        break;


                    case COLOUR_TAG:
                        if (data[i + 1] != 0) {
                            ColourConverter cc = new ColourConverter();
                            textColour = cc.convertToRGB(
                                EndianConverter.readShort(new byte[] { data[i + 1],
                                                                    data[i + 2] } ));
                            attrsSet++;
                        }
                        else {
                            textColour = null;
                            attrsSet--;
                        }
                        i += 2;
                        break;


                    case FONT_WEIGHT_TAG:
                        if (data[i + 1] == FONT_WEIGHT_BOLD
                                || data[i + 1] == FONT_WEIGHT_THICK) {
                            modifiers |= TextStyle.BOLD;
                            attrsSet++;
                        }
                        else {
                            // Its a bit field so subtracting should work okay.
                            modifiers ^= TextStyle.BOLD;
                            attrsSet--;
                        }
                        i += 2;
                        break;


                    case ITALIC_TAG:
                        if (data[i + 1] == (byte)0x01) {
                            modifiers |= TextStyle.ITALIC;
                            attrsSet++;
                        }
                        else {
                            modifiers ^= TextStyle.ITALIC;
                            attrsSet--;
                        }
                        i++;
                        break;


                    case UNDERLINE_TAG:
                        if (data[i + 1] == (byte)0x01) {
                            modifiers |= TextStyle.UNDERLINE;
                            attrsSet++;
                        }
                        else {
                            modifiers ^= TextStyle.UNDERLINE;
                            attrsSet--;
                        }
                        i++;
                        break;


                    case STRIKETHROUGH_TAG:
                        if (data[i + 1] == (byte)0x01) {
                            modifiers |= TextStyle.STRIKETHRU;
                            attrsSet++;
                        }
                        else {
                            modifiers ^= TextStyle.STRIKETHRU;
                            attrsSet--;
                        }
                        i++;
                        break;

                    case HIGHLIGHT_TAG:
                        /*
                         * Highlighting is treated by OpenOffice as a
                         * background colour.
                         */
                        if (data[i + 1] == (byte)0x01) {
                            backColour =  Color.yellow;
                            attrsSet++;
                        }
                        else {
                            backColour = null;
                            attrsSet--;
                        }
                        i++;
                        break;
                }

                inSequence = true;
                continue;
            }

            if (inSequence) {
                // Style information has been changed.  Create new style here

                inSequence = false;
                if (attrsSet > 0) {
                    ts = new TextStyle(null, TEXT_STYLE_FAMILY, DEFAULT_STYLE,
                                        mask, modifiers, fontSize, fontName, null);
                    ts.setColors(textColour, backColour);
                }
                else {
                    ts = null;
                }
            }

            /*
             * C4 xx seems to indicate a control code.  C4 00 indicates the end
             * of a paragraph; C4 04 indicates a tab space.  Only these two
             * have been seen so far.
             */
            if (data[i] == (byte)0xC4) {
                /*
                 * Redundant nodes are sometimes added to the last paragraph
                 * because a new sequence is being processed when the flag is
                 * set.
                 *
                 * To avoid this, do nothing with the last paragraph unless no
                 * text has been added for it already.  In that case, add the
                 * empty text segment being process to ensure that all
                 * paragraphs have at least one text segment.
                 */
                if (data[i + 1] == (byte)0x00) {
                    if (isLastParagraph && textSegments.size() > 0) {
                        return;
                    }
                    addTextSegment(sb.toString(), ts);
                    return;
                }
                sb.append("\t");
                sawText = true;
                i++;
                continue;
            }

            sb.append((char)data[i]);
            sawText = true;
        }
    }


    /**
     * <p>Adds details of a new text block to the <code>Paragraph</code> object.
     * </p>
     *
     * @param   text    The text of the new block.
     * @param   style   Text style object describing the formatting attached
     *                  to this block of text.
     */
    public void addTextSegment(String text, TextStyle style) {
        textLength += text.length();
        textSegments.add(new ParagraphTextSegment(text, style));
     }


    /**
     * <p>This method alters the state of the <code>Paragraph</code> object to
     *    indicate whether or not it is the final paragraph in the document.</p>
     *
     * <p>It is used during conversion from SXW format to Pocket Word format.
     *    In Pocket Word files, the last paragraph finishes with a different byte
     *    sequence to other paragraphs.</p>
     *
     * @param   isLast  true if the Paragraph is the last in the document,
     *                  false otherwise.
     */
    public void setLastParagraph(boolean isLast) {
        isLastParagraph = isLast;
    }


    /**
     * <p>Complementary method to {@link #setLastParagraph(boolean)
     *    setLastParagraph}.  Returns the terminal status of this
     *    <code>Paragraph</code> within the Pocket Word document.</p>
     *
     * @return  true if the Paragraph is the last in the document; false otherwise.
     */
    public boolean getLastParagraph () {
        return isLastParagraph;
    }


    /**
     * <p>This method returns the Pocket Word representation of this
     *    <code>Paragraph</code> in Little Endian byte order.</p>
     *
     * <p>Used when converting from SXW format to Pocket Word format.</p>
     *
     * @return  <code>byte</code> array containing the formatted representation
     *          of this Paragraph.
     */
    public byte[] getParagraphData() {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();

        postProcessText();

        /*
         * Need information about the paragraph segments in two places
         * so calculate them first.
         *
         * The stream contains the text wrapped in any formatting sequences that
         * are necessary.
         */
        ByteArrayOutputStream segs = new ByteArrayOutputStream();

        try {
            for (int i = 0; i < textSegments.size(); i++) {
                ParagraphTextSegment pts = textSegments.get(i);
                segs.write(pts.getData());
            }
        }
        catch (IOException ioe) {
            // Should never happen in a memory based stream
        }

        /*
         * Number of data words for this paragraph descriptor:
         *
         * 26 is the number of bytes prior to the start of the segment.
         * 3 comes from the C4 00 00 termintating sequence.
         */
        dataWords = (short)(26 + segs.size() + 3 + 4);
        if (isLastParagraph) {
            dataWords += 6;
        }
        if (dataWords % 4 != 0) {
            dataWords += (4 - (dataWords % 4));
        }
        dataWords /= 4;

        /*
         * The 8 bytes are made up of E6 ?0 00 and E5 ?0 00 at the start of the
         * text along with the C4 00 that terminates it.
         *
         * In the event that the paragraph is the last one E6 00 00 is also
         * present at the end of the text.  Also, as we currently use a font
         * other than the first in the index (Tahoma) E5 01 00 is also present.
         *
         * Make sure this is accurate when font specifications change
         */
        lengthWithFormatting = (short)(segs.size() + (isLastParagraph ? 14 : 8));

        try {
            bos.write(EndianConverter.writeShort(unknown1));
            bos.write(EndianConverter.writeShort(dataWords));
            bos.write(EndianConverter.writeShort((short)(textLength + 1)));
            bos.write(EndianConverter.writeShort(lengthWithFormatting));
            bos.write(EndianConverter.writeShort(lines));

            bos.write(EndianConverter.writeShort(marker));
            bos.write(EndianConverter.writeInt(unknown2));

            bos.write(EndianConverter.writeShort(specialIndentation));
            bos.write(EndianConverter.writeShort(leftIndentation));
            bos.write(EndianConverter.writeShort(rightIndentation));

            bos.write(bullets);

            if (pStyle != null && pStyle.isAttributeSet(ParaStyle.TEXT_ALIGN)) {
                switch (pStyle.getAttribute(ParaStyle.TEXT_ALIGN)) {

                    case ParaStyle.ALIGN_RIGHT:
                        bos.write(0x01);
                        break;

                    case ParaStyle.ALIGN_CENTER:
                        bos.write(0x02);
                        break;

                    default:
                        bos.write(0x00);    // Left align in all other circumstances
                        break;
                }
            }
            else {
                bos.write(0x00);
            }

            bos.write(EndianConverter.writeInt(unknown3));


            /*
             * Write out font and size.
             *
             * If font support is added then this should change as the information
             * will have to be calculated from a Font table.
             */
            bos.write(FONT_TAG);
            bos.write(EndianConverter.writeShort(defaultFont));
            bos.write(FONT_SIZE_TAG);
            bos.write(EndianConverter.writeShort(defaultSize));

            // Write out the text segments
            bos.write(segs.toByteArray());

            /*
            * If this is the last paragraph in the document then we need to make
            * sure that the paragraph text is terminated correctly with an E6 00 00
            * before the C4 00 00.
            */
            if (isLastParagraph) {
                if (defaultFont != 1) {
                    // Must always go back to the first font.
                    bos.write(FONT_TAG);
                    bos.write(EndianConverter.writeShort((short)0x01));
                }
                bos.write(FONT_SIZE_TAG);
                bos.write(EndianConverter.writeShort((short)0x00));
            }

            bos.write(new byte[] { (byte)0xC4, 0x00, 0x00 } );

            int padding = 0;
            if (bos.size() % 4 != 0) {
                padding = 4 - (bos.size() % 4);
            }
            for (int i = 0; i < padding; i++) {
                bos.write(0x00);
            }

            // Third byte should match first byte after 0xFF 0xFF
            bos.write(new byte[] { 0x42, 0x00, 0x22, 0x00} );

            /*
            * Meaning of last two bytes seems to be the number of words describing
            * lines.  This is calculated at 10 bytes per descriptor.
            *
            * May have two extra padding bytes that need to be accounted for too
            * The division below may lose 2 bytes (integer result).
            */
            int wordsRemaining = (lineDescriptors.size() * 10) / 4;
            if ((lineDescriptors.size() * 10) % 4 != 0) {
                wordsRemaining++;
            }
            bos.write(EndianConverter.writeShort((short)wordsRemaining));


            // Now write out the line descriptors
            for (int i = 0; i < lineDescriptors.size(); i++) {
                LineDescriptor ld = lineDescriptors.get(i);

                bos.write(ld.getDescriptorInfo());
            }


            if (!isLastParagraph) {
                /*
                 * There may be a need to pad this.  Will be writing at
                 * either start of 4 byte block or 2 bytes into it.
                 */
                if (bos.size() % 4 != 2) {
                    bos.write(EndianConverter.writeShort((short)0));
                }
                bos.write(EndianConverter.writeShort((short)0x41));
            }
        }
        catch (IOException ioe) {
            // Should never occur for a memory based stream
        }

        return bos.toByteArray();
    }


    /*
     * This method handles the calculation of correct values for line lengths
     * in each individual descriptor and the number of lines in the document.
     *
     * TODO: Update to take account of different font metrics.
     */
    private void postProcessText() {
        /*
         * The post-processing ...
         *
         * For each line, we need to add a line descriptor and increment
         * the number of lines in the paragraph data structure.
         *
         * To do this, make sure that no sequence goes over the given screen
         * width unless the last char is a whitespace character.
         */

        // In courier, can have no more than 29 chars per line

        int chunkStart = 0;
        StringBuffer sb = new StringBuffer("");

        // Line Descriptor info should be eliminated each time
        lineDescriptors = new ArrayList<LineDescriptor>();
        lines = 0;

        for (int i = 0; i < textSegments.size(); i++) {
            ParagraphTextSegment pts = textSegments.get(i);
            sb.append(pts.getText());
        }

        if (sb.length() == 0) {
            lines = 1;
            lineDescriptors.add(new LineDescriptor((short)1, (short)0));
            return;
        }

        while (chunkStart < sb.length()) {
            String text = "";

            try {
                text = sb.substring(chunkStart, chunkStart + 30);
            }
            catch (StringIndexOutOfBoundsException sioobe) {
                // We have less than one line left so just add it
                text = sb.substring(chunkStart);
                lineDescriptors.add(new LineDescriptor((short)(text.length() + 1), (short)(text.length() * 36)));
                chunkStart += text.length();
                lines++;
                continue;
            }

            int lastWhitespace = -1;

            for (int i = 29; i >= 0; i--) {
                if (Character.isWhitespace(text.charAt(i))) {
                    lastWhitespace = i;
                    break;
                }
            }

            if (lastWhitespace != -1) {
                // The line can be split
                lineDescriptors.add(new LineDescriptor((short)(lastWhitespace + 1), (short)(lastWhitespace  * 36)));
                chunkStart += lastWhitespace + 1;
                lines++;
            }
            else {
                // The line is completely occupied by a single word
                lineDescriptors.add(new LineDescriptor((short)29, (short)(29 * 36)));
                chunkStart += 29;
                lines++;
            }
        }
    }


    /**
     * <p>Returns the number of lines in the <code>Paragraph</code>.</p>
     *
     * @return  The number of lines in the document.
     */
    public short getLines() {
        postProcessText();

        return lines;
    }


    /**
     * <p>Toggles the flag indicating that the <code>Paragraph</code> is a
     *    bulleted paragraph.</p>
     *
     * @param   isBulleted  true to enable bulleting for this paragraph, false
     *                      otherwise.
     */
    public void setBullets(boolean isBulleted) {
        if (isBulleted) {
            bullets = (byte)0xFF;
        }
        else {
            bullets = 0;
        }
    }

    /**
     * <p>Returns the bulleting status of the <code>Paragraph</code>.</p>
     *
     * @return  true if the paragraph is bulleted, false otherwise.
     */
    public boolean isBulleted() {
        if (bullets != 0) {
            return true;
        }
        return false;
    }


    /**
     * <p>Returns the number of text characters in the <code>Paragraph</code>,
     *    excluding formatting.</p>
     *
     * @return  The length of the paragraph.
     */
    public int getTextLength () {
        return textLength;
    }


    /**
     * <p>Returns an <code>Enumeration</code> over the individual text segments
     *    of the <code>Paragraph</code>.</p>
     *
     * @return  An <code>Enumeration</code> of the text segments.
     */
    public Iterator<ParagraphTextSegment> getSegmentsEnumerator () {
        return textSegments.iterator();
    }


    /**
     * <p>Returns a paragraph style object that describes any of the paragraph
     *    level formatting used by this <code>Paragraph</code>.</p>
     *
     * @return  Paragraph style object describing the <code>Paragraph</code>.
     */
    public ParaStyle makeStyle() {
        int attrs[] = new int[] { ParaStyle.MARGIN_LEFT, ParaStyle.MARGIN_RIGHT,
                                  ParaStyle.TEXT_ALIGN };
        String values[] = new String[attrs.length];

        /*
         * Not interested in left or right indents just yet.  Don't know
         * how to calculate them.
         */

        switch (alignment) {
            case 2:
                values[2] = "center";
                break;

            case 1:
                values[2] = "right";
                break;

            case 0:
            default:
                values[2] = "left";
                return null;        // Not interested if its the default.
        }

        return new ParaStyle(null, PARAGRAPH_STYLE_FAMILY, null, attrs,
                                values, null);
    }


    /*
     * Class describing the data structures which appear following the text
     * of a Paragraph.  For each line on screen that the Paragraph uses, a
     * LineDescriptor details how many characters are on the line and how much
     * screen space they occupy.
     *
     * The screen space and character breaks are calculated during post-processing
     * of the paragraph.  See postProcessText().
     *
     * The unit of measurement used for screen space is currently unknown.
     */
    private class LineDescriptor {
        private short characters = 0;
        private int filler = 0;
        private short screen_space = 0;
        private short marker = 0;

        private LineDescriptor(short chars, short space) {
            characters = chars;
            screen_space = space;
            marker = (short)0x040C;    // Not a constant.  Depends on font used.
        }


        private byte[] getDescriptorInfo(){
            ByteArrayOutputStream bos = new ByteArrayOutputStream();

            try {
                bos.write(EndianConverter.writeShort(characters));
                bos.write(EndianConverter.writeInt(filler));
                bos.write(EndianConverter.writeShort(screen_space));
                bos.write(EndianConverter.writeShort(marker));
            }
            catch (IOException ioe) {
                // Should never happen in a memory based stream.
            }

            return bos.toByteArray();
        }
    }
}
