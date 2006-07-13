/************************************************************************
 *
 *  MathmlConverter.java
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
 *  Version 0.3.3i (2004-12-28)
 *
 */

package writer2latex.latex.content;

import java.util.Hashtable;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import writer2latex.office.*;
import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.style.I18n;

/**
 *  <p>This class converts mathml nodes to LaTeX.
 *  (Actually it only converts the starmath annotation currently, if available).</p>
 */
public final class MathmlConverter extends ConverterHelper {

    private StarMathConverter smc;

    private boolean bContainsFormulas = false;

    public MathmlConverter(Config config, ConverterPalette palette) {
        super(config,palette);
        smc = new StarMathConverter(palette.getI18n(),config);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bContainsFormulas) {
            if (config.useOoomath()) {
                pack.append("\\usepackage{ooomath}").nl();
            }
            else {
                smc.appendDeclarations(pack,decl);
            }
            // Until reading settings for formulas is implemented, we need this
            decl.append("\\everymath{\\displaystyle}").nl();
        }
    }

    public String convert(Node settings, Node formula) {
        // TODO: Use settings to determine display mode/text mode
        // formula must be a math:math node
        // First try to find a StarMath annotation
        Node semantics = Misc.getChildByTagName(formula,XMLString.MATH_SEMANTICS);
        if (semantics!=null) {
            Node annotation = Misc.getChildByTagName(semantics,XMLString.MATH_ANNOTATION);
            if (annotation!=null) {
                String sStarMath = "";
                if (annotation.hasChildNodes()) {
                    NodeList anl = annotation.getChildNodes();
                    int nLen = anl.getLength();
                    for (int i=0; i<nLen; i++) {
                        if (anl.item(i).getNodeType() == Node.TEXT_NODE) {
                            sStarMath+=anl.item(i).getNodeValue();
                        }
                    }
                    bContainsFormulas = true;
                    return smc.convert(sStarMath);
                }
            }
        }
        // No annotation was found. In this case we should convert the mathml,
        // but currently we ignore the problem.
        // TODO: Investigate if Vasil I. Yaroshevich's MathML->LaTeX
        // XSL transformation could be used here. (Potential problem:
        // OOo uses MathML 1.01, not MathML 2)
        return "\\text{Warning: No StarMath annotation}";
    }

}