/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// DJP ToDo: need way of specifying fg/bg colors on ws->DOM

package org.openoffice.xmerge.converter.xml;

import java.awt.Color;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.util.Debug;

/**
 * Represents a text {@code Style} in an OpenOffice document.
 */
public class TextStyle extends Style implements Cloneable {

    /** Indicates <i>bold</i> text. */
    final private static int BOLD        = 0x01;
    /** Indicates <i>italic</i> text. */
    final private static int ITALIC      = 0x02;
    /** Indicates <i>underlined</i> text. */
    final private static int UNDERLINE   = 0x04;
    /** Indicates <i>strike-through</i> in the text. */
    final private static int STRIKETHRU  = 0x08;
    /** Indicates <i>superscripted</i> text. */
    final private static int SUPERSCRIPT = 0x10;
    /** Indicates <i>subscripted</i> text. */
    final private static int SUBSCRIPT   = 0x20;

    /** Values of text attributes. */
    private int values = 0;
    /** Bitwise mask of text attributes. */
    private int mask = 0;

    /** Font size in points. */
    private int sizeInPoints = 0;
    /** Font name. */
    private String fontName = null;
    /** Font {@code Color}. */
    private Color fontColor = null;
    /** Background {@code Color}. */
    private Color bgColor = null;

    /**
     * Constructor for use when going from DOM to client device format.
     *
     * @param  node  The <i>style:style</i> {@code Node} containing the
     *               {@code Style}.  (This {@code Node} is assumed have a
     *               <i>family</i> attribute of <i>text</i>).
     * @param  sc    The {@code StyleCatalog}, which is used for looking up
     *               ancestor {@code Style} objects.
     */
    public TextStyle(Node node, StyleCatalog sc) {
        super(node, sc);

        // Run through the attributes of this node, saving
        // the ones we're interested in.
        NamedNodeMap attrNodes = node.getAttributes();
        if (attrNodes != null) {
            int len = attrNodes.getLength();
            for (int i = 0; i < len; i++) {
                Node attr = attrNodes.item(i);
                handleAttribute(attr.getNodeName(), attr.getNodeValue());
            }
        }

        // Look for children.  Only ones we care about are "style:properties"
        // nodes.  If any are found, recursively traverse them, passing
        // along the style element to add properties to.
        if (!node.hasChildNodes()) {
            return;
        }
        NodeList children = node.getChildNodes();
        int len = children.getLength();
        for (int i = 0; i < len; i++) {
            Node child = children.item(i);
            String nodeName = child.getNodeName();
            if (nodeName.equals("style:properties")) {
                NamedNodeMap childAttrNodes = child.getAttributes();
                if (childAttrNodes != null) {
                    int nChildAttrNodes = childAttrNodes.getLength();
                    for (int j = 0; j < nChildAttrNodes; j++) {
                        Node attr = childAttrNodes.item(j);
                        handleAttribute(attr.getNodeName(),
                        attr.getNodeValue());
                    }
                }
            }
        }
    }

    /**
     * Constructor for use when going from client device format to DOM
     *
     * @param  name     Name of text {@code Style}.  Can be {@code null}.
     * @param  family   Family of text {@code Style} (usually <i>text</i>).
     *                  Can be {@code null}.
     * @param  parent   Name of parent text {@code Style}, or {@code null} for
     *                  none.
     * @param  mask     Bitwise mask of text attributes that this text
     *                  {@code Style} will specify.  Can be any combination of
     *                  the following, or'ed together: {@link #BOLD},
     *                  {@link #ITALIC}, {@link #UNDERLINE}, {@link #STRIKETHRU},
     *                  {@link #SUPERSCRIPT}, {@link #SUBSCRIPT}. This parameter
     *                  determines what attributes this {@code Style} will
     *                  specify. When an attribute is specified in a
     *                  {@code Style}, its value can be either <i>on</i> or
     *                  <i>off</i>.  The on/off value for each attribute is
     *                  controlled by the {@code values} parameter.
     * @param values    Values of text attributes that this text {@code Style}
     *                  will be setting.  Any of the attributes ({@link #BOLD},
     *                  etc) listed for {@code mask} can be used for this.
     * @param fontSize  Font size in points.
     * @param fontName  Name of font.
     * @param sc        The {@code StyleCatalog}, which is used for looking up
     *                  ancestor {@code Style} objects.
     */
    public TextStyle(String name, String family, String parent,
    int mask, int values, int fontSize, String fontName, StyleCatalog sc) {
        super(name, family, parent, sc);
        this.mask = mask;
        this.values = values;
        this.sizeInPoints = fontSize;
        this.fontName = fontName;
    }

