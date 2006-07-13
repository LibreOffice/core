/************************************************************************
 *
 *  LaTeXDocumentPortion.java
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
 *  Version 0.3.3f (2004-08-26)
 *
 */

package writer2latex.latex;

import java.io.OutputStreamWriter;
import java.io.IOException;
import java.util.Vector;

import writer2latex.util.Misc;

/** This class represents a portion of a LaTeX document. A portion is any
number of lines, and may include subportions. */
public class LaTeXDocumentPortion {

    private Vector nodes; // The collection of all nodes in this portion

    private StringBuffer curText; // The currently active node (always the last node)
    private boolean bEmpty; // Is the active node empty?

    private boolean bWrap; // Do we allow line wrap in this portion?

    public LaTeXDocumentPortion(boolean bWrap){
        this.bWrap = bWrap;
        nodes = new Vector();
        curText = new StringBuffer();
        bEmpty = true;
    }

    /** Add another portion to the end of this portion */
    public LaTeXDocumentPortion append(LaTeXDocumentPortion ldp) {
        if (!bEmpty) {
            // add the current node to the node list and create new current node
            nodes.add(curText);
            curText = new StringBuffer();
            bEmpty = true;
        }
        nodes.add(ldp);
        return this;
    }

    /** Add a string to the end of this portion */
    public LaTeXDocumentPortion append(String s){
        curText.append(s);
        bEmpty = false; // even if this is the empty string!
        return this;
    }

    /** Add a newline to the end of this portion */
    public LaTeXDocumentPortion nl(){
        curText.append("\n");
        bEmpty = false;
        return this;
    }

    /** write a segment of text (eg. a word) to the output */
    private void writeSegment(String s, int nStart, int nEnd, OutputStreamWriter osw) throws IOException {
        for (int i=nStart; i<nEnd; i++) { osw.write(s.charAt(i)); }
    }

    /** write the contents of a StringBuffer to the output */
    private void writeBuffer(StringBuffer text, OutputStreamWriter osw, int nLineLen, String sNewline) throws IOException {
        String s = text.toString();
        int nLen = s.length();

        int[] nBreakPoints = new int[100];
        int nLastBPIndex = 99;

        int nStart = 0;

        while (nStart<nLen) {
            // identify line and breakpoints
            int nBPIndex = 0;
            boolean bEscape = false;
            boolean bComment = false;
            int nNewline = nStart;
            char c;
            while (nNewline<nLen) {
                if (nBPIndex==nLastBPIndex) {
                    nBreakPoints = Misc.doubleIntArray(nBreakPoints);
                    nLastBPIndex = nBreakPoints.length-1;
                }
                c = s.charAt(nNewline);
                if (c=='\n') {
                    nBreakPoints[nBPIndex++] = nNewline;
                    break;
                }
                if (bEscape) { bEscape = false; }
                else if (c=='\\') { bEscape = true; }
                else if (c=='%') { bComment = true; }
                else if (!bComment && c==' ') { nBreakPoints[nBPIndex++] = nNewline; }
                nNewline++;
            }
            if (nBPIndex==nLastBPIndex) {
                nBreakPoints = Misc.doubleIntArray(nBreakPoints);
                nLastBPIndex = nBreakPoints.length-1;
            }
            if (nNewline==nLen) { nBreakPoints[nBPIndex++] = nNewline; }

            // write out line
            int nCurLineLen = nBreakPoints[0]-nStart;
            writeSegment(s,nStart,nBreakPoints[0],osw);
            for (int i=0; i<nBPIndex-1; i++) {
                int nSegmentLen = nBreakPoints[i+1]-nBreakPoints[i];
                if (nSegmentLen+nCurLineLen>nLineLen) {
                    // break line before this segment
                    osw.write(sNewline);
                    nCurLineLen = nSegmentLen;
                }
                else {
                    // segment fits in current line
                    osw.write(" ");
                    nCurLineLen += nSegmentLen;
                }
                writeSegment(s,nBreakPoints[i]+1,nBreakPoints[i+1],osw);
            }
            osw.write(sNewline);
            nStart = nNewline+1;
        }
    }

    /** write the contents of a StringBuffer to the output without wrap */
    private void writeBuffer(StringBuffer text, OutputStreamWriter osw, String sNewline) throws IOException {
        String s = text.toString();
        int nLen = s.length();

        int nStart = 0;

        while (nStart<nLen) {
            // identify line
            int nNewline = nStart;
            while (nNewline<nLen) {
                if (s.charAt(nNewline)=='\n') { break; }
                nNewline++;
            }

            // write out line
            writeSegment(s,nStart,nNewline,osw);
            osw.write(sNewline);
            nStart = nNewline+1;
        }
    }

    /** Write this portion to the output (note: nLineLen=0 means no wrap) */
    public void write(OutputStreamWriter osw, int nLineLen, String sNewline) throws IOException {
        int n = nodes.size();
        for (int i=0; i<n; i++) {
            if (nodes.get(i) instanceof LaTeXDocumentPortion) {
                ((LaTeXDocumentPortion) nodes.get(i)).write(osw,nLineLen,sNewline);
            }
            else if (bWrap && nLineLen>0) {
                writeBuffer((StringBuffer) nodes.get(i),osw,nLineLen,sNewline);
            }
            else {
                writeBuffer((StringBuffer) nodes.get(i),osw,sNewline);
            }
        }
        if (!bEmpty) { // write current node as well
            if (bWrap && nLineLen>0) {
                writeBuffer(curText,osw,nLineLen,sNewline);
            }
            else {
                writeBuffer(curText,osw,sNewline);
            }
        }
    }

} // end class LaTeXDocumentPortion

// TO DO: consider StringBuffer->ByteArrayOutputStream

