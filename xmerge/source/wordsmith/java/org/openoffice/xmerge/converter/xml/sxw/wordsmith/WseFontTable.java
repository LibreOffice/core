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


/**
 *  <p>This class represents a font table in a WordSmith document.
 *  A font table is represented as follows:</p>
 *
 *  <p><blockquote>
 *   binary "3"<br>
 *   two-byte length of the table of strings which follows<br>
 *   string table (null-terminated strings) representing font names
 *  </blockquote></p>
 *
 */
class WseFontTable extends Wse {

    java.util.ArrayList<String> fontNames = new java.util.ArrayList<String>(10);


    /**
     *  Constructor for use when going from DOM to WordSmith.
     */
    public WseFontTable() {
    }


    /**
     *  Constructor for use when going from WordSmith to DOM.
     *
     *  @param  dataArray  <code>byte</code> array.
     *  @param  i          The index.
     */
    public WseFontTable(byte dataArray[], int i) {
        i++;
        int tableLen = ((dataArray[i] << 8) | (dataArray[i+1] & 0xFF));
        i += 2;
        while (tableLen > 0) {
            int j = 0;
            while (dataArray[i + j] != 0) j++;
            fontNames.add(new String(dataArray, i, j));
            tableLen -= (j + 1);
            i += (j + 1);
        }
    }


    /**
     *  Add a new font to the table.
     *
     *  @param  newFontName  The new font name.
     */
    public void add(String newFontName) {
        if (newFontName != null)
            fontNames.add(newFontName);
    }


    /**
     *  Return a font name from the table, or null if invalid index.
     *
     *  @param  index  The font name index.
     *
     *  @return  The font name.
     */
    public String getFontName(int index) {
        try {
            return fontNames.get(index);
        } catch (ArrayIndexOutOfBoundsException e) {
            return null;
        }
    }

    /**
     *  Return the index of a font name in the table, or -1 if not found.
     *
     *  @param fontName  The font name.
     *
     *  @return  The index of the font name, or -1 if not found.
     */
    public int getFontIndex(String fontName) {
        int len = fontNames.size();
        for (int i = 0; i < len; i++) {
            String name = fontNames.get(i);
            if (name.equals(fontName))
                return i;
        }
        return -1;
    }


    /**
     *  Compute the index of the first <code>byte</code> following the
     *  paragraph descriptor, assuming that
     *  <code>dataArray[startIndex]</code> is the beginning of a valid
     *  paragraph descriptor.
     *
     *  @param dataArray   <code>byte</code> array.
     *  @param startIndex  The start index.
     *
     *  @return  The index of the first <code>byte</code> following the
     *           paragraph description.
     */
    static int computeNewIndex(byte dataArray[], int startIndex) {
        startIndex++;  // Skip the leading "3"
        int tableLen = ((dataArray[startIndex] << 8) | (dataArray[startIndex+1] & 0xFF));
        tableLen &= 0xFFFF;  // eliminate problems with sign-extension
        return startIndex + tableLen + 2;
    }


    /**
     *  Return true if <code>dataArray[startIndex]</code> is the start of a
     *  valid paragraph descriptor.
     *
     *  @param  dataArray   <code>byte</code> string.
     *  @param  startIndex  Start index.
     *
     *  @return  true if <code>dataArray[startIndex]</code> is the start
     *           of a valid paragraph descriptor, false otherwise.
     */
    static boolean isValid(byte dataArray[], int startIndex) {
        try {
            if (dataArray[startIndex] != 3)
                return false;
            int len = ((dataArray[startIndex+1] << 8)
              | (dataArray[startIndex+2] & 0xFF));
            len &= 0xFFFF;  // eliminate problems with sign-extension

            if (dataArray[startIndex + len + 2] != 0)
                return false;
        } catch (ArrayIndexOutOfBoundsException e) {
            return false;
        }
        return true;
    }


    /**
     *  Return the number of bytes needed to represent this font table.
     *
     *  @return The number of bytes needed to represent this font table.
     */
    int getByteCount() {

        int length = 3;  // leading "3" plus 2 bytes for length.
        int nFonts = fontNames.size();
        for (int i = 0; i < nFonts; i++) {
            String name = fontNames.get(i);
            length += name.length() + 1;  // extra byte is for trailing "0"
        }
        return length;
    }

    /**
     *  Return a <code>byte</code> array representing this font table.
     *
     *  @return  An <code>byte</code> array representing this font table.
     */
    byte[] getBytes() {

        int length = getByteCount();
        int nFonts = fontNames.size();
        byte b[] = new byte[length];
        b[0] = 3;
        length -= 3;
        b[1] = (byte)(length >> 8);
        b[2] = (byte)(length & 0xFF);
        int indx = 3;
        for (int i = 0; i < nFonts; i++) {
            String name = fontNames.get(i);
            byte bname[] = name.getBytes();
            System.arraycopy(bname, 0, b, indx, bname.length);
            indx += bname.length;
            b[indx++] = 0;
        }
        return b;
    }
}

