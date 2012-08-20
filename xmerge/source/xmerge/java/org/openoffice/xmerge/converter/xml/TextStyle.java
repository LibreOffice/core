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
 *  Represents a text <code>Style</code> in an OpenOffice document.
 *
 *  @author   David Proulx
 */
public class TextStyle extends Style implements Cloneable {

    final protected static int FIRST_ATTR = 0x01;
    /** Indicates <i>bold</i> text. */
    final public static int BOLD        = 0x01;
    /** Indicates <i>italic</i> text. */
    final public static int ITALIC      = 0x02;
    /** Indicates <i>underlined</i> text. */
    final public static int UNDERLINE   = 0x04;
    /** Indicates <i>strike-through</i> in the text. */
    final public static int STRIKETHRU  = 0x08;
    /** Indicates <i>superscripted</i> text. */
    final public static int SUPERSCRIPT = 0x10;
    /** Indicates <i>subscripted</i> text. */
    final public static int SUBSCRIPT   = 0x20;
    /** Indicates the last attribute. */
    final protected static int LAST_ATTR = 0x20;

    /** Values of text attributes. */
    protected int values = 0;
    /** Bitwise mask of text attributes. */
    protected int mask = 0;

    /** Font size in points. */
    protected int sizeInPoints = 0;
    /** Font name. */
    protected String fontName = null;
    /** Font <code>Color</code>. */
    protected Color fontColor = null;
    /** Background <code>Color</code>. */
    protected Color bgColor = null;

    /**
     *  Constructor for use when going from DOM to client device format.
     *
     *  @param  node  The <i>style:style</i> <code>Node</code> containing
     *                the <code>Style</code>.  (This <code>Node</code> is
     *                assumed have a <i>family</i> attribute of <i>text</i>).
     *  @param  sc    The <code>StyleCatalog</code>, which is used for
     *                looking up ancestor <code>Style</code> objects.
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
        if (node.hasChildNodes()) {
            NodeList children = node.getChildNodes();
            int len = children.getLength();
            for (int i = 0; i < len; i++) {
                Node child = children.item(i);
                String name = child.getNodeName();
                if (name.equals("style:properties")) {
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
    }


    /**
     *  Constructor for use when going from client device format to DOM
     *
     *  @param  name     Name of text <code>Style</code>.  Can be null.
     *  @param  family   Family of text <code>Style</code> (usually
     *                   <i>text</i>).  Can be null.
     *  @param  parent   Name of parent text <code>Style</code>, or null
     *                   for none.
     *  @param  mask     Bitwise mask of text attributes that this text
     *                   <code>Style</code> will specify.  Can be any
     *                   combination of the following, or'ed together:
     *                   {@link #BOLD}, {@link #ITALIC}, {@link #UNDERLINE},
     *                   {@link #STRIKETHRU}, {@link #SUPERSCRIPT},
     *                   {@link #SUBSCRIPT}.  This parameter determines what
     *                   attributes this <code>Style</code> will specify.
     *                   When an attribute is specified in a
     *                   <code>Style</code>, its value can be either
     *                   <i>on</i> or <i>off</i>.  The on/off value for
     *                   each attribute is controlled by the
     *                   <code>values</code> parameter.
     *  @param values    Values of text attributes that this text
     *                   <code>Style</code> will be setting.  Any of the
     *                   attributes ({@link #BOLD}, etc) listed for
     *                   <code>mask</code> can be used for this.
     *  @param fontSize  Font size in points.
     *  @param fontName  Name of font.
     *  @param sc        The <code>StyleCatalog</code>, which is used for
     *                   looking up ancestor <code>Style</code> objects.
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
     *  Parse a color specification of the form <i>#rrggbb</i>
     *
     *  @param  value  <code>Color</code> specification to parse.
     *
     *  @return  The <code>Color</code> associated the value.
     */
    private Color parseColorString(String value) {
        // Assume color value is of form #rrggbb
        String r = value.substring(1, 3);
        String g = value.substring(3, 5);
        String b = value.substring(5, 7);
        int red = 0;
        int green = 0;
        int blue = 0;
        try {
            red = Integer.parseInt(r, 16);
            green = Integer.parseInt(g, 16);
            blue = Integer.parseInt(b, 16);
        } catch (NumberFormatException e) {
            Debug.log(Debug.ERROR, "Problem parsing a color string", e);
        }
        return new Color(red, green, blue);
    }


