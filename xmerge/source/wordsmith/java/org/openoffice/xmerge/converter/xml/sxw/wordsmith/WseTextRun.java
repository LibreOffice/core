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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import org.openoffice.xmerge.converter.xml.TextStyle;
import org.openoffice.xmerge.converter.xml.StyleCatalog;
import java.awt.Color;

/**
 *  <p>This class represents a text run (aka text atom) in a WordSmith
 *  document.</p>
 *
 *  <p>WordSmith represents a text run as follows:</p>
 *
 *  <p><ul><li>
 *    1 byte   Value of "1", indicating beginning of a text atom
 *  </li><li>
 *    2 bytes  Length of text (does not include attributes, this length field,
 *             etc)
 *  </li><li>
 *    1 byte   Font index - Index in the font table of font to be used
 *  </li><li>
 *    1 byte   Font size (DJP: get details of representation)
 *  </li><li>
 *    1 byte   Color index - Index in the color table of font color to be used
 *  </li><li>
 *    1 byte   Modifiers - bit flags for bold, italic, etc
 *  </li><li>
 *    n bytes  Text - the actual text
 *  </li></ul></p>
 *
 */
class WseTextRun extends Wse {

    /**  Font specifier.  This is an index into the font table. */
    private byte fontIndex = 0;
    private String fontName = null;

    /**  Size of the font. */
    private byte fontSize = 0;

    /**
     *  Color of the font.  This is an index into the color table.
     *  High nibble is background color index, low nibble is font color
     *  index.
     */
    private byte colorIndex = 0;

    /**
     *  Reference to color table for color lookups.
     */
    private WseColorTable ct;

    /**
     *  The modifiers for the text run.  (Mostly) Bitwise flags. The "_TOKEN"
     *  values are not yet implemented in this converter.  They may not even
     *  be implemented in WordSmith yet.
     */
    private byte modifiers = 0;
    final public static int BOLD             = 0x01;
    final public static int ITALIC           = 0x02;
    final public static int UNDERLINE        = 0x04;
    final public static int STRIKETHRU       = 0x08;
    final public static int SUPERSCRIPT      = 0x10;
    final public static int SUBSCRIPT        = 0x20;
    final public static int LINK             = 0x40;
    final public static int CUSTOM_TOKEN     = 0x80;
    final public static int IMAGE_TOKEN      = 0x80;
    final public static int BOOKMARK_TOKEN   = 0x81;
    final public static int ANNOTATION_TOKEN = 0x82;
    final public static int LINK_TOKEN       = 0x83;

    /**  The actual text. */
    private String text;

    StyleCatalog sc;


    /**
     *  Constructor for use when going from DOM to WordSmith.
     *
     *  @param txt  The text.
     *  @param t    The text style.
     *  @param sc   The <code>StyleCatalog</code>.
     *  @param ft   The font table.
     *  @param ct   The color Table.
     */
    public WseTextRun(String txt, TextStyle t, StyleCatalog sc,
                      WseFontTable ft, WseColorTable ct) {

        this.sc = sc;
        this.ct = ct;

        TextStyle ts = (TextStyle)t.getResolved();

        if (ts.isSet(TextStyle.BOLD) && ts.getAttribute(TextStyle.BOLD))
            modifiers |= BOLD;
        if (ts.isSet(TextStyle.ITALIC) && ts.getAttribute(TextStyle.ITALIC))
            modifiers |= ITALIC;
        if (ts.isSet(TextStyle.UNDERLINE) && ts.getAttribute(TextStyle.UNDERLINE))
            modifiers |= UNDERLINE;
        if (ts.isSet(TextStyle.STRIKETHRU) && ts.getAttribute(TextStyle.STRIKETHRU))
            modifiers |= STRIKETHRU;
        if (ts.isSet(TextStyle.SUPERSCRIPT) && ts.getAttribute(TextStyle.SUPERSCRIPT))
            modifiers |= SUPERSCRIPT;
        if (ts.isSet(TextStyle.SUBSCRIPT) && ts.getAttribute(TextStyle.SUBSCRIPT))
            modifiers |= SUBSCRIPT;

        fontSize = (byte)(ts.getFontSize() * 2);
        fontName = ts.getFontName();
        fontIndex = (byte)ft.getFontIndex(fontName);
        if (fontIndex == -1) {
            ft.add(fontName);
            fontIndex = (byte)ft.getFontIndex(fontName);
        }

        // Figure out the color index.
        Color c = t.getFontColor();
        if (c == null)
            c = Color.black;
        colorIndex = (byte)ct.findColor(c, true);
        c = t.getBackgroundColor();
        if (c == null)
            c = Color.white;
        colorIndex |= (byte)(ct.findColor(c, false) << 4);

        text = txt;
    }


