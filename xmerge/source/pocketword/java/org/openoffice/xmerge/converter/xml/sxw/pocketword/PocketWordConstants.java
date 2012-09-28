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


/**
 * Interface defining constants for Pocket Word attributes.
 *
 * @version 1.1
 */
public interface PocketWordConstants {
    /** File extension for Pocket Word files. */
    public static final String FILE_EXTENSION = ".psw";

    /** Name of the default style. */
    public static final String DEFAULT_STYLE = "Standard";

    /** Family name for Paragraph styles. */
    public static final String PARAGRAPH_STYLE_FAMILY = "paragraph";

    /** Family name for Text styles. */
    public static final String TEXT_STYLE_FAMILY = "text";


    /**
     * Generic Pocket Word formatting code.
     *
     * Formatting codes are 0xEz, where z indicates the specific format code.
     */
    public static final byte FORMATTING_TAG = (byte)0xE0;

    /** Font specification tag.  The two bytes following inidicate which font. */
    public static final byte FONT_TAG = (byte)0xE5;

    /** Font size tag.  The two bytes following specify font size in points. */
    public static final byte FONT_SIZE_TAG = (byte)0xE6;

    /** Colour tag.  Two bytes following index a 4-bit colour table. */
    public static final byte COLOUR_TAG = (byte)0xE7;

    /** Font weight tag.  Two bytes following indicate weighting of font. */
    public static final byte FONT_WEIGHT_TAG = (byte)0xE8;

    /** Normal font weight value. */
    public static final byte FONT_WEIGHT_NORMAL = (byte)0x04;

    /** Fine font weight value. */
    public static final byte FONT_WEIGHT_FINE = (byte)0x01;

    /** Bold font weight value. */
    public static final byte FONT_WEIGHT_BOLD = (byte)0x07;

    /** Thick font weight value. */
    public static final byte FONT_WEIGHT_THICK = (byte)0x09;

    /** Italic tag.  Single byte following indicates whether italic is on. */
    public static final byte ITALIC_TAG = (byte)0xE9;

    /** Underline tag.  Single byte following indicates whether underline is on. */
    public static final byte UNDERLINE_TAG = (byte)0xEA;

    /** Strikethrough tag.  Single byte following indicates whether strikethrough is on. */
    public static final byte STRIKETHROUGH_TAG = (byte)0XEB;

    /** Highlighting tag.  Single byte following indicates whether highlighting is on. */
    public static final byte HIGHLIGHT_TAG = (byte)0xEC;

}
