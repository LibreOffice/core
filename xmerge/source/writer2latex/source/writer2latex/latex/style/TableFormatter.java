/************************************************************************
 *
 *  TableFormatter.java
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
 *  Version 0.3.3f (2004-08-26)
 *
 */

package writer2latex.latex.style;

import org.w3c.dom.Node;

import writer2latex.util.*;
import writer2latex.office.*;

/**
 *  <p>This class converts OOo table styles to LaTeX.</p>
 *  <p> In OOo the table style is distributed on table, column and cell styles.
 *  <p> In LaTeX we have to rearrange this information slightly, so this class
 *  takes care of that.</p>
 */
public class TableFormatter {

    private WriterStyleCollection wsc;
    private boolean bApplyCellFormat;
    private TableGridModel table;
    private boolean[][] bHBorder;
    private boolean[][] bVBorder;
    private boolean[] bGlobalVBorder;
    private String[] sColumnWidth;
    private boolean bIsLongtable;
    private boolean bIsSubtable;

    /** <p>Constructor: Create from a TableGridModel.</p>
     */
    public TableFormatter(WriterStyleCollection wsc, TableGridModel table,
        boolean bAllowLongtable, boolean bApplyCellFormat) {
        this.wsc = wsc;
        this.table = table;
        this.bApplyCellFormat = bApplyCellFormat;
        int nRowCount = table.getRowCount();
        int nColCount = table.getColCount();

        // Step 1: Initialize borders:
        bHBorder = new boolean[nRowCount+1][nColCount];
        for (int nRow=0; nRow<=nRowCount; nRow++) {
            for (int nCol=0; nCol<nColCount; nCol++) {
                bHBorder[nRow][nCol] = false;
            }
        }
        bVBorder = new boolean[nRowCount][nColCount+1];
        for (int nRow=0; nRow<nRowCount; nRow++) {
            for (int nCol=0; nCol<=nColCount; nCol++) {
                bVBorder[nRow][nCol] = false;
            }
        }

        // Step 2: Collect borders from cell styles:
        for (int nRow=0; nRow<nRowCount; nRow++) {
            int nCol = 0;
            while (nCol<nColCount) {
                Node cell = table.getCell(nRow,nCol);
                String sStyleName = Misc.getAttribute(cell,XMLString.TABLE_STYLE_NAME);
                StyleWithProperties style = wsc.getCellStyle(sStyleName);
                int nColSpan = Misc.getPosInteger(Misc.getAttribute(cell,
                                   XMLString.TABLE_NUMBER_COLUMNS_SPANNED),1);
                boolean bLeft = false;
                boolean bRight = false;
                boolean bTop = false;
                boolean bBottom = false;
                if (style!=null) {
                    String sBorder = style.getProperty(XMLString.FO_BORDER);
                    if (sBorder!=null && !"none".equals(sBorder)) {
                        bLeft = true; bRight = true; bTop = true; bBottom = true;
                    }
                    sBorder = style.getProperty(XMLString.FO_BORDER_LEFT);
                    if (sBorder!=null && !"none".equals(sBorder)) {
                        bLeft = true;
                    }
                    sBorder = style.getProperty(XMLString.FO_BORDER_RIGHT);
                    if (sBorder!=null && !"none".equals(sBorder)) {
                        bRight = true;
                    }
                    sBorder = style.getProperty(XMLString.FO_BORDER_TOP);
                    if (sBorder!=null && !"none".equals(sBorder)) {
                        bTop = true;
                    }
                    sBorder = style.getProperty(XMLString.FO_BORDER_BOTTOM);
                    if (sBorder!=null && !"none".equals(sBorder)) {
                        bBottom = true;
                    }
                }
                bVBorder[nRow][nCol] |= bLeft;
                bVBorder[nRow][nCol+nColSpan] |= bRight;
                do {
                    bHBorder[nRow][nCol] |= bTop;
                    bHBorder[nRow+1][nCol] |= bBottom;
                    nCol++;
                } while (--nColSpan>0);
            }
        }

        // Step 3: Remove outer borders if this is a subtable
        bIsSubtable = XMLString.TABLE_SUB_TABLE.equals(table.getTableNode().getNodeName());
/*        if (bIsSubtable) {
            for (int nRow=0; nRow<nRowCount; nRow++) {
                bVBorder[nRow][0] = false; // outer left
                bVBorder[nRow][nColCount] = false; // outer right
            }
            for (int nCol=0; nCol<nColCount; nCol++) {
                bHBorder[0][nCol] = false; // outer top
                bHBorder[nRowCount][nCol] = false; // outer bottom
            }
        }*/

        // Step 4: Create global vertical borders based on simple majority
        // (in order to minimize the number of \multicolum{1} entries)
        bGlobalVBorder = new boolean[nColCount+1];
        for (int nCol=0; nCol<=nColCount; nCol++) {
            int nBalance = 0;
            for (int nRow=0; nRow<nRowCount; nRow++) {
                nBalance += bVBorder[nRow][nCol] ? 1 : -1;
            }
            bGlobalVBorder[nCol] = nBalance>0;
        }

        // Step 5: Read column style information
        sColumnWidth = new String[nColCount];
        for (int nCol=0; nCol<nColCount; nCol++) {
            StyleWithProperties colStyle
                = wsc.getColumnStyle(table.getCol(nCol).getStyleName());
            if (colStyle!=null) {
                sColumnWidth[nCol]
                    = colStyle.getProperty(XMLString.STYLE_COLUMN_WIDTH);
            }
            if (sColumnWidth[nCol]==null) { // Emergency! should never happen!
                sColumnWidth[nCol]="2cm";
            }
        }

        // Step 6: Check to see, if this should be a longtable
        String sStyleName = Misc.getAttribute(table.getTableNode(),XMLString.TABLE_STYLE_NAME);
        StyleWithProperties style = wsc.getTableStyle(sStyleName);
        bIsLongtable = (!bIsSubtable) && bAllowLongtable &&
            !"false".equals(style.getProperty(XMLString.STYLE_MAY_BREAK_BETWEEN_ROWS));

    }

