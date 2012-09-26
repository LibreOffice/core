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

package org.openoffice.xmerge.converter.xml.sxc;

import java.awt.Color;

/**
 *  This class specifies the format for a given spreadsheet cell.
 *
 */
public class Format implements Cloneable {

    /**  Horizontal Alignment Constants. */
    final public static int RIGHT_ALIGN     = 0x01;
    final public static int CENTER_ALIGN    = 0x02;
    final public static int LEFT_ALIGN      = 0x03;
    final public static int JUST_ALIGN      = 0x04;

    /**  Vertical Alignment Constants. */
    final public static int TOP_ALIGN       = 0x01;
    final public static int MIDDLE_ALIGN    = 0x02;
    final public static int BOTTOM_ALIGN    = 0x03;

    /** Indicates <i>bold</i> text. */
    final public static int BOLD        = 0x01;
    /** Indicates <i>italic</i> text. */
    final public static int ITALIC      = 0x02;
    /** Indicates <i>underlined</i> text. */
    final public static int UNDERLINE   = 0x04;
    /** Indicates <i>strike-through</i> in the text. */
    final public static int STRIKETHRU  = 0x08;
    /** Indicates <i>superscripted</i> text. */
    final public static int SUPERSCRIPT = 0x10;
    /** Indicates <i>subscripted</i> text. */
    final public static int SUBSCRIPT   = 0x20;

    final public static int LEFT_BORDER     = 0x40;
    final public static int RIGHT_BORDER    = 0x80;
    final public static int TOP_BORDER      = 0x100;
    final public static int BOTTOM_BORDER   = 0x200;

    final public static int WORD_WRAP       = 0x400;

    private int align;
    private int vertAlign;
    private String category;
    private String value;
    private String formatSpecifier;
    private int decimalPlaces;

    /** Font name. */
    private String fontName;
    /** Font size in points. */
    protected int sizeInPoints;

    private Color foreground, background;

    /** Values of text attributes. */
    protected int attributes = 0;
    /** Bitwise mask of text attributes. */
    protected int mask = 0;

    /**
     *  Constructor for creating a new <code>Format</code>.
     */
    public Format() {
        clearFormatting();
    }

    /**
     * Constructor that creates a new <code>Format</code> object
     * by setting all the format attributes.
     *
     * @param attributes  Attributes flags (alignment, bold, etc.)
     * @param fontSize    Size of the font in points.
     * @param fontName    Name of the font to use.
     */
       public Format(int attributes, int fontSize, String fontName) {

        this.attributes = attributes;
        sizeInPoints = fontSize;
        this.fontName = fontName;
    }

    /**
     *  Constructor for creating a new <code>Format</code> object
     *  based on an existing one.
     *
     *  @param  fmt  <code>Format</code> to copy.
     */
    public Format(Format fmt) {
        category = fmt.getCategory();
        value = fmt.getValue();
        formatSpecifier = fmt.getFormatSpecifier();
        decimalPlaces = fmt.getDecimalPlaces();

        attributes = fmt.attributes;
        mask = fmt.mask;

        fontName = fmt.getFontName();
        align = fmt.getAlign();
        vertAlign = fmt.getVertAlign();
        foreground = fmt.getForeground();
        background = fmt.getBackground();
        sizeInPoints = fmt.sizeInPoints;
    }

    /**
     *  Reset this <code>Format</code> description.
     */
    public void clearFormatting() {
       category = "";
       value = "";
       formatSpecifier = "";
       decimalPlaces = 0;
       attributes = 0;
       mask = 0;
       sizeInPoints = 10;
       align = LEFT_ALIGN;
       vertAlign = BOTTOM_ALIGN;
       fontName = "";
       foreground = null;
       background = null;
    }

    /**
     *  Set one or more text attributes.
     *
     *  @param  flags  Flag attributes to set.
     *  @param  toggle True to set flags, false to clear them.
     */
    public void setAttribute(int flags, boolean toggle) {
        mask |= flags;
        if(toggle) {
            attributes |= flags;
        } else {
            attributes &= ~flags;
        }
    }

    /**
     *  Return true if the <code>attribute</code> is set to <i>on</i>
     *
     *  @param  attribute  Attribute to check ({@link #BOLD},
     *                     {@link #ITALIC}, etc.)
     *
     *  @return  true if <code>attribute</code> is set to <i>on</i>,
     *           otherwise false.
     */
    public boolean getAttribute(int attribute) {
        if ((mask & attribute) == 0)
            return false;
        return (!((attributes & attribute) == 0));
    }

    /**
     *  Return true if text <code>attribute</code> is set in this
     *  <code>Style</code>.An attribute that is set may have a
     *  value of <i>on</i> or <i>off</i>.
     *
     *  @param  attribute  The attribute to check ({@link #BOLD},
     *                     {@link #ITALIC}, etc.).
     *
     *  @return  true if text <code>attribute</code> is set in this
     *           <code>Style</code>, false otherwise.
     */
    public boolean isSet(int attribute) {
        return (!((mask & attribute) == 0));
    }

    /**
     *  Set the formatting category of this object, ie number, date,
     *  currency.The <code>OfficeConstants</code> class contains string
     *  constants for the category types.
     *
     *  @see  org.openoffice.xmerge.converter.xml.OfficeConstants
     *
     *  @param   newCategory  The name of the category to be set.
     */
    public void setCategory(String newCategory) {
        category = newCategory;
    }

