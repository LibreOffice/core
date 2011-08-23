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

package org.openoffice.xmerge.util;


import java.awt.Color;

/**
 * Utility class mapping RGB colour specifications to the colour indices used
 * in the Pocket PC. The original converter was written for use with Pocket
 * Word it was later put into the utils so Pocket excel could use this code
 * also. For this reason the defualt values are those used by Pocket Word but
 * a colour table can be passed in through the constructor to map the 16
 * values to a colour table.
 *
 * These colour indices are based on the Windows VGA 16 colour palette, which
 * later was used as the basis for the named colours in the HTML 3.2 
 * specification.
 *
 * In Pocket Word's case, the match to the VGA 16 palette is not exact as it 
 * swaps Grey and Silver, with Silver being the darker colour (i.e. having the
 * lower RGB value).
 */

public class ColourConverter {

    /** Colour table index for Black */
    private static final short BLACK = 0;
    
    /** Colour table index for Silver */
    private static final short SILVER = 1;
    
    /** Colour table index for Grey */
    private static final short GREY = 2;
    
    /** Colour table index for White */
    private static final short WHITE = 3;
    
    /** Colour table index for Red */
    private static final short RED = 4;
    
    /** Colour table index for Lime */
    private static final short LIME = 5;
    
    /** Colour table index for Blue */
    private static final short BLUE = 6;
    
    /** Colour table index for Aqua */
    private static final short AQUA = 7;
    
    /** Colour table index for Fuchsia */
    private static final short FUCHSIA = 8;
    
    /** Colour table index for Yellow */
    private static final short YELLOW = 9;
    
    /** Colour table index for Maroon */
    private static final short MAROON = 10;
    
    /** Colour table index for Green */
    private static final short GREEN = 11;
    
    /** Colour table index for Navy */
    private static final short NAVY = 12;
    
    /** Colour table index for Teal */
    private static final short TEAL = 13;
    
    /** Colour table index for Purple */
    private static final short PURPLE = 14;
    
    /** Colour table index for Olive */
    public static final short OLIVE = 15;

    private short tableLookup[] = null;

    /** 
     * Default constructor used in the case where a lookup table is not
     * required
     */
    public ColourConverter() {
    
    }
    
    /** 
     * Constructor that passes in the colour lookup table. This is required in
     * cases where the 16 colour values are something other than there default
     * values (e.g. in the case of pocket Excel)
     * 
     * @param short[] a 16 bit array mapping the 16 colours to there values
     */
    public ColourConverter(short lookup[]) {

        tableLookup = lookup;
    }

    /**
     * Uses the colour table it it exists to translate default values to
     * values in the colorTable
     */
    private short colourLookup(short colour) {
    
        if(tableLookup!=null) {
            return tableLookup[colour];
        } else {
            return colour;
        }
    }
    
