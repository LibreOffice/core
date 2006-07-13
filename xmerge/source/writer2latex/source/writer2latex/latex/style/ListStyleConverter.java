/************************************************************************
 *
 *  ListStyleConverter.java
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

package writer2latex.latex.style;

import java.util.Hashtable;

import writer2latex.util.*;
import writer2latex.office.*;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

/* This class creates LaTeX code from OOo list styles
 */
public class ListStyleConverter extends StyleConverter {
    boolean bNeedSaveEnumCounter = false;
    private Hashtable listStyleLevelNames = new Hashtable();

    /** <p>Constructs a new <code>ListStyleConverter</code>.</p>
     */
    public ListStyleConverter(WriterStyleCollection wsc, Config config,
        ConverterPalette palette) {
        super(wsc,config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (config.formatting()>=Config.CONVERT_MOST || !styleNames.isEmpty()) {
            decl.append("% List styles").nl();
            // May need an extra counter to handle continued numbering in lists
            if (bNeedSaveEnumCounter) {
                decl.append("\\newcounter{saveenum}").nl();
            }
            // If we export formatting, we need some hooks from lists to paragraphs:
            if (config.formatting()>=Config.CONVERT_MOST) {
                decl.append("\\newcommand\\writerlistleftskip{}").nl()
                    .append("\\newcommand\\writerlistparindent{}").nl()
                    .append("\\newcommand\\writerlistlabel{}").nl()
                    .append("\\newcommand\\writerlistremovelabel{")
                    .append("\\aftergroup\\let\\aftergroup\\writerlistparindent\\aftergroup\\relax")
                    .append("\\aftergroup\\let\\aftergroup\\writerlistlabel\\aftergroup\\relax}").nl();
            }
            super.appendDeclarations(pack,decl);
        }
    }

    /** <p>Apply a list style to an ordered or unordered list.</p> */
    public void applyListStyle(String sStyleName, int nLevel, boolean bOrdered,
                        boolean bContinue, BeforeAfter ba) {
        // Step 1. We may have a style map, this always takes precedence
        if (config.getListStyleMap().contains(sStyleName)) {
            ba.add(config.getListStyleMap().getBefore(sStyleName),
                   config.getListStyleMap().getAfter(sStyleName));
            return;
        }
        // Step 2: The list style may not exist, or the user wants to ignore it.
        // In this case we create default lists
        ListStyle style = wsc.getListStyle(sStyleName);
        if (style==null || config.formatting()<=Config.IGNORE_MOST) {
            if (nLevel<=4) {
                if (bOrdered) {
                    ba.add("\\begin{enumerate}","\\end{enumerate}");
                }
                else {
                    ba.add("\\begin{itemize}","\\end{itemize}");
                }
            }
            return;
        }
        // Step 3: Export as default lists, but redefine labels
        if (config.formatting()==Config.CONVERT_BASIC) {
            if (nLevel==1) {
                if (!styleNames.containsName(sStyleName)) {
                    createListStyleLabels(sStyleName);
                }
                ba.add("\\liststyle"+styleNames.getExportName(sStyleName)+"\n","");
            }
            if (nLevel<=4) {
                String sCounterName = ((String[]) listStyleLevelNames.get(sStyleName))[nLevel];
                if (bContinue && style.isNumber(nLevel)) {
                    bNeedSaveEnumCounter = true;
                    ba.add("\\setcounter{saveenum}{\\value{"+sCounterName+"}}\n","");
                }
                if (bOrdered) {
                    ba.add("\\begin{enumerate}","\\end{enumerate}");
                }
                else {
                    ba.add("\\begin{itemize}","\\end{itemize}");
                }
                if (bContinue && style.isNumber(nLevel)) {
                    ba.add("\n\\setcounter{"+sCounterName+"}{\\value{saveenum}}","");
                }
            }
            return;
        }
        // Step 4: Export with formatting, as "Writer style" custom lists
        if (nLevel<=4) { // TODO: Max level should not be fixed
            if (!styleNames.containsName(sStyleName)) {
                createListStyle(sStyleName);
            }
            String sTeXName="list"+styleNames.getExportName(sStyleName)
                           +"level"+Misc.int2roman(nLevel);
            if (!bContinue && style.isNumber(nLevel)) {
                ba.add("\\setcounter{"+sTeXName+"}{0}\n","");
            }
            ba.add("\\begin{"+sTeXName+"}","\\end{"+sTeXName+"}");
        }
    }

    /** <p>Apply a list style to a list item.</p> */
    public void applyListItemStyle(String sStyleName, int nLevel, boolean bHeader,
                            boolean bRestart, int nStartValue, BeforeAfter ba) {
        // Step 1. We may have a style map, this always takes precedence
        if (config.getListItemStyleMap().contains(sStyleName)) {
            ba.add(config.getListItemStyleMap().getBefore(sStyleName),
                   config.getListItemStyleMap().getAfter(sStyleName));
            return;
        }
        // Step 2: The list style may not exist, or the user wants to ignore it.
        // In this case we create default lists
        ListStyle style = wsc.getListStyle(sStyleName);
        if (style==null || config.formatting()<=Config.IGNORE_MOST) {
            if (nLevel<=4) {
                if (bHeader) { ba.add("\\item[] ",""); }
                else { ba.add("\\item ",""); }
            }
            return;
        }
        // Step 3: Export as default lists (with redefined labels)
        if (config.formatting()==Config.CONVERT_BASIC) {
            if (nLevel<=4) {
                if (bHeader) {
                    ba.add("\\item[] ","");
                }
                else if (bRestart && style.isNumber(nLevel)) {
                    ba.add("\n\\setcounter{enum"+Misc.int2roman(nLevel)
                           +"}{"+(nStartValue-1)+"}\n\\item ","");
                }
                else {
                    ba.add("\\item ","");
                }
            }
            return;
        }
        // Step 4: Export with formatting, as "Writer style" custom lists
        if (nLevel<=4 && !bHeader) { // TODO: Max level should not be fixed
            String sTeXName="list"+styleNames.getExportName(sStyleName)
                           +"level"+Misc.int2roman(nLevel);
            if (bRestart && style.isNumber(nLevel)) {
                ba.add("\\setcounter{"+sTeXName+"}{"+(nStartValue-1)+"}\n","");
            }
            ba.add("\\item ","");
        }
    }


    /** <p>Create labels for default lists (enumerate/itemize) based on
     *  a List Style
     */
    private void createListStyleLabels(String sStyleName) {
        String sTeXName = styleNames.getExportName(sStyleName);
        declarations.append("\\newcommand\\liststyle")
                             .append(sTeXName).append("{%").nl();
        ListStyle style = wsc.getListStyle(sStyleName);
        int nEnum = 0;
        int nItem = 0;
        String sName[] = new String[5];
        for (int i=1; i<=4; i++) {
            if (style.isNumber(i)) { sName[i]="enum"+Misc.int2roman(++nEnum); }
            else { sName[i]="item"+Misc.int2roman(++nItem); }
        }
        listStyleLevelNames.put(sStyleName, sName);
        createLabels(style, sName, 4, false, true, false, declarations);
        declarations.append("}").nl();
    }

    /** <p>Create "Writer style" lists based on a List Style.
        <p>A list in writer is really a sequence of numbered paragraphs, so
           this is also how we implement it in LaTeX.
           The enivronment + redefined \item defines three hooks:
           \writerlistleftskip, \writerlistparindent, \writerlistlabel
           which are used by exported paragraph styles to apply numbering.
     */
    private void createListStyle(String sStyleName) {
        ListStyle style = wsc.getListStyle(sStyleName);

        // Create labels
        String sTeXName = styleNames.getExportName(sStyleName);
        String[] sLevelName = new String[5];
        for (int i=1; i<=4; i++) {
            sLevelName[i]="list"+sTeXName+"level"+Misc.int2roman(i);
        }
        createLabels(style,sLevelName,4,true,false,true,declarations);

        // Create environments
        for (int i=1; i<=4; i++) {
            String sSpaceBefore = getLength(style,i,XMLString.TEXT_SPACE_BEFORE);
            String sLabelWidth = getLength(style,i,XMLString.TEXT_MIN_LABEL_WIDTH);
            String sLabelDistance = getLength(style,i,XMLString.TEXT_MIN_LABEL_DISTANCE);
            declarations
                .append("\\newenvironment{")
                .append(sLevelName[i]).append("}{")
                .append("\\def\\writerlistleftskip{\\addtolength\\leftskip{")
                .append(Misc.add(sSpaceBefore,sLabelWidth)).append("}}")
                .append("\\def\\writerlistparindent{}")
                .append("\\def\\writerlistlabel{}");
            // Redefine \item
            declarations
                .append("\\def\\item{")
                .append("\\def\\writerlistparindent{\\setlength\\parindent{")
                .append("-").append(sLabelWidth).append("}}")
                .append("\\def\\writerlistlabel{");
            if (style.isNumber(i)) {
                declarations.append("\\stepcounter{")
                                     .append(sLevelName[i]).append("}");
            }
            declarations
                .append("\\label").append(sLevelName[i])
                .append("\\hspace{").append(sLabelDistance).append("}")
                .append("\\writerlistremovelabel}}}{}").nl();
        }
    }

    /** <p>Create LaTeX list labels from an OOo list style. Examples:</p>
     *  <p>Bullets:</p>
     *  <pre>\newcommand\labelliststylei{\textbullet}
     *  \newcommand\labelliststyleii{*}
     *  \newcommand\labelliststyleiii{\textstylebullet{>}}</pre>
     *  <p>Numbering:</p>
     *  <pre>\newcounter{liststylei}
     *  \newcounter{liststyleii}[liststylei]
     *  \newcounter{liststyleiii}[liststyleii]
     *  \renewcommand\theliststylei{\Roman{liststylei}}
     *  \renewcommand\theliststyleii{\Roman{liststylei}.\arabic{liststyleii}}
     *  \renewcommand\theliststyleiii{\alph{liststyleiii}}
     *  \newcommand\labelliststylei{\textstylelabel{\theliststylei .}}
     *  \newcommand\labelliststyleii{\textstylelabel{\theliststyleii .}}
     *  \newcommand\labelliststyleiii{\textstylelabel{\theliststyleiii )}}</pre>
     *
     *  @param <code>style</code> the OOo list style to use
     *  @param <code>sName</code> an array of label basenames to use
     *  @param <code>nMaxLevel</code> the highest level in this numbering
     *  @param <code>bDeclareCounters</code> true if counters should be declared (they may
     *  exist already, eg. "section", "subsection"... or "enumi", "enumii"...
     *  @param <code>bRenewLabels</code> true if labels should be defined with \renewcommand
     *  @param <code>bUseTextStyle</code> true if labels should be formatted with the associated text style
     *  (rather than \newcommand).
     *  @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to add LaTeX code to.
     */
    private void createLabels(ListStyle style, String[] sName, int nMaxLevel,
                      boolean bDeclareCounters, boolean bRenewLabels,
                      boolean bUseTextStyle, LaTeXDocumentPortion ldp) {
        // Declare counters if required (eg. "\newcounter{countername1}[countername2]")
        if (bDeclareCounters) {
            int j = 0;
            for (int i=1; i<=nMaxLevel; i++) {
                if (style.isNumber(i)) {
                    ldp.append("\\newcounter{").append(sName[i]).append("}");
                    if (j>0) { ldp.append("[").append(sName[j]).append("]"); }
                    ldp.nl();
                    j = i;
                }
            }
        }
        // Create numbering for each level (eg. "\arabic{countername}")
        String[] sNumFormat = new String[nMaxLevel+1];
        for (int i=1; i<=nMaxLevel; i++) {
            String s = numFormat(style.getLevelProperty(i,XMLString.STYLE_NUM_FORMAT));
            if (s==null) { sNumFormat[i]=""; }
            else { sNumFormat[i] = s + "{" + sName[i] + "}"; }
        }
        // Create numberings (ie. define "\thecountername"):
        for (int i=1; i<=nMaxLevel; i++) {
            if (style.isNumber(i)) {
                ldp.append("\\renewcommand\\the").append(sName[i]).append("{");
                int nLevels = Misc.getPosInteger(style.getLevelProperty(i,XMLString.TEXT_DISPLAY_LEVELS),1);
                for (int j=i-nLevels+1; j<i; j++) {
                    if (style.isNumber(j)) {
                        ldp.append(sNumFormat[j]).append(".");
                    }
                }
                ldp.append(sNumFormat[i]);
                ldp.append("}").nl();
            }
        }
        // Create labels (ie. define "\labelcountername"):
        for (int i=1; i<=nMaxLevel; i++) {
            ldp.append(bRenewLabels ? "\\renewcommand" : "\\newcommand")
               .append("\\label").append(sName[i]).append("{");
            // Apply text style if required
            BeforeAfter baText = new BeforeAfter();
            if (bUseTextStyle) {
                String sStyleName = style.getLevelProperty(i,XMLString.TEXT_STYLE_NAME);
                palette.getCharSc().applyTextStyle(sStyleName,baText,new Context());
            }

            // Create label content
            if (style.isNumber(i)) {
                String sPrefix = style.getLevelProperty(i,XMLString.STYLE_NUM_PREFIX);
                String sSuffix = style.getLevelProperty(i,XMLString.STYLE_NUM_SUFFIX);
                // Apply style
                ldp.append(baText.getBefore());
                if (sPrefix!=null) { ldp.append(sPrefix); }
                ldp.append("\\the").append(sName[i]);
                if (sSuffix!=null) { ldp.append(sSuffix); }
                ldp.append(baText.getAfter());
            }
            else if (style.isBullet(i)) {
                String sBullet = style.getLevelProperty(i,XMLString.TEXT_BULLET_CHAR);
                // Apply style
                ldp.append(baText.getBefore());
                if (sBullet!=null) {
                    // Bullets are usually symbols, so this should be OK:
                    ldp.append(palette.getI18n().convert(sBullet,false,"en"));
                }
                ldp.append(baText.getAfter());
            }
            else {
                // TODO: Support images!
                ldp.append("\\textbullet");
            }

            ldp.append("}").nl();
        }
    }

    /* Helper: Get a length property that defaults to 0cm. */
    private String getLength(ListStyle style,int nLevel,String sProperty) {
        String s = style.getLevelStyleProperty(nLevel,sProperty);
        if (s==null) { return "0cm"; }
        else { return s; }
    }

    /* Helper: Convert OOo number format to LaTeX number format */
    public static final String numFormat(String sFormat){
        if ("1".equals(sFormat)) { return "\\arabic"; }
        else if ("i".equals(sFormat)) { return "\\roman"; }
        else if ("I".equals(sFormat)) { return "\\Roman"; }
        else if ("a".equals(sFormat)) { return "\\alph"; }
        else if ("A".equals(sFormat)) { return "\\Alph"; }
        else { return null; }
    }

}
