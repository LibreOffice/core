/************************************************************************
 *
 *  PageStyleConverter.java
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

import java.util.Enumeration;

import org.w3c.dom.Element;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.office.*;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

/* This class creates LaTeX code from OOo page masters/master pages
 */
public class PageStyleConverter extends StyleConverter {

    // Name of first used master page, if any
    private String sMainMasterPage = null;

    // Value of attribute text:display of most recent text:chapter field
    // This is used to handle chaptermarks in headings
    private String sChapterField1 = null;
    private String sChapterField2 = null;

    /** <p>Constructs a new <code>PageMasterConverter</code>.</p>
     */
    public PageStyleConverter(WriterStyleCollection wsc, Config config,
        ConverterPalette palette) {
        super(wsc,config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        // We currently use calc.sty for the geometry (todo: remove this dependency)
        if (config.pageFormatting()==Config.CONVERT_ALL) {
            pack.append("\\usepackage{calc}").nl();
        }

        // Determine which master page and page master to use first
        // If no page master was found when traversing the document, use the first
        // master page in styles.xml
        if (sMainMasterPage==null) {
            sMainMasterPage = wsc.getFirstMasterPageName();
            if (sMainMasterPage!=null) { styleNames.addName(sMainMasterPage); }
        }
        String sMainPageMaster = null;
        if (sMainMasterPage!=null) {
            MasterPage mpStyle = wsc.getMasterPage(sMainMasterPage);
            if (mpStyle!=null) {
                sMainPageMaster = mpStyle.getProperty(XMLString.STYLE_PAGE_MASTER_NAME);
            }
        }

        convertMasterPages(decl);

        BeforeAfter ba = new BeforeAfter();
        applyMasterPage(sMainMasterPage,ba);
        decl.append(ba.getBefore());
    }

    public void setChapterField1(String s) { sChapterField1 = s; }

    public void setChapterField2(String s) { sChapterField2 = s; }

    /** <p>Use a Master Page (pagestyle in LaTeX)</p>
     *  @param sName    name of the master page to use
     *  @param ba      the <code>BeforeAfter</code> to add code to.
     */
    public void applyMasterPage(String sName, BeforeAfter ba) {
        if (config.pageFormatting()==Config.IGNORE_ALL) return;
        MasterPage style = wsc.getMasterPage(sName);
        if (style==null) { return; }
        String sNextName = style.getProperty(XMLString.STYLE_NEXT_STYLE_NAME);
        MasterPage nextStyle = wsc.getMasterPage(sNextName);
        if (style==nextStyle || nextStyle==null) {
            ba.add("\\pagestyle{"+styleNames.getExportName(sName)+"}\n", "");
        }
        else {
            ba.add("\\pagestyle{"+styleNames.getExportName(sNextName)+"}\n"+
               "\\thispagestyle{"+styleNames.getExportName(sName)+"}\n","");
        }
        // todo: should warn the user if next master also contains a next-style-name;
        // LaTeX's page style mechanism cannot handle that
    }

    public void collectMasterPage(ParStyle style) {
        if (style==null || sMainMasterPage!=null) { return; }
        String s = style.getMasterPageName();
        if (s!=null && s.length()!=0) { sMainMasterPage = s; }
    }

    /*
     * Process header or footer contents
     */
    private void convertMasterPages(LaTeXDocumentPortion ldp) {
        if (config.pageFormatting()==Config.IGNORE_ALL) { return; }

        Context context = new Context();
        context.resetFormattingFromStyle(wsc.getDefaultParStyle());
        context.setInHeaderFooter(true);

        Enumeration styles = wsc.getMasterPages().getStylesEnumeration();
        ldp.append("% Pages styles (master pages)").nl()
                .append("\\makeatletter").nl();
        while (styles.hasMoreElements()) {
            MasterPage style = (MasterPage) styles.nextElement();
            String sName = style.getName();
            if (styleNames.containsName(sName)) {
                ldp.append("\\newcommand\\ps@")
                        .append(styleNames.getExportName(sName))
                        .append("{%").nl();
                sChapterField1 = null;
                sChapterField2 = null;
                // Header
                ldp.append("\\renewcommand\\@oddhead{");
                if (style.getHeader()!=null) {
                    traverseHeaderFooter((Element)style.getHeader(),ldp,context);
                }
                ldp.append("}%").nl();
                ldp.append("\\renewcommand\\@evenhead{");
                if (style.getHeaderLeft()!=null) {
                    traverseHeaderFooter((Element)style.getHeaderLeft(),ldp,context);
                }
                else if (style.getHeader()!=null) {
                    ldp.append("\\@oddhead");
                }
                ldp.append("}%").nl();
                // Footer
                ldp.append("\\renewcommand\\@oddfoot{");
                if (style.getFooter()!=null) {
                    traverseHeaderFooter((Element)style.getFooter(),ldp,context);
                }
                ldp.append("}%").nl();
                ldp.append("\\renewcommand\\@evenfoot{");
                if (style.getFooterLeft()!=null) {
                    traverseHeaderFooter((Element)style.getFooterLeft(),ldp,context);
                }
                else if (style.getFooter()!=null) {
                    ldp.append("\\@oddfoot");
                }
                ldp.append("}%").nl();
                // Sectionmark and subsectionmark
                if (sChapterField1!=null) {
                    ldp.append("\\def\\sectionmark##1{\\markboth{");
                    if ("name".equals(sChapterField1)) { ldp.append("##1"); }
                    else if ("number".equals(sChapterField1) || "plain-number".equals(sChapterField1)) {
                        ldp.append("\\thesection");
                    }
                    else { ldp.append("\\thesection ##1"); }
                    ldp.append("}{}}").nl();
                }
                if (sChapterField2!=null) {
                    if (sChapterField1==null) {
                        ldp.append("\\def\\sectionmark##1{\\markboth{}{}}").nl();
                    }
                    ldp.append("\\def\\subsectionmark##1{\\markright{");
                    if ("name".equals(sChapterField2)) { ldp.append("##1"); }
                    else if ("number".equals(sChapterField2) || "plain-number".equals(sChapterField1)) {
                        ldp.append("\\thesubsection");
                    }
                    else { ldp.append("\\thesubsection ##1"); }
                    ldp.append("}{}}").nl();
                }
                // Convert PageMaster
                String sPageMaster = style.getProperty(XMLString.STYLE_PAGE_MASTER_NAME);
                convertPageMaster(sPageMaster,ldp);

                ldp.append("}").nl();
            }
        }
        ldp.append("\\makeatother").nl();
    }

    private void traverseHeaderFooter(Element node, LaTeXDocumentPortion ldp, Context context) {
        // get first paragraph; all other content is ignored
        Element par = Misc.getChildByTagName(node,XMLString.TEXT_P);
        if (par==null) { return; }

        String sStyleName = par.getAttribute(XMLString.TEXT_STYLE_NAME);
        BeforeAfter ba = new BeforeAfter();
        // Temp solution: Ignore hard formatting in header/footer (name clash problem)
        // only in package format. TODO: Find a better solution for 0.4!
        // TODO: Reenable this!!!!:
        /*if (palette.isPackage() && palette.getParSc().isAutomatic(sStyleName)) {
            palette.getCharSc().applyHardCharFormatting(sStyleName,ba);
        }*/

        if (par.hasChildNodes()) {
            ldp.append(ba.getBefore());
            palette.getInlineCv().traverseInlineText(par,ldp,context,false);
            ldp.append(ba.getAfter());
        }

    }


    /** <p>Convert a page master (pagesize, margins etc.)
     *  @param sName   name of the page master to use
     *  @param ldp     the <code>LaTeXDocumentPortion</code> to add code to.
     */
    private void convertPageMaster(String sName, LaTeXDocumentPortion ldp) {
        if (config.pageFormatting()!=Config.CONVERT_ALL) { return; }
        PageMaster style = wsc.getPageMaster(sName);
        if (style==null) { return; }

        String sHeadHeight = "0cm";
        String sHeadSep = "0cm";
        if (style.hasHeaderStyle()) {
            sHeadHeight = "12pt";
            sHeadSep = style.getHeaderProperty(XMLString.FO_MARGIN_BOTTOM);
            if (sHeadSep==null) { sHeadSep = "0cm"; }
        }

        String sFootHeight = "0cm";
        String sFootSep = "0cm";
        if (style.hasFooterStyle()) {
            sFootHeight = "12pt";
            sFootSep = style.getFooterProperty(XMLString.FO_MARGIN_TOP);
            if (sFootSep==null) { sFootSep = "0cm"; }
        }

        // Page dimensions
        ldp.append("\\setlength\\paperwidth{")
           .append(style.getAbsoluteProperty(XMLString.FO_PAGE_WIDTH))
           .append("}")
           .append("\\setlength\\paperheight{")
           .append(style.getAbsoluteProperty(XMLString.FO_PAGE_HEIGHT))
           .append("}");
        // PDF page dimensions, only if hyperref.sty is not loaded
        if (config.getBackend()==Config.PDFTEX && !config.useHyperref()) {
            ldp.append("\\setlength\\pdfpagewidth{")
               .append(style.getAbsoluteProperty(XMLString.FO_PAGE_WIDTH))
               .append("}")
               .append("\\setlength\\pdfpageheight{")
               .append(style.getAbsoluteProperty(XMLString.FO_PAGE_HEIGHT))
               .append("}");
        }
        // Page starts in upper left corner of paper!!
        ldp.append("\\setlength\\voffset{-1in}");
        ldp.append("\\setlength\\hoffset{-1in}");
        // Top margin
        ldp.append("\\setlength\\topmargin{")
           .append(style.getProperty(XMLString.FO_MARGIN_TOP))
           .append("}");
        // Header and footer
        ldp.append("\\setlength\\headheight{").append(sHeadHeight).append("}");
        ldp.append("\\setlength\\headsep{").append(sHeadSep).append("}");
        ldp.append("\\setlength\\footskip{")
           .append(sFootHeight).append("+").append(sFootSep).append("}");
        // Bottom margin is set indirectly via text height
        ldp.append("\\setlength\\textheight{")
           .append(style.getProperty(XMLString.FO_PAGE_HEIGHT))
           .append("-").append(style.getProperty(XMLString.FO_MARGIN_TOP))
           .append("-").append(style.getProperty(XMLString.FO_MARGIN_BOTTOM))
           .append("-").append(sHeadSep)
           .append("-").append(sHeadHeight)
           .append("-").append(sFootSep)
           .append("-").append(sFootHeight).append("}");
        // Left margin
        ldp.append("\\setlength\\oddsidemargin{")
           .append(style.getProperty(XMLString.FO_MARGIN_LEFT))
           .append("}");
        // Left margin for even (left) pages; only for mirrored page master
        if ("mirrored".equals(style.getPageUsage())) {
            ldp.append("\\setlength\\evensidemargin{")
               .append(style.getProperty(XMLString.FO_MARGIN_RIGHT))
               .append("}");
        }
        // Right margin is set indirectly via text width
        ldp.append("\\setlength\\textwidth{")
           .append(style.getProperty(XMLString.FO_PAGE_WIDTH))
           .append("-")
           .append(style.getProperty(XMLString.FO_MARGIN_LEFT))
           .append("-")
           .append(style.getProperty(XMLString.FO_MARGIN_RIGHT))
           .append("}").nl();
        // Set additional lengths
        //ldp.append("\\@colht\\textheight\\@colroom\\textheight\\vsize\\textheight").nl();

        // Define columns
        //if (style.getColCount()>1) { ldp.append("\\twocolumn").nl(); }
        //else { ldp.append("\\onecolumn").nl(); }

        // Page number
        String sNumFormat = style.getProperty(XMLString.STYLE_NUM_FORMAT);
        if (sNumFormat!=null) {
            ldp.append("\\renewcommand\\thepage{")
               .append(ListStyleConverter.numFormat(sNumFormat))
               .append("{page}}").nl();
        }
        String sPageNumber = style.getProperty(XMLString.STYLE_FIRST_PAGE_NUMBER);
        if (sPageNumber!=null && !sPageNumber.equals("continue")) {
           ldp.append("\\setcounter{page}{")
              .append(Integer.toString(Misc.getPosInteger(sPageNumber,0)))
              .append("}").nl();
        }
        // Footnote rule
        // TODO: Support alignment.
        String sBefore = style.getFootnoteProperty(XMLString.STYLE_DISTANCE_BEFORE_SEP);
        if (sBefore==null) { sBefore = "1mm"; }
        String sAfter = style.getFootnoteProperty(XMLString.STYLE_DISTANCE_AFTER_SEP);
        if (sAfter==null) { sAfter = "1mm"; }
        String sHeight = style.getFootnoteProperty(XMLString.STYLE_WIDTH);
        if (sHeight==null) { sHeight = "0.2mm"; }
        String sWidth = style.getFootnoteProperty(XMLString.STYLE_REL_WIDTH);
        if (sWidth==null) { sWidth = "25%"; }
        sWidth=Float.toString(Misc.getFloat(sWidth.substring(0,sWidth.length()-1),1)/100);
        BeforeAfter baColor = new BeforeAfter();
        palette.getCharSc().applyThisColor(style.getFootnoteProperty(XMLString.STYLE_COLOR),false,baColor);

        ldp.append("\\setlength{\\skip\\footins}{").append(sBefore).append("}");
        ldp.append("\\renewcommand\\footnoterule{\\vspace*{-").append(sHeight)
           .append("}\\noindent")
           .append(baColor.getBefore()).append("\\rule{").append(sWidth)
           .append("\\columnwidth}{").append(sHeight).append("}")
           .append(baColor.getAfter())
           .append("\\vspace*{").append(sAfter).append("}}").nl();
    }


}
