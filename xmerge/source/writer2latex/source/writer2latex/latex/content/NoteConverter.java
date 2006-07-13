/************************************************************************
 *
 *  NoteConverter.java
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
 *  Version 0.3.3h (2004-12-09)
 *
 */

package writer2latex.latex.content;

import java.util.LinkedList;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.util.ExportNameCollection;
import writer2latex.office.XMLString;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.Context;

/**
 *  <p>This class handles conversion of footnotes and endnotes, including
 *  references.</p>
 */
public class NoteConverter extends ConverterHelper {

    private ExportNameCollection footnotenames = new ExportNameCollection(true);
    private ExportNameCollection endnotenames = new ExportNameCollection(true);
    private boolean bContainsEndnotes = false;
    private boolean bContainsFootnotes = false;
    // Keep track of footnotes (inside minipage etc.), that should be typeset later
    private LinkedList postponedFootnotes = new LinkedList();

    public NoteConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    /** <p>Append declarations needed by the <code>NoteConverter</code> to
     * the preamble.
     * @param <code>pack</code> the <code>LaTeXDocumentPortion</code> to which
     * declarations of packages should be added (<code>\\usepackage</code>).
     * @param <code>decl</code> the <code>LaTeXDocumentPortion</code> to which
     * other declarations should be added.
     */
    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bContainsEndnotes) { pack.append("\\usepackage{endnotes}").nl(); }
        if (bContainsFootnotes) palette.getNoteCc().convertFootnotesConfiguration(decl);
        if (bContainsEndnotes) palette.getNoteCc().convertEndnotesConfiguration(decl);
    }

    /** <p>Process a footnote (text:footnote tag)
     * @param <code>node</code> The element containing the footnote
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleFootnote(Element node, LaTeXDocumentPortion ldp, Context oc) {
        Context ic = (Context) oc.clone();
        ic.setInFootnote(true);

        String sId = node.getAttribute(XMLString.TEXT_ID);
        Element fntbody = Misc.getChildByTagName(node,XMLString.TEXT_FOOTNOTE_BODY);
        if (fntbody != null) {
            bContainsFootnotes = true;
            if (ic.isNoFootnotes()) {
                ldp.append("\\footnotemark{}");
                postponedFootnotes.add(fntbody);
            }
            else {
                ldp.append("\\footnote");
                ldp.append("{");
                if (sId != null && footnotenames.containsName(sId)) {
                    ldp.append("\\label{fnt:"+footnotenames.getExportName(sId)+"}");
                }
                traverseNoteBody(fntbody,ldp,ic);
                ldp.append("}");
            }
        }
    }

    /** Flush the queue of postponed footnotes */
    public void flushFootnotes(LaTeXDocumentPortion ldp, Context oc) {
        // We may still be in a context with no footnotes
        if (oc.isNoFootnotes()) { return; }
        // Type out all postponed footnotes:
        Context ic = (Context) oc.clone();
        ic.setInFootnote(true);
        int n = postponedFootnotes.size();
        if (n==1) {
            ldp.append("\\footnotetext{");
            traverseNoteBody((Element) postponedFootnotes.get(0),ldp,ic);
            ldp.append("}").nl();
            postponedFootnotes.clear();
        }
        else if (n>1) {
            // Several footnotes; have to adjust the footnote counter
            ldp.append("\\addtocounter{footnote}{-"+n+"}").nl();
            for (int i=0; i<n; i++) {
                ldp.append("\\stepcounter{footnote}\\footnotetext{");
                traverseNoteBody((Element) postponedFootnotes.get(i),ldp,ic);
                ldp.append("}").nl();
            }
            postponedFootnotes.clear();
        }
    }

    /** <p>Process an endnote (text:endnote tag)
     * @param <code>node</code> The element containing the endnote
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleEndnote(Element node, LaTeXDocumentPortion ldp, Context oc) {
        Context ic = (Context) oc.clone();
        ic.setInFootnote(true);

        String sId = node.getAttribute(XMLString.TEXT_ID);
        Element entbody = Misc.getChildByTagName(node,XMLString.TEXT_ENDNOTE_BODY);
        if (entbody != null) {
            if (ic.isNoFootnotes() && !config.useEndnotes()) {
                ldp.append("\\footnotemark()");
                postponedFootnotes.add(entbody);
            }
            else {
                if (config.useEndnotes()) {
                    ldp.append("\\endnote");
                    bContainsEndnotes = true;
                }
                else {
                    ldp.append("\\footnote");
                    bContainsFootnotes = true;
                }
                ldp.append("{");
                if (sId != null && endnotenames.containsName(sId)) {
                    ldp.append("\\label{ent:"+endnotenames.getExportName(sId)+"}");
                }
                traverseNoteBody(entbody,ldp,ic);
                ldp.append("}");
            }
        }
    }

    /** <p>Insert the endnotes into the documents.
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * the endnotes should be added.
     */
    public void insertEndnotes(LaTeXDocumentPortion ldp) {
        if (bContainsEndnotes) {
            ldp.append("\\clearpage").nl()
               .append("\\theendnotes").nl();
        }
    }

    /** <p>Process a footnote reference (text:footnote-ref tag)
     * @param <code>node</code> The element containing the footnote reference
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleFootnoteRef(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFormat = node.getAttribute(XMLString.TEXT_REFERENCE_FORMAT);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if ("page".equals(sFormat) && sName!=null) {
            ldp.append("\\pageref{fnt:"+footnotenames.getExportName(sName)+"}");
        }
        else if ("text".equals(sFormat) && sName!=null) {
            ldp.append("\\ref{fnt:"+footnotenames.getExportName(sName)+"}");
        }
        else { // use current value
            palette.getInlineCv().traversePCDATA(node,ldp,oc);
        }
    }

    /** <p>Process an endnote reference (text:endnote-ref tag)
     * @param <code>node</code> The element containing the endnote reference
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleEndnoteRef(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFormat = node.getAttribute(XMLString.TEXT_REFERENCE_FORMAT);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if ("page".equals(sFormat) && sName!=null) {
            ldp.append("\\pageref{ent:"+endnotenames.getExportName(sName)+"}");
        }
        else if ("text".equals(sFormat) && sName!=null) {
            ldp.append("\\ref{ent:"+endnotenames.getExportName(sName)+"}");
        }
        else { // use current value
            palette.getInlineCv().traversePCDATA(node,ldp,oc);
        }
    }

    /** <p>Add a footnote name. The method <code>handleFootnote</code> includes
     * a <code>\label</code> only if the footnote name is already known to the
     * <code>NoteConverter</code>. Hence this method is invoked by the prepass
     * for each footnote reference. The end result is, that only necessary
     * labels will be included.
     * @param <code>sName</code> the name (id) of the footnote
     */
    public void addFootnoteName(String sName) { footnotenames.addName(sName); }

    /** <p>Add an endnote name. The method <code>handleEndnote</code> includes
     * a <code>\label</code> only if the endnote name is already known to the
     * <code>NoteConverter</code>. Hence this method is invoked by the prepass
     * for each endnote reference. The end result is, that only necessary
     * labels will be included.
     * @param <code>sName</code> the name (id) of the endnote
     */
    public void addEndnoteName(String sName) { endnotenames.addName(sName); }

    /*
     * Process the contents of a footnote or endnote
     * TODO: Merge with BlockConverter.traverseBlockText?
     */
    private void traverseNoteBody (Element node, LaTeXDocumentPortion ldp, Context oc) {
        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {
                Node childNode = nList.item(i);

                if (childNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element child = (Element)childNode;
                    String nodeName = child.getTagName();

                    palette.getInfo().addDebugInfo(child,ldp);

                    if (nodeName.equals(XMLString.TEXT_H)) {
                        palette.getBlockCv().handleHeading(child,ldp,oc);
                    }

                    if (nodeName.equals(XMLString.TEXT_P)) {
                        palette.getInlineCv().traverseInlineText(child,ldp,oc,false);
                        if (i<len-1) {
                            if (nList.item(i+1).getNodeName().startsWith(XMLString.TEXT_)) {
                                ldp.append("\\par ");
                            }
                            else {
                                ldp.nl();
                            }
                        }
                    }

                    if (nodeName.equals(XMLString.TEXT_ORDERED_LIST)) {
                        palette.getBlockCv().handleList(child,ldp,oc,true);
                    }

                    if (nodeName.equals(XMLString.TEXT_UNORDERED_LIST)) {
                        palette.getBlockCv().handleList(child,ldp,oc,false);
                    }
                }
            }
        }
    }

}