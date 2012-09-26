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

package org.openoffice.xmerge.converter.xml.sxc;

import java.awt.Color;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.converter.xml.Style;
import org.openoffice.xmerge.converter.xml.StyleCatalog;
import org.openoffice.xmerge.util.Debug;

/**
 *  Represents a text <code>Style</code> in an OpenOffice document.
 */
public class CellStyle extends Style implements Cloneable {

    private Format fmt = new Format();

    /**
     *  Constructor for use when going from DOM to client device format.
     *
     *  @param  node  The <i>style:style</i> <code>Node</code> containing
     *                the <code>Style</code>.  (This <code>Node</code> is
     *                assumed have a <i>family</i> attribute of <i>text</i>).
     *  @param  sc    The <code>StyleCatalog</code>, which is used for
     *                looking up ancestor <code>Style</code> objects.
     */
    public CellStyle(Node node, StyleCatalog sc) {
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
     *  @param  name    Name of cell <code>Style</code>.  Can be null.
     *  @param  family  Family of text <code>Style</code> (usually
     *                  <i>text</i>).  Can be null.
     *  @param  parent  Name of parent text <code>Style</code>, or null
     *                  for none.
     *  @param  fmt     size in points.
     *  @param  sc      The <code>StyleCatalog</code>, which is used for
     *                  looking up ancestor <code>Style</code> objects.
     */
    public CellStyle(String name, String family, String parent,Format fmt, StyleCatalog sc) {
        super(name, family, parent, sc);
        this.fmt = fmt;
    }

    /**
     * Returns the <code>Format</code> object for this particular style
     *
     * @return the <code>Format</code> object
     */
    public Format getFormat() {
        return fmt;
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
        return new Color(red, green, blue, 0);
    }


    /**
     *  Set an attribute.
     *
     *  @param  attr   The attribute to set.
     *  @param  value  The attribute value to set.
     */
    private void handleAttribute(String attr, String value) {

        if (attr.equals("fo:font-weight")) {
            fmt.setAttribute(Format.BOLD, value.equals("bold"));
        }

        else if (attr.equals("fo:font-style")) {
            if (value.equals("italic")  || value.equals("oblique"))
                fmt.setAttribute(Format.ITALIC, true);
            else if (value.equals("normal"))
                fmt.setAttribute(Format.ITALIC, false);
        }

        else if (attr.equals("style:text-underline")) {
            fmt.setAttribute(Format.UNDERLINE, !value.equals("none"));
        }

        else if (attr.equals("style:text-crossing-out")) {
            fmt.setAttribute(Format.STRIKETHRU, !value.equals("none"));
        }

        else if (attr.equals("style:text-position")) {
            if (value.startsWith("super "))
                fmt.setAttribute(Format.SUPERSCRIPT, true);
            else if (value.startsWith("sub "))
                fmt.setAttribute(Format.SUBSCRIPT, true);
            else if (value.startsWith("0% "))
                fmt.setAttribute(Format.SUPERSCRIPT | Format.SUBSCRIPT, false);
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
                    if (amount < 0) fmt.setAttribute(Format.SUBSCRIPT, true);
                    else if (amount > 0) fmt.setAttribute(Format.SUPERSCRIPT, false);
                }
            }
        }

        else if (attr.equals("fo:font-size")) {
            if (value.endsWith("pt")) {
                String num = value.substring(0, value.length() - 2);
                fmt.setFontSize(Integer.parseInt(num));
            }
        }

        else if (attr.equals("style:font-name"))
            fmt.setFontName(value);

        else if (attr.equals("fo:color"))
            fmt.setForeground(parseColorString(value));

        else if (attr.equals("fo:background-color"))
            fmt.setBackground(parseColorString(value));

        else if (attr.equals("fo:text-align")) {
            if(value.equals("center")) {
                fmt.setAlign(Format.CENTER_ALIGN);
            } else if(value.equals("end")) {
                fmt.setAlign(Format.RIGHT_ALIGN);
            } else if(value.equals("start")) {
                fmt.setAlign(Format.LEFT_ALIGN);
            }
        }

        else if (attr.equals("fo:vertical-align")) {
            if(value.equals("top")) {
                fmt.setVertAlign(Format.TOP_ALIGN);
            } else if(value.equals("middle")) {
                fmt.setVertAlign(Format.MIDDLE_ALIGN);
            } else if(value.equals("bottom")) {
                fmt.setVertAlign(Format.BOTTOM_ALIGN);
            }
        }

        else if (attr.equals("fo:border")) {
            fmt.setAttribute(Format.TOP_BORDER, !value.equals("none"));
            fmt.setAttribute(Format.BOTTOM_BORDER, !value.equals("none"));
            fmt.setAttribute(Format.LEFT_BORDER, !value.equals("none"));
            fmt.setAttribute(Format.RIGHT_BORDER, !value.equals("none"));
        }
        else if (attr.equals("fo:border-top")) {
                fmt.setAttribute(Format.TOP_BORDER, !value.equals("none"));
        }
        else if (attr.equals("fo:border-bottom")) {
            fmt.setAttribute(Format.BOTTOM_BORDER, !value.equals("none"));
        }
        else if (attr.equals("fo:border-left")) {
            fmt.setAttribute(Format.LEFT_BORDER, !value.equals("none"));
        }
        else if (attr.equals("fo:border-right")) {
            fmt.setAttribute(Format.RIGHT_BORDER, !value.equals("none"));
        }
        else if (attr.equals("fo:wrap-option")) {
            fmt.setAttribute(Format.WORD_WRAP, value.equals("wrap"));
        }