    /**
     * Parse a color specification of the form <i>#rrggbb</i>
     *
     * @param   value  {@code Color} specification to parse.
     *
     * @return  The {@code Color} associated the value.
     */
    private Color parseColorString(String value) {
        int red = 0;
        int green = 0;
        int blue = 0;
        try {
            // Assume color value is of form #rrggbb
            red = Integer.parseInt(value.substring(1, 3), 16);
            green = Integer.parseInt(value.substring(3, 5), 16);
            blue = Integer.parseInt(value.substring(5, 7), 16);
        } catch (NumberFormatException e) {
            Debug.log(Debug.ERROR, "Problem parsing a color string", e);
        } catch (IndexOutOfBoundsException e) {
            Debug.log(Debug.ERROR, "Problem parsing a color string", e);
        }
        return new Color(red, green, blue);
    }

    /**
     * Set an attribute.
     *
     * @param  attr   The attribute to set.
     * @param  value  The attribute value to set.
     */
    private void handleAttribute(String attr, String value) {

        if (attr.equals("fo:font-weight")) {
            if (value.equals("bold")) turnAttributesOn(BOLD);
            else if (value.equals("normal")) turnAttributesOff(BOLD);
        }

        else if (attr.equals("fo:font-style")) {
            if (value.equals("italic")) turnAttributesOn(ITALIC);
            else if (value.equals("oblique")) turnAttributesOn(ITALIC);
            else if (value.equals("normal")) turnAttributesOff(ITALIC);
        }

        else if (attr.equals("style:text-underline")) {
            if (value.equals("none"))
                turnAttributesOff(UNDERLINE);
            else
                turnAttributesOn(UNDERLINE);
        }

        else if (attr.equals("style:text-crossing-out")) {
            if (value.equals("none"))
                turnAttributesOff(STRIKETHRU);
            else
                turnAttributesOn(STRIKETHRU);
        }

        else if (attr.equals("style:text-position")) {
            if (value.startsWith("super "))
                turnAttributesOn(SUPERSCRIPT);
            else if (value.startsWith("sub "))
                turnAttributesOn(SUBSCRIPT);
            else if (value.startsWith("0% "))
                turnAttributesOff(SUPERSCRIPT | SUBSCRIPT);
            else {
                String firstPart = value.substring(0, value.indexOf(' '));
                if (firstPart.endsWith("%")) {
                    firstPart = firstPart.substring(0, value.indexOf('%'));
                    int amount;
                    try {
                        amount = Integer.parseInt(firstPart);
                    } catch (NumberFormatException e) {
                        amount = 0;
                        Debug.log(Debug.ERROR, "Problem with style:text-position tag", e);
                    }
                    if (amount < 0) turnAttributesOn(SUBSCRIPT);
                    else if (amount > 0) turnAttributesOn(SUPERSCRIPT);
                }
            }
        }

        else if (attr.equals("fo:font-size")) {
            if (value.endsWith("pt")) {
                String num = value.substring(0, value.length() - 2);
                sizeInPoints = Integer.parseInt(num);
            }
        }

        else if (attr.equals("style:font-name"))
            fontName = value;

        else if (attr.equals("fo:color"))
            fontColor = parseColorString(value);

        else if (attr.equals("style:text-background-color"))
            bgColor = parseColorString(value);

        else if (isIgnored(attr)) {}

        else {
            Debug.log(Debug.INFO, "TextStyle Unhandled: " + attr + "=" + value);
        }
    }

    /**
     * Return the font size for this {@code Style}.
     *
     * @return  The font size in points
     */
    public int getFontSize() {
        return sizeInPoints;
    }

    /**
     * Return the name of the font for this {@code Style}.
     *
     * @return  Name of font, or null if no font is specified by this
     *          {@code Style}.
     */
    public String getFontName() {
        return fontName;
    }

    /**
     * Return the font {@code Color} for this {@code Style}.
     *
     * <p>Can be {@code null} if none was specified.</p>
     *
     * @return  {@code Color} value for this {@code Style}. Can be {@code null}.
     */
    public Color getFontColor() {
        return fontColor;
    }

    /**
     * Return the background {@code Color} for this  {@code Style}.
     *
     * <p>Can be {@code null} if none was specified.</p>
     *
     * @return  Background {@code Color} value for this {@code Style}. Can be
     *          {@code null}.
     */
    public Color getBackgroundColor() {
        return bgColor;
    }

