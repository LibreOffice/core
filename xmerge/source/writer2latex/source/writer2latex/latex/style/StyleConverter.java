/************************************************************************
 *
 *  StyleConverter.java
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
 *  Version 0.3.3f (2004-09-16)
 *
 */

package writer2latex.latex.style;

import writer2latex.util.Config;
import writer2latex.util.ExportNameCollection;
import writer2latex.office.WriterStyleCollection;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.ConverterHelper;

/**
 *  <p>This is an abstract superclass for style converters.</p>
 */
public abstract class StyleConverter extends ConverterHelper {

    protected WriterStyleCollection wsc;
    protected ExportNameCollection styleNames = new ExportNameCollection(false);
    protected StyleMap styleMap = new StyleMap();
    protected LaTeXDocumentPortion declarations = new LaTeXDocumentPortion(false);

    protected StyleConverter(WriterStyleCollection wsc, Config config, ConverterPalette palette) {
        super(config,palette);
        this.wsc = wsc;
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        decl.append(declarations);
    }

}