/************************************************************************
 *
 *  CSVList.java
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
 *  Copyright: 2002 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3f (2004-08-10)
 *
 */

package writer2latex.util;

// Create a list of values separated by commas or another seperation character
public class CSVList{
    private String sSep;
    private String sNameValueSep;
    private boolean bEmpty = true;
    private StringBuffer buf = new StringBuffer();

    public CSVList(String sSep, String sNameValueSep) {
        this.sSep=sSep;
        this.sNameValueSep=sNameValueSep;
    }

    public CSVList(String sSep) {
        this(sSep,":");
    }

    public CSVList(char cSep) {
        this(Character.toString(cSep),":");
    }

    public void addValue(String sVal){
        if (sVal==null) { return; }
        if (bEmpty) { bEmpty=false; } else { buf.append(sSep); }
        buf.append(sVal);
    }

    public void addValue(String sName, String sVal) {
        if (sName==null) { return; }
        if (bEmpty) { bEmpty=false; } else { buf.append(sSep); }
        buf.append(sName).append(sNameValueSep).append(sVal);
    }

    public String toString() {
        return buf.toString();
    }

    public boolean isEmpty() {
        return bEmpty;
    }

}