    /**
     *  Set an attribute.
     *
     *  @param  attr   The attribute to set.
     *  @param  value  The attribute value to set.
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
                String firstPart = value.substring(0, value.indexOf(" "));
                if (firstPart.endsWith("%")) {
                    firstPart = firstPart.substring(0, value.indexOf("%"));
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
     *  Return true if text <code>attribute</code> is set in this
     *  <code>Style</code>.  An attribute that is set may have a
     *  value of <i>on</i> or <i>off</i>.
     *
     *  @param  attribute  The attribute to check ({@link #BOLD},
     *                     {@link #ITALIC}, etc.).
     *
     *  @return  true if text <code>attribute</code> is set in this
     *           <code>Style</code>, false otherwise.
     */
    public boolean isSet(int attribute) {
        return (!((mask & attribute) == 0));
    }


    /**
     *  Return true if the <code>attribute</code> is set to <i>on</i>
     *
     *  @param  attribute  Attribute to check ({@link #BOLD},
     *                     {@link #ITALIC}, etc.)
     *
     *  @return  true if <code>attribute</code> is set to <i>on</i>,
     *           otherwise false.
     */
    public boolean getAttribute(int attribute) {
        if ((mask & attribute) == 0)
            return false;
        return (!((values & attribute) == 0));
    }


    /**
     *  Return the font size for this <code>Style</code>.
     *
     *  @return  The font size in points
     */
    public int getFontSize() {
        return sizeInPoints;
    }


    /**
     *  Return the name of the font for this <code>Style</code>.
     *
     *  @return  Name of font, or null if no font is specified by
     *          this <code>Style</code>.
     */
    public String getFontName() {
        return fontName;
    }


    /**
     *  Return the font <code>Color</code> for this <code>Style</code>.
     *  Can be null if none was specified.
     *
     *  @return  <code>Color</code> value for this <code>Style</code>.
     *           Can be null.
     */
    public Color getFontColor() {
        return fontColor;
    }


    /**
     *  Return the background <code>Color</code> for this
     *  <code>Style</code>.  Can be null if none was specified.
     *
     *  @return  Background <code>Color</code> value for this
     *           <code>Style</code>.  Can be null.
     */
    public Color getBackgroundColor() {
        return bgColor;
    }


    /**
     *  Set the font and/or background <code>Color</code> for this
     *  <code>Style</code>.
     *
     *  @param  fontColor        The font <code>Color</code> to set.
     *  @param  backgroundColor  The background <code>Color</code> to set.
     */
    public void setColors(Color fontColor, Color backgroundColor) {
        if (fontColor != null)
            this.fontColor = fontColor;
        if (backgroundColor != null)
            this.bgColor = backgroundColor;
    }


    /**
     *  Return a <code>Style</code> object corresponding to this one,
     *  but with all of the inherited information from parent
     *  <code>Style</code> objects filled in.  The object returned will
     *  be a new object, not a reference to this object, even if it does
     *  not need any information added.
     *
     *  @return  The <code>StyleCatalog</code> in which to look up
     *           ancestors.
     */
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
     *  Set one or more text attributes to <i>on</i>.
     *
     *  @param  flags  Flag values to set <i>on</i>.
     */
    private void turnAttributesOn(int flags) {
        mask |= flags;
        values |= flags;
    }


    /**
     *  Set one or more text attributes to <i>off</i>.
     *
     *  @param  flags  The flag values to set <i>off</i>.
     */
    private void turnAttributesOff(int flags) {
        mask |= flags;
        values &= ~flags;
    }


    /**
     *  Private function to return the value as an element in
     *  a Comma Separated Value (CSV) format.
     *
     *  @param  value The value to format.
     *
     *  @return  The formatted value.
     */
    private static String toCSV(String value) {
        if (value != null)
            return "\"" + value + "\",";
        else
            return "\"\",";
    }


    /**
     *  Private function to return the value as a last element in
     *  a Comma Separated Value (CSV) format.
     *
     *  @param  value  The value to format.
     *
     *  @return  The formatted value.
     */
    private static String toLastCSV(String value) {
        if (value != null)
            return "\"" + value + "\"";
        else
            return "\"\"";
    }


