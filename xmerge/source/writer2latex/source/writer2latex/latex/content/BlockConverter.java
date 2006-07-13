/************************************************************************
 *
 *  BlockConverter.java
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
 *  Version 0.3.3g (2004-11-11)
 *
 */

package writer2latex.latex.content;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.office.XMLString;
import writer2latex.office.TableGridModel;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.HeadingMap;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.style.BeforeAfter;
import writer2latex.latex.style.StyleMap;
import writer2latex.latex.style.TableFormatter;

/**
 *  <p>This class handles basic block content, including the main text body,
 *  sections, tables, lists, headings and paragraphs.</p>
 */
public class BlockConverter extends ConverterHelper {

    public BlockConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    /** <p> Traverse block text (eg. content of body, section, list item).
     * This is traversed in logical order and dedicated handlers take care of
     * each block element.</p>
     * <p> (Note: As a rule, all handling of block level elements should add a
     * newline to the LaTeX document at the end of the block)</p>
     * @param <code>node</code> The element containing the block text
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void traverseBlockText(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // The current paragraph block:
        StyleMap blockMap = config.getParBlockStyleMap();
        String sBlockName = null;

        if (node.hasChildNodes()) {
            NodeList list = node.getChildNodes();
            int nLen = list.getLength();

            for (int i = 0; i < nLen; i++) {
                Node childNode = list.item(i);

                if (childNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element child = (Element)childNode;
                    String sTagName = child.getTagName();

                    palette.getInfo().addDebugInfo(child,ldp);

                    // Start/End a paragraph block
                    if (sTagName.equals(XMLString.TEXT_P)) {
                        String sStyleName = child.getAttribute(XMLString.TEXT_STYLE_NAME);
                        if (sBlockName!=null && !blockMap.isNext(sBlockName,sStyleName)) {
                            // end current block
                            String sAfter = blockMap.getAfter(sBlockName);
                            if (sAfter.length()>0) ldp.append(sAfter).nl();
                            sBlockName = null;
                        }
                        if (sBlockName==null && blockMap.contains(sStyleName)) {
                            // start a new block
                            sBlockName = sStyleName;
                            String sBefore = blockMap.getBefore(sBlockName);
                            if (sBefore.length()>0) ldp.append(sBefore).nl();
                        }
                    }
                    else if (sBlockName!=null) {
                        // non-paragraph: end current block
                        String sAfter = blockMap.getAfter(sBlockName);
                        if (sAfter.length()>0) ldp.append(sAfter).nl();
                        sBlockName = null;
                    }


                    // Basic block content; handle by this class
                    if (sTagName.equals(XMLString.TEXT_P)) {
                        handleParagraph(child,ldp,oc,i==nLen-1);
                    }

                    else if(sTagName.equals(XMLString.TEXT_H)) {
                        handleHeading(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_UNORDERED_LIST)) {
                        handleList(child,ldp,oc,false);
                    }

                    else if (sTagName.equals(XMLString.TEXT_ORDERED_LIST)) {
                        handleList(child,ldp,oc,true);
                    }
                    else if (sTagName.equals(XMLString.TABLE_TABLE)) {
                        handleTable(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TABLE_SUB_TABLE)) {
                        handleTable(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_SECTION)) {
                        handleSection(child,ldp,oc);
                    }

                    // Draw elements may appear in block context if they are
                    // anchored to page
                    else if (sTagName.startsWith("draw:")) {
                        palette.getDrawCv().handleDrawElement(child,ldp,oc);
                    }

                    // Indexes
                    else if (sTagName.equals(XMLString.TEXT_TABLE_OF_CONTENT)) {
                        palette.getIndexCv().handleTOC(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_ILLUSTRATION_INDEX)) {
                        palette.getIndexCv().handleLOF(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_TABLE_INDEX)) {
                        palette.getIndexCv().handleLOT(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_OBJECT_INDEX)) {
                        palette.getIndexCv().handleObjectIndex(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_USER_INDEX)) {
                        palette.getIndexCv().handleUserIndex(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_ALPHABETICAL_INDEX)) {
                        palette.getIndexCv().handleAlphabeticalIndex(child,ldp,oc);
                    }

                    else if (sTagName.equals(XMLString.TEXT_BIBLIOGRAPHY)) {
                        palette.getIndexCv().handleBibliography(child,ldp,oc);
                    }

                    // Sequence declarations appear in the main text body
                    else if (sTagName.equals(XMLString.TEXT_SEQUENCE_DECLS)) {
                        //handleSeqeuenceDecls(child);
                    }
                    // other tags are ignored
                }
            }
        }

        if (sBlockName!=null) {
            // end current block
            String sAfter = blockMap.getAfter(sBlockName);
            if (sAfter.length()>0) ldp.append(sAfter).nl();
            sBlockName = null;
        }

    }

    /** <p> Process a section (text:section tag)</p>
     * @param <code>node</code> The element containing the section
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleSection(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // TODO: Support linked sections

        // We may need a hyperlink target
        palette.getFieldCv().addTarget(node,"|region",ldp);

        // Apply the style
        String sStyleName = node.getAttribute(XMLString.TEXT_STYLE_NAME);
        BeforeAfter ba = new BeforeAfter();
        Context ic = (Context) oc.clone();
        palette.getSectionSc().applySectionStyle(sStyleName,ba,ic);

        // Do conversion
        if (ba.getBefore().length()>0) { ldp.append(ba.getBefore()).nl(); }
        traverseBlockText(node,ldp,ic);
        if (ba.getAfter().length()>0) { ldp.append(ba.getAfter()).nl(); }
    }

    /** <p> Process a table (table:table or table:sub-table tag)</p>
     * @param <code>node</code> The element containing the table
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     */
    public void handleTable(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // Get a grid model of the table
        TableGridModel table = new TableGridModel(node);

        // Update the context
        Context ic = (Context) oc.clone();
        ic.setInTable(true);

        // Apply table style
        boolean bApplyCellFormat = config.formatting()>=Config.CONVERT_MOST;
        TableFormatter formatter = palette.getTableSc().getTableFormatter(table,
            !ic.isInMulticols(), bApplyCellFormat);

        // We may need a hyperlink target
        if (!formatter.isSubtable()) {
            palette.getFieldCv().addTarget(node,"|table",ldp);
        }

        // Export table declaration
        BeforeAfter baTable = new BeforeAfter();
        formatter.applyTableStyle(baTable);
        ldp.append(baTable.getBefore()).nl();

        // Export rows
        if (bApplyCellFormat || !formatter.isLongtable()) { ic.setNoFootnotes(true); }
        int nRowCount = table.getRowCount();
        int nColCount = table.getColCount();
        boolean bInHeader = false;
        // Add interrow material before first row:
        ldp.append(formatter.getInterrowMaterial(0)).nl();
        for (int nRow=0; nRow<nRowCount; nRow++){
            // Longtables may have headers:
            if (formatter.isLongtable()) {
                if (nRow==0 && table.getRow(nRow).isHeader()) {
                    bInHeader = true;
                    if (!bApplyCellFormat) { ic.setNoFootnotes(true); }
                }
                else if (bInHeader && !table.getRow(nRow).isHeader()) {
                    bInHeader = false;
                    if (!bApplyCellFormat) { ic.setNoFootnotes(false); }
                    palette.getNoteCv().flushFootnotes(ldp,oc);
                    ldp.append("\\endhead").nl();
                }
            }

            // Export columns in this row
            int nCol = 0;
            while (nCol<nColCount) {
                Element cell = (Element) table.getCell(nRow,nCol);
                if (XMLString.TABLE_TABLE_CELL.equals(cell.getNodeName())) {
                    BeforeAfter baCell = new BeforeAfter();
                    formatter.applyCellStyle(nRow,nCol,baCell);
                    ldp.append(baCell.getBefore());
                    traverseBlockText(cell,ldp,ic);
                    ldp.append(baCell.getAfter());
                }
                // Otherwise ignore; the cell is covered by a \multicolumn entry.
                // (table:covered-table-cell)
                int nColSpan = Misc.getPosInteger(cell.getAttribute(
                                   XMLString.TABLE_NUMBER_COLUMNS_SPANNED),1);
                if (nCol+nColSpan<nColCount) { ldp.append("&").nl(); }
                nCol+=nColSpan;
            }
            ldp.append("\\\\").append(formatter.getInterrowMaterial(nRow+1)).nl();
        }

        // End table
        ldp.append(baTable.getAfter()).nl();

        // Footnotes are now allowed
        if (bApplyCellFormat || !formatter.isLongtable()) {
            palette.getNoteCv().flushFootnotes(ldp,oc);
        }
    }


