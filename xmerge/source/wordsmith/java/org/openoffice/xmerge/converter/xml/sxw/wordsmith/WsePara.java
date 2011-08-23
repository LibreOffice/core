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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import java.io.IOException;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.converter.xml.*;


/**
 *  This class represents a paragraph in a WordSmith document.
 *  (A paragraph is "5" followed by 12 bytes of attributes.)
 *
 *  @author   David Proulx
 */
class WsePara extends Wse {

    private byte spaceBefore = 0;
    private byte spaceAfter = 0;
    private byte leftIndent = 0;
    private byte firstIndent = 0;
    private byte rightIndent = 0;
    private byte misc = 0;
    private byte style = 0;
    private byte lineSpace = 0;
    private byte outline = 0;
    private byte reserved = 0;
    
    private static final byte LS_EXACTLY   = (byte)0xC0;
    private static final byte LS_ATLEAST   = (byte)0x80;
    private static final byte LS_MULTIPLE  = (byte)0x40;
    private static final byte LS_VALUEMASK = (byte)0x3F;
    
    private static final byte ALIGN_RIGHT  = (byte)2;
    private static final byte ALIGN_LEFT   = (byte)0;
    private static final byte ALIGN_CENTER = (byte)1;
    private static final byte ALIGN_JUST   = (byte)3;

    private StyleCatalog sc = null;
            

    /**
     *  Constructor for use when going from DOM to WordSmith.
     *
     *  @param  p   The paragraph style.
     *  @param  sc  The <code>StyleCatalog</code>.
     */
    public WsePara(ParaStyle p, StyleCatalog sc) {
        this.sc = sc;
        ParaStyle ps = (ParaStyle)p.getResolved();

        if (ps.isAttributeSet(ParaStyle.MARGIN_LEFT)) {
            double temp = ps.getAttribute(ParaStyle.MARGIN_LEFT) * 1.6 / 100;
            leftIndent = (byte) temp;
            if ((temp - leftIndent) > 0.5) leftIndent++;
        }

        if (ps.isAttributeSet(ParaStyle.MARGIN_RIGHT)) {
            double temp = ps.getAttribute(ParaStyle.MARGIN_RIGHT) * 1.6 / 100;
            rightIndent = (byte) temp;
            if ((temp - rightIndent) > 0.5) rightIndent++;
        }

        if (ps.isAttributeSet(ParaStyle.TEXT_INDENT)) {
            double temp = ps.getAttribute(ParaStyle.TEXT_INDENT) * 1.6 / 100;
            firstIndent = (byte) temp;
            if ((temp - firstIndent) > 0.5) firstIndent++;
        }

        if (ps.isAttributeSet(ParaStyle.MARGIN_TOP)) {
            double temp = ps.getAttribute(ParaStyle.MARGIN_TOP) * 1.6 / 100;
            spaceBefore = (byte) temp;
            if ((temp - spaceBefore) > 0.5) spaceBefore++;
        }

        if (ps.isAttributeSet(ParaStyle.MARGIN_BOTTOM)) {
            double temp = ps.getAttribute(ParaStyle.MARGIN_BOTTOM) * 1.6 / 100;
            spaceAfter = (byte) temp;
            if ((temp - spaceAfter) > 0.5) spaceAfter++;
        }

        if (ps.isAttributeSet(ParaStyle.LINE_HEIGHT)) {
            int lh = ps.getAttribute(ParaStyle.LINE_HEIGHT);
            if ((lh & ~ParaStyle.LH_VALUEMASK) == 0)
                lineSpace = (byte)(LS_MULTIPLE | (lh * 2));
            else if ((lh & ParaStyle.LH_PCT) != 0) {
                lh = (lh & ParaStyle.LH_VALUEMASK) / 100;
                lineSpace = (byte)(LS_MULTIPLE | (lh * 2));
            }
            // DJP: handle other cases....
        }

        if (ps.isAttributeSet(ParaStyle.TEXT_ALIGN)) {

            int val = ps.getAttribute(ParaStyle.TEXT_ALIGN);
          
            switch (val) {
            case ParaStyle.ALIGN_RIGHT:
                misc = ALIGN_RIGHT;
                break;
            case ParaStyle.ALIGN_LEFT:
                misc = ALIGN_LEFT;
                break;
            case ParaStyle.ALIGN_CENTER:
                misc = ALIGN_CENTER;
                break;
            case ParaStyle.ALIGN_JUST:
                misc = ALIGN_JUST;
                break;
            }
        }
        
    }
    

