/************************************************************************
 *
 *  FieldConverter.java
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
 *  Version 0.3.3f (2004-10-01)
 *
 */

package writer2latex.latex.content;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import org.w3c.dom.Element;

import writer2latex.util.Config;
import writer2latex.util.ExportNameCollection;
import writer2latex.office.XMLString;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;

/**
 *  <p>This class handles text fields and links in the document.</p>
 */
public class FieldConverter extends ConverterHelper {

    // Links & references
    private ExportNameCollection targets = new ExportNameCollection(true);
    private ExportNameCollection refnames = new ExportNameCollection(true);
    private ExportNameCollection bookmarknames = new ExportNameCollection(true);
    private ExportNameCollection seqrefnames = new ExportNameCollection(true);

    private boolean bUsesPageCount = false;

    public FieldConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    /** <p>Append declarations needed by the <code>FieldConverter</code> to
     * the preamble.</p>
     * @param <code>pack</code> the <code>LaTeXDocumentPortion</code> to which
     * declarations of packages should be added (<code>\\usepackage</code>).
     * @param <code>decl</code> the <code>LaTeXDocumentPortion</code> to which
     * other declarations should be added.
     */
    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        // page count
        if (bUsesPageCount) {
            pack.append("\\usepackage{lastpage}").nl();
            // Known bug: If the document ends with \\newpage the pagecount will be one off.
            /*decl.append("% Page count").nl()
                .append("\\makeatletter").nl()
                .append("\\newcounter{pagecount}").nl()
                .append("\\def\\makep@gecount{\\immediate\\write\\@mainaux{\\string\\setcounter{pagecount}{\\arabic{page}}}}").nl()
                .append("\\AtEndDocument{\\makep@gecount}").nl()
                .append("\\makeatother").nl();*/
        }