    /** <p> Process a list (text:ordered-lst or text:unordered-list tag)</p>
     * @param <code>node</code> The element containing the list
     * @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to which
     * LaTeX code should be added
     * @param <code>oc</code> the current context
     * @param <code>bOrdered</code> true if this list is an ordered list
     */
    public void handleList(Element node, LaTeXDocumentPortion ldp, Context oc, boolean bOrdered) {
        // Set up new context
        Context ic = (Context) oc.clone();
        ic.incListLevel();

        // If the list contains headings, ignore it!
        if (ic.isIgnoreLists() || listContainsHeadings(node)) {
            ic.setIgnoreLists(true);
            traverseList(node,ldp,ic);
            return;
        }

        // Get the style name, if we don't know it already
        if (ic.getListStyleName()==null) {
            ic.setListStyleName(node.getAttribute(XMLString.TEXT_STYLE_NAME));
        }

        // Apply the style
        BeforeAfter ba = new BeforeAfter();
        palette.getListSc().applyListStyle(ic.getListStyleName(),ic.getListLevel(),
            bOrdered,"true".equals(node.getAttribute(XMLString.TEXT_CONTINUE_NUMBERING)),
            ba);

        // Export the list
        if (ba.getBefore().length()>0) { ldp.append(ba.getBefore()).nl(); }
        traverseList(node,ldp,ic);
        if (ba.getAfter().length()>0) { ldp.append(ba.getAfter()).nl(); }
    }