     /**
      *  Return the formatting category of the object.
      *
      *  @see org.openoffice.xmerge.converter.xml.OfficeConstants
      *
      *  @return  The formatting category of the object.
      */
     public String getCategory() {
         return category;
     }

     /**
      *  In the case of Formula returns the value of the formula.
      *
      *  @return  The value of the formula
      */
     public String getValue() {
         return value;
     }

     /**
     *  In the case of formula the contents are set as the formula string and
     *  the value of the formula is a formatting attribute.
     *
     *  @param   newValue the formuala value
     */
    public void setValue(String newValue) {
        value = newValue;
    }

     /**
      *  Set the <code>Format</code> specifier for this category.
      *
      *  @param  formatString  The new <code>Format</code> specifier.
      */
     public void setFormatSpecifier(String formatString) {
         formatSpecifier = formatString;
     }

     /**
      *  Get the <code>Format</code> specifier for this category.
      *
      *  @return  <code>Format</code> specifier for this category.
      */
     public String getFormatSpecifier() {
         return formatSpecifier;
     }

     /**
      *  Set the precision of the number to be displayed.
      *
      *  @param  precision  The number of decimal places to display.
      */
     public void setDecimalPlaces(int precision) {
         decimalPlaces = precision;
     }

     /**
      *  Get the number of decimal places displayed.
      *
      *  @return  Number of decimal places.
      */
     public int getDecimalPlaces() {
         return decimalPlaces;
     }

     /**
      *  Set the font used for this cell.
      *
      *  @param  fontName  The name of the font.
      */
     public void setFontName(String fontName) {
         this.fontName = fontName;
     }

     /**
      *  Get the font used for this cell.
      *
      *  @return  The font name.
      */
     public String getFontName() {
         return fontName;
     }

     /**
      *  Set the font size (in points) used for this cell.
      *
      *  @param  fontSize  The font size in points.
      */
     public void setFontSize(int fontSize) {
         sizeInPoints = fontSize;
     }

     /**
      *  Get the font size (in points) used for this cell.
      *
      *  @return  The font size in points.
      */
     public int getFontSize() {
         return sizeInPoints;
     }

      /**
      *  Set the vertical alignment used for this cell.
      *
      *  @param  vertAlign  The vertical alignment.
      */
     public void setVertAlign(int vertAlign) {
         this.vertAlign = vertAlign;
     }

     /**
      *  Get the vertical alignment used for this cell.
      *
      *  @return  The vertical alignment.
      */
     public int getVertAlign() {
         return vertAlign;
     }

      /**
      *  Set the alignment used for this cell.
      *
      *  @param  align The alignment to use.
      */
     public void setAlign(int align) {
         this.align = align;
     }

     /**
      *  Get the alignment used for this cell.
      *
      *  @return  The alignment.
      */
     public int getAlign() {
         return align;
     }

     /**
      *  Set the Foreground <code>Color</code> for this cell.
      *
      *  @param  c  A <code>Color</code> object representing the
      *                 foreground color.
      */
     public void setForeground(Color c) {
         if(c!=null)
            foreground = new Color(c.getRGB());
     }

     /**
      *  Get the Foreground <code>Color</code> for this cell.
      *
      *  @return  Foreground <code>Color</code> value.
      */
     public Color getForeground() {
         return foreground;
     }

     /**
      *  Set the Background <code>Color</code> for this cell
      *
      *  @param  c  A <code>Color</code> object representing
      *                 the background color.
      */
     public void setBackground(Color c) {
         if(c!=null)
             background = new Color(c.getRGB());
     }

     /**
      *  Get the Background <code>Color</code> for this cell
      *
      *  @return  Background <code>Color</code> value
      */
     public Color getBackground() {
         return background;
     }

     /**
      *  Get a string representation of this <code>Format</code>
      *
      *  @return  A string indicating the value and category.
      */
     public String toString() {
         return new String("Value : " + getValue() + " Category : " + getCategory());
     }

    /**
     * Tests if the current <code>Format</code> object has default attribute
     * values.
     *
     * @return true if it contains default value
     */
    public boolean isDefault() {

        Format rhs = new Format();

        if (rhs.attributes!= attributes)
                return false;

        if (foreground!=rhs.foreground)
            return false;

        if (background!=rhs.background)
            return false;

        if (rhs.align!= align)
                return false;

        if (rhs.vertAlign!= vertAlign)
                return false;

        return true;
    }

    /**
     *  Return true if passed <code>Format</code> specifies as much or less
     *  than this <code>Format</code>, and nothing it specifies
     *  contradicts this <code>Format</code>.
     *
     *  @param  rhs  The <code>Format</code> to check.
     *
     *  @return  true if <code>rhs</code> is a subset, false
     *           otherwise.
     */
    public boolean isSubset(Format rhs) {
        if (rhs.getClass() != this.getClass())
                return false;

        if (rhs.attributes!= attributes)
                return false;

        if (rhs.sizeInPoints != 0) {
            if (sizeInPoints != rhs.sizeInPoints)
                return false;
        }

        if (fontName!=rhs.fontName)
            return false;

        if (foreground!=rhs.foreground)
            return false;

        if (background!=rhs.background)
            return false;

        if (rhs.align!= align)
                return false;

        if (rhs.vertAlign!= vertAlign)
                return false;

        return true;
    }
}
