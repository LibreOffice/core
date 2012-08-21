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

import java.awt.Color;

/**
 *  This class represents a color table in a WordSmith document.
 *
 *  @author   David Proulx
 */
class WseColorTable extends Wse {

    private Color fgColors[];
    private Color bgColors[];

    /**
     *  Constructor to use when going from DOM to WordSmith
     */
    public WseColorTable() {
        fgColors = new Color[16];
        bgColors = new Color[16];

        // Always need these two!
        fgColors[0] = Color.black;
        bgColors[0] = Color.white;

    }

    /**
     *  Constructor to use when going from WordSmith to DOM.
     *
     *  @param  dataArray  <code>byte</code> array.
     *  @param  i          The index.
     */
    public WseColorTable(byte dataArray[], int i) {
        fgColors = new Color[16];
        bgColors = new Color[16];

        i += 2;  // Skip leading "64" and table length field.
        for (int k = 0; k < 16; k++) {
            fgColors[k] = new Color(dataArray[i+1] & 0xFF,
                                    dataArray[i+2] & 0xFF,
                                    dataArray[i+3] & 0xFF);
            i += 4;
        }
        for (int k = 0; k < 16; k++) {
            bgColors[k] = new Color(dataArray[i+1] & 0xFF,
                                    dataArray[i+2] & 0xFF,
                                    dataArray[i+3] & 0xFF);
            i += 4;
       }

    }


    /**
     *  Compute the index of the first <code>byte</code> following the
     *  paragraph descriptor, assuming that
     *  <code>dataArray[startIndex]</code> is the beginning of a valid
     *  paragraph descriptor.
     *
     *  @param dataArray   <code>byte</code array.
     *  @param startIndex  The start index.
     *
     *  @return   The index of the first <code>byte</code> following the
     *            paragraph description.
     */
    static int computeNewIndex(byte dataArray[], int startIndex) {
        int tableLen = dataArray[startIndex + 1];
        tableLen &= 0xFF;  // eliminate problems with sign-extension
        return startIndex + tableLen + 2;
    }


    /**
     *  Return true if <code>dataArray[startIndex]</code> is the start
     *  of a valid paragraph descriptor.
     *
     *  @param dataArray   <code>byte</code> array.
     *  @param startIndex  Start index.
     *
     *  @return  true if <code>dataArray[startIndex]</code> is the start
     *           of a valid paragraph descriptor, false otherwise.
     */
    static boolean isValid(byte dataArray[], int startIndex) {
        try {
            if (dataArray[startIndex] != 64)
                return false;
            int len = dataArray[startIndex + 1];
            len &= 0xFF;  // eliminate problems with sign-extension
            int temp = dataArray[startIndex + len + 2];  // probe end of table
        } catch (ArrayIndexOutOfBoundsException e) {
            return false;
        }
        return true;
    }


    /**
     *  Return the number of bytes needed to represent this color table.
     *
     *  @return  The byte count.
     */
    int getByteCount() {
    return (32 * 4) + 1 + 1;
    }


    /**
     *  Return a <code>byte</code> array representing this color table.
     *
     *  @return  <code>bytes</code> array representing this color table.
     */
    byte[] getBytes() {
        byte[] b = new byte[(32 * 4) + 1 + 1];
        b[0] = 0x40;
        b[1] = (byte)128;
        int i = 2;
        // int indVal = 0xd8;
        int indVal = 0;

        for (int j = 0; j < 16; j++) {
            b[i++] = (byte)indVal++;
            if (fgColors[j] != null) {
                b[i++] = (byte)fgColors[j].getRed();
                b[i++] = (byte)fgColors[j].getGreen();
                b[i++] = (byte)fgColors[j].getBlue();
            } else {
                b[i++] = (byte)0;
                b[i++] = (byte)0;
                b[i++] = (byte)0;
            }
        }

        for (int j = 0; j < 16; j++) {
            b[i++] = (byte)indVal++;
            if (bgColors[j] != null) {
                b[i++] = (byte)bgColors[j].getRed();
                b[i++] = (byte)bgColors[j].getGreen();
                b[i++] = (byte)bgColors[j].getBlue();
            } else {
                b[i++] = (byte)0xFF;
                b[i++] = (byte)0xFF;
                b[i++] = (byte)0xFF;
            }
        }

        return b;
    }


    /**
     *  Return the index of the specified foreground or background
     *  <code>Color</code>.  (If the color is not already in the table,
     *  it will be added.)
     *
     *  Note that the implementation of this may include a "margin of
     *  error" to prevent the color table from being filled up too
     *  quickly.
     *
     *  @param c           The <code>Color</code>.
     *  @param foreground  true if foreground color, false if background
     *                    color
     *
     *  @return  The index of the specified foreground or background
     *          <code>Color</code>.
     *
     *  DJP: how to handle table overflow?
     */
    int findColor(Color c, boolean foreground) {

        Color colorArray[] = foreground ? fgColors : bgColors;

        for (int i = 0; i < 16; i++) {
            if (colorArray[i] != null) {
                if (colorArray[i].equals(c))
                    return i;
            }
            else
                break;  // hit a null entry - no more colors in table!
        }

        // Color was not found in the table.  Add it.
        for (int i = 0; i < 16; i++) {
            if (colorArray[i] == null) {
                colorArray[i] = c;
                return i;
            }
        }
        return 0;  // Default - we should never get here though.
    }


    /**
     *  Given an index, return the <code>Color</code> from the table.
     *
     *  @param index       The index
     *  @param foreground  true if foreground color, false if background
     *                     color
     *
     *  @return            The <code>Color</code> at the specified index.
     */
    Color getColor(int index, boolean foreground) {

        Color colorArray[] = foreground ? fgColors : bgColors;
        return colorArray[index];
    }
}