    /**
     * Uses the colour table it it exists to translate default values to
     * values in the colorTable
     */
    private short indexLookup(short index) {
        
        short result = 0;

        if(tableLookup!=null) {
            for(short i = 0;i < tableLookup.length;i++) {
                if(tableLookup[i]==index)
                    result = i;
            }
        } else {
            result = index;
        }

        return result;
    } 
    /**
     * This method maps a Pocket Word colour index value to an RGB value as
     * used by OpenOffice.
     *
     * @param   index   The index into Pocket Word's colour table.
     *
     * @return  A Color object representing the RGB value of the Pocket Word 
     *          colour.
     */
    public Color convertToRGB (short colour) {

        short index = indexLookup(colour);

        int r = 0;
        int g = 0;
        int b = 0;
        
        switch (index) {
            case SILVER:
                r = g = b = 128;
                break;
                
            case GREY:
                r = g = b = 192;
                break;
                
            case WHITE:
                r = g = b = 255;
                break;
                
            case RED:
                r = 255;
                break;
                
            case LIME:
                g = 255;
                break;
                
            case BLUE:
                b = 255;
                break;
                
            case AQUA:
                g = b = 255;
                break;
                
            case FUCHSIA:
                r = b = 255;
                break;
                
            case YELLOW:
                r = g = 255;
                break;
                
            case MAROON:
                r = 128;
                break;
                
            case GREEN:
                g = 128;
                break;
                
            case NAVY:
                b = 128;
                break;
                
            case TEAL:
                b = g = 128;
                break;
                
            case PURPLE:
                r = b = 128;
                break;
                
            case OLIVE:
                r = g = 128;
                break;
                
            case BLACK:
            default:
                r = g = b = 0;
                break;
        }
        
        return new Color(r, g, b);
    }
    
    
    /**
     * This method approximates an RGB value (as used by Writer) to one of the
     * 16 available colours 
     *
     * Most of the supported colours have their components set to either 0, 128
     * or 255.  The exception is 'Grey' which is 0xC0C0C0.
     *
     * @param colour    Color object representing the RGB value of the colour.
     *
     * @return  Index into the Pocket Word colour table which represents the 
     *          closest match to the specified colour.
     */
    public short convertFromRGB (Color colour) {
        int matchedRGB = 0;
        short indexColour = 0;
        int reducedMap[] =  new int[] { 0, 0, 128 };
      
          int red = colour.getRed();
          int green = colour.getGreen();
          int blue = colour.getBlue();

        // We need to convert the pale colors to their base color rather than
        // white so we modify the rgb values if the colour is sufficently
        // white
           if(red>0xC0 && green>0xC0 && blue>0xC0) {
            
            if(red!=0xFF)
                red = getClosest(red, reducedMap);
            if(green!=0xFF)
                green = getClosest(green, reducedMap);
            if(blue!=0xFF)
                blue = getClosest(blue, reducedMap);
        } 

           /*
         * Need to derive an RGB value that has been rounded to match the ones
         * Pocket Word knows about.
            */
        matchedRGB += getClosest(red)   << 16;
        matchedRGB += getClosest(green) << 8;
           matchedRGB += getClosest(blue);
        
        /*
         * The colour map used by Pocket Word doesn't have any combinations of 
         * values beyond 0 and any other value.  A value of 255 in any RGB 
         * code indicates a dominant colour.  Other colours are only modifiers
         * to the principal colour(s).  Thus, for this conversion, modifiers
         * can be dropped.
         */
        if ((matchedRGB & 0xFF0000) == 0xFF0000 || (matchedRGB & 0xFF00) == 0xFF00
                || (matchedRGB & 0xFF) == 0xFF) {
                    if ((matchedRGB & 0xFF0000) == 0x800000) {
                        matchedRGB ^= 0x800000;
                    }
                    if ((matchedRGB & 0xFF00) == 0x8000) {
                        matchedRGB ^= 0x8000;
                    }
                    if ((matchedRGB & 0xFF) == 0x80) {
                        matchedRGB ^= 0x80;
                    }   
        }
        
        
        /*
         * And now for the actual matching ...
         *
         * Colours are based on the Windows VGA 16 palette.  One difference
         * though is that Pocket Word seems to switch the RGB codes for Grey
         * and Silver.  In Pocket Word Silver is the darker colour leaving Grey
         * is closest to White.
         *
         * Shades of grey will be converted to either Silver or White, where
         * Grey may be a more appropraite colour.  This is handled specially
         * only for Silver and White matches.
         */
        switch (matchedRGB) {
            case 0x000000:
                indexColour = BLACK;   
                break;
                
            case 0x808080:
                if (!isGrey(colour)) {
                    indexColour = SILVER; 
                }
                else {
                    indexColour = GREY;
                }
                break;
                                    
            case 0xFFFFFF:
                if (!isGrey(colour)) {
                    indexColour = WHITE;
                }
                else {
                    indexColour = GREY;
                }
                break;
                
            case 0xFF0000:
                indexColour = RED;
                break;
                
            case 0x00FF00:
                indexColour = LIME;
                break;
                
            case 0x0000FF:
                indexColour = BLUE;
                break;
                
            case 0x00FFFF:
                indexColour = AQUA;
                break;
                
            case 0xFF00FF:
                indexColour = FUCHSIA;
                break;
                
            case 0xFFFF00:
                indexColour = YELLOW;
                break;
                
            case 0x800000:
                indexColour = MAROON;
                break;
                
            case 0x008000:
                indexColour = GREEN;
                break;
                
            case 0x000080:
                indexColour = NAVY;
                break;
                
            case 0x008080:
                indexColour = TEAL;
                break;
                
            case 0x800080:
                indexColour = PURPLE;
                break;
                
            case 0x808000:
                indexColour = OLIVE;
                break;
                
            default:        // Just in case!
                indexColour = BLACK;
                break;
        }
         
        return colourLookup(indexColour);
    }
    

    /*
     * Default implementation, checks for the closest of value to 0, 128 or 255.
     */
    private int getClosest(int value) {
        int points[] = new int[] { 0, 128, 255 };
        
        return getClosest(value, points);
    }
    
    
    /*
     * Utility method that returns the closest of the three points to the value 
     * supplied.
     */
    private int getClosest(int value, int[] points) {

        if (value == points[0] || value == points[1] || value == points[2]) {
            return value;
        }
        
        if (value < points[1]) {
            int x = value - points[0];
            return (Math.round((float)x / (points[1] - points[0])) == 1 ? points[1] : points[0]);
        }
        else {
            int x = value - points[1];
            return (Math.round((float)x / (points[2] - points[1])) >= 1 ? points[2] : points[1]);
        }
    }
    
    
    /*
     * Checks to see if the supplied colour can be considered to be grey.
     */
    private boolean isGrey(Color c) {
        int matchedRGB = 0;
        int points[] = new int[] { 128, 192, 255 };
        
        matchedRGB += getClosest(c.getRed(), points) << 16;
        matchedRGB += getClosest(c.getGreen(), points) << 8;
        matchedRGB += getClosest(c.getBlue(), points);
        
        if (matchedRGB == 0xC0C0C0) {
            return true;
        }
        
        return false;
    }
}
    