    /*
     * Process the contents of a list
     */
    private void traverseList (Element node, LaTeXDocumentPortion ldp, Context oc) {
        if (node.hasChildNodes()) {
            NodeList list = node.getChildNodes();
            int nLen = list.getLength();

            for (int i = 0; i < nLen; i++) {
                Node child = list.item(i);

                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();

                    palette.getInfo().addDebugInfo((Element)child,ldp);

                    if (nodeName.equals(XMLString.TEXT_LIST_ITEM)) {
                        handleListItem((Element)child,ldp,oc);
                    }
                    if (nodeName.equals(XMLString.TEXT_LIST_HEADER)) {
                        handleListItem((Element)child,ldp,oc);
                    }
                }
            }
        }
    }

    private void handleListItem(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // Are we ignoring this list?
        if (oc.isIgnoreLists()) {
            traverseBlockText(node,ldp,oc);
            return;
        }

        // Apply the style
        BeforeAfter ba = new BeforeAfter();
        palette.getListSc().applyListItemStyle(
            oc.getListStyleName(), oc.getListLevel(),
            node.getNodeName().equals(XMLString.TEXT_LIST_HEADER),
            "true".equals(node.getAttribute(XMLString.TEXT_RESTART_NUMBERING)),
            Misc.getPosInteger(node.getAttribute(XMLString.TEXT_START_VALUE),1)-1,
            ba);

        // export the list item
        if (ba.getBefore().length()>0) {
            ldp.append(ba.getBefore());
            if (config.formatting()>=Config.CONVERT_MOST) { ldp.nl(); }
        }
        traverseBlockText(node,ldp,oc);
        if (ba.getAfter().length()>0) { ldp.append(ba.getAfter()).nl(); }
    }

    /*
     * Helper: Check to see, if this list contains headings
     * (in that case we will ignore the list!)
     */
    private boolean listContainsHeadings (Node node) {
        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();
            for (int i = 0; i < len; i++) {
                Node child = nList.item(i);
                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();
                    if (nodeName.equals(XMLString.TEXT_LIST_ITEM)) {
                        if (listItemContainsHeadings(child)) return true;
                    }
                    if (nodeName.equals(XMLString.TEXT_LIST_HEADER)) {
                        if (listItemContainsHeadings(child)) return true;
                    }
                }
            }
        }
        return false;
    }

    private boolean listItemContainsHeadings(Node node) {
        if (node.hasChildNodes()) {
            NodeList nList = node.getChildNodes();
            int len = nList.getLength();
            for (int i = 0; i < len; i++) {
                Node child = nList.item(i);
                if (child.getNodeType() == Node.ELEMENT_NODE) {
                    String nodeName = child.getNodeName();
                    if(nodeName.equals(XMLString.TEXT_H)) {
                        return true;
                    }
                    if (nodeName.equals(XMLString.TEXT_ORDERED_LIST)) {
                        if (listContainsHeadings(child)) return true;
                    }
                    if (nodeName.equals(XMLString.TEXT_UNORDERED_LIST)) {
                        if (listContainsHeadings(child)) return true;
                    }
                }
            }
        }
        return false;
    }

    /** <p>Process a heading.</p>
     *  @param <code>node</code> The text:h element node containing the heading
     *  @param <code>ldp</code> The <code>LaTeXDocumentPortion</code> to add LaTeX code to
     *  @param <code>oc</code> The current context
     */
    public void handleHeading(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // Get the level, the heading map and the style name
        int nLevel = Misc.getPosInteger(node.getAttribute(XMLString.TEXT_LEVEL),1);
        HeadingMap hm = config.getHeadingMap();
        String sStyleName = node.getAttribute(XMLString.TEXT_STYLE_NAME);

        if (nLevel<=hm.getMaxLevel()) {
            // Always push the font used
            palette.getI18n().pushSpecialTable(palette.getParSc().getFontName(sStyleName));

            // Apply style
            BeforeAfter baHardPage = new BeforeAfter();
            BeforeAfter baHardChar = new BeforeAfter();
            Context ic = (Context) oc.clone();
            palette.getParSc().applyHardHeadingStyle(nLevel, sStyleName,
                baHardPage, baHardChar, ic);

            // Export the heading
            ldp.append(baHardPage.getBefore());
            ldp.append("\\"+hm.getName(nLevel));
            // If this heading contains formatting, add optional argument:
            if (baHardChar.getBefore().length()>0 || containsElements(node)) {
                ldp.append("[");
                palette.getInlineCv().traversePlainInlineText(node,ldp,ic);
                ldp.append("]");
            }
            ldp.append("{").append(baHardChar.getBefore());
            palette.getInlineCv().traverseInlineText(node,ldp,ic,false);
            ldp.append(baHardChar.getAfter()).append("}").nl();
            ldp.append(baHardPage.getAfter());

            // Include any floating frames
            palette.getDrawCv().flushFloatingFrames(ldp,ic);

            // Pop the font name
            palette.getI18n().popSpecialTable();
        }
        else { // beyond supported headings - export as ordinary paragraph
            handleParagraph(node,ldp,oc,false);
        }
    }

    /* Check to see if this node contains any element nodes, except reference marks */
    private boolean containsElements(Node node) {
        if (!node.hasChildNodes()) { return false; }
        NodeList list = node.getChildNodes();
        int nLen = list.getLength();
        for (int i = 0; i < nLen; i++) {
            Node child = list.item(i);
            if (child.getNodeType()==Node.ELEMENT_NODE &&
                !child.getNodeName().startsWith(XMLString.TEXT_REFERENCE_MARK)) {
                return true;
            }
        }
        return false;
    }

    /**
     * <p> Process a text:p tag</p>
     *  @param <code>node</code> The text:h element node containing the heading
     *  @param <code>ldp</code> The <code>LaTeXDocumentPortion</code> to add LaTeX code to
     *  @param <code>oc</code> The current context
     *  @param <code>bLastInBlock</code> If this is true, the paragraph is the
     *  last one in a block, and we need no trailing blank line (eg. right before
     *  \end{enumerate}).
     */
    public void handleParagraph(Element node, LaTeXDocumentPortion ldp, Context oc, boolean bLastInBlock) {
        // Empty paragraphs are often used to achieve vertical spacing in WYSIWYG
        // word processors. Hence we translate an empty paragraph to \bigskip.
        // This also solves the problem that LaTeX ignores empty paragraphs, Writer doesn't.
        // In a well-structured document, an empty paragraph is probably a mistake,
        // hence the configuration can specify that it should be ignored.
        if (!node.hasChildNodes()) {
            if (!config.ignoreEmptyParagraphs()) {
                ldp.nl().append("\\bigskip").nl();
                if (!bLastInBlock) { ldp.nl(); }
            }
            return;
        }

        // Get the style name for this paragraph
        String sStyleName = node.getAttribute(XMLString.TEXT_STYLE_NAME);

        // Always push the font used
        palette.getI18n().pushSpecialTable(palette.getParSc().getFontName(sStyleName));

        // Apply the style
        BeforeAfter ba = new BeforeAfter();
        Context ic = (Context) oc.clone();
        palette.getParSc().applyParStyle(sStyleName,ba,ic);

        // Do conversion
        ldp.append(ba.getBefore());
        if (ic.isVerbatim()) {
            palette.getInlineCv().traverseVerbatimInlineText(node,ldp,ic,false);
        }
        else {
            palette.getInlineCv().traverseInlineText(node,ldp,ic,false);
        }
        ldp.append(ba.getAfter());
        // Add a blank line except within verbatim and last in a block:
        if (!bLastInBlock && !ic.isVerbatim()) { ldp.nl(); }

        // Flush any floating frames from this paragraph
        palette.getDrawCv().flushFloatingFrames(ldp,ic);

        // pop the font name
        palette.getI18n().popSpecialTable();
    }

}