    /** is this a longtable? */
    public boolean isLongtable() { return bIsLongtable; }

    /** is this a subtable? (tbd: really belongs in TGM, move it there..) */
    public boolean isSubtable() { return bIsSubtable; }

    /**
     * <p>Create table environment based on table style.</p>
     * <p>Returns eg. "\begin{longtable}{p{2cm}|p{4cm}}", "\end{longtable}".</p>
     */
    public void applyTableStyle(BeforeAfter ba) {
        // Read formatting info from table style
        // Only supported properties are alignment and may-break-between-rows.
        String sStyleName = Misc.getAttribute(table.getTableNode(),XMLString.TABLE_STYLE_NAME);
        StyleWithProperties style = wsc.getTableStyle(sStyleName);
        char cAlign = 'c';
        if (style!=null && !bIsSubtable) {
            String s = style.getProperty(XMLString.TABLE_ALIGN);
            if ("left".equals(s)) { cAlign='l'; }
            else if ("right".equals(s)) { cAlign='r'; }
        }

        // Create table declaration
        if (bIsLongtable) {
            ba.add("\\begin{longtable}["+cAlign+"]", "\\end{longtable}");
        }
        else if (!bIsSubtable) {
            String s="center";
            switch (cAlign) {
                case 'c': s="center"; break;
                case 'r': s="flushright"; break;
                case 'l': s="flushleft";
            }
            ba.add("\\begin{"+s+"}\\begin{tabular}","\\end{tabular}\\end{"+s+"}");
        }
        else { // subtables should occupy the entire width, including padding!
            ba.add("\\hspace*{-\\tabcolsep}\\begin{tabular}",
                   "\\end{tabular}\\hspace*{-\\tabcolsep}");
        }

        // columns
        ba.add("{","");
        if (bGlobalVBorder[0]) { ba.add("|",""); }
        int nColCount = table.getColCount();
        for (int nCol=0; nCol<nColCount; nCol++){
            // note: The column width in OOo includes padding, which we subtract
            ba.add("p{"+Misc.add(sColumnWidth[nCol],"-0.2cm")+"}","");
            if (bGlobalVBorder[nCol+1]) { ba.add("|",""); }
        }
        ba.add("}","");
    }

