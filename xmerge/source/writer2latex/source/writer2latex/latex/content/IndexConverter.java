/************************************************************************
 *
 *  IndexConverter.java
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
 *  Version 0.3.3g (2004-11-04)
 *
 */

package writer2latex.latex.content;

import org.w3c.dom.Element;
import org.w3c.dom.Node;

import writer2latex.util.Misc;
import writer2latex.util.Config;

import writer2latex.office.XMLString;
import writer2latex.office.IndexMark;
import writer2latex.office.BibMark;

import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.Context;

import writer2latex.bibtex.BibTeXDocument;

/**
 *  <p>This class handles indexes (table of contents, list of tables, list of
 *  illustrations, object index, user index, alphabetical index, bibliography)
 *  as well as their associated index marks.</p>
 */
public class IndexConverter extends ConverterHelper {

    private BibTeXDocument bibDoc;

    private boolean bContainsAlphabeticalIndex = false;

    /** <p>Construct a new <code>IndexConverter</code>.
     * @param <code>config</code> the configuration to use
     * @param <code>palette</code> the <code>ConverterPalette</code> to link to
     * if such a document is created by the <code>IndexConverter</code>
     */
    public IndexConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    /** <p>Append declarations needed by the <code>IndexConverter</code> to
     * the preamble.
     * @param <code>pack</code> the <code>LaTeXDocumentPortion</code> to which
     * declarations of packages should be added (<code>\\usepackage</code>).
     * @param <code>decl</code> the <code>LaTeXDocumentPortion</code> to which
     * other declarations should be added.
     */
    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
       if (bContainsAlphabeticalIndex) {
            pack.append("\\usepackage{makeidx}").nl();
            decl.append("\\makeindex").nl();
        }
    }

    /** Process Table of Contents (text:table-of-content tag)
     * @param <code>node</code> The element containing the Table of Contents
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleTOC (Element node, LaTeXDocumentPortion ldp, Context oc) {
        /* TODO: Apply more formatting by modfification of \l@section etc.
        Something like this:
        \newcommand\l@section[2]{\@dottedtocline{1}{1.5em}{2.3em}{\textbf{#1}}{\textit{#2}}
        Textformatting is trivial; see article.cls for examples of more complicated
        formatting. Note: The section number can't be formatted indivdually.*/

        Element source = Misc.getChildByTagName(node,XMLString.TEXT_TABLE_OF_CONTENT_SOURCE);
        if (source!=null) {
            if ("chapter".equals(source.getAttribute(XMLString.TEXT_INDEX_SOURCE))) {
                 ldp.append("[Warning: Table of content (for this chapter) ignored!]").nl().nl();
            }
            else {
                int nLevel = Misc.getPosInteger(source.getAttribute(XMLString.TEXT_OUTLINE_LEVEL),1);
                ldp.append("\\setcounter{tocdepth}{"+nLevel+"}").nl();
                Element title = Misc.getChildByTagName(source,XMLString.TEXT_INDEX_TITLE_TEMPLATE);
                if (title!=null) {
                    ldp.append("\\renewcommand\\contentsname{");
                    palette.getInlineCv().traversePCDATA(title,ldp,oc);
                    ldp.append("}").nl();
                }
            }
        }
        ldp.append("\\tableofcontents").nl();
    }

    /** Process List of Illustrations (text:list-of-illustrations tag)
     * @param <code>node</code> The element containing the List of Illustrations
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleLOF (Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("[Warning: List of illustrations ignored]").nl().nl();
        // lpd.append("\\listoffigures").nl();
    }

    /** Process List of Tables (text:list-of-tables tag)
     * @param <code>node</code> The element containing the List of Tables
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleLOT (Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("[Warning: List of tables ignored]").nl().nl();
        // lpd.append("\\listoftables").nl();
    }

    /** Process Object Index (text:object index tag)
     * @param <code>node</code> The element containing the Object Index
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleObjectIndex (Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("[Warning: Object index ignored]").nl().nl();
    }

    /** Process User Index (text:user-index tag)
     * @param <code>node</code> The element containing the User Index
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleUserIndex (Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("[Warning: User index ignored]").nl().nl();
    }


    /** Process Bibliography (text:bibliography tag)
     * @param <code>node</code> The element containing the Bibliography
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleBibliography (Element node, LaTeXDocumentPortion ldp, Context oc) {
        if (config.useBibtex()) {
            if (bibDoc==null) { bibDoc = new BibTeXDocument(palette.getOutFileName()); }
            ldp.append("\\bibliographystyle{")
               .append(config.bibtexStyle())
               .append("}").nl();
            ldp.append("\\bibliography{")
               .append(bibDoc.getName())
               .append("}").nl();
        }
        else { // typeset current content
            Element body = Misc.getChildByTagName(node,XMLString.TEXT_INDEX_BODY);
            if (body!=null) {
                Element title = Misc.getChildByTagName(body,XMLString.TEXT_INDEX_TITLE);
                if (title!=null) { palette.getBlockCv().traverseBlockText(title,ldp,oc); }
                palette.getBlockCv().traverseBlockText(body,ldp,oc);
            }
        }
    }

    /** Process a Bibliography Mark (text:bibliography-mark tag)
     * @param <code>node</code> The element containing the Mark
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleBibliographyMark(Element node, LaTeXDocumentPortion ldp, Context oc) {
        if (config.useBibtex()) {
            if (bibDoc==null) { bibDoc = new BibTeXDocument(palette.getOutFileName()); }
            String sIdentifier = node.getAttribute(XMLString.TEXT_IDENTIFIER);
            if (!bibDoc.containsKey(sIdentifier)) {
                bibDoc.put(new BibMark(node));
            }
            ldp.append("\\cite{")
               .append(bibDoc.getExportName(sIdentifier))
               .append("}");
        }
        else { // use current value
            palette.getInlineCv().traverseInlineText(node,ldp,oc,false);
        }
    }

    /** Get the BibTeX document, if any (the document is only created if it's
     * specified in the configuration *and* the document contains bibliographic data).
     * @return the <code>BiBTeXDocument</code>, or null if it does not exist.
     */
    public BibTeXDocument getBibTeXDocument () {
        return bibDoc;
    }

    /** Process Alphabetical Index (text:alphabetical-index tag)
     * @param <code>node</code> The element containing the Alphabetical Index
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleAlphabeticalIndex (Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("\\printindex").nl();
        bContainsAlphabeticalIndex = true;
    }


    /** Process an Alphabetical Index Mark (text:alphabetical-index-mark{-start} tag)
     * @param <code>node</code> The element containing the Mark
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleAlphabeticalIndexMark(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sValue = IndexMark.getIndexValue(node);
        if (sValue!=null) {
            ldp.append("\\index{");
            String sKey1 = IndexMark.getKey1(node);
            if (sKey1!=null) {
                writeIndexText(sKey1,ldp,oc);
                ldp.append("!");
            }
            String sKey2 = IndexMark.getKey2(node);
            if (sKey2!=null) {
                writeIndexText(sKey2,ldp,oc);
                ldp.append("!");
            }
            writeIndexText(sValue,ldp,oc);
            ldp.append("}");
        }
    }

    // Helper: Write the text of an index mark, escaping special characters
    private void writeIndexText(String sText, LaTeXDocumentPortion ldp, Context oc) {
        String sTextOut = palette.getI18n().convert(sText,false,oc.getLang());
        //  need to escape !, @, | and ":
        int nLen = sTextOut.length();
        boolean bBackslash = false;
        for (int i=0; i<nLen; i++) {
            if (bBackslash) {
                ldp.append(sTextOut.substring(i,i+1));
                bBackslash = false;
            }
            else {
                switch (sTextOut.charAt(i)) {
                    case '\\' : bBackslash = true;
                                ldp.append("\\");
                                break;
                    case '!' :
                    case '@' :
                    case '|' :
                    case '"' : ldp.append("\"");
                    default : ldp.append(sTextOut.substring(i,i+1));
                }
            }
        }
    }

}