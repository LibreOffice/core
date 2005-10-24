/************************************************************************
 *
 *  PocketWordConstants.java
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


package org.openoffice.xmerge.converter.xml.sxw.pocketword;


/**
 * Interface defining constants for Pocket Word attributes.
 *
 * @author  Mark Murnane
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