    /**
     * Return a {@code Style} object corresponding to this one, but with all of
     * the inherited information from parent {@code Style} objects filled in.
     *
     * <p>The object returned will be a new object, not a reference to this
     * object, even if it does not need any information added.</p>
     *
     * @return  The {@code StyleCatalog} in which to look up ancestors.
     */
    @Override
    public Style getResolved() {
        // Create a new object to return, which is a clone of this one.
        TextStyle resolved = null;
        try {
            resolved = (TextStyle)this.clone();
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Can't clone", e);
        }

        // Look up the parentStyle.  (If there is no style catalog
        // specified, we can't do any lookups.)
        TextStyle parentStyle = null;
        if (sc != null) {
            if (parent != null) {
                parentStyle = (TextStyle)sc.lookup(parent, family, null,
                           this.getClass());
                if (parentStyle == null)
                    Debug.log(Debug.ERROR, "parent style lookup of "
                      + parent + " failed!");
                else
                    parentStyle = (TextStyle)parentStyle.getResolved();

            } else if (!name.equals("DEFAULT_STYLE")) {
                parentStyle = (TextStyle)sc.lookup("DEFAULT_STYLE", null,
                    null, this.getClass());
            }
        }

        // If we found a parent, for any attributes which we don't have
        // set, try to get the values from the parent.
        if (parentStyle != null) {
            parentStyle = (TextStyle)parentStyle.getResolved();

            if ((sizeInPoints == 0) && (parentStyle.sizeInPoints != 0))
                resolved.sizeInPoints = parentStyle.sizeInPoints;
            if ((fontName == null) && (parentStyle.fontName != null))
                resolved.fontName = parentStyle.fontName;
            if ((fontColor == null) && (parentStyle.fontColor != null))
                resolved.fontColor = parentStyle.fontColor;
            if ((bgColor == null) && (parentStyle.bgColor != null))
                resolved.bgColor = parentStyle.bgColor;
            for (int m = BOLD; m <= SUBSCRIPT; m = m << 1) {
                if (((mask & m) == 0) && ((parentStyle.mask & m) != 0)) {
                    resolved.mask |= m;
                    resolved.values |= (parentStyle.mask & m);
                }
            }

        }
        return resolved;
    }

    /**
     * Set one or more text attributes to <i>on</i>.
     *
     * @param  flags  Flag values to set <i>on</i>.
     */
    private void turnAttributesOn(int flags) {
        mask |= flags;
        values |= flags;
    }

    /**
     * Set one or more text attributes to <i>off</i>.
     *
     * @param  flags  The flag values to set <i>off</i>.
     */
    private void turnAttributesOff(int flags) {
        mask |= flags;
        values &= ~flags;
    }

    /**
     * Private function to return the value as an element in a Comma Separated
     * Value (CSV) format.
     *
     * @param   value The value to format.
     *
     * @return  The formatted value.
     */
    private static String toCSV(String value) {
        if (value != null)
            return "\"" + value + "\",";
        else
            return "\"\",";
    }

    /**
     * Private function to return the value as a last element in a Comma
     * Separated Value (CSV) format.
     *
     * @param   value  The value to format.
     *
     * @return  The formatted value.
     */
    private static String toLastCSV(String value) {
        if (value != null)
            return "\"" + value + "\"";
        else
            return "\"\"";
    }

    /**
     * Print a Comma Separated Value (CSV) header line for the spreadsheet dump.
     */
    public static void dumpHdr() {
        System.out.println(toCSV("Name") + toCSV("Family") + toCSV("parent")
        + toCSV("Font") + toCSV("Size")
        + toCSV("Bold") + toCSV("Italic") + toCSV("Underline")
        + toCSV("Strikethru") + toCSV("Superscript") + toLastCSV("Subscript"));
    }

    /**
     * Dump this {@code Style} as a Comma Separated Value (CSV) line.
     */
    public void dumpCSV() {
        String attributes = "";
        for (int bitVal = 0x01; bitVal <= 0x20; bitVal = bitVal << 1) {
            if ((bitVal & mask) != 0) {
                attributes += toCSV(((bitVal & values) != 0) ? "yes" : "no");
            } else attributes += toCSV(null);  // unspecified
        }
        System.out.println(toCSV(name) + toCSV(family) + toCSV(parent)
        + toCSV(fontName) + toCSV("" + sizeInPoints) + attributes + toLastCSV(null));
    }

