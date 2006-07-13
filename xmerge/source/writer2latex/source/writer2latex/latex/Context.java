/************************************************************************
 *
 *  Context.java
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

package writer2latex.latex;

import writer2latex.office.XMLString;
import writer2latex.office.StyleWithProperties;

public class Context {

    // *** Formatting Info (current values in the source OOo document) ***

    // Current list style
    private String sListStyleName = null;

    // Current character formatting attributes
    private String sFontName = null;
    private String sFontStyle = null;
    private String sFontVariant = null;
    private String sFontWeight = null;
    private String sFontSize = null;
    private String sFontColor = null;
    private String sLang = null;
    private String sCountry = null;

    // *** Structural Info (identifies contructions in the LaTeX document) ***

    // within the header or footer of a pagestyle
    private boolean bInHeaderFooter = false;

    // within a table cell
    private boolean bInTable = false;

    // within a multicols environment
    private boolean bInMulticols = false;

    // within a list of this level
    private int nListLevel = 0;

    // within a minipage environment
    private boolean bInFrame = false;

    // within a \footnote or \endnote
    private boolean bInFootnote = false;

    // in verbatim mode
    private boolean bVerbatim = false;

    // *** Special Info ***

    // Inside a construction, where footnotes are disallowed
    private boolean bNoFootnotes = false;

    // Inside an area, where lists are ignored
    private boolean bIgnoreLists = false;

    // *** Accessor Methods ***

    public void setListStyleName(String sListStyleName) { this.sListStyleName = sListStyleName; }

    public String getListStyleName() { return sListStyleName; }

    public void setFontName(String sFontName) { this.sFontName = sFontName; }

    public String getFontName() { return sFontName; }

    public void setFontStyle(String sFontStyle) { this.sFontStyle = sFontStyle; }

    public String getFontStyle() { return sFontStyle; }

    public void setFontVariant(String sFontVariant) { this.sFontVariant = sFontVariant; }

    public String getFontVariant() { return sFontVariant; }

    public void setFontWeight(String sFontWeight) { this.sFontWeight = sFontWeight; }

    public String getFontWeight() { return sFontWeight; }

    public void setFontSize(String sFontSize) { this.sFontSize = sFontSize; }

    public String getFontSize() { return sFontSize; }

    public void setFontColor(String sFontColor) { this.sFontColor = sFontColor; }

    public String getFontColor() { return sFontColor; }

    public void setLang(String sLang) { this.sLang = sLang; }

    public String getLang() { return sLang; }

    public void setCountry(String sCountry) { this.sCountry = sCountry; }

    public String getCountry() { return sCountry; }

    public void setInHeaderFooter(boolean bInHeaderFooter) {
        this.bInHeaderFooter = bInHeaderFooter;
    }

    public boolean isInHeaderFooter() { return bInHeaderFooter; }

    public void setInTable(boolean bInTable) { this.bInTable = bInTable; }

    public boolean isInTable() { return bInTable; }

    public void setInMulticols(boolean bInMulticols) {
        this.bInMulticols = bInMulticols;
    }

    public boolean isInMulticols() { return bInMulticols; }

    public void setListLevel(int nListLevel) { this.nListLevel = nListLevel; }

    public void incListLevel() { nListLevel++; }

    public int getListLevel() { return nListLevel; }

    public void setInFrame(boolean bInFrame) { this.bInFrame = bInFrame; }

    public boolean isInFrame() { return bInFrame; }

    public void setInFootnote(boolean bInFootnote) {
        this.bInFootnote = bInFootnote;
    }

    public boolean isInFootnote() { return bInFootnote; }

    public void setNoFootnotes(boolean bNoFootnotes) {
        this.bNoFootnotes = bNoFootnotes;
    }

    public boolean isNoFootnotes() { return bNoFootnotes; }

    public void setIgnoreLists(boolean bIgnoreLists) {
        this.bIgnoreLists = bIgnoreLists;
    }

    public boolean isIgnoreLists() { return bIgnoreLists; }

    public boolean isVerbatim() { return bVerbatim; }

    public void setVerbatim(boolean bVerbatim) { this.bVerbatim = bVerbatim; }

    // update context

    public void updateFormattingFromStyle(StyleWithProperties style) {
        String s;

        if (style==null) { return; }

        s = style.getProperty(XMLString.STYLE_FONT_NAME);
        if (s!=null) { setFontName(s); }

        s = style.getProperty(XMLString.FO_FONT_STYLE);
        if (s!=null) { setFontStyle(s); }

        s = style.getProperty(XMLString.FO_FONT_VARIANT);
        if (s!=null) { setFontVariant(s); }

        s = style.getProperty(XMLString.FO_FONT_WEIGHT);
        if (s!=null) { setFontWeight(s); }

        s = style.getProperty(XMLString.FO_FONT_SIZE);
        if (s!=null) { setFontSize(s); }

        s = style.getProperty(XMLString.FO_COLOR);
        if (s!=null) { setFontColor(s); }

        s = style.getProperty(XMLString.FO_LANGUAGE);
        if (s!=null) { setLang(s); }

        s = style.getProperty(XMLString.FO_COUNTRY);
        if (s!=null) { setCountry(s); }
    }

    public void resetFormattingFromStyle(StyleWithProperties style) {
        setFontName(null);
        setFontStyle(null);
        setFontVariant(null);
        setFontWeight(null);
        setFontSize(null);
        setFontColor(null);
        setLang(null);
        setCountry(null);
        updateFormattingFromStyle(style);
    }


    // clone this Context
    public Object clone() {
        Context newContext = new Context();

        newContext.setListStyleName(sListStyleName);
        newContext.setFontName(sFontName);
        newContext.setFontStyle(sFontStyle);
        newContext.setFontVariant(sFontVariant);
        newContext.setFontWeight(sFontWeight);
        newContext.setFontSize(sFontSize);
        newContext.setFontColor(sFontColor);
        newContext.setLang(sLang);
        newContext.setCountry(sCountry);
        newContext.setInHeaderFooter(bInHeaderFooter);
        newContext.setInTable(bInTable);
        newContext.setInMulticols(bInMulticols);
        newContext.setListLevel(nListLevel);
        newContext.setInFrame(bInFrame);
        newContext.setInFootnote(bInFootnote);
        newContext.setNoFootnotes(bNoFootnotes);
        newContext.setIgnoreLists(bIgnoreLists);

        return newContext;
    }

}