        // use hyperref.sty
        if (config.useHyperref()){
            pack.append("\\usepackage{hyperref}").nl();
            pack.append("\\hypersetup{");
            if (config.getBackend()==Config.PDFTEX) pack.append("pdftex, ");
            else if (config.getBackend()==Config.DVIPS) pack.append("dvips, ");
            //else pack.append("hypertex");
            pack.append("colorlinks=true, linkcolor=blue, filecolor=blue, pagecolor=blue, urlcolor=blue");
            if (config.getBackend()==Config.PDFTEX) {
                pack.append(createPdfMeta("pdftitle",palette.getMetaData().getTitle()))
                    .append(createPdfMeta("pdfauthor",palette.getMetaData().getCreator()))
                    .append(createPdfMeta("pdfsubject",palette.getMetaData().getSubject()))
                    .append(createPdfMeta("pdfkeywords",palette.getMetaData().getKeywords()));
            }
            pack.append("}").nl();
        }
    }

    public void addSequenceName(String sName) {
        seqrefnames.addName(urlDecode(sName));
    }

    /** <p>Process a sequence field (text:sequence tag)</p>
     * @param <code>node</code> The element containing the sequence field
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleSequence(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // Temp solution: Use current value, but support page references
        palette.getInlineCv().traversePCDATA(node,ldp,oc);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if (sName!=null && seqrefnames.containsName(sName)) {
            ldp.append("\\label{seq:"+seqrefnames.getExportName(sName)+"}");
        }
    }

    /** <p>Process a sequence reference (text:sequence-ref tag)</p>
     * @param <code>node</code> The element containing the sequence reference
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleSequenceRef(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFormat = node.getAttribute(XMLString.TEXT_REFERENCE_FORMAT);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if ("page".equals(sFormat) && sName!=null) {
            ldp.append("\\pageref{seq:"+seqrefnames.getExportName(sName)+"}");
        }
        else { // use current value
            palette.getInlineCv().traversePCDATA(node,ldp,oc);
        }
    }


    public void addReferenceName(String sName) {
        refnames.addName(urlDecode(sName));
    }

    /** <p>Process a reference mark (text:reference-mark tag)</p>
     * @param <code>node</code> The element containing the reference mark
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleReferenceMark(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // Note: Always include \label here, even when it's not used
        String sName = node.getAttribute(XMLString.TEXT_NAME);
        if (sName!=null) {
            ldp.append("\\label{ref:"+refnames.getExportName(sName)+"}");
        }
    }

    /** <p>Process a reference (text:reference-ref tag)</p>
     * @param <code>node</code> The element containing the reference
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleReferenceRef(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFormat = node.getAttribute(XMLString.TEXT_REFERENCE_FORMAT);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if ("page".equals(sFormat) && sName!=null) {
            ldp.append("\\pageref{ref:"+refnames.getExportName(sName)+"}");
        }
        else { // use current value
            palette.getInlineCv().traversePCDATA(node,ldp,oc);
        }
    }

    public void addBookmarkName(String sName) {
        bookmarknames.addName(urlDecode(sName));
    }

    /** <p>Process a bookmark (text:bookmark tag)</p>
     * <p>A bookmark may be the target for either a hyperlink or a reference,
     * so this will generate a <code>\\hyperref</code> and/or a <code>\\label</code></p>
     * @param <code>node</code> The element containing the bookmark
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleBookmark(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sName = node.getAttribute(XMLString.TEXT_NAME);
        if (sName!=null) {
            // A bookmark may be used as a target for a hyperlink as well as
            // for a reference. We export whatever is actually used:
            addTarget(node,"",ldp);
            if (bookmarknames.containsName(sName)) {
                ldp.append("\\label{bkm:"+bookmarknames.getExportName(sName)+"}");
            }
        }
    }

    /** <p>Process a bookmark reference (text:bookmark-ref tag).</p>
     * @param <code>node</code> The element containing the bookmark reference
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleBookmarkRef(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFormat = node.getAttribute(XMLString.TEXT_REFERENCE_FORMAT);
        String sName = node.getAttribute(XMLString.TEXT_REF_NAME);
        if ("page".equals(sFormat) && sName!=null) {
            ldp.append("\\pageref{bkm:"+bookmarknames.getExportName(sName)+"}");
        }
        else { // use current value
            palette.getInlineCv().traversePCDATA(node,ldp,oc);
        }
    }

    /** <p>Process a hyperlink (text:a tag)</p>
     * @param <code>node</code> The element containing the hyperlink
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleAnchor(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sHref = node.getAttribute(XMLString.XLINK_HREF);
        if (sHref!=null) {
            if (sHref.startsWith("#")) {
                // TODO: hyperlinks to headings (?) and objects
                if (config.useHyperref()) {
                    ldp.append("\\hyperlink{")
                       .append(targets.getExportName(urlDecode(sHref.substring(1))))
                       .append("}{");
                    // ignore text style (let hyperref.sty handle the decoration):
                    palette.getInlineCv().traverseInlineText(node,ldp,oc,false);
                    ldp.append("}");
                }
                else { // user don't want to include hyperlinks
                    palette.getInlineCv().traverseInlineText(node,ldp,oc,true);
                }
            }
            else {
                if (config.useHyperref()) {
                    ldp.append("\\href{").append(urlDecode(sHref)).append("}{");
                    // ignore text style (let hyperref.sty handle the decoration):
                    palette.getInlineCv().traverseInlineText(node,ldp,oc,false);
                    ldp.append("}");
                }
                else { // user don't want to include hyperlinks
                    palette.getInlineCv().traverseInlineText(node,ldp,oc,true);
                }
            }
        }
        else {
            palette.getInlineCv().traverseInlineText(node,ldp,oc,true);
        }
    }

    public void addTargetName(String sName) {
        targets.addName(urlDecode(sName));
    }

    /** <p>Add a <code>\\hypertarget</code></p>
     * @param <code>node</code> The element containing the name of the target
     * @param <code>sSuffix</code> A suffix to be added to the target,
     * e.g. "|table" for a reference to a table.
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     */
    public void addTarget(Element node, String sSuffix, LaTeXDocumentPortion ldp) {
        String sName = node.getAttribute(XMLString.TEXT_NAME);
        if (sName == null) { sName = node.getAttribute(XMLString.TABLE_NAME); }
        if (sName == null || !config.useHyperref()) { return; }
        if (!targets.containsName(sName+sSuffix)) { return; }
        ldp.append("\\hypertarget{")
           .append(targets.getExportName(sName+sSuffix))
           .append("}{}");
    }

    /** <p>Process a page number field (text:page-number tag)</p>
     * @param <code>node</code> The element containing the page number field
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handlePageNumber(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // TODO: Obey attributes!
        ldp.append("\\thepage{}");
    }

    /** <p>Process a page count field (text:page-count tag)</p>
     * @param <code>node</code> The element containing the page count field
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handlePageCount(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // TODO: Obey attributes!
        // Note: Actually LastPage refers to the page number of the last page, not the number of pages
        if (config.useLastpage()) {
            bUsesPageCount = true;
            ldp.append("\\pageref{LastPage}");
        }
        else {
            ldp.append("?");
        }
    }

    // Helpers:

    private String createPdfMeta(String sName, String sValue) {
        if (sValue==null) { return ""; }
        // Replace commas with semicolons (the keyval package doesn't like commas):
        sValue = sValue.replace(',', ';');
        // Meta data is assumed to be in the default language:
        return ", "+sName+"="+palette.getI18n().convert(sValue,false,palette.getMainContext().getLang());
    }

    private String urlDecode(String s) {
        try {
            return URLDecoder.decode(s,"UTF-8");
        }
        catch (UnsupportedEncodingException e) {
            return "";
        }
    }




}