    /**
     * Create a new {@code Node} in the {@code Document}, and write this
     * {@code Style} to it.
     *
     * @param   parentDoc  Parent {@code Document} of the {@code Node} to create.
     * @param   name       Name to use for the new {@code Node} (e.g.
     *                     <i>style:style</i>)
     *
     * @return  Created {@code Node}.
     */
    @Override
    public Node createNode(org.w3c.dom.Document parentDoc, String name) {
        Element node = parentDoc.createElement(name);
        writeAttributes(node);
        return node;
    }

    /**
     * Return {@code true} if {@code style} specifies as much or less than this
     * {@code Style}, and nothing it specifies contradicts this {@code Style}.
     *
     * @param   style  The {@code Style} to check.
     *
     * @return  {@code true} if {@code style} is a subset, {@code false}
     *          otherwise.
     */
    @Override
    public boolean isSubset(Style style) {
        if (style.getClass() != this.getClass())
                return false;
        TextStyle tStyle = (TextStyle)style;

        if (tStyle.values != values)
                return false;

        if (tStyle.sizeInPoints != 0 && sizeInPoints != tStyle.sizeInPoints)
            return false;

        if (tStyle.fontName != null) {
            if (fontName == null)
                return false;
            if (!fontName.equals(tStyle.fontName))
                return false;
        }

        if (tStyle.fontColor != null) {
            if (fontColor == null)
                return false;
            if (!fontColor.equals(tStyle.fontColor))
                return false;
        }

        if (tStyle.bgColor != null) {
            if (bgColor == null)
                return false;
            if (!bgColor.equals(tStyle.bgColor))
                return false;
        }

        return true;
    }

    /**
     * Write this {@code Style} object's attributes to a {@code Node} in the
     * {@code Document}.
     *
     * @param  node  The {@code Node} to add {@code Style} attributes.
     */
    private void writeAttributes(Element node) {

        if ((mask & BOLD) != 0 && (values & BOLD) != 0)
            node.setAttribute("fo:font-weight", "bold");

        if ((mask & ITALIC) != 0 && (values & ITALIC) != 0)
            node.setAttribute("fo:font-style", "italic");

        if ((mask & UNDERLINE) != 0 && (values & UNDERLINE) != 0)
            node.setAttribute("style:text-underline", "single");

        if ((mask & STRIKETHRU) != 0 && (values & STRIKETHRU) != 0)
            node.setAttribute("style:text-crossing-out", "single-line");

        if ((mask & SUPERSCRIPT) != 0 && (values & SUPERSCRIPT) != 0)
            node.setAttribute("style:text-position", "super 58%");

        if ((mask & SUBSCRIPT) != 0 && (values & SUBSCRIPT) != 0)
            node.setAttribute("style:text-position", "sub 58%");

        if (sizeInPoints != 0) {
            node.setAttribute("fo:font-size", Integer.toString(sizeInPoints) + "pt");
        }

        if (fontName != null)
            node.setAttribute("style:font-name", fontName);

        if (fontColor != null)
            node.setAttribute("fo:color", buildColorString(fontColor));

        if (bgColor != null)
            node.setAttribute("style:text-background-color",
                              buildColorString(bgColor));
    }

    /**
     * Given a {@code Color}, return a string of the form <i>{@literal #rrggbb}</i>.
     *
     * @param   c  The {@code Color} value.
     *
     * @return  The {@code Color} value in the form <i>{@literal #rrggbb}</i>.
     */
    private String buildColorString(Color c) {
        return String.format("#%06X", c.getRGB() & 0x00FFFFFF);
    }

    private static String[] ignored = {
        "style:text-autospace",  "style:text-underline-color",
        "fo:margin-left", "fo:margin-right", "fo:text-indent",
        "fo:margin-top", "fo:margin-bottom", "text:line-number",
        "text:number-lines", "style:country-asian",
        "style:font-size-asian", "style:font-name-complex",
        "style:language-complex", "style:country-complex",
        "style:font-size-complex", "style:punctuation-wrap",
        "fo:language", "fo:country",
        "style:font-name-asian", "style:language-asian",
        "style:line-break", "fo:keep-with-next"
    };

    /**
     * This code checks whether an attribute is one that we intentionally ignore.
     *
     * @param   attribute  The attribute to check.
     *
     * @return  {@code true} if {@code attribute} can be ignored, otherwise
     *          {@code false}.
     */
    private boolean isIgnored(String attribute) {
        for (String ignored1 : ignored) {
            if (ignored1.equals(attribute)) {
                return true;
            }
        }
        return false;
    }
}