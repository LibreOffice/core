/************************************************************************
 *
 *  TableStyleConverter.java
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
 *  Version 0.3.3f (2004-09-17)
 *
 */

package writer2latex.latex.style;

import writer2latex.util.*;
import writer2latex.office.*;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

/** <p>This class creates LaTeX code from OOo table styles.
 */
public class TableStyleConverter extends StyleConverter {

    private boolean bNeedLongtable = false;
    private boolean bContainsTables = false;

    /** <p>Constructs a new <code>TableStyleConverter</code>.</p>
     */
    public TableStyleConverter(WriterStyleCollection wsc, Config config,
        ConverterPalette palette) {
        super(wsc,config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bNeedLongtable) { pack.append("\\usepackage{longtable}").nl(); }
        // Set padding for table cells (1mm is default in OOo!)
        // For vertical padding we can only specify a relative size
        if (bContainsTables) {
            decl.append("\\setlength\\tabcolsep{1mm}").nl();
            decl.append("\\renewcommand\\arraystretch{1.3}").nl();
        }
    }

    public TableFormatter getTableFormatter(TableGridModel table,
        boolean bAllowLongtable, boolean bApplyCellFormat) {

        TableFormatter formatter = new TableFormatter(wsc,table,bAllowLongtable,bApplyCellFormat);
        bContainsTables = true;
        bNeedLongtable |= formatter.isLongtable();
        return formatter;
    }


}
