/************************************************************************
 *
 *  ParStyleConverter.java
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

package writer2latex.latex.style;

import java.util.Hashtable;

import writer2latex.util.*;
import writer2latex.office.*;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.HeadingMap;
import writer2latex.latex.ConverterPalette;
import writer2latex.latex.Context;

/* This class creates LaTeX code from OOo paragraph styles
 */
public class ParStyleConverter extends StyleConverter {
    private String[] sHeadingStyles = new String[11];

    /** <p>Constructs a new <code>ParStyleConverter</code>.</p>
     */
    public ParStyleConverter(WriterStyleCollection wsc, Config config,
        ConverterPalette palette) {
        super(wsc,config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (config.formatting()>=Config.CONVERT_MOST) {
            // We typeset with \raggedbottom since OOo doesn't use rubber lengths
            // TODO: Maybe turn vertical spacing from OOo into rubber lengths?
            decl.append("\\raggedbottom").nl();
        }

        appendHeadingStyles(decl);

        if (config.formatting()>=Config.CONVERT_MOST) {
            decl.append("% Paragraph styles").nl();
            // First default paragraph style
            palette.getCharSc().applyDefaultFont(wsc.getDefaultParStyle(),decl);
            super.appendDeclarations(pack,decl);
        }
    }

    /** <p>Use a paragraph style in LaTeX.</p>
     *  @param <code>sName</code> the name of the text style
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     *  @param <code>context</code> the current context. This method will use and update the formatting context
     */
    public void applyParStyle(String sName, BeforeAfter ba, Context context) {
        applyParStyle(sName,ba,context,true);
    }

    private void applyParStyle(String sName, BeforeAfter ba, Context context, boolean bBreakInside) {
        // No style specified?
        if (sName==null) { return; }

        // Always end with a line break
        ba.add("","\n");
        // Apply the style
        if (!styleMap.contains(sName)) { createParStyle(sName); }
        String sBefore = styleMap.getBefore(sName);
        String sAfter = styleMap.getAfter(sName);
        ba.add(sBefore,sAfter);
        // Add line breaks inside?
        if (bBreakInside && styleMap.getLineBreak(sName)) {
            if (sBefore.length()>0) { ba.add("\n",""); }
            if (sAfter.length()>0 && !"}".equals(sAfter)) { ba.add("","\n"); }
        }

        // Register master page and update context
        ParStyle style = wsc.getParStyle(sName);
        if (style==null) { return; }
        palette.getPageSc().collectMasterPage(style);
        context.updateFormattingFromStyle(style);
        context.setVerbatim(styleMap.getVerbatim(sName));
    }

    /** <p>Convert a paragraph style to LaTeX. </p>
     *  <p>A soft style is declared in <code>styleDeclarations</code> as
     *  <code>\newenvironment...</code></p>
     *  <p>A hard style is used by applying LaTeX code directly</p>
     *  @param <code>sName</code> the OOo name of the style
     */
    private void createParStyle(String sName) {
        // A paragraph style should always be created relative to main context
        Context context = (Context) palette.getMainContext().clone();
        // The style may already be declared in the configuration:
        StyleMap sm = config.getParStyleMap();
        if (sm.contains(sName)) {
            styleMap.put(sName,sm.getBefore(sName),sm.getAfter(sName),
                                  sm.getLineBreak(sName),sm.getVerbatim(sName));
            return;
        }
        // Does the style exist?
        ParStyle style = wsc.getParStyle(sName);
        if (style==null) {
            styleMap.put(sName,"","");
            return;
        }
        // Convert the style!
        switch (config.formatting()) {
            case Config.CONVERT_MOST:
                if (style.isAutomatic()) {
                    createAutomaticParStyle(style,context);
                    return;
                }
            case Config.CONVERT_ALL:
                createSoftParStyle(style,context);
                return;
            case Config.CONVERT_BASIC:
            case Config.IGNORE_MOST:
                createSimpleParStyle(style,context);
                return;
            case Config.IGNORE_ALL:
            default:
                styleMap.put(sName,"","");
        }
    }

    private void createAutomaticParStyle(ParStyle style, Context context) {
        // Hard paragraph formatting from this style should be ignored
        // (because the user wants to ignore hard paragraph formatting
        // or there is a style map for the parent.)
        BeforeAfter ba = new BeforeAfter();
        BeforeAfter baPar = new BeforeAfter();
        BeforeAfter baText = new BeforeAfter();

        // Apply paragraph formatting from parent
        // If parent is verbatim, this is all
        String sParentName = style.getParentName();
        if (styleMap.getVerbatim(sParentName)) {
            styleMap.put(style.getName(),styleMap.getBefore(sParentName),styleMap.getAfter(sParentName),
                         styleMap.getLineBreak(sParentName),styleMap.getVerbatim(sParentName));
            return;
        }
        applyParStyle(sParentName,baPar,context,false);

        // Apply hard formatting properties:
        applyPageBreak(style,false,ba);
        palette.getI18n().applyLanguage(style,true,false,baText);
        palette.getCharSc().applyFont(style,true,false,baText,context);

        // Assemble the bits. If there is any hard character formatting
        // we must group the contents.
        if (baPar.isEmpty() && !baText.isEmpty()) { ba.add("{","}"); }
        else { ba.add(baPar.getBefore(),baPar.getAfter()); }
        ba.add(baText.getBefore(),baText.getAfter());
        boolean bLineBreak = styleMap.getLineBreak(sParentName);
        if (!bLineBreak && !baText.isEmpty()) { ba.add(" ",""); }
        styleMap.put(style.getName(),ba.getBefore(),ba.getAfter(),bLineBreak,false);
    }

    private void createSimpleParStyle(ParStyle style, Context context) {
        // Export character formatting + alignment only
        if (style.isAutomatic() && config.getParStyleMap().contains(style.getParentName())) {
            createAutomaticParStyle(style,context);
        }

        BeforeAfter ba = new BeforeAfter();
        BeforeAfter baText = new BeforeAfter();

        // Apply hard formatting attributes
        // Note: Left justified text is exported as full justified text!
        applyPageBreak(style,false,ba);
        String sTextAlign = style.getProperty(XMLString.FO_TEXT_ALIGN,true);
        if ("center".equals(sTextAlign)) { baText.add("\\centering","\\par"); }
        else if ("end".equals(sTextAlign)) { baText.add("\\raggedleft","\\par"); }
        palette.getI18n().applyLanguage(style,true,true,baText);
        palette.getCharSc().applyFont(style,true,true,baText,context);

        // Assemble the bits. If there is any hard character formatting
        // or alignment we must group the contents.
        if (!baText.isEmpty()) { ba.add("{","}"); }
        ba.add(baText.getBefore(),baText.getAfter());
        styleMap.put(style.getName(),ba.getBefore(),ba.getAfter());
    }

    private void createSoftParStyle(ParStyle style, Context context) {
        // This style should be converted to an enviroment, except if
        // it's automatic and there is a config style map for the parent
        if (style.isAutomatic() && config.getParStyleMap().contains(style.getParentName())) {
            createAutomaticParStyle(style,context);
        }

        BeforeAfter ba = new BeforeAfter();
        applyParProperties(style,ba);
        ba.add("\\writerlistparindent\\writerlistleftskip","");
        palette.getI18n().applyLanguage(style,true,true,ba);
        ba.add("\\leavevmode","");
        palette.getCharSc().applyNormalFont(ba);
        palette.getCharSc().applyFont(style,true,true,ba,context);
        ba.add("\\writerlistlabel","");
        ba.add("\\ignorespaces","");
        // Declare the paragraph style (\newenvironment)
        String sTeXName = "style" + styleNames.getExportName(style.getName());
        styleMap.put(style.getName(),"\\begin{"+sTeXName+"}","\\end{"+sTeXName+"}");
        declarations.append("\\newenvironment{").append(sTeXName)
                    .append("}{").append(ba.getBefore()).append("}{")
                    .append(ba.getAfter()).append("}").nl();
    }

    /** <p>Use a paragraph style on a heading. If hard paragraph formatting
     *  is applied to a heading, page break and font is converted - other
     *  hard formatting is ignored.
     *  <p>This method also collects name of heading style and master page
     *  @param <code>nLevel</code> The level of this heading
     *  @param <code>sStyleName</code> the name of the paragraph style to use
     *  @param <code>baPage</code> a <code>BeforeAfter</code> to put page break code into
     *  @param <code>baText</code> a <code>BeforeAfter</code> to put character formatting code into
     *  @param <code>context</code> the current context. This method will use and update the formatting context
     */
    public void applyHardHeadingStyle(int nLevel, String sStyleName,
        BeforeAfter baPage, BeforeAfter baText, Context context) {

        // Get the style
        ParStyle style = wsc.getParStyle(sStyleName);
        if (style==null) { return; }

        // Register master page and heading style
        palette.getPageSc().collectMasterPage(style);
        if (sHeadingStyles[nLevel]==null) {
            sHeadingStyles[nLevel] = style.isAutomatic() ? style.getParentName() : sStyleName;
        }

        // Do conversion
        if (style.isAutomatic()) {
            applyPageBreak(style,false,baPage);
            palette.getCharSc().applyHardCharFormatting(style,baText);
        }

        // Update context
        context.updateFormattingFromStyle(style);
    }

    // utility method to get the font name from a char style
    public String getFontName(String sStyleName) {
        StyleWithProperties style = wsc.getParStyle(sStyleName);
        if (style==null) { return null; }
        String sName = style.getProperty(XMLString.STYLE_FONT_NAME);
        if (sName==null) { return null; }
        FontDeclaration fd = wsc.getFontDeclaration(sName);
        if (fd==null) { return null; }
        return fd.getProperty(XMLString.FO_FONT_FAMILY);
    }

    /** <p>Apply page break properties from a style.</p>
     *  @param <code>style</code> the paragraph style to use
     *  @param <code>bInherit</code> true if inheritance from parent style should be used
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     */
    public void applyPageBreak(ParStyle style, boolean bInherit, BeforeAfter ba) {
        if (style==null) { return; }
        if (style.isAutomatic() && config.ignoreHardPageBreaks()) { return; }
        // A page break can be a simple page break before or after...
        String s = style.getProperty(XMLString.FO_BREAK_BEFORE,bInherit);
        if ("page".equals(s)) { ba.add("\\clearpage",""); }
        s = style.getProperty(XMLString.FO_BREAK_AFTER,bInherit);
        if ("page".equals(s)) { ba.add("","\\clearpage"); }
        // ...or it can be a new master page
        String sMasterPage = style.getMasterPageName();
        if (sMasterPage==null || sMasterPage.length()==0) { return; }
        ba.add("\\clearpage","");
        palette.getPageSc().applyMasterPage(sMasterPage,ba);
    }

    // Remaining methods are private helpers

    /** <p>Apply line spacing from a style.</p>
     *  @param <code>style</code> the paragraph style to use
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     */
    private void applyLineSpacing(ParStyle style, BeforeAfter ba) {
        if (style==null) { return; }
        String sLineHeight = style.getProperty(XMLString.FO_LINE_HEIGHT);
        if (sLineHeight==null || !sLineHeight.endsWith("%")) { return; }
        float fPercent=Misc.getFloat(sLineHeight.substring(0,sLineHeight.length()-1),1);
        ba.add("\\renewcommand\\baselinestretch{"+fPercent/120+"}","");
    }

    /** <p>Helper: Get a length property that
     * defaults to 0cm.
     */
    private String getLength(ParStyle style,String sProperty) {
        String s = style.getAbsoluteProperty(sProperty);
        if (s==null) { return "0cm"; }
        else { return s; }
    }

    /** <p>Helper: Create a horizontal border.</p>
     */
    private String createBorder(String sLeft, String sRight, String sTop,
                                String sHeight, String sColor) {
        BeforeAfter baColor = new BeforeAfter();
        palette.getCharSc().applyThisColor(sColor,false,baColor);
        return "{\\setlength\\parindent{0pt}\\setlength\\leftskip{" + sLeft + "}"
               + "\\setlength\\baselineskip{0pt}\\setlength\\parskip{" + sHeight + "}"
               + baColor.getBefore()
               + "\\rule{\\textwidth-" + sLeft + "-" + sRight + "}{" + sHeight + "}"
               + baColor.getAfter()
               + "\\par}";
    }

    /** <p>Apply margin+alignment properties from a style.</p>
     *  @param <code>style</code> the paragraph style to use
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     */
    private void applyMargins(ParStyle style, BeforeAfter ba) {
        // Read padding/margin/indentation properties:
        String sPaddingTop = getLength(style,XMLString.FO_PADDING_TOP);
        String sPaddingBottom = getLength(style,XMLString.FO_PADDING_BOTTOM);
        String sPaddingLeft = getLength(style,XMLString.FO_PADDING_LEFT);
        String sPaddingRight = getLength(style,XMLString.FO_PADDING_RIGHT);
        String sMarginTop = getLength(style,XMLString.FO_MARGIN_TOP);
        String sMarginBottom = getLength(style,XMLString.FO_MARGIN_BOTTOM);
        String sMarginLeft = getLength(style,XMLString.FO_MARGIN_LEFT);
        String sMarginRight = getLength(style,XMLString.FO_MARGIN_RIGHT);
        String sTextIndent;
        if ("true".equals(style.getProperty(XMLString.STYLE_AUTO_TEXT_INDENT))) {
            sTextIndent = "2em";
        }
        else {
            sTextIndent = getLength(style,XMLString.FO_TEXT_INDENT);
        }
        // Read alignment properties:
        boolean bRaggedLeft = false; // add 1fil to \leftskip
        boolean bRaggedRight = false; // add 1fil to \rightskip
        boolean bParFill = false; // add 1fil to \parfillskip
        String sTextAlign = style.getProperty(XMLString.FO_TEXT_ALIGN);
        if ("center".equals(sTextAlign)) {
            bRaggedLeft = true; bRaggedRight = true; // centered paragraph
        }
        else if ("start".equals(sTextAlign)) {
            bRaggedRight = true; bParFill = true; // left aligned paragraph
        }
        else if ("end".equals(sTextAlign)) {
            bRaggedLeft = true; // right aligned paragraph
        }
        else if (!"justify".equals(style.getProperty(XMLString.FO_TEXT_ALIGN_LAST))) {
            bParFill = true; // justified paragraph with ragged last line
        }
        // Create formatting:
        ba.add("\\setlength\\leftskip{"+sMarginLeft+(bRaggedLeft?" plus 1fil":"")+"}","");
        ba.add("\\setlength\\rightskip{"+sMarginRight+(bRaggedRight?" plus 1fil":"")+"}","");
        ba.add("\\setlength\\parindent{"+sTextIndent+"}","");
        ba.add("\\setlength\\parfillskip{"+(bParFill?"0pt plus 1fil":"0pt")+"}","");
        ba.add("\\setlength\\parskip{"+sMarginTop+"}","\\unskip\\vspace{"+sMarginBottom+"}");
    }

    private void applyAlignment(ParStyle style, boolean bIsSimple, boolean bInherit, BeforeAfter ba) {
        if (bIsSimple || style==null) { return; }
        String sTextAlign = style.getProperty(XMLString.FO_TEXT_ALIGN,bInherit);
        if ("center".equals(sTextAlign)) { ba.add("\\centering",""); }
        else if ("start".equals(sTextAlign)) { ba.add("\\raggedright",""); }
        else if ("end".equals(sTextAlign)) { ba.add("\\raggedleft",""); }
    }


    /** <p>Apply all paragraph properties.</p>
     *  @param <code>style</code> the paragraph style to use
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     */
    private void applyParProperties(ParStyle style, BeforeAfter ba) {
        applyPageBreak(style,true,ba);
        ba.add("","\\par");
        applyLineSpacing(style,ba);
        applyMargins(style,ba);
    }

    /** <p>Convert heading styles and outline numbering to LaTeX.</p>
     *  <p>An array of stylenames to use is required: The OOo writer file format
     *  allows different paragraph styles to be applied to individual headings,
     *  so this is not included in the styles.
     *  LaTeX (and OOo Writer!) usually uses the same format for all headings.</p>
     *  @param ldp    the <code>LaTeXDocumentPortion</code> to add definitions to.
     */
    // TODO: use method from ListStyleConverter to create labels
    private void appendHeadingStyles(LaTeXDocumentPortion ldp) {
        // The user may not want to convert the formatting of headings
        if (config.formatting()<=Config.IGNORE_MOST) { return; }

        HeadingMap hm = config.getHeadingMap();

        // OK, we are going to convert. First find the max level for headings
        int nMaxLevel = 0;
        for (int i=1; i<=5; i++) { if (sHeadingStyles[i]!=null) { nMaxLevel=i; } }
        if (nMaxLevel==0) { return; } // no headings, nothing to do!
        if (nMaxLevel>hm.getMaxLevel()) { nMaxLevel = hm.getMaxLevel(); }

        boolean bOnlyNum = config.formatting()==Config.CONVERT_BASIC;
        if (bOnlyNum) {
            ldp.append("% Outline numbering").nl();
        }
        else {
            ldp.append("% Headings and outline numbering").nl()
               .append("\\makeatletter").nl();
        }

        // Paragraph style for headings:
        if (!bOnlyNum) {
            for (int i=1; i<=nMaxLevel; i++) {
                if (sHeadingStyles[i]!=null) {
                    ParStyle style = wsc.getParStyle(sHeadingStyles[i]);
                    if (style!=null) {
                        BeforeAfter decl = new BeforeAfter();
                        BeforeAfter comm = new BeforeAfter();

                        applyPageBreak(style,true,decl);

                        palette.getCharSc().applyNormalFont(decl);
                        palette.getCharSc().applyFont(style,true,true,decl,new Context());
                        applyAlignment(style,false,true,decl);

                        palette.getI18n().applyLanguage(style,false,true,comm);
                        palette.getCharSc().applyFontEffects(style,true,comm);

                        String sMarginTop = getLength(style,XMLString.FO_MARGIN_TOP);
                        String sMarginBottom = getLength(style,XMLString.FO_MARGIN_BOTTOM);
                        String sMarginLeft = getLength(style,XMLString.FO_MARGIN_LEFT);

                        String sSecName = hm.getName(i);
                        if (!comm.isEmpty()) { // have to create a cs for this heading
                            ldp.append("\\newcommand\\cs").append(sSecName).append("[1]{")
                               .append(comm.getBefore()).append("#1").append(comm.getAfter())
                               .append("}").nl();
                        }
                        ldp.append("\\renewcommand\\").append(sSecName)
                           .append("{\\@startsection{").append(sSecName).append("}{"+hm.getLevel(i))
                           .append("}{"+sMarginLeft+"}{");
                        // Suppress indentation after heading? currently not..
                        // ldp.append("-");
                        ldp.append(sMarginTop)
                           .append("}{").append(sMarginBottom).append("}{");
                        // Note: decl.getAfter() may include a page break after, which we ignore
                        ldp.append(decl.getBefore());
                        if (!comm.isEmpty()) {
                            ldp.append("\\cs").append(sSecName);
                        }
                        ldp.append("}}").nl();
                    }
                }
            }
        }

        // redefine formatting of section counters
        // simplified if the user wants to ignore formatting
        if (!bOnlyNum) {
            ldp.append("\\renewcommand\\@seccntformat[1]{")
               .append("\\csname @textstyle#1\\endcsname{\\csname the#1\\endcsname}")
               .append("\\csname @distance#1\\endcsname}").nl();
        }

        // Collect numbering styles and set secnumdepth
        int nSecnumdepth = nMaxLevel;
        ListStyle outline = wsc.getOutlineStyle();
        String[] sNumFormat = new String[6];
        for (int i=nMaxLevel; i>=1; i--) {
            sNumFormat[i] = ListStyleConverter.numFormat(outline.getLevelProperty(i,
                               XMLString.STYLE_NUM_FORMAT));
            if (sNumFormat[i]==null || "".equals(sNumFormat[i])) {
                nSecnumdepth = i-1;
            }
        }
        ldp.append("\\setcounter{secnumdepth}{"+nSecnumdepth+"}").nl();

        for (int i=1; i<=nMaxLevel; i++) {
            if (sNumFormat[i]==null || "".equals(sNumFormat[i])) {
                // no numbering at this level
                if (!bOnlyNum) {
                    ldp.append("\\newcommand\\@distance")
                       .append(hm.getName(i)).append("{}").nl()
                       .append("\\newcommand\\@textstyle")
                       .append(hm.getName(i)).append("[1]{#1}").nl();
                }
            }
            else {
                if (!bOnlyNum) {
                    // Distance between label and text:
                    String sDistance = outline.getLevelProperty(i,XMLString.TEXT_MIN_LABEL_DISTANCE);
                    ldp.append("\\newcommand\\@distance")
                       .append(hm.getName(i)).append("{");
                    if (sDistance!=null) {
                        ldp.append("\\hspace{").append(sDistance).append("{");
                    }
                    ldp.append("}").nl();
                    // Textstyle to use for label:
                    String sStyleName = outline.getLevelProperty(i,XMLString.TEXT_STYLE_NAME);
                    BeforeAfter baText = new BeforeAfter();
                    if (!bOnlyNum) {palette.getCharSc().applyTextStyle(sStyleName,baText,new Context()); }
                    ldp.append("\\newcommand\\@textstyle")
                       .append(hm.getName(i)).append("[1]{")
                       .append(baText.getBefore())
                       .append("#1")
                       .append(baText.getAfter())
                       .append("}").nl();
                }

                // The label:
                String sPrefix = outline.getLevelProperty(i,XMLString.STYLE_NUM_PREFIX);
                String sSuffix = outline.getLevelProperty(i,XMLString.STYLE_NUM_SUFFIX);
                int nLevels = Misc.getPosInteger(outline.getLevelProperty(i,
                                      XMLString.TEXT_DISPLAY_LEVELS),1);
                ldp.append("\\renewcommand\\the")
                   .append(hm.getName(i))
                   .append("{");
                StringBuffer labelbuf = new StringBuffer();
                if (sPrefix!=null) { labelbuf.append(sPrefix); }
                for (int j=i-nLevels+1; j<i; j++) {
                    labelbuf.append(sNumFormat[j])
                            .append("{").append(sectionName(j)).append("}")
                            .append(".");
                }
                labelbuf.append(sNumFormat[i])
                        .append("{").append(hm.getName(i)).append("}");
                if (sSuffix!=null) { labelbuf.append(sSuffix); }
                if (bOnlyNum) {
                    ldp.append(labelbuf.toString().trim());
                }
                else {
                    ldp.append(labelbuf.toString());
                }
                ldp.append("}").nl();
            }

        }

        if (!bOnlyNum) {
            ldp.append("\\makeatother").nl();
        }
    }

    static final String sectionName(int nLevel){
        switch (nLevel) {
            case 1: return "section";
            case 2: return "subsection";
            case 3: return "subsubsection";
            case 4: return "paragraph";
            case 5: return "subparagraph";
            default: return null;
        }
    }


}
