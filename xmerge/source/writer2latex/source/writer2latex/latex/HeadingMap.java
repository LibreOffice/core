/************************************************************************
 *
 *  HeadingMap.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3f (2004-08-26)
 *
 */

package writer2latex.latex;

/** This class contains data for the mapping of OOo headings to LaTeX headings.
    A LaTeX heading is characterized by a name and a level.
    The heading is inserted with \name{...} or \name[...]{...}
    The headings are supposed to be "normal" LaTeX headings,
    ie. the names are also counter names, and the headings
    can be reformatted using \@startsection etc.
    Otherwise max-level should be zero.
*/
public class HeadingMap {
    private int nMaxLevel;
    private String[] sName;
    private int[] nLevel;

    /** Constructor: Create a new HeadingMap
        @param nMaxLevel the maximal level of headings that are mapped */
    public HeadingMap(int nMaxLevel) {
        reset(nMaxLevel);
    }

    /** Clear all data associated with this HeadingMap (in order to reuse it) */
    public void reset(int nMaxLevel) {
        this.nMaxLevel = nMaxLevel;
        sName = new String[nMaxLevel+1];
        nLevel = new int[nMaxLevel+1];
    }

    /** Set data associated with a specific heading level */
    public void setLevelData(int nWriterLevel, String sName, int nLevel) {
        this.sName[nWriterLevel] = sName;
        this.nLevel[nWriterLevel] = nLevel;
    }

    /** Returns the maximal Writer level associated with this HeadingMap */
    public int getMaxLevel() { return nMaxLevel; }

    /** Return the name (for counter and \@startsection) for this level */
    public String getName(int nWriterLevel) { return sName[nWriterLevel]; }

    /** Return the LaTeX level for this Writer level (for \@startsection) */
    public int getLevel(int nWriterLevel) { return nLevel[nWriterLevel]; }
}
