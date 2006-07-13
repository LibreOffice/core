/************************************************************************
 *
 *  SectionStyleConverter.java
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
 *  Version 0.3.3g (2004-11-25)
 *
 */

package writer2latex.latex.style;

import writer2latex.util.*;
import writer2latex.office.*;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.Context;
import writer2latex.latex.ConverterPalette;

/** <p>This class creates LaTeX code from OOo section formatting.
 *  <p>Currently this means number of columns only
 */
public class SectionStyleConverter extends StyleConverter {

    // Do we need multicols.sty?
    private boolean bNeedMulticol = false;

    /** <p>Constructs a new <code>SectionStyleConverter</code>.</p>
     */
    public SectionStyleConverter(WriterStyleCollection wsc, Config config,
        ConverterPalette palette) {
        super(wsc,config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bNeedMulticol) { pack.append("\\usepackage{multicol}").nl(); }
    }

    public void applySectionStyle(String sStyleName, BeforeAfter ba, Context context) {
        SectionStyle style = wsc.getSectionStyle(sStyleName);
        // Don't nest multicols and require at least 2 columns
        if (context.isInMulticols() || style==null || style.getColCount()<2) { return; }
        int nCols = style.getColCount();
        bNeedMulticol = true;
        context.setInMulticols(true);
        ba.add("\\begin{multicols}{"+(nCols>10 ? 10 : nCols)+"}", "\\end{multicols}");
    }


}