    /** <p>Create interrow material</p> */
    public String getInterrowMaterial(int nRow) {
        int nColCount = table.getColCount();
        int nCount = 0;
        for (int nCol=0; nCol<nColCount; nCol++) {
            if (bHBorder[nRow][nCol]) { nCount++; }
        }
        if (nCount==0) { // no borders at this row
            return "";
        }
        else if (nCount==nColCount) { // complete set of borders
            return "\\hline";
        }
        else { // individual borders for each column
            StringBuffer buf = new StringBuffer();
            boolean bInCline = false;
            for (int nCol=0; nCol<nColCount; nCol++) {
                if (bInCline && !bHBorder[nRow][nCol]) { // close \cline
                    buf.append(nCol).append("}");
                    bInCline = false;
                }
                else if (!bInCline && bHBorder[nRow][nCol]) { // open \cline
                    buf.append("\\cline{").append(nCol+1).append("-");
                    bInCline = true;
                }
            }
            if (bInCline) { buf.append(nColCount).append("}"); }
            return buf.toString();
        }
    }

    /** <p>Get material to put before and after a table cell.
     *  In case of columnspan or different borders this will contain a \multicolumn command.
     *  If bApplyCellFormat is true, there will be a minipage environment</p>
     */
    public void applyCellStyle(int nRow, int nCol, BeforeAfter ba) {
        Node cell = table.getCell(nRow,nCol);
        int nColSpan = Misc.getPosInteger(Misc.getAttribute(cell,
                           XMLString.TABLE_NUMBER_COLUMNS_SPANNED),1);
        // Construct column declaration as needed
        boolean bNeedLeft = (nCol==0) && (bVBorder[nRow][0]!=bGlobalVBorder[0]);
        boolean bNeedRight = bVBorder[nRow][nCol+1]!=bGlobalVBorder[nCol+1];
        // calculate column width
        String sTotalColumnWidth = sColumnWidth[nCol];
        for (int i=nCol+1; i<nCol+nColSpan; i++) {
             sTotalColumnWidth = Misc.add(sTotalColumnWidth,sColumnWidth[i]);
        }
        sTotalColumnWidth = Misc.add(sTotalColumnWidth,"-0.2cm");

        if (bNeedLeft || bNeedRight || nColSpan>1) {
            ba.add("\\multicolumn{"+nColSpan+"}{","");
            if (nCol==0 && bVBorder[nRow][0]) { ba.add("|",""); }
            ba.add("p{"+sTotalColumnWidth+"}","");
            if (bVBorder[nRow][nCol+nColSpan]) { ba.add("|",""); }
            ba.add("}{","}");
        }

        if (bApplyCellFormat) {
            StyleWithProperties style = wsc.getCellStyle(Misc.getAttribute(cell,XMLString.TABLE_STYLE_NAME));
            String sValign = "c";
            if (style!=null) {
                String s = style.getProperty(XMLString.FO_VERTICAL_ALIGN);
                if ("".equals(s)) { sValign = "b"; } // seems that empty means top??
                else if ("top".equals(s)) { sValign = "b"; }
                else if ("bottom".equals(s)) { sValign = "t"; }
            }

            ba.add("\\begin{minipage}["+sValign+"]{"+sTotalColumnWidth+"}","\\end{minipage}");
        }
    }
}