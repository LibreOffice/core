/************************************************************************
 *
 *  ConverterHelper.java
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
 *  Version 0.3.3f (2004-08-29)
 *
 */

package writer2latex.latex;

import writer2latex.util.Config;
import writer2latex.latex.LaTeXDocumentPortion;

/**
 *  <p>This is an abstract superclass for converter helpers.</p>
 */
public abstract class ConverterHelper {

    protected Config config;
    protected ConverterPalette palette;

    protected ConverterHelper(Config config, ConverterPalette palette) {
        this.config = config;
        this.palette = palette;
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
    }

}