/************************************************************************
 *
 *  ListCounter.java
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
 *  Copyright: 2002-2003 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.2 (2003-03-12)
 *
 */

package writer2latex.office;

import writer2latex.util.*;

/**
 * <p>This class produces labels for OOo lists/outlines (for xhtml
 * and text, which cannot produce them on their own).</p>
 *
 */
public class ListCounter {
    private int nCounter[] = new int[11];
    private String sNumFormat[] = new String[11];
    private ListStyle style;
    private int nLevel=1; // current level

    public ListCounter(ListStyle style) {
        this.style = style;
        for (int i=1; i<=10; i++) {
            sNumFormat[i] = style.getLevelProperty(i,XMLString.STYLE_NUM_FORMAT);
        }
    }

    public ListCounter step(int nLevel) {
        nCounter[nLevel]++;
        if (nLevel<10) { nCounter[nLevel+1]=0; }
        this.nLevel = nLevel;
        return this;
    }

    public String getLabel() {
        if (sNumFormat[nLevel]==null) return "*";
        int nLevels = Misc.getPosInteger(style.getLevelProperty(nLevel,
                              XMLString.TEXT_DISPLAY_LEVELS),1);
        String sPrefix = style.getLevelProperty(nLevel,XMLString.STYLE_NUM_PREFIX);
        String sSuffix = style.getLevelProperty(nLevel,XMLString.STYLE_NUM_SUFFIX);
        String sLabel="";
        if (sPrefix!=null) { sLabel+=sPrefix; }
        for (int j=nLevel-nLevels+1; j<nLevel; j++) {
            sLabel+=formatNumber(nCounter[j],sNumFormat[j],true)+".";
        }
        // TODO: Lettersync
        sLabel+=formatNumber(nCounter[nLevel],sNumFormat[nLevel],true);
        if (sSuffix!=null) { sLabel+=sSuffix; }
        return sLabel;
    }

    // Utility method to generate number
    private String formatNumber(int number,String sStyle,boolean bLetterSync) {
        if ("a".equals(sStyle)) { return Misc.int2alph(number,bLetterSync); }
        else if ("A".equals(sStyle)) { return Misc.int2Alph(number,bLetterSync); }
        else if ("i".equals(sStyle)) { return Misc.int2roman(number); }
        else if ("I".equals(sStyle)) { return Misc.int2Roman(number); }
        else if ("1".equals(sStyle)) { return Misc.int2arabic(number); }
        else return "";
    }


}
