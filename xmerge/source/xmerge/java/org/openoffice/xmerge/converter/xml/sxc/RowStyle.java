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

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import org.openoffice.xmerge.converter.xml.Style;
import org.openoffice.xmerge.converter.xml.StyleCatalog;
import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.TwipsConverter;

/**
 * Represents a text {@code Style} in an OpenOffice document.
 */
public class RowStyle extends Style implements Cloneable {

    private int rowHeight = 255;
    /**
     * Constructor for use when going from DOM to client device format.
     *
     * @param  node  The <i>style:style</i> {@code Node} containing the
     *               {@code Style}.  (This {@code Node} is assumed have a
     *               <i>family</i> attribute of <i>text</i>).
     * @param  sc    The {@code StyleCatalog}, which is used for looking up
     *               ancestor {@code Style} objects.
     */
    public RowStyle(Node node, StyleCatalog sc) {
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
     * Constructor for use when going from client device format to DOM.
     *
     * @param  name       Name of text {@code Style}.  Can be {@code null}.
     * @param  family     Family of text {@code Style} (usually <i>text</i>).
     *                    Can be {@code null}.
     * @param  parent     Name of parent text {@code Style}, or {@code null}
     *                    for none.
     * @param  rowHeight  The height of this row.
     * @param  sc         The {@code StyleCatalog}, which is used for looking up
     *                    ancestor {@code Style} objects.
     */
    public RowStyle(String name, String family, String parent,int rowHeight, StyleCatalog sc) {
        super(name, family, parent, sc);
        this.rowHeight=rowHeight;
    }

    /**
     * Returns the height of this row.
     *
     * @return The height of this row.
     */
    public int getRowHeight() {
        return rowHeight;
    }

    /**
     * Sets the height of this row.
     *
     * @param  RowHeight  The height of this row.
     */
    private void setRowHeight(int RowHeight) {
        this.rowHeight = RowHeight;
    }

    /**
     * Parse a rowheight in the form {@literal "1.234cm"} to twips.
     *
     * @param   value  {@code String} specification to parse.
     *
     * @return  The twips equivalent.
     */
    private int parseRowHeight(String value) {
        return TwipsConverter.convert2twips(value, 255);
    }

    /**
     * Set an attribute.
     *
     * @param  attr   The attribute to set.
     * @param  value  The attribute value to set.
     */
    private void handleAttribute(String attr, String value) {

        if (attr.equals("style:row-height")) {
            rowHeight = parseRowHeight(value);
        }
        else {
            Debug.log(Debug.INFO, "RowStyle Unhandled: " + attr + "=" + value);
        }
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
        RowStyle resolved = null;
        try {
            resolved = (RowStyle)this.clone();
        } catch (Exception e) {
            Debug.log(Debug.ERROR, "Can't clone", e);
        }

        // Look up the parentStyle.  (If there is no style catalog
        // specified, we can't do any lookups.)
        RowStyle parentStyle = null;
        if (sc != null) {
            if (parent != null) {
                parentStyle = (RowStyle)sc.lookup(parent, family, null,
                           this.getClass());
                if (parentStyle == null)
                    Debug.log(Debug.ERROR, "parent style lookup of "
                      + parent + " failed!");
                else
                    parentStyle = (RowStyle)parentStyle.getResolved();

            } else if (!name.equals("DEFAULT_STYLE")) {
                parentStyle = (RowStyle)sc.lookup("DEFAULT_STYLE", null,
                    null, this.getClass());
            }
        }

        // If we found a parent, for any attributes which we don't have
        // set, try to get the values from the parent.
        if (parentStyle != null) {
            parentStyle = (RowStyle)parentStyle.getResolved();

            if ((rowHeight == 0) && (parentStyle.getRowHeight() != 0))
                resolved.setRowHeight(parentStyle.getRowHeight());
        }
        return resolved;
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
     * Return true if {@code style} specifies as much or less than this
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
        RowStyle tStyle = (RowStyle)style;

        return rowHeight == tStyle.getRowHeight();
    }

    /**
     * Write this {@code Style} object's attributes to a {@code Node} in the
     * {@code Document}.
     *
     * @param  node  The {@code Node} to add {@code Style} attributes.
     */
    private void writeAttributes(Element node) {

        if(rowHeight!=0) {
            String height = TwipsConverter.twips2cm(rowHeight) + "cm";
            node.setAttribute("style:row-height", height);
        }
    }
}