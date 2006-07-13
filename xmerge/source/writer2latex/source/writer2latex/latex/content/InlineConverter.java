/************************************************************************
 *
 *  InlineConverter.java
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

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.office.XMLString;
import writer2latex.latex.style.BeforeAfter;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.HeadingMap;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;

/**
 *  <p>This class handles basic inline text.</p>
 */
public class InlineConverter extends ConverterHelper {

    public InlineConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    /*
     * Process normal inline text
     */
    public void traverseInlineText (Element node, LaTeXDocumentPortion ldp,
            Context oc, boolean styled) {

        // don't style it if a {foot|end}note is the only content
        if (onlyNote(node)) { styled=false; }

        boolean bNoFootnotes = false;

        String styleName = node.getAttribute(XMLString.TEXT_STYLE_NAME);

        // Always push the font used
        palette.getI18n().pushSpecialTable(palette.getCharSc().getFontName(styleName));

        // Apply the style
        BeforeAfter ba = new BeforeAfter();
        Context ic = (Context) oc.clone();
        palette.getCharSc().applyTextStyle(styleName,ba,ic);

        // Footnote problems:
        // No footnotes in sub/superscript (will disappear)
        // No multiparagraph footnotes embedded in text command (eg. \textbf{..})
        // Simple solution: styled text element is forbidden footnote area
        if (styled && !ic.isInFootnote()) { bNoFootnotes = true; }
        //String s = style.getProperty(XMLString.STYLE_TEXT_POSITION);
        //if (s!=null && !bProcessingFootnote) { bNoFootnotes = true; }

        // Temp solution: Ignore hard formatting in header/footer (name clash problem)
        // only in package format.
        //TODO: Reenable this!!!
        /*if (sxwDoc.getStyleDOM()!=null && palette.getCharSc().isAutomatic(styleName) && ic.isInHeaderFooter()) {
            styled = false;
        }*/

        if (styled) {
            if (bNoFootnotes) { ic.setNoFootnotes(true); }
            ldp.append(ba.getBefore());
        }

        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {

                Node childNode = nList.item(i);
                short nodeType = childNode.getNodeType();

                switch (nodeType) {
                    case Node.TEXT_NODE:
                        String s = childNode.getNodeValue();
                        if (s.length() > 0) {
                            ldp.append(palette.getI18n().convert(s,false,ic.getLang()));
                        }
                        break;

                    case Node.ELEMENT_NODE:
                        Element child = (Element)childNode;
                        String sName = child.getTagName();
                        if (child.getNodeName().startsWith("draw:")) {
                            palette.getDrawCv().handleDrawElement(child,ldp,ic);
                        }
                        else if (sName.equals(XMLString.TEXT_S)) {
                            if (config.ignoreDoubleSpaces()) {
                                ldp.append(" ");
                            }
                            else {
                                int count= Misc.getPosInteger(child.getAttribute(XMLString.TEXT_C),1);
                                String sSpace = config.ignoreDoubleSpaces() ? " " : "\\ ";
                                for ( ; count > 0; count--) { ldp.append("\\ "); }
                            }
                        }
                        else if (sName.equals(XMLString.TEXT_TAB_STOP)) {
                            // tab stops are not supported by the onverter, but the special usage
                            // of tab stops in header and footer can be emulated with \hfill
                            // TODO: Sometimes extra \hfill should be added at end of line
                            if (ic.isInHeaderFooter()) { ldp.append("\\hfill "); }
                            else { ldp.append(" "); }
                        }
                        else if (sName.equals(XMLString.TEXT_LINE_BREAK)) {
                            if (!ic.isInHeaderFooter() && !config.ignoreHardLineBreaks()) {
                                ldp.append("\\newline").nl();
                            }
                            else { ldp.append(" "); }
                        }
                        else if (sName.equals(XMLString.TEXT_SPAN)) {
                            if (ic.isVerbatim()) {
                                traverseVerbatimInlineText(child,ldp,ic,true);
                            }
                            else {
                                traverseInlineText (child,ldp,ic,true);
                            }
                        }
                        else if (sName.equals(XMLString.TEXT_A)) {
                            palette.getFieldCv().handleAnchor(child,ldp,ic);
                        }
                        else if (sName.equals(XMLString.OFFICE_ANNOTATION)) {
                            handleOfficeAnnotation(child,ldp,ic);
                        }
                        else if (sName.equals(XMLString.TEXT_PAGE_NUMBER)) {
                            palette.getFieldCv().handlePageNumber(child,ldp,ic);
                        }
                        else if (sName.equals(XMLString.TEXT_PAGE_COUNT)) {
                            palette.getFieldCv().handlePageCount(child,ldp,ic);
                        }
                        else if (ic.isInHeaderFooter()) {
                            if (sName.equals(XMLString.TEXT_CHAPTER)) {
                                handleChapterField(child,ldp,ic);
                            }
                            else if (sName.startsWith("text:")) {
                                traverseInlineText(child,ldp,ic,false);
                            }
                        }
                        else {
                            // These tags are ignored in header and footer
                            if (sName.equals(XMLString.TEXT_FOOTNOTE)) {
                                palette.getNoteCv().handleFootnote(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_ENDNOTE)) {
                                palette.getNoteCv().handleEndnote(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_SEQUENCE)) {
                                palette.getFieldCv().handleSequence(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_SEQUENCE_REF)) {
                                palette.getFieldCv().handleSequenceRef(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_FOOTNOTE_REF)) {
                                palette.getNoteCv().handleFootnoteRef(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_ENDNOTE_REF)) {
                                palette.getNoteCv().handleEndnoteRef(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_REFERENCE_MARK)) {
                                palette.getFieldCv().handleReferenceMark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_REFERENCE_MARK_START)) {
                                palette.getFieldCv().handleReferenceMark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_REFERENCE_REF)) {
                                palette.getFieldCv().handleReferenceRef(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_BOOKMARK)) {
                                palette.getFieldCv().handleBookmark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_BOOKMARK_START)) {
                                palette.getFieldCv().handleBookmark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_BOOKMARK_REF)) {
                                palette.getFieldCv().handleBookmarkRef(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_BIBLIOGRAPHY_MARK)) {
                                palette.getIndexCv().handleBibliographyMark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_ALPHABETICAL_INDEX_MARK)) {
                                palette.getIndexCv().handleAlphabeticalIndexMark(child,ldp,ic);
                            }
                            else if (sName.equals(XMLString.TEXT_ALPHABETICAL_INDEX_MARK_START)) {
                                palette.getIndexCv().handleAlphabeticalIndexMark(child,ldp,ic);
                            }
                            else if (sName.startsWith("text:")) {
                                traverseInlineText(child,ldp,ic,false);
                            }
                        }
                        break;
                    default:
                        // Do nothing
                }
            }
        }

        if (styled) {
            ldp.append(ba.getAfter());
            ic.setNoFootnotes(false);
            if (!ic.isInFootnote()) { palette.getNoteCv().flushFootnotes(ldp,oc); }
        }

        // finally pop the special table
        palette.getI18n().popSpecialTable();
    }

    /* traverse inline text, ignoring any draw objects, footnotes, formatting and hyperlinks */
    public void traversePlainInlineText(Element node,LaTeXDocumentPortion ldp, Context oc) {
        if (node.hasChildNodes()) {

            // Always push the font used
            String styleName = node.getAttribute(XMLString.TEXT_STYLE_NAME);
            palette.getI18n().pushSpecialTable(palette.getCharSc().getFontName(styleName));

            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {

                Node childNode = nList.item(i);
                short nodeType = childNode.getNodeType();

                switch (nodeType) {
                    case Node.TEXT_NODE:
                        String s = childNode.getNodeValue();
                        if (s.length() > 0) {
                            ldp.append(palette.getI18n().convert(s,false,oc.getLang()));
                        }
                        break;

                    case Node.ELEMENT_NODE:
                        Element child = (Element)childNode;
                        String sName = child.getTagName();
                        if (sName.equals(XMLString.TEXT_S)) {
                            int count= Misc.getPosInteger(child.getAttribute(XMLString.TEXT_C),1);
                            for ( ; count > 0; count--) {
                                ldp.append("\\ ");
                            }
                        }
                        else if (sName.equals(XMLString.TEXT_TAB_STOP)) {
                            // tab stops are not supported by the onverter
                            ldp.append(" ");
                        }
                        else if (sName.equals(XMLString.TEXT_FOOTNOTE)) {
                            // ignore
                        }
                        else if (sName.equals(XMLString.TEXT_ENDNOTE)) {
                            // ignore
                        }
                        else if (sName.startsWith("text:")) {
                            traversePlainInlineText(child,ldp,oc);
                        }
                        break;
                    default:
                        // Do nothing
                }
            }
            // finally pop the special table
            palette.getI18n().popSpecialTable();
        }
    }

    /* traverse verbatim inline text, ignoring any draw objects, footnotes, formatting and hyperlinks */
    public void traverseVerbatimInlineText(Element node,LaTeXDocumentPortion ldp, Context oc, boolean bInline) {
        if (node.hasChildNodes()) {

            NodeList nList = node.getChildNodes();
            int len = nList.getLength();

            for (int i = 0; i < len; i++) {

                Node childNode = nList.item(i);
                short nodeType = childNode.getNodeType();

                switch (nodeType) {
                    case Node.TEXT_NODE:
                        String s = childNode.getNodeValue();
                        if (s.length() > 0) {
                             // text is copied verbatim! (Will be replaced by
                             // question marks if outside inputenc)
                            ldp.append(s);
                        }
                        break;

                    case Node.ELEMENT_NODE:
                        Element child = (Element)childNode;
                        String sName = child.getTagName();
                        if (sName.equals(XMLString.TEXT_S)) {
                            int count= Misc.getPosInteger(child.getAttribute(XMLString.TEXT_C),1);
                            for ( ; count > 0; count--) {
                                ldp.append(" ");
                            }
                        }
                        else if (sName.equals(XMLString.TEXT_TAB_STOP)) {
                            // tab stops are not supported by the onverter
                            ldp.append(" ");
                        }
                        else if (sName.equals(XMLString.TEXT_LINE_BREAK)) {
                            if (!bInline) { ldp.nl(); }
                        }
                        else if (sName.equals(XMLString.TEXT_FOOTNOTE)) {
                            // ignore
                        }
                        else if (sName.equals(XMLString.TEXT_ENDNOTE)) {
                            // ignore
                        }
                        else if (sName.startsWith("text:")) {
                            traverseVerbatimInlineText(child,ldp,oc,bInline);
                        }
                        break;
                    default:
                        // Do nothing
                }
            }
        }
    }

    public void traversePCDATA(Element node, LaTeXDocumentPortion ldp, Context oc) {
        if (node.hasChildNodes()) {
            NodeList nl = node.getChildNodes();
            int nLen = nl.getLength();
            for (int i=0; i<nLen; i++) {
                if (nl.item(i).getNodeType()==Node.TEXT_NODE) {
                    ldp.append(palette.getI18n().convert(nl.item(i).getNodeValue(),false,oc.getLang()));
                }
            }
        }
    }

    private void handleChapterField(Element node, LaTeXDocumentPortion ldp, Context oc) {
        HeadingMap hm = config.getHeadingMap();
        int nLevel = Misc.getPosInteger(node.getAttribute(XMLString.TEXT_OUTLINE_LEVEL),1);
        if (nLevel<=hm.getMaxLevel()) {
            int nLaTeXLevel = hm.getLevel(nLevel);
            if (nLaTeXLevel==1) {
                palette.getPageSc().setChapterField1(node.getAttribute(XMLString.TEXT_DISPLAY));
                ldp.append("{\\leftmark}");
            }
            else if (nLaTeXLevel==2) {
                palette.getPageSc().setChapterField2(node.getAttribute(XMLString.TEXT_DISPLAY));
                ldp.append("{\\rightmark}");
            }
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Annotations

    private void handleOfficeAnnotation(Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("%").nl().append("%");
        Element paragraph = Misc.getChildByTagName(node,XMLString.TEXT_P);
        if (paragraph!=null) { traversePCDATA(paragraph,ldp,oc); }
        ldp.nl();
    }

    /* Check to see if this node has a footnote or endnote as the only subnode */
    private boolean onlyNote(Node node) {
        if (!node.hasChildNodes()) { return false; }
        NodeList nList = node.getChildNodes();
        int nLen = nList.getLength();

        for (int i = 0; i < nLen; i++) {

            Node child = nList.item(i);
            short nType = child.getNodeType();
            String sName = child.getNodeName();

            switch (nType) {
                case Node.TEXT_NODE: return false;
                case Node.ELEMENT_NODE:
                    if (!XMLString.TEXT_FOOTNOTE.equals(sName) &&
                        !XMLString.TEXT_ENDNOTE.equals(sName)) { return false; }
            }
        }
        return true;
    }

}