    /**
     *  Standard constructor for use when going from WordSmith to DOM.
     *
     *  @param dataArray   <code>byte</code> array.
     *  @param startIndex  The start index.
     *  @param ft          The font table.
     *  @param ct          The color table.
     */
    public WseTextRun(byte dataArray[], int startIndex, WseFontTable ft,
                      WseColorTable ct) {

        this.ct = ct;

        startIndex++;  // Skip the leading "1"

        int textLen = ((dataArray[startIndex] << 8)
                      | (dataArray[startIndex+1] & 0xFF));
        startIndex += 2;

        fontIndex = dataArray[startIndex++];
        if (ft != null)
            fontName = ft.getFontName(fontIndex);

        fontSize = dataArray[startIndex++];

        colorIndex = dataArray[startIndex++];
        modifiers = dataArray[startIndex++];

        text = new String(dataArray, startIndex, textLen);
        startIndex += textLen;  // skip the text
    }


    /**
     *  Given a <code>byte</code> sequence, assumed to be a text run,
     *  compute the index of the first byte past the text run.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index
     *
     *  @return  The index of the first <code>byte</code> past the
     *           text run.
     */
    public static int computeNewIndex(byte dataArray[], int startIndex) {

        startIndex++;  // Skip the leading "1"

        int textLen = ((dataArray[startIndex] << 8)
        | (dataArray[startIndex+1] & 0xFF));
        startIndex += 2;

        startIndex += 4;  // skip attributes
        //        text = new String(dataArray, startIndex, textLen);
        startIndex += textLen;  // skip the text
        return startIndex;
    }


    /**
     *  Return true if the sequence starting at
     *  <code>dataArray[startIndex]</code> is a valid text run.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     *
     *  @return  true if the sequence starting at
     *           <code>dataArray[startIndex]</code> is a valid
     *           text run, false otherwise.
     */
    public static boolean isValid(byte dataArray[], int startIndex) {
        return (dataArray[startIndex] == 1);
    }

    /**
     *  Return the number of bytes needed to represent this text run.
     *
     *  @return  The number of bytes needed to represent this text run.
     */
    int getByteCount() {
        return text.length() + 7;
    }


    /**
     *  Return an <code>byte</code> array representing this text run.
     *
     *  @return  An <code>byte</code> array representing this text run.
     */
    byte[] getBytes() {
        short textLen = (short)text.length();
        byte b[] = new byte[textLen + 7];
        b[0] = 1;
        b[1] = (byte)(textLen >> 8);
        b[2] = (byte)(textLen & 0xFF);
        b[3] = fontIndex;
        b[4] = fontSize;
        b[5] = colorIndex;
        b[6] = modifiers;
        byte[] txtBytes = text.getBytes();
        System.arraycopy(txtBytes, 0, b, 7, textLen);
        return b;
    }


    /**
     *  Return the text of this run.
     *
     *  @return  The text of this run.
     */
    public String getText() {
        return text;
    }


    /**
     *  Return a <code>TextStyle</code> that reflects the formatting
     *  of this run.
     *
     *  @return  A <code>TextStyle</code> that reflects the formatting
     *           of this run.
     */
    public TextStyle makeStyle() {
        int mod = 0;
        if ((modifiers & BOLD) != 0) mod |= TextStyle.BOLD;
        if ((modifiers & ITALIC) != 0) mod |= TextStyle.ITALIC;
        if ((modifiers & UNDERLINE) != 0) mod |= TextStyle.UNDERLINE;
        if ((modifiers & STRIKETHRU) != 0)
            mod |= TextStyle.STRIKETHRU;
        if ((modifiers & SUPERSCRIPT) != 0) mod |= TextStyle.SUPERSCRIPT;
        if ((modifiers & SUBSCRIPT) != 0) mod |= TextStyle.SUBSCRIPT;

        int mask = TextStyle.BOLD | TextStyle.ITALIC
        | TextStyle.UNDERLINE
        | TextStyle.STRIKETHRU | TextStyle.SUPERSCRIPT
        | TextStyle.SUBSCRIPT;

        TextStyle x = new TextStyle(null, "text", null, mask,
          mod, (fontSize/2), fontName, sc);

        // If color table is available, set the colors.
        if (ct != null) {
            Color fc = ct.getColor(colorIndex & 0xF, true);
            Color bc = ct.getColor(colorIndex >> 4, false);
            x.setColors(fc, bc);
        }

        return x;
    }


    /**
     * Display debug information.
     */
    public void dump() {
        System.out.print("TEXT RUN: fontIndex = " + fontIndex
        + " fontsize = " + fontSize
        + " colorIndex = " + colorIndex
        + " ");
        if ((modifiers & BOLD) != 0) System.out.print("BOLD,");
        if ((modifiers & ITALIC) != 0) System.out.print("ITALIC,");
        if ((modifiers & UNDERLINE) != 0) System.out.print("UNDERLINE,");
        if ((modifiers & STRIKETHRU) != 0) System.out.print("STRIKETHRU,");
        if ((modifiers & SUPERSCRIPT) != 0) System.out.print("SUPERSCRIPT,");
        if ((modifiers & SUBSCRIPT) != 0) System.out.print("SUBSCRIPT,");
        System.out.println("\n" + text);
    }
}