        else if (isIgnored(attr)) {}

        else {
            Debug.log(Debug.INFO, "CellStyle Unhandled: " + attr + "=" + value);
        }
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
        CellStyle resolved = null;
        try {
            resolved = (CellStyle)this.clone();
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Can't clone", e);
        }

        // Look up the parentStyle.  (If there is no style catalog
        // specified, we can't do any lookups.)
        CellStyle parentStyle = null;
        if (sc != null) {
            if (parent != null) {
                parentStyle = (CellStyle)sc.lookup(parent, family, null,
                           this.getClass());
                if (parentStyle == null)
                    Debug.log(Debug.ERROR, "parent style lookup of "
                      + parent + " failed!");
                else
                    parentStyle = (CellStyle)parentStyle.getResolved();

            } else if (!name.equals("DEFAULT_STYLE")) {
                parentStyle = (CellStyle)sc.lookup("DEFAULT_STYLE", null,
                    null, this.getClass());
            }
        }

        // If we found a parent, for any attributes which we don't have
        // set, try to get the values from the parent.
        if (parentStyle != null) {
            parentStyle = (CellStyle)parentStyle.getResolved();
            Format parentFormat = parentStyle.getFormat();
            Format resolvedFormat = resolved.getFormat();

            if ((fmt.getAlign() == Format.LEFT_ALIGN) && (parentFormat.getAlign() != Format.LEFT_ALIGN))
                resolvedFormat.setAlign(parentFormat.getAlign());
            if ((fmt.getVertAlign() == Format.BOTTOM_ALIGN) && (parentFormat.getVertAlign() != Format.BOTTOM_ALIGN))
                resolvedFormat.setVertAlign(parentFormat.getVertAlign());
            if ((fmt.getFontSize() == 0) && (parentFormat.getFontSize() != 0))
                resolvedFormat.setFontSize(parentFormat.getFontSize());
            if ((fmt.getFontName() == null) && (parentFormat.getFontName() != null))
                resolvedFormat.setFontName(parentFormat.getFontName());
            if ((fmt.getForeground() == null) && (parentFormat.getForeground() != null))
                resolvedFormat.setForeground(parentFormat.getForeground());
            if ((fmt.getBackground() == null) && (parentFormat.getBackground() != null))
                resolvedFormat.setBackground(parentFormat.getBackground());
            for (int m = Format.BOLD; m <= Format.SUBSCRIPT; m = m << 1) {
                if ((fmt.getAttribute(m)) && (parentFormat.getAttribute(m))) {
                    resolvedFormat.setAttribute(m, parentFormat.getAttribute(m));
                }
            }

        }
        return resolved;
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
        CellStyle tStyle = (CellStyle)style;

        Format rhs = tStyle.getFormat();

        if(!fmt.isSubset(rhs))
            return false;

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

        if (fmt.getAlign()==Format.RIGHT_ALIGN)
            node.setAttribute("fo:text-align", "end");

        if (fmt.getAlign()==Format.LEFT_ALIGN)
            node.setAttribute("fo:text-align", "start");

        if (fmt.getAlign()==Format.CENTER_ALIGN)
            node.setAttribute("fo:text-align", "center");

         if (fmt.getVertAlign()==Format.TOP_ALIGN)
            node.setAttribute("fo:vertical-align", "top");

        if (fmt.getVertAlign()==Format.MIDDLE_ALIGN)
            node.setAttribute("fo:vertical-align", "middle");

        if (fmt.getVertAlign()==Format.BOTTOM_ALIGN)
            node.setAttribute("fo:vertical-align", "bottom");

        if (fmt.getAttribute(Format.BOLD))
            node.setAttribute("fo:font-weight", "bold");

        if (fmt.getAttribute(Format.ITALIC))
            node.setAttribute("fo:font-style", "italic");

        if (fmt.getAttribute(Format.UNDERLINE))
            node.setAttribute("style:text-underline", "single");

        if (fmt.getAttribute(Format.STRIKETHRU))
            node.setAttribute("style:text-crossing-out", "single-line");

        if (fmt.getAttribute(Format.SUPERSCRIPT))
            node.setAttribute("style:text-position", "super 58%");

        if (fmt.getAttribute(Format.SUBSCRIPT))
            node.setAttribute("style:text-position", "sub 58%");

        if (fmt.getFontSize() != 0) {
            Integer fs = new Integer(fmt.getFontSize());
            node.setAttribute("fo:font-size", fs.toString() + "pt");
        }

        if (fmt.getFontName() != null)
            node.setAttribute("style:font-name", fmt.getFontName());

        if (fmt.getForeground() != null)
            node.setAttribute("fo:color", buildColorString(fmt.getForeground()));

        if (fmt.getBackground() != null)
            node.setAttribute("fo:background-color",
                              buildColorString(fmt.getBackground()));

        if (fmt.getAttribute(Format.TOP_BORDER))
            node.setAttribute("fo:border-top", "0.0008inch solid #000000");

        if (fmt.getAttribute(Format.BOTTOM_BORDER))
            node.setAttribute("fo:border-bottom", "0.0008inch solid #000000");

        if (fmt.getAttribute(Format.RIGHT_BORDER))
            node.setAttribute("fo:border-right", "0.0008inch solid #000000");

        if (fmt.getAttribute(Format.LEFT_BORDER))
            node.setAttribute("fo:border-left", "0.0008inch solid #000000");

        if (fmt.getAttribute(Format.WORD_WRAP))
            node.setAttribute("fo:wrap-option", "wrap");

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

