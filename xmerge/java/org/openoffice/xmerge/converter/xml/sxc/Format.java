/************************************************************************
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc;

import java.awt.Color;

/**
 *  This class specifies the format for a given spreadsheet cell.
 *
 *  @author  Mark Murnane
 */
public class Format implements Cloneable {

    private String category;
    private String value;
    private String formatSpecifier;
    private int decimalPlaces;
    private boolean bold;
    private boolean italic;
    private boolean underline;
    private String font;
    private Color foreground, background;


    /**
     *  Constructor for creating a new <code>Format</code>.
     */
    public Format() {
        category = "";
        value = "";
        formatSpecifier = "";
        font = "";
        foreground = Color.black;
        background = Color.white;
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
        bold = fmt.isBold();
        italic = fmt.isItalic();
        underline = fmt.isUnderline();
        font = fmt.getFont();
        foreground = fmt.getForeground();
        background = fmt.getBackground();
    }


    /**
     *  Reset this <code>Format</code> description.
     */
    public void clearFormatting() {
       category = "";
       value = "";
       formatSpecifier = "";
       decimalPlaces = 0;
       bold = false;
       italic = false;
       underline = false;
       font = "";
       foreground = Color.black;
       background = Color.white;
    }


    /**
     *  Set the formatting category of this object, i.e. number, date,
     *  currency.  The <code>OfficeConstants</code> class contains string
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
      *  Set the object's bold flag.
      *
      *  @param  value  The value of the bold flag.  true is bold,
      *                 false is not bold.
      */
     public void setBold(boolean value) {
         bold = value;
     }


     /**
      *  Get the object's bold flag.
      *
      *  @return  true if bold flag is on, false otherwise.
      */
     public boolean isBold() {
         return bold;
     }


     /**
      *  Set the object's italic flag.
      *
      *  @param  value  The value of the italic flag.  true is italic,
      *                 false is not italic.
      */
     public void setItalic(boolean value) {
         italic = value;
     }


     /**
      *  Get the object's italic flag.
      *
      *  @return  True if italic flag is on, false otherwise.
      */
     public boolean isItalic() {
         return italic;
     }


     /**
      *  Set the object's underline flag.
      *
      *  @param  value  The value of the underline flag.  true is underlined,
      *                 false is not underlined.
      */
     public void setUnderline(boolean value) {
         underline = value;
     }


     /**
      *  Get the object's underline flag.
      *
      *  @return  true if underline flag is on, false otherwise.
      */
     public boolean isUnderline() {
         return underline;
     }


     /**
      *  Set the font used for this cell.
      *
      *  @param  fontName  The name of the font.
      */
     public void setFont(String fontName) {
         font = fontName;
     }


     /**
      *  Get the font used for this cell.
      *
      *  @return  The font name.
      */
     public String getFont() {
         return font;
     }


     /**
      *  Set the Foreground <code>Color</code> for this cell.
      *
      *  @param  color  A <code>Color</code> object representing the
      *                 foreground color.
      */
     public void setForeground(Color color) {
         foreground = new Color(color.getRGB());
     }


     /**
      *  Get the Foreground <code>Color</code> for this cell.
      *
      *  @return  Foreground <code>Color</code> value.
      */
     public Color getForeground() {
         return new Color(foreground.getRGB());
     }


     /**
      *  Set the Background <code>Color</code> for this cell
      *
      *  @param  color  A <code>Color</code> object representing
      *                 the background color.
      */
     public void setBackground(Color color) {
         background = new Color(color.getRGB());
     }


     /**
      *  Get the Foreground <code>Color</code> for this cell
      *
      *  @return  Background <code>Color</code> value
      */
     public Color getBackground() {
         return new Color(background.getRGB());
     }
}

