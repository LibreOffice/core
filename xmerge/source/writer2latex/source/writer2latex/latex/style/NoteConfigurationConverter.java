/************************************************************************
 *
 *  NoteConfigurationConverter.java
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

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.office.*;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

/** <p>This class creates LaTeX code from OOo {foot|end}note configuration,
 *  which includes numbering and formatting of the note.
 */
public class NoteConfigurationConverter extends StyleConverter {

    /** <p>Constructs a new <code>NoteConfigurationConverter</code>.</p>
     */
    public NoteConfigurationConverter(WriterStyleCollection wsc, Config config, ConverterPalette palette) {
        super(wsc,config,palette);
    }

    /** <p>Convert footnotes configuration.</p>
     *  <p>Note: All footnotes are formatted with the default style for footnotes.
     *  (This doesn't conform with the file format specification, but in LaTeX
     *  all footnotes are usually formatted in a fixed style.)</p>
     *
     *  @param ldp    the <code>LaTeXDocumentPortion</code> to add definitions to.
     */
    public void convertFootnotesConfiguration(LaTeXDocumentPortion ldp) {
        // Note: Continuation notices are not supported in LaTeX
        // TODO: Support text:footnotes-postion="document" (footnotes as endnotes)
        // TODO: Support text:start-numbering-at="page" (footnpag.sty/footmisc.sty)
        convertFootEndnotesConfiguration(wsc.getFootnotesConfiguration(),"foot",ldp);
    }

    /** <p>Convert endnotes configuration.</p>
     *  <p>Note: This method automatically includes the package "endnotes.sty".</p>
     *  <p>Note: All endnotes are formatted with the default style for endnotes.
     *  (This doesn't conform with the file format specification, but in LaTeX
     *  all endnotes are usually formatted in a fixed style.)</p>
     *
     *  @param ldp    the <code>LaTeXDocumentPortion</code> to add definitions to.
     */
    public void convertEndnotesConfiguration(LaTeXDocumentPortion ldp) {
        // Note: Continuation notices are not supported in LaTeX
        convertFootEndnotesConfiguration(wsc.getEndnotesConfiguration(),"end",ldp);
    }

    /** <p>Convert {foot|end}notes configuration.</p>
     *  <p>Note: All {foot|end}notes are formatted with the default style for {foot|end}footnotes.
     *  (This doesn't conform with the file format specification, but in LaTeX
     *  all {foot|end}notes are usually formatted in a fixed style.)</p>
     *
     *  @param ldp    the <code>LaTeXDocumentPortion</code> to add definitions to.
     */
    private void convertFootEndnotesConfiguration(PropertySet notes, String sType, LaTeXDocumentPortion ldp) {
        if (config.formatting()<Config.CONVERT_BASIC) { return; }
        String sTypeShort = sType.equals("foot") ? "fn" : "en";
        if (notes==null) { return; }
        ldp.append("% ").append(sType).append("notes configuration").nl()
           .append("\\makeatletter").nl();

        // The numbering style is controlled by \the{foot|end}note
        String sFormat = notes.getProperty(XMLString.STYLE_NUM_FORMAT);
        if (sFormat!=null) {
            ldp.append("\\renewcommand\\the").append(sType).append("note{")
               .append(ListStyleConverter.numFormat(sFormat))
               .append("{").append(sType).append("note}}").nl();
        }

        // Number {foot|end}notes by sections
        if ("chapter".equals(notes.getProperty(XMLString.TEXT_START_NUMBERING_AT))) {
            ldp.append("\\@addtoreset{").append(sType).append("note}{section}").nl();
        }

        // Set start value offset (default 0)
        int nStartValue = Misc.getPosInteger(notes.getProperty(XMLString.TEXT_START_VALUE),0);
        if (nStartValue!=0) {
            ldp.append("\\setcounter{").append(sType).append("note}{"+nStartValue+"}").nl();
        }

        if (config.formatting()>=Config.CONVERT_MOST) {
            // The formatting of the {foot|end}note citation is controlled by \@make{fn|en}mark
            String sCitBodyStyle = notes.getProperty(XMLString.TEXT_CITATION_BODY_STYLE_NAME);
            if (sCitBodyStyle!=null && wsc.getTextStyle(sCitBodyStyle)!=null) {
                BeforeAfter baText = new BeforeAfter();
                palette.getCharSc().applyTextStyle(sCitBodyStyle,baText,new Context());
                ldp.append("\\renewcommand\\@make").append(sTypeShort).append("mark{\\mbox{")
                   .append(baText.getBefore())
                   .append("\\@the").append(sTypeShort).append("mark")
                   .append(baText.getAfter())
                   .append("}}").nl();
            }

            // The layout and formatting of the {foot|end}note is controlled by \@make{fn|en}text
            String sCitStyle = notes.getProperty(XMLString.TEXT_CITATION_STYLE_NAME);
            String sStyleName = notes.getProperty(XMLString.TEXT_DEFAULT_STYLE_NAME);
            if (sStyleName!=null) {
                BeforeAfter baText = new BeforeAfter();
                palette.getCharSc().applyTextStyle(sCitStyle,baText,new Context());
                ParStyle style = wsc.getParStyle(sStyleName);
                if (style!=null) {
                    BeforeAfter baPar = new BeforeAfter();
                    palette.getCharSc().applyHardCharFormatting(style,baPar);
                    ldp.append("\\renewcommand\\@make").append(sTypeShort)
                       .append("text[1]{\\noindent")
                       .append(baText.getBefore())
                       .append("\\@the").append(sTypeShort).append("mark\\ ")
                       .append(baText.getAfter())
                       .append(baPar.getBefore())
                       .append("#1")
                       .append(baPar.getAfter());
                    ldp.append("}").nl();
                }
            }
        }

        ldp.append("\\makeatother").nl();
    }

}
