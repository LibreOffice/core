/************************************************************************
 *
 *  ExportNameCollection.java
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
 *  Version 0.3.3f (2004-09-14)
 *
 */

package writer2latex.util;

import java.util.Hashtable;

// Collection of export names
// Used for mapping named collections to simpler names (only A-Z, a-z and 0-9)
public class ExportNameCollection{
    private Hashtable exportNames = new Hashtable();
    private String sPrefix;
    private boolean bAcceptNumbers;

    public ExportNameCollection(String sPrefix, boolean b) {
        this.sPrefix=sPrefix;
        bAcceptNumbers = b;
    }

    public ExportNameCollection(boolean b) {
        this("",b);
    }

    public void addName(String sName){
        if (containsName(sName)) { return; }
        StringBuffer outbuf=new StringBuffer();
        SimpleInputBuffer inbuf=new SimpleInputBuffer(sName);

        // Don't start with a digit
        if (bAcceptNumbers && inbuf.peekChar()>='0' && inbuf.peekChar()<='9') {
            outbuf.append('a');
        }

        char c;
        // convert numbers to roman numbers and discard unwanted characters
        while ((c=inbuf.peekChar())!='\0'){
            if ((c>='a' && c<='z') || (c>='A' && c<='Z')) {
                outbuf.append(inbuf.getChar());
            }
            else if (c>='0' && c<='9'){
                if (bAcceptNumbers) {
                    outbuf.append(inbuf.getInteger());
                }
                else {
                    outbuf.append(Misc.int2roman(
                                  Integer.parseInt(inbuf.getInteger())));
                }
            }
            else {
                inbuf.getChar(); // ignore this character
            }
        }
        String sExportName=outbuf.toString();
        // the result may exist in the collecion; add a's at the end
        while (exportNames.containsValue(sExportName)){
            sExportName+="a";
        }
        exportNames.put(sName,sExportName);
    }

    public String getExportName(String sName) {
        // add the name, if it does not exist
        if (!containsName(sName)) { addName(sName); }
        return sPrefix + (String) exportNames.get(sName);
    }

    public boolean containsName(String sName) {
        return exportNames.containsKey(sName);
    }

    public boolean isEmpty() {
        return exportNames.size()==0;
    }
}