    /**
     *  Constructor for use when going from WordSmith to DOM.
     *  Assumes <code>dataArray[startIndex]</code> is the first
     *  <code>byte</code> of a valid WordSmith paragraph descriptor.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     */
    public WsePara(byte dataArray[], int startIndex) {
        spaceBefore = dataArray[startIndex + 1];
        spaceAfter  = dataArray[startIndex + 2];
        leftIndent  = dataArray[startIndex + 3];
        firstIndent = dataArray[startIndex + 4];
        rightIndent = dataArray[startIndex + 5];
        misc        = dataArray[startIndex + 6];
        style       = dataArray[startIndex + 7];
        lineSpace   = dataArray[startIndex + 8];
        outline     = dataArray[startIndex + 9];
    }
    
    
    /**
     *  Compute the index of the first <code>byte</code> following the
     *  paragraph descriptor, assuming that
     *  <code>dataArray[startIndex]</code> is the beginning of a valid
     *  paragraph descriptor.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     *
     *  @return  The index of the first <code>byte</code> following the
     *           paragraph description.
     */
    static int computeNewIndex(byte dataArray[], int startIndex) {
        return startIndex + 13;
    }
    
    
    /**
     *  Return true if <code>dataArray[startIndex]</code> is the start
     *  of a valid paragraph descriptor.
     *
     *  @param  dataArray   <code>byte</code> array.
     *  @param  startIndex  The start index.
     *
     *  @return  true if <code>dataArray[startIndex]</code> is the start
     *           of a valid paragraph descriptor, false otherwise.
     */
    static boolean isValid(byte dataArray[], int startIndex) {
        return (dataArray[startIndex] == 5);
    }

    /**
     *  Return the number of bytes needed to represent this paragraph.
     *
     *  @return  The number of bytes needed to represent this paragraph.
     */
    int getByteCount() {
        return 13;
    }    
    
    /**
     *  Return an <code>byte</code> array representing this paragraph.
     *
     *  @return  An <code>byte</code> array representing this paragraph.
     */
    byte[] getBytes() {
        byte b[] = new byte[13];
        
        b[0] = 5;
        b[1] = spaceBefore;
        b[2] = spaceAfter;
        b[3] = leftIndent;
        b[4] = firstIndent;
        b[5] = rightIndent;
        b[6] = misc;
        b[7] = style;
        b[8] = lineSpace;
        b[9] = outline;
        b[10] = reserved;
        b[11] = 0;
        b[12] = 0;
        
        return b;
    }
    
    /**
     *  Return a <code>ParaStyle</code> that reflects the formatting of
     *  this run.
     *
     *  @return  A <code>ParaStyle</code> that reflects the formatting
     *           of this run.
     */
    ParaStyle makeStyle() {
        /* Csaba: Commented out the LINE_HEIGHT syle, because there was no 
                  incoming data for that style. It was resulting a zero line
                  height in the xml document, ie. the doc looked empty.
        */
        int attrs[] = { ParaStyle.MARGIN_LEFT, ParaStyle.MARGIN_RIGHT, 
                           ParaStyle.TEXT_INDENT, //ParaStyle.LINE_HEIGHT,
                           ParaStyle.MARGIN_TOP, ParaStyle.MARGIN_BOTTOM,
                           ParaStyle.TEXT_ALIGN };
        String values[] = new String[attrs.length];
        double temp;
        
        temp = leftIndent / 1.6;
        values[0] = (new Double(temp)).toString() + "mm";
        
        temp = rightIndent / 1.6;
        values[1] = (new Double(temp)).toString() + "mm";

        temp = firstIndent / 1.6;
        values[2] = (new Double(temp)).toString() + "mm";
        
/*        if ((lineSpace & LS_MULTIPLE) != 0) {
            temp = (lineSpace & LS_VALUEMASK) / 2;
            temp *= 100;
            values[3] = (new Double(temp)).toString() + "%";
          } else {
            values[3] = (new Double(temp)).toString() + "mm";
            // DJP: handle other cases
          }
*/        
        temp = spaceBefore / 1.6;
//        values[4] = (new Double(temp)).toString() + "mm";
        values[3] = (new Double(temp)).toString() + "mm";
          
        temp = spaceAfter / 1.6;
//        values[5] = (new Double(temp)).toString() + "mm";
        values[4] = (new Double(temp)).toString() + "mm";

        switch (misc) {

// case ALIGN_RIGHT: values[6] = "right"; break;
//        case ALIGN_LEFT:  values[6] = "left"; break;
//        case ALIGN_CENTER:values[6] = "center"; break;
//        case ALIGN_JUST:  values[6] = "justified"; break;

            case ALIGN_RIGHT: values[5] = "right"; break;
            case ALIGN_LEFT:  values[5] = "left"; break;
            case ALIGN_CENTER:values[5] = "center"; break;
            case ALIGN_JUST:  values[5] = "justified"; break;
        }
        ParaStyle x = new ParaStyle(null, "paragraph", null, attrs, 
                                        values, sc);
        
        return x;
    }
}

