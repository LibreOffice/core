/************************************************************************
 *
 *  SimpleInputBuffer.java
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
 *  Version 0.3.3g (2004-11-21)
 *
 */

package writer2latex.util;

// Simple string input buffer
public class SimpleInputBuffer{

    private String sContent;
    private int nIndex, nLen;

    private static boolean isEndOrLineEnd(char cChar){
        switch (cChar){
            case '\0':
            case '\n':
            case '\r':
                return true;
            default:
                return false;
        }
    }

    private static boolean isDigitOrDot(char cChar){
        return (cChar>='0' && cChar<='9') || cChar=='.';
    }

    private static boolean isDigitOrDotOrComma(char cChar) {
        return isDigitOrDot(cChar) || cChar==',';
    }

    public SimpleInputBuffer(String sContent){
        this.sContent=sContent;
        nLen=sContent.length();
        nIndex=0;
    }

    public int getIndex(){return nIndex;}

    public char peekChar(){
        return nIndex<nLen ? sContent.charAt(nIndex) : '\0';
    }

    public char peekFollowingChar(){
        return nIndex+1<nLen ? sContent.charAt(nIndex+1) : '\0';
    }

    public char getChar(){
        return nIndex<nLen ? sContent.charAt(nIndex++) : '\0';
    }

    public String getIdentifier(){
        int nStart=nIndex;
        while (nIndex<nLen && (Character.isLetter(sContent.charAt(nIndex)) ||
               isDigitOrDot(sContent.charAt(nIndex))))
            nIndex++;
        return sContent.substring(nStart,nIndex);
    }

    public String getNumber(){
        int nStart=nIndex;
        while (nIndex<nLen && isDigitOrDotOrComma(sContent.charAt(nIndex)))
            nIndex++;
        return sContent.substring(nStart,nIndex);
    }

    public String getInteger(){
        int nStart=nIndex;
        while (nIndex<nLen && sContent.charAt(nIndex)>='0' && sContent.charAt(nIndex)<='9'){
            nIndex++;
        }
        return sContent.substring(nStart,nIndex);
    }
}
