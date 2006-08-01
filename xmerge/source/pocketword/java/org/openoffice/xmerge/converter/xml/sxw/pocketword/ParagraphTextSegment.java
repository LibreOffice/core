/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParagraphTextSegment.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:11:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import org.openoffice.xmerge.converter.xml.TextStyle;

import org.openoffice.xmerge.util.EndianConverter;

import org.openoffice.xmerge.util.ColourConverter;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.awt.Color;

/**
 * This class represents a portion of text with a particular formatting style.
 * The style may differ from the default style of the paragraph of which it
 * is part.
 *
 * @author  Mark Murnane
 * @version 1.1
 */
class ParagraphTextSegment implements PocketWordConstants {

    private String    pText;
    private TextStyle pStyle;


    /**
     * <p>Initialise a new <code>ParagraphTextSegment</p>.
     * <p>Both parameters may be <code>null</code>.</p>
     *
     * @param   data    The text of this segment.
     * @param   style   The style describing this segment.
     */
    public ParagraphTextSegment (String data, TextStyle style) {
        pText  = data;
        pStyle = style;
    }

    /**
     * <p>Sets the text for this segment.</p>
     *
     * @param   data    The text of this segment.
     */
    public void setText (String data) {
        pText = data;
    }

    /**
     * <p>Gets the text for this segment.</p>
     *
     * @return    The text of this segment.
     */
    public String getText () {
        return pText;
    }


    /**
     * <p>Sets the style for this segment.</p>
     *
     * @param   data    The style describing this segment.
     */
    public void setStyle (TextStyle style) {
        pStyle = style;
    }


    /**
     * <p>Gets the style for this segment.</p>
     *
     * @return  The style describing this segment.
     */
    public TextStyle getStyle () {
        return pStyle;
    }


    /**
     * <p>Returns the string data for this text segment wrapped with the
     *    appropriate byte codes for the formatting settings used.</p>
     *
     * @return  <code>byte</code> array containing formatted text in Pocket Word
     *          format.
     */
    public byte[] getData () {
        ByteArrayOutputStream data = new ByteArrayOutputStream();

        boolean colourSet    = false;
        boolean boldSet      = false;
        boolean italicSet    = false;
        boolean underlineSet = false;
        boolean strikeSet    = false;
        boolean highlightSet = false;

        // TODO: Font changes need to be worked out here

        try {
            if (pStyle != null) {
                if (pStyle.getFontColor() != null) {
                    ColourConverter cc = new ColourConverter();
                    short colourCode = cc.convertFromRGB(pStyle.getFontColor());
                    if (colourCode != 0) {  // not black
                        data.write(COLOUR_TAG);
                        data.write(EndianConverter.writeShort(colourCode));
                        colourSet = true;
                    }
                }
                if (pStyle.isSet(TextStyle.BOLD) && pStyle.getAttribute(TextStyle.BOLD)) {
                    data.write(new byte[] { FONT_WEIGHT_TAG, FONT_WEIGHT_BOLD, 0x00 } );
                    boldSet = true;
                }
                if (pStyle.isSet(TextStyle.ITALIC) && pStyle.getAttribute(TextStyle.ITALIC)) {
                    data.write(new byte[] { ITALIC_TAG, 0x01 } );
                    italicSet = true;
                }
                if (pStyle.isSet(TextStyle.UNDERLINE) && pStyle.getAttribute(TextStyle.UNDERLINE)) {
                    data.write(new byte[] { UNDERLINE_TAG, 0x01 } );
                    underlineSet = true;
                }
                if (pStyle.isSet(TextStyle.STRIKETHRU) && pStyle.getAttribute(TextStyle.STRIKETHRU)) {
                    data.write(new byte[] { STRIKETHROUGH_TAG, 0x01 } );
                    strikeSet = true;
                }
                if (pStyle.getBackgroundColor() != null) {
                    data.write(new byte[] { HIGHLIGHT_TAG, 0x01 } );
                    highlightSet = true;
                }
            }


            // Now write out the data
            if (!pText.equals("\t")) {
                data.write(pText.getBytes());
            }
            else {
                /*
                 * Tabs are a special case.  They are represented by Pocket Word
                * as the LE sequence 0xC4 0x04.
                */
                data.write(new byte[] { (byte)0xC4, 0x04 } );
            }


            // Now close out any of the settings changes
            if (colourSet) {
                /*
                 * Colours may change without changing back to black, but
                 * without knowing what the previous colour was, the only
                 * way to ensure correct conversion is to restore to black and
                 * let the next segment change the colour again.
                 */
                data.write(new byte[] { COLOUR_TAG, 0x00, 0x00 } );
            }
            if (boldSet) {
                data.write(new byte[] { FONT_WEIGHT_TAG, FONT_WEIGHT_NORMAL, 0x00 } );
            }
            if (italicSet) {
                data.write(new byte[] { ITALIC_TAG, 0x00 } );
            }
            if (underlineSet) {
                data.write(new byte[] { UNDERLINE_TAG, 0x00 } );
            }
            if (strikeSet) {
                data.write(new byte[] { STRIKETHROUGH_TAG, 0x00 } );
            }
            if (highlightSet) {
                data.write(new byte[] { HIGHLIGHT_TAG, 0x00 } );
            }
        }
        catch (IOException ioe) {
            // Should never occur in a memory based stream
        }

        return data.toByteArray();
    }
}