    /**
     *  Print a Comma Separated Value (CSV) header line for the
     *  spreadsheet dump.
     */
    public static void dumpHdr() {
        System.out.println(toCSV("Name") + toCSV("Family") + toCSV("parent")
        + toCSV("Font") + toCSV("Size")
        + toCSV("Bold") + toCSV("Italic") + toCSV("Underline")
        + toCSV("Strikethru") + toCSV("Superscript") + toLastCSV("Subscript"));
    }


    /**
     *  Dump this <code>Style</code> as a Comma Separated Value (CSV) line.
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
     *  Create a new <code>Node</code> in the <code>Document</code>, and
     *  write this <code>Style</code> to it.
     *
     *  @param  parentDoc  Parent <code>Document</code> of the
     *                    <code>Node</code> to create.
     *  @param  name       Name to use for the new <code>Node</code> (e.g.
     *                    <i>style:style</i>)
     *
     *  @return  Created <code>Node</code>.
     */
    public Node createNode(org.w3c.dom.Document parentDoc, String name) {
        Element node = parentDoc.createElement(name);
        writeAttributes(node);
        return node;
    }


    /**
     *  Return true if <code>style</code> specifies as much or less
     *  than this <code>Style</code>, and nothing it specifies
     *  contradicts this <code>Style</code>.
     *
     *  @param  style  The <code>Style</code> to check.
     *
     *  @return  true if <code>style</code> is a subset, false
     *           otherwise.
     */
    public boolean isSubset(Style style) {
        if (style.getClass() != this.getClass())
                return false;
        TextStyle tStyle = (TextStyle)style;

        if (tStyle.values != values)
                return false;

        if (tStyle.sizeInPoints != 0) {
            if (sizeInPoints != tStyle.sizeInPoints)
                return false;
        }

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
     *  Write this <code>Style</code> object's attributes to a
     *  <code>Node</code> in the <code>Document</code>.
     *
     *  @param  node  The <code>Node</code> to add <code>Style</code>
     *                attributes.
     */
    public void writeAttributes(Element node) {

        if ((mask & BOLD) != 0)
            if ((values & BOLD) != 0)
                node.setAttribute("fo:font-weight", "bold");

        if ((mask & ITALIC) != 0)
            if ((values & ITALIC) != 0)
                node.setAttribute("fo:font-style", "italic");

        if ((mask & UNDERLINE) != 0)
            if ((values & UNDERLINE) != 0)
                node.setAttribute("style:text-underline", "single");

        if ((mask & STRIKETHRU) != 0)
            if ((values & STRIKETHRU) != 0)
                node.setAttribute("style:text-crossing-out", "single-line");

        if ((mask & SUPERSCRIPT) != 0)
            if ((values & SUPERSCRIPT) != 0)
                node.setAttribute("style:text-position", "super 58%");

        if ((mask & SUBSCRIPT) != 0)
            if ((values & SUBSCRIPT) != 0)
                node.setAttribute("style:text-position", "sub 58%");

        if (sizeInPoints != 0) {
            Integer fs = new Integer(sizeInPoints);
            node.setAttribute("fo:font-size", fs.toString() + "pt");
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
     *  Given a <code>Color</code>, return a string of the form
     *  <i>#rrggbb</i>.
     *
     *  @param  c  The <code>Color</code> value.
     *
     *  @return  The <code>Color</code> value in the form <i>#rrggbb</i>.
     */
    private String buildColorString(Color c) {
        int v[] = new int[3];
        v[0] = c.getRed();
        v[1] = c.getGreen();
        v[2] = c.getBlue();
        String colorString = new String("#");
        for (int i = 0; i <= 2; i++) {
            String xx = Integer.toHexString(v[i]);
            if (xx.length() < 2)
        xx = "0" + xx;
            colorString += xx;
    }
        return colorString;
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


    /*
     * This code checks whether an attribute is one that we
     * intentionally ignore.
     *
     *  @param  attribute  The attribute to check.
     *
     *  @return  true if <code>attribute</code> can be ignored,
     *           otherwise false.
     */
    private boolean isIgnored(String attribute) {
        for (int i = 0; i < ignored.length; i++) {
            if (ignored[i].equals(attribute))
                return true;
        }
        return false;
    }
}

