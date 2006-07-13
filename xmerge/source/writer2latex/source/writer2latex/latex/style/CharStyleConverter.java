/************************************************************************
 *
 *  CharStyleConverter.java
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

package writer2latex.latex.style;

import java.util.Hashtable;

import writer2latex.util.*;
import writer2latex.office.*;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

/** This class creates LaTeX code from OOo character formatting
   Character formatting in OOo includes font, font effects/decorations and color.
   In addition it includes language/country information, this is however handled
   by the class <code>writer2latex.latex.style.I18n</code>
 */
public class CharStyleConverter extends StyleConverter {

    // Cache of converted font declarations
    private Hashtable fontDecls = new Hashtable();

    // Which formatting should we export?
    private boolean bUseColor;
    private boolean bIgnoreHardFontsize;
    private boolean bIgnoreFontsize;
    private boolean bIgnoreFont;
    private boolean bIgnoreAll;
    private boolean bUseUlem;
    // Do we need actually use ulem.sty or \textsubscript?
    private boolean bNeedUlem = false;
    private boolean bNeedSubscript = false;

    /** <p>Constructs a new <code>CharStyleConverter</code>.</p>
     */
    public CharStyleConverter(WriterStyleCollection wsc, Config config, ConverterPalette palette) {
        super(wsc,config,palette);

        bUseColor = config.useColor();
        bUseUlem = config.useUlem();

        // No character formatting at all:
        bIgnoreAll = config.formatting()==Config.IGNORE_ALL;
        // No font family or size:
        bIgnoreFont = config.formatting()<=Config.IGNORE_MOST;
        // No fontsize:
        bIgnoreFontsize = config.formatting()<=Config.CONVERT_BASIC;
        // No hard fontsize
        bIgnoreHardFontsize = config.formatting()<=Config.CONVERT_MOST;
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bUseColor) {
            pack.append("\\usepackage{color}").nl();
        }
        if (bNeedUlem) {
            pack.append("\\usepackage[normalem]{ulem}").nl();
        }
        if (bNeedSubscript) {
            decl.append("\\newcommand\\textsubscript[1]{\\ensuremath{{}_{\\text{#1}}}}").nl();
        }
        if (!styleNames.isEmpty()) {
            decl.append("% Text styles").nl().append(declarations);
        }
    }

    /** <p>Use a text style in LaTeX.</p>
     *  @param <code>sName</code> the name of the text style
     *  @param <code>ba</code> a <code>BeforeAfter</code> to put code into
     */
    public void applyTextStyle(String sName, BeforeAfter ba, Context context) {
        if (sName==null || bIgnoreAll) { return; }

        // Style already converted?
        if (styleMap.contains(sName)) {
            ba.add(styleMap.getBefore(sName),styleMap.getAfter(sName));
            context.updateFormattingFromStyle(wsc.getTextStyle(sName));
            // it's verbatim if specified as sich in the configuration
            StyleMap sm = config.getTextStyleMap();
            context.setVerbatim(sm.contains(sName) && sm.getVerbatim(sName));
            return;
        }

        // The style may already be declared in the configuration:
        StyleMap sm = config.getTextStyleMap();
        if (sm.contains(sName)) {
            styleMap.put(sName,sm.getBefore(sName),sm.getAfter(sName));
            applyTextStyle(sName,ba,context);
            return;
        }

        // Get the style, if it exists:
        StyleWithProperties style = wsc.getTextStyle(sName);
        if (style==null) {
            styleMap.put(sName,"","");
            applyTextStyle(sName,ba,context);
            return;
        }

        // Convert automatic style
        if (style.isAutomatic()) {
            palette.getI18n().applyLanguage(style,false,true,ba);
            applyFont(style,false,true,ba,context);
            applyFontEffects(style,true,ba);
            context.updateFormattingFromStyle(wsc.getTextStyle(sName));
            return;
        }

        // Convert soft style:
        // This must be converted relative to a blank context!
        BeforeAfter baText = new BeforeAfter();
        palette.getI18n().applyLanguage(style,false,true,baText);
        applyFont(style,false,true,baText,new Context());
        applyFontEffects(style,true,baText);
        // declare the text style (\newcommand)
        String sTeXName = styleNames.getExportName(sName);
        styleMap.put(sName,"\\textstyle"+sTeXName+"{","}");
        declarations.append("\\newcommand\\textstyle")
            .append(sTeXName).append("[1]{")
            .append(baText.getBefore()).append("#1").append(baText.getAfter())
            .append("}").nl();
        applyTextStyle(sName,ba,context);
    }

    // Get the font name from a char style
    public String getFontName(String sStyleName) {
        StyleWithProperties style = wsc.getTextStyle(sStyleName);
        if (style==null) { return null; }
        String sName = style.getProperty(XMLString.STYLE_FONT_NAME);
        if (sName==null) { return null; }
        FontDeclaration fd = wsc.getFontDeclaration(sName);
        if (fd==null) { return null; }
        return fd.getProperty(XMLString.FO_FONT_FAMILY);
    }

    /** <p>Apply hard character formatting (no inheritance).</p>
     *  <p>This is used in sections and {foot|end}notes</p>
     *  @param <code>style</code> the style to use
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to
     */
    public void applyHardCharFormatting(StyleWithProperties style, BeforeAfter ba) {
        palette.getI18n().applyLanguage(style,true,false,ba);
        applyFont(style,true,false,ba,new Context());
        if (!ba.isEmpty()) { ba.add(" ",""); }
    }

    /** <p>Apply all font attributes (family, series, shape, size and color).</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyFont(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null) { return; }
        applyNfssSize(style,bDecl,bInherit,ba,context);
        applyNfssFamily(style,bDecl,bInherit,ba,context);
        applyNfssSeries(style,bDecl,bInherit,ba,context);
        applyNfssShape(style,bDecl,bInherit,ba,context);
        applyColor(style,bDecl,bInherit,ba,context);
    }

    /** <p>Reset to normal font, size and color.</p>
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyNormalFont(BeforeAfter ba) {
        if (bUseColor) {
            ba.add("\\normalfont\\normalsize\\normalcolor","");
        }
        else {
            ba.add("\\normalfont\\normalsize","");
        }
    }

    /** <p>Apply default font attributes (family, series, shape, size and color).</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>ldp</code> the <code>LaTeXDocumentPortion</code> to add LaTeX code to.
     */
    public void applyDefaultFont(StyleWithProperties style, LaTeXDocumentPortion ldp) {
        if (style==null) { return; }

        String s = convertFontDeclaration(style.getProperty(XMLString.STYLE_FONT_NAME));
        if (s!=null){
            ldp.append("\\renewcommand\\familydefault{\\")
               .append(s).append("default}").nl();
        } // TODO: Else read props directly from the style

        s = nfssSeries(style.getProperty(XMLString.FO_FONT_WEIGHT));
        if (s!=null) {
            ldp.append("\\renewcommand\\seriesdefault{\\")
               .append(s).append("default}").nl();
        }

        s = nfssShape(style.getProperty(XMLString.FO_FONT_VARIANT),
                             style.getProperty(XMLString.FO_FONT_STYLE));
        if (s!=null) {
            ldp.append("\\renewcommand\\shapedefault{\\")
               .append(s).append("default}").nl();
        }

        if (bUseColor) {
            s = color(style.getProperty(XMLString.FO_COLOR));
            if (s!=null) {
                ldp.append("\\renewcommand\\normalcolor{\\color")
                   .append(s).append("}").nl();
            }
        }
    }

    /** <p>Apply font effects (position, underline, crossout, change case.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyFontEffects(StyleWithProperties style, boolean bInherit, BeforeAfter ba) {
        if (style==null) { return; }
        applyTextPosition(style, bInherit, ba);
        applyUnderline(style, bInherit, ba);
        applyCrossout(style, bInherit, ba);
        applyChangeCase(style, bInherit, ba);
    }

    /** <p>Apply font color.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyColor(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null || !bUseColor || bIgnoreAll) { return; }
        String sColor = color(style.getProperty(XMLString.FO_COLOR,bInherit));
        if (sColor==null) { return; }
        if (sColor.equals(color(context.getFontColor()))) { return; }
        if (bDecl) { ba.add("\\color"+sColor,""); }
        else { ba.add("\\textcolor"+sColor+"{","}"); }
    }

    /** <p>Apply a specific font color.</p>
     *  @param <code>sColor</code> the rgb color to use
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyThisColor(String sColor, boolean bDecl, BeforeAfter ba) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (sColor==null || !bUseColor || bIgnoreAll) { return; }
        String s = color(sColor);
        if (s==null) { return; }
        if (bDecl) { ba.add("\\color"+s,""); }
        else { ba.add("\\textcolor"+s+"{","}"); }
    }

    // Remaining methods are private

    /** <p>Apply font family.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyNfssFamily(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null || bIgnoreFont) { return; }
        String sFontName=style.getProperty(XMLString.STYLE_FONT_NAME,bInherit);
        if (sFontName!=null){
            String sFamily = convertFontDeclaration(sFontName);
            if (sFamily==null) { return; }
            if (sFamily.equals(convertFontDeclaration(context.getFontName()))) { return; }
            if (bDecl) { ba.add("\\"+sFamily+"family",""); }
            else { ba.add("\\text"+sFamily+"{","}"); }
        } // TODO: Else read props directly from the style
    }

    /** <p>Apply font series.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyNfssSeries(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null) { return; }
        if (bIgnoreAll) { return; }
        String sSeries = nfssSeries(style.getProperty(XMLString.FO_FONT_WEIGHT,bInherit));
        if (sSeries==null) { return; }
        if (sSeries.equals(nfssSeries(context.getFontWeight()))) { return; }
        if (bDecl) { ba.add("\\"+sSeries+"series",""); }
        else { ba.add("\\text"+sSeries+"{","}"); }
    }

    /** <p>Apply font shape.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyNfssShape(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null) { return; }
        if (bIgnoreAll) { return; }
        String sShape = nfssShape(style.getProperty(XMLString.FO_FONT_VARIANT, bInherit),
                                  style.getProperty(XMLString.FO_FONT_STYLE, bInherit));
        if (sShape==null) { return; }
        if (sShape.equals(nfssShape(context.getFontVariant(),context.getFontStyle()))) { return; }
        if (bDecl) { ba.add("\\"+sShape+"shape",""); }
        else { ba.add("\\text"+sShape+"{","}"); }
    }

    /** <p>Apply font size.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyNfssSize(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba, Context context) {
        // Note: if bDecl is true, nothing will be put in the "after" part of ba.
        if (style==null|| bIgnoreFontsize || (bIgnoreHardFontsize && style.isAutomatic())) { return; }
        if (style.getProperty(XMLString.FO_FONT_SIZE, bInherit)==null) { return; }
        String sSize = nfssSize(style.getAbsoluteProperty(XMLString.FO_FONT_SIZE));
        if (sSize==null) { return; }
        if (sSize.equals(nfssSize(context.getFontSize()))) { return; }
        if (bDecl) { ba.add(sSize,""); }
        else { ba.add("{"+sSize+" ","}"); }
    }

    // Remaining methods are not context-sensitive

    /** <p>Apply text position.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyTextPosition(StyleWithProperties style, boolean bInherit, BeforeAfter ba) {
        if (style==null) { return; }
        if (bIgnoreAll) { return; }
        String s = textPosition(style.getProperty(XMLString.STYLE_TEXT_POSITION, bInherit));
        if (s!=null) { ba.add(s+"{","}"); }
    }

    /** <p>Apply text underline.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyUnderline(StyleWithProperties style, boolean bInherit, BeforeAfter ba) {
        if (style==null || !bUseUlem) { return; }
        if (bIgnoreAll) { return; }
        String s = underline(style.getProperty(XMLString.STYLE_TEXT_UNDERLINE, bInherit));
        if (s!=null) { bNeedUlem = true; ba.add(s+"{","}"); }
    }

    /** <p>Apply text crossout.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyCrossout(StyleWithProperties style, boolean bInherit, BeforeAfter ba) {
        if (style==null || !bUseUlem) { return; }
        if (bIgnoreAll) { return; }
        String s = crossout(style.getProperty(XMLString.STYLE_TEXT_CROSSING_OUT, bInherit));
        if (s!=null) { bNeedUlem = true; ba.add(s+"{","}"); }
    }

    /** <p>Apply change case.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    private void applyChangeCase(StyleWithProperties style, boolean bInherit, BeforeAfter ba) {
        if (style==null) { return; }
        if (bIgnoreAll) { return; }
        String s = changeCase(style.getProperty(XMLString.FO_TEXT_TRANSFORM));
        if (s!=null) { ba.add(s+"{","}"); }
    }

    /** <p>Convert font declarations to LaTeX.</p>
     *  <p>It returns a generic LaTeX font family (rm, tt, sf).</p>
     *  <p>It returns null if the font declaration doesn't exist.</p>
     *  @return <code>String</code> with a LaTeX generic fontfamily
     *  @param  <code>sName</code> the name of the font declaration
     */
    private String convertFontDeclaration(String sName) {
        FontDeclaration fd = wsc.getFontDeclaration(sName);
        if (fd==null) { return null; }
        if (!fontDecls.containsKey(sName)) {
            String sFontFamily = fd.getProperty(XMLString.FO_FONT_FAMILY);
            String sFontPitch = fd.getProperty(XMLString.STYLE_FONT_PITCH);
            String sFontFamilyGeneric = fd.getProperty(XMLString.STYLE_FONT_FAMILY_GENERIC);
            fontDecls.put(sName,nfssFamily(sFontFamily,sFontFamilyGeneric,sFontPitch));
        }
        return (String) fontDecls.get(sName);
    }

    // The remaining methods are static helpers to convert single style properties

    // Font change. These methods return the declaration form if the paramater
    // bDecl is true, and otherwise the command form

    private static final String nfssFamily(String sFontFamily, String sFontFamilyGeneric,
                                   String sFontPitch){
        // Note: Defaults to rm
        // TODO: What about decorative, script, system?
        if ("fixed".equals(sFontPitch)) return "tt";
        else if ("modern".equals(sFontFamilyGeneric)) return "tt";
        else if ("swiss".equals(sFontFamilyGeneric)) return "sf";
        else return "rm";
    }

    private static final String nfssSeries(String sFontWeight){
        if (sFontWeight==null) return null;
        if ("bold".equals(sFontWeight)) return "bf";
        else return "md";
    }

    private static final String nfssShape(String sFontVariant, String sFontStyle){
        if (sFontVariant==null && sFontStyle==null) return null;
        if ("small-caps".equals(sFontVariant)) return "sc";
        else if ("italic".equals(sFontStyle)) return "it";
        else if ("oblique".equals(sFontStyle)) return "sl";
        else return "up";
    }

    private static final String nfssSize(String sFontSize){
        if (sFontSize==null) return null;
        return "\\fontsize{"+sFontSize+"}{"+Misc.multiply("120%",sFontSize)+"}\\selectfont";
    }

    private static final String color(String sColor){
        if ("#000000".equalsIgnoreCase(sColor)) { return "{black}"; }
        else if ("#ff0000".equalsIgnoreCase(sColor)) { return "{red}"; }
        else if ("#00ff00".equalsIgnoreCase(sColor)) { return "{green}"; }
        else if ("#0000ff".equalsIgnoreCase(sColor)) { return "{blue}"; }
        else if ("#ffff00".equalsIgnoreCase(sColor)) { return "{yellow}"; }
        else if ("#ff00ff".equalsIgnoreCase(sColor)) { return "{magenta}"; }
        else if ("#00ffff".equalsIgnoreCase(sColor)) { return "{cyan}"; }
        //no white, since we don't have background colors:
        //else if ("#ffffff".equalsIgnoreCase(sColor)) { return "{white}"; }
        else {
            float sRed, sGreen, sBlue;
            if (sColor==null || sColor.length()!=7) return null;
            sRed=(float)Misc.getIntegerFromHex(sColor.substring(1,3),0)/255;
            sGreen=(float)Misc.getIntegerFromHex(sColor.substring(3,5),0)/255;
            sBlue=(float)Misc.getIntegerFromHex(sColor.substring(5,7),0)/255;
            // avoid very bright colors (since we don't have background colors):
            if (sRed+sGreen+sBlue>2.7) { return "{black}"; }
            else { return "[rgb]{"+sRed+","+sGreen+","+sBlue+"}"; }
        }
    }

    // other character formatting

    private final String textPosition(String sTextPosition){
        if (sTextPosition==null) return null;
        if (sTextPosition.startsWith("super")) return "\\textsuperscript";
        if (sTextPosition.startsWith("sub") || sTextPosition.startsWith("-")) {
            bNeedSubscript = true;
            return "\\textsubscript";
        }
        if (sTextPosition.startsWith("0%")) return null;
        return "\\textsuperscript";
    }

    private static final String underline(String sUnderline) {
        if (sUnderline==null) { return null; }
        if (sUnderline.equals("none")) { return null; }
        if (sUnderline.indexOf("wave")>=0) { return "\\uwave"; }
        return "\\uline";
    }

    private static final String crossout(String sCrossout) {
        if (sCrossout==null) { return null; }
        if (sCrossout.equals("X")) { return "\\xout"; }
        if (sCrossout.equals("slash")) { return "\\xout"; }
        return "\\sout";
    }

    private static final String changeCase(String sTextTransform){
        if ("lowercase".equals(sTextTransform)) return "\\MakeLowercase";
        if ("uppercase".equals(sTextTransform)) return "\\MakeUppercase";
        return null;
    }

}
